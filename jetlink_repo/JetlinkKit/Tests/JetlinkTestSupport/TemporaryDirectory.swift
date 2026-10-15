import Foundation

/// A fresh directory, removed when the test lets go of it.
public final class TemporaryDirectory: @unchecked Sendable {
  public let url: URL

  public init() throws {
    url = FileManager.default.temporaryDirectory.appending(path: "jetlink-tests-\(UUID().uuidString)", directoryHint: .isDirectory)
    try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
  }

  deinit {
    try? FileManager.default.removeItem(at: url)
  }

  public var path: String { url.path }

  /// `text` in a file `name` here.
  @discardableResult
  public func file(_ name: String, _ text: String) throws -> URL {
    let file = url.appending(path: name)
    try Data(text.utf8).write(to: file)
    return file
  }

  /// The names in `sub`, sorted; none when it is missing.
  public func names(_ sub: String = "") -> [String] {
    (try? FileManager.default.contentsOfDirectory(atPath: url.appending(path: sub).path))?.sorted() ?? []
  }
}
