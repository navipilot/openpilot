import JetlinkKit
import JetlinkUI
import SwiftUI
import UIKit

/// The Status tab: the headroom and everything behind it, under a large title
/// whose subtitle says where things stand.
struct StatusScreen: View {
  @Environment(AppModel.self) private var app
  @Environment(\.verticalSizeClass) private var verticalSizeClass
  @Environment(\.horizontalSizeClass) private var horizontalSizeClass
  @Binding var tab: RootView.Tab

  var body: some View {
    let state = app.status
    let arrangement = self.arrangement
    let landscape = arrangement == .sideways
    NavigationStack {
      StatusContent(state: state, arrangement: arrangement, actions: actions)
        .navigationTitle("Jetlink")
        .navigationSubtitle(state.subtitle)
        .toolbarTitleDisplayMode(landscape ? .inline : .large)
        .toolbar {
          ToolbarItem(placement: .topBarTrailing) {
            NavigationLink {
              LogsScreen()
            } label: {
              Label("Logs", systemImage: "doc.text")
            }
          }
        }
    }
    // On its side an iPhone is a dashboard: nothing but the numbers. An
    // iPad keeps its tabs in any shape of window.
    .toolbarVisibility(landscape && !ThisDevice.isPad ? .hidden : .automatic, for: .tabBar)
    .sensoryFeedback(trigger: state.link.state) { old, new in
      if new == .connected { return .success }
      if old == .connected { return .warning }
      return nil
    }
  }

  /// A short screen sets the cards side by side, as an iPhone on its side
  /// has it; a wide one in two columns, as an iPad has it; anything else,
  /// an iPhone upright or an iPad in a narrow window, in one.
  private var arrangement: StatusContent.Arrangement {
    if verticalSizeClass == .compact { return .sideways }
    return horizontalSizeClass == .regular ? .columns : .column
  }

  private var actions: StatusActions {
    StatusActions(
      useDefault: useDefault,
      openModels: { tab = .models },
      retry: retry,
      openSettings: openSettings,
      refreshCatalog: { app.models.refreshCatalog() })
  }

  private func useDefault() {
    guard let row = app.models.rows.first(where: \.isDefault) else { return }
    app.models.use(row, confirmedInterruption: true)
  }

  /// A stopped server starts again; a failed model is asked for again.
  private func retry() {
    if case .failed = app.server.runState {
      app.server.restart()
    } else if let sha = app.server.engine.sha256, let row = app.models.rows.first(where: { $0.sha256 == sha }) {
      app.models.use(row, confirmedInterruption: true)
    } else {
      tab = .models
    }
  }

  private func openSettings() {
    if let url = URL(string: UIApplication.openSettingsURLString) {
      UIApplication.shared.open(url)
    }
  }
}
