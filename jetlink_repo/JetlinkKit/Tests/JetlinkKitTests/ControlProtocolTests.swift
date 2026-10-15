import Foundation
import Testing

@testable import JetlinkKit

/// The fixture files, copied into the test bundle as the Fixtures folder.
enum Fixture {
  static let directory = Bundle.module.url(forResource: "Fixtures", withExtension: nil)

  static func data(_ name: String) throws -> Data {
    guard let directory else { throw CocoaError(.fileNoSuchFile) }
    return try Data(contentsOf: directory.appending(path: name))
  }

  static func lines(_ name: String) throws -> [Data] {
    let text = try String(decoding: data(name), as: UTF8.self)
    return text.split(separator: "\n", omittingEmptySubsequences: true).map { Data($0.utf8) }
  }
}

struct ControlProtocolTests {
  @Test func commandsReadFromTheirObjects() throws {
    let cases: [(ControlCommand, [String: Any])] = [
      (.status, ["cmd": "status"]),
      (.catalog(refresh: true), ["cmd": "catalog", "refresh": true]),
      (.catalog(refresh: false), ["cmd": "catalog"]),
      (.download(ref: "f877d7a0ccc3cce943c76e285214c020cd65c899", sha256: nil), ["cmd": "download", "ref": "f877d7a0ccc3cce943c76e285214c020cd65c899"]),
      (.download(ref: nil, sha256: "a086"), ["cmd": "download", "ref": NSNull(), "sha256": "a086"]),
      (.cancelDownload(sha256: "a086"), ["cmd": "cancel_download", "sha256": "a086"]),
      (.importModel(path: "/Users/me/Downloads/big.onnx"), ["cmd": "import", "path": "/Users/me/Downloads/big.onnx"]),
      (.prepare(sha256: "a086", frameSkip: 2), ["cmd": "prepare", "sha256": "a086", "frame_skip": 2]),
      (.prepare(sha256: "a086", frameSkip: Pinned.defaultFrameSkip), ["cmd": "prepare", "sha256": "a086"]),
      (.unload, ["cmd": "unload"]),
      (.forget(sha256: "a086", artifacts: true, model: false), ["cmd": "forget", "sha256": "a086"]),
      (.forget(sha256: "a086", artifacts: false, model: true), ["cmd": "forget", "sha256": "a086", "artifacts": false, "model": true]),
      (.inventory, ["cmd": "inventory"]),
      (.shutdown, ["cmd": "shutdown"]),
      (.benchmark(seconds: 30), ["cmd": "benchmark", "seconds": 30]),
      (.benchmark(seconds: 60), ["cmd": "benchmark"]),
      (.cancelBenchmark, ["cmd": "cancel_benchmark"]),
    ]
    for (command, object) in cases {
      #expect(try ControlCommand(object: object) == command, "\(object)")
    }
    #expect(throws: ControlCommand.Invalid.self) { try ControlCommand(object: ["cmd": "prepare"]) }
    #expect(throws: ControlCommand.Invalid.self) { try ControlCommand(object: ["cmd": "reboot"]) }
  }

  func line(_ event: ControlEvent) throws -> [String: Any] {
    let data = event.jsonLine(at: Date(timeIntervalSince1970: 5))
    #expect(data.last == 0x0A && !data.dropLast().contains(0x0A))
    return try #require(try JSONSerialization.jsonObject(with: data) as? [String: Any])
  }

  @Test func eventsAreOneLineWithSnakeCaseKeysAndNoNulls() throws {
    let link = try line(.link(LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3")))
    #expect(link["event"] as? String == "link" && link["t"] as? Double == 5)
    #expect(link["state"] as? String == "connected" && link["medium"] as? String == "usb3")
    let server = try line(.server(ServerEvent(state: "running", detail: "", backend: "ort", runtimeVersion: nil, device: nil)))
    #expect(server["backend"] as? String == "ort")
    #expect(Set(server.keys) == ["event", "t", "state", "detail", "backend"])
    let reply = try line(.reply(ReplyEvent(id: nil, ok: true, error: nil, extras: ["queued": .bool(true)])))
    #expect(Set(reply.keys) == ["event", "t", "ok", "queued"])
    let shutdown = try line(.shutdownRequest(ShutdownRequestEvent(reason: "car battery")))
    #expect(shutdown["event"] as? String == "shutdown_request" && shutdown["reason"] as? String == "car battery")
    #expect(try line(.hello(HelloEvent(version: "0.7.0"))).count == 3)
  }

  @Test func aReadyEngineSaysWhatRunsItOnlyWhenItsBackendDoes() throws {
    let ready = EngineEvent(state: .ready, sha256: "ab", detail: "", stage: nil, frac: 1, msg: "", loadOnly: false)
    #expect(try line(.engine(ready))["accelerator"] == nil)
    let fellBack = EngineEvent(state: .ready, sha256: "ab", detail: "", stage: nil, frac: 1, msg: "", loadOnly: false, accelerator: "GPU(fp16)")
    #expect(try line(.engine(fellBack))["accelerator"] as? String == "GPU(fp16)")
    let read = try JSONDecoder().decode(EngineEvent.self, from: Data(#"{"state":"ready","detail":"","frac":1,"msg":"","loadOnly":false}"#.utf8))
    #expect(read.accelerator == nil)
  }

  @Test func aBenchmarkReportGoesOutWhole() throws {
    let report = BenchmarkReport(
      sha256: String(repeating: "a", count: 64), device: "ane-Apple A19 Pro", seconds: 60, frames: 1195, frame: BenchmarkStats.empty,
      accelerator: BenchmarkStats.empty, queues: BenchmarkStats.empty, output: BenchmarkStats.empty, build: "Release build, CPU keep-warm on",
      over35: 3, over50: 0, windows: [BenchmarkWindow(startSecond: 0, frame: BenchmarkStats.empty, thermal: "nominal")], thermalAtStart: "nominal",
      thermalAtEnd: "fair", cancelled: false)
    let done = BenchmarkEvent(state: "done", elapsed: 60, total: 60, frames: 1195, frame: nil, report: report, detail: "")
    let object = try line(.benchmark(done))
    #expect(object["event"] as? String == "benchmark" && object["frame"] == nil)
    let written = try #require(object["report"] as? [String: Any])
    #expect(written["thermal_at_end"] as? String == "fair" && written["over35"] as? Int == 3)
    #expect((written["windows"] as? [[String: Any]])?.first?["start_second"] as? Int == 0)
    #expect(report.text.contains("over 35 ms: 3"))
  }
}
