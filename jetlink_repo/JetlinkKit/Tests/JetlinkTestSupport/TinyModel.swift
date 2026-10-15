import Foundation

/// The tiny graphs the server's tests serve, and the specs Python derived
/// for them, read in place.
public enum TinyModel {
  public static let fixtures = SourceTree.root().appending(path: "JetlinkKit/Tests/JetlinkServerTests/Fixtures", directoryHint: .isDirectory)
  public static let queued = fixtures.appending(path: "tiny_queued.onnx")
  public static let stateful = fixtures.appending(path: "tiny_stateful.onnx")

  /// `model`'s spec, as its `.spec.json` beside it has it.
  public static func spec(_ model: URL) throws -> [String: Any] {
    let data = try Data(contentsOf: model.deletingPathExtension().appendingPathExtension("spec.json"))
    guard let spec = try JSONSerialization.jsonObject(with: data) as? [String: Any] else {
      throw TestError("\(model.lastPathComponent)'s spec is not an object")
    }
    return spec
  }

  public static func sha256(_ model: URL) throws -> String {
    guard let sha = try spec(model)["sha256"] as? String else { throw TestError("\(model.lastPathComponent)'s spec names no sha256") }
    return sha
  }
}
