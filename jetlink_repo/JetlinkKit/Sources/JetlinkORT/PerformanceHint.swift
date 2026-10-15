import Foundation

#if os(Android)
  import Android

  /// Android's performance hints (ADPF) for the frame path: the thread that
  /// runs the model tells the system each frame's time, and the system holds
  /// that thread's CPU clocks up to meet the target instead of letting them
  /// fall between frames. What CPUKeepWarm does on a Mac, without a busy
  /// core.
  ///
  /// The target is under any model's run, so the hint only ever asks for
  /// speed, never for the slowest clock that meets a deadline. The API is
  /// Android 13's (API 33), found at run time; `make()` is nil below that or
  /// where the device's power HAL has no hint sessions. Not yet measured on
  /// a phone.
  final class PerformanceHint: @unchecked Sendable {
    static let targetNanoseconds: Int64 = 10_000_000

    private typealias GetManager = @convention(c) () -> OpaquePointer?
    private typealias PreferredRate = @convention(c) (OpaquePointer?) -> Int64
    private typealias CreateSession = @convention(c) (OpaquePointer?, UnsafePointer<Int32>?, Int, Int64) -> OpaquePointer?
    private typealias ReportWork = @convention(c) (OpaquePointer?, Int64) -> Int32
    private typealias CloseSession = @convention(c) (OpaquePointer?) -> Void

    private let manager: OpaquePointer
    private let create: CreateSession
    private let reportWork: ReportWork
    private let closeSession: CloseSession
    private let lock = NSLock()
    private var session: OpaquePointer?
    private var thread: Int32 = 0

    static func make() -> PerformanceHint? {
      guard let library = dlopen("libandroid.so", RTLD_NOW),
        let getManager = dlsym(library, "APerformanceHint_getManager"),
        let preferredRate = dlsym(library, "APerformanceHint_getPreferredUpdateRateNanos"),
        let create = dlsym(library, "APerformanceHint_createSession"),
        let report = dlsym(library, "APerformanceHint_reportActualWorkDuration"),
        let close = dlsym(library, "APerformanceHint_closeSession"),
        let manager = unsafeBitCast(getManager, to: GetManager.self)(),
        // -1 where the power HAL has no hint sessions
        unsafeBitCast(preferredRate, to: PreferredRate.self)(manager) > 0
      else { return nil }
      return PerformanceHint(
        manager: manager, create: unsafeBitCast(create, to: CreateSession.self),
        reportWork: unsafeBitCast(report, to: ReportWork.self), closeSession: unsafeBitCast(close, to: CloseSession.self))
    }

    private init(manager: OpaquePointer, create: CreateSession, reportWork: ReportWork, closeSession: CloseSession) {
      self.manager = manager
      self.create = create
      self.reportWork = reportWork
      self.closeSession = closeSession
    }

    /// The frame just run on the calling thread took `nanoseconds`. The
    /// session follows the thread that reports.
    func report(_ nanoseconds: UInt64) {
      let tid = gettid()
      lock.lock()
      defer { lock.unlock() }
      if session == nil || thread != tid {
        if let session { closeSession(session) }
        var id = tid
        session = create(manager, &id, 1, PerformanceHint.targetNanoseconds)
        thread = tid
      }
      if let session {
        _ = reportWork(session, Int64(clamping: nanoseconds))
      }
    }

    func close() {
      lock.lock()
      defer { lock.unlock() }
      if let session { closeSession(session) }
      session = nil
    }
  }
#else
  /// Android's performance hints; elsewhere CPUKeepWarm does their job.
  final class PerformanceHint: Sendable {
    static func make() -> PerformanceHint? { nil }
    func report(_ nanoseconds: UInt64) {}
    func close() {}
  }
#endif
