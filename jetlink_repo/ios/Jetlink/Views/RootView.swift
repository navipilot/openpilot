import JetlinkKit
import JetlinkUI
import SwiftUI

/// Four tabs: Status, Models, Benchmark and Settings. Away from Status, the state rides
/// along in a pill above the tab bar, the way Music keeps what is playing in
/// view, and a tap on it goes back.
struct RootView: View {
  enum Tab: Hashable {
    case status, models, benchmark, settings
  }

  @Environment(AppModel.self) private var app
  @State private var tab: Tab = RootView.initialTab
  @State private var shutdownAlert = false

  var body: some View {
    TabView(selection: $tab) {
      SwiftUI.Tab("Status", systemImage: "gauge.with.needle.fill", value: .status) {
        StatusScreen(tab: $tab)
      }
      SwiftUI.Tab("Models", systemImage: "shippingbox.fill", value: .models) {
        ModelsScreen()
      }
      SwiftUI.Tab("Benchmark", systemImage: "stopwatch.fill", value: .benchmark) {
        BenchmarkScreen()
      }
      SwiftUI.Tab("Settings", systemImage: "gearshape.fill", value: .settings) {
        SettingsScreen()
      }
    }
    .tabBarMinimizeBehavior(.onScrollDown)
    .tabViewBottomAccessory(isEnabled: tab != .status) {
      StatusAccessory(state: app.status)
        .onTapGesture { tab = .status }
    }
    .onChange(of: app.server.state.shutdownRequests) { _, count in
      shutdownAlert = count > 0
    }
    .alert("Comma Asked to Shut Down", isPresented: $shutdownAlert) {
      Button("OK") {}
    } message: {
      // The reason the comma gave is in Logs.
      Text("Jetlink can't turn off your \(ThisDevice.name), but you can close the app.")
    }
  }

  /// `-tab models`, `-tab benchmark`, `-tab settings`, `-tab logs` or
  /// `-tab connect` on the command line opens there, for screenshots from
  /// the simulator.
  static var initialTab: Tab {
    switch UserDefaults.standard.string(forKey: "tab") {
    case "models": .models
    case "benchmark": .benchmark
    case "settings", "logs", "connect": .settings
    default: .status
    }
  }
}

/// The state in one line: the summary's symbol and word, then the headroom
/// while serving, or the model.
struct StatusAccessory: View {
  let state: StatusState
  @Environment(\.tabViewBottomAccessoryPlacement) private var placement

  var body: some View {
    let summary = state.summary
    HStack(spacing: 10) {
      Image(systemName: summary.symbol)
        .foregroundStyle(summary.tone.color)
        .font(.body.weight(.semibold))
      Text(summary.title)
        .font(.subheadline.weight(.semibold))
      Spacer(minLength: 8)
      Text(detail)
        .font(.subheadline.monospacedDigit())
        .foregroundStyle(.secondary)
        .lineLimit(1)
    }
    .padding(.horizontal, placement == .inline ? 12 : 16)
    .contentShape(.rect)
    .accessibilityElement(children: .combine)
  }

  private var detail: String {
    if let recent = state.servedStats {
      return FrameBudgetView.headroomText(p99: recent.servedMs.p99)
    }
    return state.modelName ?? ""
  }
}
