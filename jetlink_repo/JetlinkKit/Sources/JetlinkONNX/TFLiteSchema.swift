import Foundation

/// The part of TFLite's model schema the LiteRT preparation writes: schema
/// version 3 (with 3a's builtin_code and 3c's buffers outside the
/// flatbuffer), from tflite/converter/schema/schema.fbs at LiteRT's v2.2.0
/// tag, the release whose runtime loads these files. Field slots are the
/// order the schema declares the fields in, deprecated ones included; a
/// union takes two slots, its type and then its value.
enum TFLite {
  /// TensorType.
  enum TensorType: Int8, Sendable {
    case float32 = 0
    case float16 = 1
    case int32 = 2
    case uint8 = 3
    case int64 = 4
    case bool = 6
    case int16 = 7
    case int8 = 9

    var size: Int {
      switch self {
      case .float32, .int32: 4
      case .float16, .int16: 2
      case .uint8, .bool, .int8: 1
      case .int64: 8
      }
    }

    var name: String {
      switch self {
      case .float32: "FLOAT32"
      case .float16: "FLOAT16"
      case .int32: "INT32"
      case .uint8: "UINT8"
      case .int64: "INT64"
      case .bool: "BOOL"
      case .int16: "INT16"
      case .int8: "INT8"
      }
    }
  }

  /// BuiltinOperator, the codes the preparation emits.
  enum Op: Int32, Sendable, CaseIterable {
    case add = 0
    case concatenation = 2
    case conv2d = 3
    case depthwiseConv2d = 4
    case dequantize = 6
    case fullyConnected = 9
    case logistic = 14
    case mul = 18
    case relu = 19
    case reshape = 22
    case softmax = 25
    case tanh = 28
    case pad = 34
    case gather = 36
    case transpose = 39
    case mean = 40
    case sub = 41
    case div = 42
    case stridedSlice = 45
    case exp = 47
    case cast = 53
    case maximum = 55
    case minimum = 57
    case neg = 59
    case slice = 65
    case log = 73
    case sum = 74
    case sqrt = 75
    case rsqrt = 76
    case pow = 78
    case reduceMax = 82
    case reduceMin = 89
    case logicalNot = 87
    case abs = 101
    case selectV2 = 123
    case batchMatmul = 126
    case gelu = 150

    var name: String {
      switch self {
      case .add: "ADD"
      case .concatenation: "CONCATENATION"
      case .conv2d: "CONV_2D"
      case .depthwiseConv2d: "DEPTHWISE_CONV_2D"
      case .dequantize: "DEQUANTIZE"
      case .fullyConnected: "FULLY_CONNECTED"
      case .logistic: "LOGISTIC"
      case .mul: "MUL"
      case .relu: "RELU"
      case .reshape: "RESHAPE"
      case .softmax: "SOFTMAX"
      case .tanh: "TANH"
      case .pad: "PAD"
      case .gather: "GATHER"
      case .transpose: "TRANSPOSE"
      case .mean: "MEAN"
      case .sub: "SUB"
      case .div: "DIV"
      case .stridedSlice: "STRIDED_SLICE"
      case .exp: "EXP"
      case .cast: "CAST"
      case .maximum: "MAXIMUM"
      case .minimum: "MINIMUM"
      case .neg: "NEG"
      case .slice: "SLICE"
      case .log: "LOG"
      case .sum: "SUM"
      case .sqrt: "SQRT"
      case .rsqrt: "RSQRT"
      case .pow: "POW"
      case .reduceMax: "REDUCE_MAX"
      case .reduceMin: "REDUCE_MIN"
      case .logicalNot: "LOGICAL_NOT"
      case .abs: "ABS"
      case .selectV2: "SELECT_V2"
      case .batchMatmul: "BATCH_MATMUL"
      case .gelu: "GELU"
      }
    }

    /// The operator version written. Version 1 everywhere, which every
    /// runtime and the GPU accelerator accept, except DEQUANTIZE: an fp16
    /// input needs version 3.
    var version: Int32 { self == .dequantize ? 3 : 1 }
  }

  enum Padding: Int8 {
    case same = 0
    case valid = 1
  }

  /// One operator's builtin_options, the union member and its fields.
  enum Options: Equatable, Sendable {
    case none
    case conv2d(padding: Padding, strideW: Int32, strideH: Int32, dilationW: Int32, dilationH: Int32)
    case depthwiseConv2d(padding: Padding, strideW: Int32, strideH: Int32, multiplier: Int32, dilationW: Int32, dilationH: Int32)
    case fullyConnected(keepNumDims: Bool)
    case softmax(beta: Float)
    case concatenation(axis: Int32)
    case add, sub, mul, div
    case reshape(newShape: [Int32])
    case pad
    case gather(axis: Int32)
    case transpose
    case reducer(keepDims: Bool)
    case stridedSlice(beginMask: Int32, endMask: Int32)
    case cast(from: TensorType, to: TensorType)
    case dequantize
    case maximumMinimum
    case slice
    case abs
    case selectV2
    case batchMatmul(adjX: Bool, adjY: Bool)
    case gelu(approximate: Bool)
    case pow
    case exp
    case neg
    case logicalNot

    /// The BuiltinOptions union's type: the member's position, from 1.
    var type: UInt8 {
      switch self {
      case .none: 0
      case .conv2d: 1
      case .depthwiseConv2d: 2
      case .fullyConnected: 8
      case .softmax: 9
      case .concatenation: 10
      case .add: 11
      case .reshape: 17
      case .mul: 21
      case .pad: 22
      case .gather: 23
      case .transpose: 26
      case .reducer: 27
      case .sub: 28
      case .div: 29
      case .stridedSlice: 32
      case .exp: 33
      case .cast: 37
      case .dequantize: 38
      case .maximumMinimum: 39
      case .neg: 42
      case .slice: 48
      case .pow: 56
      case .logicalNot: 63
      case .abs: 78
      case .selectV2: 98
      case .batchMatmul: 101
      case .gelu: 116
      }
    }

    /// Writes the options table, nil for none.
    func write(_ b: inout FlatBufferBuilder) -> FlatBufferBuilder.Offset? {
      switch self {
      case .none:
        return nil
      case .conv2d(let padding, let strideW, let strideH, let dilationW, let dilationH):
        // padding, stride_w, stride_h, fused_activation_function, dilation_w_factor, dilation_h_factor
        b.startTable(fields: 7)
        b.add(1, strideW)
        b.add(2, strideH)
        b.add(4, dilationW, default: 1)
        b.add(5, dilationH, default: 1)
        b.add(0, padding.rawValue)
        return b.endTable()
      case .depthwiseConv2d(let padding, let strideW, let strideH, let multiplier, let dilationW, let dilationH):
        // padding, stride_w, stride_h, depth_multiplier, fused_activation_function, dilation_w_factor, dilation_h_factor
        b.startTable(fields: 7)
        b.add(1, strideW)
        b.add(2, strideH)
        b.add(3, multiplier)
        b.add(5, dilationW, default: 1)
        b.add(6, dilationH, default: 1)
        b.add(0, padding.rawValue)
        return b.endTable()
      case .fullyConnected(let keepNumDims):
        // fused_activation_function, weights_format, keep_num_dims, ...
        b.startTable(fields: 5)
        b.add(2, keepNumDims)
        return b.endTable()
      case .softmax(let beta):
        b.startTable(fields: 1)
        b.add(0, beta)
        return b.endTable()
      case .concatenation(let axis):
        // axis, fused_activation_function
        b.startTable(fields: 2)
        b.add(0, axis)
        return b.endTable()
      case .reshape(let newShape):
        let shape = b.vector(newShape)
        b.startTable(fields: 1)
        b.add(0, shape)
        return b.endTable()
      case .gather(let axis):
        // axis, batch_dims
        b.startTable(fields: 2)
        b.add(0, axis)
        return b.endTable()
      case .reducer(let keepDims):
        b.startTable(fields: 1)
        b.add(0, keepDims)
        return b.endTable()
      case .stridedSlice(let beginMask, let endMask):
        // begin_mask, end_mask, ellipsis_mask, new_axis_mask, shrink_axis_mask, offset
        b.startTable(fields: 6)
        b.add(0, beginMask)
        b.add(1, endMask)
        return b.endTable()
      case .cast(let from, let to):
        // in_data_type, out_data_type
        b.startTable(fields: 2)
        b.add(0, from.rawValue, force: true)
        b.add(1, to.rawValue, force: true)
        return b.endTable()
      case .batchMatmul(let adjX, let adjY):
        // adj_x, adj_y, asymmetric_quantize_inputs
        b.startTable(fields: 3)
        b.add(0, adjX)
        b.add(1, adjY)
        return b.endTable()
      case .gelu(let approximate):
        b.startTable(fields: 1)
        b.add(0, approximate)
        return b.endTable()
      case .add, .sub:
        // fused_activation_function, pot_scale_int16 (true)
        b.startTable(fields: 2)
        return b.endTable()
      case .mul, .div, .pad, .transpose, .dequantize, .maximumMinimum, .slice, .abs, .selectV2, .pow, .exp, .neg,
        .logicalNot:
        b.startTable(fields: 0)
        return b.endTable()
      }
    }
  }

  struct Tensor: Equatable, Sendable {
    var name: String
    var shape: [Int]
    var type: TensorType
    /// Index into the model's buffers; 0 is the empty buffer every tensor
    /// without constant data points at.
    var buffer: Int = 0

    var elementCount: Int { shape.reduce(1, *) }
  }

  struct Operator: Equatable, Sendable {
    var op: Op
    /// Tensor indices; -1 is an optional input left out.
    var inputs: [Int]
    var outputs: [Int]
    var options: Options = .none
  }

  /// Where a buffer's bytes are, once the file is laid out.
  enum Placement: Equatable {
    /// The empty buffer.
    case empty
    /// In the flatbuffer, as Buffer.data.
    case inline([UInt8])
    /// After the flatbuffer: `offset` from the start of the file, `size`
    /// bytes. Schema 3c's layout for models over 2 GB, which every LiteRT
    /// runtime reads; an offset of 0 or 1 means none.
    case external(offset: UInt64, size: UInt64)
  }

  /// One subgraph, one signature: everything a driving model needs.
  struct Model {
    var tensors: [Tensor] = []
    var operators: [Operator] = []
    var inputs: [Int] = []
    var outputs: [Int] = []
    var description = "jetlink"
    /// The signature's key; LiteRT's APIs look a signature up by it.
    var signatureKey = "serving_default"
  }

  /// The flatbuffer for `model` with its buffers placed as `buffers` says,
  /// and where each external buffer's offset is in it (8 bytes, little
  /// endian), by buffer index. Offsets are written whatever their value, so
  /// one can be written in later, once the flatbuffer's length says where
  /// the buffers after it start.
  static func encode(_ model: Model, buffers: [Placement]) -> (bytes: [UInt8], offsetFields: [Int: Int]) {
    var b = FlatBufferBuilder(capacity: 1 << 20)

    // Operator codes, in the order operators first use them.
    var codeIndex: [Op: Int] = [:]
    var codes: [Op] = []
    for o in model.operators where codeIndex[o.op] == nil {
      codeIndex[o.op] = codes.count
      codes.append(o.op)
    }

    // Each external offset's distance from the end, which stays where it is
    // as the builder writes towards the front.
    var offsetsFromEnd: [Int: Int] = [:]
    let bufferTables = buffers.enumerated().map { i, placement -> FlatBufferBuilder.Offset in
      switch placement {
      case .empty:
        b.startTable(fields: 3)
        return b.endTable()
      case .inline(let data):
        // data:[ubyte] (force_align: 16)
        let vector = data.withUnsafeBytes { b.bytes($0, alignment: 16) }
        b.startTable(fields: 3)
        b.add(0, vector)
        return b.endTable()
      case .external(let offset, let size):
        b.startTable(fields: 3)
        b.add(1, offset, force: true)
        offsetsFromEnd[i] = b.size
        b.add(2, size, force: true)
        return b.endTable()
      }
    }

    let tensorTables = model.tensors.map { t -> FlatBufferBuilder.Offset in
      let shape = b.vector(t.shape.map { Int32($0) })
      let signature = b.vector(t.shape.map { Int32($0) })
      let name = b.string(t.name)
      // shape, type, buffer, name, quantization, is_variable, sparsity, shape_signature, has_rank
      b.startTable(fields: 9)
      b.add(0, shape)
      b.add(2, UInt32(t.buffer))
      b.add(3, name)
      b.add(7, signature)
      b.add(1, t.type.rawValue)
      b.add(8, true)
      return b.endTable()
    }

    let operatorTables = model.operators.map { o -> FlatBufferBuilder.Offset in
      let inputs = b.vector(o.inputs.map { Int32($0) })
      let outputs = b.vector(o.outputs.map { Int32($0) })
      let options = o.options.write(&b)
      // opcode_index, inputs, outputs, builtin_options_type, builtin_options, ...
      b.startTable(fields: 5)
      b.add(0, UInt32(codeIndex[o.op]!))
      b.add(1, inputs)
      b.add(2, outputs)
      b.add(4, options)
      b.add(3, o.options.type)
      return b.endTable()
    }

    let subgraph: FlatBufferBuilder.Offset = {
      let tensors = b.vector(tensorTables)
      let inputs = b.vector(model.inputs.map { Int32($0) })
      let outputs = b.vector(model.outputs.map { Int32($0) })
      let operators = b.vector(operatorTables)
      let name = b.string("main")
      // tensors, inputs, outputs, operators, name
      b.startTable(fields: 5)
      b.add(0, tensors)
      b.add(1, inputs)
      b.add(2, outputs)
      b.add(3, operators)
      b.add(4, name)
      return b.endTable()
    }()

    let codeTables = codes.map { op -> FlatBufferBuilder.Offset in
      // deprecated_builtin_code, custom_code, version, builtin_code. The old
      // byte field holds codes below 127 and 127
      // (PLACEHOLDER_FOR_GREATER_OP_CODES) for the rest.
      b.startTable(fields: 4)
      b.add(3, op.rawValue, force: true)
      b.add(2, op.version, default: 1)
      b.add(0, Int8(min(op.rawValue, 127)), force: true)
      return b.endTable()
    }

    let signature: FlatBufferBuilder.Offset = {
      func maps(_ indices: [Int]) -> FlatBufferBuilder.Offset {
        let entries = indices.map { i -> FlatBufferBuilder.Offset in
          let name = b.string(model.tensors[i].name)
          // name, tensor_index
          b.startTable(fields: 2)
          b.add(0, name)
          b.add(1, UInt32(i), force: true)
          return b.endTable()
        }
        return b.vector(entries)
      }
      let inputs = maps(model.inputs)
      let outputs = maps(model.outputs)
      let key = b.string(model.signatureKey)
      // inputs, outputs, signature_key, deprecated_tag, subgraph_index
      b.startTable(fields: 5)
      b.add(0, inputs)
      b.add(1, outputs)
      b.add(2, key)
      b.add(4, UInt32(0), force: true)
      return b.endTable()
    }()

    let codesVector = b.vector(codeTables)
    let subgraphs = b.vector([subgraph])
    let description = b.string(model.description)
    let buffersVector = b.vector(bufferTables)
    let signatures = b.vector([signature])
    // version, operator_codes, subgraphs, description, buffers, metadata_buffer, metadata, signature_defs
    b.startTable(fields: 8)
    b.add(0, UInt32(3))
    b.add(1, codesVector)
    b.add(2, subgraphs)
    b.add(3, description)
    b.add(4, buffersVector)
    b.add(7, signatures)
    let bytes = b.finish(b.endTable(), identifier: "TFL3")
    return (bytes, offsetsFromEnd.mapValues { bytes.count - $0 })
  }
}
