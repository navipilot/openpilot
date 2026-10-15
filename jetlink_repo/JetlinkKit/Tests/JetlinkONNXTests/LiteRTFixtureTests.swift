import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkONNX

/// The test models through the whole LiteRT preparation: the server's tiny
/// driving models computed against what tests/tiny_model.py says they
/// compute, and every graph the CoreML tests prepare converting with its
/// inputs and outputs as they were.
@Suite struct LiteRTFixtureTests {
  static func convert(_ url: URL) throws -> (file: TFLiteFile, report: LiteRTPreparation.Report) {
    let dir = try TemporaryDirectory()
    let report = try LiteRTPreparation.prepare(source: url, into: dir.url)
    return (try TFLiteFile(report.url), report)
  }

  /// An initializer's values, read from the ONNX file.
  static func initializer(_ url: URL, _ name: String) throws -> [Float] {
    let data = try Data(contentsOf: url)
    return try data.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      let g = try #require(try Decode.model(src).graph)
      let t = try #require(g.initializers.first { $0.key == name })
      return try Elements.floats(Elements.littleEndian(t, src), as: t.elementType, for: name)
    }
  }

  static func affine(_ features: [Float], _ w: [Float], _ b: [Float]) -> [Float] {
    (0..<b.count).map { j in
      var acc = b[j]
      for k in features.indices { acc += features[k] * w[k * b.count + j] }
      return acc
    }
  }

  /// The mean of each of `channels` planes.
  static func planeMeans(_ x: [Float], channels: Int) -> [Float] {
    let plane = x.count / channels
    return (0..<channels).map { c in x[(c * plane)..<((c + 1) * plane)].reduce(0, +) / Float(plane) }
  }

  @Test func tinyQueuedComputesItsReference() throws {
    // It has tinygrad's Contiguous, which the lowering refuses: converting at
    // all means the preparation stripped it.
    let (file, _) = try Self.convert(TinyModel.queued)
    let types = file.inputs.map { file.tensors[$0].type }
    #expect(file.inputs.map { file.tensors[$0].name } == ["img", "big_img", "desire_pulse", "traffic_convention", "action_t", "features_buffer"])
    #expect(types == [3, 3, 1, 1, 1, 1])
    #expect(file.tensors[file.outputs[0]].name == "outputs" && file.tensors[file.outputs[0]].type == 1)

    var v = SeededValues(seed: 31)
    let img = (0..<1536).map { Float(($0 * 37) % 256) }
    let big = (0..<1536).map { Float(($0 * 11 + 5) % 256) }
    let inputs: [String: [Float]] = [
      "img": img, "big_img": big, "desire_pulse": v(264), "traffic_convention": [1, 0], "action_t": v(2), "features_buffer": v(1024),
    ]
    let out = try LiteRTLoweringTests.run(file, inputs)["outputs"]!
    let features =
      Self.planeMeans(img + big, channels: 24) + inputs["desire_pulse"]! + inputs["traffic_convention"]! + inputs["action_t"]!
      + inputs["features_buffer"]!
    let want = Self.affine(features, try Self.initializer(TinyModel.queued, "W"), try Self.initializer(TinyModel.queued, "B"))
    // The file's output is fp16: one rounding of it apart.
    #expect(zip(out, want).allSatisfy { abs($0 - $1) <= max(abs($1), 1) * 1e-3 })
  }

  @Test func tinyStatefulComputesItsReferenceFrameAfterFrame() throws {
    let (file, _) = try Self.convert(TinyModel.stateful)
    // The rank-5 frame queue is a 4-D view of the same bytes.
    #expect(file.tensors[try #require(file.tensor(named: "state_img_q"))].shape == [2, 30, 8, 16])
    #expect(file.tensors.allSatisfy { $0.shape.count <= 4 })
    let w = try Self.initializer(TinyModel.stateful, "W")
    let b = try Self.initializer(TinyModel.stateful, "B")

    var state: [String: [Float]] = [
      "state_img_q": [Float](repeating: 0, count: 7680), "state_desire_q": [Float](repeating: 0, count: 48),
      "state_feat_q": [Float](repeating: 0, count: 64),
    ]
    var v = SeededValues(seed: 41)
    for frame in 0..<3 {
      let fresh = (0..<1536).map { Float(($0 * 13 + frame * 7) % 256) }
      var desire = [Float](repeating: 0, count: 8)
      desire[frame] = 1
      let inputs = ["new_img": fresh, "desire": desire, "traffic_convention": [1, 0], "action_t": v(2)].merging(state) { a, _ in a }
      let out = try LiteRTLoweringTests.run(file, inputs)

      // stateful_step: push the frame and the desire, read frames 0 and 4.
      let old = state["state_img_q"]!
      var queue: [Float] = []
      var imgs: [Float] = []
      for c in 0..<2 {
        let camera = Array(old[(c * 3840 + 768)..<((c + 1) * 3840)]) + Array(fresh[(c * 768)..<((c + 1) * 768)])
        queue += camera
        imgs += Array(camera[0..<768]) + Array(camera[3072..<3840])
      }
      let desires: [Float] = Array(state["state_desire_q"]![8...]) + desire
      var features: [Float] = Self.planeMeans(imgs, channels: 24).map { $0 * Float(1.0 / 255.0) }
      features += desires
      features += [1, 0]
      features += inputs["action_t"]!
      features += state["state_feat_q"]!
      let outputs = Self.affine(features, w, b)
      let feats = Array(state["state_feat_q"]![16...]) + outputs[32..<48]

      #expect(out["next_state_img_q"] == queue, "frame \(frame)")
      #expect(out["next_state_desire_q"] == desires, "frame \(frame)")
      #expect(maxError(out["outputs"]!, outputs) < 1e-4, "frame \(frame)")
      #expect(maxError(out["next_state_feat_q"]!, feats) < 1e-4, "frame \(frame)")
      state = ["state_img_q": out["next_state_img_q"]!, "state_desire_q": out["next_state_desire_q"]!, "state_feat_q": out["next_state_feat_q"]!]
    }
  }

  static let graphs = ["stateful", "queued", "variants", "nocut", "noentry", "noshape", "notype", "unrecorded"]

  /// Every graph the CoreML preparation tests use converts, with its inputs
  /// and outputs named, typed and sized as in the ONNX, and runs.
  @Test(arguments: graphs)
  func fixtureConverts(_ name: String) throws {
    let url = Fixtures.url("\(name).onnx")
    let (file, _) = try Self.convert(url)
    let meta = try OnnxMeta.read(contentsOf: url)
    let types: [Int32: Int8] = [DataType.float: 0, DataType.float16: 1, DataType.uint8: 3]
    for (edge, tensors) in [(meta.inputs, file.inputs), (meta.outputs, file.outputs)] {
      #expect(edge.map(\.name) == tensors.map { file.tensors[$0].name })
      for (onnx, t) in zip(edge, tensors) {
        #expect(types[onnx.elemType] == file.tensors[t].type, "\(onnx.name)")
        #expect(onnx.dims.reduce(1, *) == Int64(file.tensors[t].shape.reduce(1, *)), "\(onnx.name)")
      }
    }
    #expect(file.tensors.allSatisfy { $0.shape.count <= 4 && $0.type != TFLite.TensorType.bool.rawValue })
    var interpreter = TFLiteInterpreter(file)
    var inputs: [String: [Float]] = [:]
    for i in file.inputs {
      inputs[file.tensors[i].name] = [Float](repeating: 1, count: file.tensors[i].shape.reduce(1, *))
    }
    let out = try interpreter.run(inputs)
    #expect(out.values.allSatisfy { $0.allSatisfy(\.isFinite) })
  }
}
