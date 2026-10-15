#if canImport(SwiftUI)
  import JetlinkKit
  import SwiftUI

  /// The frame budget as a gauge: a 270° track as long as the 50 ms the comma
  /// gives each frame, filled to the p99 frame time, with the room left as the
  /// one big number inside. Made to be read at a glance from a phone in a car
  /// mount, so it carries one number, one state and nothing else.
  ///
  /// The fill carries the state (the same three the frame budget view uses);
  /// the track is a lighter step of the same colour, so the state reads across
  /// the whole ring, and the state is also spelled out underneath with its icon.
  public struct HeadroomRing: View {
    let p99: Double
    let lineWidth: CGFloat

    public init(p99: Double, lineWidth: CGFloat = 22) {
      self.p99 = p99
      self.lineWidth = lineWidth
    }

    /// The ring's share of a circle; the gap sits at the bottom.
    static let sweep = 0.75

    public var body: some View {
      let room = FrameBudgetView.Room(headroomMs: FrameBudgetView.budgetMs - p99)
      let tint = room.tone.color
      let fraction = min(max(p99 / FrameBudgetView.budgetMs, 0), 1)
      ZStack {
        arc(to: HeadroomRing.sweep)
          .stroke(tint.opacity(0.22), style: StrokeStyle(lineWidth: lineWidth, lineCap: .round))
        // A shade deeper where the fill starts, as the Activity rings draw theirs.
        arc(to: HeadroomRing.sweep * max(fraction, 0.001))
          .stroke(
            AngularGradient(
              colors: [tint.mix(with: .black, by: 0.15), tint], center: .center,
              startAngle: .degrees(135), endAngle: .degrees(135 + 360 * HeadroomRing.sweep * max(fraction, 0.001))),
            style: StrokeStyle(lineWidth: lineWidth, lineCap: .round)
          )
          .animation(.smooth(duration: 0.6), value: fraction)
        center(room: room)
        Text("\(Int(FrameBudgetView.budgetMs)) ms")
          .font(.footnote.weight(.semibold))
          .foregroundStyle(.secondary)
          .frame(maxHeight: .infinity, alignment: .bottom)
      }
      .padding(lineWidth / 2)
      .aspectRatio(1, contentMode: .fit)
      .accessibilityElement(children: .ignore)
      .accessibilityLabel(accessibilityText(room))
    }

    private func arc(to fraction: Double) -> some Shape {
      Circle()
        .trim(from: 0, to: fraction)
        .rotation(.degrees(135))
    }

    private func center(room: FrameBudgetView.Room) -> some View {
      let headroom = FrameBudgetView.budgetMs - p99
      return VStack(spacing: 4) {
        HStack(alignment: .firstTextBaseline, spacing: 4) {
          Text(abs(headroom).formatted(.number.precision(.fractionLength(1))))
            .font(.system(size: 64, weight: .bold, design: .rounded))
            .contentTransition(.numericText(value: headroom))
            .animation(.smooth, value: headroom)
          Text(headroom >= 0 ? "ms" : "ms over")
            .font(.system(.title3, design: .rounded, weight: .semibold))
            .foregroundStyle(.secondary)
        }
        .minimumScaleFactor(0.5)
        .lineLimit(1)
        Label(room.title, systemImage: room.symbol)
          .font(.headline)
          .foregroundStyle(room.tone.color)
      }
      .padding(.horizontal, lineWidth * 1.5)
    }

    private func accessibilityText(_ room: FrameBudgetView.Room) -> String {
      "\(FrameBudgetView.headroomText(p99: p99)) of \(Int(FrameBudgetView.budgetMs)) milliseconds. \(room.title)."
    }
  }

  #Preview("Good") {
    HeadroomRing(p99: 31.6)
      .frame(width: 280)
      .padding()
  }

  #Preview("Tight and over") {
    HStack {
      HeadroomRing(p99: 44.2)
      HeadroomRing(p99: 58.3)
    }
    .padding()
  }
#endif
