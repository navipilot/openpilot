import Foundation
import JetlinkTestSupport

@testable import JetlinkONNX

/// Reads a FlatBuffer the way the generated readers do, written apart from
/// FlatBufferBuilder so the tests do not check the builder against itself.
struct FlatBufferReader {
  let bytes: [UInt8]

  func u8(_ at: Int) -> UInt8 { bytes[at] }
  func u16(_ at: Int) -> UInt16 { UInt16(bytes[at]) | UInt16(bytes[at + 1]) << 8 }
  func u32(_ at: Int) -> UInt32 { (0..<4).reduce(UInt32(0)) { $0 | UInt32(bytes[at + $1]) << (8 * UInt32($1)) } }
  func u64(_ at: Int) -> UInt64 { (0..<8).reduce(UInt64(0)) { $0 | UInt64(bytes[at + $1]) << (8 * UInt64($1)) } }
  func i32(_ at: Int) -> Int32 { Int32(bitPattern: u32(at)) }

  var root: Table { Table(reader: self, at: Int(u32(0))) }
  var identifier: String { String(decoding: bytes[4..<8], as: UTF8.self) }

  struct Table {
    let reader: FlatBufferReader
    let at: Int

    var vtable: Int { at - Int(reader.i32(at)) }

    /// Where a field's value is, nil when the vtable leaves it out.
    func field(_ slot: Int) -> Int? {
      let entry = 4 + 2 * slot
      guard entry < Int(reader.u16(vtable)) else { return nil }
      let offset = Int(reader.u16(vtable + entry))
      return offset == 0 ? nil : at + offset
    }

    func u8(_ slot: Int, _ fallback: UInt8 = 0) -> UInt8 { field(slot).map(reader.u8) ?? fallback }
    func i8(_ slot: Int, _ fallback: Int8 = 0) -> Int8 { field(slot).map { Int8(bitPattern: reader.u8($0)) } ?? fallback }
    func u32(_ slot: Int, _ fallback: UInt32 = 0) -> UInt32 { field(slot).map(reader.u32) ?? fallback }
    func i32(_ slot: Int, _ fallback: Int32 = 0) -> Int32 { field(slot).map(reader.i32) ?? fallback }
    func u64(_ slot: Int, _ fallback: UInt64 = 0) -> UInt64 { field(slot).map(reader.u64) ?? fallback }
    func f32(_ slot: Int, _ fallback: Float = 0) -> Float { field(slot).map { Float(bitPattern: reader.u32($0)) } ?? fallback }
    func bool(_ slot: Int) -> Bool { u8(slot) != 0 }

    private func target(_ slot: Int) -> Int? {
      field(slot).map { $0 + Int(reader.u32($0)) }
    }

    func table(_ slot: Int) -> Table? { target(slot).map { Table(reader: reader, at: $0) } }

    /// A vector's elements' start and count.
    func vector(_ slot: Int) -> (start: Int, count: Int)? {
      target(slot).map { ($0 + 4, Int(reader.u32($0))) }
    }

    func string(_ slot: Int) -> String? {
      vector(slot).map { String(decoding: reader.bytes[$0.start..<($0.start + $0.count)], as: UTF8.self) }
    }

    func int32s(_ slot: Int) -> [Int32] {
      guard let v = vector(slot) else { return [] }
      return (0..<v.count).map { reader.i32(v.start + 4 * $0) }
    }

    func bytes(_ slot: Int) -> [UInt8]? {
      vector(slot).map { Array(reader.bytes[$0.start..<($0.start + $0.count)]) }
    }

    func tables(_ slot: Int) -> [Table] {
      guard let v = vector(slot) else { return [] }
      return (0..<v.count).map { k in
        let at = v.start + 4 * k
        return Table(reader: reader, at: at + Int(reader.u32(at)))
      }
    }
  }
}

/// A .tflite file decoded with FlatBufferReader into what the tests check.
struct TFLiteFile {
  struct Tensor {
    let name: String
    let shape: [Int]
    let signature: [Int]
    let type: Int8
    let buffer: Int
  }

  struct Operator {
    let op: String
    let code: Int32
    let deprecatedCode: Int8
    let version: Int32
    let inputs: [Int]
    let outputs: [Int]
    let optionsType: UInt8
    let options: FlatBufferReader.Table?
  }

  enum Buffer {
    case empty
    case inline([UInt8])
    case external(offset: Int, size: Int)
  }

  let bytes: [UInt8]
  let version: UInt32
  let tensors: [Tensor]
  let operators: [Operator]
  let inputs: [Int]
  let outputs: [Int]
  let buffers: [Buffer]
  let signatureKey: String?
  let signatureInputs: [(String, Int)]
  let signatureOutputs: [(String, Int)]

  init(_ url: URL) throws {
    try self.init(bytes: [UInt8](Data(contentsOf: url)))
  }

  init(bytes: [UInt8]) throws {
    self.bytes = bytes
    let r = FlatBufferReader(bytes: bytes)
    guard r.identifier == "TFL3" else { throw TestError("not a TFLite file: \(r.identifier)") }
    let model = r.root
    version = model.u32(0)
    let names = Dictionary(uniqueKeysWithValues: TFLite.Op.allCases.map { ($0.rawValue, $0.name) })
    let codes = model.tables(1).map { t in (code: t.i32(3), deprecated: t.i8(0), version: t.i32(2, 1)) }
    let subgraph = model.tables(2)[0]
    tensors = subgraph.tables(0).map { t in
      Tensor(
        name: t.string(3) ?? "", shape: t.int32s(0).map { Int($0) }, signature: t.int32s(7).map { Int($0) }, type: t.i8(1),
        buffer: Int(t.u32(2)))
    }
    inputs = subgraph.int32s(1).map { Int($0) }
    outputs = subgraph.int32s(2).map { Int($0) }
    operators = subgraph.tables(3).map { o in
      let c = codes[Int(o.u32(0))]
      return Operator(
        op: names[c.code] ?? "\(c.code)", code: c.code, deprecatedCode: c.deprecated, version: c.version,
        inputs: o.int32s(1).map { Int($0) }, outputs: o.int32s(2).map { Int($0) }, optionsType: o.u8(3), options: o.table(4))
    }
    buffers = model.tables(4).map { b in
      if let data = b.bytes(0) { return .inline(data) }
      if b.u64(1) > 1 { return .external(offset: Int(b.u64(1)), size: Int(b.u64(2))) }
      return .empty
    }
    let signature = model.tables(7).first
    signatureKey = signature?.string(2)
    signatureInputs = signature?.tables(0).map { ($0.string(0) ?? "", Int($0.u32(1))) } ?? []
    signatureOutputs = signature?.tables(1).map { ($0.string(0) ?? "", Int($0.u32(1))) } ?? []
  }

  /// A constant tensor's bytes, wherever its buffer keeps them.
  func data(_ tensor: Int) -> [UInt8]? {
    switch buffers[tensors[tensor].buffer] {
    case .empty: return nil
    case .inline(let b): return b
    case .external(let offset, let size): return Array(bytes[offset..<(offset + size)])
    }
  }

  func tensor(named name: String) -> Int? {
    tensors.firstIndex { $0.name == name }
  }

  var opNames: [String] { operators.map(\.op) }

  func count(_ op: String) -> Int { operators.filter { $0.op == op }.count }
}

/// A plain fp32 interpreter for the operators LiteRTPreparation writes,
/// following TFLite's reference kernels, so a lowering can be checked by
/// what it computes. Small graphs only: every loop is the obvious one.
struct TFLiteInterpreter {
  let file: TFLiteFile
  /// Every tensor's values, as Float (uint8, int32 and bool included).
  var values: [Int: [Float]] = [:]

  init(_ file: TFLiteFile) {
    self.file = file
  }

  static func strides(_ shape: [Int]) -> [Int] {
    var s = [Int](repeating: 1, count: shape.count)
    var acc = 1
    for i in stride(from: shape.count - 1, through: 0, by: -1) {
      s[i] = acc
      acc *= shape[i]
    }
    return s
  }

  /// Every multi-index of `shape`, in row-major order.
  static func indices(_ shape: [Int]) -> [[Int]] {
    let count = shape.reduce(1, *)
    let s = strides(shape)
    return (0..<count).map { flat in shape.indices.map { (flat / s[$0]) % shape[$0] } }
  }

  func constant(_ t: Int) throws -> [Float]? {
    guard let b = file.data(t) else { return nil }
    switch file.tensors[t].type {
    case TFLite.TensorType.float32.rawValue: return try Elements.floats(b, as: DataType.float, for: file.tensors[t].name)
    case TFLite.TensorType.float16.rawValue: return try Elements.floats(b, as: DataType.float16, for: file.tensors[t].name)
    case TFLite.TensorType.int32.rawValue: return try Elements.integers(b, as: DataType.int32, for: file.tensors[t].name).map { Float($0) }
    default: return b.map { Float($0) }
    }
  }

  func get(_ t: Int) throws -> [Float] {
    if let v = values[t] { return v }
    if let c = try constant(t) { return c }
    throw TestError("tensor \(file.tensors[t].name) is read before it is written")
  }

  func ints(_ t: Int) throws -> [Int] { try get(t).map { Int($0) } }

  /// Runs every operator on `inputs`, by input name; returns the outputs by name.
  mutating func run(_ inputs: [String: [Float]]) throws -> [String: [Float]] {
    for i in file.inputs {
      guard let v = inputs[file.tensors[i].name] else { throw TestError("no value for input \(file.tensors[i].name)") }
      values[i] = v
    }
    for o in file.operators {
      values[o.outputs[0]] = try step(o)
    }
    var out: [String: [Float]] = [:]
    for i in file.outputs { out[file.tensors[i].name] = try get(i) }
    return out
  }

  func shape(_ t: Int) -> [Int] { file.tensors[t].shape }

  func step(_ o: TFLiteFile.Operator) throws -> [Float] {
    let x = try get(o.inputs[0])
    let inShape = shape(o.inputs[0])
    let outShape = shape(o.outputs[0])
    switch o.op {
    case "ADD", "SUB", "MUL", "DIV", "MAXIMUM", "MINIMUM", "POW":
      let f: (Float, Float) -> Float =
        switch o.op {
        case "ADD": (+)
        case "SUB": (-)
        case "MUL": (*)
        case "DIV": (/)
        case "MAXIMUM": { max($0, $1) }
        case "MINIMUM": { min($0, $1) }
        default: { Float(pow(Double($0), Double($1))) }
        }
      let a = broadcast(x, inShape, outShape)
      let b = broadcast(try get(o.inputs[1]), shape(o.inputs[1]), outShape)
      return zip(a, b).map(f)
    case "SELECT_V2":
      let c = broadcast(x, inShape, outShape)
      let a = broadcast(try get(o.inputs[1]), shape(o.inputs[1]), outShape)
      let b = broadcast(try get(o.inputs[2]), shape(o.inputs[2]), outShape)
      return c.indices.map { c[$0] != 0 ? a[$0] : b[$0] }
    case "ABS": return x.map { abs($0) }
    case "SQRT": return x.map { $0.squareRoot() }
    case "RSQRT": return x.map { 1 / $0.squareRoot() }
    case "LOGISTIC": return x.map { 1 / (1 + Float(exp(-Double($0)))) }
    case "RELU": return x.map { max($0, 0) }
    case "TANH": return x.map { Float(tanh(Double($0))) }
    case "EXP": return x.map { Float(exp(Double($0))) }
    case "LOG": return x.map { Float(log(Double($0))) }
    case "NEG": return x.map { -$0 }
    case "LOGICAL_NOT": return x.map { $0 == 0 ? 1 : 0 }
    case "GELU":
      let approximate = o.options?.bool(0) ?? false
      return x.map { v in
        approximate
          ? 0.5 * v * (1 + Float(tanh(Double(0.797_884_6 * (v + 0.044_715 * v * v * v)))))
          : 0.5 * v * (1 + Float(erf(Double(v) / 2.0.squareRoot())))
      }
    case "CAST":
      if file.tensors[o.outputs[0]].type == TFLite.TensorType.float16.rawValue { return x.map { Float(Float16($0)) } }
      if file.tensors[o.outputs[0]].type == TFLite.TensorType.int32.rawValue { return x.map { $0.rounded(.towardZero) } }
      return x
    case "DEQUANTIZE", "RESHAPE":
      return x
    case "TRANSPOSE":
      let perm = try ints(o.inputs[1])
      let s = Self.strides(inShape)
      return Self.indices(outShape).map { idx in x[zip(idx, perm).reduce(0) { $0 + $1.0 * s[$1.1] }] }
    case "SLICE", "STRIDED_SLICE":
      let begin = try ints(o.inputs[1])
      let step = o.op == "SLICE" ? [Int](repeating: 1, count: inShape.count) : try ints(o.inputs[3])
      let s = Self.strides(inShape)
      return Self.indices(outShape).map { idx in x[idx.indices.reduce(0) { $0 + (begin[$1] + idx[$1] * step[$1]) * s[$1] }] }
    case "CONCATENATION":
      let axis = Int(o.options?.i32(0) ?? 0)
      let parts = try o.inputs.map { (try get($0), shape($0)) }
      return Self.indices(outShape).map { idx in
        var at = idx[axis]
        for (v, sh) in parts {
          if at < sh[axis] {
            var j = idx
            j[axis] = at
            return v[zip(j, Self.strides(sh)).reduce(0) { $0 + $1.0 * $1.1 }]
          }
          at -= sh[axis]
        }
        return .nan
      }
    case "PAD":
      let pads = try ints(o.inputs[1])
      let s = Self.strides(inShape)
      return Self.indices(outShape).map { idx in
        var flat = 0
        for k in idx.indices {
          let i = idx[k] - pads[2 * k]
          if i < 0 || i >= inShape[k] { return 0 }
          flat += i * s[k]
        }
        return x[flat]
      }
    case "MEAN", "SUM", "REDUCE_MAX", "REDUCE_MIN":
      let axes = Set(try ints(o.inputs[1]))
      let kept = inShape.enumerated().map { axes.contains($0.offset) ? 1 : $0.element }
      var groups = [[Float]](repeating: [], count: kept.reduce(1, *))
      let ks = Self.strides(kept)
      for (flat, idx) in Self.indices(inShape).enumerated() {
        groups[idx.indices.reduce(0) { $0 + (axes.contains($1) ? 0 : idx[$1]) * ks[$1] }].append(x[flat])
      }
      return groups.map { g in
        switch o.op {
        case "MEAN": g.reduce(0, +) / Float(g.count)
        case "SUM": g.reduce(0, +)
        case "REDUCE_MAX": g.max()!
        default: g.min()!
        }
      }
    case "SOFTMAX":
      let n = inShape.last!
      return stride(from: 0, to: x.count, by: n).flatMap { r -> [Float] in
        let row = x[r..<(r + n)]
        let m = row.max()!
        let e = row.map { Float(exp(Double($0 - m))) }
        let total = e.reduce(0, +)
        return e.map { $0 / total }
      }
    case "FULLY_CONNECTED":
      let w = try get(o.inputs[1])
      let units = shape(o.inputs[1])[0]
      let k = shape(o.inputs[1])[1]
      let bias = o.inputs.count > 2 && o.inputs[2] >= 0 ? try get(o.inputs[2]) : [Float](repeating: 0, count: units)
      return (0..<(x.count / k)).flatMap { r in (0..<units).map { u in (0..<k).reduce(bias[u]) { $0 + x[r * k + $1] * w[u * k + $1] } } }
    case "BATCH_MATMUL":
      let y = try get(o.inputs[1])
      let ys = shape(o.inputs[1])
      let adjY = o.options?.bool(1) ?? false
      let (m, k) = (inShape[inShape.count - 2], inShape[inShape.count - 1])
      let n = adjY ? ys[ys.count - 2] : ys[ys.count - 1]
      let batchShape = Array(outShape.dropLast(2))
      let xb = Array(inShape.dropLast(2))
      let yb = Array(ys.dropLast(2))
      return Self.indices(batchShape.isEmpty ? [1] : batchShape).flatMap { bi -> [Float] in
        let b = batchShape.isEmpty ? [] : bi
        let xo = batchOffset(b, xb) * m * k
        let yo = batchOffset(b, yb) * ys[ys.count - 2] * ys[ys.count - 1]
        return (0..<(m * n)).map { e in
          let (i, j) = (e / n, e % n)
          return (0..<k).reduce(Float(0)) { $0 + x[xo + i * k + $1] * y[yo + (adjY ? j * k + $1 : $1 * n + j)] }
        }
      }
    case "CONV_2D", "DEPTHWISE_CONV_2D":
      return try conv(o, x, inShape, outShape)
    case "GATHER":
      // Indices of rank 0 or 1, which is all the lowering writes.
      let axis = Int(o.options?.i32(0) ?? 0)
      let idx = try ints(o.inputs[1])
      let s = Self.strides(inShape)
      let scalar = shape(o.inputs[1]).isEmpty
      return Self.indices(outShape).map { j in
        var src = j
        if scalar { src.insert(idx[0], at: axis) } else { src[axis] = idx[j[axis]] }
        return x[zip(src, s).reduce(0) { $0 + $1.0 * $1.1 }]
      }
    default:
      throw TestError("the reference interpreter has no \(o.op)")
    }
  }

  /// The flat batch index into a batch shape that broadcasts to `b`.
  private func batchOffset(_ b: [Int], _ own: [Int]) -> Int {
    let padded = [Int](repeating: 1, count: b.count - own.count) + own
    let s = Self.strides(padded)
    return b.indices.reduce(0) { $0 + (padded[$1] == 1 ? 0 : b[$1]) * s[$1] }
  }

  func broadcast(_ v: [Float], _ from: [Int], _ to: [Int]) -> [Float] {
    if from == to { return v }
    let padded = [Int](repeating: 1, count: to.count - from.count) + from
    let s = Self.strides(padded)
    return Self.indices(to).map { idx in v[idx.indices.reduce(0) { $0 + (padded[$1] == 1 ? 0 : idx[$1]) * s[$1] }] }
  }

  private func conv(_ o: TFLiteFile.Operator, _ x: [Float], _ inShape: [Int], _ outShape: [Int]) throws -> [Float] {
    let w = try get(o.inputs[1])
    let ws = shape(o.inputs[1])
    let bias = try get(o.inputs[2])
    let opts = o.options!
    let depthwise = o.op == "DEPTHWISE_CONV_2D"
    let same = opts.i8(0) == TFLite.Padding.same.rawValue
    let (sw, sh) = (Int(opts.i32(1)), Int(opts.i32(2)))
    let (dw, dh) = depthwise ? (Int(opts.i32(5, 1)), Int(opts.i32(6, 1))) : (Int(opts.i32(4, 1)), Int(opts.i32(5, 1)))
    let multiplier = depthwise ? Int(opts.i32(3)) : 1
    let (h, wd, c) = (inShape[1], inShape[2], inShape[3])
    let (kh, kw) = (ws[1], ws[2])
    let (oh, ow, oc) = (outShape[1], outShape[2], outShape[3])
    var (pt, pl) = (0, 0)
    if same {
      pt = max((oh - 1) * sh + (kh - 1) * dh + 1 - h, 0) / 2
      pl = max((ow - 1) * sw + (kw - 1) * dw + 1 - wd, 0) / 2
    }
    var out = [Float](repeating: 0, count: outShape.reduce(1, *))
    for n in 0..<inShape[0] {
      for y in 0..<oh {
        for xo in 0..<ow {
          for f in 0..<oc {
            var acc = bias[f]
            for i in 0..<kh {
              for j in 0..<kw {
                let (yy, xx) = (y * sh - pt + i * dh, xo * sw - pl + j * dw)
                guard yy >= 0, yy < h, xx >= 0, xx < wd else { continue }
                let base = ((n * h + yy) * wd + xx) * c
                if depthwise {
                  acc += x[base + f / multiplier] * w[(i * kw + j) * oc + f]
                } else {
                  for ch in 0..<c { acc += x[base + ch] * w[((f * kh + i) * kw + j) * c + ch] }
                }
              }
            }
            out[((n * oh + y) * ow + xo) * oc + f] = acc
          }
        }
      }
    }
    return out
  }
}
