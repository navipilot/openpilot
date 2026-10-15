import Foundation
import JetlinkKit

/// A rolling window of served frames, for the stats event once a second.
final class FrameStats: @unchecked Sendable {
  struct Sample {
    let at: TimeInterval
    let totalUs: UInt32
    let gpuUs: UInt32
    let queueUs: UInt32
    let sendUs: UInt32
  }

  /// A frame over this on the server counts as slow.
  static let slowUs: UInt32 = 60_000

  private let lock = NSLock()
  private var samples: [Sample] = []
  private let capacity = 2000
  /// The clock samples are stamped and windowed by; a test replays the
  /// Python's sample times on one of its own.
  private let now: @Sendable () -> TimeInterval

  init(now: @escaping @Sendable () -> TimeInterval = { ProcessInfo.processInfo.systemUptime }) {
    self.now = now
  }

  func record(totalUs: UInt32, gpuUs: UInt32, queueUs: UInt32, sendUs: UInt32) {
    let sample = Sample(at: now(), totalUs: totalUs, gpuUs: gpuUs, queueUs: queueUs, sendUs: sendUs)
    lock.lock()
    samples.append(sample)
    // Trimmed in chunks: one removeFirst a frame would shift the window every frame.
    if samples.count > capacity + capacity / 4 {
      samples.removeFirst(samples.count - capacity)
    }
    lock.unlock()
  }

  /// The `stats` event, or nil when no frame landed in the window.
  func summary(window seconds: TimeInterval, framesTotal: Int) -> StatsEvent? {
    let cutoff = now() - seconds
    lock.lock()
    let rows = samples.filter { $0.at >= cutoff }
    lock.unlock()
    guard !rows.isEmpty else { return nil }
    let n = Double(rows.count)
    func mean(_ value: (Sample) -> UInt32) -> Double {
      rows.reduce(0.0) { $0 + Double(value($1)) } / n / 1000
    }
    func spread(_ values: [UInt32]) -> StatsEvent.Total {
      let sorted = values.sorted()
      let meanValue = sorted.reduce(0.0) { $0 + Double($1) } / Double(sorted.count) / 1000
      let p99 = Double(sorted[Int(0.99 * Double(sorted.count - 1))]) / 1000
      return StatsEvent.Total(mean: round2(meanValue), p99: round2(p99), max: round2(Double(sorted.last!) / 1000))
    }
    let total = mean(\.totalUs)
    let gpu = mean(\.gpuUs)
    let queue = mean(\.queueUs)
    return StatsEvent(
      frames: framesTotal,
      fps: round2(n / seconds),
      servedMs: spread(rows.map { $0.totalUs + $0.sendUs }),
      stagesMs: StatsEvent.Stages(queue: round2(queue), gpu: round2(gpu), other: round2(max(0, total - gpu - queue)), send: round2(mean(\.sendUs))),
      slow: rows.filter { $0.totalUs > FrameStats.slowUs }.count,
      windowS: pythonRound(seconds, 1),
      totalMs: spread(rows.map(\.totalUs)),
      gpuMs: StatsEvent.Mean(mean: round2(gpu)))
  }
}

/// `round(value, 2)`, as the Python server rounds what it publishes.
func round2(_ value: Double) -> Double {
  pythonRound(value, 2)
}
