#if canImport(SwiftUI)
  import Charts
  import JetlinkKit
  import SwiftUI

  /// Where a frame's time goes, as Fitness draws heart rate zones: the average
  /// as one big number, then each stage on a row of its own, its time on the
  /// right and a capsule underneath measured against the 50 ms budget, so the
  /// room left shows in every row.
  ///
  /// The labels and values stay in text colours; the capsules carry the stage's
  /// colour, the same four the frame budget view uses.
  public struct LatencyBreakdown: View {
    let stats: StatsEvent
    /// Tighter, for a phone on its side.
    let compact: Bool

    public init(stats: StatsEvent, compact: Bool = false) {
      self.stats = stats
      self.compact = compact
    }

    public var body: some View {
      VStack(alignment: .leading, spacing: compact ? 12 : 18) {
        HStack(alignment: .firstTextBaseline, spacing: 4) {
          Text(stats.servedMs.mean.formatted(.number.precision(.fractionLength(1))))
            .font(.system(size: compact ? 32 : 40, weight: .bold, design: .rounded))
            .contentTransition(.numericText(value: stats.servedMs.mean))
          Text("ms avg")
            .font(.system(.title3, design: .rounded, weight: .semibold))
            .foregroundStyle(.secondary)
        }
        VStack(spacing: compact ? 8 : 14) {
          ForEach(FrameStage.allCases) { stage in
            row(stage, stage.value(stats.stagesMs))
          }
        }
      }
      .accessibilityElement(children: .ignore)
      .accessibilityLabel(accessibilityText)
    }

    private func row(_ stage: FrameStage, _ ms: Double) -> some View {
      VStack(spacing: 6) {
        HStack(alignment: .firstTextBaseline) {
          Text(stage.title)
            .font(.subheadline.weight(.semibold))
          Spacer()
          HStack(alignment: .firstTextBaseline, spacing: 2) {
            Text(ms.formatted(.number.precision(.fractionLength(1))))
              .font(.system(.subheadline, design: .rounded, weight: .semibold))
              .contentTransition(.numericText(value: ms))
            Text("ms")
              .font(.system(.caption, design: .rounded, weight: .semibold))
              .foregroundStyle(.secondary)
          }
          .monospacedDigit()
        }
        CapsuleBar(fraction: ms / FrameBudgetView.budgetMs, color: stage.color)
      }
    }

    private var accessibilityText: String {
      let parts = FrameStage.allCases.map { "\($0.title) \(FrameBudgetView.ms($0.value(stats.stagesMs)))" }
      return "Average \(FrameBudgetView.ms(stats.servedMs.mean)): " + parts.joined(separator: ", ")
    }
  }

  /// A capsule on a neutral track, filled to `fraction` of the width.
  /// Never thinner than it is tall, so a stage of a tenth of a millisecond still
  /// shows as a dot rather than vanishing.
  struct CapsuleBar: View {
    let fraction: Double
    let color: Color
    var height: CGFloat = 10

    var body: some View {
      GeometryReader { proxy in
        ZStack(alignment: .leading) {
          Capsule()
            .fill(Color.primary.opacity(0.08))
          Capsule()
            .fill(color.gradient)
            .frame(width: max(height, proxy.size.width * min(max(fraction, 0), 1)))
            .animation(.smooth(duration: 0.6), value: fraction)
        }
      }
      .frame(height: height)
    }
  }

  /// The last two minutes as Fitness draws a trend: a rounded bar for every five
  /// seconds, as tall as the slowest 1% of its frames and coloured by the room it
  /// left, under a thin line at the 50 ms budget.
  public struct LatencyHistoryChart: View {
    let history: [StatsSample]

    /// Seconds per bar: 24 bars over two minutes, wide enough to read on a phone.
    static let bucketSeconds = 5.0

    public init(history: [StatsSample]) {
      self.history = history
    }

    struct Bucket: Identifiable {
      let id: Int
      /// Seconds before now at the bucket's end, 0 for the newest.
      let age: Double
      let p99: Double
    }

    /// Five-second buckets, newest last; each the worst p99 of its seconds.
    var buckets: [Bucket] {
      guard let latest = history.last?.at else { return [] }
      var worst: [Int: Double] = [:]
      for sample in history {
        let age = latest.timeIntervalSince(sample.at)
        let index = Int(age / LatencyHistoryChart.bucketSeconds)
        worst[index] = max(worst[index] ?? 0, sample.stats.servedMs.p99)
      }
      return worst.keys.sorted(by: >).map { index in
        Bucket(id: index, age: Double(index) * LatencyHistoryChart.bucketSeconds, p99: worst[index]!)
      }
    }

    public var body: some View {
      let buckets = self.buckets
      let top = max(FrameBudgetView.budgetMs * 1.2, (buckets.map(\.p99).max() ?? 0) * 1.1)
      Chart {
        ForEach(buckets) { bucket in
          BarMark(
            x: .value("Seconds ago", -bucket.age),
            yStart: .value("Floor", 0),
            yEnd: .value("P99", min(bucket.p99, top)),
            width: .fixed(7)
          )
          .foregroundStyle(FrameBudgetView.Room(headroomMs: FrameBudgetView.budgetMs - bucket.p99).tone.color.gradient)
          .clipShape(Capsule())
        }
        // The budget, a shade stronger than the gridlines; the axis names it.
        RuleMark(y: .value("Budget", FrameBudgetView.budgetMs))
          .foregroundStyle(Color.secondary.opacity(0.7))
          .lineStyle(StrokeStyle(lineWidth: 1))
      }
      .chartXScale(domain: -Double(StatsSample.historyLength)...LatencyHistoryChart.bucketSeconds / 2)
      .chartYScale(domain: 0...top)
      .chartXAxis {
        AxisMarks(values: [-120.0, -60, 0]) { value in
          AxisValueLabel(anchor: value.as(Double.self) == 0 ? .topTrailing : value.as(Double.self) == -120 ? .topLeading : .top) {
            Text(LatencyHistoryChart.axisLabel(value.as(Double.self) ?? 0))
          }
        }
      }
      .chartYAxis {
        AxisMarks(position: .trailing, values: [0, 25, 50]) { value in
          AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5))
            .foregroundStyle(Color.secondary.opacity(0.25))
          AxisValueLabel {
            if let ms = value.as(Double.self) {
              Text(ms == FrameBudgetView.budgetMs ? "\(Int(ms)) ms" : "\(Int(ms))")
            }
          }
        }
      }
      .frame(height: 150)
      .accessibilityElement(children: .ignore)
      .accessibilityLabel(accessibilityText(buckets))
    }

    static func axisLabel(_ seconds: Double) -> String {
      switch Int(-seconds.rounded()) {
      case 0: "Now"
      case 60: "1 min"
      default: "2 min"
      }
    }

    private func accessibilityText(_ buckets: [Bucket]) -> String {
      let worst = buckets.map(\.p99).max() ?? 0
      let over = buckets.filter { $0.p99 > FrameBudgetView.budgetMs }.count
      return "Last two minutes: worst P99 \(FrameBudgetView.ms(worst)), \(over) of \(buckets.count) five-second spans over budget."
    }
  }
#endif
