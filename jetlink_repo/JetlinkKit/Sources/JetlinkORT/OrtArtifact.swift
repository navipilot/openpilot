import Foundation
import JetlinkKit
import JetlinkServer

/// An onnxruntime artifact as OrtBackend builds and loads it: a directory
/// holding each session's model and a `sessions.json` manifest naming them,
/// with the sidecar beside it.
enum OrtArtifact {
  static let manifestName = "sessions.json"

  static func writeManifest(_ manifest: [[String: Any]], in directory: URL) throws {
    try JSONSerialization.data(withJSONObject: manifest, options: [.prettyPrinted]).write(to: directory.appending(path: manifestName))
  }

  /// A built artifact's manifest and sidecar. Throws `ArtifactInvalid`,
  /// which rebuilds it, when the manifest is unreadable, the preparation was
  /// another version, a session's model is missing, or `check` says what is
  /// wrong with an entry.
  static func open(
    _ artifact: URL, version expected: Int, check: ([String: Any]) -> String? = { _ in nil }
  ) throws -> (manifest: [[String: Any]], meta: [String: Any]) {
    let name = artifact.lastPathComponent
    guard let data = try? Data(contentsOf: artifact.appending(path: manifestName)),
      let manifest = try? JSONSerialization.jsonObject(with: data) as? [[String: Any]], !manifest.isEmpty
    else {
      throw ArtifactInvalid("\(name): no readable \(manifestName) inside")
    }
    let models = manifest.compactMap { $0["model"] as? String }
    guard models.count == manifest.count else {
      throw ArtifactInvalid("\(name): a session names no model")
    }
    // Artifacts from before the sidecar said are version 1.
    let meta = try Artifact.open(artifact, version: expected, unversioned: 1, runtime: "onnxruntime", files: models)
    if let problem = manifest.lazy.compactMap(check).first {
      throw ArtifactInvalid("\(name): \(problem)")
    }
    return (manifest, meta)
  }

  /// The sidecar keys every build writes, as the Python backend names them.
  static func meta(
    _ backend: OrtBackend, manifest: [[String: Any]], providers: [String], model: URL, started: Date
  ) -> [String: Any] {
    var meta = Artifact.meta(backend, runtimeKey: "onnxruntime", model: model, started: started)
    meta["sessions"] = manifest
    meta["providers"] = providers
    meta["prepare"] = backend.profile.prepareVersion
    meta["preparer"] = "swift"
    return meta
  }
}
