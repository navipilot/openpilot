import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkRegistry

/// Against the real servers, and only when asked: JETLINK_LIVE=1 swift test
/// --filter LiveTests. It fetches the catalog and resolves the pointers (a few
/// hundred bytes each) into a temp dir. It never downloads a model.
@Suite(.enabled(if: ProcessInfo.processInfo.environment["JETLINK_LIVE"] == "1"))
struct LiveTests {
  @Test func catalogAndPointers() async throws {
    let tmp = try TemporaryDirectory()
    let registry = Registry(layout: tmp.layout, session: .shared)

    let catalog = await registry.catalog(refresh: true)
    print("live catalog: \(catalog.models.count) models, error \(catalog.error ?? "none"), url \(catalog.url)")
    for model in catalog.models {
      print("  \(model.index) \(model.ref.prefix(10)) \(model.shortName) \(model.name)")
    }
    #expect(catalog.error == nil)
    #expect(!catalog.models.isEmpty)

    let results = await registry.resolveMissing(catalog.models.map(\.ref))
    let failures = results.filter { (try? $0.value.get()) == nil }
    print("live pointers: \(results.count - failures.count) resolved, \(failures.count) failed")
    for (ref, result) in failures {
      if case .failure(let error) = result { print("  \(ref.prefix(10)) failed: \(error.kind.pythonName): \(error.message)") }
    }
    for (ref, result) in results.sorted(by: { $0.key < $1.key }) {
      if case .success(let pointer) = result { print("  \(ref.prefix(10)) -> \(pointer.oid.prefix(16)) \(pointer.size)") }
    }
    if let pointer = try? results[Catalog.defaultBigModelRef]?.get() {
      print("live pointer: \(Catalog.defaultBigModelRef) -> \(pointer.oid) \(pointer.size) bytes")
    }

    let resolved = await registry.catalog(refresh: false, maxAge: .infinity)
    #expect(resolved.models.filter { $0.sha256 != nil }.count == results.count - failures.count)
    #expect(failures.isEmpty)
  }
}
