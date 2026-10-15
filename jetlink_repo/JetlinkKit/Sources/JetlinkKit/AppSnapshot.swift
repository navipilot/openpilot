import Foundation

/// Everything an app's screens show, kept from the server's events: the
/// server as every app keeps it (ServerViewState) and the Models rows as
/// ModelStore builds them, behind a lock instead of the main actor, and
/// handed out whole as JSON. The Android app draws these
/// snapshots (JetlinkAndroid); `android_snapshot.json` in the tests' fixtures
/// is one, which the Android app's own tests parse.
public final class AppSnapshot: @unchecked Sendable {
  /// The window the headline numbers cover, as on the iPhone.
  public static let recentWindow: TimeInterval = 10

  private let condition = NSCondition()
  private var version = 1
  private var running = false
  /// What every app keeps of the server; the rest here is the Models screen's.
  private let state = ServerViewState()
  private var catalog: CatalogEvent?
  private var inventory: InventoryEvent?
  private var downloads: [String: DownloadEvent] = [:]
  private var finished: [String: Date] = [:]
  private var imports: [ImportEvent] = []
  /// Each history sample's stats, encoded once when it arrived.
  private var encodedStats: [Date: Any] = [:]
  /// The Models rows, built again only when what they come from changes.
  private var rows: [[String: Any]]?

  public init() {}

  public func reset() {
    condition.lock()
    state.reset()
    downloads = [:]
    finished = [:]
    imports = []
    encodedStats = [:]
    rows = nil
    bump()
    condition.unlock()
  }

  public func serverStarted() {
    condition.lock()
    running = true
    bump()
    condition.unlock()
  }

  public func serverStopped() {
    condition.lock()
    running = false
    state.serverStopped()
    encodedStats = [:]
    rows = nil
    bump()
    condition.unlock()
  }

  /// Under the lock.
  private func bump() {
    version += 1
    condition.broadcast()
  }

  public func apply(_ event: ControlEvent) {
    condition.lock()
    defer { condition.unlock() }
    switch event {
    case .link, .engine:
      rows = nil
    case .inventory(let value):
      inventory = value
      rows = nil
    case .catalog(let value):
      catalog = value
      rows = nil
    case .download(let value):
      downloads[value.sha256] = value
      finished[value.sha256] = value.isTerminal ? Date() : nil
      rows = nil
    case .importEvent(let value):
      if let index = imports.firstIndex(where: { $0.path == value.path }) {
        imports[index] = value
      } else {
        imports.append(value)
      }
    case .hello, .reply:
      return
    case .server, .stats, .benchmark, .shutdownRequest:
      break
    }
    state.apply(event)
    if case .stats = event, let sample = state.statsHistory.last {
      encodedStats[sample.at] = ControlEvent.object(sample.stats)
    }
    if encodedStats.count > state.statsHistory.count {
      let kept = Set(state.statsHistory.map(\.at))
      encodedStats = encodedStats.filter { kept.contains($0.key) }
    }
    bump()
  }

  /// Waits until the state is newer than `after`, or `timeout` passes, and
  /// returns it whole; nil when there is nothing new, except while a comma is
  /// connected, whose live numbers move every second. `port` is where the
  /// server listens; `recent` the frames of the last `recentWindow` seconds.
  public func snapshot(after: Int, timeout: TimeInterval, port: Int? = nil, recent: () -> StatsEvent? = { nil }) -> [String: Any]? {
    condition.lock()
    defer { condition.unlock() }
    let deadline = Date().addingTimeInterval(max(0, timeout))
    while version <= after {
      if !condition.wait(until: deadline) { break }
    }
    let lingered = finished.filter { Date().timeIntervalSince($0.value) >= ModelStore.terminalDownloadLinger.seconds }.keys
    if !lingered.isEmpty {
      for sha in lingered {
        downloads[sha] = nil
        finished[sha] = nil
      }
      rows = nil
      bump()
    }
    let link = state.link
    guard version > after || link.state == .connected else { return nil }
    if rows == nil {
      rows = ModelRowBuilder.build(catalog: catalog, inventory: inventory, downloads: downloads, engine: state.engine, link: link)
        .map { AppSnapshot.row($0, inventory: inventory) }
    }
    let recentStats = link.state == .connected ? recent() : nil
    // with the report as the other apps share it
    var benchmark: Any = NSNull()
    if let event = state.benchmark {
      var object = ControlEvent.benchmark(event).payload()
      if let text = event.report?.text { object["report_text"] = text }
      benchmark = object
    }
    return [
      "version": version,
      "running": running,
      "port": port ?? NSNull(),
      "server": state.server.map { ControlEvent.object($0) } ?? NSNull(),
      "link": ControlEvent.object(link),
      // what the connected comma's link is carried over, as the apps name it
      "medium": link.connectedMedium.map { ["name": $0.rawValue, "title": $0.title, "slow": $0.isSlow] as [String: Any] } ?? NSNull(),
      "engine": ControlEvent.object(state.engine),
      "recent": recentStats.map { ControlEvent.object($0) } ?? NSNull(),
      "history": state.statsHistory.map { ["at": $0.at.timeIntervalSince1970, "stats": encodedStats[$0.at] ?? NSNull()] as [String: Any] },
      "models": rows ?? [],
      "catalog": catalog.map { catalog -> Any in
        [
          "fetched_at": catalog.fetchedAt ?? NSNull(),
          "error": catalog.error ?? NSNull(),
          "default_ref": catalog.defaultRef,
          "count": catalog.models.count,
        ] as [String: Any]
      } ?? NSNull(),
      "disk": inventory.map { ControlEvent.object($0.disk) } ?? NSNull(),
      "imports": imports.map { ControlEvent.object($0) },
      "benchmark": benchmark,
      "shutdown_requests": state.shutdownRequests,
    ]
  }

  /// A Models row as the app draws it.
  static func row(_ row: ModelRow, inventory: InventoryEvent?) -> [String: Any] {
    var status: [String: Any]
    switch row.status {
    case .unresolved: status = ["kind": "unresolved"]
    case .notDownloaded: status = ["kind": "not_downloaded"]
    case .downloading(let frac, let rate): status = ["kind": "downloading", "frac": frac, "rate_bps": rate]
    case .downloaded: status = ["kind": "downloaded"]
    case .preparing(let stage, let frac, let msg): status = ["kind": "preparing", "stage": stage, "frac": frac, "msg": msg]
    case .prepared: status = ["kind": "prepared"]
    case .loaded: status = ["kind": "loaded"]
    case .failed(let detail): status = ["kind": "failed", "detail": detail]
    }
    return [
      "id": row.id,
      "name": row.name,
      "display_name": row.displayName,
      "ref": row.ref ?? NSNull(),
      "sha256": row.sha256 ?? NSNull(),
      "bytes": row.bytes ?? NSNull(),
      "build_time": row.buildTime ?? NSNull(),
      "status": status,
      "prepared_for": row.preparedFor.map { ControlEvent.object($0) },
      "is_loaded": row.isLoaded,
      "is_default": row.isDefault,
      "is_requested_by_comma": row.isRequestedByComma,
      "is_local": row.isLocal,
      "is_orphan": row.isOrphan,
      "can_use": ModelStore.canUse(row),
      "has_files": row.hasFiles(in: inventory),
    ]
  }
}

extension Duration {
  /// In seconds, as TimeInterval counts them.
  var seconds: TimeInterval { Double(components.seconds) + Double(components.attoseconds) / 1e18 }
}
