import Foundation

/// Copies with the casts the model's inputs need, into whatever type the engine
/// declared. The Swift form of `queues.store`, the bulk casts through Convert.
enum Stage {
  static func store(u8 source: UnsafeRawPointer, count: Int, into dest: UnsafeMutableRawPointer, as type: ElementType) {
    switch type {
    case .float16:
      Convert.u8ToF16(source, dest, count: count)
    case .float:
      let src = source.assumingMemoryBound(to: UInt8.self)
      let out = dest.assumingMemoryBound(to: Float.self)
      for i in 0..<count { out[i] = Float(src[i]) }
    case .uint8:
      dest.copyMemory(from: source, byteCount: count)
    default:
      preconditionFailure("images staged as \(type.name)")
    }
  }

  /// float32 off the wire (possibly unaligned) into a float32 or float16 input.
  static func store(f32 source: UnsafeRawPointer, count: Int, into dest: UnsafeMutableRawPointer, as type: ElementType) {
    switch type {
    case .float:
      dest.copyMemory(from: source, byteCount: count * 4)
    case .float16:
      Convert.f32ToF16(source, dest, count: count)
    default:
      preconditionFailure("scalars staged as \(type.name)")
    }
  }

  /// float16 bits from a queue into a float16 or float32 input, or a uint8
  /// one for an image queue.
  static func store(f16 source: UnsafePointer<UInt16>, count: Int, into dest: UnsafeMutableRawPointer, as type: ElementType) {
    switch type {
    case .float16:
      dest.copyMemory(from: source, byteCount: count * 2)
    case .float:
      Convert.f16ToF32(source, dest, count: count)
    case .uint8:
      Convert.f16ToU8(source, dest, count: count)
    default:
      preconditionFailure("queue staged as \(type.name)")
    }
  }
}

/// A fixed-length FIFO of float16 rows over a preallocated buffer. Logical row
/// i (0 = oldest) lives at physical row (head + i) % n. Ring buffers rather
/// than openpilot's rolling `cat(buf[1:], new)`, which copies 8.4 MB a frame.
final class RingQueue {
  let rows: Int
  let rowCount: Int
  private let buffer: UnsafeMutablePointer<UInt16>
  private var head = 0

  init(shape: [Int]) {
    rows = shape[0]
    rowCount = shape.dropFirst().reduce(1, *)
    buffer = .allocate(capacity: rows * rowCount)
    buffer.initialize(repeating: 0, count: rows * rowCount)
  }

  deinit {
    buffer.deallocate()
  }

  func reset() {
    buffer.update(repeating: 0, count: rows * rowCount)
    head = 0
  }

  /// The slot the oldest row occupies becomes the newest once head moves.
  func push(u8 source: UnsafeRawPointer) {
    Stage.store(u8: source, count: rowCount, into: UnsafeMutableRawPointer(buffer + head * rowCount), as: .float16)
    head = (head + 1) % rows
  }

  func push(f32 source: UnsafeRawPointer) {
    Stage.store(f32: source, count: rowCount, into: UnsafeMutableRawPointer(buffer + head * rowCount), as: .float16)
    head = (head + 1) % rows
  }

  func push(f16 source: UnsafePointer<UInt16>) {
    (buffer + head * rowCount).update(from: source, count: rowCount)
    head = (head + 1) % rows
  }

  func row(_ logical: Int) -> UnsafePointer<UInt16> {
    UnsafePointer(buffer + ((head + logical) % rows) * rowCount)
  }

  /// openpilot's `buf[::frame_skip]`: logical rows 0, step, 2*step, ... oldest
  /// first, written contiguously into `dest`.
  func gather(step: Int, into dest: UnsafeMutableRawPointer, as type: ElementType) {
    var out = 0
    var logical = 0
    while logical < rows {
      Stage.store(f16: row(logical), count: rowCount, into: dest + out * rowCount * type.size, as: type)
      out += 1
      logical += step
    }
  }
}

/// A model's server-side history. The Swift form of `queues.PolicyQueues` and
/// `queues.StateLoop`: the comma sends only the newest warped frame and the
/// packed scalars, and this turns them into the model's inputs, the hidden
/// state the last frame returned included.
protocol FrameStaging: AnyObject {
  /// The frame's sizes, worked out once at load.
  var layout: FrameLayout { get }
  /// RESET_QUEUES: empty history and nothing to feed back.
  func reset()
  /// A hello: the next frame feeds back zeros, as a new modeld's prev_feat
  /// did. The queues stay until the client resets them.
  func newClient()
  /// Writes one frame's inputs into the engine's buffers. `packed` is read
  /// where the request put it, aligned or not.
  func stage(warped: UnsafeRawPointer, packed: UnsafeRawPointer) throws
  /// The frame's driving output as the engine wrote it, all finite, which
  /// the next frame feeds back. Never after NOT_FINITE or a failed run:
  /// modeld fed back only what reached it, and those raise on the comma first.
  func keep(outputs: UnsafeRawPointer, type: ElementType)
}

/// A model's INFER_REQ and reply as the session handles them, from the spec
/// and the engine once at load: the spec derives each size through arrays,
/// which a frame should not build.
struct FrameLayout {
  /// INFER_REQ's payload: the request head, the warped frame, the packed floats.
  let requestBytes: Int
  let warpedBytes: Int
  /// The driving output, float32 on the wire whatever the engine gives, and
  /// where the engine writes it, which stays put from load to close.
  let outputCount: Int
  let outputType: ElementType?
  let output: UnsafeRawPointer?
  /// The part of it the reply leaves out unless asked, when the model has one.
  let hidden: Range<Int>?

  init(spec: ModelSpec, engine: any Engine) {
    requestBytes = spec.inferReqBytes
    warpedBytes = spec.warpedBytes
    outputCount = spec.outputCount
    outputType = engine.outputs[ModelConstants.drivingOutput]?.type
    output = engine.output(ModelConstants.drivingOutput)
    hidden = spec.hiddenRange
  }
}

enum StagingError: Error, CustomStringConvertible {
  case missingInput(String)
  case noStatePairs
  case noHiddenState(Int)

  var description: String {
    switch self {
    case .missingInput(let name): return "the engine has no input \(name) to stage"
    case .noStatePairs: return "the graph takes new_img but returns no next_state_ outputs"
    case .noHiddenState(let count): return "a queued graph needs a hidden_state output of \(count) floats to feed back"
    }
  }
}

/// An engine input's host buffer and type, looked up once at load so a frame
/// looks nothing up. The buffers an engine gives for the inputs the host
/// writes stay put from load to close: EngineCore allocates them once, and
/// only a looped pair's state_ input is ever double-buffered or released,
/// which staging never writes.
struct StagingTarget {
  let pointer: UnsafeMutableRawPointer
  let type: ElementType

  init(_ engine: any Engine, _ name: String) throws {
    guard let io = engine.inputs[name], let pointer = engine.hostInput(name) else { throw StagingError.missingInput(name) }
    self.pointer = pointer
    self.type = io.type
  }
}

/// A queued graph (V1, V2): the image, feature and desire queues live here,
/// and the hidden state each frame returns, which the next pushes into the
/// feature queue where modeld's prev_feat went.
final class PolicyQueues: FrameStaging {
  let layout: FrameLayout
  private let frameSkip: Int
  /// One camera's share of the warped frame.
  private let cameraBytes: Int
  // The queues hold float16, as Python's do, so the cast is paid once per row.
  private let imgQueue: RingQueue
  private let bigImgQueue: RingQueue
  private let featQueue: RingQueue
  private let desireQueue: RingQueue
  private let gathers: [(queue: RingQueue, target: StagingTarget)]
  private let desire: StagingTarget
  private let scalars: [(offset: Int, count: Int, target: StagingTarget)]
  private let desireOffset: Int
  /// Where hidden_state sits in the driving output.
  private let hidden: Range<Int>
  /// The last good frame's hidden state, in the feature queue's float16:
  /// exact whether the engine gave float16 or float32, as the comma's float32
  /// copy of a float16 output was.
  private let prevFeat: UnsafeMutablePointer<UInt16>

  init(spec: ModelSpec, engine: any Engine) throws {
    layout = FrameLayout(spec: spec, engine: engine)
    frameSkip = spec.frameSkip
    cameraBytes = spec.warpedBytes / 2
    guard let hidden = spec.hiddenRange, hidden.count == spec.prevFeatCount else {
      throw StagingError.noHiddenState(spec.prevFeatCount)
    }
    self.hidden = hidden
    imgQueue = RingQueue(shape: spec.imgBufShape)
    bigImgQueue = RingQueue(shape: spec.imgBufShape)
    featQueue = RingQueue(shape: spec.featQShape)
    desireQueue = RingQueue(shape: spec.desireQShape)
    let packed = Dictionary(uniqueKeysWithValues: spec.packedLayout.map { ($0.name, $0.range) })
    gathers = try [(imgQueue, "img"), (bigImgQueue, "big_img"), (featQueue, "features_buffer")].map { ($0, try StagingTarget(engine, $1)) }
    desire = try StagingTarget(engine, "desire_pulse")
    scalars = try ["traffic_convention", "action_t"].map { name in
      let target = try StagingTarget(engine, name)
      return (packed[name]!.lowerBound * 4, engine.inputs[name]!.count, target)
    }
    desireOffset = packed["desire"]!.lowerBound * 4
    prevFeat = .allocate(capacity: hidden.count)
    prevFeat.initialize(repeating: 0, count: hidden.count)
  }

  deinit {
    prevFeat.deallocate()
  }

  func reset() {
    for queue in [imgQueue, bigImgQueue, featQueue, desireQueue] { queue.reset() }
    newClient()
  }

  func newClient() {
    prevFeat.update(repeating: 0, count: hidden.count)
  }

  func keep(outputs: UnsafeRawPointer, type: ElementType) {
    let source = outputs + hidden.lowerBound * type.size
    switch type {
    case .float16: prevFeat.update(from: source.assumingMemoryBound(to: UInt16.self), count: hidden.count)
    default: Stage.store(f32: source, count: hidden.count, into: prevFeat, as: .float16)
    }
  }

  func stage(warped: UnsafeRawPointer, packed: UnsafeRawPointer) throws {
    // push casts into the queue's float16, so the cast costs one row here
    // rather than the whole sampled window later
    imgQueue.push(u8: warped)
    bigImgQueue.push(u8: warped + cameraBytes)
    desireQueue.push(f32: packed + desireOffset)
    featQueue.push(f16: prevFeat)

    for gather in gathers {
      gather.queue.gather(step: frameSkip, into: gather.target.pointer, as: gather.target.type)
    }
    sampleDesire(into: desire.pointer, as: desire.type)
    for scalar in scalars {
      Stage.store(f32: packed + scalar.offset, count: scalar.count, into: scalar.target.pointer, as: scalar.target.type)
    }
  }

  /// openpilot: `buf.reshape(-1, frame_skip, *buf.shape[1:]).max(1)`, the
  /// strongest desire in each group of frame_skip frames. As numpy's max: a
  /// NaN wins, and of two NaNs or two equal values (0 and -0) the earlier
  /// stays, so the result is one of the group's float16s, bit for bit.
  private func sampleDesire(into dest: UnsafeMutableRawPointer, as type: ElementType) {
    let skip = frameSkip
    let width = desireQueue.rowCount
    let groups = desireQueue.rows / skip
    for group in 0..<groups {
      for column in 0..<width {
        var best = desireQueue.row(group * skip)[column]
        for k in 1..<skip {
          let value = desireQueue.row(group * skip + k)[column]
          let kept = Float(Float16(bitPattern: best))
          if !(kept >= Float(Float16(bitPattern: value)) || kept.isNaN) {
            best = value
          }
        }
        let index = group * width + column
        switch type {
        case .float16: dest.assumingMemoryBound(to: UInt16.self)[index] = best
        case .float: dest.assumingMemoryBound(to: Float.self)[index] = Float(Float16(bitPattern: best))
        default: preconditionFailure("desire_pulse staged as \(type.name)")
        }
      }
    }
  }
}

/// A stateful graph (openpilot #38916): the frame goes into new_img and the
/// scalars into their inputs as they are; the engine loops the queues itself.
final class StateLoop: FrameStaging {
  let layout: FrameLayout
  private let engine: any Engine
  /// new_img's type picks the conversion: uint8 would be a plain copy, for
  /// an engine that casts on the device.
  private let frame: StagingTarget
  private let scalars: [(offset: Int, count: Int, target: StagingTarget)]

  init(spec: ModelSpec, engine: any Engine) throws {
    self.engine = engine
    layout = FrameLayout(spec: spec, engine: engine)
    let pairs = spec.statePairs
    guard !pairs.isEmpty else { throw StagingError.noStatePairs }
    for name in [ModelConstants.statefulFrame] + spec.packedLayout.map(\.name) where engine.inputs[name] == nil {
      throw StagingError.missingInput(name)
    }
    // Looping first: that is when an engine settles which buffers it keeps.
    try engine.loopState(pairs)
    frame = try StagingTarget(engine, ModelConstants.statefulFrame)
    scalars = try spec.packedLayout.map { ($0.range.lowerBound * 4, $0.range.count, try StagingTarget(engine, $0.name)) }
  }

  func reset() {
    engine.resetState()
  }

  /// Nothing: the graph's state is not the client's; RESET_QUEUES clears it.
  func newClient() {}

  /// Nothing: the graph keeps its hidden state itself.
  func keep(outputs: UnsafeRawPointer, type: ElementType) {}

  func stage(warped: UnsafeRawPointer, packed: UnsafeRawPointer) throws {
    Stage.store(u8: warped, count: layout.warpedBytes, into: frame.pointer, as: frame.type)
    for scalar in scalars {
      Stage.store(f32: packed + scalar.offset, count: scalar.count, into: scalar.target.pointer, as: scalar.target.type)
    }
  }
}

enum Staging {
  /// Queues for a queued graph, the loop for a stateful one.
  static func forModel(_ spec: ModelSpec, engine: any Engine) throws -> any FrameStaging {
    spec.stateful ? try StateLoop(spec: spec, engine: engine) : try PolicyQueues(spec: spec, engine: engine)
  }
}
