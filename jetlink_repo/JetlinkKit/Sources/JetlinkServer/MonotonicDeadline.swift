import Foundation

/// A moment on the monotonic clock (`systemUptime`, CLOCK_MONOTONIC on
/// Linux), which a step of the wall clock does not move. A Jetson's clock
/// steps when timesyncd first syncs, and a USB transfer timed on `Date()`
/// then ended at once: the write's URB discarded, "peer went away", the
/// session dropped.
///
/// NSCondition waits only until a wall-clock `Date`, so a wait goes until
/// `wallClock()`, what is left read off this clock just before it, and the
/// caller checks `passed` again when it wakes: a forward step then only ends
/// a wait early. A backward step during a wait can still stretch it, until
/// the next broadcast.
struct MonotonicDeadline: Comparable, Sendable {
  /// Seconds on the monotonic clock.
  let uptime: TimeInterval

  /// `seconds` after `now`.
  init(in seconds: TimeInterval, now: TimeInterval = MonotonicDeadline.now()) {
    uptime = now + seconds
  }

  static func now() -> TimeInterval {
    ProcessInfo.processInfo.systemUptime
  }

  /// Seconds left, never negative.
  func remaining(now: TimeInterval = MonotonicDeadline.now()) -> TimeInterval {
    max(0, uptime - now)
  }

  func passed(now: TimeInterval = MonotonicDeadline.now()) -> Bool {
    now >= uptime
  }

  /// The wall-clock moment a wait for what is left ends at, `date` being the
  /// wall clock at `now`.
  func wallClock(now: TimeInterval = MonotonicDeadline.now(), date: Date = Date()) -> Date {
    date.addingTimeInterval(remaining(now: now))
  }

  static func < (a: MonotonicDeadline, b: MonotonicDeadline) -> Bool {
    a.uptime < b.uptime
  }
}
