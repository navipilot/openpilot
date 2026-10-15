import Foundation

/// A driving model prepared for LiteRT (TensorFlow Lite): model.tflite,
/// written on the device from the ONNX file, as every jetlink platform
/// prepares its engine, so custom and uploaded models work the same.
///
/// 1. tinygrad's layout ops stripped, the one ONNX rewrite;
/// 2. every node lowered to TFLite operators (`LiteRTLowering`), keeping
///    ONNX's layout, names and I/O types, in forms LiteRT's GPU runs whole:
///    rank-5 tensors as views of rank 4 at most, constant gathers as slices,
///    LayerNormalization in a form that stays inside fp16;
/// 3. TRANSPOSE pairs the lowering left back to back taken out (the NHWC
///    convolutions meet the graph's own NHWC permutes), TRANSPOSEs moved
///    past elementwise operators to meet the ones that undo them, and
///    RESHAPEs of RESHAPEs joined (the views around rank-5 layout ops meet);
/// 4. the flatbuffer written, then every weight after it, each referenced by
///    offset and size: schema 3c's layout for models over 2 GB, used for every
///    model here so the weights can stream.
///
/// Memory is the point, as in CoreMLPreparation, because a phone runs this.
/// The source is memory-mapped; a weight stays a range of it until it is
/// copied to the file, and a convolution weight that has to be transposed is
/// transposed a filter at a time as it is written. Only the flatbuffer, a
/// few MB, is held. The weights keep their fp16 bytes, so the file is about
/// the size of the ONNX.
public enum LiteRTPreparation {
  public struct Report: Sendable, Equatable {
    public let url: URL
    /// tinygrad layout ops stripped before the lowering.
    public let stripped: Int
    /// The TFLite operators written, by TFLite's name for them.
    public let operators: [String: Int]
    /// The lowering's special cases, by name: fp16-safe LayerNormalizations,
    /// masked Wheres, rank-5 layout ops on views.
    public let lowerings: [String: Int]
    /// TRANSPOSEs taken out after the lowering.
    public let transposesRemoved: Int
    /// TRANSPOSEs moved past an elementwise operator to meet their inverse.
    public let transposesMoved: Int
    /// RESHAPEs that read another RESHAPE's input instead, or went.
    public let reshapesFused: Int
    /// The flatbuffer at the start of the file.
    public let flatbufferBytes: Int
    /// Everything after it: the weights.
    public let weightBytes: Int64

    public var fileBytes: Int64 { Int64(flatbufferBytes) + weightBytes }

    /// What the conversion did, in a line for the log.
    public var summary: String {
      let special = lowerings.sorted { $0.key < $1.key }.map { "\($0.value) \($0.key)" }
      return "\(stripped) tinygrad op(s) stripped; \(special.isEmpty ? "no special cases" : special.joined(separator: ", ")); "
        + "\(operators.values.reduce(0, +)) operators, \(transposesRemoved) transposes removed and \(transposesMoved) moved, "
        + "\(reshapesFused) reshapes fused, \(fileBytes / 1_000_000) MB"
    }
  }

  /// The file's name in the directory.
  public static let fileName = "model.tflite"
  /// Buffers of at least this many bytes go after the flatbuffer.
  static let externalFrom = 1024
  /// Where each buffer after the flatbuffer starts: a multiple of this.
  static let alignment = 64

  public static func prepare(source: URL, into directory: URL) throws -> Report {
    let data = try Data(contentsOf: source, options: .alwaysMapped)
    return try data.withUnsafeBytes { buf in
      try prepare(Source(bytes: buf), into: directory)
    }
  }

  private static func prepare(_ src: Source, into directory: URL) throws -> Report {
    var (model, g) = try Decode.preparable(src)
    let stripped = try Patches.stripTinygradOps(&g, &model.opsets)
    guard let opset = model.opsets.first(where: { $0.domain.isEmpty || $0.domain == "ai.onnx" })?.version else {
      throw OnnxError("the model imports no default-domain opset")
    }

    var lowered = try LiteRTLowering.lower(g, opset: opset, src)
    var removed = 0
    var fused = 0
    var moved = 0
    while true {
      let r = TFLiteOptimizer.cancelTransposes(&lowered)
      let f = TFLiteOptimizer.fuseReshapes(&lowered)
      let m = TFLiteOptimizer.sinkTransposes(&lowered)
      removed += r
      fused += f
      moved += m
      if r + f + m == 0 { break }
    }
    let (tflite, buffers) = TFLiteOptimizer.compact(lowered)

    // The flatbuffer is written with every weight's offset 0, then each
    // offset goes in once the flatbuffer's length says where the weights start.
    let placements = try buffers.map { b -> TFLite.Placement in
      if b.count == 0 { return .empty }
      return b.count < externalFrom ? .inline(try inlineBytes(b, src)) : .external(offset: 0, size: UInt64(b.count))
    }
    var (flatbuffer, offsetFields) = TFLite.encode(tflite, buffers: placements)
    var external: [(index: Int, offset: Int)] = []
    var at = align(flatbuffer.count)
    for i in buffers.indices {
      guard let field = offsetFields[i] else { continue }
      withUnsafeBytes(of: UInt64(at).littleEndian) { flatbuffer.replaceSubrange(field..<(field + 8), with: $0) }
      external.append((i, at))
      at = align(at + buffers[i].count)
    }

    var file = Encoded()
    file.bytes(flatbuffer)
    for (index, offset) in external {
      file.bytes([UInt8](repeating: 0, count: offset - file.count))
      file.append(buffers[index])
    }

    try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
    let url = directory.appendingPathComponent(fileName)
    do {
      try PartWriter.write(file, src, to: url) { _ in }
    } catch {
      try? FileManager.default.removeItem(at: url)
      throw error
    }

    var operators: [String: Int] = [:]
    for o in tflite.operators { operators[o.op.name, default: 0] += 1 }
    return Report(
      url: url, stripped: stripped, operators: operators, lowerings: lowered.counts, transposesRemoved: removed, transposesMoved: moved,
      reshapesFused: fused, flatbufferBytes: flatbuffer.count, weightBytes: Int64(file.count - flatbuffer.count))
  }

  static func align(_ n: Int) -> Int {
    (n + alignment - 1) / alignment * alignment
  }

  /// A small buffer's bytes, for the flatbuffer itself.
  static func inlineBytes(_ e: Encoded, _ src: Source) throws -> [UInt8] {
    var out: [UInt8] = []
    out.reserveCapacity(e.count)
    for piece in e.allPieces {
      switch piece {
      case .bytes(let b): out += b
      case .source(let r): out += src.slice(r)
      case .transposed, .widened: throw OnnxError("a small weight that is transposed or widened is made in memory, not when written")
      }
    }
    return out
  }
}

/// What the lowering leaves to tidy: TRANSPOSEs that undo each other, and
/// tensors and buffers nothing reads any more.
enum TFLiteOptimizer {
  /// Which operator writes each tensor and which read it, by index.
  static func links(_ operators: [TFLite.Operator]) -> (producer: [Int: Int], readers: [Int: [Int]]) {
    var producer: [Int: Int] = [:]
    var readers: [Int: [Int]] = [:]
    for (k, o) in operators.enumerated() {
      for t in o.outputs { producer[t] = k }
      for t in o.inputs where t >= 0 { readers[t, default: []].append(k) }
    }
    return (producer, readers)
  }

  /// Takes out each TRANSPOSE whose input another TRANSPOSE made, composing
  /// the two: into nothing when they undo each other, into one otherwise.
  /// The first goes too once nothing else reads it. Returns how many went.
  static func cancelTransposes(_ l: inout LiteRTLowering) -> Int {
    var removed = 0
    var changed = true
    while changed {
      changed = false
      var (producer, readers) = links(l.model.operators)
      let graphOutputs = Set(l.model.outputs)
      var dead = Set<Int>()
      for k in l.model.operators.indices where !dead.contains(k) {
        let b = l.model.operators[k]
        guard b.op == .transpose, let ka = producer[b.inputs[0]], !dead.contains(ka), l.model.operators[ka].op == .transpose,
          let p = perm(l, l.model.operators[ka]), let q = perm(l, b)
        else { continue }
        let a = l.model.operators[ka]
        let x = a.inputs[0]
        let y = b.inputs[0]
        let z = b.outputs[0]
        let composed = q.map { p[$0] }
        if composed == Array(composed.indices) {
          // z is x: whatever reads z reads x, unless z is a graph output.
          guard !graphOutputs.contains(z) else { continue }
          for r in readers[z] ?? [] {
            l.model.operators[r].inputs = l.model.operators[r].inputs.map { $0 == z ? x : $0 }
            readers[x, default: []].append(r)
          }
          dead.insert(k)
        } else {
          l.model.operators[k].inputs = [x, l.int32Tensor(composed, "\(l.model.tensors[z].name)__perm")]
          readers[x, default: []].append(k)
        }
        readers[y]?.removeAll { $0 == k }
        removed += 1
        if (readers[y] ?? []).isEmpty, !graphOutputs.contains(y) {
          dead.insert(ka)
          removed += 1
        }
        changed = true
      }
      l.model.operators = l.model.operators.enumerated().filter { !dead.contains($0.offset) }.map(\.element)
    }
    return removed
  }

  /// Makes a RESHAPE of a RESHAPE read the first one's input, and takes out
  /// a RESHAPE to the shape its input already has. The views the lowering
  /// puts around rank-5 layout ops meet this way, and the rank-5 tensors
  /// between them stop being read. Returns how many changed.
  static func fuseReshapes(_ l: inout LiteRTLowering) -> Int {
    var changed = 0
    var producer: [Int: Int] = [:]
    for k in l.model.operators.indices {
      let o = l.model.operators[k]
      // Operators are in order, so the producer's own input is already fused.
      if o.op == .reshape, let p = producer[o.inputs[0]], l.model.operators[p].op == .reshape {
        l.model.operators[k].inputs[0] = l.model.operators[p].inputs[0]
        changed += 1
      }
      for t in o.outputs { producer[t] = k }
    }
    let graphOutputs = Set(l.model.outputs)
    var same: [Int: Int] = [:]
    for o in l.model.operators where o.op == .reshape && !graphOutputs.contains(o.outputs[0]) {
      if l.model.tensors[o.inputs[0]].shape == l.model.tensors[o.outputs[0]].shape {
        same[o.outputs[0]] = o.inputs[0]
      }
    }
    guard !same.isEmpty else { return changed }
    func source(_ t: Int) -> Int {
      var t = t
      while let s = same[t] { t = s }
      return t
    }
    for k in l.model.operators.indices {
      l.model.operators[k].inputs = l.model.operators[k].inputs.map { $0 < 0 ? $0 : source($0) }
    }
    // The bypassed RESHAPEs read nothing anyone needs now; compact drops them.
    l.model.operators.removeAll { $0.op == .reshape && same[$0.outputs[0]] != nil }
    return changed + same.count
  }

  static let unaryOps: Set<TFLite.Op> = [.abs, .exp, .gelu, .log, .logistic, .neg, .relu, .rsqrt, .sqrt, .tanh]
  static let binaryOps: Set<TFLite.Op> = [.add, .sub, .mul, .div, .maximum, .minimum]

  /// Moves a TRANSPOSE past the elementwise operator that reads it, so that
  /// it can meet the TRANSPOSE that undoes it further on. A ConvNeXt block
  /// runs NHWC from its depthwise convolution to its last linear, then the
  /// graph permutes back to NCHW for the layer scale and the residual add,
  /// and the next block's convolution permutes to NHWC again; moved past the
  /// multiply and the add, the two meet and cancel, and the blocks chain in
  /// NHWC. A TRANSPOSE only moves when that costs nothing: the elementwise
  /// operator is the only reader of what it makes, the result is no larger,
  /// and every other operand is a constant (read in the new layout), comes
  /// out of the same TRANSPOSE, or is already there untransposed. Returns
  /// how many moved.
  ///
  /// One pass in order: the operators are copied to a new list, each moved
  /// TRANSPOSE written after the operator it moved past, where the operators
  /// that read it, all later, can move it on.
  static func sinkTransposes(_ l: inout LiteRTLowering) -> Int {
    let operators = l.model.operators
    // Readers by operator: its index in `operators`, or past them for the
    // TRANSPOSEs moved here.
    var readers = links(operators).readers
    let graphOutputs = Set(l.model.outputs)
    var out: [TFLite.Operator] = []
    out.reserveCapacity(operators.count)
    // Where in `out` each tensor is made, and where each operator went.
    var producer: [Int: Int] = [:]
    var position: [Int: Int] = [:]
    var dequantizes: [TFLite.Operator] = []
    func append(_ o: TFLite.Operator, _ id: Int) {
      for t in o.outputs { producer[t] = out.count }
      position[id] = out.count
      out.append(o)
    }
    func transposeOf(_ t: Int) -> (input: Int, perm: [Int])? {
      guard let p = producer[t], out[p].op == .transpose, let perm = perm(l, out[p]) else { return nil }
      return (out[p].inputs[0], perm)
    }
    func isConstant(_ t: Int) -> Bool {
      if l.model.tensors[t].buffer > 0 { return true }
      guard let p = producer[t], out[p].op == .dequantize else { return false }
      return l.model.tensors[out[p].inputs[0]].buffer > 0
    }
    func written(_ id: Int) -> TFLite.Operator? {
      position[id].map { out[$0] }
    }

    var moved = 0
    for (k, o) in operators.enumerated() {
      guard unaryOps.contains(o.op) || binaryOps.contains(o.op),
        let s = sink(o, k, l, readers, graphOutputs, transposeOf, isConstant, written)
      else {
        append(o, k)
        continue
      }
      var moving = o
      var inputs = s.inputs
      for (at, shape) in s.constants {
        let t = l.model.tensors[o.inputs[at]]
        if t.buffer > 0 {
          inputs[at] = l.addTensor("\(t.name)__moved", shape, t.type, buffer: t.buffer)
        } else if let p = producer[o.inputs[at]] {
          // The same fp16 bytes, read in the new shape by a DEQUANTIZE of its own.
          let narrow = l.model.tensors[out[p].inputs[0]]
          let narrowMoved = l.addTensor("\(narrow.name)__moved", shape, narrow.type, buffer: narrow.buffer)
          inputs[at] = l.addTensor("\(t.name)__moved", shape, t.type)
          dequantizes.append(TFLite.Operator(op: .dequantize, inputs: [narrowMoved], outputs: [inputs[at]]))
        }
      }
      for t in o.inputs where t >= 0 { readers[t]?.removeAll { $0 == k } }
      for t in inputs where t >= 0 { readers[t, default: []].append(k) }
      let z = l.model.tensors[o.outputs[0]]
      let result = l.addTensor("\(z.name)__moved", inverse(s.perm).map { z.shape[$0] }, z.type)
      moving.inputs = inputs
      moving.outputs = [result]
      append(moving, k)
      let id = operators.count + moved
      readers[result] = [id]
      append(TFLite.Operator(op: .transpose, inputs: [result, l.int32Tensor(s.perm, "\(z.name)__perm")], outputs: o.outputs, options: .transpose), id)
      moved += 1
    }
    l.model.operators = dequantizes + out
    return moved
  }

  private struct Sink {
    /// What the elementwise operator reads in the moved layout.
    let inputs: [Int]
    let perm: [Int]
    /// Constants to read in the moved layout: operand position and shape.
    let constants: [(position: Int, shape: [Int])]
  }

  private static func inverse(_ p: [Int]) -> [Int] {
    var inverse = [Int](repeating: 0, count: p.count)
    for (j, axis) in p.enumerated() { inverse[axis] = j }
    return inverse
  }

  /// How elementwise operator `o` (operator `id`) can read its inputs with a
  /// TRANSPOSE moved past it, or nil if none can move. `written` is an
  /// operator already in the new list, by id.
  private static func sink(
    _ o: TFLite.Operator, _ id: Int, _ l: LiteRTLowering, _ readers: [Int: [Int]], _ graphOutputs: Set<Int>,
    _ transposeOf: (Int) -> (input: Int, perm: [Int])?, _ isConstant: (Int) -> Bool, _ written: (Int) -> TFLite.Operator?
  ) -> Sink? {
    let out = l.model.tensors[o.outputs[0]].shape
    for (i, a) in o.inputs.enumerated() {
      guard let (source, p) = transposeOf(a), readers[a] == [id], !graphOutputs.contains(a), l.model.tensors[a].shape == out else {
        continue
      }
      let back = inverse(p)
      var inputs = o.inputs
      inputs[i] = source
      var constants: [(position: Int, shape: [Int])] = []
      var movable = true
      for (j, b) in o.inputs.enumerated() where j != i {
        if let (bSource, q) = transposeOf(b), q == p {
          inputs[j] = bSource
        } else if let undo = readers[b]?.lazy.compactMap(written).first(where: { $0.op == .transpose && perm(l, $0) == back }) {
          inputs[j] = undo.outputs[0]
        } else if isConstant(b), l.model.tensors[b].shape.count <= p.count {
          let shape = l.model.tensors[b].shape
          let padded = [Int](repeating: 1, count: p.count - shape.count) + shape
          let view = back.map { padded[$0] }
          // Only a view of the same bytes: the axes that are not 1 keep their order.
          guard padded.filter({ $0 != 1 }) == view.filter({ $0 != 1 }) else {
            movable = false
            break
          }
          constants.append((j, view))
        } else {
          movable = false
          break
        }
      }
      if movable { return Sink(inputs: inputs, perm: p, constants: constants) }
    }
    return nil
  }

  /// A TRANSPOSE's permutation, from its constant second input.
  static func perm(_ l: LiteRTLowering, _ o: TFLite.Operator) -> [Int]? {
    guard o.inputs.count == 2 else { return nil }
    let t = l.model.tensors[o.inputs[1]]
    guard t.type == .int32, t.buffer > 0 else { return nil }
    let e = l.buffers[t.buffer]
    guard e.allPieces.count == 1, case .bytes(let b) = e.allPieces[0] else { return nil }
    return try? Elements.integers(b, as: DataType.int32, for: t.name).map { Int($0) }
  }

  /// The model with only what it uses: operators whose outputs something
  /// reads (or the graph returns), and the tensors and buffers those touch,
  /// renumbered in order.
  static func compact(_ l: LiteRTLowering) -> (TFLite.Model, [Encoded]) {
    var model = l.model
    var live = Set(model.outputs)
    var keep = [Bool](repeating: false, count: model.operators.count)
    for k in model.operators.indices.reversed() where model.operators[k].outputs.contains(where: live.contains) {
      keep[k] = true
      live.formUnion(model.operators[k].inputs.filter { $0 >= 0 })
    }
    model.operators = model.operators.enumerated().filter { keep[$0.offset] }.map(\.element)

    var used = Set(model.inputs).union(model.outputs)
    for o in model.operators {
      used.formUnion(o.inputs.filter { $0 >= 0 })
      used.formUnion(o.outputs)
    }
    var tensorIndex: [Int: Int] = [:]
    var bufferIndex: [Int: Int] = [0: 0]
    var tensors: [TFLite.Tensor] = []
    var buffers: [Encoded] = [Encoded()]
    for (i, t) in model.tensors.enumerated() where used.contains(i) {
      var t = t
      if t.buffer > 0 {
        if let b = bufferIndex[t.buffer] {
          t.buffer = b
        } else {
          buffers.append(l.buffers[t.buffer])
          bufferIndex[t.buffer] = buffers.count - 1
          t.buffer = buffers.count - 1
        }
      }
      tensorIndex[i] = tensors.count
      tensors.append(t)
    }
    func remap(_ i: Int) -> Int { i < 0 ? i : tensorIndex[i]! }
    model.tensors = tensors
    model.inputs = model.inputs.map(remap)
    model.outputs = model.outputs.map(remap)
    for k in model.operators.indices {
      model.operators[k].inputs = model.operators[k].inputs.map(remap)
      model.operators[k].outputs = model.operators[k].outputs.map(remap)
    }
    return (model, buffers)
  }
}
