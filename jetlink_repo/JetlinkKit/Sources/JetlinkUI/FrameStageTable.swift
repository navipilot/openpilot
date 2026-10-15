#if canImport(SwiftUI)
  import JetlinkKit
  import SwiftUI

  /// The frame's stages as a small table, two to a row: the key to the stacked
  /// bar, carrying every value so the bar never has to be read on its own. For a
  /// phone, where `FrameStageLegend`'s single row does not fit.
  public struct FrameStageTable: View {
    let stages: StatsEvent.Stages

    public init(stages: StatsEvent.Stages) {
      self.stages = stages
    }

    public var body: some View {
      Grid(alignment: .leading, horizontalSpacing: 10, verticalSpacing: 8) {
        ForEach(Array(stride(from: 0, to: FrameStage.allCases.count, by: 2)), id: \.self) { start in
          GridRow {
            ForEach(FrameStage.allCases[start..<min(start + 2, FrameStage.allCases.count)]) { stage in
              RoundedRectangle(cornerRadius: 2)
                .fill(stage.color)
                .frame(width: 10, height: 10)
              Text(stage.title)
                .foregroundStyle(.secondary)
              Text(FrameBudgetView.ms(stage.value(stages)))
                .monospacedDigit()
                .gridColumnAlignment(.trailing)
                .padding(.trailing, 12)
            }
          }
        }
      }
      .font(.subheadline)
      .accessibilityElement(children: .combine)
    }
  }
#endif
