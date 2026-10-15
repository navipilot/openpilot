import Foundation
import JetlinkServer

/// A backend that is only its names: all a cache, a server and its
/// controller ask of one before any model. TensorRT's, whose plans are
/// files, or onnxruntime's, whose artifacts are directories.
public final class NamingBackend: EngineBackend {
  public let name: String
  public let suffix: String
  public let runtimeVersion: String

  public init(trt: Bool = false) {
    (name, suffix, runtimeVersion) = trt ? ("trt", ".plan", "10.3.0") : ("ort", ".ortcache", "1.29.0")
  }

  public func deviceTag() -> String { name == "trt" ? "Orin-sm87" : "cpu" }

  public func deriveSpec(model: URL, sha256: String, nbytes: Int64, frameSkip: Int) throws -> ModelSpec {
    throw TestError("names only")
  }

  public func build(model: URL, artifact: URL, report: @escaping ProgressFn, metaExtra: [String: Any]) throws {
    throw TestError("names only")
  }

  public func load(artifact: URL, report: @escaping ProgressFn) throws -> any Engine {
    throw TestError("names only")
  }
}

extension EngineHost {
  /// Whether the job in hand ends, ready or failed, within `timeout`.
  public func settles(timeout: TimeInterval = 120) -> Bool {
    eventually(timeout: timeout) { snapshot().state == .ready || snapshot().state == .failed }
  }
}
