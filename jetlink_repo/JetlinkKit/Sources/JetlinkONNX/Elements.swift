import Foundation

/// A tensor's values, as numpy_helper.to_array reads them: raw_data when the
/// tensor has it, otherwise the typed field onnx keeps that element type in;
/// and elements to and from little-endian bytes. Only for small tensors (the
/// indices, shapes and constants the patches and the LiteRT lowering read or
/// fold) and for a weight in the typed fields at the moment it is transposed.
enum Elements {
  /// The typed field a type's values live in (onnx.helper.tensor_dtype_to_field).
  static func typedField(_ type: Int32) -> Int? {
    switch type {
    case 1, 14: 4
    case 2, 3, 4, 5, 6, 9, 10, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26: 5
    case 7: 7
    case 11, 15: 10
    case 12, 13: 11
    default: nil
    }
  }

  /// The elements' little-endian bytes, as raw_data would hold them.
  static func littleEndian(_ t: Tensor, _ src: Source) throws -> [UInt8] {
    guard !t.isExternal else {
      throw OnnxError("initializer \(t.key) keeps its data in an external file, which the preparation does not read")
    }
    let type = t.elementType
    guard let size = DataType.size(type) else {
      throw OnnxError("initializer \(t.key) has element type \(type), which the preparation cannot read")
    }
    let expected = t.elementCount * size
    var bytes: [UInt8]
    if let raw = t.raw {
      switch raw {
      case .source(let r): bytes = Array(src.slice(r))
      case .owned(let b): bytes = b
      case .transposed: throw OnnxError("initializer \(t.key) is being transposed and has no bytes yet")
      case .widened: throw OnnxError("initializer \(t.key) is being widened to fp32 and has no bytes yet")
      }
    } else if let field = typedField(type), let ranges = t.typed[field] {
      bytes = []
      bytes.reserveCapacity(expected)
      switch field {
      case 4, 10:
        // Packed floats and doubles are the raw layout already.
        for r in ranges { bytes.append(contentsOf: src.slice(r)) }
      default:
        // int32_data holds the narrower types one per varint (fp16 as its
        // bit pattern); int64_data and uint64_data hold theirs whole.
        for r in ranges {
          var reader = src.reader(r)
          while !reader.atEnd {
            let v = try reader.varint()
            for i in 0..<size {
              bytes.append(UInt8(truncatingIfNeeded: v >> (8 * UInt64(i))))
            }
          }
        }
      }
    } else {
      bytes = []
    }
    guard bytes.count == expected else {
      throw OnnxError("initializer \(t.key) has \(bytes.count) bytes of data where its dims \(t.dims) need \(expected)")
    }
    return bytes
  }

  /// How many elements the typed field holds, without decoding them.
  static func typedCount(_ t: Tensor, _ src: Source) throws -> Int {
    let type = t.elementType
    guard let field = typedField(type), let size = DataType.size(type) else { return 0 }
    let ranges = t.typed[field] ?? []
    switch field {
    case 4, 10:
      return ranges.reduce(0) { $0 + $1.count } / size
    default:
      var count = 0
      for r in ranges {
        for b in src.slice(r) where b < 0x80 {
          count += 1
        }
      }
      return count
    }
  }

  /// An integer tensor's values.
  static func integers(_ t: Tensor, _ src: Source) throws -> [Int64] {
    guard DataType.isInteger(t.elementType) else {
      throw OnnxError("initializer \(t.key) has element type \(t.elementType), not an integer type")
    }
    return try integers(littleEndian(t, src), as: t.elementType, for: t.key)
  }

  /// Little-endian integers of `type`, as raw_data holds them.
  static func integers(_ bytes: [UInt8], as type: Int32, for name: String) throws -> [Int64] {
    guard DataType.isInteger(type), let size = DataType.size(type) else {
      throw OnnxError("\(name): element type \(type) is not an integer type")
    }
    let signed = [DataType.int8, DataType.int16, DataType.int32, DataType.int64].contains(type)
    var values: [Int64] = []
    values.reserveCapacity(bytes.count / size)
    var i = 0
    while i < bytes.count {
      var u: UInt64 = 0
      for b in 0..<size {
        u |= UInt64(bytes[i + b]) << (8 * UInt64(b))
      }
      if signed, size < 8, u & (1 << UInt64(size * 8 - 1)) != 0 {
        u |= ~UInt64(0) << UInt64(size * 8)
      }
      values.append(Int64(bitPattern: u))
      i += size
    }
    return values
  }

  /// Integers as raw_data of the given type.
  static func encode(_ values: [Int64], as type: Int32, for name: String) throws -> [UInt8] {
    guard DataType.isInteger(type), let size = DataType.size(type) else {
      throw OnnxError("\(name): element type \(type) is not an integer type")
    }
    let signed = [DataType.int8, DataType.int16, DataType.int32, DataType.int64].contains(type)
    var bytes: [UInt8] = []
    bytes.reserveCapacity(values.count * size)
    for v in values {
      if size < 8 {
        let bits = Int64(size * 8)
        let fits = signed ? (v >= -(1 << (bits - 1)) && v < (1 << (bits - 1))) : (v >= 0 && v < (1 << bits))
        guard fits else { throw OnnxError("\(name): \(v) does not fit element type \(type)") }
      }
      let u = UInt64(bitPattern: v)
      for b in 0..<size {
        bytes.append(UInt8(truncatingIfNeeded: u >> (8 * UInt64(b))))
      }
    }
    return bytes
  }

  /// Little-endian elements of a float type (FLOAT, FLOAT16, DOUBLE or
  /// BFLOAT16), as raw_data holds them, as Floats.
  static func floats(_ bytes: [UInt8], as type: Int32, for name: String) throws -> [Float] {
    func load<T: FixedWidthInteger>(_: T.Type, _ f: (T) -> Float) -> [Float] {
      bytes.withUnsafeBytes { p in
        (0..<(bytes.count / MemoryLayout<T>.size)).map { f(T(littleEndian: p.loadUnaligned(fromByteOffset: $0 * MemoryLayout<T>.size, as: T.self))) }
      }
    }
    switch type {
    case DataType.float: return load(UInt32.self) { Float(bitPattern: $0) }
    case DataType.float16: return load(UInt16.self) { Float(Float16(bitPattern: $0)) }
    case DataType.double: return load(UInt64.self) { Float(Double(bitPattern: $0)) }
    case DataType.bfloat16: return load(UInt16.self) { Float(bitPattern: UInt32($0) << 16) }
    default: throw OnnxError("\(name): element type \(OnnxMeta.typeName(type)) is not a float type")
    }
  }

  /// Floats as little-endian elements of FLOAT, FLOAT16 or DOUBLE, each
  /// rounded to the nearest value the type holds.
  static func encode(_ values: [Float], as type: Int32) -> [UInt8] {
    func store<T: FixedWidthInteger>(_ f: (Float) -> T) -> [UInt8] {
      var bytes: [UInt8] = []
      bytes.reserveCapacity(values.count * MemoryLayout<T>.size)
      for v in values {
        withUnsafeBytes(of: f(v).littleEndian) { bytes.append(contentsOf: $0) }
      }
      return bytes
    }
    switch type {
    case DataType.float: return store { $0.bitPattern }
    case DataType.float16: return store { Float16($0).bitPattern }
    case DataType.double: return store { Double($0).bitPattern }
    default: preconditionFailure("floats are not written as element type \(type)")
    }
  }
}
