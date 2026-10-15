import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkKit

/// The snapshot the Android app draws. Its JSON is pinned by
/// Fixtures/android_snapshot.json, which the Android app's unit tests parse,
/// so a change here that the app would not read fails one side or the other.
/// JETLINK_WRITE_FIXTURES=1 rewrites the fixture from this code.
struct AppSnapshotTests {
  static let sha = "a086d5249fc308bb7c0ffbcbb2b8a53c3f6b1f0a1d2c3b4a5e6f7081920304050"
  static let ref = "f877d7a0ccc3cce943c76e285214c020cd65c899"
  static let otherRef = "37bfa1413edcdc2e8844984b83727c33f81d8f46"

  static let stats = StatsEvent(
    frames: 12_345, fps: 19.9, servedMs: StatsEvent.Total(mean: 31.6, p99: 38.4, max: 41.9),
    stagesMs: StatsEvent.Stages(queue: 0.6, gpu: 29.4, other: 1.2, send: 0.4), slow: 0, windowS: 1.0)

  /// A server serving a comma over USB 3 with a model loaded, a second
  /// model downloading, and the catalog fetched.
  static func serving() -> AppSnapshot {
    let snapshot = AppSnapshot()
    snapshot.serverStarted()
    snapshot.apply(.server(ServerEvent(state: "serving", detail: "", backend: "ort", runtimeVersion: "1.29.0", device: "htp-SM8650")))
    snapshot.apply(.link(LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3")))
    snapshot.apply(.engine(EngineEvent(state: .ready, sha256: sha, detail: "", stage: nil, frac: 1, msg: "", loadOnly: false)))
    snapshot.apply(.stats(stats))
    snapshot.apply(
      .inventory(
        InventoryEvent(
          loaded: sha, lastLoaded: sha,
          models: [InventoryModel(sha256: sha, bytes: 765_953_504, path: "/data/jetlink/models/a086d5249fc308bb.onnx", name: "BMRLNAP Model v4", ref: ref)],
          artifacts: [
            InventoryArtifact(
              sha256: sha, key: "a086d5249fc308bb.ort1.29.0.htp-SM8650", path: "/data/jetlink/engines/a086d5249fc308bb.ort1.29.0.htp-SM8650.ortcache",
              bytes: 800_000_000, backend: "ort", runtimeVersion: "1.29.0", device: "htp-SM8650", builtAt: "2026-09-28T10:00:00Z",
              buildSeconds: 190.5, checkpoint: nil, current: true)
          ],
          disk: InventoryDisk(modelsBytes: 765_953_504, enginesBytes: 800_000_000, freeBytes: 40_000_000_000))))
    snapshot.apply(
      .catalog(
        CatalogEvent(
          fetchedAt: 1_759_000_000, url: "https://example.invalid/driving_models_chestnut_v26.json", defaultRef: otherRef, error: nil,
          models: [
            CatalogModel(
              name: "Cinque Terre Model V3", shortName: "CTMV3", ref: otherRef, buildTime: "2026-09-17T11:04:00Z", index: 13,
              sha256: "404a18cfd86d29630000000000000000000000000000000000000000000000ff", bytes: 766_000_000),
            CatalogModel(
              name: "BMRLNAP Model v4", shortName: "BMRLNAP", ref: ref, buildTime: "2026-08-30T09:41:12Z", index: 11, sha256: sha,
              bytes: 765_953_504),
          ])))
    snapshot.apply(
      .download(
        DownloadEvent(
          sha256: "404a18cfd86d29630000000000000000000000000000000000000000000000ff", ref: otherRef, state: "progress", frac: 0.42,
          bytes: 321_000_000, total: 766_000_000, rateBps: 41_000_000, detail: "", source: "https://example.invalid/info/lfs")))
    let frame = BenchmarkStats(mean: 24.1, p50: 23.8, p90: 25.2, p99: 27.9, max: 31.4)
    let report = BenchmarkReport(
      sha256: sha, device: "htp-SM8650", seconds: 60.1, frames: 1195, frame: frame,
      accelerator: BenchmarkStats(mean: 21.9, p50: 21.7, p90: 22.8, p99: 25.1, max: 28.2),
      queues: BenchmarkStats(mean: 0.4, p50: 0.4, p90: 0.5, p99: 0.7, max: 1.1),
      output: BenchmarkStats(mean: 0.2, p50: 0.2, p90: 0.2, p99: 0.3, max: 0.4), build: "Release build, frame thread user-interactive, CPU keep-warm off",
      over35: 0, over50: 0,
      windows: [
        BenchmarkWindow(startSecond: 0, frame: BenchmarkStats(mean: 23.9, p50: 23.7, p90: 24.9, p99: 26.8, max: 29.0), thermal: "nominal"),
        BenchmarkWindow(startSecond: 10, frame: BenchmarkStats(mean: 24.3, p50: 24.0, p90: 25.4, p99: 27.9, max: 31.4), thermal: "fair"),
      ],
      thermalAtStart: "nominal", thermalAtEnd: "fair", cancelled: false)
    snapshot.apply(
      .benchmark(BenchmarkEvent(state: "done", elapsed: 60.1, total: 60, frames: 1195, frame: frame, report: report, detail: "")))
    return snapshot
  }

  /// The snapshot with the clock taken out: history times are when the
  /// stats event arrived.
  static func json(_ snapshot: AppSnapshot) throws -> [String: Any] {
    var object = try #require(snapshot.snapshot(after: 0, timeout: 0, port: 5599) { stats })
    if var history = object["history"] as? [[String: Any]] {
      for index in history.indices { history[index]["at"] = 1_759_000_000.0 + Double(index) }
      object["history"] = history
    }
    return object
  }

  static var fixtureURL: URL {
    SourceTree.root().appending(path: "JetlinkKit/Tests/JetlinkKitTests/Fixtures/android_snapshot.json")
  }

  @Test func matchesTheFixtureTheAndroidAppParses() throws {
    let object = try Self.json(Self.serving())
    let data = try JSONSerialization.data(withJSONObject: object, options: [.prettyPrinted, .sortedKeys, .withoutEscapingSlashes])
    if ProcessInfo.processInfo.environment["JETLINK_WRITE_FIXTURES"] == "1" {
      try (data + Data("\n".utf8)).write(to: Self.fixtureURL)
    }
    let fixture = try JSONSerialization.jsonObject(with: Fixture.data("android_snapshot.json"))
    var comparison = JSONComparison()
    comparison.compare(python: fixture, swift: object, at: "")
    #expect(comparison.differences.isEmpty, "\(comparison.differences)")
  }

  @Test func rowsComeFromTheSharedBuilder() throws {
    let object = try Self.json(Self.serving())
    let rows = try #require(object["models"] as? [[String: Any]])
    #expect(rows.count == 2)
    let loaded = try #require(rows.first { ($0["sha256"] as? String) == Self.sha })
    #expect(loaded["is_loaded"] as? Bool == true)
    #expect((loaded["status"] as? [String: Any])?["kind"] as? String == "loaded")
    #expect(loaded["can_use"] as? Bool == false)
    #expect(loaded["has_files"] as? Bool == true)
    let downloading = try #require(rows.first { ($0["ref"] as? String) == Self.otherRef })
    #expect((downloading["status"] as? [String: Any])?["kind"] as? String == "downloading")
    #expect(downloading["is_default"] as? Bool == true)
    #expect(downloading["has_files"] as? Bool == false)
  }

  @Test func disconnectingClearsTheLiveNumbers() throws {
    let snapshot = Self.serving()
    snapshot.apply(.link(LinkEvent(state: .disconnected, detail: "the device went away", peer: nil)))
    let object = try #require(snapshot.snapshot(after: 0, timeout: 0) { Self.stats })
    #expect((object["history"] as? [Any])?.isEmpty == true)
    #expect(object["recent"] is NSNull)
  }

  @Test func aSnapshotWaitsForNews() throws {
    let snapshot = AppSnapshot()
    let version = try #require(snapshot.snapshot(after: 0, timeout: 0)?["version"] as? Int)
    let started = Date()
    #expect(snapshot.snapshot(after: version, timeout: 0.2) == nil)
    #expect(Date().timeIntervalSince(started) >= 0.15)
    let waiter = Thread {
      Thread.sleep(forTimeInterval: 0.05)
      snapshot.apply(.link(LinkEvent(state: .waiting, detail: "", peer: nil)))
    }
    waiter.start()
    let early = Date()
    let next = snapshot.snapshot(after: version, timeout: 5)
    #expect(Date().timeIntervalSince(early) < 2)
    #expect((next?["version"] as? Int ?? 0) > version)
  }

  @Test func aConnectedCommaAlwaysHasNews() throws {
    let snapshot = Self.serving()
    let version = try #require(snapshot.snapshot(after: 0, timeout: 0)?["version"] as? Int)
    // the live numbers move every second while a comma is connected
    #expect(snapshot.snapshot(after: version, timeout: 0) != nil)
  }
}
