import Foundation

// The parts of ONNX's ModelProto the preparation reads or changes, decoded
// from a memory-mapped file. What the preparation does not interpret is kept
// as ranges of the source and written back verbatim, so nothing is dropped.
// Field numbers are onnx.proto's (proto2; onnx 1.22).

/// ONNX's TensorProto.DataType values the preparation cares about.
enum DataType {
  static let float: Int32 = 1
  static let uint8: Int32 = 2
  static let int8: Int32 = 3
  static let uint16: Int32 = 4
  static let int16: Int32 = 5
  static let int32: Int32 = 6
  static let int64: Int32 = 7
  static let string: Int32 = 8
  static let bool: Int32 = 9
  static let float16: Int32 = 10
  static let double: Int32 = 11
  static let uint32: Int32 = 12
  static let uint64: Int32 = 13
  static let bfloat16: Int32 = 16

  /// Bytes per element in raw_data, for the types stored a whole byte or more
  /// per element. nil for strings, the 4- and 2-bit types and anything unknown.
  static func size(_ type: Int32) -> Int? {
    switch type {
    case 2, 3, 9, 17, 18, 19, 20, 24: 1
    case 4, 5, 10, 16: 2
    case 1, 6, 12: 4
    case 7, 11, 13, 14: 8
    case 15: 16
    default: nil
    }
  }

  /// numpy's integer types, which `np.issubdtype(dtype, np.integer)` accepts.
  static func isInteger(_ type: Int32) -> Bool {
    [uint8, int8, uint16, int16, int32, int64, uint32, uint64].contains(type)
  }
}

struct OpsetImport {
  let raw: Range<Int>
  let domain: String
  /// 0 when the source writes none, as protobuf reads an absent int64.
  let version: Int64
}

/// A StringStringEntryProto in ModelProto.metadata_props.
struct Prop {
  /// The entry as the source wrote it; nil for one the preparation adds.
  let raw: Range<Int>?
  let key: String
  let value: String
}

/// A FunctionProto, kept whole. The split carries along the functions its
/// nodes call, so it needs each one's name and the ops of its own nodes.
struct LocalFunction {
  let raw: Range<Int>
  let name: String
  let domain: String
  let calls: [(opType: String, domain: String)]
}

/// An AttributeProto, written back whole. Only what the patches and the
/// LiteRT lowering read is decoded.
struct Attribute {
  enum Bytes {
    /// The whole field in the source, tag included.
    case source(Range<Int>)
    /// The message's payload, for an attribute the preparation adds.
    case owned([UInt8])
  }

  let bytes: Bytes
  let name: String
  let type: Int32
  let i: Int64
  let f: Float
  let ints: [Int64]
  let s: String
  let floats: [Float]
  /// A tensor attribute (Constant's value), its data left in the source.
  let t: Tensor?

  /// An INT attribute as onnx.helper.make_attribute writes it: name, i, type.
  static func int(_ name: String, _ value: Int64) -> Attribute {
    var e = Encoded()
    e.stringField(1, name)
    e.intField(3, value)
    e.intField(20, 2)
    return Attribute(bytes: .owned(e.tail), name: name, type: 2, i: value, f: 0, ints: [], s: "", floats: [], t: nil)
  }

  /// An INTS attribute as make_attribute writes it: name, one ints field per
  /// value (AttributeProto.ints is not declared packed), type.
  static func ints(_ name: String, _ values: [Int64]) -> Attribute {
    var e = Encoded()
    e.stringField(1, name)
    for v in values { e.intField(8, v) }
    e.intField(20, 7)
    return Attribute(bytes: .owned(e.tail), name: name, type: 7, i: 0, f: 0, ints: values, s: "", floats: [], t: nil)
  }
}

struct Node {
  var inputs: [String] = []
  var outputs: [String] = []
  var name: String?
  var opType: String?
  var domain: String?
  var attributes: [Attribute] = []
  var extras: [RawField] = []

  var op: String { opType ?? "" }
  var domainName: String { domain ?? "" }
  var displayName: String { name ?? "" }

  func attribute(_ name: String) -> Attribute? {
    attributes.first { $0.name == name }
  }
}

struct Dim {
  var value: Int64?
  var param: String?
}

struct Shape {
  /// The whole TensorShapeProto field, tag included, written back as it came;
  /// nil for a shape the preparation writes from `dims`.
  let raw: Range<Int>?
  let dims: [Dim]

  /// A static shape the preparation gives a tensor it makes or reshapes.
  static func of(_ dims: [Int64]) -> Shape {
    Shape(raw: nil, dims: dims.map { Dim(value: $0) })
  }
}

struct TensorType {
  var elemType: Int32?
  var shape: Shape?
  var extras: [RawField] = []
}

struct TypeInfo {
  var tensor: TensorType?
  var extras: [RawField] = []
}

struct ValueInfo {
  var name: String?
  var type: TypeInfo?
  var extras: [RawField] = []

  var key: String { name ?? "" }
  var elemType: Int32 { type?.tensor?.elemType ?? 0 }
  var shape: Shape? { type?.tensor?.shape }

  /// A tensor's value info as onnx.helper.make_tensor_value_info writes it:
  /// the name, the element type and a static shape.
  static func tensor(_ name: String, _ elemType: Int32, _ dims: [Int64]) -> ValueInfo {
    ValueInfo(name: name, type: TypeInfo(tensor: TensorType(elemType: elemType, shape: .of(dims))))
  }
}

/// A transposed 2-D weight, produced block by block while it is written.
struct Transpose {
  enum Elements {
    /// Little-endian elements in the source: raw_data, or a packed float or
    /// double field, which has the same layout.
    case source(Range<Int>)
    /// Little-endian elements the preparation holds.
    case owned([UInt8])
    /// A weight in the typed fields that has to be decoded first. Decoded
    /// when it is written, so only one is ever held.
    indirect case typed(Tensor)
  }

  let elements: Elements
  let rows: Int
  let cols: Int
  let elementSize: Int

  var byteCount: Int { rows * cols * elementSize }
}

extension Transpose.Elements {
  /// The elements in `range`, a byte range of these: one filter of a
  /// weight. Typed elements are decoded first, so they cannot be sliced.
  func slice(_ range: Range<Int>) -> Transpose.Elements {
    switch self {
    case .source(let r): .source((r.lowerBound + range.lowerBound)..<(r.lowerBound + range.upperBound))
    case .owned(let b): .owned(Array(b[range]))
    case .typed: preconditionFailure("a weight in the typed fields is decoded before it is sliced")
    }
  }
}

/// An fp16 weight's fp32 copy (`numpy_helper.to_array(w).astype(np.float32)`
/// in heads_in_fp32), produced a band at a time while it is written.
struct Widen {
  enum Elements {
    /// Little-endian fp16 elements in the source: raw_data.
    case source(Range<Int>)
    /// Little-endian fp16 elements the preparation holds.
    case owned([UInt8])
    /// A weight in the typed fields (int32_data holds fp16 bit patterns),
    /// decoded when it is written.
    indirect case typed(Tensor)
    /// A weight the Gemm rewrite transposes, widened band by band as the
    /// transpose produces it.
    indirect case transposed(Transpose)
  }

  let elements: Elements
  /// How many fp16 values the source holds.
  let count: Int

  var sourceByteCount: Int { count * 2 }
  var byteCount: Int { count * 4 }
}

/// A TensorProto. The header is decoded; the data stays where it is.
struct Tensor {
  enum Data {
    case source(Range<Int>)
    case owned([UInt8])
    case transposed(Transpose)
    case widened(Widen)

    var count: Int {
      switch self {
      case .source(let r): r.count
      case .owned(let b): b.count
      case .transposed(let t): t.byteCount
      case .widened(let w): w.byteCount
      }
    }
  }

  var name: String?
  var dims: [Int64] = []
  var dataType: Int32?
  /// raw_data. nil when the field is absent, which is not the same as empty.
  var raw: Data?
  /// The payloads of the typed data fields (float_data 4, int32_data 5,
  /// int64_data 7, double_data 10, uint64_data 11), by field number. Each is
  /// what that field's packed encoding carries, so they are written packed,
  /// as the schema declares, however the source wrote them.
  var typed: [Int: [Range<Int>]] = [:]
  var isExternal = false
  var extras: [RawField] = []

  var key: String { name ?? "" }
  var elementType: Int32 { dataType ?? 0 }
  var elementCount: Int { dims.reduce(1) { $0 * Int($1) } }
}

struct Graph {
  var nodes: [Node] = []
  var name: String?
  var initializers: [Tensor] = []
  var inputs: [ValueInfo] = []
  var outputs: [ValueInfo] = []
  var valueInfo: [ValueInfo] = []
  var sparseInitializers = 0
  var quantizationAnnotations = 0
  var extras: [RawField] = []
}

struct Model {
  var irVersion: Int64?
  var producerName: String?
  var graph: Graph?
  var opsets: [OpsetImport] = []
  var props: [Prop] = []
  var functions: [LocalFunction] = []
  var extras: [RawField] = []
}

// MARK: decoding

enum Decode {
  static func model(_ src: Source) throws -> Model {
    var m = Model()
    var r = src.reader(0..<src.count)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.varint, "ModelProto.ir_version")
        m.irVersion = Int64(bitPattern: f.value)
      case 2:
        try f.expect(.bytes, "ModelProto.producer_name")
        m.producerName = try src.string(f.payload)
      case 7:
        try f.expect(.bytes, "ModelProto.graph")
        guard m.graph == nil else { throw OnnxError("malformed ONNX: the model has two graphs") }
        m.graph = try graph(src, f.payload)
      case 8:
        try f.expect(.bytes, "ModelProto.opset_import")
        m.opsets.append(try opset(src, f))
      case 14:
        try f.expect(.bytes, "ModelProto.metadata_props")
        m.props.append(try prop(src, f))
      case 25:
        try f.expect(.bytes, "ModelProto.functions")
        m.functions.append(try function(src, f))
      default:
        m.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return m
  }

  /// The model and its graph, as a preparation starts from them. A model
  /// whose initializers keep their data in an external file is refused:
  /// no preparation reads one.
  static func preparable(_ src: Source) throws -> (model: Model, graph: Graph) {
    let m = try model(src)
    guard let g = m.graph else { throw OnnxError("the model has no graph") }
    if let t = g.initializers.first(where: \.isExternal) {
      throw OnnxError("initializer \(t.key) keeps its data in an external file, which the preparation does not read")
    }
    return (m, g)
  }

  /// The graph's inputs and outputs and the model's metadata_props, and
  /// nothing else: nodes and initializers are skipped by their lengths, so
  /// the weights are never read.
  static func meta(_ src: Source) throws -> (inputs: [ValueInfo], outputs: [ValueInfo], props: [Prop]) {
    var inputs: [ValueInfo] = []
    var outputs: [ValueInfo] = []
    var props: [Prop] = []
    var r = src.reader(0..<src.count)
    while let f = try r.next() {
      switch f.number {
      case 7:
        try f.expect(.bytes, "ModelProto.graph")
        var g = src.reader(f.payload)
        while let gf = try g.next() {
          switch gf.number {
          case 11:
            try gf.expect(.bytes, "GraphProto.input")
            inputs.append(try valueInfo(src, gf.payload))
          case 12:
            try gf.expect(.bytes, "GraphProto.output")
            outputs.append(try valueInfo(src, gf.payload))
          default:
            break
          }
        }
      case 14:
        try f.expect(.bytes, "ModelProto.metadata_props")
        props.append(try prop(src, f))
      default:
        break
      }
    }
    return (inputs, outputs, props)
  }

  static func graph(_ src: Source, _ range: Range<Int>) throws -> Graph {
    var g = Graph()
    var r = src.reader(range)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.bytes, "GraphProto.node")
        g.nodes.append(try node(src, f.payload))
      case 2:
        try f.expect(.bytes, "GraphProto.name")
        g.name = try src.string(f.payload)
      case 5:
        try f.expect(.bytes, "GraphProto.initializer")
        g.initializers.append(try tensor(src, f.payload))
      case 11:
        try f.expect(.bytes, "GraphProto.input")
        g.inputs.append(try valueInfo(src, f.payload))
      case 12:
        try f.expect(.bytes, "GraphProto.output")
        g.outputs.append(try valueInfo(src, f.payload))
      case 13:
        try f.expect(.bytes, "GraphProto.value_info")
        g.valueInfo.append(try valueInfo(src, f.payload))
      case 14:
        g.quantizationAnnotations += 1
        g.extras.append(RawField(number: f.number, range: f.whole))
      case 15:
        g.sparseInitializers += 1
        g.extras.append(RawField(number: f.number, range: f.whole))
      default:
        g.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return g
  }

  static func node(_ src: Source, _ range: Range<Int>) throws -> Node {
    var n = Node()
    var r = src.reader(range)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.bytes, "NodeProto.input")
        n.inputs.append(try src.string(f.payload))
      case 2:
        try f.expect(.bytes, "NodeProto.output")
        n.outputs.append(try src.string(f.payload))
      case 3:
        try f.expect(.bytes, "NodeProto.name")
        n.name = try src.string(f.payload)
      case 4:
        try f.expect(.bytes, "NodeProto.op_type")
        n.opType = try src.string(f.payload)
      case 5:
        try f.expect(.bytes, "NodeProto.attribute")
        n.attributes.append(try attribute(src, f))
      case 7:
        try f.expect(.bytes, "NodeProto.domain")
        n.domain = try src.string(f.payload)
      default:
        n.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return n
  }

  static func attribute(_ src: Source, _ field: WireField) throws -> Attribute {
    var name = ""
    var type: Int32 = 0
    var i: Int64 = 0
    var float: Float = 0
    var ints: [UInt64] = []
    var s = ""
    var floats: [Float] = []
    var t: Tensor?
    var fields: [WireField] = []
    var r = src.reader(field.payload)
    while let f = try r.next() {
      fields.append(f)
      switch f.number {
      case 1:
        try f.expect(.bytes, "AttributeProto.name")
        name = try src.string(f.payload)
      case 2:
        try f.expect(.fixed32, "AttributeProto.f")
        float = Float(bitPattern: UInt32(truncatingIfNeeded: f.value))
      case 3:
        try f.expect(.varint, "AttributeProto.i")
        i = Int64(bitPattern: f.value)
      case 4:
        try f.expect(.bytes, "AttributeProto.s")
        s = String(decoding: src.slice(f.payload), as: UTF8.self)
      case 5:
        try f.expect(.bytes, "AttributeProto.t")
        t = try tensor(src, f.payload)
      case 7:
        // Not declared packed, but parsers take it packed too.
        if f.wire == .fixed32 {
          floats.append(Float(bitPattern: UInt32(truncatingIfNeeded: f.value)))
        } else {
          try f.expect(.bytes, "AttributeProto.floats")
          var p = src.reader(f.payload)
          while !p.atEnd {
            floats.append(Float(bitPattern: src.slice(try p.take(4)).loadUnaligned(as: UInt32.self).littleEndian))
          }
        }
      case 8:
        try f.appendVarints(to: &ints, src, "AttributeProto.ints")
      case 20:
        try f.expect(.varint, "AttributeProto.type")
        type = Int32(truncatingIfNeeded: f.value)
      default:
        break
      }
    }
    // Almost always the source wrote it the way Python would, and it is
    // copied as it is. Otherwise it is written again the way Python does.
    let bytes: Attribute.Bytes = try Canonical.attribute(fields, src).map { .owned($0) } ?? .source(field.whole)
    return Attribute(
      bytes: bytes, name: name, type: type, i: i, f: float, ints: ints.map { Int64(bitPattern: $0) }, s: s, floats: floats, t: t)
  }

  static func valueInfo(_ src: Source, _ range: Range<Int>) throws -> ValueInfo {
    var vi = ValueInfo()
    var r = src.reader(range)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.bytes, "ValueInfoProto.name")
        vi.name = try src.string(f.payload)
      case 2:
        try f.expect(.bytes, "ValueInfoProto.type")
        guard vi.type == nil else { throw OnnxError("malformed ONNX: a value info carries two types") }
        vi.type = try typeInfo(src, f.payload)
      default:
        vi.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return vi
  }

  static func typeInfo(_ src: Source, _ range: Range<Int>) throws -> TypeInfo {
    var t = TypeInfo()
    var r = src.reader(range)
    while let f = try r.next() {
      if f.number == 1 {
        try f.expect(.bytes, "TypeProto.tensor_type")
        guard t.tensor == nil else { throw OnnxError("malformed ONNX: a type carries two tensor types") }
        t.tensor = try tensorType(src, f.payload)
      } else {
        t.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return t
  }

  static func tensorType(_ src: Source, _ range: Range<Int>) throws -> TensorType {
    var t = TensorType()
    var r = src.reader(range)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.varint, "TypeProto.Tensor.elem_type")
        t.elemType = Int32(truncatingIfNeeded: f.value)
      case 2:
        try f.expect(.bytes, "TypeProto.Tensor.shape")
        guard t.shape == nil else { throw OnnxError("malformed ONNX: a tensor type carries two shapes") }
        t.shape = Shape(raw: f.whole, dims: try dims(src, f.payload))
      default:
        t.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return t
  }

  static func dims(_ src: Source, _ range: Range<Int>) throws -> [Dim] {
    var out: [Dim] = []
    var r = src.reader(range)
    while let f = try r.next() {
      guard f.number == 1 else { continue }
      try f.expect(.bytes, "TensorShapeProto.dim")
      // dim_value and dim_param are a oneof: the last one written is the one set.
      var d = Dim()
      var dr = src.reader(f.payload)
      while let df = try dr.next() {
        switch df.number {
        case 1:
          try df.expect(.varint, "Dimension.dim_value")
          d.value = Int64(bitPattern: df.value)
          d.param = nil
        case 2:
          try df.expect(.bytes, "Dimension.dim_param")
          d.param = try src.string(df.payload)
          d.value = nil
        default:
          break
        }
      }
      out.append(d)
    }
    return out
  }

  static func tensor(_ src: Source, _ range: Range<Int>) throws -> Tensor {
    var t = Tensor()
    var r = src.reader(range)
    while let f = try r.next() {
      switch f.number {
      case 1:
        var values: [UInt64] = []
        try f.appendVarints(to: &values, src, "TensorProto.dims")
        t.dims.append(contentsOf: values.map { Int64(bitPattern: $0) })
      case 2:
        try f.expect(.varint, "TensorProto.data_type")
        t.dataType = Int32(truncatingIfNeeded: f.value)
      case 8:
        try f.expect(.bytes, "TensorProto.name")
        t.name = try src.string(f.payload)
      case 9:
        try f.expect(.bytes, "TensorProto.raw_data")
        t.raw = .source(f.payload)
      case 4, 5, 7, 10, 11:
        // Packed, or a single element written unpacked. Either way the bytes
        // after the tag are what the packed encoding holds for it: a fixed
        // value's bytes, or a varint's.
        let expected: Wire = (f.number == 4) ? .fixed32 : (f.number == 10 ? .fixed64 : .varint)
        guard f.wire == .bytes || f.wire == expected else {
          throw OnnxError("malformed ONNX: TensorProto field \(f.number) has wire type \(f.wire.rawValue)")
        }
        t.typed[f.number, default: []].append(f.payload)
      case 13:
        t.isExternal = true
        t.extras.append(RawField(number: f.number, range: f.whole))
      case 14:
        if f.wire == .varint, f.value == 1 { t.isExternal = true }
        t.extras.append(RawField(number: f.number, range: f.whole))
      default:
        t.extras.append(RawField(number: f.number, range: f.whole))
      }
    }
    return t
  }

  static func opset(_ src: Source, _ field: WireField) throws -> OpsetImport {
    var domain = ""
    var version: Int64 = 0
    var r = src.reader(field.payload)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.bytes, "OperatorSetIdProto.domain")
        domain = try src.string(f.payload)
      case 2:
        try f.expect(.varint, "OperatorSetIdProto.version")
        version = Int64(bitPattern: f.value)
      default:
        break
      }
    }
    return OpsetImport(raw: field.whole, domain: domain, version: version)
  }

  static func prop(_ src: Source, _ field: WireField) throws -> Prop {
    var key = ""
    var value = ""
    var r = src.reader(field.payload)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.bytes, "StringStringEntryProto.key")
        key = try src.string(f.payload)
      case 2:
        try f.expect(.bytes, "StringStringEntryProto.value")
        value = try src.string(f.payload)
      default:
        break
      }
    }
    return Prop(raw: field.whole, key: key, value: value)
  }

  static func function(_ src: Source, _ field: WireField) throws -> LocalFunction {
    var name = ""
    var domain = ""
    var calls: [(opType: String, domain: String)] = []
    var r = src.reader(field.payload)
    while let f = try r.next() {
      switch f.number {
      case 1:
        try f.expect(.bytes, "FunctionProto.name")
        name = try src.string(f.payload)
      case 7:
        try f.expect(.bytes, "FunctionProto.node")
        let n = try node(src, f.payload)
        calls.append((n.op, n.domainName))
      case 10:
        try f.expect(.bytes, "FunctionProto.domain")
        domain = try src.string(f.payload)
      default:
        break
      }
    }
    return LocalFunction(raw: field.whole, name: name, domain: domain, calls: calls)
  }
}

// MARK: encoding

/// Writes the decoded messages back the way Python's protobuf serializes
/// them: known fields in field-number order, repeated scalars packed or not as
/// onnx.proto declares, unknown fields last. A model that Python wrote comes
/// out byte for byte as Python would write it after the same changes.
enum Encode {
  static let modelFields: Set<Int> = [1, 2, 3, 4, 5, 6, 7, 8, 14, 20, 25, 26]
  static let graphFields: Set<Int> = [1, 2, 5, 10, 11, 12, 13, 14, 15, 16]
  static let nodeFields: Set<Int> = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
  static let valueInfoFields: Set<Int> = [1, 2, 3, 4]
  static let typeFields: Set<Int> = [1, 4, 5, 6, 8, 9]
  static let tensorTypeFields: Set<Int> = [1, 2]
  static let tensorFields: Set<Int> = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16]

  static func model(_ m: Model, _ src: Source) -> Encoded {
    var out = Encoded()
    var extras = FieldCursor(m.extras, known: modelFields)
    if let v = m.irVersion { out.intField(1, v) }
    extras.upTo(2, &out, src)
    if let v = m.producerName { out.stringField(2, v) }
    extras.upTo(7, &out, src)
    if let g = m.graph { out.message(7, graph(g, src)) }
    extras.upTo(8, &out, src)
    for o in m.opsets { out.source(o.raw, src) }
    extras.upTo(14, &out, src)
    for p in m.props { prop(p, &out, src) }
    extras.upTo(25, &out, src)
    for f in m.functions { out.source(f.raw, src) }
    extras.rest(&out, src)
    return out
  }

  static func prop(_ p: Prop, _ out: inout Encoded, _ src: Source) {
    if let raw = p.raw {
      out.source(raw, src)
    } else {
      var e = Encoded()
      e.stringField(1, p.key)
      e.stringField(2, p.value)
      out.message(14, e)
    }
  }

  static func graph(_ g: Graph, _ src: Source) -> Encoded {
    var out = Encoded()
    var extras = FieldCursor(g.extras, known: graphFields)
    for n in g.nodes { out.message(1, node(n, src)) }
    extras.upTo(2, &out, src)
    if let v = g.name { out.stringField(2, v) }
    extras.upTo(5, &out, src)
    for t in g.initializers { out.message(5, tensor(t, src)) }
    extras.upTo(11, &out, src)
    for vi in g.inputs { out.message(11, valueInfo(vi, src)) }
    for vi in g.outputs { out.message(12, valueInfo(vi, src)) }
    for vi in g.valueInfo { out.message(13, valueInfo(vi, src)) }
    extras.rest(&out, src)
    return out
  }

  static func node(_ n: Node, _ src: Source) -> Encoded {
    var out = Encoded()
    var extras = FieldCursor(n.extras, known: nodeFields)
    for s in n.inputs { out.stringField(1, s) }
    for s in n.outputs { out.stringField(2, s) }
    if let v = n.name { out.stringField(3, v) }
    if let v = n.opType { out.stringField(4, v) }
    for a in n.attributes {
      switch a.bytes {
      case .source(let r): out.source(r, src)
      case .owned(let b): out.bytesField(5, b)
      }
    }
    extras.upTo(7, &out, src)
    if let v = n.domain { out.stringField(7, v) }
    extras.rest(&out, src)
    return out
  }

  static func valueInfo(_ vi: ValueInfo, _ src: Source) -> Encoded {
    var out = Encoded()
    var extras = FieldCursor(vi.extras, known: valueInfoFields)
    if let v = vi.name { out.stringField(1, v) }
    if let t = vi.type { out.message(2, typeInfo(t, src)) }
    extras.rest(&out, src)
    return out
  }

  static func typeInfo(_ t: TypeInfo, _ src: Source) -> Encoded {
    var out = Encoded()
    var extras = FieldCursor(t.extras, known: typeFields)
    if let tt = t.tensor {
      var inner = Encoded()
      var innerExtras = FieldCursor(tt.extras, known: tensorTypeFields)
      if let e = tt.elemType { inner.intField(1, Int64(e)) }
      if let s = tt.shape { shape(s, &inner, src) }
      innerExtras.rest(&inner, src)
      out.message(1, inner)
    }
    extras.rest(&out, src)
    return out
  }

  /// A shape from the source as it came; one the preparation wrote as
  /// make_tensor_value_info does, each dimension a dim_value.
  static func shape(_ s: Shape, _ out: inout Encoded, _ src: Source) {
    if let raw = s.raw {
      out.source(raw, src)
      return
    }
    var dims = Encoded()
    for d in s.dims {
      var dim = Encoded()
      if let v = d.value {
        dim.intField(1, v)
      } else if let p = d.param {
        dim.stringField(2, p)
      }
      dims.message(1, dim)
    }
    out.message(2, dims)
  }

  static func tensor(_ t: Tensor, _ src: Source) -> Encoded {
    var out = Encoded()
    var extras = FieldCursor(t.extras, known: tensorFields)
    for d in t.dims { out.intField(1, d) }
    if let v = t.dataType { out.intField(2, Int64(v)) }
    extras.upTo(4, &out, src)
    packed(4, t, &out, src)
    packed(5, t, &out, src)
    extras.upTo(7, &out, src)
    packed(7, t, &out, src)
    if let v = t.name { out.stringField(8, v) }
    if let data = t.raw {
      out.tag(9, .bytes)
      out.varint(UInt64(data.count))
      switch data {
      case .source(let r): out.source(r, src)
      case .owned(let b): out.bytes(b)
      case .transposed(let tr): out.transposed(tr)
      case .widened(let w): out.widened(w)
      }
    }
    packed(10, t, &out, src)
    packed(11, t, &out, src)
    extras.rest(&out, src)
    return out
  }

  private static func packed(_ number: Int, _ t: Tensor, _ out: inout Encoded, _ src: Source) {
    guard let ranges = t.typed[number] else { return }
    let length = ranges.reduce(0) { $0 + $1.count }
    // An empty repeated field is not written at all.
    guard length > 0 else { return }
    out.tag(number, .bytes)
    out.varint(UInt64(length))
    for r in ranges { out.source(r, src) }
  }
}

/// Python's protobuf writes a message it parsed in a canonical form: the
/// fields its schema knows in number order, a repeated scalar packed or not as
/// the schema declares it, a singular field once with its last value, and the
/// unknown fields last. For the messages the preparation copies whole, this
/// says whether the source already has that form and, if not, writes it.
enum Canonical {
  static let attributeFields: Set<Int> = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 20, 21, 22, 23]
  /// AttributeProto's singular scalars and strings. Its singular messages
  /// (t, g, tp, sparse_tensor) would merge rather than replace; a source that
  /// repeats one is not handled.
  static let attributeSingular: Set<Int> = [1, 2, 3, 4, 13, 20, 21]

  /// An AttributeProto's payload rewritten, or nil when it is already what
  /// Python would write. floats (7) and ints (8) are not declared packed.
  static func attribute(_ fields: [WireField], _ src: Source) throws -> [UInt8]? {
    try message(fields, src, known: attributeFields, singular: attributeSingular, unpacked: [7: .fixed32, 8: .varint])
  }

  static func message(
    _ fields: [WireField], _ src: Source, known: Set<Int>, singular: Set<Int>,
    unpacked: [Int: Wire]
  ) throws -> [UInt8]? {
    func key(_ f: WireField) -> Int { known.contains(f.number) ? f.number : Int.max }
    var seen = Set<Int>()
    var canonical = true
    var last = 0
    for f in fields {
      let k = key(f)
      if k < last
        || (singular.contains(f.number) && !seen.insert(f.number).inserted)
        || (unpacked[f.number] != nil && f.wire == .bytes)
      {
        canonical = false
        break
      }
      last = k
    }
    if canonical { return nil }

    // A singular field keeps only its last occurrence.
    var lastIndex: [Int: Int] = [:]
    for (n, f) in fields.enumerated() where singular.contains(f.number) {
      lastIndex[f.number] = n
    }
    let kept = fields.enumerated().filter { n, f in !singular.contains(f.number) || lastIndex[f.number] == n }
    let ordered = kept.sorted { (key($0.element), $0.offset) < (key($1.element), $1.offset) }.map(\.element)

    var out = Encoded()
    for f in ordered {
      guard let element = unpacked[f.number], f.wire == .bytes else {
        out.bytes(src.slice(f.whole))
        continue
      }
      // A packed run, written one element per tag.
      var r = src.reader(f.payload)
      while !r.atEnd {
        out.tag(f.number, element)
        switch element {
        case .fixed32: out.bytes(src.slice(try r.take(4)))
        case .fixed64: out.bytes(src.slice(try r.take(8)))
        default: out.varint(try r.varint())
        }
      }
    }
    return out.tail
  }
}
