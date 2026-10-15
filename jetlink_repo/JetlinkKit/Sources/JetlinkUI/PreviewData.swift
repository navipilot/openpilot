#if canImport(SwiftUI)
  import Foundation
  import JetlinkKit

  /// Sample events for previews and formatting tests, shared by the Mac and
  /// iPhone apps. Nothing here is used by a running app; the values are copies
  /// of what a real server sends.
  public enum PreviewData {
    // MARK: Identifiers

    public static let bigModelSHA = "a086d5249fc308bb7c0ffbcbb2b8a53c3f6b1f0a1d2c3b4a5e6f7081920304050"
    public static let bigModelRef = "f877d7a0ccc3cce943c76e285214c020cd65c899"
    public static let bigModelName = "BMRLNAP Model v4 (August 30, 2026)"
    public static let cinqueRef = "37bfa1413edcdc2e8844984b83727c33f81d8f46"

    // MARK: Link

    public static let linkWaiting = LinkEvent(state: .waiting, detail: "waiting for a jetlink gadget at 1209:0001", peer: nil)
    public static let linkConnected = LinkEvent(state: .connected, detail: "", peer: "usb")
    public static let linkDisconnected = LinkEvent(state: .disconnected, detail: "the device went away", peer: nil)

    // MARK: Engine

    public static let engineNone = EngineEvent(state: .none, sha256: nil, detail: "", stage: nil, frac: 0, msg: "", loadOnly: false)
    public static let engineBuilding = EngineEvent(
      state: .building,
      sha256: bigModelSHA,
      detail: "",
      stage: "convert",
      frac: 0.27,
      msg: "converting for CoreML, 207 MB of 766 MB written",
      loadOnly: false
    )
    public static let engineLoading = EngineEvent(
      state: .loading, sha256: bigModelSHA, detail: "", stage: "load", frac: 0.5, msg: "loading the CoreML model, 1.2 GB of 2.5 GB resident", loadOnly: true)
    public static let engineReady = EngineEvent(state: .ready, sha256: bigModelSHA, detail: "", stage: nil, frac: 1, msg: "", loadOnly: false)
    public static let engineFailed = EngineEvent(
      state: .failed,
      sha256: bigModelSHA,
      detail: "the model could not be parsed: unsupported operator Gather",
      stage: "failed",
      frac: 0,
      msg: "",
      loadOnly: false
    )

    // MARK: Stats

    public static let stats = StatsEvent(
      frames: 12_345,
      fps: 19.9,
      servedMs: StatsEvent.Total(mean: 31.6, p99: 38.4, max: 41.9),
      stagesMs: StatsEvent.Stages(queue: 0.6, gpu: 29.4, other: 1.2, send: 0.4),
      slow: 0,
      windowS: 1.0
    )

    /// Two minutes of one-second summaries, with one second over the budget.
    public static let statsHistory: [StatsSample] = {
      let end = Date(timeIntervalSinceReferenceDate: 780_000_000)
      return (0..<120).map { i in
        let wobble = sin(Double(i) / 6) * 1.4 + (i % 7 == 0 ? 1.1 : 0)
        let mean = 31.6 + wobble
        let p99 = mean + 5.5 + (i % 11 == 0 ? 4 : 0)
        let max = i == 83 ? 57.2 : p99 + 2.1
        let stats = StatsEvent(
          frames: 12_345 - (119 - i) * 20, fps: 20,
          servedMs: StatsEvent.Total(mean: mean, p99: p99, max: max),
          stagesMs: StatsEvent.Stages(queue: 0.6, gpu: mean - 2.2, other: 1.2, send: 0.4),
          slow: 0, windowS: 1)
        return StatsSample(at: end.addingTimeInterval(Double(i - 119)), stats: stats)
      }
    }()

    // MARK: Inventory

    public static let artifact = InventoryArtifact(
      sha256: bigModelSHA,
      key: "a086d5249fc308bb.ort1.29.0.coreml-Apple_M1_Pro",
      path: "/Users/me/Library/Application Support/Jetlink/cache/engines/a086d5249fc308bb.ortcache",
      bytes: 5_900_000_000,
      backend: "ort",
      runtimeVersion: "1.29.0",
      device: "coreml-Apple_M1_Pro",
      builtAt: "2026-09-08T21:19:15Z",
      buildSeconds: 548.9,
      checkpoint: "b9facbcc-6f1e-4c2a-9d3b-0a1c2d3e4f50",
      current: true
    )

    public static let inventoryModel = InventoryModel(
      sha256: bigModelSHA,
      bytes: 765_953_504,
      path: "/Users/me/Library/Application Support/Jetlink/cache/models/a086d5249fc308bb.onnx",
      name: bigModelName,
      ref: bigModelRef
    )

    public static let inventory = InventoryEvent(
      loaded: bigModelSHA,
      lastLoaded: bigModelSHA,
      models: [inventoryModel],
      artifacts: [artifact],
      disk: InventoryDisk(modelsBytes: 765_953_504, enginesBytes: 5_900_000_000, freeBytes: 120_000_000_000)
    )

    public static let emptyInventory = InventoryEvent(
      loaded: nil,
      lastLoaded: nil,
      models: [],
      artifacts: [],
      disk: InventoryDisk(modelsBytes: 0, enginesBytes: 0, freeBytes: 120_000_000_000)
    )

    // MARK: Catalog

    public static let catalog = CatalogEvent(
      fetchedAt: 1_757_440_000.0,
      url: "https://raw.githubusercontent.com/sunnypilot/sunnypilot-models/refs/heads/gh-pages/docs/driving_models_chestnut_v25.json",
      defaultRef: bigModelRef,
      error: nil,
      models: [
        CatalogModel(
          name: "Cinque Terre Model V2 (September 08, 2026)", shortName: "CTMV2", ref: cinqueRef,
          buildTime: "2026-09-08T11:04:00Z", index: 12, sha256: nil, bytes: nil),
        CatalogModel(
          name: bigModelName, shortName: "BMRLNAP", ref: bigModelRef,
          buildTime: "2026-08-30T09:41:12Z", index: 11, sha256: bigModelSHA, bytes: 765_953_504),
      ]
    )

    // MARK: Downloads

    public static let download = DownloadEvent(
      sha256: bigModelSHA,
      ref: bigModelRef,
      state: "progress",
      frac: 0.42,
      bytes: 321_000_000,
      total: 765_953_504,
      rateBps: 41_000_000,
      detail: "",
      source: "https://gitlab.com/commaai/openpilot-lfs.git/info/lfs"
    )

    // MARK: Rows

    public static let loadedRow = ModelRow(
      id: bigModelSHA,
      name: bigModelName,
      ref: bigModelRef,
      sha256: bigModelSHA,
      bytes: 765_953_504,
      buildTime: "2026-08-30T09:41:12Z",
      status: .loaded,
      preparedFor: [artifact],
      isLoaded: true,
      isDefault: true,
      isRequestedByComma: true,
      isLocal: false,
      isOrphan: false
    )

    public static let notDownloadedRow = ModelRow(
      id: "ref:\(cinqueRef)",
      name: "Cinque Terre Model V2 (September 08, 2026)",
      ref: cinqueRef,
      sha256: nil,
      bytes: nil,
      buildTime: "2026-09-08T11:04:00Z",
      status: .unresolved,
      preparedFor: [],
      isLoaded: false,
      isDefault: false,
      isRequestedByComma: false,
      isLocal: false,
      isOrphan: false
    )

    public static let downloadingRow = ModelRow(
      id: "b1c2d3e4f5061728394a5b6c7d8e9f00112233445566778899aabbccddeeff00",
      name: "Los Angeles Model (August 12, 2026)",
      ref: "aa11bb22cc33dd44ee55ff6677889900aabbccdd",
      sha256: "b1c2d3e4f5061728394a5b6c7d8e9f00112233445566778899aabbccddeeff00",
      bytes: 765_950_064,
      buildTime: "2026-08-12T18:02:44Z",
      status: .downloading(frac: 0.42, rateBps: 41_000_000),
      preparedFor: [],
      isLoaded: false,
      isDefault: false,
      isRequestedByComma: false,
      isLocal: false,
      isOrphan: false
    )

    public static let localRow = ModelRow(
      id: "c0ffee1122334455667788990011223344556677889900aabbccddeeff001122",
      name: "big_driving_supercombo.onnx",
      ref: nil,
      sha256: "c0ffee1122334455667788990011223344556677889900aabbccddeeff001122",
      bytes: 765_953_504,
      buildTime: nil,
      status: .downloaded,
      preparedFor: [],
      isLoaded: false,
      isDefault: false,
      isRequestedByComma: false,
      isLocal: true,
      isOrphan: false
    )

    public static let rows = [notDownloadedRow, loadedRow, downloadingRow, localRow]
  }
#endif
