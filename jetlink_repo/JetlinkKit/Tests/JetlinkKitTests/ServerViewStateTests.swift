import Foundation
import Testing

@testable import JetlinkKit

/// The reducer every app keeps its server with.
struct ServerViewStateTests {
  static let stats = StatsEvent(
    frames: 100, fps: 20, servedMs: StatsEvent.Total(mean: 30, p99: 35, max: 40),
    stagesMs: StatsEvent.Stages(queue: 0.5, gpu: 28, other: 1, send: 0.5), slow: 0, windowS: 1)
  static let connected = LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3")

  @Test func keepsWhatTheScreensShowAndLeavesTheModelEventsToOthers() {
    let state = ServerViewState()
    let server = ServerEvent(state: "serving", detail: "", backend: "ort", runtimeVersion: "1.29.0", device: "cpu-cpu")
    #expect(state.apply(.server(server)))
    #expect(state.apply(.link(Self.connected)))
    #expect(state.apply(.stats(Self.stats)))
    #expect(state.server == server && state.link == Self.connected && state.stats == Self.stats)
    let disk = InventoryDisk(modelsBytes: 0, enginesBytes: 0, freeBytes: 0)
    #expect(!state.apply(.inventory(InventoryEvent(loaded: nil, lastLoaded: nil, models: [], artifacts: [], disk: disk))))
    #expect(!state.apply(.reply(ReplyEvent(id: 1, ok: true, error: nil))))
  }

  @Test func theNumbersGoWithTheComma() {
    let state = ServerViewState()
    state.apply(.link(Self.connected))
    let start = Date()
    for second in 0..<(StatsSample.historyLength + 5) {
      state.apply(.stats(Self.stats), at: start.addingTimeInterval(Double(second)))
    }
    #expect(state.statsHistory.count == StatsSample.historyLength)
    #expect(state.statsHistory.first?.at == start.addingTimeInterval(5))
    state.apply(.link(LinkEvent(state: .disconnected, detail: "the device went away", peer: nil)))
    #expect(state.statsHistory.isEmpty && state.stats == nil)
  }

  @Test func aStopClearsTheLiveStateAndARestartTheRest() {
    let state = ServerViewState()
    state.apply(.link(Self.connected))
    state.apply(.engine(EngineEvent(state: .ready, sha256: "ab", detail: "", stage: nil, frac: 1, msg: "", loadOnly: false)))
    state.apply(.stats(Self.stats))
    state.apply(.benchmark(BenchmarkEvent(state: "done", elapsed: 1, total: 1, frames: 20, frame: nil, report: nil, detail: "")))
    state.apply(.shutdownRequest(ShutdownRequestEvent(reason: "car off")))
    state.serverStopped()
    #expect(state.link == .waiting && state.engine == .none && state.statsHistory.isEmpty)
    #expect(state.benchmark != nil && state.shutdownRequests == 1)
    state.reset()
    #expect(state.benchmark == nil && state.shutdownRequest == nil)
    // a request after a restart is still a new one
    state.apply(.shutdownRequest(ShutdownRequestEvent(reason: "car off")))
    #expect(state.shutdownRequests == 2)
  }
}
