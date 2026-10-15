import Foundation
import JetlinkTestSupport

@testable import JetlinkONNX

extension Attribute {
  /// A FLOAT attribute as onnx.helper.make_attribute writes it: name, f, type.
  static func float(_ name: String, _ value: Float) -> Attribute {
    var e = Encoded()
    e.stringField(1, name)
    e.tag(2, .fixed32)
    withUnsafeBytes(of: value.bitPattern.littleEndian) { e.bytes($0) }
    e.intField(20, 1)
    return Attribute(bytes: .owned(e.tail), name: name, type: 1, i: 0, f: value, ints: [], s: "", floats: [], t: nil)
  }
}

/// A small ONNX model written from Swift for the LiteRT lowering tests:
/// nodes, initializers as raw_data and value infos with static shapes, in a
/// Graph that Encode.model writes as onnx.save would.
struct OnnxGraphBuilder {
  var opset: Int64
  var graph = Graph(name: "test")

  init(opset: Int64 = 17) {
    self.opset = opset
  }

  mutating func input(_ name: String, _ type: Int32, _ dims: [Int64]) {
    graph.inputs.append(.tensor(name, type, dims))
  }

  mutating func output(_ name: String, _ type: Int32, _ dims: [Int64]) {
    graph.outputs.append(.tensor(name, type, dims))
  }

  mutating func valueInfo(_ name: String, _ type: Int32, _ dims: [Int64]) {
    graph.valueInfo.append(.tensor(name, type, dims))
  }

  mutating func node(_ op: String, _ inputs: [String], _ outputs: [String], _ attributes: [Attribute] = []) {
    graph.nodes.append(Node(inputs: inputs, outputs: outputs, name: "\(op)_\(graph.nodes.count)", opType: op, attributes: attributes))
  }

  mutating func initializer(_ name: String, _ type: Int32, _ dims: [Int64], _ raw: [UInt8]) {
    graph.initializers.append(Tensor(name: name, dims: dims, dataType: type, raw: .owned(raw)))
  }

  mutating func fp16(_ name: String, _ dims: [Int64], _ values: [Float]) {
    initializer(name, DataType.float16, dims, Elements.encode(values, as: DataType.float16))
  }

  mutating func fp32(_ name: String, _ dims: [Int64], _ values: [Float]) {
    initializer(name, DataType.float, dims, Elements.encode(values, as: DataType.float))
  }

  mutating func int64(_ name: String, _ values: [Int64], dims: [Int64]? = nil) {
    var t = Patches.int64Tensor(values, name)
    t.dims = dims ?? t.dims
    graph.initializers.append(t)
  }

  mutating func bool(_ name: String, _ dims: [Int64], _ values: [Bool]) {
    initializer(name, DataType.bool, dims, values.map { $0 ? 1 : 0 })
  }

  var bytes: [UInt8] {
    // Nothing is read from a source: every field is made here.
    let none = Source(bytes: UnsafeRawBufferPointer(start: nil, count: 0))
    var m = Encode.model(Model(irVersion: 8, graph: graph), none)
    // An opset import is copied from its source; this one is written.
    var opsetImport = Encoded()
    opsetImport.stringField(1, "")
    opsetImport.intField(2, opset)
    m.message(8, opsetImport)
    return flatten(m, none)
  }

  /// The model converted by LiteRTPreparation, read back.
  func convert() throws -> (file: TFLiteFile, report: LiteRTPreparation.Report) {
    let dir = try TemporaryDirectory()
    let source = dir.url.appendingPathComponent("model.onnx")
    try Data(bytes).write(to: source)
    let report = try LiteRTPreparation.prepare(source: source, into: dir.url.appendingPathComponent("out"))
    return (try TFLiteFile(report.url), report)
  }
}

/// Seeded values for tests, the same on every platform.
struct SeededValues {
  private var state: UInt64

  init(seed: UInt64) {
    state = seed &* 0x9E37_79B9_7F4A_7C15 | 1
  }

  /// Uniform in [-1, 1), on a grid fp16 holds exactly.
  mutating func next() -> Float {
    state ^= state << 13
    state ^= state >> 7
    state ^= state << 17
    return Float(Int(state % 2048) - 1024) / 1024
  }

  mutating func callAsFunction(_ count: Int) -> [Float] {
    (0..<count).map { _ in next() }
  }
}

/// The worst absolute difference.
func maxError(_ a: [Float], _ b: [Float]) -> Float {
  precondition(a.count == b.count, "\(a.count) values against \(b.count)")
  return zip(a, b).map { abs($0 - $1) }.max() ?? 0
}
