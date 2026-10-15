import AppKit
import JetlinkKit
import JetlinkUI
import SwiftUI

/// What the server, the comma and the engine are doing right now.
struct StatusView: View {
  @Environment(ServerStore.self) private var server
  @Environment(ModelStore.self) private var models
  @Environment(AppSettings.self) private var settings
  @Environment(LogBuffer.self) private var logs
  @Binding var selection: SidebarItem?
  @State private var confirmingUnload = false

  var body: some View {
    Form {
      serverSection
      commaSection
      frameBudgetSection
      engineSection
    }
    .formStyle(.grouped)
    .confirmationDialog("Stop using the model?", isPresented: $confirmingUnload) {
      Button("Stop Using Model") { models.unload() }
      Button("Cancel", role: .cancel) {}
    } message: {
      Text("The comma drives on its small model until a model is in use again.")
    }
  }

  // MARK: Server

  @ViewBuilder
  private var serverSection: some View {
    Section("Server") {
      LabeledContent("State") {
        StatusBadge(text: serverStateText, tone: serverStateTone, showsProgress: isBusy)
      }
      LabeledContent("Backend") {
        VStack(alignment: .trailing, spacing: 2) {
          Text((server.info?.choice ?? settings.backend).title)
          if let info = server.info {
            Text(StatusView.runtimeLine(version: info.runtimeVersion, device: info.device))
              .font(.callout)
              .foregroundStyle(.secondary)
          }
        }
      }
      if let startedAt = server.startedAt, server.runState == .serving {
        LabeledContent("Uptime") {
          TimelineView(.periodic(from: .now, by: 30)) { context in
            Text(StatusView.uptimeText(from: startedAt, to: context.date))
          }
        }
      }
      if case let .failed(reason) = server.runState {
        VStack(alignment: .leading, spacing: 8) {
          Text(failureDetail(reason))
            .font(.system(size: 12, design: .monospaced))
            .foregroundStyle(.red)
            .textSelection(.enabled)
            .fixedSize(horizontal: false, vertical: true)
          if !lastLogLines.isEmpty {
            VStack(alignment: .leading, spacing: 1) {
              ForEach(Array(lastLogLines.enumerated()), id: \.offset) { _, line in
                Text(line)
                  .font(.system(size: 12, design: .monospaced))
                  .textSelection(.enabled)
                  .frame(maxWidth: .infinity, alignment: .leading)
              }
            }
          }
          Button("Show Logs") { selection = .logs }
        }
      }
    }
  }

  private var serverStateText: String {
    switch server.runState {
    case .stopped: "Stopped"
    case .starting: "Starting…"
    case .serving: "Serving"
    case .stopping: "Stopping…"
    case .failed: "Failed"
    }
  }

  private var serverStateTone: StatusBadge.Tone {
    switch server.runState {
    case .stopped: .neutral
    case .starting: .info
    case .serving: .good
    case .stopping: .neutral
    case .failed: .bad
    }
  }

  private var isBusy: Bool {
    server.runState == .starting || server.runState == .stopping
  }

  // MARK: Comma

  @ViewBuilder
  private var commaSection: some View {
    Section {
      LabeledContent("Link") {
        VStack(alignment: .trailing, spacing: 2) {
          StatusBadge(text: linkText, tone: linkTone)
          if showsLinkDetail {
            Text(server.link.detail)
              .font(.callout)
              .foregroundStyle(.secondary)
          }
          if let advice = server.link.connectedMedium?.advice {
            Text(advice)
              .font(.callout)
              .foregroundStyle(.orange)
          }
        }
      }
      if server.link.state == .connected, let stats = server.state.stats {
        LabeledContent("Frames", value: stats.frames.formatted(.number.grouping(.automatic)))
        LabeledContent("Rate", value: "\(stats.fps.formatted(.number.precision(.fractionLength(1)))) per second")
        LabeledContent("Slow frames") {
          Text(stats.slow.formatted())
            .foregroundStyle(stats.slow > 0 ? .red : .primary)
        }
        .help("Frames over 60 ms in the last second")
      }
    } header: {
      Text("Comma")
    } footer: {
      Text("Connect to the comma with a USB 3 cable. Until it connects, the comma drives on its small model.")
        .font(.callout)
        .foregroundStyle(.secondary)
    }
  }

  // MARK: Frame budget

  @ViewBuilder
  private var frameBudgetSection: some View {
    if server.link.state == .connected, let stats = server.state.stats {
      Section {
        FrameBudgetView(stats: stats, history: server.state.statsHistory)
      } header: {
        Text("Frame Budget")
      } footer: {
        Text(
          "Measured on this Mac, from a frame's arrival to its reply leaving. The comma's own work and the transfer to the Mac use the same 50 ms, so leave room."
        )
        .font(.callout)
        .foregroundStyle(.secondary)
      }
    }
  }

  private var linkText: String {
    switch server.link.state {
    case .waiting:
      return "Waiting for comma"
    case .connected:
      return "Connected over \((server.link.connectedMedium ?? .usb).title)"
    case .disconnected:
      return "Disconnected"
    }
  }

  /// The disconnect reason. The waiting line ("waiting for a jetlink gadget",
  /// or a TCP listening address) only repeats the badge.
  private var showsLinkDetail: Bool {
    server.link.state == .disconnected && !server.link.detail.isEmpty
  }

  private var linkTone: StatusBadge.Tone {
    switch server.link.state {
    case .waiting: .neutral
    case .connected: server.link.connectedMedium?.isSlow == true ? .warning : .good
    case .disconnected: .warning
    }
  }

  // MARK: Engine

  @ViewBuilder
  private var engineSection: some View {
    Section("Model") {
      if showsEmptyState {
        VStack(alignment: .leading, spacing: 8) {
          Label("No model in use", systemImage: "shippingbox")
            .font(.headline)
          Text(
            "Use the model your comma drives with, and leave Jetlink running. Otherwise the comma sends its model when it connects, and drives on its small model until the Mac is ready."
          )
          .font(.callout)
          .foregroundStyle(.secondary)
          .fixedSize(horizontal: false, vertical: true)
          emptyStateAction
        }
        .frame(maxWidth: .infinity, alignment: .leading)
      } else {
        LabeledContent("Model") {
          VStack(alignment: .trailing, spacing: 2) {
            Text(loadedModelName)
            if let sha = server.engine.sha256 {
              Text(String(sha.prefix(16)))
                .font(.system(.callout, design: .monospaced))
                .foregroundStyle(.secondary)
            }
          }
        }
        LabeledContent("State") {
          StatusBadge(text: engineStateText, tone: engineStateTone)
        }
        if server.engine.state == .building || server.engine.state == .loading {
          ProgressRow(stage: server.engine.stage, frac: server.engine.frac, msg: server.engine.msg)
        }
        if server.engine.state == .failed, !server.engine.detail.isEmpty {
          Text(server.engine.detail)
            .foregroundStyle(.red)
            .textSelection(.enabled)
            .fixedSize(horizontal: false, vertical: true)
        }
        HStack {
          Spacer()
          Button("Show Cache in Finder") {
            NSWorkspace.shared.activateFileViewerSelecting([cacheURL])
          }
          if server.engine.state == .ready {
            Button("Stop Using Model") { confirmingUnload = true }
          }
        }
      }
    }
  }

  /// One click for the default model, which is what a comma nobody changed
  /// asks for, and the list for anyone who did change it.
  @ViewBuilder
  private var emptyStateAction: some View {
    if let row = defaultRow, case .downloading = row.status {
      ModelStatusLabel(row.status)
        .frame(maxWidth: 260, alignment: .leading)
    } else if let row = defaultRow, ModelStore.canUse(row) {
      HStack {
        Button("Use \(row.displayName)") { models.use(row) }
          .buttonStyle(.borderedProminent)
          .help(ModelFormatting.useHelp(row, device: "this Mac"))
        Button("Choose Another Model…") { selection = .models }
      }
    } else {
      Button("Open Models") { selection = .models }
    }
  }

  private var defaultRow: ModelRow? {
    models.rows.first { $0.isDefault }
  }

  private var showsEmptyState: Bool {
    server.engine.state == .none && (models.inventory?.artifacts.isEmpty ?? true)
  }

  private var loadedModelName: String {
    guard let sha = server.engine.sha256 else { return "None" }
    return models.row(for: sha)?.displayName ?? "Unknown model"
  }

  private var engineStateText: String {
    switch server.engine.state {
    case .none: "Not in use"
    case .building: "Preparing"
    case .loading: "Loading"
    case .ready: "In use"
    case .failed: "Failed"
    }
  }

  private var engineStateTone: StatusBadge.Tone {
    switch server.engine.state {
    case .none: .neutral
    case .building, .loading: .info
    case .ready: .good
    case .failed: .bad
    }
  }

  // MARK: Failures

  /// What went wrong, in the server's own words.
  private func failureDetail(_ reason: String) -> String {
    let failure = server.lastFailure ?? reason
    return failure.isEmpty ? reason : failure
  }

  private var lastLogLines: [String] {
    Array(logs.lines.suffix(20))
  }

  // MARK: Helpers

  private var cacheURL: URL {
    if let cache = server.info?.cache, !cache.isEmpty {
      return URL(fileURLWithPath: cache)
    }
    return settings.cacheDirectory
  }

  /// "onnxruntime 1.29.0, Apple M1 Pro": what is actually running. The
  /// device loses the backend prefix it repeats.
  static func runtimeLine(version: String, device: String) -> String {
    let version = version.split(separator: "+", maxSplits: 1).first.map(String.init) ?? version
    var hardware = device
    if let dash = device.firstIndex(of: "-") {
      hardware = String(device[device.index(after: dash)...])
    }
    hardware = hardware.replacingOccurrences(of: "_", with: " ")
    let head = version.isEmpty ? "onnxruntime" : "onnxruntime \(version)"
    return hardware.isEmpty ? head : "\(head), \(hardware)"
  }

  static func uptimeText(from start: Date, to now: Date) -> String {
    let seconds = max(0, Int(now.timeIntervalSince(start)))
    if seconds < 60 {
      return "Less than a minute"
    }
    return Duration.seconds(seconds).formatted(.units(allowed: [.hours, .minutes], width: .wide))
  }
}

#Preview("Connected") {
  @Previewable @State var selection: SidebarItem? = .status
  StatusView(selection: $selection)
    .environment(
      ServerStore.preview(
        runState: .serving, info: PreviewData.serverInfo, link: PreviewData.linkConnected,
        engine: PreviewData.engineReady, statsHistory: PreviewData.statsHistory)
    )
    .environment(ModelStore.preview(catalog: PreviewData.catalog, inventory: PreviewData.inventory, engine: PreviewData.engineReady))
    .environment(AppSettings.preview())
    .environment(LogBuffer.preview(lines: PreviewData.logLines))
    .frame(width: 680, height: 980)
}

#Preview("Building") {
  @Previewable @State var selection: SidebarItem? = .status
  StatusView(selection: $selection)
    .environment(
      ServerStore.preview(
        runState: .serving, info: PreviewData.serverInfo, link: PreviewData.linkWaiting,
        engine: PreviewData.engineBuilding)
    )
    .environment(ModelStore.preview(catalog: PreviewData.catalog, inventory: PreviewData.inventory, engine: PreviewData.engineBuilding))
    .environment(AppSettings.preview())
    .environment(LogBuffer.preview(lines: PreviewData.logLines))
    .frame(width: 640, height: 620)
}

#Preview("Nothing prepared") {
  @Previewable @State var selection: SidebarItem? = .status
  StatusView(selection: $selection)
    .environment(ServerStore.preview(runState: .stopped, info: nil, link: PreviewData.linkWaiting, engine: PreviewData.engineNone))
    .environment(ModelStore.preview(catalog: nil, inventory: PreviewData.emptyInventory, engine: PreviewData.engineNone))
    .environment(AppSettings.preview())
    .environment(LogBuffer.preview(lines: PreviewData.logLines))
    .frame(width: 640, height: 620)
}
