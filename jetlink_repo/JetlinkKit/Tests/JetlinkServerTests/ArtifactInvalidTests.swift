import Foundation
import JetlinkKit
import JetlinkTestSupport
import Testing

@testable import JetlinkServer

/// A cached artifact that will not load is deleted and rebuilt once from
/// the ONNX, for any backend.
@Suite("Invalid artifacts", .serialized)
struct ArtifactInvalidTests {
  /// A host with `golden`'s model built and cached, then unloaded.
  func cached(_ golden: Golden, in tmp: TemporaryDirectory, backend: FlakyBackend) throws -> (ServerCache, Request) {
    let cache = try ServerCache(root: tmp.url, backend: backend)
    let model = try Data(contentsOf: golden.model)
    let request = try Request(sha256: golden.sha256, nbytes: Int64(model.count), frameSkip: 4)
    try model.write(to: cache.modelPath(request))
    let host = EngineHost(cache: cache)
    _ = host.request(request, session: nil)
    #expect(host.settles() && host.snapshot().state == .ready)
    host.close()
    #expect(cache.entry(request).exists)
    return (cache, request)
  }

  @Test("An artifact that will not load is rebuilt once from the ONNX on disk")
  func rebuildsOnce() throws {
    let golden = try Golden("tiny_stateful")
    let tmp = try TemporaryDirectory()
    let backend = FlakyBackend()
    let (cache, request) = try cached(golden, in: tmp, backend: backend)

    backend.failLoads(with: ArtifactInvalid("built by another runtime"), times: 1)
    let host = EngineHost(cache: cache)
    defer { host.close() }
    _ = host.request(request, session: nil)
    #expect(host.settles())
    #expect(host.snapshot().state == .ready)
    #expect(backend.counts == (builds: 1, loads: 2))
    #expect(cache.entry(request).exists)
  }

  @Test("Without the ONNX the artifact still goes, and the client is asked to upload")
  func withoutTheModel() throws {
    let golden = try Golden("tiny_stateful")
    let tmp = try TemporaryDirectory()
    let backend = FlakyBackend()
    let (cache, request) = try cached(golden, in: tmp, backend: backend)
    try FileManager.default.removeItem(at: cache.modelPath(request))

    backend.failLoads(with: ArtifactInvalid("built by another runtime"), times: 1)
    let host = EngineHost(cache: cache)
    defer { host.close() }
    _ = host.request(request, session: nil)
    #expect(host.settles())
    let event = host.snapshot()
    #expect(event.state == .failed)
    #expect(event.detail.contains("artifact invalid and the model is not on disk"))
    #expect(backend.counts == (builds: 0, loads: 1))
    #expect(!cache.entry(request).exists)
    #expect(host.request(request, session: nil)["state"] as? String == "need_upload")
  }
}
