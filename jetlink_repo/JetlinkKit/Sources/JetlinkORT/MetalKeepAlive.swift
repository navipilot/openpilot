#if canImport(Metal)
  import Foundation
  import Metal

  /// Keeps the GPU clocked up between the frames of a 20 Hz stream, as
  /// `backends/ort/metal.py` does for the Mac's worker.
  ///
  /// On an M2 Pro, pauses in a 20 Hz stream let the GPU clocks fall and frame
  /// times pass 50 ms, though back to back inference took 34 ms; on an M1 Pro
  /// the split measured 46.4 ms mean without this and 35.8 with it. A small,
  /// independent kernel, one command at a time, keeps the clocks up for a second
  /// after each frame. It touches no model buffers, and an idle engine submits
  /// nothing.
  final class MetalKeepAlive: @unchecked Sendable {
    static let idleSeconds = 1.0
    /// About 0.3 ms at full clock on an M2 Pro; a finite loop also bounds the work.
    static let rounds: UInt32 = 10_000
    static let shader = """
      #include <metal_stdlib>
      using namespace metal;
      kernel void keep_active(device uint *out [[buffer(0)]],
                              constant uint &rounds [[buffer(1)]],
                              uint tid [[thread_position_in_grid]]) {
        uint x = out[tid];
        for (uint i = 0; i < rounds; i++) x = x * 1664525u + 1013904223u;
        out[tid] = x;
      }
      """

    private let queue: MTLCommandQueue
    private let pipeline: MTLComputePipelineState
    private let buffer: MTLBuffer
    private let condition = NSCondition()
    private var deadline = 0.0
    private var closed = false

    /// Nil where Metal is unavailable; inference goes on without it.
    static func make() -> MetalKeepAlive? {
      guard let device = MTLCreateSystemDefaultDevice(),
        let queue = device.makeCommandQueue(),
        let library = try? device.makeLibrary(source: shader, options: nil),
        let function = library.makeFunction(name: "keep_active"),
        let pipeline = try? device.makeComputePipelineState(function: function),
        let buffer = device.makeBuffer(length: 128, options: .storageModeShared)
      else { return nil }
      return MetalKeepAlive(queue: queue, pipeline: pipeline, buffer: buffer)
    }

    private init(queue: MTLCommandQueue, pipeline: MTLComputePipelineState, buffer: MTLBuffer) {
      self.queue = queue
      self.pipeline = pipeline
      self.buffer = buffer
      let thread = Thread { [weak self] in self?.loop() }
      thread.name = "jetlink-metal-keepalive"
      thread.qualityOfService = .userInitiated
      thread.start()
    }

    func pulse() {
      condition.lock()
      if !closed {
        deadline = ProcessInfo.processInfo.systemUptime + MetalKeepAlive.idleSeconds
        condition.signal()
      }
      condition.unlock()
    }

    func pause() {
      condition.lock()
      deadline = 0
      condition.signal()
      condition.unlock()
    }

    func close() {
      condition.lock()
      closed = true
      condition.signal()
      condition.unlock()
    }

    private func loop() {
      var rounds = MetalKeepAlive.rounds
      while true {
        condition.lock()
        while !closed && ProcessInfo.processInfo.systemUptime >= deadline {
          condition.wait()
        }
        let stop = closed
        condition.unlock()
        if stop { return }
        autoreleasepool {
          guard let command = queue.makeCommandBuffer(), let encoder = command.makeComputeCommandEncoder() else {
            pause()
            return
          }
          encoder.setComputePipelineState(pipeline)
          encoder.setBuffer(buffer, offset: 0, index: 0)
          encoder.setBytes(&rounds, length: MemoryLayout<UInt32>.size, index: 1)
          encoder.dispatchThreadgroups(MTLSize(width: 1, height: 1, depth: 1), threadsPerThreadgroup: MTLSize(width: 32, height: 1, depth: 1))
          encoder.endEncoding()
          command.commit()
          // Never a backlog: one command at a time.
          command.waitUntilCompleted()
          if command.status != .completed {
            // A backgrounded iOS app may not use the GPU. Wait for the next
            // frame rather than spin on refusals.
            pause()
          }
        }
      }
    }
  }
#else
  /// No Metal here. On Android QNN's own performance mode does this job.
  final class MetalKeepAlive {
    static func make() -> MetalKeepAlive? { nil }
    func pulse() {}
    func pause() {}
    func close() {}
  }
#endif
