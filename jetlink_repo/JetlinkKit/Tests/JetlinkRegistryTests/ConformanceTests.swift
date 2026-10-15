import Foundation
import JetlinkKit
import JetlinkTestSupport
import Testing

@testable import JetlinkRegistry

/// tests/fixtures/conformance/registry.json, which
/// JetlinkKit/Scripts/make_conformance_fixtures.py writes from the Python
/// registry: pointers, identities, catalog parsing and merging, and the
/// catalog and inventory payloads the Python makes of one cache directory.
extension JSON {
  fileprivate var int64: Int64? {
    if case .int(let value) = self { return value }
    return nil
  }

  fileprivate var bool: Bool? {
    if case .bool(let value) = self { return value }
    return nil
  }
}

struct RegistryConformanceTests {
  private var fixture: JSON { RegistryFixture.json("conformance/registry.json") }

  @Test func pointersParseAsPythonParsesThem() throws {
    let cases = try #require(fixture["pointers"]?.array)
    #expect(cases.count > 10)
    for c in cases {
      let text = try #require(c["text"]?.string)
      let expected = c["expected"]?.object.map { Pointer(oid: $0["oid"]!.string!, size: $0["size"]!.int64!) }
      #expect(LFS.parsePointer(text) == expected, "\(text.prefix(60).debugDescription)")
    }
  }

  @Test func identitiesAreWhatPythonCallsThem() throws {
    for c in try #require(fixture["identities"]?.array) {
      let value = try #require(c["value"]?.string)
      #expect(CacheLayout.isRef(value) == c["is_ref"]?.bool, "\(value)")
      #expect(CacheLayout.isSHA256(value) == c["is_sha256"]?.bool, "\(value)")
    }
  }

  @Test func catalogsParseAsPythonParsesThem() throws {
    let cases = try #require(fixture["parses"]?.array)
    for c in cases {
      let name = c["name"]?.string ?? "?"
      let catalog = c["catalog_file"]?.string.map { RegistryFixture.json($0) } ?? c["catalog"] ?? .null
      let expected = (c["expected"]?.array ?? []).map { m in
        Catalog.Entry(
          name: m["name"]?.string ?? "", shortName: m["short_name"]?.string ?? "", ref: m["ref"]?.string ?? "",
          buildTime: m["build_time"]?.string ?? "", index: Int(m["index"]?.int64 ?? -1))
      }
      #expect(Catalog.parse(catalog) == expected, "\(name)")
    }
  }

  @Test func catalogsMergeAsPythonMergesThem() throws {
    for c in try #require(fixture["merges"]?.array) {
      let catalogs = (c["catalogs"]?.array ?? []).compactMap(\.object)
      let merged = Catalog.merge(catalogs)
      let expected = c["expected"]?.object ?? JSONObject()
      #expect(merged == expected, "\(c["name"]?.string ?? "?")")
    }
  }

  @Test func diffsGiveThePointerPythonFindsInThem() throws {
    let cases = try #require(fixture["diff_pointers"]?.array)
    #expect(cases.count > 10)
    for c in cases {
      let name = c["name"]?.string ?? "?"
      let text = try c["patch_file"]?.string.map { String(decoding: RegistryFixture.data($0), as: UTF8.self) } ?? #require(c["text"]?.string)
      let expected = c["expected"]?.object.map { Pointer(oid: $0["oid"]!.string!, size: $0["size"]!.int64!) }
      #expect(LFS.diffPointer(text) == expected, "\(name)")
    }
  }

  @Test func squashMergesAreReadAsPythonReadsThem() throws {
    for c in try #require(fixture["pull_numbers"]?.array) {
      let subject = try #require(c["subject"]?.string)
      #expect(LFS.pullNumber(in: subject) == c["expected"]?.string, "\(subject)")
    }
  }

  /// The cache directory the Python registry was shown, written again here.
  private func writeTree(_ tree: JSONObject, under root: URL) throws {
    for (relative, content) in tree {
      let url = root.appending(path: relative)
      try FileManager.default.createDirectory(at: url.deletingLastPathComponent(), withIntermediateDirectories: true)
      if let file = content["raw_file"]?.string, var object = content["json"]?.object {
        object["raw"] = RegistryFixture.json(file)
        try JSON.object(object).data().write(to: url)
      } else if let json = content["json"] {
        try json.data().write(to: url)
      } else if let text = content["text"]?.string {
        try Data(text.utf8).write(to: url)
      } else {
        try Data(count: Int(content["bytes"]?.int64 ?? 0)).write(to: url)
      }
    }
  }

  /// The payload as the Python fixture spells it: the root as $ROOT, no free space.
  private func normalized<T: Encodable>(_ value: T, root: URL) throws -> Any {
    let encoder = JSONEncoder()
    encoder.keyEncodingStrategy = .convertToSnakeCase
    encoder.outputFormatting = [.withoutEscapingSlashes]
    var text = String(decoding: try encoder.encode(value), as: UTF8.self)
    for prefix in [root.path, root.resolvingSymlinksInPath().path] {
      text = text.replacingOccurrences(of: prefix, with: "$ROOT")
    }
    var object = try JSONSerialization.jsonObject(with: Data(text.utf8))
    if var dict = object as? [String: Any], var disk = dict["disk"] as? [String: Any] {
      disk["free_bytes"] = 0
      dict["disk"] = disk
      object = dict
    }
    return object
  }

  @Test func oneCacheDirectoryReadsTheSameFromBothSides() throws {
    let raw = try Data(contentsOf: RegistryFixture.directory.appending(path: "conformance/registry.json"))
    let python = try #require((try JSONSerialization.jsonObject(with: raw) as? [String: Any])?["cache"] as? [String: Any])
    let cache = try #require(fixture["cache"]?.object)
    let tmp = try TemporaryDirectory()
    try writeTree(try #require(cache["tree"]?.object), under: tmp.url)
    let registry = Registry(layout: tmp.layout)

    let catalog = try #require(registry.cachedCatalog())
    var comparison = JSONComparison()
    comparison.compare(python: python["catalog"]!, swift: try normalized(catalog, root: tmp.url), at: "catalog")
    let inventory = registry.inventory(
      artifactTag: cache["artifact_tag"]?.string, artifactSuffix: cache["artifact_suffix"]?.string ?? "", loaded: nil)
    comparison.compare(python: python["inventory"]!, swift: try normalized(inventory, root: tmp.url), at: "inventory")
    #expect(comparison.differences.isEmpty, "\(comparison.differences)")
  }
}
