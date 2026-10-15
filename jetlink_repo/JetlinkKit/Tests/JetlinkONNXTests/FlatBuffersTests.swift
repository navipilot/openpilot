import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkONNX

@Suite struct FlatBuffersTests {
  /// The builder against Python's flatbuffers 25.12.19, call for call: a
  /// string, an int vector, a byte vector aligned to 16, two tables that
  /// share a vtable (one field left at its default), a vector of them, and a
  /// root with a 64-bit field, finished with an identifier.
  @Test func matchesPythonsBuilder() throws {
    var b = FlatBufferBuilder(capacity: 0)
    let s = b.string("hi")
    let ints = b.vector([Int32(1), 2, -3])
    let data = b.vector([UInt8(9), 8, 7], alignment: 16)
    var tables: [FlatBufferBuilder.Offset] = []
    for k: Int8 in [5, 7] {
      b.startTable(fields: 4)
      b.add(0, s)
      b.add(1, k)
      b.add(2, Int32(0))
      b.add(3, ints)
      tables.append(b.endTable())
    }
    let vector = b.vector(tables)
    b.startTable(fields: 3)
    b.add(0, vector)
    b.add(1, UInt64(1) << 40)
    b.add(2, data)
    let bytes = b.finish(b.endTable(), identifier: "TEST")
    #expect(
      hexString(bytes)
        == "200000005445535400000000000000000000000000000a0014001000080004000a00000048000000000000000001000004000000020000002400000004000000f0ffffff34000000000000073c0000000c0010000c000b00000004000c000000180000000000000520000000030000000908070000000000030000000100000002000000fdffffff0200000068690000"
    )

    let r = FlatBufferReader(bytes: bytes)
    #expect(r.identifier == "TEST")
    let root = r.root
    #expect(root.u64(1) == 1 << 40)
    let children = root.tables(0)
    #expect(children.map { $0.i8(1) } == [5, 7])
    #expect(children.allSatisfy { $0.string(0) == "hi" && $0.int32s(3) == [1, 2, -3] && $0.field(2) == nil })
    // One vtable for both.
    #expect(children[0].vtable == children[1].vtable)
    let v = try #require(root.vector(2))
    #expect(v.start % 16 == 0 && r.bytes[v.start..<(v.start + 3)] == [9, 8, 7])
  }

  @Test func growsPastItsCapacity() {
    var b = FlatBufferBuilder(capacity: 8)
    let big = [UInt8](repeating: 0xab, count: 100_000)
    let v = big.withUnsafeBytes { b.bytes($0, alignment: 16) }
    let names = (0..<500).map { b.string("tensor \($0)") }
    let list = b.vector(names)
    b.startTable(fields: 2)
    b.add(0, v)
    b.add(1, list)
    let bytes = b.finish(b.endTable(), identifier: "TFL3")
    let root = FlatBufferReader(bytes: bytes).root
    #expect(root.bytes(0) == big)
    let strings = root.vector(1)!
    #expect(strings.count == 500)
    let r = root.reader
    let at = strings.start + 4 * 499
    let target = at + Int(r.u32(at))
    #expect(String(decoding: r.bytes[(target + 4)..<(target + 4 + Int(r.u32(target)))], as: UTF8.self) == "tensor 499")
  }

  /// A TFLite model with every kind of buffer, read back field by field.
  @Test func tfliteModelReadsBack() throws {
    var m = TFLite.Model()
    m.tensors = [
      TFLite.Tensor(name: "x", shape: [1, 4], type: .float16),
      TFLite.Tensor(name: "x__f32", shape: [1, 4], type: .float32),
      TFLite.Tensor(name: "w", shape: [4], type: .float32, buffer: 1),
      TFLite.Tensor(name: "big__fp16", shape: [1, 4], type: .float16, buffer: 2),
      TFLite.Tensor(name: "big", shape: [1, 4], type: .float32),
      TFLite.Tensor(name: "y", shape: [1, 4], type: .float32),
      TFLite.Tensor(name: "out", shape: [1, 4], type: .uint8),
    ]
    m.operators = [
      TFLite.Operator(op: .dequantize, inputs: [3], outputs: [4]),
      TFLite.Operator(op: .cast, inputs: [0], outputs: [1], options: .cast(from: .float16, to: .float32)),
      TFLite.Operator(op: .add, inputs: [1, 2], outputs: [5], options: .add),
      TFLite.Operator(op: .gelu, inputs: [5], outputs: [5], options: .gelu(approximate: true)),
      TFLite.Operator(op: .cast, inputs: [5], outputs: [6], options: .cast(from: .float32, to: .uint8)),
    ]
    m.inputs = [0]
    m.outputs = [6]
    let placements: [TFLite.Placement] = [.empty, .inline([1, 2, 3, 4]), .external(offset: 4096, size: 8)]
    let file = try TFLiteFile(bytes: TFLite.encode(m, buffers: placements).bytes)
    #expect(file.version == 3)
    #expect(file.tensors.map(\.name) == ["x", "x__f32", "w", "big__fp16", "big", "y", "out"])
    #expect(file.tensors.allSatisfy { $0.signature == $0.shape })
    #expect(file.tensors.map(\.type) == [1, 0, 0, 1, 0, 0, 3])
    #expect(file.tensors.map(\.buffer) == [0, 0, 1, 2, 0, 0, 0])
    #expect(file.opNames == ["DEQUANTIZE", "CAST", "ADD", "GELU", "CAST"])
    // A code past 127 keeps the old byte field at the placeholder.
    #expect(file.operators.map(\.deprecatedCode) == [6, 53, 0, 127, 53])
    #expect(file.operators.map(\.version) == [3, 1, 1, 1, 1])
    #expect(file.operators[1].optionsType == 37 && file.operators[1].options?.i8(0) == 1 && file.operators[1].options?.i8(1) == 0)
    #expect(file.operators[3].optionsType == 116 && file.operators[3].options?.bool(0) == true)
    guard case .inline(let data) = file.buffers[1], case .external(let offset, let size) = file.buffers[2] else {
      Issue.record("buffers \(file.buffers)")
      return
    }
    #expect(data == [1, 2, 3, 4] && offset == 4096 && size == 8)
    #expect(file.signatureKey == "serving_default")
    #expect(file.signatureInputs.map(\.0) == ["x"] && file.signatureInputs.map(\.1) == [0])
    #expect(file.signatureOutputs.map(\.0) == ["out"] && file.signatureOutputs.map(\.1) == [6])
  }

  /// Where an external buffer's offset sits, so the preparation can write
  /// it in once it knows the flatbuffer's length.
  @Test func externalOffsetsCanBeWrittenIn() throws {
    var m = TFLite.Model()
    m.tensors = [TFLite.Tensor(name: "w", shape: [2], type: .float32, buffer: 1)]
    var (bytes, fields) = TFLite.encode(m, buffers: [.empty, .external(offset: 0, size: 8)])
    let field = try #require(fields[1])
    #expect(fields.count == 1 && bytes[field..<(field + 8)].allSatisfy { $0 == 0 })
    withUnsafeBytes(of: UInt64(3 << 40).littleEndian) { bytes.replaceSubrange(field..<(field + 8), with: $0) }
    guard case .external(let offset, let size) = try TFLiteFile(bytes: bytes).buffers[1] else {
      Issue.record("buffer 1 is not external")
      return
    }
    #expect(offset == 3 << 40 && size == 8)
  }
}
