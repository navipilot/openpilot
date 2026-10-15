import Foundation

/// The read-only status page the daemon serves to a phone on the comma's
/// hotspot. The page is a resource of this module, in a bundle SwiftPM puts
/// beside the executable (`JetlinkKit_JetlinkStatusPage.resources` on Linux).
public enum StatusPage {
  /// The page's bundle is not beside the executable, as when the binary was
  /// copied without it. The server runs on without the page.
  public struct Unavailable: Error, CustomStringConvertible {
    public let searched: [String]

    public var description: String {
      "status page unavailable: no \(bundleName) with the page in \(searched.joined(separator: " or "))"
    }
  }

  /// The page itself.
  public static func page() throws -> Data {
    try page(searching: bundleDirectories)
  }

  /// Looked up by path rather than through `Bundle.module`, which ends the
  /// process when the bundle is missing.
  static func page(searching directories: [URL]) throws -> Data {
    for directory in directories {
      if let bundle = Bundle(url: directory.appendingPathComponent(bundleName)),
        let url = bundle.url(forResource: "index", withExtension: "html", subdirectory: "Resources")
      {
        return try Data(contentsOf: url)
      }
    }
    throw Unavailable(searched: directories.map(\.path))
  }

  #if canImport(Darwin)
    static let bundleName = "JetlinkKit_JetlinkStatusPage.bundle"
  #else
    static let bundleName = "JetlinkKit_JetlinkStatusPage.resources"
  #endif

  /// Beside the executable, and beside the test bundle, which on a Mac is not
  /// the main bundle under `swift test`.
  static var bundleDirectories: [URL] {
    let main = Bundle.main.bundleURL
    let code = Bundle(for: Marker.self).bundleURL
    return code == main ? [main] : [main, code.deletingLastPathComponent()]
  }

  private final class Marker {}
}
