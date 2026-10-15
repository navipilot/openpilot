#if canImport(SwiftUI)
  import Charts
  import JetlinkKit
  import SwiftUI

  /// Where a served frame's time goes, against the 50 ms the comma gives each
  /// frame at 20 Hz: the room left at p99 as the headline, the stages as one bar
  /// on the budget, and the last two minutes as a chart.
  public struct FrameBudgetView: View {
    let stats: StatsEvent
    let history: [StatsSample]

    public init(stats: StatsEvent, history: [StatsSample]) {
      self.stats = stats
      self.history = history
    }

    public nonisolated static let budgetMs = 50.0
    /// Less room than this at p99 reads as tight: the comma's own work and the
    /// transfer to the Mac come out of the same 50 ms, and are not measured here.
    public nonisolated static let tightMs = 10.0

    public var body: some View {
      VStack(alignment: .leading, spacing: 14) {
        headline
        FrameStageBar(stats: stats)
        FrameStageLegend(stages: stats.stagesMs)
        if history.count > 1 {
          Divider()
          FrameTimeChart(history: history)
        }
      }
      .padding(.vertical, 4)
    }

    private var headline: some View {
      let room = Room(headroomMs: FrameBudgetView.budgetMs - stats.servedMs.p99)
      return HStack(alignment: .firstTextBaseline) {
        VStack(alignment: .leading, spacing: 4) {
          Text(FrameBudgetView.headroomText(p99: stats.servedMs.p99))
            .font(.title2.weight(.semibold))
          Label {
            Text("\(room.title) · P99 of \(Int(FrameBudgetView.budgetMs)) ms")
              .foregroundStyle(.secondary)
          } icon: {
            Image(systemName: room.symbol)
              .foregroundStyle(room.tone.color)
          }
          .font(.callout)
        }
        Spacer()
        VStack(alignment: .trailing, spacing: 2) {
          Text("Avg \(FrameBudgetView.ms(stats.servedMs.mean))")
          Text("P99 \(FrameBudgetView.ms(stats.servedMs.p99)) · Max \(FrameBudgetView.ms(stats.servedMs.max))")
            .foregroundStyle(.secondary)
        }
        .font(.callout)
        .monospacedDigit()
      }
    }

    /// "18.4 ms headroom", or "3.2 ms over" once p99 is past the budget.
    public nonisolated static func headroomText(p99: Double) -> String {
      let room = budgetMs - p99
      return room >= 0 ? "\(ms(room)) headroom" : "\(ms(-room)) over"
    }

    public nonisolated static func ms(_ value: Double) -> String {
      "\(value.formatted(.number.precision(.fractionLength(1)))) ms"
    }

    public enum Room: Equatable, Sendable {
      case plenty, tight, over

      public init(headroomMs: Double) {
        if headroomMs < 0 {
          self = .over
        } else if headroomMs < FrameBudgetView.tightMs {
          self = .tight
        } else {
          self = .plenty
        }
      }

      public var title: String {
        switch self {
        case .plenty: "Good"
        case .tight: "Tight"
        case .over: "Over Budget"
        }
      }

      public var symbol: String {
        switch self {
        case .plenty: "checkmark.circle.fill"
        case .tight: "exclamationmark.triangle.fill"
        case .over: "xmark.octagon.fill"
        }
      }

      public var tone: StatusBadge.Tone {
        switch self {
        case .plenty: .good
        case .tight: .warning
        case .over: .bad
        }
      }
    }
  }

  /// The four places a frame's time goes, in the order they happen.
  public enum FrameStage: CaseIterable, Identifiable, Sendable {
    case inputs, model, overhead, reply

    public var id: Self { self }

    public var title: String {
      switch self {
      case .inputs: "Input"
      case .model: "Model"
      case .overhead: "Other"
      case .reply: "Send"
      }
    }

    public var detail: String {
      switch self {
      case .inputs: "Staging the frame for the model"
      case .model: "Running the model"
      case .overhead: "Everything else in the frame"
      case .reply: "Sending the result to the comma"
      }
    }

    public func value(_ stages: StatsEvent.Stages) -> Double {
      switch self {
      case .inputs: stages.queue
      case .model: stages.gpu
      case .overhead: stages.other
      case .reply: stages.send
      }
    }

    /// Categorical slots 2, 1, 3 and 4 of the chart palette, in stack order so
    /// neighbours stay apart under colour blindness, each with its own dark step.
    public var color: Color {
      switch self {
      case .inputs: Color(light: 0xEB6834, dark: 0xD95926)
      case .model: Color(light: 0x2A78D6, dark: 0x3987E5)
      case .overhead: Color(light: 0x1BAF7A, dark: 0x199E70)
      case .reply: Color(light: 0xEDA100, dark: 0xC98500)
      }
    }
  }

  /// One bar: the stages' means stacked from zero, over a track as long as the
  /// budget, with the budget and the p99 marked across it.
  public struct FrameStageBar: View {
    let stats: StatsEvent

    public init(stats: StatsEvent) {
      self.stats = stats
    }

    static let thickness: CGFloat = 16
    static let gap: CGFloat = 2
    static let labelBand: CGFloat = 16
    static let axisBand: CGFloat = 16

    public var body: some View {
      GeometryReader { proxy in
        let scale = FrameStageBar.domainMax(stats)
        let x = { (ms: Double) in CGFloat(ms / scale) * proxy.size.width }
        let top = FrameStageBar.labelBand
        ZStack(alignment: .topLeading) {
          RoundedRectangle(cornerRadius: 4)
            .fill(.quaternary)
            .frame(width: x(FrameBudgetView.budgetMs), height: FrameStageBar.thickness)
            .offset(y: top)
            .help("The \(Int(FrameBudgetView.budgetMs)) ms frame")

          ForEach(segments) { segment in
            UnevenRoundedRectangle(
              bottomTrailingRadius: segment.isLast ? 4 : 0,
              topTrailingRadius: segment.isLast ? 4 : 0
            )
            .fill(segment.stage.color)
            .frame(
              width: max(2, x(segment.end) - x(segment.start) - (segment.isLast ? 0 : FrameStageBar.gap)),
              height: FrameStageBar.thickness
            )
            .offset(x: x(segment.start), y: top)
            .help("\(segment.stage.title), \(FrameBudgetView.ms(segment.end - segment.start)): \(segment.stage.detail)")
          }

          marker(at: x(FrameBudgetView.budgetMs), width: 1, color: .primary.opacity(0.55))
          marker(at: x(stats.servedMs.p99), width: 2, color: .primary)
          Text("P99")
            .font(.caption)
            .foregroundStyle(.secondary)
            .fixedSize()
            .position(x: x(stats.servedMs.p99), y: top / 2 - 2)

          ForEach(FrameStageBar.ticks(scale), id: \.self) { tick in
            Text(tick == FrameBudgetView.budgetMs ? "\(Int(tick)) ms" : "\(Int(tick))")
              .font(.caption)
              .fontWeight(tick == FrameBudgetView.budgetMs ? .semibold : .regular)
              .foregroundStyle(tick == FrameBudgetView.budgetMs ? .primary : .secondary)
              .monospacedDigit()
              .fixedSize()
              .position(x: max(x(tick), 4), y: top + FrameStageBar.thickness + FrameStageBar.axisBand / 2 + 2)
          }
        }
      }
      .frame(height: FrameStageBar.labelBand + FrameStageBar.thickness + FrameStageBar.axisBand + 4)
      .accessibilityElement(children: .ignore)
      .accessibilityLabel(accessibilityText)
    }

    private func marker(at x: CGFloat, width: CGFloat, color: Color) -> some View {
      Rectangle()
        .fill(color)
        .frame(width: width, height: FrameStageBar.thickness + 8)
        .offset(x: x - width / 2, y: FrameStageBar.labelBand - 4)
    }

    struct Segment: Identifiable {
      let stage: FrameStage
      let start: Double
      let end: Double
      let isLast: Bool
      var id: FrameStage { stage }
    }

    /// The stages with any time in them, laid end to end.
    private var segments: [Segment] {
      let stages = stats.stagesMs
      let present = FrameStage.allCases.filter { $0.value(stages) > 0 }
      var segments: [Segment] = []
      var start = 0.0
      for (index, stage) in present.enumerated() {
        let end = start + stage.value(stages)
        segments.append(Segment(stage: stage, start: start, end: end, isLast: index == present.count - 1))
        start = end
      }
      return segments
    }

    /// The budget with room after it, or further when the frames run past it.
    static func domainMax(_ stats: StatsEvent) -> Double {
      max(FrameBudgetView.budgetMs * 1.1, stats.servedMs.p99 * 1.08, stats.servedMs.mean * 1.08)
    }

    /// Every 10 ms from zero, as far as the bar goes.
    static func ticks(_ domainMax: Double) -> [Double] {
      stride(from: 0.0, through: domainMax, by: 10).map { $0 }
    }

    private var accessibilityText: String {
      let parts = segments.map { "\($0.stage.title) \(FrameBudgetView.ms($0.end - $0.start))" }
      return "Frame time by stage: " + parts.joined(separator: ", ") + ". p99 \(FrameBudgetView.ms(stats.servedMs.p99)) of \(Int(FrameBudgetView.budgetMs)) ms."
    }
  }

  /// The key to the bar, carrying every value so nothing depends on hovering.
  public struct FrameStageLegend: View {
    let stages: StatsEvent.Stages

    public init(stages: StatsEvent.Stages) {
      self.stages = stages
    }

    public var body: some View {
      HStack(spacing: 18) {
        ForEach(FrameStage.allCases) { stage in
          HStack(spacing: 6) {
            RoundedRectangle(cornerRadius: 2)
              .fill(stage.color)
              .frame(width: 10, height: 10)
            Text(stage.title)
              .foregroundStyle(.secondary)
            Text(FrameBudgetView.ms(stage.value(stages)))
              .monospacedDigit()
          }
          .help(stage.detail)
        }
      }
      .font(.callout)
    }
  }

  /// The last two minutes, one point a second: the mean as a line, the spread up
  /// to p99 as a wash, the budget as a rule, and a dot for any second whose
  /// worst frame went past it. Hovering reads out a second.
  public struct FrameTimeChart: View {
    let history: [StatsSample]
    /// Off where the chart sits under a title of its own.
    let showsTitle: Bool
    @State private var selectedX: Double?

    public init(history: [StatsSample], showsTitle: Bool = true) {
      self.history = history
      self.showsTitle = showsTitle
    }

    public var body: some View {
      // Once a render: hovering re-renders the chart at the pointer's rate.
      let over = overBudget
      let top = yMax
      VStack(alignment: .leading, spacing: 8) {
        HStack(spacing: 14) {
          if showsTitle {
            Text("Last 2 Minutes")
              .font(.callout)
              .foregroundStyle(.secondary)
            Spacer()
          }
          legendItem("Avg") {
            Capsule().fill(Color.primary).frame(width: 14, height: 2)
          }
          legendItem("P99") {
            RoundedRectangle(cornerRadius: 2).fill(Color.primary.opacity(0.12)).frame(width: 14, height: 10)
          }
          if !over.isEmpty {
            legendItem("Over Budget") {
              Circle().fill(Color.red).frame(width: 8, height: 8)
            }
          }
        }
        .font(.caption)
        chart(yMax: top, overBudget: over)
          .frame(height: 150)
      }
    }

    private func legendItem(_ title: String, @ViewBuilder swatch: () -> some View) -> some View {
      HStack(spacing: 5) {
        swatch()
        Text(title)
          .foregroundStyle(.secondary)
      }
    }

    private func chart(yMax: Double, overBudget: [StatsSample]) -> some View {
      // A spike past the top is drawn at the top; the readout has its real value.
      let clamped = { (ms: Double) in min(ms, yMax) }
      let length = Double(StatsSample.historyLength)
      return Chart {
        ForEach(history) { sample in
          AreaMark(
            x: .value("Seconds", seconds(sample)),
            yStart: .value("Mean", clamped(sample.stats.servedMs.mean)),
            yEnd: .value("p99", clamped(sample.stats.servedMs.p99))
          )
          .foregroundStyle(Color.primary.opacity(0.12))
        }
        ForEach(history) { sample in
          LineMark(x: .value("Seconds", seconds(sample)), y: .value("Mean", clamped(sample.stats.servedMs.mean)))
            .foregroundStyle(Color.primary)
            .lineStyle(StrokeStyle(lineWidth: 2, lineCap: .round, lineJoin: .round))
        }
        RuleMark(y: .value("Budget", FrameBudgetView.budgetMs))
          .foregroundStyle(Color.red.opacity(0.75))
          .lineStyle(StrokeStyle(lineWidth: 1))
          .annotation(position: .top, alignment: .leading, spacing: 2) {
            Text("\(Int(FrameBudgetView.budgetMs)) ms budget")
              .font(.caption)
              .foregroundStyle(.secondary)
          }
        ForEach(overBudget) { sample in
          PointMark(x: .value("Seconds", seconds(sample)), y: .value("Worst frame", clamped(sample.stats.servedMs.max)))
            .foregroundStyle(Color.red)
            .symbolSize(50)
        }
        if let selected {
          RuleMark(x: .value("Seconds", seconds(selected)))
            .foregroundStyle(Color.secondary.opacity(0.6))
            .lineStyle(StrokeStyle(lineWidth: 1))
            .annotation(position: .top, spacing: 0, overflowResolution: .init(x: .fit(to: .chart), y: .disabled)) {
              readout(selected)
            }
        }
      }
      .chartXScale(domain: -length...0)
      .chartYScale(domain: 0...yMax)
      .chartXAxis {
        AxisMarks(values: Array(stride(from: -length, through: 0, by: 30))) { value in
          let seconds = value.as(Double.self) ?? 0
          AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5))
          // A centred label at either end would hang off the chart and be dropped.
          AxisValueLabel(anchor: seconds == 0 ? .topTrailing : seconds <= -length ? .topLeading : .top) {
            Text(FrameTimeChart.axisLabel(seconds))
          }
        }
      }
      .chartYAxis {
        AxisMarks(position: .leading, values: .stride(by: 10)) { value in
          AxisGridLine(stroke: StrokeStyle(lineWidth: 0.5))
          AxisValueLabel {
            if let ms = value.as(Double.self) {
              Text("\(Int(ms))")
                .monospacedDigit()
            }
          }
        }
      }
      .chartXSelection(value: $selectedX)
    }

    private func readout(_ sample: StatsSample) -> some View {
      let served = sample.stats.servedMs
      return VStack(alignment: .leading, spacing: 2) {
        Text(FrameTimeChart.agoText(-seconds(sample)))
          .foregroundStyle(.secondary)
        Text("\(FrameBudgetView.ms(served.mean)) mean")
        Text("\(FrameBudgetView.ms(served.p99)) p99, \(FrameBudgetView.ms(served.max)) max")
      }
      .font(.caption)
      .monospacedDigit()
      .padding(6)
      .background(RoundedRectangle(cornerRadius: 6).fill(.background).shadow(color: .black.opacity(0.2), radius: 3, y: 1))
    }

    private var latest: Date { history.last?.at ?? Date() }

    private func seconds(_ sample: StatsSample) -> Double {
      sample.at.timeIntervalSince(latest)
    }

    private var overBudget: [StatsSample] {
      history.filter { $0.stats.servedMs.max > FrameBudgetView.budgetMs }
    }

    private var selected: StatsSample? {
      guard let selectedX else { return nil }
      return history.min { abs(seconds($0) - selectedX) < abs(seconds($1) - selectedX) }
    }

    /// Room above the budget, and above the highest p99 when that is higher.
    private var yMax: Double {
      let top = history.map(\.stats.servedMs.p99).max() ?? 0
      return max(FrameBudgetView.budgetMs * 1.2, (top * 1.1 / 10).rounded(.up) * 10)
    }

    static func axisLabel(_ seconds: Double) -> String {
      let ago = Int(-seconds.rounded())
      switch ago {
      case 0: return "now"
      case StatsSample.historyLength: return "\(ago / 60) min"
      default: return "\(ago) s"
      }
    }

    static func agoText(_ seconds: Double) -> String {
      let ago = Int(seconds.rounded())
      return ago == 0 ? "Just now" : "\(ago) s ago"
    }
  }

  #Preview("Good") {
    Form {
      Section("Frame Budget") {
        FrameBudgetView(stats: PreviewData.stats, history: PreviewData.statsHistory)
      }
    }
    .formStyle(.grouped)
    .frame(width: 680, height: 460)
  }

  #Preview("Over budget") {
    let stats = StatsEvent(
      frames: 900, fps: 18.2, servedMs: StatsEvent.Total(mean: 48.5, p99: 59.1, max: 72.4),
      stagesMs: StatsEvent.Stages(queue: 0.7, gpu: 43.7, other: 2.7, send: 1.4), slow: 2, windowS: 1)
    return Form {
      Section("Frame Budget") {
        FrameBudgetView(stats: stats, history: [])
      }
    }
    .formStyle(.grouped)
    .frame(width: 680, height: 260)
  }
#endif
