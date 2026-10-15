import AppKit
import JetlinkKit
import JetlinkUI
import SwiftUI

/// Is this Mac fast enough, and does it stay fast enough? The loaded model at
/// the comma's pace on the Mac alone: the server's benchmark, the one the
/// iPhone app runs, with the same verdict.
struct BenchmarkView: View {
  @Environment(ServerStore.self) private var server
  @Environment(ModelStore.self) private var models
  @State private var refusal: String?
  @State private var starting = false

  var body: some View {
    Form {
      Section {
        LabeledContent("Model", value: modelName)
        if let event, running {
          progress(event)
        } else {
          HStack {
            Button("Run for 1 Minute", systemImage: "play.fill") { start(seconds: 60) }
            Button("Run for 10 Minutes", systemImage: "flame.fill") { start(seconds: 600) }
          }
          .disabled(blocker != nil || starting)
        }
        if let text = refusal ?? (event?.state == "failed" ? event?.detail : nil) {
          Text(text)
            .foregroundStyle(.red)
        } else if !running, let blocker {
          Text(blocker)
            .foregroundStyle(.orange)
        }
      } header: {
        Text("Benchmark")
      } footer: {
        Text("Runs the selected model at 20 Hz with mock camera frames. Does not include USB latency.")
          .font(.callout)
          .foregroundStyle(.secondary)
          .fixedSize(horizontal: false, vertical: true)
      }

      if let report {
        Section {
          BenchmarkVerdictSummary(report: report)
            .padding(.vertical, 4)
          LabeledContent("Frames", value: "\(report.frames.formatted()) in \(BenchmarkClock.text(report.seconds))")
          if report.frames > 0 {
            LabeledContent("Over 50 ms", value: report.over50.formatted())
            LabeledContent("Over 35 ms", value: report.over35.formatted())
            LabeledContent("Model alone", value: "\(FrameBudgetView.ms(report.accelerator.mean)) mean")
          }
          LabeledContent("Temperature") {
            let end = ThermalLevel(label: report.thermalAtEnd)
            Label(end.title, systemImage: end.symbol)
              .foregroundStyle(end.tone)
          }
        } header: {
          Text(report.cancelled ? "Result (stopped early)" : "Result")
        }
        if report.windows.count > 1 {
          Section("Ten Seconds at a Time") {
            BenchmarkWindowRows(windows: report.windows)
          }
        }
      }
    }
    .formStyle(.grouped)
    .navigationTitle("Benchmark")
    .toolbar {
      if let report {
        Button("Copy Report", systemImage: "doc.on.doc") {
          NSPasteboard.general.clearContents()
          NSPasteboard.general.setString(report.text, forType: .string)
        }
      }
    }
  }

  // MARK: state

  private var event: BenchmarkEvent? { server.state.benchmark }
  private var running: Bool { event.map { !$0.isFinished } ?? false }
  private var report: BenchmarkReport? { event?.report }
  private var sha256: String? { server.engine.state == .ready ? server.engine.sha256 : nil }

  private var modelName: String {
    guard let sha256 else { return "None loaded" }
    return models.row(for: sha256)?.displayName ?? "Unknown model \(sha256.prefix(16))"
  }

  /// Why a run cannot start now, in a few words; nil when it can.
  private var blocker: String? {
    BenchmarkBlocker.reason(serving: server.runState == .serving, modelLoaded: sha256 != nil, commaConnected: server.link.state == .connected)
  }

  private func progress(_ event: BenchmarkEvent) -> some View {
    VStack(alignment: .leading, spacing: 10) {
      ProgressView(value: min(event.elapsed / max(event.total, 1), 1))
      HStack {
        Text("\(BenchmarkClock.text(event.elapsed)) of \(BenchmarkClock.text(event.total)), \(event.frames.formatted()) frames")
          .monospacedDigit()
          .foregroundStyle(.secondary)
        Spacer()
        Button("Cancel", systemImage: "stop.fill", role: .destructive) {
          Task { _ = try? await server.send(.cancelBenchmark) }
        }
      }
      HStack(spacing: 0) {
        Figure("P50", ms: event.frame?.p50)
        Divider().frame(height: 32)
        Figure("P99", ms: event.frame?.p99)
      }
    }
  }

  private func start(seconds: Double) {
    refusal = nil
    starting = true
    Task {
      defer { starting = false }
      do {
        let reply = try await server.send(.benchmark(seconds: seconds))
        if !reply.ok { refusal = reply.error ?? "The benchmark could not start." }
      } catch {
        refusal = error.localizedDescription
      }
    }
  }
}
