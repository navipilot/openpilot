import Foundation

/// The host's sensors, read off the frame path: the server wraps
/// `ServerHooks.telemetry` in one, because a sensor read on the session thread
/// is sysfs or driver I/O that can stall a reply. Python's `CachedTelemetry`.
///
/// One worker thread reads `source` when a reader wants a sample, at most once
/// a `period`, and encodes it for a frame's reply there too. `read()` hands
/// back the latest sample, never waits for a new one, and gives `{}` once it
/// is older than `maxAge`, which the comma takes as "no health" rather than as
/// a cold idle board. The worker never holds the lock while it reads, so a
/// stuck sensor costs stale telemetry, never a frame.
public final class TelemetrySampler: @unchecked Sendable {
  public typealias Source = @Sendable () -> [String: Any]

  static let period: TimeInterval = 0.1
  static let maxAge: TimeInterval = 1.0
  static let empty = Data("{}".utf8)

  private let source: Source
  private let clock: @Sendable () -> TimeInterval
  private let condition = NSCondition()
  private var sample: [String: Any] = [:]
  private var encoded = TelemetrySampler.empty
  private var sampleTime = -TimeInterval.infinity
  private var nextRead: TimeInterval = 0
  /// Asked for at the start, so the first hello has a reading.
  private var requested = true
  private var closed = false

  public init(clock: @escaping @Sendable () -> TimeInterval = { ProcessInfo.processInfo.systemUptime }, source: @escaping Source) {
    self.clock = clock
    self.source = source
    let thread = Thread { [self] in run() }
    thread.name = "jetlink-telemetry"
    thread.start()
  }

  /// The latest sample, or `{}` when there is none fresh enough.
  public func read() -> [String: Any] {
    latest { $0 ? sample : [:] }
  }

  /// `read()` as the JSON a frame's reply carries.
  func json() -> Data {
    latest { $0 ? encoded : TelemetrySampler.empty }
  }

  /// `pick(fresh)`, asking the worker for the next sample once a period has passed.
  private func latest<T>(_ pick: (_ fresh: Bool) -> T) -> T {
    let now = clock()
    condition.lock()
    defer { condition.unlock() }
    if closed { return pick(false) }
    if now >= nextRead {
      requested = true
      condition.signal()
    }
    return pick(now - sampleTime <= TelemetrySampler.maxAge)
  }

  /// Stops sampling. The worker is not waited for: it may be stuck in a
  /// sensor's driver, and it ends on its own once that returns.
  public func close() {
    condition.lock()
    closed = true
    condition.signal()
    condition.unlock()
  }

  private func run() {
    while true {
      condition.lock()
      while !requested && !closed {
        condition.wait()
      }
      if closed {
        condition.unlock()
        return
      }
      requested = false
      // The age starts before the read, or a slow read would look fresh.
      let started = clock()
      nextRead = .infinity
      condition.unlock()
      let fresh = source()
      let bytes = JSONLine.encode(fresh)
      condition.lock()
      sample = fresh
      encoded = bytes
      sampleTime = started
      nextRead = clock() + TelemetrySampler.period
      condition.unlock()
    }
  }
}
