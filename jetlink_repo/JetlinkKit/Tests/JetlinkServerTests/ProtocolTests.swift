import Foundation
import JetlinkKit
import JetlinkTestSupport
import Testing

@testable import JetlinkServer

/// Protocol 3 on the server's side: the hidden state stays here.
@Suite("Protocol 3", .serialized)
struct ProtocolTests {
  /// The queued golden frames carry a prev_feat of their own, as protocol 2
  /// sent it. Taken in through the server's own feedback path, as a hidden
  /// state kept from the frame before, they give protocol 2's outputs: the
  /// staging and the engine are unchanged, only where the hidden state comes
  /// from is.
  @Test("Protocol 2's outputs, from the server's own feedback path")
  func protocol2Outputs() throws {
    let golden = try Golden("tiny_queued")
    try serve { server, client in
      let ready = try client.ensureEngine(model: golden.model, sha256: golden.sha256)
      client.close()
      let spec = try ModelSpec.from(ready["spec"] as! [String: Any])
      let hidden = try #require(spec.hiddenRange)

      server.host.lock.lock()
      defer { server.host.lock.unlock() }
      let loaded = try #require(server.host.loaded)
      let type = try #require(loaded.engine.outputs[ModelConstants.drivingOutput]?.type)
      var kept = [Float](repeating: 0, count: spec.outputCount)
      var output = [Float](repeating: 0, count: spec.outputCount)
      let frameBytes = golden.frameBytes(spec)
      loaded.staging.reset()
      for i in 0..<(golden.frames.count / frameBytes) {
        try golden.frames.withUnsafeBytes { raw in
          let frame = raw.baseAddress! + i * frameBytes
          let packed = frame + spec.warpedBytes
          // the recorded prev_feat, after the scalars, as the last frame's hidden state
          kept.withUnsafeMutableBytes { k in
            (k.baseAddress! + hidden.lowerBound * 4).copyMemory(from: packed + spec.packedBytes, byteCount: hidden.count * 4)
          }
          kept.withUnsafeBytes { loaded.staging.keep(outputs: $0.baseAddress!, type: .float) }
          try loaded.staging.stage(warped: frame, packed: packed)
        }
        try loaded.engine.run()
        let out = try #require(loaded.engine.output(ModelConstants.drivingOutput))
        output.withUnsafeMutableBytes { o in
          if type == .float16 {
            Convert.f16ToF32(out, o.baseAddress!, count: spec.outputCount)
          } else {
            o.baseAddress!.copyMemory(from: out, byteCount: spec.outputBytes)
          }
        }
        let got = output.withUnsafeBytes { Data($0) }
        let expected = Data(golden.expected[(i * spec.outputBytes)..<((i + 1) * spec.outputBytes)])
        #if os(Android) || os(Linux)
          // fp16 arithmetic there; see CommaClient.replay
          #expect(Golden.correlation(got, expected) >= 0.999, "frame \(i)")
        #else
          #expect(got == expected, "frame \(i) differs from protocol 2's by up to \(Golden.worstDifference(got, expected))")
        #endif
      }
    }
  }
}
