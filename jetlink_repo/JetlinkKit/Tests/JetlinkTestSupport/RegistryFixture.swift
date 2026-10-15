import Foundation
import JetlinkRegistry

/// The registry's fixtures, the Python suite's own: <repo>/tests/fixtures,
/// and the routes a MockNet answers them on.
public enum RegistryFixture {
  public static let directory = SourceTree.root().appending(path: "tests/fixtures")
  /// The catalog fixture's big model: its ref, and its pointer's object.
  public static let ref = "f877d7a0ccc3cce943c76e285214c020cd65c899"
  public static let oid = "a086d5249fc308bb73993d1e64630c669d4c7df5bde85f42ad61902543648525"
  public static let size: Int64 = 765_953_504
  public static let newestRef = "37bfa1413edcdc2e8844984b83727c33f81d8f46"
  /// A ref whose pointer names `blob`.
  public static let smallRef = String(repeating: "a", count: 40)
  public static let blob = Data(String(repeating: "onnx", count: 1024).utf8)
  public static let blobSHA = "9cf0cb741f1509847a90129514b5de3f9fff85e2e40de72369f516e9d7ed5b04"

  /// A fixture's bytes; none when it is missing, which the test then fails on.
  public static func data(_ name: String) -> Data {
    (try? Data(contentsOf: directory.appending(path: name))) ?? Data()
  }

  /// The catalog fixture at `Catalog.url`, and `extra`.
  public static func catalogRoutes(_ extra: [String: MockNet.Reply] = [:]) -> [String: MockNet.Reply] {
    var routes: [String: MockNet.Reply] = [Catalog.url: .body(data("catalog_chestnut_v25.json"))]
    routes.merge(extra) { _, new in new }
    return routes
  }

  public static func pointerText(oid: String = blobSHA, size: Int64 = Int64(blob.count)) -> Data {
    Data("version https://git-lfs.github.com/spec/v1\noid sha256:\(oid)\nsize \(size)\n".utf8)
  }

  /// A batch response fixture, retargeted at `oid` and served from `href`;
  /// the one that has no object without an `href`.
  public static func batch(oid: String, size: Int64, href: String?) -> Data {
    guard
      var payload = (try? JSONSerialization.jsonObject(with: data(href == nil ? "lfs_batch_missing.json" : "lfs_batch_response.json"))) as? [String: Any],
      var objects = payload["objects"] as? [[String: Any]], !objects.isEmpty
    else { return Data() }
    objects[0]["oid"] = oid
    objects[0]["size"] = size
    if let href {
      objects[0]["actions"] = ["download": ["href": href, "header": [String: String]()]]
    }
    payload["objects"] = objects
    return (try? JSONSerialization.data(withJSONObject: payload)) ?? Data()
  }

  /// `smallRef` points at `body` under `oid`; the first LFS server has
  /// nothing, the second serves it from `href`, which a local server may
  /// answer itself.
  public static func smallRoutes(
    _ body: Data = blob, oid: String = blobSHA, size: Int64? = nil, href: String = "https://blob.example/object"
  ) -> [String: MockNet.Reply] {
    let size = size ?? Int64(body.count)
    var routes: [String: MockNet.Reply] = [
      LFS.pointerURL(ref: smallRef): .body(pointerText(oid: oid, size: size)),
      "\(LFS.endpoints[0])/objects/batch": .body(batch(oid: oid, size: size, href: nil)),
      "\(LFS.endpoints[1])/objects/batch": .body(batch(oid: oid, size: size, href: href)),
    ]
    if !href.hasPrefix("http://127.0.0.1") {
      routes[href] = .body(body)
    }
    return routes
  }
}
