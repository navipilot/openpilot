import Foundation

/// The last lines this process logged: the status page's /logs (a page must
/// not start journalctl) and the Android app's Logs screen. The server's log
/// (`Log.write`) keeps every line it writes in `shared`, on every platform.
public final class LogRing: @unchecked Sendable {
  public static let shared = LogRing(capacity: 5000)

  public let capacity: Int
  private let lock = NSLock()
  private var times: [Date]
  private var texts: [String]
  private var next = 0
  private var count = 0
  /// Lines appended since the start, which numbers them.
  private var total = 0

  public init(capacity: Int) {
    precondition(capacity > 0)
    self.capacity = capacity
    times = Array(repeating: Date(timeIntervalSince1970: 0), count: capacity)
    texts = Array(repeating: "", count: capacity)
  }

  /// Keeps `line`, dropping the oldest once full. The slots are written in
  /// place: a line costs a lock and no allocation on the thread that logs.
  public func append(_ line: String, at date: Date = Date()) {
    lock.lock()
    times[next] = date
    texts[next] = line
    next = (next + 1) % capacity
    count = min(count + 1, capacity)
    total += 1
    lock.unlock()
  }

  /// The kept lines, oldest first, each after its local time.
  public func lines() -> [String] {
    lines(after: 0).lines
  }

  /// The kept lines numbered past `after`, and the number to ask after next time.
  public func lines(after: Int) -> (next: Int, lines: [String]) {
    lock.lock()
    let skip = min(max(after - (total - count), 0), count)
    let first = (next - count + capacity) % capacity
    let kept = (skip..<count).map { (times[(first + $0) % capacity], texts[(first + $0) % capacity]) }
    let end = total
    lock.unlock()
    return (end, kept.map { "\(logTimestamp($0, separator: ".")) \($1)" })
  }
}

/// "2026-09-28 12:53:24.982" in local time, with `separator` before the
/// milliseconds: Python's logging writes a comma.
public func logTimestamp(_ date: Date, separator: String) -> String {
  var seconds = time_t(date.timeIntervalSince1970.rounded(.down))
  var parts = tm()
  localtime_r(&seconds, &parts)
  let millis = min(999, Int((date.timeIntervalSince1970 - Double(seconds)) * 1000))
  let time = String(
    format: "%04d-%02d-%02d %02d:%02d:%02d", Int(parts.tm_year) + 1900, Int(parts.tm_mon) + 1, Int(parts.tm_mday), Int(parts.tm_hour),
    Int(parts.tm_min), Int(parts.tm_sec))
  return time + separator + String(format: "%03d", millis)
}
