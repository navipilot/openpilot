import Foundation
import JetlinkTestSupport

@testable import JetlinkONNX

/// The fixtures sit next to this file. The test target declares no resources,
/// so they are found through the source path rather than a bundle.
enum Fixtures {
  static let directory = SourceTree.root().appendingPathComponent("JetlinkKit/Tests/JetlinkONNXTests/Fixtures")

  static func url(_ name: String) -> URL {
    directory.appendingPathComponent(name)
  }
}

/// An encoded message's bytes, source ranges resolved.
func flatten(_ e: Encoded, _ src: Source) -> [UInt8] {
  var out: [UInt8] = []
  for piece in e.allPieces {
    switch piece {
    case .bytes(let b): out += b
    case .source(let r): out += Array(src.slice(r))
    case .transposed: fatalError("no transposes in these tests")
    case .widened: fatalError("no widenings in these tests")
    }
  }
  return out
}
