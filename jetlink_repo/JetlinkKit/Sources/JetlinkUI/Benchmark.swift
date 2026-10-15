#if canImport(SwiftUI)
  import Foundation
  import JetlinkKit
  import SwiftUI

  /// Fast enough, tight, or too slow: the benchmark's one-word answer, the same
  /// on a phone and on a Mac.
  public enum BenchmarkVerdict: Equatable, Sendable {
    case good, tight, slow
    /// Stopped before a frame was measured.
    case none

    /// A P99 at or under this, with no frame over the budget, leaves room for
    /// the cable in the 50 ms budget (`FrameBudgetView.budgetMs`).
    public static let roomyP99Ms = 35.0

    /// A run with no frames is too slow, unless it was stopped first: one
    /// frame outlasted the whole run.
    public init(_ report: BenchmarkReport) {
      if report.frames == 0 {
        self = report.cancelled ? .none : .slow
      } else {
        self.init(p99: report.frame.p99, over50: report.over50)
      }
    }

    public init(p99: Double, over50: Int = 0) {
      if p99 <= BenchmarkVerdict.roomyP99Ms && over50 == 0 {
        self = .good
      } else if p99 <= FrameBudgetView.budgetMs {
        self = .tight
      } else {
        self = .slow
      }
    }

    public var title: String {
      switch self {
      case .good: "Fast Enough"
      case .tight: "Tight"
      case .slow: "Too Slow"
      case .none: "No Frames"
      }
    }

    public var detail: String {
      switch self {
      case .good: "Room for the cable in the 50 ms budget."
      case .tight: "Little room left for the cable."
      case .slow: "Misses 20 frames a second."
      case .none: "Stopped before a frame was measured."
      }
    }

    public var symbol: String {
      switch self {
      case .good: "checkmark.seal.fill"
      case .tight: "exclamationmark.triangle.fill"
      case .slow: "xmark.octagon.fill"
      case .none: "stopwatch"
      }
    }

    public var tone: Color {
      switch self {
      case .good: .green
      case .tight: .orange
      case .slow: .red
      case .none: .secondary
      }
    }
  }

  /// How warm the machine is, from `ProcessInfo.thermalState` or the word a
  /// benchmark report carries.
  public enum ThermalLevel: Equatable, Sendable {
    case nominal, fair, serious, critical

    public init(_ state: ProcessInfo.ThermalState) {
      switch state {
      case .nominal: self = .nominal
      case .fair: self = .fair
      case .serious: self = .serious
      case .critical: self = .critical
      @unknown default: self = .fair
      }
    }

    /// From the word a benchmark report carries: nominal, fair, serious, critical.
    public init(label: String) {
      switch label {
      case "nominal": self = .nominal
      case "serious": self = .serious
      case "critical": self = .critical
      default: self = .fair
      }
    }

    public var title: String {
      switch self {
      case .nominal: "Normal"
      case .fair: "Warm"
      case .serious: "Hot"
      case .critical: "Critical"
      }
    }

    /// Said only when the heat costs frames.
    public var note: String? {
      switch self {
      case .nominal, .fair: nil
      case .serious, .critical: "Throttling"
      }
    }

    public var symbol: String {
      switch self {
      case .nominal: "thermometer.low"
      case .fair: "thermometer.medium"
      case .serious: "thermometer.high"
      case .critical: "flame.fill"
      }
    }

    public var tone: Color {
      switch self {
      case .nominal, .fair: .secondary
      case .serious: .orange
      case .critical: .red
      }
    }
  }

  /// Why a benchmark cannot start now, in a sentence; nil when it can. The
  /// server refuses the same things; this says so before the button is pressed.
  public enum BenchmarkBlocker {
    public static func reason(serving: Bool, modelLoaded: Bool, commaConnected: Bool) -> String? {
      if !serving { return "The server is not running." }
      if !modelLoaded { return "Load a model first." }
      if commaConnected { return "Disconnect the comma first." }
      return nil
    }
  }

  public enum BenchmarkClock {
    /// "1:00" for 60 seconds.
    public static func text(_ seconds: Double) -> String {
      let whole = Int(seconds.rounded(.down))
      return "\(whole / 60):\(String(format: "%02d", whole % 60))"
    }
  }

  /// A label over a number of milliseconds, the dashboards' unit of account.
  public struct Figure: View {
    let label: String
    let ms: Double?
    let tone: Color

    public init(_ label: String, ms: Double?, tone: Color = .primary) {
      self.label = label
      self.ms = ms
      self.tone = tone
    }

    public var body: some View {
      VStack(spacing: 2) {
        Text(label)
          .font(.footnote.weight(.medium))
          .foregroundStyle(.secondary)
        HStack(alignment: .firstTextBaseline, spacing: 2) {
          Text(ms.map { $0.formatted(.number.precision(.fractionLength(1))) } ?? "--")
            .font(.system(.title3, design: .rounded, weight: .semibold))
            .foregroundStyle(tone)
            .contentTransition(.numericText(value: ms ?? 0))
          Text("ms")
            .font(.system(.footnote, design: .rounded, weight: .semibold))
            .foregroundStyle(.secondary)
        }
      }
      .frame(maxWidth: .infinity)
      .accessibilityElement(children: .combine)
    }
  }

  /// The verdict and the numbers behind it: P99, max and mean of a frame.
  public struct BenchmarkVerdictSummary: View {
    let report: BenchmarkReport

    public init(report: BenchmarkReport) {
      self.report = report
    }

    public var body: some View {
      let verdict = BenchmarkVerdict(report)
      VStack(alignment: .leading, spacing: 14) {
        VStack(alignment: .leading, spacing: 4) {
          Text(verdict.title)
            .font(.system(.title, design: .rounded, weight: .bold))
            .foregroundStyle(verdict.tone)
          Text(BenchmarkVerdictSummary.detail(report, verdict))
            .font(.subheadline)
            .foregroundStyle(.secondary)
        }
        // no frames, no numbers: zeros would read as fast
        if report.frames > 0 {
          HStack(spacing: 0) {
            Figure("P99", ms: report.frame.p99, tone: verdict.tone)
            Divider().frame(height: 32)
            Figure("Max", ms: report.frame.max)
            Divider().frame(height: 32)
            Figure("Mean", ms: report.frame.mean)
          }
        }
      }
    }
  }

  extension BenchmarkVerdictSummary {
    /// The line under the verdict: what it means, or that no frame finished.
    public static func detail(_ report: BenchmarkReport, _ verdict: BenchmarkVerdict) -> String {
      report.frames == 0 && verdict == .slow ? "Not one frame finished in \(BenchmarkClock.text(report.seconds))." : verdict.detail
    }
  }

  /// The run ten seconds at a time, with the machine's temperature as each closed.
  public struct BenchmarkWindowRows: View {
    let windows: [BenchmarkWindow]

    public init(windows: [BenchmarkWindow]) {
      self.windows = windows
    }

    public var body: some View {
      VStack(spacing: 0) {
        ForEach(windows, id: \.startSecond) { window in
          let thermal = ThermalLevel(label: window.thermal)
          HStack {
            Text(BenchmarkClock.text(Double(window.startSecond)))
              .foregroundStyle(.secondary)
              .frame(width: 44, alignment: .leading)
            let verdict = BenchmarkVerdict(p99: window.frame.p99)
            Text("P99 \(FrameBudgetView.ms(window.frame.p99))")
              .foregroundStyle(verdict == .good ? .primary : verdict.tone)
            Spacer()
            Label(thermal.title, systemImage: thermal.symbol)
              .foregroundStyle(thermal.tone)
              .labelStyle(.titleAndIcon)
          }
          .font(.subheadline.monospacedDigit())
          .padding(.vertical, 6)
          if window.startSecond != windows.last?.startSecond {
            Divider()
          }
        }
      }
    }
  }
#endif
