import Foundation

/// The checkout the tests read their fixtures from in place: found from the
/// test's own source path, or $JETLINK_TEST_ROOT when the tests run where the
/// sources are not, as on an Android emulator (android/README.md).
public enum SourceTree {
  /// The repository root.
  public static func root(from sourceFile: String = #filePath) -> URL {
    if let root = ProcessInfo.processInfo.environment["JETLINK_TEST_ROOT"], !root.isEmpty {
      return URL(fileURLWithPath: root, isDirectory: true)
    }
    var url = URL(fileURLWithPath: sourceFile).deletingLastPathComponent()
    while url.lastPathComponent != "JetlinkKit" && url.pathComponents.count > 1 {
      url.deleteLastPathComponent()
    }
    return url.deletingLastPathComponent()
  }
}
