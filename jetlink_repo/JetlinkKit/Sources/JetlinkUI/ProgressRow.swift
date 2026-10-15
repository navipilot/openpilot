#if canImport(SwiftUI)
  import JetlinkKit
  import SwiftUI

  /// The progress of a build or a load, with the server's own message underneath.
  public struct ProgressRow: View {
    let stage: String?
    let frac: Double
    let msg: String

    public init(stage: String?, frac: Double, msg: String) {
      self.stage = stage
      self.frac = frac
      self.msg = msg
    }

    public var body: some View {
      VStack(alignment: .leading, spacing: 4) {
        Text(ProgressRow.stageName(stage))
        if frac > 0 {
          ProgressView(value: min(max(frac, 0), 1))
        } else {
          ProgressView()
            .progressViewStyle(.linear)
        }
        if !msg.isEmpty {
          Text(msg)
            .font(.callout)
            .foregroundStyle(.secondary)
            .fixedSize(horizontal: false, vertical: true)
        }
      }
      .accessibilityElement(children: .combine)
    }

    /// The server's stage names in plain English.
    public nonisolated static func stageName(_ stage: String?) -> String {
      switch stage {
      case "upload": "Receiving"
      case "patch": "Preparing"
      case "parse": "Reading"
      case "convert": "Converting"
      case "compile": "Compiling"
      case "build": "Building"
      case "save": "Saving"
      case "load": "Loading"
      case "warm": "Warming Up"
      case "failed": "Failed"
      default: "Working"
      }
    }

    /// How far along: "42%", or for a step with nothing to go by the
    /// seconds so far, "12 s", from the server's "…, 12 s elapsed"
    /// (Ticker.paced). Nil before either.
    public nonisolated static func amount(frac: Double, msg: String) -> String? {
      if frac > 0 { return frac.formatted(.percent.precision(.fractionLength(0))) }
      guard let range = msg.range(of: #"\d+ s(?= elapsed$)"#, options: .regularExpression) else { return nil }
      return String(msg[range])
    }

    /// "Loading · 42%", "Loading · 12 s", or just "Loading".
    public nonisolated static func text(stage: String?, frac: Double, msg: String) -> String {
      [stageName(stage), amount(frac: frac, msg: msg)].compactMap { $0 }.joined(separator: " · ")
    }
  }

  #Preview {
    VStack(alignment: .leading, spacing: 16) {
      ProgressRow(stage: "convert", frac: 0.42, msg: "converting for CoreML, 412 MB of 766 MB written")
      ProgressRow(stage: "compile", frac: 0.68, msg: "compiling for CoreML, 1.4 GB of 2.1 GB written")
      ProgressRow(stage: "load", frac: 0, msg: "")
    }
    .frame(width: 360)
    .padding()
  }
#endif
