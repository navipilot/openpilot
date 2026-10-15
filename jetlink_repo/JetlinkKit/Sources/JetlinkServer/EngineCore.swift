import Foundation

/// Where an engine's host-side tensors live: the heap for onnxruntime, which
/// runs on them in place, or page-locked memory for TensorRT, whose copies to
/// and from the device then go straight out of the host buffer.
public struct HostAllocator: Sendable {
  public let allocate: @Sendable (_ bytes: Int) throws -> UnsafeMutableRawPointer
  public let free: @Sendable (UnsafeMutableRawPointer) -> Void

  public init(
    allocate: @escaping @Sendable (_ bytes: Int) throws -> UnsafeMutableRawPointer, free: @escaping @Sendable (UnsafeMutableRawPointer) -> Void
  ) {
    self.allocate = allocate
    self.free = free
  }

  /// 64-byte aligned, for the runtimes' vector loads.
  public static let heap = HostAllocator(
    allocate: { UnsafeMutableRawPointer.allocate(byteCount: $0, alignment: 64) },
    free: { $0.deallocate() })
}

/// What every engine keeps, whatever runs the model: its inputs and outputs by
/// name, a zeroed host buffer per tensor from its allocator, the state pairs
/// it loops, and the run's guard and timing. An engine subclasses it, supplies
/// `execute()`, and `bindLoop(_:)` to feed state back itself.
///
/// `hostInput(_:)` and `output(_:)` point into those buffers, so the host
/// stages straight into memory the runtime reads, and reads what it wrote.
///
/// A looped pair stays out of the host's way in one of two ways. onnxruntime
/// double-buffers the state_ input (`doubleBuffer`) and binds the two copies
/// alternately, reading one while next_state_ writes the other, with `parity`
/// saying which the next run reads. TensorRT copies next_state_ to state_ on
/// the device and frees the pair's host buffers (`releaseHostBuffers`).
open class EngineCore: Engine, @unchecked Sendable {
  public let inputs: [String: TensorSpec]
  public let outputs: [String: TensorSpec]
  /// The inputs the host writes each frame, and the outputs it reads: all
  /// but the looped pairs, which never reach the host.
  public private(set) var hostInputs: [String]
  public private(set) var hostOutputs: [String]
  /// The pairs the engine feeds back itself since `loopState`.
  public private(set) var looped: [(input: String, output: String)] = []
  /// Which of a double-buffered input's two buffers the next run reads.
  public private(set) var parity = 0
  /// Wall time of the last `run()`.
  public private(set) var lastRunNanoseconds: UInt64 = 0
  public private(set) var lastGpuUs: UInt32 = 0
  public private(set) var isClosed = false

  private let allocator: HostAllocator
  private var buffers: [String: UnsafeMutableRawPointer] = [:]
  private var spares: [String: UnsafeMutableRawPointer] = [:]
  private var loopedOutputs: Set<String> = []

  /// `sizes` is every tensor the engine runs over and its bytes: the inputs
  /// and outputs, and in a chain of sessions what one hands the next. Nil
  /// means the inputs and outputs alone.
  public init(
    inputs: [String: TensorSpec], outputs: [String: TensorSpec], sizes: [String: Int]? = nil, allocator: HostAllocator = .heap
  ) throws {
    self.inputs = inputs
    self.outputs = outputs
    self.allocator = allocator
    hostInputs = inputs.keys.sorted()
    hostOutputs = outputs.keys.sorted()
    let sizes = sizes ?? (Array(inputs.values) + Array(outputs.values)).reduce(into: [String: Int]()) { $0[$1.name] = $1.byteCount }
    do {
      for (name, size) in sizes {
        buffers[name] = try allocateZeroed(size)
      }
    } catch {
      freeBuffers()
      throw error
    }
  }

  public func hostInput(_ name: String) -> UnsafeMutableRawPointer? {
    guard inputs[name] != nil else { return nil }
    return buffer(name, parity: parity)
  }

  public func output(_ name: String) -> UnsafeRawPointer? {
    guard outputs[name] != nil, !loopedOutputs.contains(name) else { return nil }
    return buffer(name, parity: parity).map(UnsafeRawPointer.init)
  }

  /// Where `name` is for a run of `parity`: a double-buffered input's second
  /// buffer on odd runs, else its one buffer; nil once released.
  public func buffer(_ name: String, parity: Int) -> UnsafeMutableRawPointer? {
    if parity == 1, let spare = spares[name] { return spare }
    return buffers[name]
  }

  /// Keeps a stateful graph's queues in the engine (`bindLoop`).
  public func loopState(_ pairs: [(input: String, output: String)]) throws {
    try bindLoop(pairs)
    looped = pairs
    loopedOutputs = Set(pairs.map(\.output))
    let loopedInputs = Set(pairs.map(\.input))
    hostInputs = inputs.keys.filter { !loopedInputs.contains($0) }.sorted()
    hostOutputs = outputs.keys.filter { !loopedOutputs.contains($0) }.sorted()
    resetState()
  }

  /// Makes the engine feed `pairs` back from the next run on, or throws why
  /// it cannot. Called by `loopState` before `looped` changes, so the engine
  /// sees the new pairs here. The default cannot.
  open func bindLoop(_ pairs: [(input: String, output: String)]) throws {
    throw HostError.failed("\(type(of: self)) cannot loop state")
  }

  /// Empty state, as openpilot's warmup leaves it.
  public func resetState() {
    for pair in looped {
      guard let bytes = inputs[pair.input]?.byteCount else { continue }
      buffers[pair.input]?.initializeMemory(as: UInt8.self, repeating: 0, count: bytes)
      spares[pair.input]?.initializeMemory(as: UInt8.self, repeating: 0, count: bytes)
    }
    parity = 0
    stateDidReset()
  }

  /// For state the host buffers do not hold: a device copy to zero before
  /// the next run.
  open func stateDidReset() {}

  /// Gives each input a second zeroed host buffer, and from then on `run()`
  /// flips `parity` after every run.
  public func doubleBuffer(_ names: [String]) throws {
    for name in names where spares[name] == nil {
      guard let bytes = inputs[name]?.byteCount else { continue }
      spares[name] = try allocateZeroed(bytes)
    }
  }

  /// Frees the host buffers of tensors the host never touches again, a pair
  /// looped on the device. `hostInput` and `output` return nil for them.
  public func releaseHostBuffers(_ names: [String]) {
    for name in names {
      if let buffer = buffers.removeValue(forKey: name) { allocator.free(buffer) }
      if let spare = spares.removeValue(forKey: name) { allocator.free(spare) }
    }
  }

  /// One frame over the host buffers. Throws once closed; times the run.
  open func run() throws {
    guard !isClosed else { throw HostError.failed("engine is closed") }
    let started = DispatchTime.now().uptimeNanoseconds
    try execute()
    if !spares.isEmpty {
      parity ^= 1
    }
    lastRunNanoseconds = DispatchTime.now().uptimeNanoseconds - started
    lastGpuUs = UInt32(min(UInt64(UInt32.max), lastRunNanoseconds / 1000))
  }

  /// The engine's own work for one `run()`.
  open func execute() throws {
    preconditionFailure("\(type(of: self)) does not execute")
  }

  /// Two runs: the first pays for what the runtime does lazily, the second is
  /// the steady state. Returns what ran, for the log.
  open func warm() throws -> String {
    try run()
    try run()
    return "\(type(of: self)) warm in \(lastGpuUs) us"
  }

  open var notes: String { "" }
  open var accelerator: String { "" }
  open var coolsWhenIdle: Bool { false }

  /// Logs whatever timing of its own the engine has gathered but not said
  /// yet (TensorRT's --gpu-timing), for a benchmark's end. Nothing by default.
  open func flushTiming() {}

  /// Frees the host buffers. An engine with more to release overrides this,
  /// letting go of whatever uses the buffers before calling super, and of
  /// whatever the allocator needs after. Nothing here closes on deinit: an
  /// engine that wants that says so in its own.
  open func close() {
    guard !isClosed else { return }
    isClosed = true
    freeBuffers()
  }

  private func allocateZeroed(_ bytes: Int) throws -> UnsafeMutableRawPointer {
    let count = max(bytes, 1)
    let buffer = try allocator.allocate(count)
    buffer.initializeMemory(as: UInt8.self, repeating: 0, count: count)
    return buffer
  }

  private func freeBuffers() {
    for buffer in buffers.values { allocator.free(buffer) }
    for buffer in spares.values { allocator.free(buffer) }
    buffers = [:]
    spares = [:]
  }
}
