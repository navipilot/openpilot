import Foundation

/// Keeps the CPU clocked up between the frames of a Neural Engine session.
///
/// Paced at 20 Hz on an M1 Pro, the Neural Engine build ran 39.3 ms mean and
/// 53.9 ms p99, against 30 ms back to back. Keeping one CPU core busy while
/// frames arrive made it 32.1 and 36.3, with no frame over 50 ms in 1,200
/// (2026-09-24). Keeping the Neural Engine busy with a small model changed
/// little, and so did keeping the CPU busy only around each frame's arrival
/// (49.6 ms p99) or one millisecond in two (52.7): the CPU side of the
/// prediction, CoreML's dispatch and the ops it leaves on the CPU, runs
/// slowly whenever the clocks have dropped, and they drop within
/// milliseconds.
///
/// So while frames arrive, a thread at the frame path's priority stays busy.
/// It does no useful work, touches nothing of the model's, and stops a second
/// after the last frame. It costs one CPU core's power while driving.
final class CPUKeepWarm: @unchecked Sendable {
  static let idleSeconds = 1.0
  private let condition = NSCondition()
  private var lastPulse = 0.0
  private var closed = false
  /// Where the spin's arithmetic lands, so the compiler keeps it.
  private var sink: UInt64 = 0

  init() {
    let thread = Thread { [weak self] in self?.loop() }
    thread.name = "jetlink-cpu-keepwarm"
    thread.qualityOfService = .userInteractive
    thread.start()
  }

  /// A frame is starting now.
  func pulse() {
    condition.lock()
    lastPulse = ProcessInfo.processInfo.systemUptime
    condition.signal()
    condition.unlock()
  }

  func close() {
    condition.lock()
    closed = true
    condition.signal()
    condition.unlock()
  }

  private var isClosed: Bool {
    condition.lock()
    defer { condition.unlock() }
    return closed
  }

  private func loop() {
    while true {
      condition.lock()
      while !closed && ProcessInfo.processInfo.systemUptime - lastPulse > CPUKeepWarm.idleSeconds {
        condition.wait()
      }
      if closed {
        condition.unlock()
        return
      }
      let last = lastPulse
      condition.unlock()
      // Busy until frames stop; look up now and then for new frames or a close.
      spin(until: min(last + CPUKeepWarm.idleSeconds, ProcessInfo.processInfo.systemUptime + 0.05))
    }
  }

  /// Busy until `end`, or a close.
  private func spin(until end: Double) {
    var x: UInt64 = 0x9E37_79B9_7F4A_7C15
    var n = 0
    while ProcessInfo.processInfo.systemUptime < end {
      // Some arithmetic the compiler cannot drop, then a look at the state.
      for _ in 0..<256 { x = x &* 6_364_136_223_846_793_005 &+ 1 }
      n += 1
      if n % 16 == 0 && isClosed { break }
    }
    condition.lock()
    sink ^= x
    condition.unlock()
  }
}
