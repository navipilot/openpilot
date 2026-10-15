import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkONNX

/// Each lowering on a graph of a node or a few: the file it makes, and what
/// that file computes (TFLiteInterpreter) against ONNX's definition of the
/// op, written out here in plain loops.
@Suite struct LiteRTLoweringTests {
  static let f32 = DataType.float
  static let f16 = DataType.float16

  /// The flat position of `idx` in a row-major `shape`.
  static func at(_ idx: [Int], _ shape: [Int]) -> Int {
    zip(idx, TFLiteInterpreter.strides(shape)).reduce(0) { $0 + $1.0 * $1.1 }
  }

  static func sum(_ n: Int, _ term: (Int) -> Float) -> Float {
    var total: Float = 0
    for i in 0..<n { total += term(i) }
    return total
  }

  static func run(_ file: TFLiteFile, _ inputs: [String: [Float]]) throws -> [String: [Float]] {
    var interpreter = TFLiteInterpreter(file)
    return try interpreter.run(inputs)
  }

  // MARK: elementwise and types

  @Test func elementwiseBroadcastsAsONNXDoes() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3, 4])
    g.input("y", Self.f32, [3, 1])
    g.fp16("c", [4], [0.5, -1, 2, 0.25])
    g.node("Add", ["x", "c"], ["a"])
    g.node("Mul", ["a", "y"], ["m"])
    g.node("Sub", ["m", "x"], ["s"])
    g.node("Div", ["s", "c"], ["d"])
    g.node("Max", ["d", "x"], ["out"])
    g.output("out", Self.f32, [2, 3, 4])
    let (file, _) = try g.convert()
    // y is read at run time with a lower rank: it gets the leading 1.
    #expect(file.count("RESHAPE") == 1)
    #expect(file.opNames.filter { $0 != "RESHAPE" } == ["ADD", "MUL", "SUB", "DIV", "MAXIMUM"])
    var v = SeededValues(seed: 1)
    let x = v(24)
    let y = v(3)
    let c: [Float] = [0.5, -1, 2, 0.25]
    let out = try Self.run(file, ["x": x, "y": y])["out"]!
    let want = TFLiteInterpreter.indices([2, 3, 4]).map { i -> Float in
      let xv = x[Self.at(i, [2, 3, 4])]
      return max(((xv + c[i[2]]) * y[i[1]] - xv) / c[i[2]], xv)
    }
    #expect(maxError(out, want) < 1e-5)
  }

  /// fp16 graph I/O keeps its type behind a CAST at the edge; inside, fp16
  /// and fp32 are both FLOAT32 and the Casts between them go; uint8 stays
  /// uint8 up to the Cast that reads it.
  @Test func castsBetweenFloatTypesDisappear() throws {
    var g = OnnxGraphBuilder()
    g.input("h", Self.f16, [1, 4])
    g.input("img", DataType.uint8, [1, 4])
    g.node("Cast", ["h"], ["h32"], [.int("to", 1)])
    g.node("Cast", ["img"], ["img16"], [.int("to", 10)])
    g.node("Cast", ["img16"], ["img32"], [.int("to", 1)])
    g.node("Add", ["h32", "img32"], ["sum"])
    g.node("Relu", ["sum"], ["r"])
    g.node("Cast", ["r"], ["o"], [.int("to", 10)])
    g.output("o", Self.f16, [1, 4])
    let (file, _) = try g.convert()
    #expect(file.inputs.map { file.tensors[$0].name } == ["h", "img"])
    #expect(file.inputs.map { file.tensors[$0].type } == [TFLite.TensorType.float16.rawValue, TFLite.TensorType.uint8.rawValue])
    #expect(file.outputs.map { file.tensors[$0].name } == ["o"])
    #expect(file.tensors[file.outputs[0]].type == TFLite.TensorType.float16.rawValue)
    // h in, img to float, o out: three CASTs, none for the fp16 <-> fp32 Casts.
    #expect(file.count("CAST") == 3)
    let out = try Self.run(file, ["h": [-2, 0.5, 1, 3], "img": [0, 1, 2, 255]])["o"]!
    #expect(out == [0, 1.5, 3, 258])
  }

  // MARK: convolution

  /// ONNX's Conv, NCHW, in plain loops.
  static func conv(
    _ x: [Float], _ xs: [Int], _ w: [Float], _ ws: [Int], _ b: [Float]?, pads: [Int], strides: [Int], dilations: [Int], group: Int
  ) -> (values: [Float], shape: [Int]) {
    let (n, h, wd) = (xs[0], xs[2], xs[3])
    let (o, cg, kh, kw) = (ws[0], ws[1], ws[2], ws[3])
    let oh = (h + pads[0] + pads[2] - ((kh - 1) * dilations[0] + 1)) / strides[0] + 1
    let ow = (wd + pads[1] + pads[3] - ((kw - 1) * dilations[1] + 1)) / strides[1] + 1
    let shape = [n, o, oh, ow]
    let values = TFLiteInterpreter.indices(shape).map { i -> Float in
      let groupIndex = i[1] / (o / group)
      var acc = b?[i[1]] ?? 0
      for ci in 0..<cg {
        for p in 0..<kh {
          for q in 0..<kw {
            let y = i[2] * strides[0] - pads[0] + p * dilations[0]
            let xx = i[3] * strides[1] - pads[1] + q * dilations[1]
            guard y >= 0, y < h, xx >= 0, xx < wd else { continue }
            acc += x[at([i[0], groupIndex * cg + ci, y, xx], xs)] * w[at([i[1], ci, p, q], ws)]
          }
        }
      }
      return acc
    }
    return (values, shape)
  }

  struct ConvCase: CustomTestStringConvertible, Sendable {
    let name: String
    let input: [Int]
    let weight: [Int]
    let pads: [Int]
    let strides: [Int]
    let dilations: [Int]
    let group: Int
    let bias: Bool
    /// What the lowering should pick: "SAME", "VALID" or "PAD".
    let padding: String

    var testDescription: String { name }
  }

  static let convCases: [ConvCase] = [
    ConvCase(
      name: "3x3 same", input: [1, 3, 5, 6], weight: [4, 3, 3, 3], pads: [1, 1, 1, 1], strides: [1, 1], dilations: [1, 1], group: 1,
      bias: true, padding: "SAME"),
    ConvCase(
      name: "4x4 stride 4, fp16 behind DEQUANTIZE", input: [1, 12, 8, 8], weight: [8, 12, 4, 4], pads: [0, 0, 0, 0], strides: [4, 4],
      dilations: [1, 1], group: 1, bias: true, padding: "VALID"),
    ConvCase(
      name: "asymmetric pads", input: [1, 2, 5, 5], weight: [3, 2, 3, 3], pads: [1, 0, 0, 1], strides: [2, 2], dilations: [1, 1], group: 1,
      bias: false, padding: "PAD"),
    ConvCase(
      name: "depthwise, multiplier 2, dilated", input: [1, 4, 7, 7], weight: [8, 1, 3, 3], pads: [2, 2, 2, 2], strides: [1, 1],
      dilations: [2, 2], group: 4, bias: true, padding: "SAME"),
    ConvCase(
      name: "depthwise 7x7, fp16 behind DEQUANTIZE", input: [1, 32, 6, 6], weight: [32, 1, 7, 7], pads: [3, 3, 3, 3], strides: [1, 1],
      dilations: [1, 1], group: 32, bias: true, padding: "SAME"),
    ConvCase(
      name: "1x1", input: [1, 3, 4, 4], weight: [5, 3, 1, 1], pads: [0, 0, 0, 0], strides: [1, 1], dilations: [1, 1], group: 1, bias: true,
      padding: "VALID"),
  ]

  @Test(arguments: convCases)
  func convComputesONNXsConv(_ c: ConvCase) throws {
    var v = SeededValues(seed: 7)
    let x = v(c.input.reduce(1, *))
    let w = v(c.weight.reduce(1, *))
    let b = v(c.weight[0])
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, c.input.map { Int64($0) })
    g.fp16("w", c.weight.map { Int64($0) }, w)
    if c.bias { g.fp16("b", [Int64(c.weight[0])], b) }
    g.node(
      "Conv", c.bias ? ["x", "w", "b"] : ["x", "w"], ["y"],
      [
        .int("group", Int64(c.group)), .ints("pads", c.pads.map { Int64($0) }),
        .ints("strides", c.strides.map { Int64($0) }), .ints("dilations", c.dilations.map { Int64($0) }),
      ])
    let want = Self.conv(x, c.input, w, c.weight, c.bias ? b : nil, pads: c.pads, strides: c.strides, dilations: c.dilations, group: c.group)
    g.output("y", Self.f32, want.shape.map { Int64($0) })
    let (file, _) = try g.convert()

    let depthwise = c.group > 1
    let op = depthwise ? "DEPTHWISE_CONV_2D" : "CONV_2D"
    #expect(file.count(op) == 1)
    #expect(file.count("TRANSPOSE") == 2)
    #expect(file.count("PAD") == (c.padding == "PAD" ? 1 : 0))
    let conv = try #require(file.operators.first { $0.op == op })
    let same = conv.options?.i8(0) == TFLite.Padding.same.rawValue
    #expect(same == (c.padding == "SAME"))
    let filter = file.tensors[conv.inputs[1]]
    let (o, i, kh, kw) = (c.weight[0], c.weight[1], c.weight[2], c.weight[3])
    #expect(filter.shape == (depthwise ? [1, kh, kw, o] : [o, kh, kw, i]))
    // A large fp16 weight stays fp16, transposed as it was written.
    #expect(file.count("DEQUANTIZE") == (w.count >= 1024 ? 1 : 0))

    let out = try Self.run(file, ["x": x])["y"]!
    #expect(maxError(out, want.values) < 1e-4)
  }

  /// The NHWC convolution meets the graph's own NHWC permute, and the two
  /// TRANSPOSEs in between go.
  @Test func transposePairsCancel() throws {
    var v = SeededValues(seed: 3)
    let w = v(12)
    let x = v(48)
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [1, 3, 4, 4])
    g.fp16("w", [4, 3, 1, 1], w)
    g.node("Conv", ["x", "w"], ["y"])
    g.node("Transpose", ["y"], ["nhwc"], [.ints("perm", [0, 2, 3, 1])])
    g.node("Relu", ["nhwc"], ["r"])
    g.node("Transpose", ["r"], ["back"], [.ints("perm", [0, 3, 1, 2])])
    g.node("Sigmoid", ["back"], ["out"])
    g.output("out", Self.f32, [1, 4, 4, 4])
    let (file, report) = try g.convert()
    // Left: the one into the convolution, and the one back to NCHW at the end.
    #expect(file.count("TRANSPOSE") == 2)
    #expect(report.transposesRemoved == 2)
    let y = Self.conv(x, [1, 3, 4, 4], w, [4, 3, 1, 1], nil, pads: [0, 0, 0, 0], strides: [1, 1], dilations: [1, 1], group: 1)
    let want = y.values.map { (v: Float) -> Float in 1 / (1 + Float(exp(-Double(max(v, 0))))) }
    #expect(maxError(try Self.run(file, ["x": x])["out"]!, want) < 1e-5)
  }

  /// Two ConvNeXt blocks: depthwise convolution, a stretch in NHWC, back to
  /// NCHW for the layer scale and the residual add. The TRANSPOSEs move past
  /// the multiply and the add and cancel, so only the way in and the way out
  /// are left. The layer scale has 1024 channels, so it moves as an fp16
  /// constant behind a DEQUANTIZE of its own.
  @Test func convNeXtBlocksChainInNHWC() throws {
    let c = 1024
    var v = SeededValues(seed: 17)
    let x = v(c * 4)
    let w = [v(c * 9), v(c * 9)]
    let gamma = [v(c), v(c)]
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [1, Int64(c), 2, 2])
    var previous = "x"
    for k in 0..<2 {
      g.fp16("w\(k)", [Int64(c), 1, 3, 3], w[k])
      g.fp16("gamma\(k)", [1, Int64(c), 1, 1], gamma[k])
      g.node("Conv", [previous, "w\(k)"], ["dw\(k)"], [.int("group", Int64(c)), .ints("pads", [1, 1, 1, 1])])
      g.node("Transpose", ["dw\(k)"], ["nhwc\(k)"], [.ints("perm", [0, 2, 3, 1])])
      g.node("Relu", ["nhwc\(k)"], ["mlp\(k)"])
      g.node("Transpose", ["mlp\(k)"], ["nchw\(k)"], [.ints("perm", [0, 3, 1, 2])])
      g.node("Mul", ["nchw\(k)", "gamma\(k)"], ["scaled\(k)"])
      g.node("Add", [previous, "scaled\(k)"], ["block\(k)"])
      previous = "block\(k)"
    }
    g.output("block1", Self.f32, [1, Int64(c), 2, 2])
    let (file, report) = try g.convert()
    #expect(file.count("TRANSPOSE") == 2)
    #expect(report.transposesMoved > 0)
    let gammas = file.operators.filter { $0.op == "MUL" }.map { file.tensors[$0.inputs[1]] }
    #expect(gammas.allSatisfy { $0.shape == [1, 1, 1, c] })

    var want = x
    for k in 0..<2 {
      let dw = Self.conv(want, [1, c, 2, 2], w[k], [c, 1, 3, 3], nil, pads: [1, 1, 1, 1], strides: [1, 1], dilations: [1, 1], group: c)
      want = want.indices.map { want[$0] + max(dw.values[$0], 0) * gamma[k][$0 / 4] }
    }
    #expect(maxError(try Self.run(file, ["x": x])["block1"]!, want) < 1e-4)
  }

  // MARK: Gemm and MatMul

  @Test func gemmIsFullyConnected() throws {
    var v = SeededValues(seed: 11)
    let a = v(2 * 64)
    let wT = v(32 * 64)
    let w = v(64 * 32)
    let bias = v(32)
    let full = v(2 * 32)
    var g = OnnxGraphBuilder()
    g.input("a", Self.f32, [2, 64])
    g.fp16("wT", [32, 64], wT)
    g.fp16("w", [64, 32], w)
    g.fp16("bias", [32], bias)
    g.fp16("row", [1, 32], bias)
    g.fp32("full", [2, 32], full)
    g.node("Gemm", ["a", "wT", "bias"], ["g1"], [.int("transB", 1)])
    g.node("Gemm", ["a", "w", "row"], ["g2"])
    g.node("Gemm", ["a", "wT", "full"], ["g3"], [.int("transB", 1)])
    g.output("g1", Self.f32, [2, 32])
    g.output("g2", Self.f32, [2, 32])
    g.output("g3", Self.f32, [2, 32])
    let (file, _) = try g.convert()
    #expect(file.count("FULLY_CONNECTED") == 3)
    // A bias that is not one row is an ADD after.
    #expect(file.count("ADD") == 1)
    // Both weights are fp16 behind a DEQUANTIZE; w was transposed to [N, K].
    #expect(file.count("DEQUANTIZE") == 2)
    let fc = file.operators.filter { $0.op == "FULLY_CONNECTED" }
    #expect(fc.allSatisfy { file.tensors[$0.inputs[1]].shape == [32, 64] })
    let out = try Self.run(file, ["a": a])
    func gemm(_ weight: (Int, Int) -> Float, _ c: (Int, Int) -> Float) -> [Float] {
      (0..<64).map { (e: Int) -> Float in c(e / 32, e % 32) + Self.sum(64) { a[(e / 32) * 64 + $0] * weight($0, e % 32) } }
    }
    #expect(maxError(out["g1"]!, gemm({ wT[$1 * 64 + $0] }, { bias[$1] })) < 1e-4)
    #expect(maxError(out["g2"]!, gemm({ w[$0 * 32 + $1] }, { bias[$1] })) < 1e-4)
    #expect(maxError(out["g3"]!, gemm({ wT[$1 * 64 + $0] }, { full[$0 * 32 + $1] })) < 1e-4)
  }

  @Test func matmulIsBatchMatmul() throws {
    var v = SeededValues(seed: 12)
    let x = v(2 * 3 * 5)
    let w = v(5 * 4)
    let q = v(2 * 3 * 4)
    let k = v(2 * 4 * 3)
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3, 5])
    g.input("q", Self.f32, [1, 2, 3, 4])
    g.input("k", Self.f32, [2, 4, 3])
    g.fp32("w", [5, 4], w)
    g.node("MatMul", ["x", "w"], ["xw"])
    g.node("MatMul", ["q", "k"], ["qk"])
    g.output("xw", Self.f32, [2, 3, 4])
    g.output("qk", Self.f32, [1, 2, 3, 3])
    let (file, _) = try g.convert()
    #expect(file.count("BATCH_MATMUL") == 2)
    // k is lifted to q's rank; the constant keeps [K, N].
    #expect(file.count("RESHAPE") == 1)
    let out = try Self.run(file, ["x": x, "q": q, "k": k])
    let xw = (0..<24).map { (e: Int) -> Float in Self.sum(5) { x[(e / 4) * 5 + $0] * w[$0 * 4 + e % 4] } }
    let qk = (0..<18).map { (e: Int) -> Float in
      let (b, i, j) = (e / 9, (e / 3) % 3, e % 3)
      return Self.sum(4) { q[(b * 3 + i) * 4 + $0] * k[(b * 4 + $0) * 3 + j] }
    }
    #expect(maxError(out["xw"]!, xw) < 1e-5)
    #expect(maxError(out["qk"]!, qk) < 1e-5)
  }

  // MARK: reductions and Softmax

  @Test(arguments: [Int64(17), 20])
  func reductionsTakeAxesEitherWay(_ opset: Int64) throws {
    var g = OnnxGraphBuilder(opset: opset)
    g.input("x", Self.f32, [2, 3, 4])
    if opset < 18 {
      g.node("ReduceMean", ["x"], ["mean"], [.ints("axes", [1])])
      g.node("ReduceMax", ["x"], ["max"], [.ints("axes", [-1]), .int("keepdims", 0)])
    } else {
      g.int64("one", [1])
      g.int64("last", [-1])
      g.node("ReduceMean", ["x", "one"], ["mean"])
      g.node("ReduceMax", ["x", "last"], ["max"], [.int("keepdims", 0)])
    }
    g.int64("both", [0, 2])
    g.node("ReduceSum", ["x", "both"], ["sum"], [.int("keepdims", 0)])
    g.output("mean", Self.f32, [2, 1, 4])
    g.output("max", Self.f32, [2, 3])
    g.output("sum", Self.f32, [3])
    let (file, _) = try g.convert()
    #expect(Set(file.opNames) == ["MEAN", "REDUCE_MAX", "SUM"])
    var v = SeededValues(seed: 5)
    let x = v(24)
    let out = try Self.run(file, ["x": x])
    let mean = (0..<8).map { (e: Int) -> Float in Self.sum(3) { x[(e / 4) * 12 + $0 * 4 + e % 4] } / 3 }
    let maxima = (0..<6).map { (e: Int) -> Float in (0..<4).map { x[e * 4 + $0] }.max()! }
    let sums = (0..<3).map { (j: Int) -> Float in Self.sum(8) { x[($0 / 4) * 12 + j * 4 + $0 % 4] } }
    #expect(maxError(out["mean"]!, mean) < 1e-6)
    #expect(out["max"]! == maxima)
    #expect(maxError(out["sum"]!, sums) < 1e-5)
  }

  @Test func softmaxOnAnyAxis() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3, 4])
    g.node("Softmax", ["x"], ["last"])
    g.node("Softmax", ["x"], ["middle"], [.int("axis", 1)])
    g.output("last", Self.f32, [2, 3, 4])
    g.output("middle", Self.f32, [2, 3, 4])
    let (file, _) = try g.convert()
    #expect(file.count("SOFTMAX") == 2)
    // The middle axis goes last and back.
    #expect(file.count("TRANSPOSE") == 2)
    var v = SeededValues(seed: 6)
    let x = v(24)
    let out = try Self.run(file, ["x": x])
    func softmax(_ index: (Int, Int) -> Int, rows: Int, n: Int) -> [Float] {
      var r = [Float](repeating: 0, count: 24)
      for row in 0..<rows {
        let e = (0..<n).map { (k: Int) -> Float in Float(exp(Double(x[index(row, k)]))) }
        let total = e.reduce(0, +)
        for k in 0..<n { r[index(row, k)] = e[k] / total }
      }
      return r
    }
    #expect(maxError(out["last"]!, softmax({ $0 * 4 + $1 }, rows: 6, n: 4)) < 1e-6)
    #expect(maxError(out["middle"]!, softmax({ ($0 / 4) * 12 + $1 * 4 + $0 % 4 }, rows: 8, n: 3)) < 1e-6)
  }

  // MARK: layout ops

  @Test func slicesGathersAndReshapes() throws {
    let shape = [3, 4, 5]
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [3, 4, 5])
    g.int64("s1", [1, -3])
    g.int64("e1", [3, 100])
    g.int64("a1", [0, 2])
    g.node("Slice", ["x", "s1", "e1", "a1"], ["sliced"])
    g.int64("s2", [0])
    g.int64("e2", [4])
    g.int64("a2", [1])
    g.int64("st2", [2])
    g.node("Slice", ["x", "s2", "e2", "a2", "st2"], ["strided"])
    g.int64("s3", [-1])
    g.int64("e3", [Int64.min])
    g.int64("a3", [2])
    g.int64("st3", [-1])
    g.node("Slice", ["x", "s3", "e3", "a3", "st3"], ["reversed"])
    g.int64("one", [1], dims: [])
    g.node("Gather", ["x", "one"], ["picked"], [.int("axis", 1)])
    g.int64("cols", [0, 1, -2])
    g.node("Gather", ["x", "cols"], ["columns"], [.int("axis", 2)])
    g.int64("rows", [2, 0], dims: [1, 2])
    g.node("Gather", ["x", "rows"], ["grid"])
    g.int64("sizes", [2, 3])
    g.node("Split", ["x", "sizes"], ["left", "right"], [.int("axis", 2)])
    g.node("Concat", ["right", "left"], ["swapped"], [.int("axis", -1)])
    g.node("Transpose", ["x"], ["moved"], [.ints("perm", [2, 0, 1])])
    g.int64("shape", [0, -1])
    g.node("Reshape", ["x", "shape"], ["flat"])
    g.node("Flatten", ["x"], ["flat2"], [.int("axis", 2)])
    g.int64("ax", [0])
    g.node("Unsqueeze", ["picked", "ax"], ["lifted"])
    g.node("Squeeze", ["lifted", "ax"], ["dropped"])
    for (name, dims) in [
      ("sliced", [2, 4, 3]), ("strided", [3, 2, 5]), ("reversed", [3, 4, 5]), ("picked", [3, 5]), ("columns", [3, 4, 3]),
      ("grid", [1, 2, 4, 5]), ("swapped", [3, 4, 5]), ("moved", [5, 3, 4]), ("flat", [3, 20]), ("flat2", [12, 5]), ("dropped", [3, 5]),
    ] {
      g.output(name, Self.f32, dims.map { Int64($0) })
    }
    let (file, _) = try g.convert()
    #expect(!file.opNames.contains("GATHER"))
    #expect(file.count("STRIDED_SLICE") == 2)
    var v = SeededValues(seed: 9)
    let x = v(60)
    let out = try Self.run(file, ["x": x])
    func view(_ outShape: [Int], _ source: ([Int]) -> [Int]) -> [Float] {
      TFLiteInterpreter.indices(outShape).map { x[Self.at(source($0), shape)] }
    }
    #expect(out["sliced"] == view([2, 4, 3]) { [$0[0] + 1, $0[1], $0[2] + 2] })
    #expect(out["strided"] == view([3, 2, 5]) { [$0[0], $0[1] * 2, $0[2]] })
    #expect(out["reversed"] == view([3, 4, 5]) { [$0[0], $0[1], 4 - $0[2]] })
    #expect(out["picked"] == view([3, 5]) { [$0[0], 1, $0[1]] })
    #expect(out["columns"] == view([3, 4, 3]) { [$0[0], $0[1], [0, 1, 3][$0[2]]] })
    #expect(out["grid"] == view([1, 2, 4, 5]) { [[2, 0][$0[1]], $0[2], $0[3]] })
    #expect(out["swapped"] == view([3, 4, 5]) { [$0[0], $0[1], ($0[2] + 2) % 5] })
    #expect(out["moved"] == view([5, 3, 4]) { [$0[1], $0[2], $0[0]] })
    #expect(out["flat"] == x && out["flat2"] == x)
    #expect(out["dropped"] == out["picked"])
  }

  @Test func expandMultipliesByOnes() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [1, 3, 1])
    g.int64("shape", [2, 1, 3, 4])
    g.node("Expand", ["x", "shape"], ["y"])
    g.output("y", Self.f32, [2, 1, 3, 4])
    let (file, _) = try g.convert()
    #expect(file.count("MUL") == 1)
    let out = try Self.run(file, ["x": [1, -2, 3]])["y"]!
    #expect(out == TFLiteInterpreter.indices([2, 1, 3, 4]).map { [1, -2, 3][$0[2]] })
  }

  // MARK: masks

  /// Where(Not(mask), -inf, scores) before a Softmax, an attention mask: a
  /// multiply and an add with a finite fill, no BOOL tensor left, and the
  /// masked probabilities still exactly 0.
  @Test func attentionMasksBecomeArithmetic() throws {
    let causal = (0..<9).map { $0 / 3 >= $0 % 3 }
    var g = OnnxGraphBuilder()
    g.input("scores", Self.f32, [1, 2, 3, 3])
    g.bool("mask", [1, 1, 3, 3], causal)
    g.fp16("ninf", [], [-.infinity])
    g.node("Not", ["mask"], ["masked"])
    g.node("Where", ["masked", "ninf", "scores"], ["filled"])
    g.node("Softmax", ["filled"], ["p"], [.int("axis", -1)])
    g.output("p", Self.f32, [1, 2, 3, 3])
    let (file, report) = try g.convert()
    #expect(!file.tensors.contains { $0.type == TFLite.TensorType.bool.rawValue })
    #expect(file.opNames == ["MUL", "ADD", "SOFTMAX"])
    #expect(report.lowerings["finite mask fills"] == 1)
    var v = SeededValues(seed: 4)
    let s = v(18)
    let p = try Self.run(file, ["scores": s])["p"]!
    for row in 0..<6 {
      let keep = (0..<3).filter { causal[(row % 3) * 3 + $0] }
      let e = keep.map { (k: Int) -> Float in Float(exp(Double(s[row * 3 + k]))) }
      for k in 0..<3 {
        let want = keep.firstIndex(of: k).map { e[$0] / e.reduce(0, +) } ?? 0
        #expect(abs(p[row * 3 + k] - want) < 1e-6)
        if !keep.contains(k) { #expect(p[row * 3 + k] == 0) }
      }
    }
  }

  /// An infinite fill not read by a Softmax would change under a finite
  /// one, so it stays a select.
  @Test func otherWheresSelect() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 2])
    g.bool("mask", [2, 2], [true, false, false, true])
    g.fp32("ninf", [], [-.infinity])
    g.node("Where", ["mask", "ninf", "x"], ["y"])
    g.output("y", Self.f32, [2, 2])
    let (file, _) = try g.convert()
    #expect(file.opNames == ["SELECT_V2"])
    #expect(try Self.run(file, ["x": [1, 2, 3, 4]])["y"]! == [-.infinity, 2, 3, -.infinity])
  }

  // MARK: LayerNormalization and constants

  /// ONNX's LayerNormalization over the axes from `axis` on, in Double:
  /// the output, the mean and the inverse deviation, row by row.
  static func layerNorm(_ x: [Float], inner: Int, scale: [Float], bias: [Float]?, epsilon: Double)
    -> (y: [Float], mean: [Float], invStd: [Float])
  {
    var (y, means, invStds) = ([Float](), [Float](), [Float]())
    for start in stride(from: 0, to: x.count, by: inner) {
      let row = x[start..<(start + inner)].map { Double($0) }
      let mean = row.reduce(0, +) / Double(inner)
      let invStd = 1 / (row.map { ($0 - mean) * ($0 - mean) }.reduce(0, +) / Double(inner) + epsilon).squareRoot()
      y += row.indices.map { Float((row[$0] - mean) * invStd * Double(scale[$0]) + Double(bias?[$0] ?? 0)) }
      means.append(Float(mean))
      invStds.append(Float(invStd))
    }
    return (y, means, invStds)
  }

  /// Rows that spread widely, as the norms' inputs do in Cinque Terre V3:
  /// squared, their deviations pass fp16's 65504.
  static let spread = (0..<48).map { (i: Int) -> Float in Float((i * 37) % 48) * 300 - 7000 + (i % 3 == 0 ? 2500 : 0) }

  /// Every LayerNormalization in the fp16-safe form, over the last axis or
  /// the last two, with and without a bias: what is squared stays within 1.
  @Test(arguments: [(Int64(-1), true), (1, true), (-1, false)])
  func layerNormalizationIsFP16Safe(_ axis: Int64, _ bias: Bool) throws {
    let inner = axis == -1 ? 8 : 24
    let scale = (0..<inner).map { 0.5 + Float($0) / 16 }
    let shift = (0..<inner).map { Float($0) / 8 - 1 }
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3, 8])
    let dims: [Int64] = axis == -1 ? [8] : [3, 8]
    g.fp16("scale", dims, scale)
    g.fp16("bias", dims, shift)
    g.node("LayerNormalization", bias ? ["x", "scale", "bias"] : ["x", "scale"], ["y"], [.int("axis", axis), .float("epsilon", 1e-5)])
    g.output("y", Self.f32, [2, 3, 8])
    let (file, report) = try g.convert()
    #expect(report.lowerings["fp16-safe LayerNormalizations"] == 1)
    #expect(file.count("RSQRT") == 1 && file.count("REDUCE_MAX") == 1 && file.count("MEAN") == 2)
    var interpreter = TFLiteInterpreter(file)
    let y = try interpreter.run(["x": Self.spread])["y"]!
    let squared = try #require(file.tensor(named: "y__sq"))
    #expect(interpreter.values[squared]!.allSatisfy { $0 <= 1 })
    let want = Self.layerNorm(Self.spread, inner: inner, scale: scale, bias: bias ? shift : nil, epsilon: 1e-5).y
    #expect(maxError(y, want) < 1e-4)
  }

  /// A norm whose Mean and InvStdDev the graph reads gives them from the
  /// scaled statistics.
  @Test func layerNormalizationMeanAndInvStdDev() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3, 8])
    g.fp16("scale", [8], [Float](repeating: 1, count: 8))
    g.node("LayerNormalization", ["x", "scale"], ["y", "mean", "inv"], [.float("epsilon", 1e-3)])
    g.output("y", Self.f32, [2, 3, 8])
    g.output("mean", Self.f32, [2, 3, 1])
    g.output("inv", Self.f32, [2, 3, 1])
    let (file, _) = try g.convert()
    let out = try Self.run(file, ["x": Self.spread])
    let want = Self.layerNorm(Self.spread, inner: 8, scale: [Float](repeating: 1, count: 8), bias: nil, epsilon: 1e-3)
    #expect(maxError(out["y"]!, want.y) < 1e-4)
    #expect(maxError(out["mean"]!, want.mean) < 1e-3)
    #expect(zip(out["inv"]!, want.invStd).allSatisfy { abs($0 - $1) <= 1e-5 * $1 })
  }

  /// What only reads constants is worked out here: no operator is left with
  /// two constant inputs, which the GPU refuses.
  @Test func constantsFold() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3])
    g.fp16("table", [4, 3], (0..<12).map { Float($0) })
    g.int64("pick", [2, 0])
    g.fp16("offset", [3], [0.5, 0.25, 0.125])
    g.node("Gather", ["table", "pick"], ["rows"])
    g.node("Add", ["rows", "offset"], ["shifted"])
    g.node("Cast", ["shifted"], ["wide"], [.int("to", 1)])
    g.node("Mul", ["x", "wide"], ["y"])
    g.output("y", Self.f32, [2, 3])
    let (file, report) = try g.convert()
    #expect(file.opNames == ["MUL"])
    #expect(report.lowerings["folded constant arithmetic"] == 1)
    let y = try Self.run(file, ["x": [1, 1, 1, 2, 2, 2]])["y"]!
    #expect(y == [6.5, 7.25, 8.125, 1, 2.5, 4.25])
  }

  // MARK: rank 5

  /// The uint8 frame queue, rank 5, as tinygrad exports it: the graph's
  /// inputs and outputs become 4-D views of the same bytes, and the layout
  /// ops run on views, so no tensor above rank 4 is left for the GPU.
  @Test func rankFiveRunsOnViews() throws {
    var g = OnnxGraphBuilder()
    g.input("new_img", DataType.uint8, [2, 6, 2, 2])
    g.input("state_img_q", DataType.uint8, [2, 5, 6, 2, 2])
    g.int64("ax1", [1])
    g.int64("one", [1])
    g.int64("end", [Int64.max])
    g.int64("zero", [0])
    g.int64("four", [4])
    g.int64("i0", [0], dims: [])
    g.int64("cam", [1, 12, 2, 2])
    g.node("Unsqueeze", ["new_img", "ax1"], ["frame"])
    g.node("Slice", ["state_img_q", "one", "end", "ax1"], ["tail"])
    g.node("Concat", ["tail", "frame"], ["next_state_img_q"], [.int("axis", 1)])
    g.node("Gather", ["next_state_img_q", "i0"], ["road"])
    g.node("Slice", ["road", "zero", "end", "zero", "four"], ["pair"])
    g.node("Reshape", ["pair", "cam"], ["imgs"])
    g.node("Cast", ["imgs"], ["out"], [.int("to", 1)])
    g.output("out", Self.f32, [1, 12, 2, 2])
    g.output("next_state_img_q", DataType.uint8, [2, 5, 6, 2, 2])
    let (file, report) = try g.convert()
    #expect(file.tensors[file.inputs[1]].shape == [2, 30, 2, 2])
    #expect(file.tensors[file.outputs[1]].shape == [2, 30, 2, 2])
    #expect(file.tensors.allSatisfy { $0.shape.count <= 4 }, "\(file.tensors.filter { $0.shape.count > 4 }.map(\.name))")
    #expect((report.lowerings["graph inputs and outputs as 4-D views"] ?? 0) == 2)
    let fresh = (0..<48).map { Float($0) }
    let queue = (0..<240).map { Float($0 % 256) }
    let out = try Self.run(file, ["new_img": fresh, "state_img_q": queue])
    let next = TFLiteInterpreter.indices([2, 5, 24]).map { i in
      i[1] < 4 ? queue[Self.at([i[0], i[1] + 1, i[2]], [2, 5, 24])] : fresh[Self.at([i[0], i[2]], [2, 24])]
    }
    #expect(out["next_state_img_q"] == next)
    // Camera 0, frames 0 and 4.
    #expect(out["out"] == Array(next[0..<24]) + Array(next[96..<120]))
  }

  /// tinygrad's attention: a reshape to [B, T, 3, H, D], a rank-5 transpose,
  /// and slices of it.
  @Test func rankFiveTransposeRunsOnAView() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [1, 3, 24])
    g.int64("heads", [1, 3, 3, 2, 4])
    g.node("Reshape", ["x", "heads"], ["qkv"])
    g.node("Transpose", ["qkv"], ["split"], [.ints("perm", [2, 0, 3, 1, 4])])
    g.int64("k0", [1])
    g.int64("k1", [2])
    g.int64("ax", [0])
    g.int64("shape", [1, 2, 3, 4])
    g.node("Slice", ["split", "k0", "k1", "ax"], ["key5"])
    g.node("Reshape", ["key5", "shape"], ["key"])
    g.output("key", Self.f32, [1, 2, 3, 4])
    let (file, _) = try g.convert()
    #expect(file.tensors.allSatisfy { $0.shape.count <= 4 }, "\(file.tensors.filter { $0.shape.count > 4 }.map(\.name))")
    let x = (0..<72).map { Float($0) }
    let key = try Self.run(file, ["x": x])["key"]!
    // key[0, h, t, d] = x[0, t, 1 * 8 + h * 4 + d]
    #expect(key == TFLiteInterpreter.indices([1, 2, 3, 4]).map { x[$0[2] * 24 + 8 + $0[1] * 4 + $0[3]] })
  }

  // MARK: the file

  /// fp16 weights of 1024 elements or more stay FLOAT16 behind a DEQUANTIZE
  /// (version 3) and sit after the flatbuffer, 64-byte aligned, with the
  /// ONNX file's bytes; smaller ones are FLOAT32 in the flatbuffer.
  @Test func weightsLiveAfterTheFlatbuffer() throws {
    var v = SeededValues(seed: 21)
    let w = v(64 * 32)
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [1, 64])
    g.fp16("w", [64, 32], w)
    g.fp16("b", [32], v(32))
    g.node("MatMul", ["x", "w"], ["xw"])
    g.node("Add", ["xw", "b"], ["y"])
    g.output("y", Self.f32, [1, 32])
    let (file, report) = try g.convert()
    let deq = try #require(file.operators.first { $0.op == "DEQUANTIZE" })
    #expect(deq.version == 3 && file.operators.first?.op == "DEQUANTIZE")
    let narrow = file.tensors[deq.inputs[0]]
    #expect(narrow.type == TFLite.TensorType.float16.rawValue && narrow.shape == [64, 32])
    guard case .external(let offset, let size) = file.buffers[narrow.buffer] else {
      Issue.record("the fp16 weight is not after the flatbuffer: \(file.buffers[narrow.buffer])")
      return
    }
    #expect(offset % 64 == 0 && offset >= report.flatbufferBytes && size == 64 * 32 * 2)
    #expect(file.data(deq.inputs[0]) == Elements.encode(w, as: DataType.float16))
    let add = try #require(file.operators.first { $0.op == "ADD" })
    let bias = file.tensors[add.inputs[1]]
    #expect(bias.type == TFLite.TensorType.float32.rawValue)
    guard case .inline = file.buffers[bias.buffer] else {
      Issue.record("the small bias is not inline")
      return
    }
    #expect(report.fileBytes == Int64(file.bytes.count))
    #expect(report.weightBytes == report.fileBytes - Int64(report.flatbufferBytes))
  }

  @Test func unknownOpsAreNamed() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [1, 4])
    g.node("Hardmax", ["x"], ["y"])
    g.output("y", Self.f32, [1, 4])
    #expect(throws: OnnxError("Hardmax 'Hardmax_0': Hardmax is not lowered to LiteRT")) { try g.convert() }
  }

  @Test func aWrongRecordedShapeStopsTheLowering() throws {
    var g = OnnxGraphBuilder()
    g.input("x", Self.f32, [2, 3])
    g.node("Relu", ["x"], ["r"])
    g.node("Sigmoid", ["r"], ["y"])
    g.valueInfo("r", Self.f32, [3, 2])
    g.output("y", Self.f32, [2, 3])
    #expect(throws: OnnxError("Relu 'Relu_0': lowered r to shape [2, 3] where the model records [3, 2]")) { try g.convert() }
  }
}
