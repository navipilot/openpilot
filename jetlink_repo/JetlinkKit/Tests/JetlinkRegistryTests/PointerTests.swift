import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkRegistry

/// tests/test_registry.py's pointers, lfs and precompiled-pkl sections.
struct PointerTests {
  @Test func parsesTheFixture() {
    #expect(
      LFS.parsePointer(String(decoding: RegistryFixture.data("pointer_f877d7a0.txt"), as: UTF8.self))
        == Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size))
  }

  @Test(arguments: [
    "not a pointer at all",
    "oid sha256:\(RegistryFixture.oid)\n",  // no size
    "oid sha256:nothex\nsize 12\n",
    "oid sha256:\(RegistryFixture.oid)\nsize twelve\n",
    String(repeating: "x", count: 5000),  // an onnx served where a pointer was expected
    "oid sha256:\(RegistryFixture.oid)\nsize 0\n",
  ])
  func refusesAnythingElse(text: String) {
    #expect(LFS.parsePointer(text) == nil)
  }

  @Test func readsAPointerWithOtherLineBreaks() {
    #expect(
      LFS.parsePointer("version x\r\noid sha256:\(RegistryFixture.oid)\r\nsize 765_953_504\r\n")
        == Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size))
  }

  @Test func resolveFetchesAPointerOnceAndKeepsIt() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet([LFS.pointerURL(ref: RegistryFixture.ref): .body(RegistryFixture.data("pointer_f877d7a0.txt"))])
    #expect(
      try await Registry(layout: tmp.layout, session: net.session).resolve(ref: RegistryFixture.ref)
        == Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size))
    #expect(
      try await Registry(layout: tmp.layout, session: net.session).resolve(ref: RegistryFixture.ref)
        == Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size))
    #expect(net.calls.count == 1)
  }

  @Test func resolveRefusesSomethingThatIsNotACommit() async throws {
    let tmp = try TemporaryDirectory()
    let error = await #expect(throws: RegistryError.self) {
      try await Registry(layout: tmp.layout, session: MockNet().session).resolve(ref: "nothex")
    }
    #expect(error?.message == "'nothex' is not a 40 character commit")
  }

  @Test func aPointerHostThatServesTheWholeModelIsCutOff() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet([LFS.pointerURL(ref: RegistryFixture.ref): .chunks(Array(repeating: Data(count: 64 << 10), count: 32))])
    let error = await #expect(throws: RegistryError.self) {
      try await Registry(layout: tmp.layout, session: net.session).resolve(ref: RegistryFixture.ref)
    }
    #expect(error?.message == "f877d7a0cc did not serve an lfs pointer")
  }

  @Test func resolveMissingReportsTheFailuresAndKeepsTheRest() async throws {
    let tmp = try TemporaryDirectory()
    let (good, bad) = (RegistryFixture.newestRef, RegistryFixture.ref)
    let net = MockNet([LFS.pointerURL(ref: good): .body(RegistryFixture.data("pointer_f877d7a0.txt"))])
    let registry = Registry(layout: tmp.layout, session: net.session)

    let out = await registry.resolveMissing([good, bad, "nothex", good])

    #expect(try out[good]?.get() == Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size))
    #expect(throws: RegistryError.self) { try out[bad]?.get() }
    #expect(throws: RegistryError.self) { try out["nothex"]?.get() }
    let written = Files.readJSON(tmp.layout.pointersURL)
    #expect(written == [good: ["oid": .string(RegistryFixture.oid), "size": .int(RegistryFixture.size)]])

    // a second pass asks only for what is still missing
    let before = net.calls.count
    let again = await registry.resolveMissing([good, bad])
    #expect(try again[good]?.get() == Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size))
    #expect(net.calls.count == before + 1)
  }

  @Test func resolveMissingRunsEightAtATimeAndKeepsEveryPointer() async throws {
    let tmp = try TemporaryDirectory()
    let refs = (0..<20).map { String(format: "%040x", $0 + 1) }
    var routes: [String: MockNet.Reply] = [:]
    for (i, ref) in refs.enumerated() {
      routes[LFS.pointerURL(ref: ref)] = .body(RegistryFixture.pointerText(oid: String(format: "%064x", i + 1), size: Int64(i + 1)))
    }
    let net = MockNet(routes)
    let out = await Registry(layout: tmp.layout, session: net.session).resolveMissing(refs)
    #expect(out.count == 20)
    #expect(out.values.allSatisfy { (try? $0.get()) != nil })
    // pointers.json keeps the order the refs were asked in, as the Python dict does
    #expect(Files.readJSON(tmp.layout.pointersURL)?.object?.keys == refs)
  }

  @Test func concurrentResolvesDoNotLoseEachOthersPointers() async throws {
    let tmp = try TemporaryDirectory()
    let refs = (0..<24).map { String(format: "%040x", $0 + 100) }
    var routes: [String: MockNet.Reply] = [:]
    for (i, ref) in refs.enumerated() {
      routes[LFS.pointerURL(ref: ref)] = .body(RegistryFixture.pointerText(oid: String(format: "%064x", i + 1), size: 10))
    }
    let net = MockNet(routes)
    let registry = Registry(layout: tmp.layout, session: net.session)
    try await withThrowingTaskGroup(of: Void.self) { group in
      for ref in refs {
        group.addTask { _ = try await registry.resolve(ref: ref) }
      }
      try await group.waitForAll()
    }
    #expect(Set(Files.readJSON(tmp.layout.pointersURL)?.object?.keys ?? []) == Set(refs))
  }
}

struct LFSBatchTests {
  private let pointer = Pointer(oid: RegistryFixture.oid, size: RegistryFixture.size)

  @Test func returnsTheHref() async {
    let net = MockNet(["\(LFS.endpoints[0])/objects/batch": .body(RegistryFixture.data("lfs_batch_response.json"))])
    let href = await LFS.resolve(endpoint: LFS.endpoints[0], pointer: pointer, session: net.session)
    #expect(href?.hasPrefix("https://gitlab.com/commaai/openpilot-lfs.git/gitlab-lfs/objects/") == true)

    let call = net.calls.first
    #expect(call?.method == "POST")
    let body = call?.body.flatMap { try? JSON.parse($0) }
    #expect(body == ["operation": "download", "transfers": ["basic"], "objects": [["oid": .string(RegistryFixture.oid), "size": .int(RegistryFixture.size)]]])
  }

  @Test func isNilWhenTheServerLacksItOrIsDown() async {
    let missing = MockNet(["\(LFS.endpoints[0])/objects/batch": .body(RegistryFixture.data("lfs_batch_missing.json"))])
    #expect(await LFS.resolve(endpoint: LFS.endpoints[0], pointer: pointer, session: missing.session) == nil)
    #expect(await LFS.resolve(endpoint: LFS.endpoints[0], pointer: pointer, session: MockNet().session) == nil)
    let failing = MockNet(["\(LFS.endpoints[0])/objects/batch": .status(500)])
    #expect(await LFS.resolve(endpoint: LFS.endpoints[0], pointer: pointer, session: failing.session) == nil)
  }

  @Test func theEndpointsAreAskedInPythonsOrder() {
    #expect(
      LFS.endpoints == [
        "https://gitlab.com/commaai/openpilot-lfs.git/info/lfs",
        "https://huggingface.co/commaai/openpilot-lfs.git/info/lfs",
        "https://huggingface.co/commaai/openpilot_driving_models.git/info/lfs",
      ])
    #expect(
      LFS.pointerURL(ref: RegistryFixture.ref)
        == "https://raw.githubusercontent.com/commaai/openpilot/\(RegistryFixture.ref)/openpilot/selfdrive/modeld/models/big_driving_supercombo.onnx")
    #expect(LFS.commitPatchURL(ref: RegistryFixture.ref) == "https://github.com/commaai/openpilot/commit/\(RegistryFixture.ref).patch")
    #expect(LFS.drivingModelsTreeURL == "https://huggingface.co/api/models/commaai/openpilot_driving_models/tree/main")
  }
}

// MARK: - a commit that ships a precompiled pkl
// Cinque Terre V3, as github and huggingface served it on 2026-09-25.

let v3Ref = "bf3e3631b3f91d92a1020a5e0dd4298b93ff4244"
let v3OID = "404a18cfd86d29637d20c697dfde245bb47c666ae016730ab674c65f4d1e1aa4"
let v3Size: Int64 = 766_354_845
let v3Folder = "f78ed37d-afad-4dbc-8050-40ea885eedde"

func patchHead(_ subject: String) -> Data {
  Data(
    ("From \(v3Ref) Mon Sep 17 00:00:00 2001\nFrom: Bruce Wayne <x@example.com>\n"
      + "Date: Tue, 15 Sep 2026 23:30:35 -0700\nSubject: [PATCH] \(subject)\n\n---\n"
      + " openpilot/selfdrive/modeld/models/big_driving_tinygrad.pkl | 2 +-\n").utf8)
}

func onnxEntry(_ path: String, oid: String = v3OID, size: Int64 = v3Size) -> JSON {
  [
    "type": "file", "path": .string("\(path)/big_driving_supercombo.onnx"), "size": .int(size),
    "lfs": ["oid": .string(oid), "size": .int(size), "pointerSize": 134],
  ]
}

func exportRoutes(subject: String = "Use f78ed37d for the precompiled eGPU driving model", folderFiles: [JSON]? = nil) -> [String: MockNet.Reply] {
  let top: JSON = [
    ["type": "directory", "path": "1a421175-db71-4e3d-9d62-e2166421b02b"],
    ["type": "directory", "path": .string(v3Folder)],
    ["type": "file", "path": "README.md", "size": 21],
  ]
  let files = JSON.array(folderFiles ?? [["type": "directory", "path": .string("\(v3Folder)/12864")], onnxEntry("\(v3Folder)/12864")])
  return [
    LFS.pointerURL(ref: v3Ref): .status(404),
    LFS.commitPatchURL(ref: v3Ref): .body(patchHead(subject)),
    LFS.drivingModelsTreeURL: .body(top.data()),
    "\(LFS.drivingModelsTreeURL)/\(v3Folder)?recursive=true": .body(files.data()),
  ]
}

struct ExportPointerTests {
  @Test func aCommitWithoutTheONNXResolvesToTheExportItsSubjectNames() async throws {
    let tmp = try TemporaryDirectory()
    let net = MockNet(exportRoutes())
    #expect(try await Registry(layout: tmp.layout, session: net.session).resolve(ref: v3Ref) == Pointer(oid: v3OID, size: v3Size))
    // kept for good like any other pointer
    #expect(try await Registry(layout: tmp.layout, session: MockNet().session).resolve(ref: v3Ref) == Pointer(oid: v3OID, size: v3Size))
  }

  @Test func theExportRepoIsTheLastLFSServerAsked() {
    #expect(LFS.endpoints.last == "https://huggingface.co/commaai/openpilot_driving_models.git/info/lfs")
  }

  @Test func aSubjectThatNamesTheCheckpointPicksAmongSeveral() async throws {
    let files = [onnxEntry("\(v3Folder)/12000", oid: String(repeating: "1", count: 64)), onnxEntry("\(v3Folder)/12864")]
    let named = MockNet(exportRoutes(subject: "\(v3Folder)/12864", folderFiles: files))
    #expect(try await LFS.fetchPointer(ref: v3Ref, http: HTTP(session: named.session)) == Pointer(oid: v3OID, size: v3Size))

    let unnamed = MockNet(exportRoutes(folderFiles: files))
    let error = await #expect(throws: RegistryError.self) {
      try await LFS.fetchPointer(ref: v3Ref, http: HTTP(session: unnamed.session))
    }
    #expect(error?.message.contains("2 copies") == true)
  }

  @Test func aFoldedSubjectIsReadWholeAndTheDiffstatIsNot() async throws {
    let head = patchHead("Use f78ed37d for the precompiled eGPU\n driving model")
    let net = MockNet([LFS.commitPatchURL(ref: v3Ref): .body(head)])
    #expect(try await LFS.commitSubject(ref: v3Ref, http: HTTP(session: net.session)) == "Use f78ed37d for the precompiled eGPU driving model")
  }

  @Test(arguments: [
    ("Update tinygrad and use retargetable model artifacts (#38933)", "names no export"),
    ("Use 0badc0de for the precompiled eGPU driving model", "no folder"),
  ])
  func aSubjectThatLeadsNowhereSaysSo(subject: String, match: String) async throws {
    var routes = exportRoutes(subject: subject)
    // the pull request a squash merge names carries no model either
    routes[LFS.pullPatchURL(number: "38933")] = .body(patchHead("Update tinygrad and use retargetable model artifacts"))
    let net = MockNet(routes)
    let error = await #expect(throws: RegistryError.self) {
      try await LFS.fetchPointer(ref: v3Ref, http: HTTP(session: net.session))
    }
    #expect(error?.message.contains(match) == true)
    #expect(error?.kind == .registry)
  }

  @Test func onlyAMissingFileFallsBackAndAnOutageDoesNot() async throws {
    var routes = exportRoutes()
    routes[LFS.pointerURL(ref: v3Ref)] = .status(503)
    let net = MockNet(routes)
    let error = await #expect(throws: RegistryError.self) {
      try await LFS.fetchPointer(ref: v3Ref, http: HTTP(session: net.session))
    }
    #expect(error?.isNetwork == true)
    #expect(net.urls == [LFS.pointerURL(ref: v3Ref)])
  }

  @Test func exportIDsAreWholeHexWords() {
    #expect(LFS.exportIDs(in: "Use f78ed37d for the precompiled eGPU driving model") == ["f78ed37d"])
    #expect(LFS.exportIDs(in: "1a421175-db71-4e3d-9d62-e2166421b02b/12864 and 1a421175 again") == ["1a421175"])
    #expect(LFS.exportIDs(in: "deadbeefcafe f78ED37d _f78ed37d f78ed37d_") == [])
    #expect(LFS.stripPatchTag("[PATCH 2/3]   Use it") == "Use it")
    #expect(LFS.pythonQuote("a b/c?d") == "a%20b/c%3Fd")
  }
}

// MARK: - a commit that compiles the ONNX away
// ResAction (#39037), as github served it on 2026-10-05: 2fb4ac4aa0 adds the
// ONNX, 219f4e7ba3 ("compiled") swaps it for the pkl.

let resCompiled = "219f4e7ba38cda8788cc31f2d0a5ca49d9566231"
let resPointer = Pointer(oid: "1563b85f6bd00d9e2edf50bdb69646f0d95d5ac0f218d5abe3fe54a13427d71b", size: 792_485_215)
let squash = String(repeating: "f", count: 40)

func compiledRoutes(_ ref: String, _ patch: Data) -> [String: MockNet.Reply] {
  [LFS.pointerURL(ref: ref): .status(404), LFS.commitPatchURL(ref: ref): .body(patch)]
}

struct CompiledCommitTests {
  @Test func aCommitThatCompilesTheONNXAwayResolvesToTheOneItDeleted() async throws {
    let net = MockNet(compiledRoutes(resCompiled, RegistryFixture.data("patch_219f4e7b.patch")))
    #expect(try await LFS.fetchPointer(ref: resCompiled, http: HTTP(session: net.session)) == resPointer)
    #expect(!net.urls.contains(LFS.drivingModelsTreeURL), "'compiled' names no export to look up")
  }

  @Test func aSquashMergeResolvesToTheLastONNXItsPullRequestCarried() async throws {
    var routes = compiledRoutes(squash, patchHead("ResAction (#39037)"))
    routes[LFS.pullPatchURL(number: "39037")] = .body(RegistryFixture.data("pull_39037.patch"))
    #expect(try await LFS.fetchPointer(ref: squash, http: HTTP(session: MockNet(routes).session)) == resPointer)
  }

  @Test func aSquashMergeOfAPrecompiledPullRequestFollowsTheExportItNamed() async throws {
    var routes = exportRoutes().merging(compiledRoutes(squash, patchHead("Cinque v3 (#38932)"))) { _, new in new }
    routes[LFS.pullPatchURL(number: "38932")] = .body(patchHead("Use f78ed37d for the precompiled eGPU driving model"))
    #expect(try await LFS.fetchPointer(ref: squash, http: HTTP(session: MockNet(routes).session)) == Pointer(oid: v3OID, size: v3Size))
  }

  @Test func anOutageReadingThePullRequestIsAFailure() async throws {
    var routes = compiledRoutes(squash, patchHead("ResAction (#39037)"))
    routes[LFS.pullPatchURL(number: "39037")] = .status(503)
    let error = await #expect(throws: RegistryError.self) {
      try await LFS.fetchPointer(ref: squash, http: HTTP(session: MockNet(routes).session))
    }
    #expect(error?.isNetwork == true)
  }

  @Test func diffPointerTakesTheModelsPointerAndNothingElse() {
    func diff(_ path: String, _ old: Pointer?, _ new: Pointer?) -> String {
      var lines = ["diff --git a/\(path) b/\(path)", "--- a/\(path)", "+++ b/\(path)", "@@ -1,3 +1,3 @@"]
      if let old { lines += ["-version https://git-lfs.github.com/spec/v1", "-oid sha256:\(old.oid)", "-size \(old.size)"] }
      if let new { lines += ["+version https://git-lfs.github.com/spec/v1", "+oid sha256:\(new.oid)", "+size \(new.size)"] }
      return lines.joined(separator: "\n") + "\n"
    }
    let model = "openpilot/selfdrive/modeld/models/big_driving_supercombo.onnx"
    let a = Pointer(oid: String(repeating: "a", count: 64), size: 1)
    let b = Pointer(oid: String(repeating: "b", count: 64), size: 2)
    #expect(LFS.diffPointer(diff(model, a, b)) == b, "a changed model is the new one")
    #expect(LFS.diffPointer(diff(model, a, nil)) == a, "a deleted model is the one it was")
    #expect(LFS.diffPointer(diff(model, nil, a) + diff(model, a, b)) == b, "the last diff wins")
    #expect(LFS.diffPointer(diff("openpilot/selfdrive/modeld/models/big_driving_tinygrad.pkl", a, b)) == nil)
    #expect(LFS.diffPointer(diff(model + ".bak", a, b)) == nil)
    #expect(LFS.diffPointer(diff(model, a, nil) + diff("README.md", nil, nil)) == a)
    #expect(LFS.diffPointer("") == nil && LFS.diffPointer("not a patch") == nil)
  }

  @Test func aSquashMergeIsTheSubjectThatEndsWithItsPullRequest() {
    #expect(LFS.pullNumber(in: "ResAction (#39037)") == "39037")
    #expect(LFS.pullNumber(in: "two (#1) (#2)") == "2")
    #expect(LFS.pullNumber(in: "ResAction (#39037) again") == nil)
    #expect(LFS.pullNumber(in: "(#)") == nil)
    #expect(LFS.pullNumber(in: "(#12a)") == nil)
    #expect(LFS.pullNumber(in: "compiled") == nil)
  }
}
