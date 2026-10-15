import Foundation

/// ONNX element types the engine stages, with their sizes.
public enum ElementType: Int32, Sendable {
  case float = 1
  case uint8 = 2
  case int8 = 3
  case uint16 = 4
  case int16 = 5
  case int32 = 6
  case int64 = 7
  case bool = 9
  case float16 = 10
  case double = 11

  public var size: Int {
    switch self {
    case .uint8, .int8, .bool: 1
    case .uint16, .int16, .float16: 2
    case .float, .int32: 4
    case .int64, .double: 8
    }
  }

  public var name: String {
    switch self {
    case .float: "float32"
    case .uint8: "uint8"
    case .int8: "int8"
    case .uint16: "uint16"
    case .int16: "int16"
    case .int32: "int32"
    case .int64: "int64"
    case .bool: "bool"
    case .float16: "float16"
    case .double: "float64"
    }
  }
}

/// One input or output as a session declares it.
public struct TensorSpec: Sendable, Equatable {
  public let name: String
  public let type: ElementType
  public let shape: [Int]

  public init(name: String, type: ElementType, shape: [Int]) {
    self.name = name
    self.type = type
    self.shape = shape
  }

  public var count: Int { shape.reduce(1, *) }
  public var byteCount: Int { count * type.size }
}

extension TensorSpec {
  /// The `count` inputs or outputs a runtime's C shim describes, `info`
  /// filling in one's name (NUL terminated), ONNX element type, dims and
  /// rank. Throws for one jetlink cannot stage: an element type ElementType
  /// does not name, or a dimension that is not fixed.
  package static func described(
    count: Int, _ info: (_ index: Int, _ name: inout [CChar], _ type: inout Int32, _ dims: inout [Int64], _ rank: inout Int) throws -> Void
  ) throws -> [TensorSpec] {
    try (0..<count).map { index in
      var name = [CChar](repeating: 0, count: 512)
      var type: Int32 = 0
      var dims = [Int64](repeating: 0, count: 16)
      var rank = 0
      try info(index, &name, &type, &dims, &rank)
      let tensor = String(decoding: name.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, as: UTF8.self)
      guard let element = ElementType(rawValue: type) else {
        throw HostError.failed("\(tensor) has ONNX element type \(type), which jetlink does not stage")
      }
      let shape = dims.prefix(rank).map { Int($0) }
      if shape.contains(where: { $0 <= 0 }) {
        throw HostError.failed("\(tensor) has a dynamic shape \(shape); jetlink builds fixed-shape engines")
      }
      return TensorSpec(name: tensor, type: element, shape: shape)
    }
  }
}
