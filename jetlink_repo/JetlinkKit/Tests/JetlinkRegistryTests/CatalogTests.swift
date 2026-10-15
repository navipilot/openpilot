import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkRegistry

/// tests/test_registry.py's catalog section: parsing, merging, probing and the payload.
struct CatalogParseTests {
  private var catalog: JSON { RegistryFixture.json("catalog_chestnut_v25.json") }

  private func mutated(_ change: (inout JSONObject) -> Void) -> JSON {
    guard var data = catalog.object, var bundles = data["bundles"]?.array, var first = bundles[0].object else { return .null }
    change(&first)
    bundles[0] = .object(first)
    data["bundles"] = .array(bundles)
    return .object(data)
  }

  @Test func keepsTheBigModelsNewestFirst() {
    let models = Catalog.parse(catalog)
    #expect(models.count == 13)
    #expect(models[0].ref == RegistryFixture.newestRef)
    #expect(models[0].shortName == "CTMV2")
    #expect(models.map(\.index) == models.map(\.index).sorted(by: >))
  }

  @Test(arguments: [
    ("minimum_selector_version", JSON.string("18")),
    ("ref", JSON.string("not-a-commit")),
    ("is_big", JSON.bool(false)),
  ])
  func dropsABundleItCannotUse(key: String, value: JSON) {
    let models = Catalog.parse(mutated { $0[key] = value })
    #expect(models.count == 12)
    #expect(models.allSatisfy { $0.ref != "fa0c6876d3cf070e91e25e5353ceadc68a5b3285" })
  }

  @Test func survivesRubbish() {
    #expect(Catalog.parse([:]).isEmpty)
    #expect(Catalog.parse(["bundles": "nope"]).isEmpty)
    #expect(Catalog.parse(["bundles": [nil, 5, ["ref": .string(RegistryFixture.ref), "minimum_selector_version": "x", "is_big": true]]]).isEmpty)
  }

  @Test func aDuplicateRefIsListedOnce() {
    guard var data = catalog.object, var bundles = data["bundles"]?.array, var copy = bundles[0].object else {
      Issue.record("fixture")
      return
    }
    copy["display_name"] = "A copy"
    copy["index"] = 99
    bundles.append(.object(copy))
    data["bundles"] = .array(bundles)
    let models = Catalog.parse(.object(data))
    #expect(models.count == 13)
    #expect(models.allSatisfy { $0.name != "A copy" })
  }

  @Test func equalIndexesKeepCatalogOrder() {
    let a = String(repeating: "a", count: 40)
    let b = String(repeating: "b", count: 40)
    let data: JSON = ["bundles": [bundle(a, 5), bundle(b, 5)]]
    #expect(Catalog.parse(data).map(\.ref) == [a, b])
  }

  @Test func pythonIntRules() {
    // index as a numeric string is int()-able; a float truncates; null drops the bundle
    let a = String(repeating: "a", count: 40)
    var withString = bundle(a, 0).object!
    withString["index"] = " 7 "
    #expect(Catalog.parse(["bundles": [.object(withString)]]).first?.index == 7)
    var withNull = withString
    withNull["index"] = nil
    #expect(Catalog.parse(["bundles": [.object(withNull)]]).first?.index == 0, "a missing index is 0")
    withNull["index"] = .null
    #expect(Catalog.parse(["bundles": [.object(withNull)]]).isEmpty, "int(None) drops the bundle")
    var selector = bundle(a, 1).object!
    selector["minimum_selector_version"] = 19
    #expect(Catalog.parse(["bundles": [.object(selector)]]).count == 1, "the selector may be a number")
  }

  @Test func theCatalogVersionComesFromTheURL() {
    #expect(Catalog.version(of: Catalog.url) == 26)
    #expect(Catalog.version(of: "https://example.com/catalog.json") == nil)
    #expect(Catalog.url == "https://raw.githubusercontent.com/sunnypilot/sunnypilot-models/refs/heads/gh-pages/docs/driving_models_chestnut_v26.json")
    #expect(Catalog.requiredSelectorVersion == 19)
    #expect(Catalog.probeLimit == 10)
    #expect(Catalog.defaultBigModelRef == "bf3e3631b3f91d92a1020a5e0dd4298b93ff4244")
  }
}

func bundle(_ ref: String, _ index: Int64, selector: String = "19", name: String = "") -> JSON {
  [
    "ref": .string(ref), "index": .int(index), "minimum_selector_version": .string(selector), "is_big": true,
    "display_name": .string(name.isEmpty ? String(ref.prefix(6)) : name), "short_name": .string(String(name.prefix(4))),
    "generation": "12", "environment": "development", "runner": "tinygrad", "build_time": "2026-09-25T00:00:00Z",
    "overrides": ["folder": "Master Models"],
    "models": [["type": "chunked", "artifact": ["file_name": .string("\(ref.prefix(6)).pkl")]]],
  ]
}

/// A model sunnypilot publishes after this release is still listed.
struct NewerCatalogTests {
  @Test func versionsAreProbedUpToTheFirstMissingOne() async throws {
    let v = Catalog.version
    let net = MockNet([
      catalogURL(v + 1): .body(Data(#"{"bundles": []}"#.utf8)),
      catalogURL(v + 2): .body(Data(#"{"bundles": []}"#.utf8)),
      catalogURL(v + 3): .status(404),
    ])
    let tmp = try TemporaryDirectory()
    let found = try await Registry(layout: tmp.layout, session: net.session).newerCatalogs(after: Catalog.url)
    #expect(found.count == 2)
    #expect(net.urls == [catalogURL(v + 1), catalogURL(v + 2), catalogURL(v + 3)])
  }

  @Test func anOutagePastThePinIsAFailureNotAShortList() async throws {
    let net = MockNet([
      catalogURL(Catalog.version + 1): .body(Data(#"{"bundles": []}"#.utf8)),
      catalogURL(Catalog.version + 2): .failure,
    ])
    let tmp = try TemporaryDirectory()
    await #expect(throws: RegistryError.self) {
      try await Registry(layout: tmp.layout, session: net.session).newerCatalogs(after: Catalog.url)
    }
  }

  @Test func theMergeKeepsBuildsAtOurVersionAndAddsTheRestForAnAccelerator() {
    let a = String(repeating: "a", count: 40)
    let b = String(repeating: "b", count: 40)
    let c = String(repeating: "c", count: 40)
    let pinned: JSONObject = ["tinygrad_ref": "pinned", "bundles": [bundle(a, 1), bundle(b, 2)]]
    let nextRuntime: JSONObject = [
      "tinygrad_ref": "next", "bundles": [bundle(a, 1, selector: "20"), bundle(b, 2, selector: "20"), bundle(c, 3, selector: "20", name: "Old name")],
    ]
    let newest: JSONObject = ["tinygrad_ref": "newer", "bundles": [bundle(c, 3, selector: "20", name: "Cinque Terre V4")]]

    let merged = Catalog.merge([pinned, nextRuntime, newest])

    #expect(merged["tinygrad_ref"] == "pinned")
    let byRef = Dictionary(uniqueKeysWithValues: Catalog.bundles(.object(merged)).map { ($0["ref"]!.string!, $0) })
    #expect(byRef[a] == bundle(a, 1).object)
    #expect(byRef[b] == bundle(b, 2).object)
    #expect(byRef[c]?["display_name"] == "Cinque Terre V4")
    #expect(byRef[c]?["minimum_selector_version"] == "19")
    #expect(byRef[c]?["models"] == .array([]))
    #expect(byRef[c]?["overrides"] == ["folder": "Master Models"])
    #expect(Catalog.parse(.object(merged)).map(\.ref) == [c, b, a])
    #expect(Catalog.merge([]).isEmpty)
  }

  @Test func theRegistryListsAModelOnlyANewerCatalogHas() async throws {
    let fresh = String(repeating: "e", count: 40)
    let newer: JSON = ["bundles": [bundle(fresh, 99, selector: "20", name: "Cinque Terre V4")]]
    let net = MockNet(RegistryFixture.catalogRoutes([catalogURL(Catalog.version + 1): .body(newer.data())]))
    let tmp = try TemporaryDirectory()
    let models = await Registry(layout: tmp.layout, session: net.session).catalog().models
    #expect(models.first?.ref == fresh)
    #expect(models.first?.name == "Cinque Terre V4")
    #expect(models.count == 14)
  }
}

/// zoompilot's own list, after sunnypilot's: catalog/extra_big_models.json.
struct ExtraModelTests {
  @Test func anExtraModelIsListedAfterSunnypilots() async throws {
    let preview = String(repeating: "b", count: 40)
    let extra: JSON = ["bundles": [bundle(preview, 99, name: "A preview")]]
    let net = MockNet(RegistryFixture.catalogRoutes([Catalog.extraURL: .body(extra.data())]))
    let tmp = try TemporaryDirectory()
    let models = await Registry(layout: tmp.layout, session: net.session).catalog().models
    #expect(models.count == 14)
    #expect(models.first?.ref == preview && models.first?.name == "A preview")
    #expect(net.urls.last == Catalog.extraURL)
  }

  @Test func sunnypilotsEntryForTheSameCommitWins() async throws {
    let extra: JSON = ["bundles": [bundle(RegistryFixture.ref, 99, name: "Our name")]]
    let net = MockNet(RegistryFixture.catalogRoutes([Catalog.extraURL: .body(extra.data())]))
    let tmp = try TemporaryDirectory()
    let models = await Registry(layout: tmp.layout, session: net.session).catalog().models
    #expect(models.count == 13)
    #expect(!models.contains { $0.name == "Our name" })
  }

  @Test func anOutageOfTheListKeepsThePreviousOne() async throws {
    let tmp = try TemporaryDirectory()
    _ = await Registry(layout: tmp.layout, session: MockNet(RegistryFixture.catalogRoutes()).session).catalog()
    let down = MockNet(RegistryFixture.catalogRoutes([Catalog.extraURL: .failure]))
    let payload = await Registry(layout: tmp.layout, session: down.session).catalog(refresh: true)
    #expect(payload.error?.contains(Catalog.extraURL) == true)
    #expect(payload.models.count == 13)
  }

  @Test func theListIsOnMain() {
    #expect(Catalog.extraURL == "https://raw.githubusercontent.com/zoompilot/jetlink/refs/heads/main/catalog/extra_big_models.json")
  }
}

struct CatalogPayloadTests {
  @Test func isCachedUntilItGoesStale() async throws {
    let net = MockNet(RegistryFixture.catalogRoutes())
    let tmp = try TemporaryDirectory()
    let registry = Registry(layout: tmp.layout, session: net.session)

    let payload = await registry.catalog()
    #expect(payload.models.count == 13)
    #expect(payload.error == nil)
    #expect(payload.fetchedAt != nil)
    #expect(payload.defaultRef == "bf3e3631b3f91d92a1020a5e0dd4298b93ff4244")
    #expect(payload.url == Catalog.url)

    _ = await registry.catalog()
    #expect(net.count(Catalog.url) == 1, "a fresh cache must not go to the network")

    _ = await registry.catalog(maxAge: -1)
    #expect(net.count(Catalog.url) == 2)

    _ = await registry.catalog(refresh: true)
    #expect(net.count(Catalog.url) == 3)
  }

  @Test func aFailedRefreshKeepsThePreviousList() async throws {
    let tmp = try TemporaryDirectory()
    _ = await Registry(layout: tmp.layout, session: MockNet(RegistryFixture.catalogRoutes()).session).catalog()

    let payload = await Registry(layout: tmp.layout, session: MockNet().session).catalog(refresh: true)

    #expect(payload.error?.contains(Catalog.url) == true)
    #expect(payload.models.count == 13)
    #expect(payload.fetchedAt != nil)
  }

  @Test func anEmptyCacheAndNoNetworkIsAnEmptyList() async throws {
    let tmp = try TemporaryDirectory()
    let payload = await Registry(layout: tmp.layout, session: MockNet().session).catalog()
    #expect(payload.models.isEmpty)
    #expect(payload.fetchedAt == nil)
    #expect(payload.error?.isEmpty == false)
  }

  @Test func aCatalogThatIsNotAnObjectIsANetworkError() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet([Catalog.url: .body(Data("[1, 2]".utf8))])
    let payload = await Registry(layout: tmp.layout, session: net.session).catalog()
    #expect(payload.error == "\(Catalog.url) did not serve a JSON dict")
    let broken = MockNet([Catalog.url: .body(Data("<html>".utf8))])
    #expect(await Registry(layout: tmp.layout, session: broken.session).catalog().error?.contains("did not serve JSON") == true)
  }

  @Test func theCachedCatalogNeverTouchesTheNetwork() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet(RegistryFixture.catalogRoutes())
    let registry = Registry(layout: tmp.layout, session: net.session)
    #expect(registry.cachedCatalog() == nil)
    #expect(net.calls.isEmpty)

    let fetched = await registry.catalog()
    let calls = net.calls.count
    let cached = registry.cachedCatalog()
    #expect(cached == fetched)
    #expect(net.calls.count == calls)

    let empty = Registry.emptyCatalog(error: "offline")
    #expect(empty.models.isEmpty && empty.fetchedAt == nil && empty.error == "offline")
    #expect(empty.url == Catalog.url && empty.defaultRef == Catalog.defaultBigModelRef)
  }

  @Test func theCatalogCarriesAResolvedPointer() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet(RegistryFixture.catalogRoutes([LFS.pointerURL(ref: RegistryFixture.ref): .body(RegistryFixture.data("pointer_f877d7a0.txt"))]))
    let registry = Registry(layout: tmp.layout, session: net.session)
    _ = try await registry.resolve(ref: RegistryFixture.ref)
    let payload = await registry.catalog()
    let entry = payload.models.first { $0.ref == RegistryFixture.ref }
    #expect(entry?.sha256 == RegistryFixture.oid)
    #expect(entry?.bytes == RegistryFixture.size)
    #expect(payload.models.first { $0.ref == RegistryFixture.newestRef }?.sha256 == nil)
  }

  @Test func nameForFindsTheCatalogName() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet(RegistryFixture.catalogRoutes([LFS.pointerURL(ref: RegistryFixture.ref): .body(RegistryFixture.data("pointer_f877d7a0.txt"))]))
    let registry = Registry(layout: tmp.layout, session: net.session)
    _ = await registry.catalog()
    _ = try await registry.resolve(ref: RegistryFixture.ref)
    #expect(registry.name(for: RegistryFixture.oid) == ("BMRLNAP Model v4 (August 30, 2026)", RegistryFixture.ref))
    #expect(registry.name(for: String(repeating: "b", count: 64)) == (nil, nil))
    #expect(registry.ref(for: RegistryFixture.oid) == RegistryFixture.ref)
    #expect(registry.ref(for: String(repeating: "b", count: 64)) == nil)
  }

  @Test func stateFilesAreWrittenAtomically() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet(RegistryFixture.catalogRoutes([LFS.pointerURL(ref: RegistryFixture.ref): .body(RegistryFixture.data("pointer_f877d7a0.txt"))]))
    let registry = Registry(layout: tmp.layout, session: net.session)
    _ = await registry.catalog()
    _ = try await registry.resolve(ref: RegistryFixture.ref)
    #expect(tmp.names("registry") == ["catalog.json", "pointers.json"])
    let cached = Files.readJSON(tmp.layout.catalogURL)
    #expect(cached?["url"]?.string == Catalog.url)
    let fetchedAt = try #require(cached?["fetched_at"]?.pythonNumber)
    #expect(Date().timeIntervalSince1970 - fetchedAt < 60)
  }
}
