import AppKit
import JetlinkKit
import JetlinkUI
import SwiftUI

/// The menu behind the menu bar icon: what is happening, and the things worth
/// doing without opening the window.
struct MenuBarView: View {
  @Environment(ServerStore.self) private var server
  @Environment(ModelStore.self) private var models
  @Environment(UpdateStore.self) private var updates
  @Environment(\.openWindow) private var openWindow

  var body: some View {
    Button(statusLine) {}
      .disabled(true)
    Button(modelLine) {}
      .disabled(true)
    Divider()
    switch server.runState {
    case .stopped, .failed:
      Button("Start Server") { server.start() }
    case .serving:
      Button("Stop Server") { server.stop() }
    case .starting, .stopping:
      Button("Start Server") {}
        .disabled(true)
    }
    Button("Open Jetlink") {
      openWindow(id: "main")
      NSApp.activate(ignoringOtherApps: true)
    }
    if updates.isAvailable {
      // Sparkle brings the app forward for a check the user asked for.
      Button(updates.heldUpdate.map { "Update to Jetlink \($0)…" } ?? "Check for Updates…") { updates.checkForUpdates() }
        .disabled(!updates.canCheckForUpdates)
    }
    Divider()
    Button("Quit Jetlink") { NSApp.terminate(nil) }
      .keyboardShortcut("q")
  }

  /// "Serving, waiting for comma", plus the frame rate once frames are flowing.
  private var statusLine: String {
    let (text, _) = StatusBadge.summary(runState: server.runState, link: server.link, engine: server.engine)
    if server.link.state == .connected, let stats = server.state.stats, stats.fps > 0 {
      let over = server.link.connectedMedium.map { " over \($0.title)" } ?? ""
      return "Comma connected\(over), \(stats.fps.formatted(.number.precision(.fractionLength(1)))) fps"
    }
    return text
  }

  private var modelLine: String {
    let engine = server.engine
    switch engine.state {
    case .none:
      return "No model in use"
    case .building, .loading:
      let name = engine.state == .building ? "Preparing model" : "Loading model"
      return "\(name), \(Int((engine.frac * 100).rounded()))%"
    case .ready:
      return "In use: \(loadedName)"
    case .failed:
      return "Model failed"
    }
  }

  private var loadedName: String {
    guard let sha = server.engine.sha256 else { return "unknown model" }
    return models.row(for: sha)?.displayName ?? String(sha.prefix(16))
  }
}
