#if os(Linux)
  import Foundation
  import Glibc
  import JetlinkServer

  /// Suspends the box once the comma has been gone long enough (Python's
  /// `jetlink/server/sleep.py`).
  ///
  /// On an always-on supply a Jetson idles at about 7 W, a flat battery over
  /// a long park. Deep suspend keeps the engine resident: about 6 s back,
  /// against about 65 s for a cold boot and a plan reload. USB is the wake
  /// source and both edges wake it, so this is a loop, not a command from the
  /// comma: ignition-off pulls the gadget and wakes the box, and no gadget for
  /// `after` seconds means sleep again. Writing `mem` to /sys/power/state
  /// blocks until resume, so the USB loop carries on where it stopped with
  /// the engine, the CUDA context and the gadget's descriptor intact.
  ///
  /// `jetlink caffeinate` holds the box awake with a shared flock on
  /// /run/jetlink-awake.lock; a held lock puts off the suspend until it goes.
  public final class Sleeper: @unchecked Sendable {
    public static let retryMin: TimeInterval = 10
    public static let retryMax: TimeInterval = 300
    /// The USB wake is not guaranteed: a sleeping Jetson once answered a
    /// bind with a bus reset and no enumeration through four connect cycles
    /// and needed its button, which in the car is a drive on the small model.
    /// An RTC alarm does not depend on the path that failed. A park wakes on
    /// it every half hour and, with no gadget, sleeps again after
    /// `backstopGrace`: about 20 s awake in 30 min counting the resume, near
    /// 1%, under a tenth of a watt at the 7-8 W it draws awake. Waiting the
    /// whole count instead kept it up 6% of the park, about half a watt.
    public static let wakeBackstop: TimeInterval = 1800
    /// How long a wake the backstop caused waits for a gadget before it
    /// sleeps again: time for a USB wake that raced the alarm to enumerate
    /// the comma's gadget and for the USB loop to claim it.
    public static let backstopGrace: TimeInterval = 15
    /// Outside any systemd RuntimeDirectory, so a restart of the server never
    /// unlinks it under a holder.
    public static let awakeLock = "/run/jetlink-awake.lock"

    public let after: TimeInterval
    let root: HostRoot
    let lockPath: String
    /// CLOCK_MONOTONIC, for the idle count: it stands still while asleep.
    let monotonic: @Sendable () -> TimeInterval
    /// Every write to sysfs: `mem` to /sys/power/state returns once the box
    /// is back. A test's stand-in for the kernel.
    let write: @Sendable (String, String) throws(KernelError) -> Void
    let log: ServerLog

    private let lock = NSLock()
    private(set) var enabled = true
    private var lastSeen: TimeInterval
    /// Seconds without a gadget before the next suspend: `after`, or the
    /// grace after a wake the backstop caused, until anything touches.
    private var limit: TimeInterval
    private var retryAt: TimeInterval = 0
    private var backoff = Sleeper.retryMin
    private var held = false
    /// The last poll found no gadget.
    private var gone = false

    init(
      after: TimeInterval, root: HostRoot, lockPath: String, monotonic: @escaping @Sendable () -> TimeInterval = { Sleeper.now(CLOCK_MONOTONIC) },
      write: @escaping @Sendable (String, String) throws(KernelError) -> Void = Sysfs.write, log: ServerLog = ServerLog(category: "sleep")
    ) {
      self.after = after
      self.root = root
      self.lockPath = lockPath
      self.monotonic = monotonic
      self.write = write
      self.log = log
      lastSeen = monotonic()
      limit = after
    }

    static func now(_ clock: clockid_t) -> TimeInterval {
      var time = timespec()
      clock_gettime(clock, &time)
      return TimeInterval(time.tv_sec) + TimeInterval(time.tv_nsec) / 1e9
    }

    /// `ServerHooks.gadgetIdle`: only an absent gadget may end in a suspend.
    /// True when the box slept.
    public func handle(_ event: GadgetIdleEvent) -> Bool {
      switch event {
      case .present:
        lock.withLock { gone = false }
        touch()
        return false
      case .connected, .disconnected:
        touch()
        return false
      case .absent:
        // The count starts when the gadget goes, as Python's did. A gadget
        // held with no session is not polled, so without this one that
        // leaves after `after` seconds of that suspends the box at once.
        let leaving = lock.withLock {
          defer { gone = true }
          return !gone
        }
        if leaving {
          touch()
          return false
        }
        return idle()
      }
    }

    public func touch() {
      lock.withLock {
        lastSeen = monotonic()
        limit = after
        backoff = Sleeper.retryMin
      }
    }

    /// True when the box slept, and is back.
    public func idle() -> Bool {
      let due: TimeInterval? = lock.withLock {
        let now = monotonic()
        return (enabled && now - lastSeen >= limit && now >= retryAt) ? limit : nil
      }
      guard let due else { return false }
      let holding = heldAwake()
      let wasHeld = lock.withLock {
        defer { held = holding }
        return held
      }
      if holding {
        if !wasHeld { log.info("held awake by jetlink caffeinate") }
        return false
      }
      if wasHeld {
        // The count starts again from the release, not from the comma.
        log.info("no longer held awake; suspending after \(Int(after)) s more without a gadget")
        touch()
        return false
      }
      let wake = suspend(idleFor: due)
      lock.withLock {
        let now = monotonic()
        if let wake {
          // Woken by an edge: give whatever caused it the whole count to show
          // up. The backstop's alarm brings no one, so it gets the grace.
          lastSeen = now
          limit = wake == .backstop ? min(Sleeper.backstopGrace, after) : after
          backoff = Sleeper.retryMin
        } else {
          retryAt = now + backoff
          backoff = min(backoff * 2, Sleeper.retryMax)
        }
      }
      return wake != nil
    }

    /// Creates the hold-awake lock for `jetlink caffeinate`, world-readable
    /// so a holder needs no sudo. Throws when it cannot (not root).
    public static func createAwakeLock(_ path: String = Sleeper.awakeLock) throws(KernelError) {
      let fd = open(path, O_RDONLY | O_CREAT | O_CLOEXEC, 0o644)
      guard fd >= 0 else { throw KernelError(what: path, errno: errno) }
      // The umask may have taken bits off.
      fchmod(fd, 0o644)
      close(fd)
    }

    /// Whether a `jetlink caffeinate` holds the lock. A holder that died let
    /// go with its descriptor, so a stale one never counts.
    func heldAwake() -> Bool {
      let fd = open(lockPath, O_RDONLY | O_CLOEXEC)
      guard fd >= 0 else { return false }
      defer { close(fd) }
      if flock(fd, LOCK_EX | LOCK_NB) == 0 {
        flock(fd, LOCK_UN)
        return false
      }
      return errno == EWOULDBLOCK
    }

    // MARK: the suspend

    /// What ended a suspend that slept.
    enum Wake {
      /// The RTC reached the backstop's alarm.
      case backstop
      /// Anything before it: USB, the button.
      case edge
    }

    /// The wake, or nil when the box did not sleep.
    private func suspend(idleFor idle: TimeInterval) -> Wake? {
      guard selectDeep() else {
        lock.withLock { enabled = false }
        return nil
      }
      let power = root.path("/sys/power")
      let successes = Sysfs.readInt("\(power)/suspend_stats/success") ?? -1
      // CLOCK_BOOTTIME keeps counting while asleep
      let start = Sleeper.now(CLOCK_BOOTTIME)
      log.info("no gadget for \(Int(idle)) s\(idle < after ? " after the wake backstop" : ""), suspending")
      armHubWakeup()
      let alarm = armBackstop()
      do throws(KernelError) {
        try write("\(power)/state", "mem")
      } catch {
        disarmBackstop(alarm)
        if [EACCES, EPERM, EROFS, ENOENT].contains(error.errno) {
          // Configuration, not weather: nothing will change by the next try.
          log.error("cannot write \(power)/state (\(error)); sleep disabled")
          lock.withLock { enabled = false }
        } else {
          // EBUSY is the freezer giving up, EINVAL a mode the platform
          // refused: both are worth another try later.
          log.warning("suspend failed: \(error) (\(failure()))")
        }
        return nil
      }
      disarmBackstop(alarm)
      let asleep = Sleeper.now(CLOCK_BOOTTIME) - start
      if successes >= 0 && (Sysfs.readInt("\(power)/suspend_stats/success") ?? -1) <= successes {
        // A clean return with the counter unmoved: a wake edge landed during
        // the freeze and the box never left.
        log.warning("suspend returned after \(String(format: "%.1f", asleep)) s without sleeping (\(failure()))")
        return nil
      }
      // The alarm's own counter says whether its time came. A USB edge that
      // raced it shows its gadget within the grace; an RTC that cannot be
      // read, or fired early, gets the whole count as any other wake.
      let rtc = root.path("/sys/class/rtc/rtc0/since_epoch")
      if let alarm, let now = Sysfs.readInt(rtc), now >= alarm {
        let grace = Int(min(Sleeper.backstopGrace, after))
        log.info("resumed after \(Int(asleep.rounded())) s asleep, woken by the wake backstop: sleeping again unless a gadget shows up within \(grace) s")
        return .backstop
      }
      log.info("resumed after \(Int(asleep.rounded())) s asleep")
      return .edge
    }

    /// Deep suspend: s2idle keeps the CPUs in idle states and saves nothing
    /// worth having. False when deep is not to be had.
    private func selectDeep() -> Bool {
      let path = root.path("/sys/power/mem_sleep")
      // No mem_sleep at all: "mem" means whatever the platform does.
      guard let modes = Sysfs.read(path), !modes.isEmpty else { return true }
      if modes.contains("[deep]") { return true }
      guard modes.split(separator: " ").contains("deep") else {
        log.error("deep suspend is not available (mem_sleep: \(modes)), not sleeping")
        return false
      }
      do {
        try write(path, "deep")
      } catch {
        log.error("could not select deep suspend: \(error)")
        return false
      }
      return true
    }

    /// Arms remote wakeup on every hub, and says so loudly when it cannot.
    /// The comma hangs off the carrier's Realtek hub, which has to signal a
    /// connect up before the root hub hears about it and which ships
    /// disarmed: disarmed, a sleeping Jetson answered a bind with a bus reset
    /// and needed its button; armed, it resumed 4 s after the bind. Which hub
    /// carries the gadget depends on the negotiated speed, so all of them.
    @discardableResult
    func armHubWakeup() -> [String] {
      let devices = "/sys/bus/usb/devices"
      var disarmed: [String] = []
      for device in root.list(devices) {
        // Interfaces ("2-1:1.0") have no bDeviceClass and are skipped here.
        guard Sysfs.read(root.path("\(devices)/\(device)/bDeviceClass")) == "09" else { continue }
        let wakeup = root.path("\(devices)/\(device)/power/wakeup")
        guard Sysfs.read(wakeup) == "disabled" else { continue }
        do {
          try write(wakeup, "enabled\n")
        } catch {
          disarmed.append(device)
        }
      }
      if !disarmed.isEmpty {
        log.error(
          "hub(s) \(disarmed.joined(separator: ", ")) could not be armed for remote wakeup: the comma presenting its gadget may not wake this box"
        )
      }
      return disarmed
    }

    /// An RTC alarm `wakeBackstop` seconds out, against the RTC's own count:
    /// this box boots unset and never sees NTP in the car, so only both sides
    /// coming from the same counter matters. The alarm's time on that count,
    /// or nil when none is armed.
    private func armBackstop() -> Int? {
      let backstop = Int(Sleeper.wakeBackstop)
      let rtc = root.path("/sys/class/rtc/rtc0")
      do {
        guard let now = Sysfs.readInt("\(rtc)/since_epoch") else {
          throw KernelError(what: "\(rtc)/since_epoch", errno: ENODATA)
        }
        // A stale alarm blocks setting a new one.
        try write("\(rtc)/wakealarm", "0\n")
        try write("\(rtc)/wakealarm", "\(now + backstop)\n")
        return now + backstop
      } catch {
        // The USB edge is still the wake source; this was only the backstop.
        log.warning("could not arm the \(backstop) s wake backstop: \(error)")
        return nil
      }
    }

    private func disarmBackstop(_ alarm: Int?) {
      guard alarm != nil else { return }
      // It has either fired or fires once and is spent: a failure is fine.
      try? write(root.path("/sys/class/rtc/rtc0/wakealarm"), "0\n")
    }

    private func failure() -> String {
      let stats = root.path("/sys/power/suspend_stats")
      let step = Sysfs.read("\(stats)/last_failed_step").flatMap { $0.isEmpty ? nil : $0 } ?? "?"
      let device = Sysfs.read("\(stats)/last_failed_dev") ?? ""
      return "last failed step \(step)" + (device.isEmpty ? "" : " in \(device)")
    }
  }
#endif
