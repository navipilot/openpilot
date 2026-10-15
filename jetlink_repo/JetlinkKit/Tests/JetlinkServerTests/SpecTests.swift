import Foundation
import JetlinkKit
import Testing

@testable import JetlinkServer

@Suite("Model spec")
struct SpecTests {
  @Test("The spec read from the ONNX is the dict Python sends", arguments: ["tiny_queued", "tiny_stateful"])
  func matchesPython(_ name: String) throws {
    let python = try Fixture.json("\(name).spec.json")
    let spec = try ONNXPreparer().readSpec(
      model: Fixture.url("\(name).onnx"), sha256: python["sha256"] as! String,
      nbytes: (python["nbytes"] as! NSNumber).int64Value, frameSkip: 4)
    let swift = try JSONSerialization.jsonObject(with: JSONSerialization.data(withJSONObject: spec.dictionary())) as! NSDictionary
    #expect(swift == python as NSDictionary)
    // and back, as a comma or a sidecar would read it
    #expect(try ModelSpec.from(python).dictionary() as NSDictionary == spec.dictionary() as NSDictionary)
  }

  @Test("The wire sizes follow the model's shapes")
  func sizes() throws {
    let queued = try ModelSpec.from(Fixture.json("tiny_queued.spec.json"))
    #expect(!queued.stateful)
    #expect(queued.warpedShape == [2, 6, 8, 16])
    // prev_feat stays here (protocol 3), and so does hidden_state, 32 of the 64
    #expect(queued.packedShapes.map(\.name) == ["desire", "traffic_convention", "action_t"])
    #expect(queued.packedCount == 8 + 2 + 2)
    #expect(queued.prevFeatCount == 32)
    #expect(queued.inferReqBytes == Wire.inferReqSize + 1536 + 4 * 12)
    #expect(queued.hiddenRange == 32..<64)
    #expect(queued.inferRespBytes == Wire.inferRespSize + 4 * 32)

    let stateful = try ModelSpec.from(Fixture.json("tiny_stateful.spec.json"))
    #expect(stateful.stateful)
    #expect(Set(stateful.statePairs.map(\.input)) == ["state_img_q", "state_desire_q", "state_feat_q"])
    #expect(stateful.packedCount == 8 + 2 + 2)
    #expect(stateful.hiddenRange == 32..<48)
    #expect(stateful.inferRespBytes == Wire.inferRespSize + 4 * 48)
  }

  /// Lebowski's pad is `slice(-2, None)`, which its sidecar and ENGINE_RESP
  /// carry as `[-2, null]`. `expected` is jetlink.spec's `to_dict` of this
  /// spec, `json.dumps(sort_keys=True, separators=(',', ':'))`.
  @Test("Slices with an open end or one counted from the back go back as Python writes them")
  func pythonSlices() throws {
    let expected =
      #"{"checkpoint":null,"frame_skip":4,"input_shapes":{"action_t":[1,2],"desire":[1,8],"new_img":[2,6,128,256],"state_feat_q":[4,1,512],"traffic_convention":[1,2]},"nbytes":123,"output_shapes":{"next_state_feat_q":[4,1,512],"outputs":[1,2580]},"output_slices":{"action":[2574,2578],"head":[null,4],"pad":[-2,null],"plan":[0,2574]},"sha256":"abababababababababababababababababababababababababababababababab"}"#
    let spec = try ModelSpec.from(try #require(try JSONSerialization.jsonObject(with: Data(expected.utf8)) as? [String: Any]))
    #expect(ControlJSON.data(spec.dictionary()).map { String(decoding: $0, as: UTF8.self) } == expected)
    let slices = Dictionary(uniqueKeysWithValues: spec.outputSlices.map { ($0.name, $0) })
    #expect(slices["pad"] == NamedSlice("pad", start: -2, stop: nil))
    #expect(slices["pad"]?.range(in: spec.outputCount) == 2578..<2580)
    #expect(slices["head"]?.range(in: spec.outputCount) == 0..<4)
    #expect(NamedSlice("x", start: 5, stop: -1).range(in: 10) == 5..<9)
    #expect(NamedSlice("x", start: 8, stop: 2).range(in: 10) == 8..<8)
    // no hidden_state: the reply is the whole output, as hidden_range says
    #expect(spec.hiddenRange == nil && spec.replyCount == 2580)
    // one with an open end runs to the end, as Python slices it
    let open = ModelSpec(
      sha256: spec.sha256, nbytes: 1, frameSkip: 4, inputShapes: spec.inputShapes, outputShapes: spec.outputShapes,
      outputSlices: [NamedSlice("hidden_state", start: 2000, stop: nil)], checkpoint: nil)
    #expect(open.hiddenRange == 2000..<2580 && open.replyCount == 2000)
  }

  /// A 766 MB model's frame, whichever layout: one 16 KB read on the comma
  /// down (it was five), and 64 KB less up for a queued one.
  @Test("A big model's reply fits the one read the comma keeps posted")
  func bigModel() {
    let slices = [NamedSlice("plan", 917..<1907), NamedSlice("hidden_state", 2066..<18450), NamedSlice("pad", 18450..<18452)]
    let queued = ModelSpec(
      sha256: "", nbytes: 0, frameSkip: 4,
      inputShapes: [
        NamedShape("img", [1, 12, 128, 256]), NamedShape("big_img", [1, 12, 128, 256]), NamedShape("desire_pulse", [1, 25, 8]),
        NamedShape("traffic_convention", [1, 2]), NamedShape("action_t", [1, 2]), NamedShape("features_buffer", [1, 32, 32, 512]),
      ], outputShapes: [NamedShape("outputs", [1, 18452])], outputSlices: slices, checkpoint: nil)
    #expect(Wire.headerSize + queued.inferReqBytes == 393_304)
    #expect(Wire.headerSize + queued.inferRespBytes == 8_324)
    #expect(queued.prevFeatCount == 16_384)
  }
}
