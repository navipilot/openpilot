import Foundation
import JetlinkKit
import JetlinkRegistry

/// An artifact as every backend builds, loads and describes it: a file or a
/// directory under engines/, and beside it the sidecar JSON that says how it
/// was built and how long it and its last load took, which the next build and
/// load read for their progress estimates.
package enum Artifact {
  /// `<artifact>.json`, whichever kind the artifact is.
  package static func sidecarURL(_ artifact: URL) -> URL {
    beside(artifact, "json")
  }

  /// `<artifact>.<ext>`: a file of the artifact's own beside it, which
  /// removing the model's prepared files removes with it.
  package static func beside(_ artifact: URL, _ ext: String) -> URL {
    artifact.deletingLastPathComponent().appending(path: artifact.deletingPathExtension().lastPathComponent + "." + ext)
  }

  /// The sidecar, or empty when there is none.
  package static func sidecar(_ artifact: URL) -> [String: Any] {
    guard let data = try? Data(contentsOf: sidecarURL(artifact)) else { return [:] }
    return ((try? JSONSerialization.jsonObject(with: data)) as? [String: Any]) ?? [:]
  }

  package static func writeSidecar(_ artifact: URL, _ meta: [String: Any]) throws {
    try write(meta, to: sidecarURL(artifact))
  }

  /// A built artifact's sidecar, once the artifact is one this build loads.
  /// Throws `ArtifactInvalid`, which rebuilds it, when the sidecar's
  /// `prepare` is not `version` (`unversioned` standing in for a sidecar
  /// without one) or one of `files` is missing from it. `runtime` names the
  /// backend in the message.
  package static func open(_ artifact: URL, version: Int, unversioned: Int, runtime: String, files: [String]) throws -> [String: Any] {
    let name = artifact.lastPathComponent
    let meta = sidecar(artifact)
    let built = (meta["prepare"] as? NSNumber)?.intValue ?? unversioned
    if built != version {
      throw ArtifactInvalid("\(name): prepared as version \(built), \(runtime) builds are now at \(version); rebuilding")
    }
    if let missing = files.first(where: { !FileManager.default.fileExists(atPath: artifact.appending(path: $0).path) }) {
      throw ArtifactInvalid("\(name): \(missing) is missing")
    }
    return meta
  }

  static func write(_ meta: [String: Any], to url: URL) throws {
    let data = try JSONSerialization.data(withJSONObject: meta, options: [.prettyPrinted, .sortedKeys, .withoutEscapingSlashes])
    // The seconds are Python's round(x, 1) floats, which keep their ".0";
    // JSONSerialization writes a whole 36.0 as 36.
    let text = String(decoding: data, as: UTF8.self).replacingOccurrences(
      of: #"("(?:build|load)_seconds"\s*:\s*-?\d+)(?=\s*[,}\n])"#, with: "$1.0", options: .regularExpression)
    try Data(text.utf8).write(to: url, options: .atomic)
  }

  /// The keys every build's sidecar has, as the Python backends name them.
  /// The runtime's release goes under `runtimeKey`, `onnxruntime` or
  /// `trt_version`, which the registry's inventory reads back.
  package static func meta(_ backend: any EngineBackend, runtimeKey: String, model: URL, started: Date) -> [String: Any] {
    [
      "backend": backend.name,
      runtimeKey: backend.runtimeVersion,
      "device": backend.deviceTag(),
      "build_seconds": pythonRound(Date().timeIntervalSince(started), 1),
      "onnx": model.lastPathComponent,
      "built_at": ISO8601DateFormatter().string(from: Date()),
    ]
  }

  /// Builds `artifact`. `body` makes `staged`, the file or directory, in a
  /// `tmp*` directory beside the artifact where the cache's sweep finds it if
  /// the build is killed, and returns the sidecar. The staged artifact then
  /// replaces any old one, the sidecar goes beside it with `metaExtra` on
  /// top, and the build reports done. The staging goes either way.
  package static func build(
    _ artifact: URL, metaExtra: [String: Any], report: ProgressFn, _ body: (_ staged: URL) throws -> [String: Any]
  ) throws {
    let fm = FileManager.default
    let temp = artifact.deletingLastPathComponent().appending(path: "tmp\(UUID().uuidString.prefix(8))", directoryHint: .isDirectory)
    try fm.createDirectory(at: temp, withIntermediateDirectories: true)
    defer { try? fm.removeItem(at: temp) }
    let staged = temp.appending(path: "artifact")
    var meta = try body(staged)
    if fm.fileExists(atPath: artifact.path) {
      try fm.removeItem(at: artifact)
    }
    try fm.moveItem(at: staged, to: artifact)
    for (key, value) in metaExtra { meta[key] = value }
    try writeSidecar(artifact, meta)
    report("build", 1, "done in \(meta["build_seconds"] ?? 0)s")
  }

  /// Loads with progress paced by the last load's time, then records this
  /// one's in the sidecar for the next.
  package static func load<E: Engine>(
    _ artifact: URL, meta: [String: Any], what: String, report: @escaping ProgressFn, _ body: () throws -> E
  ) throws -> (engine: E, seconds: TimeInterval) {
    let started = Date()
    let took = (meta["load_seconds"] as? NSNumber)?.doubleValue ?? 0
    report("load", 0, "loading \(what)")
    let engine = try Ticker.during(interval: 1, Ticker.paced("load", "loading \(what)", took: took, report: report), body)
    let seconds = Date().timeIntervalSince(started)
    report("load", 1, "loaded in \(Int(seconds.rounded())) s")
    if !meta.isEmpty {
      var updated = meta
      updated["load_seconds"] = pythonRound(seconds, 1)
      try? writeSidecar(artifact, updated)
    }
    return (engine, seconds)
  }
}

/// Calls `body(elapsed)` every `interval` seconds on its own thread until stopped.
package final class Ticker: @unchecked Sendable {
  private let condition = NSCondition()
  private var stopped = false

  package init(interval: TimeInterval, _ body: @escaping @Sendable (TimeInterval) -> Void) {
    let started = Date()
    let thread = Thread { [self] in
      while true {
        condition.lock()
        if !stopped {
          _ = condition.wait(until: Date().addingTimeInterval(interval))
        }
        let done = stopped
        condition.unlock()
        if done { return }
        body(Date().timeIntervalSince(started))
      }
    }
    thread.name = "jetlink-progress"
    thread.start()
  }

  package func stop() {
    condition.lock()
    stopped = true
    condition.signal()
    condition.unlock()
  }
}

package func formatBytes(_ n: Int64) -> String {
  n >= 1_000_000_000 ? String(format: "%.1f GB", Double(n) / 1e9) : String(format: "%.0f MB", Double(n) / 1e6)
}

extension Ticker {
  /// Ticks for `stage` paced by how long it took last time, "what, 12 s of
  /// about 40 s", or without a last time "what, 12 s elapsed".
  package static func paced(_ stage: String, _ what: String, took: TimeInterval, report: @escaping ProgressFn) -> @Sendable (TimeInterval) -> Void {
    { elapsed in
      if took > 0 {
        report(stage, min(0.95, elapsed / took), "\(what), \(Int(elapsed)) s of about \(Int(took.rounded())) s")
      } else {
        report(stage, 0, "\(what), \(Int(elapsed)) s elapsed")
      }
    }
  }

  /// Runs `body` while ticking every `interval`, and stops the ticks however
  /// it ends.
  package static func during<T>(interval: TimeInterval, _ tick: @escaping @Sendable (TimeInterval) -> Void, _ body: () throws -> T) rethrows -> T {
    let ticker = Ticker(interval: interval, tick)
    defer { ticker.stop() }
    return try body()
  }
}
