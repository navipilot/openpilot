import Foundation

/// An ONNX graph as TFLite operators, node by node, for LiteRTPreparation.
///
/// The ONNX semantics stay as they are: NCHW, ONNX's broadcasting, the
/// graph's own tensor names. What changes is the arithmetic type: fp16 graph
/// math runs as FLOAT32, as onnx2tf converts it, and LiteRT's GPU runs FLOAT32
/// graphs in fp16 anyway. So a Cast between fp16 and fp32 disappears, an fp16
/// weight is stored FLOAT16 behind one DEQUANTIZE (TFLite's fp16
/// post-training layout, which the CPU and GPU fold back into the weight),
/// and an fp16 graph input or output keeps its type behind a CAST at the
/// edge. uint8 stays uint8 up to the Cast that reads the images.
///
/// Every output's shape is worked out here and checked against the shape the
/// file records for it, so a lowering that goes wrong stops at the node that
/// went wrong rather than at the runtime.
struct LiteRTLowering {
  /// A constant buffer and the type it holds; FLOAT16 is read as FLOAT32
  /// through a DEQUANTIZE.
  typealias Stored = (buffer: Int, type: TFLite.TensorType)

  /// What an ONNX tensor name stands for.
  enum Value {
    /// A TFLite tensor, by index.
    case tensor(Int)
    /// A constant, which becomes a tensor only where an operator reads it.
    case constant(Constant)
  }

  struct Constant {
    enum Bytes {
      /// An initializer, its data still in the source.
      case initializer(Tensor)
      /// Little-endian elements made here: a folded constant.
      case owned([UInt8])
    }

    var name: String
    var dims: [Int]
    /// ONNX's element type.
    var type: Int32
    var bytes: Bytes

    var count: Int { dims.reduce(1, *) }
  }

  /// fp16 constants with at least this many elements are stored FLOAT16
  /// behind a DEQUANTIZE; smaller ones are widened to FLOAT32 in place
  /// (what the spike's fp16_weights.py did, after TFLite's converter).
  static let fp16MinElements = 1024
  /// An infinite Where fill becomes the largest finite fp16 instead: LiteRT's
  /// GPU runs fp16 kernels compiled with fast math, where infinities are not
  /// promised to behave.
  static let finiteFill: Float = 65504

  let src: Source
  /// The default domain's opset, for the ops whose defaults changed.
  let opset: Int64
  var model = TFLite.Model()
  /// Each buffer's bytes; 0 is the empty buffer.
  var buffers: [Encoded] = [Encoded()]
  /// The DEQUANTIZEs of the fp16 weights, which run before everything else.
  var prologue: [TFLite.Operator] = []
  var values: [String: Value] = [:]
  /// The shapes the file records, to check the lowering against.
  private let recorded: [String: [Int64]]
  /// Each name's readers, by node index.
  var readerIndices: [String: [Int]] = [:]
  var nodes: [Node] = []
  var names = Set<String>()
  /// A constant's tensor, by name and shape, so each is made once.
  var constantTensors: [String: Int] = [:]
  /// A constant's buffer, by name, so one read at two shapes is stored once.
  var constantBuffers: [String: Stored] = [:]
  /// Each INT32 constant, by its values and shape, so each is stored once.
  var int32Tensors: [[[Int]]: Int] = [:]
  /// How often each special case fired, for the report: masks, views.
  var counts: [String: Int] = [:]

  private init(_ src: Source, opset: Int64, recorded: [String: [Int64]]) {
    self.src = src
    self.opset = opset
    self.recorded = recorded
  }

  /// Lowers `g`: its inputs, every node in order, its outputs.
  static func lower(_ g: Graph, opset: Int64, _ src: Source) throws -> LiteRTLowering {
    var l = LiteRTLowering(src, opset: opset, recorded: Patches.staticDims(g))
    l.nodes = g.nodes
    for (k, n) in g.nodes.enumerated() {
      for name in n.inputs where !name.isEmpty {
        l.readerIndices[name, default: []].append(k)
      }
    }
    for t in g.initializers {
      l.values[t.key] = .constant(Constant(name: t.key, dims: t.dims.map { Int($0) }, type: t.elementType, bytes: .initializer(t)))
    }
    let initializers = Set(g.initializers.map(\.key))
    for vi in g.inputs where !initializers.contains(vi.key) {
      try l.input(vi)
    }
    for n in g.nodes {
      do {
        try l.lower(n)
      } catch let e as OnnxError {
        throw OnnxError("\(n.op) \(pyRepr(n.displayName)): \(e.message)")
      }
    }
    for vi in g.outputs {
      try l.output(vi)
    }
    l.model.operators = l.prologue + l.model.operators
    l.prologue = []
    return l
  }

  // MARK: the graph's edges

  private mutating func input(_ vi: ValueInfo) throws {
    guard let shape = vi.shape else { throw OnnxError("input \(vi.key) has no shape") }
    let dims = try shape.dims.map { d -> Int in
      guard let v = d.value, v > 0 else { throw OnnxError("input \(vi.key) has a dimension that is not fixed") }
      return Int(v)
    }
    let edge = try Self.edgeType(vi.elemType, vi.key)
    let view = Self.edgeView(dims)
    let index = addTensor(vi.key, view, edge)
    model.inputs.append(index)
    var t = index
    if edge == .float16 {
      // The graph's math is FLOAT32; the input keeps its type for whoever feeds it.
      t = addTensor("\(vi.key)__f32", view, .float32)
      emit(.cast, [index], [t], .cast(from: .float16, to: .float32))
    }
    if view != dims {
      t = reshaped(t, dims, "\(vi.key)__nd")
      count("graph inputs and outputs as 4-D views")
    }
    values[vi.key] = .tensor(t)
  }

  private mutating func output(_ vi: ValueInfo) throws {
    let name = vi.key
    let type = try Self.edgeType(vi.elemType, name)
    var index = try tensor(name)
    let view = Self.edgeView(model.tensors[index].shape)
    if view != model.tensors[index].shape {
      if model.tensors[index].name == name { rename(index, to: "\(name)__nd") }
      index = reshaped(index, view, name)
      count("graph inputs and outputs as 4-D views")
    }
    if model.tensors[index].type != type {
      guard model.tensors[index].type == .float32, type == .float16 else {
        throw OnnxError("output \(name) is \(model.tensors[index].type.name) where the model says \(type.name)")
      }
      if model.tensors[index].name == name { rename(index, to: "\(name)__f32") }
      let narrow = addTensor(name, model.tensors[index].shape, .float16)
      emit(.cast, [index], [narrow], .cast(from: .float32, to: .float16))
      index = narrow
    } else if model.tensors[index].name != name || model.inputs.contains(index) || model.outputs.contains(index) {
      if !model.inputs.contains(index), !model.outputs.contains(index), !names.contains(name) {
        rename(index, to: name)
      } else {
        // An input handed straight back, or one tensor behind two outputs:
        // the output needs a tensor of its own.
        let shape = model.tensors[index].shape
        let copy = addTensor(name, shape, type)
        emit(.reshape, [index, int32Tensor(shape, "\(name)__shape")], [copy], .reshape(newShape: shape.map { Int32($0) }))
        index = copy
      }
    }
    if let want = vi.shape?.dims.map({ $0.value ?? -1 }), want.reduce(1, *) != model.tensors[index].elementCount {
      throw OnnxError("output \(name) has \(model.tensors[index].elementCount) elements where the model says \(want)")
    }
    model.outputs.append(index)
  }

  /// The shape a graph input or output has in the file. LiteRT's GPU takes
  /// rank 4 at most, so a larger one is the same bytes as
  /// [d0, d1 * ... * d(r-3), d(r-2), d(r-1)]: the uint8 frame queue
  /// [2, 5, 6, 128, 256] is [2, 30, 128, 256]. Element counts are what the
  /// server checks, and they stay.
  static func edgeView(_ dims: [Int]) -> [Int] {
    guard dims.count > 4 else { return dims }
    return [dims[0], dims[1...(dims.count - 3)].reduce(1, *), dims[dims.count - 2], dims[dims.count - 1]]
  }

  /// The TFLite type a graph input or output keeps.
  private static func edgeType(_ type: Int32, _ name: String) throws -> TFLite.TensorType {
    switch type {
    case DataType.float: .float32
    case DataType.float16: .float16
    case DataType.uint8: .uint8
    case DataType.int8: .int8
    case DataType.int32: .int32
    case DataType.bool: .bool
    default: throw OnnxError("\(name) has element type \(OnnxMeta.typeName(type)), which LiteRT's graph inputs and outputs do not take")
    }
  }

  /// The TFLite type an ONNX tensor's values are computed in.
  static func computeType(_ type: Int32) throws -> TFLite.TensorType {
    switch type {
    case DataType.float, DataType.float16, DataType.double, DataType.bfloat16: .float32
    case DataType.uint8: .uint8
    case DataType.int8: .int8
    case DataType.int16: .int16
    case DataType.int32, DataType.int64: .int32
    case DataType.bool: .bool
    default: throw OnnxError("element type \(OnnxMeta.typeName(type)) has no TFLite equivalent here")
    }
  }

  // MARK: tensors and operators

  @discardableResult
  mutating func addTensor(_ name: String, _ shape: [Int], _ type: TFLite.TensorType, buffer: Int = 0) -> Int {
    model.tensors.append(TFLite.Tensor(name: uniqueName(name), shape: shape, type: type, buffer: buffer))
    return model.tensors.count - 1
  }

  mutating func rename(_ index: Int, to name: String) {
    names.remove(model.tensors[index].name)
    model.tensors[index].name = uniqueName(name)
  }

  /// `name`, or `name__1`, `name__2`... if a tensor has it, taken.
  private mutating func uniqueName(_ name: String) -> String {
    var unique = name
    var n = 1
    while names.contains(unique) {
      unique = "\(name)__\(n)"
      n += 1
    }
    names.insert(unique)
    return unique
  }

  mutating func emit(_ op: TFLite.Op, _ inputs: [Int], _ outputs: [Int], _ options: TFLite.Options = .none) {
    model.operators.append(TFLite.Operator(op: op, inputs: inputs, outputs: outputs, options: options))
  }

  mutating func buffer(_ e: Encoded) -> Int {
    buffers.append(e)
    return buffers.count - 1
  }

  /// A buffer of owned bytes.
  mutating func buffer(_ bytes: [UInt8]) -> Int {
    var e = Encoded()
    e.bytes(bytes)
    return buffer(e)
  }

  /// A constant INT32 vector: shapes, axes, permutations, slice bounds.
  mutating func int32Tensor(_ values: [Int], _ name: String, shape: [Int]? = nil) -> Int {
    let shape = shape ?? [values.count]
    if let i = int32Tensors[[values, shape]] { return i }
    var bytes: [UInt8] = []
    bytes.reserveCapacity(values.count * 4)
    for v in values {
      withUnsafeBytes(of: Int32(truncatingIfNeeded: v).littleEndian) { bytes.append(contentsOf: $0) }
    }
    let i = addTensor(name, shape, .int32, buffer: buffer(bytes))
    int32Tensors[[values, shape]] = i
    return i
  }

  /// A constant FLOAT32 tensor of `values`.
  mutating func floatTensor(_ values: [Float], _ shape: [Int], _ name: String) -> Int {
    storedTensor(floatBuffer(values), shape, name)
  }

  /// `values` stored as FLOAT32, or as FLOAT16 when there are many and
  /// every one is exact in fp16.
  mutating func floatBuffer(_ values: [Float]) -> Stored {
    if values.count >= Self.fp16MinElements, values.allSatisfy({ Float(Float16($0)) == $0 }) {
      return (buffer(Elements.encode(values, as: DataType.float16)), .float16)
    }
    return (buffer(Elements.encode(values, as: DataType.float)), .float32)
  }

  /// A tensor over a stored buffer. A FLOAT16 one is read as FLOAT32
  /// through a DEQUANTIZE that runs before everything else.
  mutating func storedTensor(_ stored: Stored, _ shape: [Int], _ name: String) -> Int {
    guard stored.type == .float16 else { return addTensor(name, shape, stored.type, buffer: stored.buffer) }
    let narrow = addTensor("\(name)__fp16", shape, .float16, buffer: stored.buffer)
    let wide = addTensor(name, shape, .float32)
    prologue.append(TFLite.Operator(op: .dequantize, inputs: [narrow], outputs: [wide]))
    return wide
  }

  // MARK: reading values

  func value(_ name: String) throws -> Value {
    guard let v = values[name] else { throw OnnxError("\(name) is read before anything produces it") }
    return v
  }

  func dims(_ v: Value) -> [Int] {
    switch v {
    case .tensor(let i): model.tensors[i].shape
    case .constant(let c): c.dims
    }
  }

  func dims(_ name: String) throws -> [Int] { dims(try value(name)) }

  func constant(_ name: String) -> Constant? {
    if case .constant(let c) = values[name] { return c }
    return nil
  }

  /// A name as a tensor operators can read; a constant is stored, once per shape.
  mutating func tensor(_ name: String) throws -> Int {
    try tensor(value(name))
  }

  mutating func tensor(_ v: Value) throws -> Int {
    switch v {
    case .tensor(let i): return i
    case .constant(let c): return try constantTensor(c)
    }
  }

  /// A constant as a TFLite tensor in its compute type. A scalar becomes [1].
  mutating func constantTensor(_ c: Constant) throws -> Int {
    let shape = c.dims.isEmpty ? [1] : c.dims
    let key = "\(c.name)\u{0}\(shape)"
    if let i = constantTensors[key] { return i }
    let index: Int
    switch c.type {
    case DataType.int32, DataType.int64, DataType.int16, DataType.uint16, DataType.uint32:
      let ints = try integers(c)
      guard ints.allSatisfy({ $0 >= Int64(Int32.min) && $0 <= Int64(Int32.max) }) else {
        throw OnnxError("constant \(c.name) does not fit INT32")
      }
      index = int32Tensor(ints.map { Int($0) }, c.name, shape: shape)
    default:
      let stored = try constantBuffers[c.name] ?? buffer(for: c)
      constantBuffers[c.name] = stored
      index = storedTensor(stored, shape, c.name)
    }
    constantTensors[key] = index
    return index
  }

  /// A constant's buffer: an fp16 weight's own bytes, small floats widened
  /// to FLOAT32, bytes and booleans as they are.
  private mutating func buffer(for c: Constant) throws -> Stored {
    switch c.type {
    case DataType.float16 where c.count >= Self.fp16MinElements: return (buffer(try stored(c, elementSize: 2)), .float16)
    case DataType.float16, DataType.double, DataType.bfloat16: return floatBuffer(try floats(c))
    case DataType.float: return (buffer(try stored(c, elementSize: 4)), .float32)
    case DataType.uint8, DataType.int8, DataType.bool: return (buffer(try stored(c, elementSize: 1)), try Self.computeType(c.type))
    default: throw OnnxError("constant \(c.name) has element type \(OnnxMeta.typeName(c.type)), which is not lowered")
    }
  }

  /// A constant's bytes as a buffer: a range of the source where the
  /// initializer has raw_data there, so a weight is never copied into memory.
  func stored(_ c: Constant, elementSize: Int) throws -> Encoded {
    var e = Encoded()
    switch c.bytes {
    case .owned(let b):
      guard b.count == c.count * elementSize else { throw OnnxError("constant \(c.name) has \(b.count) bytes for \(c.count) elements") }
      e.bytes(b)
    case .initializer(let t):
      if case .source(let r)? = t.raw {
        guard r.count == c.count * elementSize else {
          throw OnnxError("initializer \(c.name) has \(r.count) bytes of data where its dims need \(c.count * elementSize)")
        }
        e.source(r, src)
      } else {
        e.bytes(try Elements.littleEndian(t, src))
      }
    }
    return e
  }

  /// A constant's little-endian bytes, in memory: for the small constants
  /// the lowering reads or folds.
  func bytes(_ c: Constant) throws -> [UInt8] {
    switch c.bytes {
    case .owned(let b): return b
    case .initializer(let t): return try Elements.littleEndian(t, src)
    }
  }

  func floats(_ c: Constant) throws -> [Float] {
    if DataType.isInteger(c.type) || c.type == DataType.bool {
      return try integers(c).map { Float($0) }
    }
    return try Elements.floats(bytes(c), as: c.type, for: "constant \(c.name)")
  }

  func integers(_ c: Constant) throws -> [Int64] {
    if c.type == DataType.bool {
      return try bytes(c).map { $0 == 0 ? 0 : 1 }
    }
    return try Elements.integers(bytes(c), as: c.type, for: "constant \(c.name)")
  }

  /// A constant input read as integers: shapes, axes, slice bounds.
  func constantInts(_ node: Node, _ i: Int) throws -> [Int]? {
    guard i < node.inputs.count, !node.inputs[i].isEmpty else { return nil }
    guard let c = constant(node.inputs[i]) else {
      throw OnnxError("input \(i) (\(node.inputs[i])) is computed at run time; only a constant is lowered")
    }
    return try integers(c).map { Int(clamping: $0) }
  }

  // MARK: defining outputs

  /// A new tensor for node output `name`, checked against the recorded shape.
  mutating func define(_ name: String, _ shape: [Int], _ type: TFLite.TensorType) throws -> Int {
    try check(name, shape)
    let i = addTensor(name, shape, type)
    values[name] = .tensor(i)
    return i
  }

  mutating func alias(_ name: String, _ v: Value) throws {
    try check(name, dims(v))
    values[name] = v
  }

  mutating func defineConstant(_ name: String, _ dims: [Int], _ type: Int32, _ bytes: [UInt8]) throws {
    try check(name, dims)
    values[name] = .constant(Constant(name: name, dims: dims, type: type, bytes: .owned(bytes)))
  }

  func check(_ name: String, _ shape: [Int]) throws {
    guard let want = recorded[name], !want.contains(where: { $0 < 0 }) else { return }
    guard want.map({ Int($0) }) == shape else {
      throw OnnxError("lowered \(name) to shape \(shape) where the model records \(want)")
    }
  }

  func recordedDims(_ name: String) -> [Int64]? { recorded[name] }

  /// The nodes that read `name`.
  func readers(of name: String) -> [Node] {
    (readerIndices[name] ?? []).map { nodes[$0] }
  }

  mutating func count(_ key: String) {
    counts[key, default: 0] += 1
  }
}
