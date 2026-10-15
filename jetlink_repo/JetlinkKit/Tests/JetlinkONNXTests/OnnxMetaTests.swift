import Foundation
import Testing

@testable import JetlinkONNX

@Suite struct OnnxMetaTests {
  @Test func queuedFixture() throws {
    let meta = try OnnxMeta.read(contentsOf: Fixtures.url("queued.onnx"))
    #expect(
      meta.inputs == [
        .init(name: "img", elemType: 2, dims: [1, 12, 8, 16]),
        .init(name: "big_img", elemType: 2, dims: [1, 12, 8, 16]),
        .init(name: "desire_pulse", elemType: 1, dims: [1, 4, 8]),
        .init(name: "traffic_convention", elemType: 1, dims: [1, 2]),
        .init(name: "features_buffer", elemType: 1, dims: [1, 4, 16]),
      ])
    #expect(meta.outputs == [.init(name: "outputs", elemType: 1, dims: [1, 104])])
    #expect(meta.props.map(\.key) == ["output_slices", "model_checkpoint", "CACHE_KEY"])
    #expect(meta.modelCheckpoint == "queued-test")
    #expect(
      try meta.outputSlices() == [
        OutputSlice(name: "plan", start: 0, stop: 48),
        OutputSlice(name: "lead", start: 48, stop: 96),
        OutputSlice(name: "hidden_state", start: 96, stop: 104),
      ])
    #expect(meta.inputs[0].typeName == "uint8")
  }

  @Test func statefulFixture() throws {
    let meta = try OnnxMeta.read(contentsOf: Fixtures.url("stateful.onnx"))
    #expect(
      meta.inputs.map(\.name) == [
        "new_img", "desire", "traffic_convention", "action_t", "state_img_q", "state_desire_q", "state_feat_q",
      ])
    #expect(meta.outputs.map(\.name) == ["outputs", "next_state_img_q", "next_state_desire_q", "next_state_feat_q"])
    #expect(meta.outputs[1] == .init(name: "next_state_img_q", elemType: 2, dims: [2, 5, 6, 8, 16]))
    #expect(meta.inputs[1].dims == [8])
  }

  /// The prepared model's images are fp16, and the key is the last prop.
  @Test func preparedFixture() throws {
    let meta = try OnnxMeta.read(contentsOf: Fixtures.url("queued.whole.model.expected.onnx"))
    #expect(meta.inputs[0].typeName == "float16")
    #expect(meta.props.map(\.key) == ["output_slices", "model_checkpoint", "COREML_CACHE_KEY"])
    #expect(meta.prop("COREML_CACHE_KEY") == "fixturemodel")
  }

  /// A dim_param, and a dimension with neither value, read as 0.
  @Test func symbolicDimsAreZero() throws {
    var dimParam = Encoded()
    dimParam.stringField(2, "batch")
    var dimValue = Encoded()
    dimValue.intField(1, 3)
    let dimEmpty = Encoded()
    var shape = Encoded()
    shape.message(1, dimParam)
    shape.message(1, dimValue)
    shape.message(1, dimEmpty)
    var tensorType = Encoded()
    tensorType.intField(1, 1)
    tensorType.message(2, shape)
    var type = Encoded()
    type.message(1, tensorType)
    var vi = Encoded()
    vi.stringField(1, "x")
    vi.message(2, type)
    var graph = Encoded()
    graph.message(11, vi)
    var model = Encoded()
    model.intField(1, 8)
    model.message(7, graph)
    let meta = try OnnxMeta.read(Data(model.tail))
    #expect(meta.inputs == [.init(name: "x", elemType: 1, dims: [0, 3, 0])])
    #expect(meta.outputs.isEmpty && meta.props.isEmpty)
    #expect(throws: OnnxError("output_slices not in model metadata_props")) { try meta.outputSlices() }
  }

  @Test func garbageIsRefused() {
    #expect(throws: OnnxError.self) { try OnnxMeta.read(Data([0x0a, 0xff, 0xff, 0xff, 0xff, 0x0f])) }
  }
}
