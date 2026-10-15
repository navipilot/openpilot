import Foundation
import JetlinkKit

/// openpilot's ModelConstants, as `jetlink/spec.py` duplicates them.
public enum ModelConstants {
  public static let runFrequency = Pinned.modelRunFrequency
  public static let contextFrequency = Pinned.modelContextFrequency
  public static let defaultFrameSkip = Pinned.defaultFrameSkip  // run / context, 4
  /// The upload chunk the comma sends a model in.
  public static let chunk = Pinned.uploadChunk
  /// The driving output every layout has, 18452 floats in openpilot's layout.
  public static let drivingOutput = "outputs"
  /// The slice of it that stays on the server; 16384 of the 18452.
  public static let hiddenState = "hidden_state"
  /// The input only a stateful graph (openpilot #38916) has.
  public static let statefulFrame = "new_img"
  public static let stateOutputPrefix = "next_"
}

public struct NamedShape: Sendable, Equatable {
  public let name: String
  public let shape: [Int]

  public init(_ name: String, _ shape: [Int]) {
    self.name = name
    self.shape = shape
  }

  public var count: Int { shape.reduce(1, *) }
}

/// One of openpilot's output_slices as Python's `slice` holds it: either end
/// may be None, and either may count from the end, as Lebowski's `pad`,
/// `slice(-2, None)`, does. Kept as written, so the spec goes back on the
/// wire as `to_dict` wrote it.
public struct NamedSlice: Sendable, Equatable {
  public let name: String
  public let start: Int?
  public let stop: Int?

  public init(_ name: String, start: Int?, stop: Int?) {
    self.name = name
    self.start = start
    self.stop = stop
  }

  public init(_ name: String, _ range: Range<Int>) {
    self.init(name, start: range.lowerBound, stop: range.upperBound)
  }

  /// The indices it takes of `count`, as Python's `slice.indices` gives them.
  public func range(in count: Int) -> Range<Int> {
    func index(_ value: Int?, _ none: Int) -> Int {
      guard let value else { return none }
      return value < 0 ? max(value + count, 0) : min(value, count)
    }
    let lower = index(start, 0)
    return lower..<max(lower, index(stop, count))
  }
}

/// Everything both ends need to agree on, derived from the ONNX. The Swift
/// twin of `jetlink/spec.py`'s ModelSpec: the same properties, the same wire
/// sizes, and the same dict on the wire.
public struct ModelSpec: Sendable, Equatable {
  public let sha256: String
  public let nbytes: Int64
  public let frameSkip: Int
  public let inputShapes: [NamedShape]
  public let outputShapes: [NamedShape]
  public let outputSlices: [NamedSlice]
  public let checkpoint: String?

  public init(
    sha256: String, nbytes: Int64, frameSkip: Int, inputShapes: [NamedShape], outputShapes: [NamedShape], outputSlices: [NamedSlice], checkpoint: String?
  ) {
    self.sha256 = sha256
    self.nbytes = nbytes
    self.frameSkip = frameSkip
    self.inputShapes = inputShapes
    self.outputShapes = outputShapes
    self.outputSlices = outputSlices
    self.checkpoint = checkpoint
  }

  public func input(_ name: String) -> [Int]? {
    inputShapes.first { $0.name == name }?.shape
  }

  public func output(_ name: String) -> [Int]? {
    outputShapes.first { $0.name == name }?.shape
  }

  public func withFrameSkip(_ frameSkip: Int) -> ModelSpec {
    ModelSpec(
      sha256: sha256, nbytes: nbytes, frameSkip: frameSkip, inputShapes: inputShapes, outputShapes: outputShapes, outputSlices: outputSlices,
      checkpoint: checkpoint)
  }

  // MARK: layout

  /// The graph keeps its own history (openpilot #38916).
  public var stateful: Bool { input(ModelConstants.statefulFrame) != nil }

  /// state_<q> input to the next_state_<q> output that feeds it next frame.
  /// Empty for a queued graph. Same rule as openpilot's ModelState.
  public var statePairs: [(input: String, output: String)] {
    inputShapes.compactMap { entry in
      let next = ModelConstants.stateOutputPrefix + entry.name
      return output(next) != nil ? (entry.name, next) : nil
    }
  }

  // MARK: vision

  /// (1, 12, H, W); queued graphs only.
  public var imgShape: [Int] { input("img") ?? [] }

  public var nFrames: Int { (imgShape.count > 1 ? imgShape[1] : 0) / 6 }

  public var modelHW: (h: Int, w: Int) {
    // new_img is (2, 6, H, W) and img (1, 12, H, W): the last two either way
    let shape = stateful ? (input(ModelConstants.statefulFrame) ?? []) : imgShape
    guard shape.count >= 2 else { return (0, 0) }
    return (shape[shape.count - 2], shape[shape.count - 1])
  }

  public var imgBufShape: [Int] {
    let (h, w) = modelHW
    return [frameSkip * (nFrames - 1) + 1, 6, h, w]
  }

  /// What `warp` on the comma produces: narrow and wide stacked.
  public var warpedShape: [Int] {
    let (h, w) = modelHW
    return [2, 6, h, w]
  }

  public var warpedBytes: Int { warpedShape.reduce(1, *) }

  // MARK: recurrent and scalar inputs

  /// Flattened per-frame feature size. (1,32,32,512) -> 16384.
  public var featDim: Int {
    guard let fb = input("features_buffer"), fb.count > 2 else { return 0 }
    return fb[2...].reduce(1, *)
  }

  /// The floats a frame sends after the image: compile_modeld's
  /// packed_npy_inputs less prev_feat, which the server keeps.
  public var packedShapes: [NamedShape] {
    let scalars = [
      NamedShape("traffic_convention", input("traffic_convention") ?? []),
      NamedShape("action_t", input("action_t") ?? []),
    ]
    if stateful {
      // the pulse is the graph's own input
      return [NamedShape("desire", [(input("desire") ?? []).reduce(1, *)])] + scalars
    }
    let dp = input("desire_pulse") ?? [0, 0, 0]
    return [NamedShape("desire", [dp.count > 2 ? dp[2] : 0])] + scalars
  }

  /// The hidden state a queued graph's server feeds back each frame, as floats.
  public var prevFeatCount: Int { (input("features_buffer")?.first ?? 0) * featDim }

  /// Where each of packedShapes sits in the flat floats.
  public var packedLayout: [(name: String, range: Range<Int>, shape: [Int])] {
    var out: [(String, Range<Int>, [Int])] = []
    var offset = 0
    for entry in packedShapes {
      out.append((entry.name, offset..<(offset + entry.count), entry.shape))
      offset += entry.count
    }
    return out
  }

  public var packedCount: Int { packedShapes.reduce(0) { $0 + $1.count } }
  public var packedBytes: Int { packedCount * 4 }

  public var featQShape: [Int] {
    let fb = input("features_buffer") ?? [0, 0]
    return [frameSkip * fb[1], fb[0], featDim]
  }

  public var desireQShape: [Int] {
    let dp = input("desire_pulse") ?? [0, 0, 0]
    return [frameSkip * dp[1], dp[0], dp[2]]
  }

  // MARK: output

  public var outputCount: Int { (output(ModelConstants.drivingOutput) ?? []).reduce(1, *) }
  /// float32 on the wire, as openpilot's JIT returns.
  public var outputBytes: Int { outputCount * 4 }

  /// Where hidden_state sits in the output: what the reply leaves out. Nil
  /// when the model names no such slice or it takes nothing, and then the
  /// reply is whole. Its ends resolve as Python's `slice.indices` resolves
  /// them, as `hidden_range` does, so both ends of the wire size the reply
  /// alike (the layout conformance fixture holds them to it).
  public var hiddenRange: Range<Int>? {
    guard let range = outputSlices.first(where: { $0.name == ModelConstants.hiddenState })?.range(in: outputCount), !range.isEmpty
    else { return nil }
    return range
  }

  /// The floats an INFER_RESP carries: the output less hidden_state.
  public var replyCount: Int { outputCount - (hiddenRange?.count ?? 0) }

  // MARK: wire sizes

  public var inferReqBytes: Int { Wire.inferReqSize + warpedBytes + packedBytes }
  /// Without telemetry; Flag.wantHidden adds hidden_state back.
  public var inferRespBytes: Int { Wire.inferRespSize + replyCount * 4 }

  // MARK: the wire form, spec.py's to_dict and from_dict

  public func dictionary() -> [String: Any] {
    [
      "sha256": sha256,
      "nbytes": nbytes,
      "frame_skip": frameSkip,
      "checkpoint": checkpoint ?? NSNull(),
      "input_shapes": Dictionary(uniqueKeysWithValues: inputShapes.map { ($0.name, $0.shape) }),
      "output_shapes": Dictionary(uniqueKeysWithValues: outputShapes.map { ($0.name, $0.shape) }),
      "output_slices": Dictionary(uniqueKeysWithValues: outputSlices.map { ($0.name, [$0.start ?? NSNull(), $0.stop ?? NSNull()] as [Any]) }),
    ]
  }

  public enum DecodeError: Error, CustomStringConvertible {
    case missing(String)

    public var description: String {
      switch self {
      case .missing(let field): return "model spec has no valid \(field)"
      }
    }
  }

  public static func from(_ d: [String: Any]) throws -> ModelSpec {
    guard let sha = d["sha256"] as? String else { throw DecodeError.missing("sha256") }
    guard let nbytes = (d["nbytes"] as? NSNumber)?.int64Value else { throw DecodeError.missing("nbytes") }
    let frameSkip = (d["frame_skip"] as? NSNumber)?.intValue ?? ModelConstants.defaultFrameSkip
    func shapes(_ key: String) throws -> [NamedShape] {
      guard let raw = d[key] as? [String: Any] else { throw DecodeError.missing(key) }
      return try raw.keys.sorted().map { name in
        guard let dims = raw[name] as? [NSNumber] else { throw DecodeError.missing("\(key).\(name)") }
        return NamedShape(name, dims.map(\.intValue))
      }
    }
    guard let rawSlices = d["output_slices"] as? [String: Any] else { throw DecodeError.missing("output_slices") }
    let slices = try rawSlices.keys.sorted().map { name -> NamedSlice in
      // [start, stop], each an int or null, as slice(*v) takes them back
      guard let bounds = rawSlices[name] as? [Any], bounds.count == 2, bounds.allSatisfy({ $0 is NSNumber || $0 is NSNull }) else {
        throw DecodeError.missing("output_slices.\(name)")
      }
      return NamedSlice(name, start: (bounds[0] as? NSNumber)?.intValue, stop: (bounds[1] as? NSNumber)?.intValue)
    }
    return ModelSpec(
      sha256: sha, nbytes: nbytes, frameSkip: frameSkip,
      inputShapes: try shapes("input_shapes"), outputShapes: try shapes("output_shapes"),
      outputSlices: slices, checkpoint: d["checkpoint"] as? String)
  }
}
