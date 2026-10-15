import JetlinkKit
import JetlinkUI
import SwiftUI

/// Is this iPhone or iPad fast enough, and does it stay fast enough? The
/// loaded model at the comma's pace on the device alone.
struct BenchmarkScreen: View {
  @Environment(AppModel.self) private var app
  @Environment(\.horizontalSizeClass) private var horizontalSizeClass
  @State private var refusal: String?
  @State private var starting = false
  /// `-benchmark 60` on the command line runs one once a model is loaded,
  /// for screenshots from the simulator.
  @State private var scripted: Double? = UserDefaults.standard.double(forKey: "benchmark") > 0 ? UserDefaults.standard.double(forKey: "benchmark") : nil

  var body: some View {
    NavigationStack {
      ScrollView {
        Group {
          // An iPad has room for the run and its verdict beside the
          // numbers behind it; a narrower screen has them one under the other.
          if horizontalSizeClass == .regular, let report, report.frames > 0 {
            HStack(alignment: .top, spacing: StatusContent.spacing) {
              VStack(spacing: StatusContent.spacing) {
                runCard
                VerdictCard(report: report)
              }
              VStack(spacing: StatusContent.spacing) { results(report) }
            }
          } else {
            VStack(spacing: StatusContent.spacing) {
              runCard
              if let report {
                VerdictCard(report: report)
                if report.frames > 0 { results(report) }
              }
            }
            .frame(maxWidth: readableContentWidth)
            .frame(maxWidth: .infinity)
          }
        }
        .padding(.bottom, 24)
      }
      .contentMargins(.horizontal, StatusContent.margin, for: .scrollContent)
      .background(Color.groupedBackground)
      .navigationTitle("Benchmark")
      .onChange(of: sha256, initial: true) {
        if let seconds = scripted, blocker == nil {
          scripted = nil
          start(seconds: seconds)
        }
      }
      .toolbar {
        if let report {
          ToolbarItem(placement: .topBarTrailing) {
            ShareLink(item: report.text, preview: SharePreview("Jetlink Benchmark"))
          }
        }
      }
    }
  }

  /// The numbers behind the verdict: the totals, then the run ten seconds at a time.
  @ViewBuilder
  private func results(_ report: BenchmarkReport) -> some View {
    totals(report)
    if report.windows.count > 1 {
      WindowsCard(windows: report.windows)
    }
  }

  // MARK: state

  private var event: BenchmarkEvent? { app.server.state.benchmark }
  private var running: Bool { event.map { !$0.isFinished } ?? false }
  private var report: BenchmarkReport? { event?.report }
  private var sha256: String? { app.server.engine.state == .ready ? app.server.engine.sha256 : nil }
  private var connected: Bool { app.server.link.state == .connected }

  /// Why a run cannot start now, in a few words; nil when it can.
  private var blocker: String? {
    BenchmarkBlocker.reason(serving: app.server.runState == .serving, modelLoaded: sha256 != nil, commaConnected: connected)
  }

  // MARK: the run

  private var runCard: some View {
    SummaryCard(title: "Benchmark", systemImage: "stopwatch.fill", tint: .blue, trailing: app.settings.device.title) {
      VStack(alignment: .leading, spacing: 14) {
        Text(sha256.flatMap(app.modelName) ?? (sha256 == nil ? "No Model" : "Model"))
          .font(.title2.weight(.bold))
        if let event, running {
          progress(event)
        } else {
          buttons
        }
        if let text = refusal ?? (event?.state == "failed" ? "The benchmark failed. See Logs for details." : nil) {
          Text(text)
            .font(.footnote)
            .foregroundStyle(.red)
        } else if !running, let blocker {
          Text(blocker)
            .font(.footnote)
            .foregroundStyle(.orange)
        }
        Text("Run it with the \(ThisDevice.name) charging and in its mount.")
          .font(.footnote)
          .foregroundStyle(.secondary)
      }
    }
  }

  private var buttons: some View {
    HStack(spacing: 12) {
      Button {
        start(seconds: 60)
      } label: {
        Label("1 Minute", systemImage: "play.fill")
          .frame(maxWidth: .infinity)
      }
      .buttonStyle(.glassProminent)
      Button {
        start(seconds: 600)
      } label: {
        Label("10 Minutes", systemImage: "flame.fill")
          .frame(maxWidth: .infinity)
      }
      .buttonStyle(.glass)
    }
    .controlSize(.large)
    .disabled(blocker != nil || starting)
  }

  private func progress(_ event: BenchmarkEvent) -> some View {
    VStack(alignment: .leading, spacing: 12) {
      ProgressView(value: min(event.elapsed / max(event.total, 1), 1))
        .tint(.blue)
      HStack {
        Text("\(BenchmarkClock.text(event.elapsed)) of \(BenchmarkClock.text(event.total))")
        Spacer()
        Text("\(event.frames.formatted()) frames")
      }
      .font(.subheadline.monospacedDigit())
      .foregroundStyle(.secondary)
      HStack(spacing: 0) {
        Figure("P50", ms: event.frame?.p50)
        Divider().frame(height: 32)
        Figure("P99", ms: event.frame?.p99)
      }
      Button(role: .destructive) {
        cancel()
      } label: {
        Label("Cancel", systemImage: "stop.fill")
          .frame(maxWidth: .infinity)
      }
      .buttonStyle(.glass)
      .controlSize(.large)
    }
  }

  private func start(seconds: Double) {
    refusal = nil
    starting = true
    Task {
      defer { starting = false }
      do {
        let reply = try await app.server.send(.benchmark(seconds: seconds))
        if !reply.ok { refuse(reply.error) }
      } catch {
        refuse(error.localizedDescription)
      }
    }
  }

  /// A short line on the card, and the server's reason in Logs.
  private func refuse(_ reason: String?) {
    refusal = "Couldn't start the benchmark. See Logs for details."
    app.server.note(.warning, "the benchmark could not start: \(reason ?? "no reason given")")
  }

  private func cancel() {
    Task { _ = try? await app.server.send(.cancelBenchmark) }
  }

  // MARK: totals

  private func totals(_ report: BenchmarkReport) -> some View {
    MetricGrid {
      GridRow {
        MetricTile(
          title: "Frames", systemImage: "film.stack", tint: .teal, value: report.frames.formatted(),
          note: "\(BenchmarkClock.text(report.seconds)) at 20 Hz")
        MetricTile(
          title: "Over Budget", systemImage: "tortoise.fill", tint: .pink, value: report.over50.formatted(),
          note: report.over35 > 0 ? "\(report.over35.formatted()) over 35 ms" : "None over 35 ms", noteTone: report.over50 > 0 ? .red : nil)
      }
      GridRow {
        MetricTile(
          title: "Temperature", systemImage: ThermalLevel(label: report.thermalAtEnd).symbol, tint: .orange,
          value: ThermalLevel(label: report.thermalAtEnd).title,
          note: report.thermalAtStart == report.thermalAtEnd ? "Throughout" : "From \(ThermalLevel(label: report.thermalAtStart).title.lowercased())",
          noteTone: ThermalLevel(label: report.thermalAtEnd).tone)
        MetricTile(
          title: "Model", systemImage: "cpu", tint: .purple, value: report.accelerator.mean.formatted(.number.precision(.fractionLength(1))), unit: "ms",
          note: "Mean, model alone")
      }
    }
  }
}

/// Fast enough, tight, or too slow, and the numbers that say so.
struct VerdictCard: View {
  let report: BenchmarkReport

  var body: some View {
    let verdict = BenchmarkVerdict(report)
    SummaryCard(title: "Verdict", systemImage: verdict.symbol, tint: verdict.tone, trailing: report.cancelled ? "Stopped Early" : nil) {
      BenchmarkVerdictSummary(report: report)
    }
  }
}

/// The run ten seconds at a time, with the phone's temperature as each closed.
struct WindowsCard: View {
  let windows: [BenchmarkWindow]

  var body: some View {
    SummaryCard(title: "Over Time", systemImage: "chart.bar.fill", tint: .indigo, trailing: "10 s each") {
      BenchmarkWindowRows(windows: windows)
    }
  }
}
