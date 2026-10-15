import Foundation
import JetlinkKit

/// The host's side of the page's hardware panel: JetlinkLinux's
/// `PageHardware` on a Jetson or a Linux PC, none elsewhere. Only the page's
/// sampler thread calls it, one call at a time, and only while a page is open.
public protocol PageHardwareSource: AnyObject, Sendable {
  /// The `host` event's fields: hostname, board, os, kernel, gpu, jetson.
  /// Asked once.
  func host() -> [String: Any]
  /// One `hw` event's fields.
  func sample() -> [String: Any]
  /// Forgets the last CPU counters: after a pause, a load would average over it.
  func reset()
}

/// What every open page hears, and what a page that opens late is given
/// first: the latest of each state event, the last two minutes of stats (the
/// chart's), the host and the latest hardware sample.
///
/// A server thread only hands an event over: `publish` queues it and
/// returns, so no thread of the server's ever waits on a lock a page's
/// low-priority thread holds. Each event is encoded once, off the server's
/// threads, and every page is given the same bytes.
final class PageFeed: @unchecked Sendable {
  struct Limits: Sendable {
    /// Event streams at once; each holds a thread.
    var streams = 8
    /// Stats kept for a page that opens mid-drive: the chart's two minutes.
    var history: TimeInterval = 120
    /// Hardware sampling outlives the last page by this much: a reload, a
    /// phone waking.
    var grace: TimeInterval = 60
    var period: TimeInterval = 1
    /// How far a page may fall behind before it is dropped. It reconnects
    /// and starts again from the latest state.
    var behind = 1024
  }

  /// One open page's queue of server-sent events.
  final class Stream: @unchecked Sendable {
    fileprivate var frames: [Data] = []
    fileprivate var dropped = false
  }

  /// Replayed in this order, then the stats history. The catalog, the last
  /// download and the last benchmark exist only once a signed-in page asked
  /// for them.
  static let order = ["hello", "server", "link", "engine", "inventory", "catalog", "download", "benchmark"]
  /// Frames this recent mean the car is driving on the big model.
  static let driving: TimeInterval = 10
  static let keepalive = Data(": keepalive\n\n".utf8)

  let limits: Limits
  private let hardware: (any PageHardwareSource)?
  /// Monotonic seconds, for the history and the grace; `t` is wall time.
  private let clock: @Sendable () -> TimeInterval
  static let uptime: @Sendable () -> TimeInterval = { ProcessInfo.processInfo.systemUptime }
  private let handoff = DispatchQueue(label: "jetlink-page-feed", qos: .background)
  private let condition = NSCondition()
  private var latest: [String: (event: ControlEvent, frame: Data)] = [:]
  private var stats: [(frame: Data, at: TimeInterval)] = []
  private var streams: [Stream] = []
  private var hostFrame: Data?
  private var hwFrame: Data?
  private var sampling = false
  private var lastLeft = -TimeInterval.infinity
  private var lastStats = -TimeInterval.infinity
  private var stopped = false

  init(hardware: (any PageHardwareSource)?, limits: Limits = Limits(), clock: @escaping @Sendable () -> TimeInterval = PageFeed.uptime) {
    self.hardware = hardware
    self.limits = limits
    self.clock = clock
  }

  /// What a page shows. The catalog, downloads and benchmarks only come
  /// after a signed-in page's command; imports are the apps', and replies go
  /// to the page that asked, never here.
  static func isRelayed(_ event: ControlEvent) -> Bool {
    switch event {
    case .hello, .server, .link, .engine, .inventory, .stats, .catalog, .download, .benchmark: true
    default: false
    }
  }

  /// From any thread, the server's included: hands `event` over and returns.
  func publish(_ event: ControlEvent) {
    guard PageFeed.isRelayed(event) else { return }
    let date = Date()
    let at = clock()
    handoff.async { self.accept(event, date: date, at: at, onlyNew: false) }
  }

  /// What no event has said yet, from `ServerController.currentState()`.
  /// Runs after everything handed over before it, and an event that already
  /// arrived is newer than the seed, so it stands.
  func seed(_ events: [ControlEvent]) {
    let date = Date()
    let at = clock()
    handoff.sync {
      for event in events where PageFeed.isRelayed(event) {
        accept(event, date: date, at: at, onlyNew: true)
      }
    }
  }

  func flush() {
    handoff.sync {}
  }

  /// On `handoff`.
  private func accept(_ event: ControlEvent, date: Date, at: TimeInterval, onlyNew: Bool) {
    let isStats = if case .stats = event { true } else { false }
    if onlyNew && isStats { return }
    let frame = PageFeed.sse(event.jsonLine(at: date))
    condition.lock()
    defer { condition.unlock() }
    if isStats {
      stats.append((frame, at))
      lastStats = max(lastStats, at)
      trimHistory(now: at)
    } else {
      if onlyNew && latest[event.name] != nil { return }
      latest[event.name] = (event, frame)
    }
    push(frame)
  }

  /// Under `condition`.
  private func push(_ frame: Data) {
    for stream in streams where !stream.dropped {
      stream.frames.append(frame)
      if stream.frames.count > limits.behind {
        stream.dropped = true
        stream.frames = []
      }
    }
    condition.broadcast()
  }

  /// Under `condition`.
  private func trimHistory(now: TimeInterval) {
    let keep = stats.firstIndex { $0.at >= now - limits.history } ?? stats.count
    stats.removeFirst(keep)
  }

  // MARK: pages

  /// A new page's stream, holding the whole picture so far; nil while
  /// `limits.streams` pages are open.
  func open() -> Stream? {
    condition.lock()
    guard !stopped, streams.count < limits.streams else {
      condition.unlock()
      return nil
    }
    let stream = Stream()
    trimHistory(now: clock())
    if let hostFrame { stream.frames.append(hostFrame) }
    stream.frames += PageFeed.order.compactMap { latest[$0]?.frame }
    stream.frames += stats.map(\.frame)
    if let hwFrame { stream.frames.append(hwFrame) }
    streams.append(stream)
    let start = hardware != nil && !sampling
    if start { sampling = true }
    condition.unlock()
    if start, let hardware {
      PageThread.start("jetlink-page-hw") { [self] in sampleWhileWatched(hardware) }
    }
    return stream
  }

  func close(_ stream: Stream) {
    condition.lock()
    streams.removeAll { $0 === stream }
    if streams.isEmpty { lastLeft = clock() }
    condition.unlock()
  }

  /// What `stream` has to send: waits up to `timeout` and gives [] when
  /// nothing came (time for a keepalive), nil when the stream is over.
  func next(_ stream: Stream, timeout: TimeInterval) -> [Data]? {
    let deadline = Date(timeIntervalSinceNow: timeout)
    condition.lock()
    defer { condition.unlock() }
    while stream.frames.isEmpty && !stream.dropped && !stopped {
      if !condition.wait(until: deadline) { break }
    }
    if stream.dropped || stream.frames.isEmpty && stopped { return nil }
    let frames = stream.frames
    stream.frames = []
    return frames
  }

  /// The comma is connected and frames came in the last `driving` seconds:
  /// nothing that restarts, builds, downloads or deletes may start now.
  var isDriving: Bool {
    flush()
    condition.lock()
    defer { condition.unlock() }
    guard case .link(let link)? = latest["link"]?.event, link.state == .connected else { return false }
    return clock() - lastStats < PageFeed.driving
  }

  /// Whether the hardware sampler runs: the one thing a test cannot see from
  /// outside without racing it.
  var isSampling: Bool {
    condition.lock()
    defer { condition.unlock() }
    return sampling
  }

  /// Tells the open pages the server is stopping and ends their streams.
  func stop() {
    flush()
    condition.lock()
    if !stopped, case .server(let server)? = latest["server"]?.event {
      let stopping = ServerEvent(state: "stopping", detail: "", backend: server.backend, runtimeVersion: server.runtimeVersion, device: server.device)
      let frame = PageFeed.sse(ControlEvent.server(stopping).jsonLine())
      latest["server"] = (.server(stopping), frame)
      push(frame)
    }
    stopped = true
    condition.broadcast()
    condition.unlock()
  }

  // MARK: hardware

  /// The sampler: from the first page opening until `limits.grace` after the
  /// last one closes, never while nobody is looking.
  private func sampleWhileWatched(_ hardware: any PageHardwareSource) {
    condition.lock()
    let known = hostFrame != nil
    condition.unlock()
    if !known {
      let frame = PageFeed.frame("host", hardware.host())
      condition.lock()
      hostFrame = frame
      push(frame)
      condition.unlock()
    }
    hardware.reset()
    while true {
      condition.lock()
      if stopped || streams.isEmpty && clock() - lastLeft >= limits.grace {
        // A page that opens later must not be shown this sample as current.
        sampling = false
        hwFrame = nil
        condition.unlock()
        return
      }
      condition.unlock()
      let frame = PageFeed.frame("hw", hardware.sample())
      condition.lock()
      hwFrame = frame
      push(frame)
      let next = Date(timeIntervalSinceNow: limits.period)
      while !stopped && Date() < next {
        _ = condition.wait(until: next)
      }
      condition.unlock()
    }
  }

  // MARK: encoding

  /// One server-sent event: `data: ` and the JSON line without its newline,
  /// then a blank line. JSON escapes every newline inside it.
  static func sse(_ line: Data) -> Data {
    var out = Data("data: ".utf8)
    out.append(line.last == 0x0A ? line.dropLast() : line)
    out.append(contentsOf: [0x0A, 0x0A])
    return out
  }

  /// A host-made event, keys sorted as `jsonLine()` sorts them.
  static func frame(_ name: String, _ fields: [String: Any], at date: Date = Date()) -> Data {
    sse(ControlJSON.line(event: name, fields, at: date) ?? Data(#"{"error":"the host's \#(name) fields are not JSON","event":"\#(name)"}"#.utf8))
  }
}

/// The page's threads: its own, at the lowest priority, so a page open
/// during a drive takes no CPU time the comma's frames want.
enum PageThread {
  static func start(_ name: String, _ body: @escaping @Sendable () -> Void) {
    let thread = Thread {
      lowerPriority()
      body()
    }
    thread.name = name
    thread.qualityOfService = .background
    thread.start()
  }

  /// Nice 19 for the calling thread alone: Linux keeps a nice value per
  /// thread, and `setpriority` with who 0 changes the caller's. Darwin
  /// has the thread's background QoS instead.
  static func lowerPriority() {
    #if os(Linux)
      _ = setpriority(__priority_which_t(PRIO_PROCESS.rawValue), 0, 19)
    #endif
  }
}
