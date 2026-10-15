import Foundation
import Testing

@testable import JetlinkServer

@Suite("Staging")
struct StagingTests {
  /// A queued graph cannot run without the hidden state to feed back.
  @Test("A queued graph with no hidden_state of the feature size is refused at load")
  func needsHiddenState() throws {
    let fixture = try StagingCase(frameSkip: 4)
    let spec = fixture.spec
    for slices in [[NamedSlice("plan", 0..<16)], [NamedSlice("hidden_state", 32..<60)]] {
      let other = ModelSpec(
        sha256: spec.sha256, nbytes: spec.nbytes, frameSkip: 4, inputShapes: spec.inputShapes, outputShapes: spec.outputShapes,
        outputSlices: slices, checkpoint: nil)
      #expect(throws: StagingError.self) { try PolicyQueues(spec: other, engine: StagingEngine(fixture.inputs)) }
    }
  }

  /// numpy's max, which `sample_desire` takes over each group of frame_skip
  /// frames: a NaN wins wherever it is, the first of two NaNs stays, of two
  /// equal values (0 and -0) the first stays, and whatever wins keeps its
  /// float16 bits. Each column below is one such group, pushed oldest first.
  @Test("Desire is sampled as numpy's max takes it, NaN and signed zeros included", arguments: [ElementType.float16, .float])
  func desireMax(type: ElementType) throws {
    let fixture = try StagingCase(frameSkip: 4) { $0 == "desire_pulse" ? type : .float16 }
    let spec = fixture.spec
    let engine = StagingEngine(fixture.inputs)
    let staging = try PolicyQueues(spec: spec, engine: engine)
    let nan = Float(bitPattern: 0x7FC0_0000)
    // column: the four frames' values, and the float16 numpy's max gives
    let columns: [([Float], UInt16)] = [
      ([1, nan, 2, 3], 0x7E00),
      ([1, Float(bitPattern: 0x7FC0_2000), Float(bitPattern: 0x7FC0_4000), 6], 0x7E01),
      ([0, -0.0, -1, -2], 0x0000),
      ([-0.0, 0, -1, -2], 0x8000),
      ([1, 3, 2, -1], 0x4200),
      ([Float(bitPattern: 0xFFC0_0000), 1, 2, 3], 0xFE00),
      ([-.infinity, -5, nan, 7], 0x7E00),
      ([2, .infinity, 3, 4], 0x7C00),
    ]
    let width = try #require(spec.input("desire_pulse")?.last)
    #expect(width == columns.count)
    let desire = try #require(spec.packedLayout.first { $0.name == "desire" }).range
    let warped = [UInt8](repeating: 0, count: spec.warpedBytes)
    var packed = [Float](repeating: 0, count: spec.packedCount)
    staging.reset()
    for frame in 0..<4 {
      for (column, (values, _)) in columns.enumerated() {
        packed[desire.lowerBound + column] = values[frame]
      }
      try warped.withUnsafeBytes { w in try packed.withUnsafeBytes { p in try staging.stage(warped: w.baseAddress!, packed: p.baseAddress!) } }
    }
    // The four frames fill the newest group; the older ones are still the
    // zeros a reset leaves.
    let out = engine.hostInput("desire_pulse")!
    let count = spec.input("desire_pulse")!.reduce(1, *)
    let got: [UInt16] = (0..<count).map { i in
      type == .float16 ? out.load(fromByteOffset: 2 * i, as: UInt16.self) : Float16(out.load(fromByteOffset: 4 * i, as: Float.self)).bitPattern
    }
    #expect(got.dropLast(width).allSatisfy { $0 == 0 })
    #expect(Array(got.suffix(width)) == columns.map(\.1))
  }
}
