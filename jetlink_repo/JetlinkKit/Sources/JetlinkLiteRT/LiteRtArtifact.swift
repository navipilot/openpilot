import Foundation
import JetlinkKit
import JetlinkServer

/// A LiteRT artifact as LiteRtBackend builds and loads it: a directory
/// holding the converted model, the GPU's compile cache and the model
/// compiled for the NPU, with the sidecar beside it.
enum LiteRtArtifact {
  /// The converted model, as LiteRTPreparation names it.
  static let model = "model.tflite"
  /// The GPU's compile cache, a directory.
  static let cache = "gpu-cache"
  /// LiteRT's copy of the model compiled for the NPU, a directory of its
  /// own layout (litert/core/cache/compilation_cache.cc); empty but for the
  /// NPU's profile.
  static let npuCache = "npu-cache"

  /// Whether `directory`, an artifact or its staging, holds LiteRT's copy
  /// of the model compiled for the NPU.
  static func keptNPUModel(_ directory: URL) -> Bool {
    let cache = directory.appending(path: npuCache, directoryHint: .isDirectory)
    return FileManager.default.enumerator(at: cache, includingPropertiesForKeys: nil)?.contains { ($0 as? URL)?.pathExtension == "tflite" } ?? false
  }

  /// What the GPU files its cache under: the artifact's name, which the
  /// build's staging directory does not have.
  static func cacheKey(_ artifact: URL) -> String {
    artifact.deletingPathExtension().lastPathComponent
  }

  /// A built artifact's sidecar. Throws `ArtifactInvalid`, which rebuilds
  /// it, when the conversion was another version or the model is missing.
  static func open(_ artifact: URL) throws -> [String: Any] {
    try Artifact.open(artifact, version: LiteRtBackend.conversionVersion, unversioned: 0, runtime: "LiteRT", files: [model])
  }

  /// The sidecar keys every build writes; the runtime's release goes under
  /// `litert`, as onnxruntime's goes under `onnxruntime`.
  static func meta(_ backend: LiteRtBackend, model: URL, started: Date) -> [String: Any] {
    var meta = Artifact.meta(backend, runtimeKey: "litert", model: model, started: started)
    meta["prepare"] = LiteRtBackend.conversionVersion
    meta["preparer"] = "swift"
    return meta
  }
}
