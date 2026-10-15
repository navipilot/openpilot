import Foundation

/// A driving model's graph metadata: the inputs and outputs with their shapes
/// and element types, and the model's metadata_props, where openpilot keeps
/// output_slices and model_checkpoint. The Swift side of jetlink/onnx_meta.py.
///
/// Reading it walks the protobuf's tags and lengths only. The file is
/// memory-mapped and the nodes and initializers are skipped by their lengths,
/// so the metadata of a 766 MB model costs a few pages of it.
public struct OnnxMeta: Sendable, Equatable {
  public struct Tensor: Sendable, Equatable {
    public let name: String
    /// ONNX's TensorProto.DataType; 0 when the file records none.
    public let elemType: Int32
    /// The shape. A symbolic dimension (a dim_param, or no value at all) is 0,
    /// as onnx_meta.py reports it: `dim_value` of a dimension without one.
    public let dims: [Int64]

    public init(name: String, elemType: Int32, dims: [Int64]) {
      self.name = name
      self.elemType = elemType
      self.dims = dims
    }

    /// numpy's name for the element type.
    public var typeName: String { OnnxMeta.typeName(elemType) }
  }

  public struct Prop: Sendable, Equatable {
    public let key: String
    public let value: String

    public init(key: String, value: String) {
      self.key = key
      self.value = value
    }
  }

  public let inputs: [Tensor]
  public let outputs: [Tensor]
  /// metadata_props in file order. A key can in principle repeat; `prop(_:)`
  /// takes the last, as onnx_meta.py's dict does.
  public let props: [Prop]

  public init(inputs: [Tensor], outputs: [Tensor], props: [Prop]) {
    self.inputs = inputs
    self.outputs = outputs
    self.props = props
  }

  public static func read(contentsOf url: URL) throws -> OnnxMeta {
    let data = try Data(contentsOf: url, options: .alwaysMapped)
    return try read(data)
  }

  public static func read(_ data: Data) throws -> OnnxMeta {
    try data.withUnsafeBytes { buf in
      let meta = try Decode.meta(Source(bytes: buf))
      func tensors(_ vis: [ValueInfo]) -> [Tensor] {
        vis.map { vi in
          Tensor(name: vi.key, elemType: vi.elemType, dims: (vi.shape?.dims ?? []).map { $0.value ?? 0 })
        }
      }
      return OnnxMeta(
        inputs: tensors(meta.inputs), outputs: tensors(meta.outputs),
        props: meta.props.map { Prop(key: $0.key, value: $0.value) })
    }
  }

  public func prop(_ key: String) -> String? {
    props.last { $0.key == key }?.value
  }

  public var modelCheckpoint: String? { prop("model_checkpoint") }

  /// openpilot's output_slices, decoded from the base64 pickle it stores.
  public func outputSlices() throws -> [OutputSlice] {
    guard let raw = prop("output_slices") else {
      throw OnnxError("output_slices not in model metadata_props")
    }
    return try OutputSlices.decode(base64: raw)
  }

  /// numpy's dtype name for an ONNX element type, and `unknown(<n>)` for the rest.
  public static func typeName(_ elemType: Int32) -> String {
    switch elemType {
    case 1: "float32"
    case 2: "uint8"
    case 3: "int8"
    case 4: "uint16"
    case 5: "int16"
    case 6: "int32"
    case 7: "int64"
    case 9: "bool"
    case 10: "float16"
    case 11: "float64"
    case 12: "uint32"
    case 13: "uint64"
    default: "unknown(\(elemType))"
    }
  }
}
