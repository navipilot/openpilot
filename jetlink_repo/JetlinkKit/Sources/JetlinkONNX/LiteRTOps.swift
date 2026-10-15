import Foundation

// The lowering of each ONNX op LiteRTPreparation meets: the four driving
// models, with tinygrad's ops stripped, and the test fixtures. Shapes are static
// throughout, so every shape, axis and bound is worked out here and written
// as a constant.

extension LiteRTLowering {
  mutating func lower(_ n: Node) throws {
    guard n.domainName.isEmpty || n.domainName == "ai.onnx" else {
      throw OnnxError("ops of domain \(n.domainName) are not lowered to LiteRT")
    }
    switch n.op {
    case "Identity": try alias(n.outputs[0], value(n.inputs[0]))
    case "Constant": try constantNode(n)
    case "Cast": try cast(n)
    case "Add": try binary(.add, n, .add)
    case "Sub": try binary(.sub, n, .sub)
    case "Mul": try binary(.mul, n, .mul)
    case "Div": try binary(.div, n, .div)
    case "Max": try variadic(.maximum, n)
    case "Min": try variadic(.minimum, n)
    case "Pow": try binary(.pow, n, .pow)
    case "Abs": try unary(.abs, n, .abs)
    case "Sqrt": try unary(.sqrt, n)
    case "Sigmoid": try unary(.logistic, n)
    case "Relu": try unary(.relu, n)
    case "Tanh": try unary(.tanh, n)
    case "Exp": try unary(.exp, n, .exp)
    case "Log": try unary(.log, n)
    case "Neg": try unary(.neg, n, .neg)
    case "Reciprocal": try reciprocal(n)
    case "Gelu":
      // approximate is "none" (erf) or "tanh"; GELU's options default to erf.
      let approximate = n.attribute("approximate")?.s ?? "none"
      try unary(.gelu, n, approximate == "tanh" ? .gelu(approximate: true) : .none)
    case "Softmax": try softmax(n)
    case "ReduceMean": try reduce(.mean, n)
    case "ReduceMax": try reduce(.reduceMax, n)
    case "ReduceMin": try reduce(.reduceMin, n)
    case "ReduceSum": try reduce(.sum, n)
    case "Reshape": try reshape(n)
    case "Flatten": try flatten(n)
    case "Squeeze": try squeeze(n)
    case "Unsqueeze": try unsqueeze(n)
    case "Transpose": try transpose(n)
    case "Concat": try concat(n)
    case "Slice": try slice(n)
    case "Split": try split(n)
    case "Gather": try gather(n)
    case "GatherND": try gatherND(n)
    case "Expand": try expand(n)
    case "MatMul": try matmul(n)
    case "Gemm": try gemm(n)
    case "Conv": try conv(n)
    case "LayerNormalization": try layerNorm(n)
    case "Where": try `where`(n)
    case "Not": try not(n)
    default: throw OnnxError("\(n.op) is not lowered to LiteRT")
    }
  }

  // MARK: shapes

  static func broadcast(_ a: [Int], _ b: [Int]) throws -> [Int] {
    let r = max(a.count, b.count)
    let pa = [Int](repeating: 1, count: r - a.count) + a
    let pb = [Int](repeating: 1, count: r - b.count) + b
    return try zip(pa, pb).map { x, y in
      if x == y || y == 1 { return x }
      if x == 1 { return y }
      throw OnnxError("shapes \(a) and \(b) do not broadcast")
    }
  }

  static func normalize(_ axis: Int, rank: Int) throws -> Int {
    let a = axis < 0 ? axis + rank : axis
    guard a >= 0, a < max(rank, 1) else { throw OnnxError("axis \(axis) is out of range for rank \(rank)") }
    return a
  }

  /// Row-major strides, in elements.
  static func strides(_ dims: [Int]) -> [Int] {
    var s = [Int](repeating: 1, count: dims.count)
    var acc = 1
    for i in stride(from: dims.count - 1, through: 0, by: -1) {
      s[i] = acc
      acc *= dims[i]
    }
    return s
  }

  /// Copies a strided view of `bytes` (`size`-byte elements): the element at
  /// output index (i0, i1, ...) of `out` is the one at base + sum(ik * strides[k]).
  /// Every fold of a constant's layout is one of these.
  static func stridedCopy(_ bytes: [UInt8], size: Int, out: [Int], base: Int, strides: [Int]) -> [UInt8] {
    let count = out.reduce(1, *)
    var result = [UInt8](repeating: 0, count: count * size)
    guard count > 0 else { return result }
    var index = [Int](repeating: 0, count: out.count)
    var from = base
    bytes.withUnsafeBytes { src in
      result.withUnsafeMutableBytes { dst in
        for e in 0..<count {
          dst.baseAddress!.advanced(by: e * size).copyMemory(from: src.baseAddress!.advanced(by: from * size), byteCount: size)
          // Step the index like an odometer, moving `from` along.
          var k = out.count - 1
          while k >= 0 {
            index[k] += 1
            from += strides[k]
            if index[k] < out[k] { break }
            from -= strides[k] * index[k]
            index[k] = 0
            k -= 1
          }
        }
      }
    }
    return result
  }

  // MARK: plumbing

  /// A tensor reshaped to `shape`, as a new tensor.
  mutating func reshaped(_ x: Int, _ shape: [Int], _ name: String) -> Int {
    let o = addTensor(name, shape, model.tensors[x].type)
    emit(.reshape, [x, int32Tensor(shape, "\(name)__shape")], [o], .reshape(newShape: shape.map { Int32($0) }))
    return o
  }

  mutating func transposed(_ x: Int, _ perm: [Int], _ name: String) -> Int {
    let shape = perm.map { model.tensors[x].shape[$0] }
    let o = addTensor(name, shape, model.tensors[x].type)
    emitTranspose(x, perm, into: o, name)
    return o
  }

  /// An operand of an elementwise op whose output has rank `rank`. A tensor
  /// of lower rank gets leading 1s (TFLite broadcasts like numpy, but the GPU
  /// wants run-time operands of one rank); a constant keeps its own shape.
  mutating func operand(_ v: Value, rank: Int, _ name: String) throws -> Int {
    switch v {
    case .constant(let c):
      return try constantTensor(c)
    case .tensor(let i):
      let shape = model.tensors[i].shape
      guard shape.count < rank else { return i }
      return reshaped(i, [Int](repeating: 1, count: rank - shape.count) + shape, name)
    }
  }

  /// `op(x, y)` with ONNX's broadcasting, into `name`.
  @discardableResult
  mutating func elementwise(
    _ op: TFLite.Op, _ x: Value, _ y: Value, _ name: String, _ options: TFLite.Options, define: Bool = true
  ) throws -> Int {
    let out = try Self.broadcast(dims(x), dims(y))
    let tx = try operand(x, rank: out.count, "\(name)__lhs")
    let ty = try operand(y, rank: out.count, "\(name)__rhs")
    let type: TFLite.TensorType
    if case .tensor(let i) = x {
      type = model.tensors[i].type
    } else if case .tensor(let i) = y {
      type = model.tensors[i].type
    } else {
      type = model.tensors[tx].type
    }
    let o = define ? try self.define(name, out, type) : addTensor(name, out, type)
    emit(op, [tx, ty], [o], options)
    return o
  }

  // MARK: elementwise

  mutating func binary(_ op: TFLite.Op, _ n: Node, _ options: TFLite.Options) throws {
    let x = try value(n.inputs[0])
    let y = try value(n.inputs[1])
    if case .constant(let cx) = x, case .constant(let cy) = y {
      // The GPU takes at most one constant operand; two are folded here.
      try fold(op, cx, cy, n.outputs[0])
      return
    }
    try elementwise(op, x, y, n.outputs[0], options)
  }

  /// `op` on two constants, broadcast, in fp32 and then rounded to the
  /// operands' type, as onnxruntime rounds an fp16 result.
  mutating func fold(_ op: TFLite.Op, _ x: Constant, _ y: Constant, _ name: String) throws {
    guard x.type == y.type, [DataType.float, DataType.float16].contains(x.type) else {
      throw OnnxError("cannot fold \(op.name) of \(OnnxMeta.typeName(x.type)) and \(OnnxMeta.typeName(y.type)) constants")
    }
    let out = try Self.broadcast(x.dims, y.dims)
    let a = try Self.broadcastFloats(floats(x), from: x.dims, to: out)
    let b = try Self.broadcastFloats(floats(y), from: y.dims, to: out)
    let f: (Float, Float) -> Float
    switch op {
    case .add: f = (+)
    case .sub: f = (-)
    case .mul: f = (*)
    case .div: f = (/)
    case .maximum: f = { Swift.max($0, $1) }
    case .minimum: f = { Swift.min($0, $1) }
    default: throw OnnxError("cannot fold \(op.name)")
    }
    try defineConstant(name, out, x.type, Elements.encode(zip(a, b).map(f), as: x.type))
    count("folded constant arithmetic")
  }

  mutating func variadic(_ op: TFLite.Op, _ n: Node) throws {
    let inputs = n.inputs.filter { !$0.isEmpty }
    guard let first = inputs.first else { throw OnnxError("no inputs") }
    if inputs.count == 1 {
      try alias(n.outputs[0], value(first))
      return
    }
    var acc = try value(first)
    for (j, name) in inputs.dropFirst().enumerated() {
      let last = j == inputs.count - 2
      let out = last ? n.outputs[0] : "\(n.outputs[0])__\(j)"
      acc = .tensor(try elementwise(op, acc, value(name), out, .maximumMinimum, define: last))
    }
  }

  mutating func unary(_ op: TFLite.Op, _ n: Node, _ options: TFLite.Options = .none) throws {
    let x = try tensor(n.inputs[0])
    let o = try define(n.outputs[0], model.tensors[x].shape, model.tensors[x].type)
    emit(op, [x], [o], options)
  }

  /// 1/x as DIV(1, x), as onnx2tf writes it: TFLite has no reciprocal.
  mutating func reciprocal(_ n: Node) throws {
    let one = Constant(name: "\(n.outputs[0])__one", dims: [1], type: DataType.float, bytes: .owned(Elements.encode([1], as: DataType.float)))
    try elementwise(.div, .constant(one), value(n.inputs[0]), n.outputs[0], .div)
  }

  // MARK: Cast, Constant, Not

  mutating func cast(_ n: Node) throws {
    guard let to = n.attribute("to").map({ Int32($0.i) }) else { throw OnnxError("Cast has no 'to'") }
    let target = try Self.computeType(to)
    let v = try value(n.inputs[0])
    switch v {
    case .constant(let c):
      try defineConstant(n.outputs[0], c.dims, to, try castBytes(c, to: to))
    case .tensor(let i):
      if model.tensors[i].type == target {
        // fp16 and fp32 are both FLOAT32 here: the Cast is a no-op.
        try alias(n.outputs[0], v)
        return
      }
      let o = try define(n.outputs[0], model.tensors[i].shape, target)
      emit(.cast, [i], [o], .cast(from: model.tensors[i].type, to: target))
    }
  }

  /// A constant's elements converted to `to`, as ONNX's Cast does for the
  /// types the lowering meets.
  func castBytes(_ c: Constant, to: Int32) throws -> [UInt8] {
    switch to {
    case DataType.float, DataType.float16, DataType.double: return Elements.encode(try floats(c), as: to)
    case DataType.bool: return try floats(c).map { $0 != 0 ? 1 : 0 }
    default:
      guard DataType.isInteger(to) else { throw OnnxError("cannot fold a Cast to \(OnnxMeta.typeName(to))") }
      let values: [Int64] =
        DataType.isInteger(c.type) || c.type == DataType.bool ? try integers(c) : try floats(c).map { Int64($0.rounded(.towardZero)) }
      return try Elements.encode(values, as: to, for: c.name)
    }
  }

  mutating func constantNode(_ n: Node) throws {
    if let t = n.attribute("value")?.t {
      try check(n.outputs[0], t.dims.map { Int($0) })
      var renamed = t
      renamed.name = n.outputs[0]
      values[n.outputs[0]] = .constant(Constant(name: n.outputs[0], dims: t.dims.map { Int($0) }, type: t.elementType, bytes: .initializer(renamed)))
    } else if let f = n.attribute("value_float")?.f {
      try defineConstant(n.outputs[0], [], DataType.float, Elements.encode([f], as: DataType.float))
    } else if let floats = n.attribute("value_floats")?.floats, !floats.isEmpty {
      try defineConstant(n.outputs[0], [floats.count], DataType.float, Elements.encode(floats, as: DataType.float))
    } else if let i = n.attribute("value_int")?.i {
      try defineConstant(n.outputs[0], [], DataType.int64, Elements.encode([i], as: DataType.int64, for: n.outputs[0]))
    } else if let ints = n.attribute("value_ints")?.ints, !ints.isEmpty {
      try defineConstant(n.outputs[0], [ints.count], DataType.int64, Elements.encode(ints, as: DataType.int64, for: n.outputs[0]))
    } else {
      throw OnnxError("a Constant of this kind is not lowered")
    }
  }

  mutating func not(_ n: Node) throws {
    switch try value(n.inputs[0]) {
    case .constant(let c):
      try defineConstant(n.outputs[0], c.dims, DataType.bool, try bytes(c).map { $0 == 0 ? 1 : 0 })
    case .tensor(let i):
      let o = try define(n.outputs[0], model.tensors[i].shape, .bool)
      emit(.logicalNot, [i], [o], .logicalNot)
    }
  }

  // MARK: reductions and Softmax

  /// A node's axes as written: the attribute (the reductions before opset
  /// 18, ReduceSum, Squeeze and Unsqueeze before 13) or the second input;
  /// nil when it has neither.
  func rawAxes(_ n: Node) throws -> [Int]? {
    if let ints = n.attribute("axes")?.ints, !ints.isEmpty { return ints.map { Int($0) } }
    return try constantInts(n, 1)
  }

  /// A node's axes counted from the front of a rank-`rank` shape, each once,
  /// in order.
  func axes(_ n: Node, rank: Int) throws -> [Int]? {
    try rawAxes(n).map { raw in try Array(Set(raw.map { try Self.normalize($0, rank: rank) })).sorted() }
  }

  mutating func reduce(_ op: TFLite.Op, _ n: Node) throws {
    let x = try tensor(n.inputs[0])
    let shape = model.tensors[x].shape
    let keep = (n.attribute("keepdims")?.i ?? 1) != 0
    var axes = try self.axes(n, rank: shape.count) ?? []
    if axes.isEmpty {
      if (n.attribute("noop_with_empty_axes")?.i ?? 0) != 0 {
        try alias(n.outputs[0], .tensor(x))
        return
      }
      axes = Array(shape.indices)
    }
    var out: [Int] = []
    for (k, d) in shape.enumerated() {
      if !axes.contains(k) {
        out.append(d)
      } else if keep {
        out.append(1)
      }
    }
    let o = try define(n.outputs[0], out, model.tensors[x].type)
    emit(op, [x, int32Tensor(axes, "\(n.outputs[0])__axes")], [o], .reducer(keepDims: keep))
  }

  mutating func softmax(_ n: Node) throws {
    let x = try tensor(n.inputs[0])
    let shape = model.tensors[x].shape
    let rank = shape.count
    let axis = try Self.normalize(Int(n.attribute("axis")?.i ?? (opset >= 13 ? -1 : 1)), rank: rank)
    let name = n.outputs[0]
    if axis == rank - 1 {
      let o = try define(name, shape, .float32)
      emit(.softmax, [x], [o], .softmax(beta: 1))
      return
    }
    // Before opset 13 Softmax flattens to 2-D at the axis, which is the last
    // axis's softmax only when the axis is the last.
    guard opset >= 13 else { throw OnnxError("a Softmax before opset 13 over more than the last axis is not lowered") }
    // SOFTMAX works on the last axis; the axis goes there and back.
    var perm = Array(0..<rank)
    perm.swapAt(axis, rank - 1)
    let t = transposed(x, perm, "\(name)__last")
    let s = addTensor("\(name)__softmax", model.tensors[t].shape, .float32)
    emit(.softmax, [t], [s], .softmax(beta: 1))
    let o = try define(name, shape, .float32)
    emit(.transpose, [s, int32Tensor(perm, "\(name)__perm")], [o], .transpose)
  }

  // MARK: reshapes

  /// `x` as `shape`: a constant keeps its bytes, a tensor goes through RESHAPE.
  mutating func reshape(_ name: String, _ x: Value, _ shape: [Int]) throws {
    switch x {
    case .constant(var c):
      guard c.count == shape.reduce(1, *) else { throw OnnxError("cannot reshape \(c.dims) to \(shape)") }
      try check(name, shape)
      c.dims = shape
      values[name] = .constant(c)
    case .tensor(let i):
      guard model.tensors[i].elementCount == shape.reduce(1, *) else {
        throw OnnxError("cannot reshape \(model.tensors[i].shape) to \(shape)")
      }
      let o = try define(name, shape, model.tensors[i].type)
      emit(.reshape, [i, int32Tensor(shape, "\(name)__shape")], [o], .reshape(newShape: shape.map { Int32($0) }))
    }
  }

  mutating func reshape(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let have = dims(x)
    let total = have.reduce(1, *)
    var target: [Int]
    if let c = constant(n.inputs[1]) {
      target = try integers(c).map { Int($0) }
    } else if let want = recordedShape(n.outputs[0]) {
      // A shape computed at run time, which the export recorded statically.
      target = want
    } else {
      throw OnnxError("the target shape is computed at run time and not recorded")
    }
    if (n.attribute("allowzero")?.i ?? 0) == 0 {
      for k in target.indices where target[k] == 0 {
        guard k < have.count else { throw OnnxError("shape \(target) copies a dimension \(have) does not have") }
        target[k] = have[k]
      }
    }
    if let k = target.firstIndex(of: -1) {
      let known = target.enumerated().filter { $0.offset != k }.reduce(1) { $0 * $1.element }
      guard known > 0, total % known == 0 else { throw OnnxError("cannot reshape \(have) to \(target)") }
      target[k] = total / known
    }
    try reshape(n.outputs[0], x, target)
  }

  mutating func flatten(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    let axis = try Self.normalize(Int(n.attribute("axis")?.i ?? 1), rank: shape.count + 1)
    try reshape(n.outputs[0], x, [shape[..<axis].reduce(1, *), shape[axis...].reduce(1, *)])
  }

  mutating func squeeze(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    let axes = try self.axes(n, rank: shape.count) ?? shape.indices.filter { shape[$0] == 1 }
    for k in axes where shape[k] != 1 {
      throw OnnxError("cannot squeeze axis \(k) of \(shape)")
    }
    try reshape(n.outputs[0], x, shape.enumerated().filter { !axes.contains($0.offset) }.map(\.element))
  }

  mutating func unsqueeze(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    let raw = try rawAxes(n) ?? []
    let rank = shape.count + raw.count
    let axes = Set(try raw.map { try Self.normalize($0, rank: rank) })
    var rest = shape.makeIterator()
    try reshape(n.outputs[0], x, (0..<rank).map { axes.contains($0) ? 1 : rest.next()! })
  }

  func recordedShape(_ name: String) -> [Int]? {
    guard let want = recordedDims(name), !want.contains(where: { $0 < 0 }) else { return nil }
    return want.map { Int($0) }
  }

  // MARK: layout ops

  mutating func transpose(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    let perm = n.attribute("perm")?.ints.map { Int($0) } ?? Array((0..<shape.count).reversed())
    guard perm.count == shape.count, Set(perm) == Set(0..<shape.count) else {
      throw OnnxError("perm \(perm) does not fit rank \(shape.count)")
    }
    let out = perm.map { shape[$0] }
    switch x {
    case .constant(let c):
      let size = try Self.elementSize(c)
      let strides = Self.strides(shape)
      try defineConstant(n.outputs[0], out, c.type, Self.stridedCopy(bytes(c), size: size, out: out, base: 0, strides: perm.map { strides[$0] }))
    case .tensor(let i):
      let o = try define(n.outputs[0], out, model.tensors[i].type)
      emitTranspose(i, perm, into: o, n.outputs[0])
    }
  }

  static func elementSize(_ c: Constant) throws -> Int {
    guard let size = DataType.size(c.type) else { throw OnnxError("constant \(c.name) has element type \(c.type)") }
    return size
  }

  mutating func concat(_ n: Node) throws {
    let inputs = try n.inputs.filter { !$0.isEmpty }.map { try value($0) }
    guard let first = inputs.first, let axisValue = n.attribute("axis")?.i else { throw OnnxError("Concat needs inputs and an axis") }
    let rank = dims(first).count
    let axis = try Self.normalize(Int(axisValue), rank: rank)
    var out = dims(first)
    out[axis] = 0
    for v in inputs {
      let d = dims(v)
      guard d.count == rank, d.indices.allSatisfy({ $0 == axis || d[$0] == out[$0] }) else {
        throw OnnxError("cannot concatenate \(inputs.map(dims)) on axis \(axis)")
      }
      out[axis] += d[axis]
    }
    // Inputs with nothing along the axis add nothing.
    let parts = inputs.filter { dims($0)[axis] > 0 }
    let constants = parts.compactMap { v -> Constant? in
      if case .constant(let c) = v { return c }
      return nil
    }
    if constants.count == parts.count, let type = constants.first?.type, constants.allSatisfy({ $0.type == type }) {
      // Folded: each part's rows, in turn, for every outer index.
      let size = try Self.elementSize(constants[0])
      let outer = out[..<axis].reduce(1, *)
      let inner = out[(axis + 1)...].reduce(1, *)
      var bytes: [UInt8] = []
      bytes.reserveCapacity(out.reduce(1, *) * size)
      let all = try constants.map { try self.bytes($0) }
      for o in 0..<outer {
        for (c, b) in zip(constants, all) {
          let run = c.dims[axis] * inner * size
          bytes += b[(o * run)..<((o + 1) * run)]
        }
      }
      try defineConstant(n.outputs[0], out, type, bytes)
      return
    }
    if parts.count == 1 {
      try alias(n.outputs[0], parts[0])
      return
    }
    let tensors = try parts.map { try tensor($0) }
    let o = try define(n.outputs[0], out, model.tensors[tensors[0]].type)
    if rank > Self.gpuRank {
      // Above rank 4, on [outer, axis, inner] views of every part.
      let outer = out[..<axis].reduce(1, *)
      let inner = out[(axis + 1)...].reduce(1, *)
      let views = tensors.enumerated().map { k, t in reshaped(t, [outer, model.tensors[t].shape[axis], inner], "\(n.outputs[0])__view\(k)") }
      let joined = addTensor("\(n.outputs[0])__joined", [outer, out[axis], inner], model.tensors[o].type)
      emit(.concatenation, views, [joined], .concatenation(axis: 1))
      emit(.reshape, [joined, int32Tensor(out, "\(n.outputs[0])__shape")], [o], .reshape(newShape: out.map { Int32($0) }))
      count("rank-\(rank) concatenations on a view")
      return
    }
    emit(.concatenation, tensors, [o], .concatenation(axis: Int32(axis)))
  }

  /// ONNX Slice's start, end and step for one axis of size `dim`, clamped as
  /// the operator defines: (begin, end, step, count).
  static func sliceBounds(start: Int, end: Int, step: Int, dim: Int) throws -> (Int, Int, Int, Int) {
    guard step != 0 else { throw OnnxError("a Slice step is 0") }
    var s = start < 0 ? start + dim : start
    var e = end < 0 ? end + dim : end
    if step > 0 {
      s = min(max(s, 0), dim)
      e = min(max(e, 0), dim)
      return (s, e, step, max(0, (e - s + step - 1) / step))
    }
    s = min(max(s, 0), dim - 1)
    e = min(max(e, -1), dim - 1)
    return (s, e, step, max(0, (s - e - step - 1) / -step))
  }

  mutating func slice(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    guard let starts = try constantInts(n, 1), let ends = try constantInts(n, 2) else { throw OnnxError("Slice needs starts and ends") }
    let axes = try (constantInts(n, 3) ?? Array(starts.indices)).map { try Self.normalize($0, rank: shape.count) }
    let steps = try constantInts(n, 4) ?? [Int](repeating: 1, count: starts.count)
    guard starts.count == ends.count, axes.count == starts.count, steps.count == starts.count else {
      throw OnnxError("Slice's starts, ends, axes and steps differ in length")
    }
    var begin = [Int](repeating: 0, count: shape.count)
    var end = shape
    var step = [Int](repeating: 1, count: shape.count)
    var out = shape
    for (j, axis) in axes.enumerated() {
      let (b, e, s, c) = try Self.sliceBounds(start: starts[j], end: ends[j], step: steps[j], dim: shape[axis])
      (begin[axis], end[axis], step[axis], out[axis]) = (b, e, s, c)
    }
    try slice(n.outputs[0], x, begin: begin, end: end, step: step, out: out)
  }

  /// A slice already in bounds: SLICE when every step is 1, STRIDED_SLICE
  /// otherwise; folded for a constant.
  mutating func slice(_ name: String, _ x: Value, begin: [Int], end: [Int], step: [Int], out: [Int]) throws {
    guard !out.contains(0) else { throw OnnxError("a slice that leaves nothing is not lowered") }
    let shape = dims(x)
    if out == shape && step.allSatisfy({ $0 == 1 }) {
      try alias(name, x)
      return
    }
    switch x {
    case .constant(let c):
      let size = try Self.elementSize(c)
      let strides = Self.strides(shape)
      let base = zip(begin, strides).reduce(0) { $0 + $1.0 * $1.1 }
      try defineConstant(name, out, c.type, Self.stridedCopy(bytes(c), size: size, out: out, base: base, strides: zip(step, strides).map { $0 * $1 }))
    case .tensor(let i):
      let o = try define(name, out, model.tensors[i].type)
      emitSlice(i, begin: begin, end: end, step: step, into: o, name)
    }
  }

  /// SLICE when every step is 1, STRIDED_SLICE otherwise, from tensor `i`
  /// into tensor `o`. Above rank 4 it runs on a view (see `sliceView`).
  mutating func emitSlice(_ i: Int, begin: [Int], end: [Int], step: [Int], into o: Int, _ name: String) {
    let shape = model.tensors[i].shape
    let out = model.tensors[o].shape
    if shape.count > Self.gpuRank, let v = Self.sliceView(shape: shape, begin: begin, end: end, step: step, out: out), v.shape.count <= Self.gpuRank {
      let input = reshaped(i, v.shape, "\(name)__view")
      let sliced = addTensor("\(name)__sliced", v.out, model.tensors[i].type)
      emitSlice(input, begin: v.begin, end: v.end, step: v.step, into: sliced, name)
      emit(.reshape, [sliced, int32Tensor(out, "\(name)__shape")], [o], .reshape(newShape: out.map { Int32($0) }))
      count("rank-\(shape.count) slices on a view")
      return
    }
    if step.allSatisfy({ $0 == 1 }) {
      emit(.slice, [i, int32Tensor(begin, "\(name)__begin"), int32Tensor(out, "\(name)__size")], [o], .slice)
      return
    }
    // An end of -1 under a negative step means "past the front", which
    // TFLite would read as the last element: its end_mask says it instead.
    var mask = 0
    var stops = end
    for k in end.indices where step[k] < 0 && end[k] < 0 {
      mask |= 1 << k
      stops[k] = 0
    }
    emit(
      .stridedSlice,
      [i, int32Tensor(begin, "\(name)__begin"), int32Tensor(stops, "\(name)__end"), int32Tensor(step, "\(name)__strides")],
      [o], .stridedSlice(beginMask: 0, endMask: Int32(mask)))
  }

  /// The largest rank LiteRT's GPU takes.
  static let gpuRank = 4

  /// A slice of a tensor of rank above 4 as a slice of a smaller view of
  /// it: axes of size 1 dropped and, if that leaves more than 4, each run of
  /// neighbouring axes the slice takes whole merged into one. Merging no
  /// more than that keeps the view's last axes the tensor's, so the RESHAPEs
  /// around it are more often ones that cancel than copies on the GPU. nil
  /// when nothing can go.
  static func sliceView(shape: [Int], begin: [Int], end: [Int], step: [Int], out: [Int])
    -> (shape: [Int], begin: [Int], end: [Int], step: [Int], out: [Int])?
  {
    let merge = shape.filter { $0 != 1 }.count > gpuRank
    var groups: [(whole: Bool, axes: [Int])] = []
    for k in shape.indices where shape[k] != 1 {
      let whole = begin[k] == 0 && step[k] == 1 && out[k] == shape[k]
      if merge, whole, let last = groups.last, last.whole {
        groups[groups.count - 1].axes.append(k)
      } else {
        groups.append((whole, [k]))
      }
    }
    guard groups.count < shape.count else { return nil }
    if groups.isEmpty { groups = [(true, [])] }
    var v: (shape: [Int], begin: [Int], end: [Int], step: [Int], out: [Int]) = ([], [], [], [], [])
    for g in groups {
      let size = g.axes.reduce(1) { $0 * shape[$1] }
      if g.whole {
        v.shape.append(size)
        v.begin.append(0)
        v.end.append(size)
        v.step.append(1)
        v.out.append(size)
      } else {
        let k = g.axes[0]
        v.shape.append(shape[k])
        v.begin.append(begin[k])
        v.end.append(end[k])
        v.step.append(step[k])
        v.out.append(out[k])
      }
    }
    return v
  }

  /// TRANSPOSE from tensor `i` into tensor `o`. Above rank 4 it runs on a
  /// view: axes of size 1 dropped and, if that leaves more than 4, axes that
  /// stay neighbours in the same order merged.
  mutating func emitTranspose(_ i: Int, _ perm: [Int], into o: Int, _ name: String) {
    let shape = model.tensors[i].shape
    if shape.count > Self.gpuRank {
      let v = Self.transposeView(shape: shape, perm: perm)
      if v.perm.count < shape.count && v.perm.count <= Self.gpuRank {
        let input = reshaped(i, v.shape, "\(name)__view")
        let moved = addTensor("\(name)__moved", v.perm.map { v.shape[$0] }, model.tensors[i].type)
        emit(.transpose, [input, int32Tensor(v.perm, "\(name)__perm")], [moved], .transpose)
        let out = model.tensors[o].shape
        emit(.reshape, [moved, int32Tensor(out, "\(name)__shape")], [o], .reshape(newShape: out.map { Int32($0) }))
        count("rank-\(shape.count) transposes on a view")
        return
      }
    }
    emit(.transpose, [i, int32Tensor(perm, "\(name)__perm")], [o], .transpose)
  }

  static func transposeView(shape: [Int], perm: [Int]) -> (shape: [Int], perm: [Int]) {
    let kept = shape.indices.filter { shape[$0] != 1 }
    let order = perm.filter { shape[$0] != 1 }.map { kept.firstIndex(of: $0)! }
    // Runs of input axes that come out together and in order, in output
    // order; each axis its own run when dropping the 1s is enough.
    var runs: [[Int]] = []
    for a in order {
      if kept.count > gpuRank, let last = runs.last?.last, a == last + 1 {
        runs[runs.count - 1].append(a)
      } else {
        runs.append([a])
      }
    }
    let byInput = runs.indices.sorted { runs[$0][0] < runs[$1][0] }
    let shapeView = byInput.map { runs[$0].reduce(1) { $0 * shape[kept[$1]] } }
    let permView = runs.indices.map { r in byInput.firstIndex(of: r)! }
    return (shapeView.isEmpty ? [1] : shapeView, permView.isEmpty ? [0] : permView)
  }

  mutating func split(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    let axis = try Self.normalize(Int(n.attribute("axis")?.i ?? 0), rank: shape.count)
    var sizes = n.attribute("split")?.ints.map { Int($0) } ?? []
    if sizes.isEmpty, let given = try constantInts(n, 1) { sizes = given }
    if sizes.isEmpty {
      let parts = n.outputs.count
      let each = (shape[axis] + parts - 1) / parts
      sizes = (0..<parts).map { min(each, shape[axis] - $0 * each) }
    }
    guard sizes.count == n.outputs.count, sizes.reduce(0, +) == shape[axis] else {
      throw OnnxError("split sizes \(sizes) do not fit axis \(axis) of \(shape)")
    }
    var at = 0
    for (name, size) in zip(n.outputs, sizes) {
      var begin = [Int](repeating: 0, count: shape.count)
      var end = shape
      var out = shape
      begin[axis] = at
      end[axis] = at + size
      out[axis] = size
      if !name.isEmpty {
        try slice(name, x, begin: begin, end: end, step: [Int](repeating: 1, count: shape.count), out: out)
      }
      at += size
    }
  }

  // MARK: gathers

  mutating func gather(_ n: Node) throws {
    let x = try value(n.inputs[0])
    let shape = dims(x)
    let axis = try Self.normalize(Int(n.attribute("axis")?.i ?? 0), rank: shape.count)
    guard let indices = constant(n.inputs[1]) else {
      // Indices computed at run time: GATHER itself, on INT32 indices.
      var idx = try tensor(n.inputs[1])
      if model.tensors[idx].type != .int32 {
        let wide = addTensor("\(n.inputs[1])__i32", model.tensors[idx].shape, .int32)
        emit(.cast, [idx], [wide], .cast(from: model.tensors[idx].type, to: .int32))
        idx = wide
      }
      let out = Array(shape[..<axis]) + model.tensors[idx].shape + Array(shape[(axis + 1)...])
      let o = try define(n.outputs[0], out, model.tensors[try tensor(x)].type)
      emit(.gather, [try tensor(x), idx], [o], .gather(axis: Int32(axis)))
      return
    }
    let raw = try integers(indices).map { Int($0) }
    let positions = try raw.map { i -> Int in
      let p = i < 0 ? i + shape[axis] : i
      guard p >= 0, p < shape[axis] else { throw OnnxError("index \(i) is out of range for axis \(axis) of \(shape)") }
      return p
    }
    try gather(n.outputs[0], x, axis: axis, positions: positions, indexDims: indices.dims)
  }

  /// Gather with constant positions: folded for a constant, otherwise one
  /// SLICE per run of consecutive positions, a CONCATENATION of the runs and
  /// a RESHAPE to the gathered shape, all of which the GPU runs.
  mutating func gather(_ name: String, _ x: Value, axis: Int, positions: [Int], indexDims: [Int]) throws {
    let shape = dims(x)
    let out = Array(shape[..<axis]) + indexDims + Array(shape[(axis + 1)...])
    guard !positions.isEmpty else { throw OnnxError("a gather of nothing is not lowered") }
    if case .constant(let c) = x {
      let size = try Self.elementSize(c)
      let bytes = try self.bytes(c)
      let strides = Self.strides(shape)
      var rows = shape
      rows[axis] = 1
      let parts = positions.map { p in Self.stridedCopy(bytes, size: size, out: rows, base: p * strides[axis], strides: strides) }
      // Interleave: for every outer index, each position's row in turn.
      let outer = shape[..<axis].reduce(1, *)
      let run = shape[(axis + 1)...].reduce(1, *) * size
      var folded: [UInt8] = []
      folded.reserveCapacity(out.reduce(1, *) * size)
      for o in 0..<outer {
        for part in parts { folded += part[(o * run)..<((o + 1) * run)] }
      }
      try defineConstant(name, out, c.type, folded)
      return
    }
    var runs: [(Int, Int)] = []
    for p in positions {
      if let last = runs.last, last.1 == p {
        runs[runs.count - 1].1 = p + 1
      } else {
        runs.append((p, p + 1))
      }
    }
    var cat = shape
    cat[axis] = positions.count
    let catName = cat == out ? name : "\(name)__rows"
    var pieces: [Int] = []
    for (j, run) in runs.enumerated() {
      var begin = [Int](repeating: 0, count: shape.count)
      var end = shape
      var piece = shape
      begin[axis] = run.0
      end[axis] = run.1
      piece[axis] = run.1 - run.0
      let pieceName = runs.count == 1 ? catName : "\(name)__run\(j)"
      if runs.count == 1 && cat == out {
        try slice(name, x, begin: begin, end: end, step: [Int](repeating: 1, count: shape.count), out: piece)
        return
      }
      let i = try tensor(x)
      let o = addTensor(pieceName, piece, model.tensors[i].type)
      emitSlice(i, begin: begin, end: end, step: [Int](repeating: 1, count: shape.count), into: o, pieceName)
      pieces.append(o)
    }
    var rows = pieces[0]
    if pieces.count > 1 {
      rows = cat == out ? try define(name, cat, model.tensors[pieces[0]].type) : addTensor(catName, cat, model.tensors[pieces[0]].type)
      emit(.concatenation, pieces, [rows], .concatenation(axis: Int32(axis)))
      if cat == out { return }
    }
    try reshape(name, .tensor(rows), out)
  }

  /// GatherND with constant indices, as a Gather on the leading axes
  /// flattened into one.
  mutating func gatherND(_ n: Node) throws {
    guard (n.attribute("batch_dims")?.i ?? 0) == 0 else { throw OnnxError("GatherND with batch_dims is not lowered") }
    let x = try value(n.inputs[0])
    let shape = dims(x)
    guard let indices = constant(n.inputs[1]), let k = indices.dims.last, k >= 1, k <= shape.count else {
      throw OnnxError("GatherND is lowered for constant indices only")
    }
    let raw = try integers(indices).map { Int($0) }
    let lead = Array(shape[..<k])
    let strides = Self.strides(lead)
    var positions: [Int] = []
    for row in stride(from: 0, to: raw.count, by: k) {
      var p = 0
      for j in 0..<k {
        var i = raw[row + j]
        if i < 0 { i += lead[j] }
        guard i >= 0, i < lead[j] else { throw OnnxError("index \(raw[row + j]) is out of range for axis \(j) of \(shape)") }
        p += i * strides[j]
      }
      positions.append(p)
    }
    var flat = x
    if k > 1 {
      let flatShape = [lead.reduce(1, *)] + Array(shape[k...])
      try reshape("\(n.outputs[0])__flat", x, flatShape)
      flat = try value("\(n.outputs[0])__flat")
    }
    try gather(n.outputs[0], flat, axis: 0, positions: positions, indexDims: Array(indices.dims.dropLast()))
  }

  // MARK: Expand and Where

  /// Expand as a multiply by ones, which is exact and which the GPU runs
  /// (onnx2tf lowers it the same way). The ones are 1 wide on the axes the
  /// input already has, so the multiply broadcasts both ways.
  mutating func expand(_ n: Node) throws {
    let x = try value(n.inputs[0])
    guard let target = try constantInts(n, 1) else { throw OnnxError("Expand needs a constant shape") }
    let out = try Self.broadcast(dims(x), target)
    if out == dims(x) {
      try alias(n.outputs[0], x)
      return
    }
    if case .constant(let c) = x {
      let size = try Self.elementSize(c)
      let padded = [Int](repeating: 1, count: out.count - c.dims.count) + c.dims
      let strides = Self.strides(padded).enumerated().map { padded[$0.offset] == 1 ? 0 : $0.element }
      try defineConstant(n.outputs[0], out, c.type, Self.stridedCopy(bytes(c), size: size, out: out, base: 0, strides: strides))
      return
    }
    guard case .tensor(let i) = x, model.tensors[i].type == .float32 else { throw OnnxError("Expand is lowered for float tensors only") }
    let tx = try operand(x, rank: out.count, "\(n.outputs[0])__lhs")
    let spread = zip(model.tensors[tx].shape, out).map { $0 == $1 ? 1 : $1 }
    let ones = floatTensor([Float](repeating: 1, count: spread.reduce(1, *)), spread, "\(n.outputs[0])__ones")
    let o = try define(n.outputs[0], out, .float32)
    emit(.mul, [tx, ones], [o], .mul)
  }

  /// Where with a constant condition and one constant branch, the attention
  /// masks: `x * keep + fill`, with keep 1 where the run-time branch is taken
  /// and fill the constant where it is not (onnx2tf's lowering). It is exact
  /// for finite inputs. An infinite fill becomes the largest finite fp16,
  /// which is only exact when a Softmax over the last axis reads the result
  /// and every row keeps something: exp then underflows to exactly 0 either
  /// way. That is checked, and anything else is SELECT_V2.
  mutating func `where`(_ n: Node) throws {
    let cond = try value(n.inputs[0])
    let x = try value(n.inputs[1])
    let y = try value(n.inputs[2])
    let name = n.outputs[0]
    let out = try Self.broadcast(Self.broadcast(dims(cond), dims(x)), dims(y))
    if case .constant(let mask) = cond {
      let branches: (taken: Value, fill: Constant, keepWhere: Bool)? =
        switch (x, y) {
        case (.constant(let c), .tensor): (y, c, false)
        case (.tensor, .constant(let c)): (x, c, true)
        default: nil
        }
      guard let branches else {
        return try select(n, cond, x, y, out)
      }
      let (taken, fill, keepWhere) = branches
      let bits = try bytes(mask).map { $0 != 0 }
      let keep = bits.map { $0 == keepWhere ? Float(1) : 0 }
      // The fill and keep, both at the broadcast of the mask's and the fill's shapes.
      let shape = try Self.broadcast(mask.dims, fill.dims)
      let keepFull = try Self.broadcastFloats(keep, from: mask.dims, to: shape)
      var fills = try Self.broadcastFloats(floats(fill), from: fill.dims, to: shape)
      if fills.contains(where: \.isInfinite) {
        guard try softmaxReadsEveryRow(name, keep: keepFull, shape: shape, out: out) else {
          return try select(n, cond, x, y, out)
        }
        fills = fills.map { $0.isInfinite ? ($0 < 0 ? -Self.finiteFill : Self.finiteFill) : $0 }
        count("finite mask fills")
      }
      let term = zip(keepFull, fills).map { $0 == 1 ? Float(0) : $1 }
      let keepTensor = floatTensor(keepFull, shape, "\(name)__keep")
      let termTensor = floatTensor(term, shape, "\(name)__fill")
      let kept = try elementwise(.mul, taken, .tensor(keepTensor), "\(name)__kept", .mul, define: false)
      let o = try define(name, out, .float32)
      emit(.add, [kept, termTensor], [o], .add)
      count("masked Wheres")
      return
    }
    try select(n, cond, x, y, out)
  }

  /// Whether `name` is read only by Softmaxes over its last axis and every
  /// row of `keep` (broadcast to `out`) keeps at least one value.
  private func softmaxReadsEveryRow(_ name: String, keep: [Float], shape: [Int], out: [Int]) throws -> Bool {
    let readers = readers(of: name)
    guard !readers.isEmpty else { return false }
    for r in readers {
      guard r.op == "Softmax" else { return false }
      let axis = r.attribute("axis").map { Int($0.i) } ?? (opset >= 13 ? -1 : 1)
      guard try Self.normalize(axis, rank: out.count) == out.count - 1 else { return false }
    }
    guard let last = shape.last, last == out.last else { return false }
    for row in stride(from: 0, to: keep.count, by: last) where !keep[row..<(row + last)].contains(1) {
      return false
    }
    return true
  }

  static func broadcastFloats(_ values: [Float], from: [Int], to: [Int]) throws -> [Float] {
    if from == to { return values }
    let padded = [Int](repeating: 1, count: to.count - from.count) + from
    let strides = Self.strides(padded).enumerated().map { padded[$0.offset] == 1 ? 0 : $0.element }
    let copied = Self.stridedCopy(Elements.encode(values, as: DataType.float), size: 4, out: to, base: 0, strides: strides)
    return try Elements.floats(copied, as: DataType.float, for: "a broadcast")
  }

  private mutating func select(_ n: Node, _ cond: Value, _ x: Value, _ y: Value, _ out: [Int]) throws {
    let rank = out.count
    let c = try operand(cond, rank: rank, "\(n.outputs[0])__cond")
    let tx = try operand(x, rank: rank, "\(n.outputs[0])__x")
    let ty = try operand(y, rank: rank, "\(n.outputs[0])__y")
    let o = try define(n.outputs[0], out, model.tensors[tx].type)
    emit(.selectV2, [c, tx, ty], [o], .selectV2)
  }

  // MARK: MatMul, Gemm, Conv

  mutating func matmul(_ n: Node) throws {
    let a = try value(n.inputs[0])
    let b = try value(n.inputs[1])
    let da = dims(a)
    let db = dims(b)
    guard da.count >= 2, db.count >= 2 else { throw OnnxError("MatMul of a vector is not lowered") }
    guard da[da.count - 1] == db[db.count - 2] else { throw OnnxError("MatMul of \(da) and \(db)") }
    let batch = try Self.broadcast(Array(da.dropLast(2)), Array(db.dropLast(2)))
    let out = batch + [da[da.count - 2], db[db.count - 1]]
    // A constant right-hand side stays [K, N]; run-time operands share a rank.
    let rank = max(da.count, db.count)
    let ta = try operand(a, rank: rank, "\(n.outputs[0])__lhs")
    let tb = try operand(b, rank: rank, "\(n.outputs[0])__rhs")
    let o = try define(n.outputs[0], out, .float32)
    emit(.batchMatmul, [ta, tb], [o], .batchMatmul(adjX: false, adjY: false))
  }

  mutating func gemm(_ n: Node) throws {
    guard (n.attribute("alpha")?.f ?? 1) == 1, (n.attribute("beta")?.f ?? 1) == 1 else {
      throw OnnxError("Gemm with alpha or beta other than 1 is not lowered")
    }
    let name = n.outputs[0]
    var x = try tensor(n.inputs[0])
    if (n.attribute("transA")?.i ?? 0) != 0 {
      x = transposed(x, [1, 0], "\(name)__a")
    }
    let transB = (n.attribute("transB")?.i ?? 0) != 0
    let dx = model.tensors[x].shape
    guard dx.count == 2 else { throw OnnxError("Gemm of a rank-\(dx.count) input") }
    guard let w = constant(n.inputs[1]) else {
      // Both run-time: a matmul.
      let b = try tensor(n.inputs[1])
      let db = model.tensors[b].shape
      let cols = transB ? db[0] : db[1]
      let mm = addTensor(n.inputs.count > 2 ? "\(name)__mm" : name, [dx[0], cols], .float32)
      emit(.batchMatmul, [x, b], [mm], .batchMatmul(adjX: false, adjY: transB))
      try finishGemm(n, .tensor(mm), name)
      return
    }
    guard w.dims.count == 2 else { throw OnnxError("Gemm weight of rank \(w.dims.count)") }
    // FULLY_CONNECTED takes the weight as [N, K]: Gemm's B when transB is set.
    let (rows, cols) = (w.dims[0], w.dims[1])
    let weight = try transB ? constantTensor(w) : transposedWeight(w, rows: rows, cols: cols, "\(w.name)__t")
    let units = transB ? rows : cols
    guard model.tensors[weight].shape == [units, dx[1]] else { throw OnnxError("Gemm of \(dx) and \(w.dims), transB \(transB)") }
    var bias = -1
    var rest: Value?
    if n.inputs.count > 2, !n.inputs[2].isEmpty {
      let c = try value(n.inputs[2])
      if case .constant(var cc) = c, cc.count == units, cc.dims.filter({ $0 != 1 }).count <= 1 {
        cc.dims = [units]
        bias = try constantTensor(cc)
      } else {
        rest = c
      }
    }
    let fcName = rest == nil ? name : "\(name)__fc"
    let o = rest == nil ? try define(name, [dx[0], units], .float32) : addTensor(fcName, [dx[0], units], .float32)
    emit(.fullyConnected, [x, weight, bias], [o], .fullyConnected(keepNumDims: false))
    if let rest {
      try elementwise(.add, .tensor(o), rest, name, .add)
    }
  }

  private mutating func finishGemm(_ n: Node, _ mm: Value, _ name: String) throws {
    guard n.inputs.count > 2, !n.inputs[2].isEmpty else { return }
    try elementwise(.add, mm, value(n.inputs[2]), name, .add)
  }

  /// A 2-D weight's transpose, made block by block when the file is written.
  private mutating func transposedWeight(_ w: Constant, rows: Int, cols: Int, _ name: String) throws -> Int {
    let size = try Self.elementSize(w)
    if w.type == DataType.float16 && w.count < Self.fp16MinElements {
      let values = try floats(w)
      return floatTensor((0..<cols).flatMap { j in (0..<rows).map { values[$0 * cols + j] } }, [cols, rows], name)
    }
    var e = Encoded()
    e.transposed(Transpose(elements: try transposeElements(w), rows: rows, cols: cols, elementSize: size))
    switch w.type {
    case DataType.float16: return storedTensor((buffer(e), .float16), [cols, rows], name)
    case DataType.float: return addTensor(name, [cols, rows], .float32, buffer: buffer(e))
    default: throw OnnxError("weight \(w.name) has element type \(OnnxMeta.typeName(w.type))")
    }
  }

  /// A constant's elements for a transpose made as the file is written.
  private func transposeElements(_ w: Constant) throws -> Transpose.Elements {
    switch w.bytes {
    case .initializer(let t): try Patches.transposeElements(of: t, src)
    case .owned(let b): .owned(b)
    }
  }

  mutating func conv(_ n: Node) throws {
    let name = n.outputs[0]
    let x = try tensor(n.inputs[0])
    let shape = model.tensors[x].shape
    guard shape.count == 4 else { throw OnnxError("only 2-D convolutions (rank-4 inputs) are lowered") }
    guard let w = constant(n.inputs[1]), w.dims.count == 4 else { throw OnnxError("Conv needs a constant rank-4 weight") }
    let (batch, channels, height, width) = (shape[0], shape[1], shape[2], shape[3])
    let (filters, perGroup, kh, kw) = (w.dims[0], w.dims[1], w.dims[2], w.dims[3])
    let group = Int(n.attribute("group")?.i ?? 1)
    guard channels == perGroup * group else { throw OnnxError("Conv input has \(channels) channels for weight \(w.dims), group \(group)") }
    let depthwise = group > 1 && group == channels && perGroup == 1 && filters % channels == 0
    guard group == 1 || depthwise else { throw OnnxError("grouped convolutions other than depthwise are not lowered") }
    let strides = n.attribute("strides")?.ints.map { Int($0) } ?? [1, 1]
    let dilations = n.attribute("dilations")?.ints.map { Int($0) } ?? [1, 1]
    let kernel = [kh, kw]
    let input = [height, width]
    // pads are [top, left, bottom, right].
    var pads = n.attribute("pads")?.ints.map { Int($0) } ?? [0, 0, 0, 0]
    var same = [0, 0, 0, 0]
    for k in 0..<2 {
      let outSize = (input[k] + strides[k] - 1) / strides[k]
      let total = max((outSize - 1) * strides[k] + (kernel[k] - 1) * dilations[k] + 1 - input[k], 0)
      same[k] = total / 2
      same[k + 2] = total - total / 2
    }
    switch n.attribute("auto_pad")?.s ?? "NOTSET" {
    case "SAME_UPPER": pads = same
    case "SAME_LOWER": pads = [same[2], same[3], same[0], same[1]]
    case "VALID": pads = [0, 0, 0, 0]
    default: break
    }
    var out: [Int] = []
    for k in 0..<2 {
      let extent = (kernel[k] - 1) * dilations[k] + 1
      out.append((input[k] + pads[k] + pads[k + 2] - extent) / strides[k] + 1)
    }

    // NCHW to NHWC and back around the NHWC convolution.
    var nhwc = transposed(x, [0, 2, 3, 1], "\(name)__nhwc_in")
    let padding: TFLite.Padding
    if pads == [0, 0, 0, 0] {
      padding = .valid
    } else if pads == same {
      padding = .same
    } else {
      let padded = addTensor("\(name)__padded", [batch, height + pads[0] + pads[2], width + pads[1] + pads[3], channels], .float32)
      let amounts = int32Tensor([0, 0, pads[0], pads[2], pads[1], pads[3], 0, 0], "\(name)__pads", shape: [4, 2])
      emit(.pad, [nhwc, amounts], [padded], .pad)
      nhwc = padded
      padding = .valid
    }
    let filter = try convFilter(w, depthwise: depthwise, "\(name)__filter")
    let bias: Int
    if n.inputs.count > 2, !n.inputs[2].isEmpty {
      guard var b = constant(n.inputs[2]), b.count == filters else { throw OnnxError("Conv needs a constant bias of \(filters)") }
      b.dims = [filters]
      bias = try constantTensor(b)
    } else {
      bias = floatTensor([Float](repeating: 0, count: filters), [filters], "\(name)__bias")
    }
    let result = addTensor("\(name)__nhwc_out", [batch, out[0], out[1], filters], .float32)
    if depthwise {
      emit(
        .depthwiseConv2d, [nhwc, filter, bias], [result],
        .depthwiseConv2d(
          padding: padding, strideW: Int32(strides[1]), strideH: Int32(strides[0]), multiplier: Int32(filters / channels),
          dilationW: Int32(dilations[1]), dilationH: Int32(dilations[0])))
    } else {
      emit(
        .conv2d, [nhwc, filter, bias], [result],
        .conv2d(padding: padding, strideW: Int32(strides[1]), strideH: Int32(strides[0]), dilationW: Int32(dilations[1]), dilationH: Int32(dilations[0])))
    }
    let o = try define(name, [batch, filters, out[0], out[1]], .float32)
    emit(.transpose, [result, int32Tensor([0, 3, 1, 2], "\(name)__nchw_perm")], [o], .transpose)
  }

  /// The weight in TFLite's layout: OIHW to OHWI for CONV_2D, [O, 1, kH, kW]
  /// to [1, kH, kW, O] for DEPTHWISE_CONV_2D. A large fp16 weight is
  /// transposed filter by filter as the file is written, never held whole.
  private mutating func convFilter(_ w: Constant, depthwise: Bool, _ name: String) throws -> Int {
    let (o, i, kh, kw) = (w.dims[0], w.dims[1], w.dims[2], w.dims[3])
    let taps = kh * kw
    let shape = depthwise ? [1, kh, kw, o] : [o, kh, kw, i]
    let size = try Self.elementSize(w)
    if w.type == DataType.float16 && w.count < Self.fp16MinElements || (w.type != DataType.float16 && w.type != DataType.float) {
      let values = try floats(w)
      var permuted = [Float](repeating: 0, count: values.count)
      for f in 0..<o {
        for c in 0..<i {
          for t in 0..<taps {
            let from = (f * i + c) * taps + t
            permuted[depthwise ? t * o + f : (f * taps + t) * i + c] = values[from]
          }
        }
      }
      return floatTensor(permuted, shape, name)
    }
    var e = Encoded()
    if taps == 1 || (!depthwise && i == 1) {
      // Nothing moves: [O, I, 1, 1] is already [O, 1, 1, I].
      e = try stored(w, elementSize: size)
    } else if depthwise {
      e.transposed(Transpose(elements: try transposeElements(w), rows: o, cols: taps, elementSize: size))
    } else {
      // Filter by filter; a weight in the typed fields is decoded once for all of them.
      var elements = try transposeElements(w)
      if case .typed(let t) = elements { elements = .owned(try Elements.littleEndian(t, src)) }
      let filterBytes = i * taps * size
      for f in 0..<o {
        e.transposed(Transpose(elements: elements.slice((f * filterBytes)..<((f + 1) * filterBytes)), rows: i, cols: taps, elementSize: size))
      }
    }
    return storedTensor((buffer(e), w.type == DataType.float16 ? .float16 : .float32), shape, name)
  }

  // MARK: LayerNormalization

  /// LayerNormalization over the axes from `axis` on, as
  ///
  ///     d = x - mean(x);  s = max(max|d|, 1e-2);  y = (d/s) / sqrt(mean((d/s)^2) + eps/s^2)
  ///
  /// then scale and bias. It is LayerNorm exactly in real arithmetic, but
  /// nothing in it exceeds 1 before the square root, where the textbook form
  /// squares deviations up to 5.5e6 in Cinque Terre V3's norms: past fp16's
  /// 65504, which a GPU computing in fp16 turns into wrong outputs. Metal
  /// fuses the textbook form and gets away with it; no Android GPU has been
  /// seen to. The statistics are FLOAT32 like all the math here, which is
  /// what stash_type asks for. Mean and InvStdDev, where the graph reads
  /// them, come from the same statistics: InvStdDev is (1/s) times the
  /// scaled form's reciprocal deviation.
  mutating func layerNorm(_ n: Node) throws {
    let name = n.outputs[0]
    let x = try tensor(n.inputs[0])
    let shape = model.tensors[x].shape
    let axis = try Self.normalize(Int(n.attribute("axis")?.i ?? -1), rank: shape.count)
    let axes = int32Tensor(Array(axis..<shape.count), "\(name)__axes")
    let epsilon = n.attribute("epsilon")?.f ?? 1e-5
    var reduced = shape
    for k in axis..<shape.count { reduced[k] = 1 }
    func step(_ op: TFLite.Op, _ inputs: [Int], _ suffix: String, _ shape: [Int], _ options: TFLite.Options) -> Int {
      let o = addTensor("\(name)__\(suffix)", shape, .float32)
      emit(op, inputs, [o], options)
      return o
    }
    let mean = step(.mean, [x, axes], "mean", reduced, .reducer(keepDims: true))
    let d = step(.sub, [x, mean], "d", shape, .sub)
    let magnitude = step(.abs, [d], "abs", shape, .abs)
    let peak = step(.reduceMax, [magnitude, axes], "peak", reduced, .reducer(keepDims: true))
    let s = step(.maximum, [peak, floatTensor([1e-2], [1], "\(name)__floor")], "s", reduced, .maximumMinimum)
    let r = step(.div, [floatTensor([1], [1], "\(name)__one"), s], "r", reduced, .div)
    let dn = step(.mul, [d, r], "dn", shape, .mul)
    let square = step(.mul, [dn, dn], "sq", shape, .mul)
    let variance = step(.mean, [square, axes], "var", reduced, .reducer(keepDims: true))
    let r2 = step(.mul, [r, r], "r2", reduced, .mul)
    let scaledEpsilon = step(.mul, [r2, floatTensor([epsilon], [1], "\(name)__epsilon")], "eps", reduced, .mul)
    let sum = step(.add, [variance, scaledEpsilon], "ve", reduced, .add)
    let rs = step(.rsqrt, [sum], "rs", reduced, .none)
    var y = Value.tensor(step(.mul, [dn, rs], "normalized", shape, .mul))
    let hasBias = n.inputs.count > 2 && !n.inputs[2].isEmpty
    y = .tensor(try elementwise(.mul, y, value(n.inputs[1]), hasBias ? "\(name)__scaled" : name, .mul, define: !hasBias))
    if hasBias {
      try elementwise(.add, y, value(n.inputs[2]), name, .add)
    }
    if n.outputs.count > 1, !n.outputs[1].isEmpty {
      try alias(n.outputs[1], .tensor(mean))
    }
    if n.outputs.count > 2, !n.outputs[2].isEmpty {
      let o = try define(n.outputs[2], reduced, .float32)
      emit(.mul, [r, rs], [o], .mul)
    }
    count("fp16-safe LayerNormalizations")
  }
}
