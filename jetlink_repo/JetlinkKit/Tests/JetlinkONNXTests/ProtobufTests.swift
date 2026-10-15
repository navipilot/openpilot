import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkONNX

// The expected bytes are what Python's protobuf (upb, protobuf 7.36.1, onnx
// 1.22) wrote for the same messages on 2026-09-26.
@Suite struct ProtobufTests {
  @Test func varintsRoundTrip() throws {
    let values: [UInt64] = [0, 1, 127, 128, 300, 16_383, 16_384, UInt64(Int32.max), UInt64(bitPattern: -1), .max]
    var e = Encoded()
    for v in values { e.varint(v) }
    let bytes = e.tail
    #expect(bytes.count == values.reduce(0) { $0 + varintSize($1) })
    try bytes.withUnsafeBytes { buf throws in
      var r = ProtoReader(buf, 0..<buf.count)
      for v in values {
        let read = try r.varint()
        #expect(read == v)
      }
      #expect(r.atEnd)
    }
  }

  @Test func negativeIntsAreTenBytes() {
    var e = Encoded()
    e.intField(1, -1)
    #expect(hexString(e.tail) == "08ffffffffffffffffff01")
  }

  @Test func truncatedInputIsRefused() {
    // A length-delimited field that claims more bytes than there are.
    let bytes = hex("0a0561")
    #expect(throws: OnnxError.self) {
      try bytes.withUnsafeBytes { buf in
        var r = ProtoReader(buf, 0..<buf.count)
        _ = try r.next()
      }
    }
  }

  @Test func groupsAreSkipped() throws {
    // field 3 as a group holding a varint, then field 1 varint 5
    let bytes = hex("1b08011c0805")
    try bytes.withUnsafeBytes { buf throws in
      var r = ProtoReader(buf, 0..<buf.count)
      let group = try #require(try r.next())
      #expect(group.number == 3 && group.wire == .startGroup && group.whole == 0..<4)
      let next = try #require(try r.next())
      #expect(next.number == 1 && next.value == 5)
    }
  }

  /// A tensor written out of order, with a field onnx.proto does not have:
  /// Python puts the known fields in number order and the unknown one last.
  @Test func knownFieldsInOrderUnknownLast() throws {
    try assertReencodes(tensor: "4201619806050802" + "1001", to: "0802100142016198" + "0605")
  }

  /// An unknown field numbered below known ones still goes last. Field 3 is
  /// Segment in a tensor, but GraphProto reserves 3, so there it is unknown:
  /// graph {unknown 3 = 1, name = "g", doc_string = "d"} is written as
  /// name, doc_string, then the unknown field.
  @Test func lowNumberedUnknownFieldGoesLast() throws {
    let bytes = hex("1801" + "120167" + "520164")
    try bytes.withUnsafeBytes { buf throws in
      let src = Source(bytes: buf)
      let g = try Decode.graph(src, 0..<buf.count)
      #expect(hexString(flatten(Encode.graph(g, src), src)) == "120167" + "520164" + "1801")
    }
  }

  /// dims is a proto2 repeated int64 without [packed=true]: read packed,
  /// written unpacked.
  @Test func dimsAreWrittenUnpacked() throws {
    try assertReencodes(tensor: "0a0202031007", to: "080208031007")
  }

  /// float_data is declared packed: read unpacked, written packed.
  @Test func floatDataIsWrittenPacked() throws {
    try assertReencodes(tensor: "250000803f25000000401001", to: "100122080000803f00000040")
  }

  /// numpy_helper.from_array(np.array([[1, 2], [3, 4]], np.int64), 'x')
  @Test func newTensorMatchesFromArray() {
    let t = Patches.int64Tensor([1, 2, 3, 4], "x")
    var shaped = t
    shaped.dims = [2, 2]
    let bytes = [UInt8]()
    bytes.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      #expect(
        hexString(flatten(Encode.tensor(shaped, src), src))
          == "0802080210074201784a20" + "0100000000000000" + "0200000000000000" + "0300000000000000" + "0400000000000000")
    }
  }

  /// helper.make_node('Gemm', ['a', 'b', 'c'], ['y'], name='g', transB=1)
  @Test func newNodeMatchesMakeNode() {
    let node = Node(inputs: ["a", "b", "c"], outputs: ["y"], name: "g", opType: "Gemm", attributes: [.int("transB", 1)])
    let bytes = [UInt8]()
    bytes.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      #expect(
        hexString(flatten(Encode.node(node, src), src))
          == "0a01610a01620a01631201791a0167220447656d6d2a0d0a067472616e73421801a00102")
    }
  }

  /// An attribute whose ints were written packed: Python writes them one
  /// per tag, since AttributeProto.ints is not declared packed.
  @Test func packedAttributeIntsAreUnpacked() throws {
    try assertReencodes(attribute: "0a0470616473" + "4203010203" + "a00107", to: "0a0470616473400140024003a00107")
  }

  /// Packed floats, the type first, and the name twice: Python keeps the
  /// last name and writes the fields in number order.
  @Test func oddAttributeIsWrittenAsPythonWritesIt() throws {
    try assertReencodes(
      attribute: "a00106" + "0a0178" + "3a080000803f00000040" + "0a0173",
      to: "0a01733d0000803f3d00000040a00106")
  }

  /// An attribute Python wrote is copied as it is.
  @Test func canonicalAttributeIsCopied() throws {
    let bytes = hex("2a" + "0f" + "0a0470616473400140024003a00107")
    try bytes.withUnsafeBytes { buf throws in
      let src = Source(bytes: buf)
      var r = src.reader(0..<buf.count)
      let field = try #require(try r.next())
      let a = try Decode.attribute(src, field)
      guard case .source(let range) = a.bytes else {
        Issue.record("rewritten, not copied")
        return
      }
      #expect(range == 0..<buf.count)
      #expect(a.name == "pads" && a.type == 7)
    }
  }

  @Test func invalidUTF8NamesAreRefused() {
    let bytes = hex("4202ff61")  // name = 0xff 'a'
    #expect(throws: OnnxError.self) {
      try bytes.withUnsafeBytes { buf in
        _ = try Decode.tensor(Source(bytes: buf), 0..<buf.count)
      }
    }
  }

  /// onnx.helper.make_tensor_value_info('q', TensorProto.UINT8, [2, 30, 128, 256])
  @Test func valueInfoMatchesMakeTensorValueInfo() {
    let bytes = [UInt8]()
    bytes.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      #expect(
        hexString(flatten(Encode.valueInfo(.tensor("q", 2, [2, 30, 128, 256]), src), src))
          == "0a017112180a16080212120a0208020a02081e0a030880010a03088002")
    }
  }

  /// helper.make_node('Transpose', ['a'], ['b'], name='t', perm=[0, 2, 1, 3])
  @Test func intsAttributeMatchesMakeNode() {
    let node = Node(inputs: ["a"], outputs: ["b"], name: "t", opType: "Transpose", attributes: [.ints("perm", [0, 2, 1, 3])])
    let bytes = [UInt8]()
    bytes.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      #expect(
        hexString(flatten(Encode.node(node, src), src))
          == "0a01611201621a017422095472616e73706f73652a110a047065726d4000400240014003a00107")
    }
  }

  /// make_opsetid('', 20) and make_attribute('epsilon', 1e-5), read back.
  @Test func opsetVersionAndFloatAttributesAreRead() throws {
    let opset = hex("0a001014")
    try opset.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      let o = try Decode.opset(src, WireField(number: 8, wire: .bytes, start: 0, end: buf.count, value: 0, payload: 0..<buf.count))
      #expect(o.domain == "" && o.version == 20)
    }
    let attribute = hex("0a07657073696c6f6e15acc52737a00101")
    try attribute.withUnsafeBytes { buf in
      let src = Source(bytes: buf)
      let a = try Decode.attribute(src, WireField(number: 5, wire: .bytes, start: 0, end: buf.count, value: 0, payload: 0..<buf.count))
      #expect(a.name == "epsilon" && a.f == Float(1e-5) && a.type == 1)
    }
  }

  private func assertReencodes(attribute input: String, to expected: String) throws {
    // Wrapped as NodeProto.attribute (field 5), then the node written back.
    let payload = hex(input)
    let bytes = [0x2a, UInt8(payload.count)] + payload
    try bytes.withUnsafeBytes { buf throws in
      let src = Source(bytes: buf)
      let node = try Decode.node(src, 0..<buf.count)
      #expect(hexString(flatten(Encode.node(node, src), src)) == "2a" + String(format: "%02x", expected.count / 2) + expected)
    }
  }

  private func assertReencodes(tensor input: String, to expected: String) throws {
    let bytes = hex(input)
    try bytes.withUnsafeBytes { buf throws in
      let src = Source(bytes: buf)
      let t = try Decode.tensor(src, 0..<buf.count)
      #expect(hexString(flatten(Encode.tensor(t, src), src)) == expected)
    }
  }
}
