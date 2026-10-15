#if JL_TRT_FAKE
  import CTrt
  import Foundation
  import JetlinkTestSupport
  import Testing

  @testable import JetlinkServer
  @testable import JetlinkTRT

  /// TrtEngine on the fake shim, which keeps CUDA's and TensorRT's rules
  /// (capture, addresses, destroy order) and counts the work and the live
  /// objects.
  @Suite("TensorRT engine")
  struct TrtEngineTests {
    let tmp: TemporaryDirectory
    let trt: TensorRT

    init() throws {
      tmp = try TemporaryDirectory()
      trt = try fakeTensorRT()
    }

    func load(_ lines: String = defaultLines, built: String? = nil, timing: Bool = false, faultAfter: Int? = nil) throws -> TrtEngine {
      let plan = try tmp.file("model.plan", fakePlan(lines, built: built))
      return try TrtEngine(plan: plan, trt: trt, gpuTiming: timing, faultAfter: faultAfter)
    }

    @Test("A looped state stays on the device: y = x + state, and the state advances a frame at a time")
    func deviceLoop() throws {
      let engine = try load()
      try engine.loopState([(input: "state", output: "next_state")])
      #expect(engine.hostInputs == ["x"] && engine.hostOutputs == ["y"])
      #expect(engine.hostInput("state") == nil && engine.output("next_state") == nil)
      // the pair's pinned buffers went: x and y are left
      #expect(trt.stats.host_allocs == 2)
      engine.setInput("x", [1, 2, 3, 4, 5, 6, 7, 8])
      #expect(try engine.warm() == "cuda graph captured")
      #expect(engine.graphCaptured)
      engine.resetState()
      for k in 0..<4 {
        try engine.run()
        #expect(engine.floats("y") == (1...8).map { Float($0 + k) })
      }
      engine.resetState()
      try engine.run()
      #expect(engine.floats("y") == (1...8).map { Float($0) })
      engine.close()
    }

    @Test("A frame copies the host's input in and its output back, and nothing of the loop")
    func copiesOnlyWhatTheHostReads() throws {
      let engine = try load()
      try engine.loopState([(input: "state", output: "next_state")])
      _ = try engine.warm()
      // warm: a plain enqueue, then the capture and one replay
      #expect(trt.work == [2, 2, 2, 1, 2, 1])
      engine.resetState()
      try engine.run()
      #expect(trt.work == [3, 3, 3, 2, 3, 2])
      try engine.run()
      // no memset after the reset's first frame
      #expect(trt.work == [4, 4, 4, 2, 4, 3])
      engine.close()
    }

    @Test("Without a loop every input and output crosses, and the graph has no reply event")
    func noLoop() throws {
      let engine = try load()
      engine.setInput("x", Array(repeating: 1, count: 8))
      engine.setInput("state", Array(repeating: 2, count: 8))
      #expect(try engine.warm() == "cuda graph captured")
      #expect(trt.stats.events == 0)
      try engine.run()
      #expect(engine.floats("y") == Array(repeating: 3, count: 8))
      #expect(engine.floats("next_state") == Array(repeating: 3, count: 8))
      #expect(trt.work == [6, 6, 0, 0, 3, 2])
      engine.close()
    }

    @Test("A pair that differs in type cannot loop")
    func refusesAMismatchedPair() throws {
      let lines = defaultLines.replacingOccurrences(of: "output next_state float16", with: "output next_state float32")
      let engine = try load(lines)
      #expect(throws: TrtError.self) { try engine.loopState([(input: "state", output: "next_state")]) }
      engine.close()
    }

    @Test("loopState after the graph is captured is refused")
    func loopAfterCapture() throws {
      let engine = try load()
      _ = try engine.warm()
      #expect(throws: TrtError.self) { try engine.loopState([(input: "state", output: "next_state")]) }
      engine.close()
    }

    @Test("A capture that fails leaves the engine enqueueing each frame", arguments: ["capture_begin", "graph_instantiate", "context_enqueue"])
    func graphFallback(call: String) throws {
      let engine = try load()
      try engine.loopState([(input: "state", output: "next_state")])
      engine.setInput("x", Array(repeating: 1, count: 8))
      // the second enqueue is the one inside the capture
      trt.fail(call, nth: call == "context_enqueue" ? 2 : 1, code: Int32(JL_TRT_CUDA_ERROR), "CUDA_ERROR_STREAM_CAPTURE_UNSUPPORTED")
      #expect(try engine.warm() == "cuda graph unavailable, enqueueing per frame")
      #expect(!engine.graphCaptured)
      #expect(trt.stats.events == 0 && trt.stats.graphs == 0 && trt.stats.graph_execs == 0)
      engine.resetState()
      let before = trt.work
      for k in 0..<3 {
        try engine.run()
        #expect(engine.floats("y") == Array(repeating: Float(1 + k), count: 8))
      }
      #expect(trt.work[5] == before[5])
      #expect(trt.work[4] == before[4] + 3)
      #expect(!trt.isSticky)
      engine.close()
      #expect(trt.live.allSatisfy { $0 == 0 })
    }

    @Test("Closing frees everything, in an order the fake accepts, and only once")
    func closesClean() throws {
      let engine = try load(timing: true)
      try engine.loopState([(input: "state", output: "next_state")])
      _ = try engine.warm()
      try engine.run()
      #expect(trt.stats.events == 3)
      engine.close()
      engine.close()
      #expect(trt.live.allSatisfy { $0 == 0 }, "\(trt.live)")
      #expect(!trt.isSticky)
      #expect(throws: (any Error).self) { try engine.run() }
    }

    @Test("A plan from another TensorRT build, an empty one and a stranger are ArtifactInvalid")
    func invalidPlans() throws {
      #expect(throws: ArtifactInvalid.self) { try load(built: "10.16.2.10") }
      let empty = try tmp.file("empty.plan", "")
      #expect(throws: ArtifactInvalid.self) { try TrtEngine(plan: empty, trt: trt) }
      let stranger = try tmp.file("stranger.plan", "not a plan at all")
      #expect(throws: ArtifactInvalid.self) { try TrtEngine(plan: stranger, trt: trt) }
      let same = try load(built: "10.3.0.30")
      #expect(same.inputs.count == 2)
      same.close()
      #expect(trt.live.allSatisfy { $0 == 0 }, "\(trt.live)")
    }

    @Test("A deserialize refused for want of device memory or of a runtime is a plain error, not the plan's fault")
    func notThePlan() throws {
      let refusals: [(code: Int32, message: String)] = [
        (Int32(JL_TRT_ERROR), "[runtime.cpp::allocate::415] Error Code 1: Cuda Runtime (out of memory)"),
        (Int32(JL_TRT_ERROR), "Requested amount of GPU memory (1718548480 bytes) could not be ALLOCATED"),
        (Int32(JL_TRT_ERROR), "std::exception: OutOfMemory"),
        (Int32(JL_TRT_CUDA_ERROR), "createInferRuntime: TensorRT returned no runtime"),
      ]
      for refusal in refusals {
        trt.fail("engine_deserialize", code: refusal.code, refusal.message)
        #expect {
          _ = try load()
        } throws: {
          guard let error = $0 as? TrtError else { return false }
          return error.code == JL_TRT_CUDA_ERROR && !error.isFatal && error.description.contains(refusal.message)
        }
      }
      trt.fail("engine_deserialize", code: Int32(JL_TRT_ERROR), "Serialization assertion stdVersionRead == kSERIALIZATION_VERSION failed")
      #expect(throws: ArtifactInvalid.self) { try load() }
      #expect(!trt.isSticky)
      #expect(trt.live.allSatisfy { $0 == 0 }, "\(trt.live)")
    }

    @Test("A deserialize refused behind a sticky error is fatal, not the plan's fault")
    func stickyDeserialize() throws {
      trt.fail("engine_deserialize", code: Int32(JL_TRT_CUDA_STICKY), "CUDA_ERROR_ILLEGAL_ADDRESS: an illegal memory access was encountered")
      do {
        _ = try load()
        Issue.record("loaded")
      } catch let error as TrtError {
        #expect(error.isFatal)
      }
    }

    @Test("A dynamic shape is refused, and nothing is left behind")
    func dynamicShape() throws {
      #expect(throws: TrtError.self) { try load("input x float16 1 -1\noutput y float32 1 8\n") }
      #expect(trt.live.allSatisfy { $0 == 0 }, "\(trt.live)")
    }

    @Test("A sticky CUDA error in a frame is fatal; another is not")
    func stickyIsFatal() throws {
      let engine = try load()
      try engine.loopState([(input: "state", output: "next_state")])
      _ = try engine.warm()
      trt.fail("event_sync", code: Int32(JL_TRT_CUDA_ERROR), "cuEventSynchronize: CUDA_ERROR_LAUNCH_TIMEOUT")
      do {
        try engine.run()
        Issue.record("ran")
      } catch let error as FatalEngineError {
        #expect(!error.isFatal)
      }
      trt.fail("graph_launch", code: Int32(JL_TRT_CUDA_STICKY), "cuGraphLaunch: CUDA_ERROR_ILLEGAL_ADDRESS")
      do {
        try engine.run()
        Issue.record("ran")
      } catch let error as FatalEngineError {
        #expect(error.isFatal)
      }
      #expect(trt.isSticky)
      engine.close()
    }

    @Test("JETLINK_FAULT_CUDA_AFTER: the first N frames after warm-up run, every later one is fatal")
    func faultHook() throws {
      let engine = try load(faultAfter: 2)
      try engine.loopState([(input: "state", output: "next_state")])
      _ = try engine.warm()
      try engine.run()
      try engine.run()
      for _ in 0..<2 {
        do {
          try engine.run()
          Issue.record("ran")
        } catch let error as TrtError {
          #expect(error.isFatal && error.description.contains("JETLINK_FAULT_CUDA_AFTER=2"))
        }
      }
      engine.close()
    }

    @Test("CUDA-event timing is said in the engine's notes")
    func gpuTiming() throws {
      let engine = try load(timing: true)
      try engine.loopState([(input: "state", output: "next_state")])
      #expect(engine.notes == "cuda graph off, cuda-event timing on")
      _ = try engine.warm()
      #expect(engine.notes == "cuda graph on, cuda-event timing on")
      try engine.run()
      let plain = try load()
      #expect(plain.notes == "cuda graph off")
      plain.close()
      engine.close()
    }

    @Test("CUDA-event timing says a block shorter than 1,200 frames when flushed and at close, the last launch included")
    func gpuTimingFlush() throws {
      let engine = try load(timing: true)
      try engine.loopState([(input: "state", output: "next_state")])
      _ = try engine.warm()
      // more launches than the fake backend's half-second benchmark makes,
      // which logs a block of its own meanwhile
      let first = timingLines("37 frames")
      for _ in 0..<37 { try engine.run() }
      engine.flushTiming()
      #expect(timingLines("37 frames") == first + 1)
      // flushed: nothing left to say until the next launch
      #expect(engine.timing?.samples.isEmpty == true)
      let second = timingLines("23 frames")
      for _ in 0..<23 { try engine.run() }
      // a swap closes the engine the session ran
      engine.close()
      #expect(timingLines("23 frames") == second + 1)
      #expect(trt.live.allSatisfy { $0 == 0 }, "\(trt.live)")
    }

    @Test("The pinned host buffers are TensorRT's, and zeroed")
    func pinnedBuffers() throws {
      let engine = try load()
      #expect(trt.stats.host_allocs == 4 && trt.stats.device_allocs == 4)
      let x = engine.hostInput("x")!
      #expect((0..<16).allSatisfy { x.load(fromByteOffset: $0, as: UInt8.self) == 0 })
      engine.close()
    }
  }

  /// A stateful graph's queues go round on the device.
  @Suite("TensorRT state loop")
  struct TrtStateLoopTests {
    func serve(frames: Int, resetAt: Int) throws -> (outputs: [[Float]], work: [UInt64]) {
      let tmp = try TemporaryDirectory()
      let trt = try fakeTensorRT()
      let spec = try tinySpec()
      let plan = try tmp.file("tiny.plan", fakePlan(tinyPlanLines))
      let engine = try TrtEngine(plan: plan, trt: trt)
      defer { engine.close() }
      try checkShapes(engine, spec: spec)
      let staging = try Staging.forModel(spec, engine: engine)
      var warped = [UInt8](repeating: 0, count: spec.warpedBytes)
      var packed = [Float](repeating: 0, count: spec.packedCount)
      try staging.stage(warped: &warped, packed: &packed)
      _ = try engine.warm()
      staging.reset()
      var outputs: [[Float]] = []
      for i in warped.indices { warped[i] = UInt8(i % 7) }
      for i in packed.indices { packed[i] = Float(i * 3 % 5) }
      for frame in 0..<frames {
        if frame == resetAt {
          staging.reset()
        }
        try staging.stage(warped: &warped, packed: &packed)
        try engine.run()
        outputs.append(engine.floats("outputs"))
      }
      return (outputs, trt.work)
    }

    @Test("The device loop advances the queues a frame at a time, and starts over at a reset")
    func deviceLoop() throws {
      let device = try serve(frames: 6, resetAt: 4)
      // the three queues each advance by one a frame, and start over at the reset
      let first = device.outputs[0]
      for (frame, since) in [(1, 1), (3, 3), (4, 0), (5, 1)] {
        #expect(device.outputs[frame] == first.map { $0 + Float(3 * since) }, "frame \(frame)")
      }
      // the loop copied on the device
      #expect(device.work[2] > 0)
    }
  }
#endif
