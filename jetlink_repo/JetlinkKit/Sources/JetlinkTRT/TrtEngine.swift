import CTrt
import Foundation
import JetlinkKit
import JetlinkServer

#if canImport(Android)
  import Android
#endif

/// A TensorRT plan, loaded: the Swift form of the Python server's
/// trt/engine.py, the design the car was validated on.
///
/// Device buffers, page-locked host buffers and the stream are made at load,
/// so a frame never allocates: the 50 ms budget is broken by the tail, not
/// the mean. The host stages straight into the pinned buffers `hostInput`
/// hands out, and each frame is replayed as a CUDA graph captured at warm-up
/// (H2D, enqueueV3, D2H), which saves the per-launch CPU work and most of its
/// jitter.
///
/// A stateful graph's queues never leave the GPU: `loopState` drops them
/// from the per-frame copies and adds a device copy of each next_state_
/// output onto its state_ input, inside the graph. That copy is 12 MB, so the
/// graph also records when the outputs are home, and `run()` returns then:
/// 0.3 ms of the reply on an Orin.
///
/// No deinit-driven close: whoever swaps an engine out closes it.
public final class TrtEngine: EngineCore, @unchecked Sendable {
  public let trt: TensorRT
  private let parts: Parts
  private var graph: OpaquePointer?
  /// Recorded inside the graph once the outputs are home; set only when a
  /// looped state's copy follows it.
  private var replyEvent: OpaquePointer?
  private var zeroState = false
  private var copies: Copies?
  private(set) var timing: GPUTiming?
  private let faultAfter: Int?
  private var warmed = false
  private var served = 0
  private let log = ServerLog(category: "trt")

  public var graphCaptured: Bool { graph != nil }

  public override var notes: String {
    "cuda graph \(graph != nil ? "on" : "off")" + (timing != nil ? ", cuda-event timing on" : "")
  }

  /// `gpuTiming` puts a pair of timing events around every launch, off the
  /// reply's path, and logs their spread every 1,200 frames, and what is left
  /// at `flushTiming` and `close`. `faultAfter` is TrtBackend's H6 fault hook.
  public init(plan: URL, trt: TensorRT, gpuTiming: Bool = false, faultAfter: Int? = nil) throws {
    let parts = try Parts(plan: plan, trt: trt)
    self.trt = trt
    self.parts = parts
    self.faultAfter = faultAfter
    do {
      try super.init(inputs: parts.inputs, outputs: parts.outputs, allocator: trt.pinned)
    } catch {
      parts.release(trt)
      throw error
    }
    do {
      for (name, address) in parts.device {
        try trt.check { jl_trt_context_set_address(parts.context, name, address, $0, $1) }
      }
      if gpuTiming {
        timing = try GPUTiming(trt)
      }
    } catch {
      close()
      throw error
    }
  }

  /// Takes each pair onto the device; a pair that differs in size or type
  /// cannot loop. Only before the graph is captured, since it changes what
  /// the graph does.
  public override func bindLoop(_ pairs: [(input: String, output: String)]) throws {
    guard graph == nil else { throw TrtError("loopState after the cuda graph was captured") }
    for pair in pairs {
      guard let a = inputs[pair.input], let b = outputs[pair.output], a.byteCount == b.byteCount, a.type == b.type else {
        throw TrtError("state \(pair.input) cannot loop from \(pair.output): sizes or types differ")
      }
    }
    // never copied again: 24 MB of page-locked memory on a board that
    // shares it with the GPU
    releaseHostBuffers(pairs.flatMap { [$0.input, $0.output] })
    copies = nil
  }

  /// Zero the looped state before the next run: empty queues.
  public override func stateDidReset() {
    zeroState = true
  }

  public override func flushTiming() {
    timing?.flush(trt, log: log)
  }

  /// One plain run so lazy CUDA state is paid for, then the graph, then one
  /// replay, so the steady state is never the first thing a frame does.
  public override func warm() throws -> String {
    try run()
    let captured = try capture()
    if captured {
      try run()
    }
    warmed = true
    return captured ? "cuda graph captured" : "cuda graph unavailable, enqueueing per frame"
  }

  public override func execute() throws {
    if let faultAfter, warmed {
      // H6: a sticky error after the first faultAfter frames, for the D15 exit
      guard served < faultAfter else {
        throw TrtError(code: Int32(JL_TRT_CUDA_STICKY), "CUDA_ERROR_ILLEGAL_ADDRESS: injected by JETLINK_FAULT_CUDA_AFTER=\(faultAfter)")
      }
      served += 1
    }
    let copies = currentCopies()
    let h = trt.handle
    let stream = parts.stream
    if zeroState {
      // outside the graph, ahead of it on the same stream
      for zero in copies.zero {
        try trt.check { jl_trt_memset(h, zero.device, 0, zero.bytes, stream, $0, $1) }
      }
      zeroState = false
    }
    // The warm-up's runs pay for lazy CUDA state and the capture, so the
    // timing starts after them.
    let timing = warmed ? self.timing : nil
    try timing?.begin(trt, stream: stream, log: log)
    if let graph {
      try trt.check { jl_trt_graph_launch(h, graph, stream, $0, $1) }
    } else {
      try enqueue(copies, reply: nil)
    }
    try timing?.end(trt, stream: stream)
    if let replyEvent {
      try trt.check { jl_trt_event_sync(h, replyEvent, $0, $1) }
    } else {
      try trt.check { jl_trt_stream_sync(h, stream, $0, $1) }
    }
  }

  /// Sync the stream (the last frame's state copy may still be running into
  /// these buffers), let the events go, free the pinned buffers while the
  /// handle is alive, then the device's and the rest, a context before its
  /// engine.
  public override func close() {
    guard !isClosed else { return }
    let h = trt.handle
    _ = jl_trt_stream_sync(h, parts.stream, nil, 0)
    if let replyEvent {
      jl_trt_event_destroy(h, replyEvent)
      self.replyEvent = nil
    }
    timing?.flush(trt, log: log)
    timing?.destroy(trt)
    timing = nil
    super.close()
    if let graph {
      jl_trt_graph_exec_destroy(h, graph)
      self.graph = nil
    }
    parts.release(trt)
  }

  // MARK: the frame's work

  /// Every frame's copies, resolved once rather than looked up by name.
  private struct Copies {
    var h2d: [(device: jl_trt_dptr, host: UnsafeMutableRawPointer, bytes: Int)] = []
    var d2h: [(host: UnsafeMutableRawPointer, device: jl_trt_dptr, bytes: Int)] = []
    /// next_state_ onto state_: a copy rather than swapping addresses, which
    /// the captured graph has baked in.
    var d2d: [(state: jl_trt_dptr, next: jl_trt_dptr, bytes: Int)] = []
    var zero: [(device: jl_trt_dptr, bytes: Int)] = []
  }

  private func currentCopies() -> Copies {
    if let copies { return copies }
    var made = Copies()
    for name in hostInputs {
      made.h2d.append((parts.device[name]!, buffer(name, parity: 0)!, inputs[name]!.byteCount))
    }
    for name in hostOutputs {
      made.d2h.append((buffer(name, parity: 0)!, parts.device[name]!, outputs[name]!.byteCount))
    }
    for pair in looped {
      let bytes = inputs[pair.input]!.byteCount
      made.d2d.append((parts.device[pair.input]!, parts.device[pair.output]!, bytes))
      made.zero.append((parts.device[pair.input]!, bytes))
    }
    copies = made
    return made
  }

  /// H2D the host's inputs, run, D2H the host's outputs, mark the reply,
  /// then advance the looped state.
  private func enqueue(_ copies: Copies, reply: OpaquePointer?) throws(TrtError) {
    let h = trt.handle
    let stream = parts.stream
    for copy in copies.h2d {
      try trt.check { jl_trt_copy_h2d(h, copy.device, copy.host, copy.bytes, stream, $0, $1) }
    }
    try trt.check { jl_trt_context_enqueue(parts.context, stream, $0, $1) }
    for copy in copies.d2h {
      try trt.check { jl_trt_copy_d2h(h, copy.host, copy.device, copy.bytes, stream, $0, $1) }
    }
    if let reply {
      // the outputs are home: run() may return while the state copy runs.
      // The next frame queues behind it, and nothing it touches on the host
      // is read by the copy.
      try trt.check { jl_trt_event_record(h, reply, stream, UInt32(JL_TRT_RECORD_EXTERNAL), $0, $1) }
    }
    for copy in copies.d2d {
      try trt.check { jl_trt_copy_d2d(h, copy.state, copy.next, copy.bytes, stream, $0, $1) }
    }
  }

  /// The frame's work as a CUDA graph. Valid only because no buffer ever
  /// moves. False, with the reason logged, leaves the engine enqueueing per
  /// frame; a sticky error is thrown, since nothing will run.
  private func capture() throws -> Bool {
    guard graph == nil else { return true }
    let h = trt.handle
    let stream = parts.stream
    let copies = currentCopies()
    var event: OpaquePointer?
    do {
      if !looped.isEmpty {
        // blocks rather than spins: a default event's wait cost 37% of a core on the Orin
        let flags = UInt32(JL_TRT_EVENT_BLOCKING_SYNC | JL_TRT_EVENT_DISABLE_TIMING)
        try trt.check { jl_trt_event_create(h, flags, &event, $0, $1) }
      }
      try trt.check { jl_trt_capture_begin(h, stream, $0, $1) }
      do {
        try enqueue(copies, reply: event)
      } catch {
        // end it however the enqueue failed, so the stream stays usable
        var partial: OpaquePointer?
        _ = jl_trt_capture_end(h, stream, &partial, nil, 0)
        jl_trt_graph_destroy(h, partial)
        throw error
      }
      var captured: OpaquePointer?
      try trt.check { jl_trt_capture_end(h, stream, &captured, $0, $1) }
      defer { jl_trt_graph_destroy(h, captured) }
      var exec: OpaquePointer?
      try trt.check { jl_trt_graph_instantiate(h, captured, &exec, $0, $1) }
      graph = exec
      replyEvent = event
      return true
    } catch {
      jl_trt_event_destroy(h, event)
      if (error as? TrtError)?.isFatal == true { throw error }
      log.warning("cuda graph capture failed: \(error)")
      return false
    }
  }
}

/// What a plan loads into besides the host buffers: the engine, its context
/// and stream, and a device buffer per IO tensor.
private struct Parts {
  var engine: OpaquePointer?
  var context: OpaquePointer?
  var stream: OpaquePointer?
  var inputs: [String: TensorSpec] = [:]
  var outputs: [String: TensorSpec] = [:]
  var device: [String: jl_trt_dptr] = [:]

  init(plan: URL, trt: TensorRT) throws {
    let h = trt.handle
    let engine = try deserialize(plan, trt)
    self.engine = engine
    do {
      try trt.check { jl_trt_context_create(engine, &context, $0, $1) }
      try trt.check { jl_trt_stream_create(h, &stream, $0, $1) }
      for index in 0..<jl_trt_engine_io_count(engine) {
        let tensor = try io(engine, index, trt)
        var address: jl_trt_dptr = 0
        try trt.check { jl_trt_mem_alloc(h, max(tensor.spec.byteCount, 1), &address, $0, $1) }
        device[tensor.spec.name] = address
        if tensor.isInput {
          inputs[tensor.spec.name] = tensor.spec
        } else {
          outputs[tensor.spec.name] = tensor.spec
        }
      }
    } catch {
      release(trt)
      throw error
    }
  }

  func release(_ trt: TensorRT) {
    let h = trt.handle
    for address in device.values { jl_trt_mem_free(h, address) }
    jl_trt_context_destroy(context)
    jl_trt_engine_destroy(engine)
    jl_trt_stream_destroy(h, stream)
  }
}

/// The plan mapped read-only for the call, so a 1.7 GB plan is never read
/// into the heap. A plan TensorRT will not take is `ArtifactInvalid` (D21):
/// plans are valid for one TensorRT build only, and a JetPack update that
/// keeps the tag strands them, so the host deletes it and rebuilds once.
/// Only JL_TRT_ERROR is the plan's fault: device memory that ran out, no
/// runtime or a sticky error fails the job and leaves the plan for the next.
private func deserialize(_ plan: URL, _ trt: TensorRT) throws -> OpaquePointer {
  let name = plan.lastPathComponent
  let fd = open(plan.path, O_RDONLY | O_CLOEXEC)
  guard fd >= 0 else { throw TrtError("cannot open \(name): \(String(cString: strerror(errno)))") }
  defer { _ = close(fd) }
  var status = stat()
  guard fstat(fd, &status) == 0 else { throw TrtError("cannot stat \(name): \(String(cString: strerror(errno)))") }
  let size = Int(status.st_size)
  guard size > 0 else { throw ArtifactInvalid("\(name): the plan is empty") }
  // Optional on Glibc, not on Bionic, whose MAP_FAILED Swift cannot import.
  let mapped: UnsafeMutableRawPointer? = mmap(nil, size, PROT_READ, MAP_PRIVATE, fd, 0)
  guard let base = mapped, base != UnsafeMutableRawPointer(bitPattern: -1) else {
    throw TrtError("cannot map \(name): \(String(cString: strerror(errno)))")
  }
  defer { munmap(base, size) }
  var engine: OpaquePointer?
  var err = [CChar](repeating: 0, count: 512)
  let rc = jl_trt_engine_deserialize(trt.handle, base, size, &engine, &err, err.count)
  guard rc == JL_TRT_OK, let engine else {
    let error = TrtError(code: rc, string(err))
    throw rc == JL_TRT_ERROR ? ArtifactInvalid("\(name): \(error.description)") : error as any Error
  }
  return engine
}

/// One IO tensor. jetlink builds fixed-shape engines, and stages only the
/// types ElementType names.
private func io(_ engine: OpaquePointer, _ index: Int32, _ trt: TensorRT) throws -> (spec: TensorSpec, isInput: Bool) {
  var name: UnsafePointer<CChar>?
  var isInput: Int32 = 0
  var type: Int32 = 0
  var rank: Int32 = 0
  var dims = [Int64](repeating: 0, count: Int(JL_TRT_MAX_DIMS))
  try trt.check { jl_trt_engine_io(engine, index, &name, &isInput, &type, &dims, &rank, $0, $1) }
  let tensor = String(cString: name!)
  let shape = dims.prefix(Int(rank)).map { Int($0) }
  if shape.contains(where: { $0 < 0 }) {
    throw TrtError("tensor \(tensor) has a dynamic shape \(shape); jetlink builds fixed-shape engines")
  }
  guard let element = ElementType(rawValue: type) else {
    throw TrtError("tensor \(tensor) has a type jetlink does not stage")
  }
  return (TensorSpec(name: tensor, type: element, shape: shape), isInput != 0)
}

/// A timing event pair around each launch, outside the graph. A frame's
/// times are read at the start of the next, when its events are long done,
/// so the reply never waits on them. The last frame's are read at `flush`.
final class GPUTiming {
  static let block = 1200
  private var start: OpaquePointer?
  private var stop: OpaquePointer?
  private var pending = false
  private(set) var samples: [Double] = []

  init(_ trt: TensorRT) throws {
    let h = trt.handle
    let flags = UInt32(JL_TRT_EVENT_BLOCKING_SYNC)
    do {
      try trt.check { jl_trt_event_create(h, flags, &start, $0, $1) }
      try trt.check { jl_trt_event_create(h, flags, &stop, $0, $1) }
    } catch {
      destroy(trt)
      throw error
    }
    samples.reserveCapacity(Self.block)
  }

  func begin(_ trt: TensorRT, stream: OpaquePointer?, log: ServerLog) throws(TrtError) {
    try collect(trt)
    if samples.count == Self.block {
      report(log)
    }
    try trt.check { jl_trt_event_record(trt.handle, start, stream, 0, $0, $1) }
  }

  func end(_ trt: TensorRT, stream: OpaquePointer?) throws(TrtError) {
    try trt.check { jl_trt_event_record(trt.handle, stop, stream, 0, $0, $1) }
    pending = true
  }

  /// The last launch's time, into the samples.
  private func collect(_ trt: TensorRT) throws(TrtError) {
    guard pending else { return }
    let h = trt.handle
    var ms: Float = 0
    try trt.check { jl_trt_event_sync(h, stop, $0, $1) }
    try trt.check { jl_trt_event_elapsed(h, start, stop, &ms, $0, $1) }
    pending = false
    samples.append(Double(ms))
  }

  /// Logs what the block holds so far, the last launch included, and starts
  /// a new one: at a swap, a close or a benchmark's end, which would
  /// otherwise drop a session shorter than a block without a line.
  func flush(_ trt: TensorRT, log: ServerLog) {
    if (try? collect(trt)) == nil {
      // after a sticky error: that launch has no time to read
      pending = false
    }
    report(log)
  }

  private func report(_ log: ServerLog) {
    guard !samples.isEmpty else { return }
    let stats = BenchmarkStats.of(samples)
    log.info("gpu (cuda events), \(samples.count) frames: mean \(stats.mean) p50 \(stats.p50) p99 \(stats.p99) max \(stats.max) ms")
    samples.removeAll(keepingCapacity: true)
  }

  func destroy(_ trt: TensorRT) {
    jl_trt_event_destroy(trt.handle, start)
    jl_trt_event_destroy(trt.handle, stop)
    start = nil
    stop = nil
  }
}
