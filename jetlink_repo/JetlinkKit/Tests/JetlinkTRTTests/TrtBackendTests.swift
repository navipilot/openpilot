import CTrt
import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkServer
@testable import JetlinkTRT

@Suite("TensorRT backend: names and sizes")
struct TrtNamingTests {
  @Test("The workspace is 40% of MemAvailable between 256 MB and 4 GB, and 4 GB when unknown")
  func workspace() {
    #expect(TrtBackend.workspaceBytes(available: 0) == 4 << 30)
    #expect(TrtBackend.workspaceBytes(available: 100 << 20) == 256 << 20)
    #expect(TrtBackend.workspaceBytes(available: 1 << 30) == 429_496_729)
    #expect(TrtBackend.workspaceBytes(available: 16 << 30) == 4 << 30)
  }

  @Test("Build progress is the root phase's step over its steps, as _Monitor made it")
  func monitor() {
    let seen = Recorded<Event>()
    let monitor = BuildMonitor(seen.report)
    let start = Int32(JL_TRT_PHASE_START)
    let step = Int32(JL_TRT_PHASE_STEP)
    let finish = Int32(JL_TRT_PHASE_FINISH)
    monitor.event(step, phase: "orphan", parent: nil, value: 1)
    monitor.event(start, phase: "Building", parent: nil, value: 4)
    monitor.event(start, phase: "Tactics", parent: "Building", value: 10)
    monitor.event(step, phase: "Tactics", parent: nil, value: 9)
    monitor.event(finish, phase: "Tactics", parent: nil, value: 0)
    monitor.event(step, phase: "Building", parent: nil, value: 2)
    monitor.event(step, phase: "Building", parent: nil, value: 7)
    monitor.event(finish, phase: "Building", parent: nil, value: 0)
    monitor.event(step, phase: "Building", parent: nil, value: 3)
    monitor.event(start, phase: "Empty", parent: nil, value: 0)
    #expect(
      seen.all == [
        Event("build", 0, "Building"), Event("build", 0, "Building"), Event("build", 0, "Building"),
        Event("build", 0.5, "Building"), Event("build", 1, "Building"), Event("build", 0, "Empty"),
      ])
  }
}

struct Event: Equatable, CustomStringConvertible {
  let stage: String
  let frac: Double
  let msg: String

  init(_ stage: String, _ frac: Double, _ msg: String) {
    self.stage = stage
    self.frac = frac
    self.msg = msg
  }

  var description: String { "\(stage) \(frac) \(msg)" }
}

extension Recorded<Event> {
  var report: ProgressFn {
    { stage, frac, msg in self.append(Event(stage, frac, msg)) }
  }
}

#if JL_TRT_FAKE
  @Suite("TensorRT backend on the fake")
  struct TrtBackendTests {
    let tmp: TemporaryDirectory
    let engines: URL

    init() throws {
      tmp = try TemporaryDirectory()
      engines = tmp.url.appending(path: "engines", directoryHint: .isDirectory)
      try FileManager.default.createDirectory(at: engines, withIntermediateDirectories: true)
    }

    func backend(device: String = "Orin", free: Int = 1 << 30, _ configure: (inout jl_trt_fake_config) -> Void = { _ in }) throws -> TrtBackend {
      let trt = try fakeTensorRT(device: device, configure)
      trt.setBuild(tinyPlanLines)
      return TrtBackend(trt: trt, available: { free })
    }

    func plan(_ backend: TrtBackend) -> URL {
      engines.appending(path: "b0669bc51ec95a6a.\(backend.tag()).plan")
    }

    func text(_ url: URL) throws -> String {
      try String(contentsOf: url, encoding: .utf8)
    }

    @Test("The tag, the plan's name and the timing cache's, for JetPack 6, JetPack 7 and a PC")
    func tags() throws {
      let jp6 = try backend()
      #expect(jp6.tag() == "trt10.3.0.Orin-sm87")
      // Python's tensorrt.__version__ left the build out on JetPack 6 only
      #expect(try backend { ($0.patch, $0.build) = (1, 2) }.runtimeVersion == "10.3.1")
      #expect(jp6.runtimeVersion == "10.3.0" && jp6.deviceTag() == "Orin-sm87")
      #expect(jp6.describe() == ["backend": "trt", "runtime_version": "10.3.0", "device": "Orin-sm87", "trt_version": "10.3.0.30"])

      let jp7 = try backend {
        ($0.major, $0.minor, $0.patch, $0.build) = (10, 16, 2, 10)
      }
      // the bench Jetson's plans, as the Python server named them
      let cache = try ServerCache(root: tmp.url, backend: jp7)
      let entry = try cache.entry("b0669bc51ec95a6a8bccf3ddf614098148ee1447025d99f4dd5e5d501ca41868")
      #expect(entry.path.lastPathComponent == "b0669bc51ec95a6a.trt10.16.2.10.Orin-sm87.plan")
      #expect(entry.metaPath.lastPathComponent == "b0669bc51ec95a6a.trt10.16.2.10.Orin-sm87.json")
      #expect(jp7.timingCache(beside: entry.path).lastPathComponent == "timing.trt10.16.2.10.Orin-sm87.cache")

      let pc = try backend(device: "NVIDIA GeForce RTX 4090") {
        ($0.major, $0.minor, $0.patch, $0.build, $0.cc_major, $0.cc_minor, $0.strongly_typed) = (11, 3, 0, 99, 8, 9, 1)
      }
      #expect(pc.tag() == "trt11.3.0.99.NVIDIA_GeForce_RTX_4090-sm89")
      #expect(pc.name == "trt" && pc.suffix == ".plan")
    }

    @Test("A build reports Python's stages and messages, and writes Python's sidecar keys")
    func build() throws {
      let backend = try backend()
      let seen = Recorded<Event>()
      let artifact = plan(backend)
      try backend.build(model: TinyModel.stateful, artifact: artifact, report: seen.report, metaExtra: ["spec": ["sha256": "x"]])
      var events = seen.all
      let done = events.removeLast()
      #expect(done.stage == "build" && done.frac == 1 && done.msg.hasPrefix("done in ") && done.msg.hasSuffix("s"))
      let third = 1.0 / 3
      let twoThirds = 2.0 / 3
      let phases = [0, 0, 0, 0, 0, 0, 0, 0, third, third, third, third, twoThirds].map { Event("build", $0, "fake build") }
      #expect(
        events == [
          Event("patch", 0, "retyping uint8 image inputs to fp16"), Event("patch", 1, "patched"), Event("parse", 0, "parsing onnx"),
          Event("parse", 1, "3 layers"), Event("build", 0, "building engine"),
        ] + phases)

      let meta = Artifact.sidecar(artifact)
      // a float, as Python writes round(x, 1): "0.0" rather than "0"
      #expect(try text(Artifact.sidecarURL(artifact)).range(of: #""build_seconds" ?: ?\d+\.\d"#, options: .regularExpression) != nil)
      #expect(
        Set(meta.keys) == [
          "backend", "trt_version", "device", "fp16", "strongly_typed", "optimization_level", "build_seconds", "onnx", "built_at", "spec",
        ])
      #expect(meta["backend"] as? String == "trt" && meta["trt_version"] as? String == "10.3.0" && meta["device"] as? String == "Orin-sm87")
      #expect(meta["fp16"] as? Bool == true && meta["strongly_typed"] as? Bool == false && meta["optimization_level"] as? Int == 3)
      #expect(meta["onnx"] as? String == "tiny_stateful.onnx" && meta["build_seconds"] is Double)
      #expect((meta["spec"] as? [String: String]) == ["sha256": "x"])
      let builtAt = try #require(meta["built_at"] as? String)
      #expect(builtAt.wholeMatch(of: /\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z/) != nil)

      // the fake records what the build was given
      #expect(try text(artifact).contains("settings fp16=1 optimization_level=3 workspace=429496729 timing_cache=cold"))
      // staged beside it and moved: nothing else is left in engines/
      let names = try FileManager.default.contentsOfDirectory(atPath: engines.path).sorted()
      #expect(names == ["b0669bc51ec95a6a.trt10.3.0.Orin-sm87.json", artifact.lastPathComponent, "timing.trt10.3.0.Orin-sm87.cache"].sorted())
      #expect(try text(backend.timingCache(beside: artifact)) == "jl_trt_fake_timing 10.3.0.30 1\n")
      #expect(backend.trt.stats.builds == 0)

      // the next build starts warm and adds to the cache
      try backend.build(model: TinyModel.stateful, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
      #expect(try text(artifact).contains("timing_cache=warm"))
      #expect(try text(backend.timingCache(beside: artifact)) == "jl_trt_fake_timing 10.3.0.30 2\n")
    }

    @Test("TensorRT 11 builds strongly typed, without the FP16 flag", arguments: [0, 16 << 30])
    func stronglyTyped(free: Int) throws {
      let backend = try backend(free: free) {
        ($0.major, $0.minor, $0.patch, $0.build, $0.strongly_typed) = (11, 3, 0, 99, 1)
      }
      let artifact = plan(backend)
      try backend.build(model: TinyModel.stateful, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
      #expect(try text(artifact).contains("settings fp16=0 optimization_level=3 workspace=4294967296 timing_cache=cold"))
      let meta = Artifact.sidecar(artifact)
      #expect(meta["strongly_typed"] as? Bool == true && meta["fp16"] as? Bool == true && meta["trt_version"] as? String == "11.3.0.99")
    }

    @Test("Another build's or a truncated timing cache is replaced, and the build refills it", arguments: ["jl_trt_fake_timing 10.2.0.1 5\n", "trunc"])
    func unusableTimingCache(cache: String) throws {
      let backend = try backend()
      let artifact = plan(backend)
      let cacheURL = backend.timingCache(beside: artifact)
      try Data(cache.utf8).write(to: cacheURL)
      try backend.build(model: TinyModel.stateful, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
      #expect(try text(artifact).contains("timing_cache=cold"))
      #expect(try text(cacheURL) == "jl_trt_fake_timing 10.3.0.30 1\n")
      #expect(!FileManager.default.fileExists(atPath: cacheURL.path + ".tmp"))
    }

    @Test("No plan from TensorRT reads as Python's message, and leaves nothing behind")
    func noEngine() throws {
      let backend = try backend()
      backend.trt.fail("build_write_plan", code: Int32(JL_TRT_ERROR), "buildSerializedNetwork: TensorRT returned no engine")
      let artifact = plan(backend)
      #expect(throws: TrtError("TensorRT returned no engine; see the build log")) {
        try backend.build(model: TinyModel.stateful, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
      }
      #expect(try FileManager.default.contentsOfDirectory(atPath: engines.path).isEmpty)
      #expect(backend.trt.live.allSatisfy { $0 == 0 })
    }

    @Test("A parser's refusal says so, with its errors")
    func parseFailure() throws {
      let backend = try backend()
      backend.trt.fail("build_parse", code: Int32(JL_TRT_ERROR), "(parseFromFile): INVALID_GRAPH: no")
      #expect(throws: TrtError("onnx parse failed:\nbuild_parse: (parseFromFile): INVALID_GRAPH: no")) {
        try backend.build(model: TinyModel.stateful, artifact: plan(backend), report: { _, _, _ in }, metaExtra: [:])
      }
    }

    @Test("A sticky error in a build is fatal")
    func stickyBuild() throws {
      let backend = try backend()
      backend.trt.fail("build_write_plan", code: Int32(JL_TRT_CUDA_STICKY), "CUDA_ERROR_ILLEGAL_ADDRESS")
      do {
        try backend.build(model: TinyModel.stateful, artifact: plan(backend), report: { _, _, _ in }, metaExtra: [:])
        Issue.record("built")
      } catch let error as TrtError {
        #expect(error.isFatal)
      }
    }

    // MARK: through the host

    func request(_ cache: ServerCache) throws -> Request {
      let model = try Data(contentsOf: TinyModel.stateful)
      let spec = try tinySpec()
      let request = try Request(sha256: spec.sha256, nbytes: Int64(model.count), frameSkip: 4)
      try model.write(to: cache.modelPath(request))
      return request
    }

    @Test("A stateful model builds, loads, warms into a graph and serves frames; the new plan outlives the prune")
    func serves() throws {
      let backend = try backend()
      let cache = try ServerCache(root: tmp.url, backend: backend)
      let request = try request(cache)
      // plans that look newer than anything built now, as on a Jetson that
      // booted without NTP
      for i in 0..<8 {
        let other = engines.appending(path: "\(String(format: "%016x", i)).\(backend.tag()).plan")
        try Data("old".utf8).write(to: other)
        try FileManager.default.setAttributes([.modificationDate: Date().addingTimeInterval(86400 + Double(i))], ofItemAtPath: other.path)
      }
      let host = EngineHost(cache: cache)
      defer { host.close() }
      _ = host.request(request, session: nil)
      #expect(host.settles(timeout: 60))
      #expect(host.snapshot().state == .ready, "\(host.snapshot().detail)")
      let plans = try FileManager.default.contentsOfDirectory(atPath: engines.path).filter { $0.hasSuffix(".plan") }
      #expect(plans.count == cache.keep)
      #expect(plans.contains(cache.entry(request).path.lastPathComponent))
      #expect(!plans.contains { $0.hasPrefix("0000000000000000") })

      let report = try host.benchmark(seconds: 0.5, run: BenchmarkRun())
      #expect(report.frames > 0)
      #expect(report.build.contains("cuda graph on"))
      host.close()
      #expect(backend.trt.live.allSatisfy { $0 == 0 }, "\(backend.trt.live)")
    }

    @Test("With --gpu-timing a benchmark's launches are said at its end, however few")
    func benchmarkTiming() throws {
      let trt = try fakeTensorRT()
      trt.setBuild(tinyPlanLines)
      let backend = TrtBackend(trt: trt, gpuTiming: true, available: { 1 << 30 })
      let cache = try ServerCache(root: tmp.url, backend: backend)
      let request = try request(cache)
      let host = EngineHost(cache: cache)
      defer { host.close() }
      _ = host.request(request, session: nil)
      #expect(host.settles(timeout: 60) && host.snapshot().state == .ready)
      let report = try host.benchmark(seconds: 0.5, run: BenchmarkRun())
      // the warm-up frames are launches too
      let launches = report.frames + EngineHost.benchmarkWarmup
      #expect(launches < GPUTiming.block)
      #expect(timingLines("\(launches) frames") == 1)
    }

    @Test("A plan TensorRT refuses is rebuilt once; one that met a full device or no runtime stays, and only the job fails")
    func refusedPlans() throws {
      let backend = try backend()
      let cache = try ServerCache(root: tmp.url, backend: backend)
      let request = try request(cache)
      let plan = cache.entry(request).path
      let built = EngineHost(cache: cache)
      _ = built.request(request, session: nil)
      #expect(built.settles(timeout: 60) && built.snapshot().state == .ready)
      built.close()

      /// One request on a fresh host, whose first deserialize is refused
      /// with `code`; the plan is dated long ago first, so a rebuild shows.
      func attempt(_ code: Int32, _ message: String) throws -> (event: String, detail: String, rebuilt: Bool) {
        let old = Date(timeIntervalSince1970: 1_000_000_000)
        try FileManager.default.setAttributes([.modificationDate: old], ofItemAtPath: plan.path)
        backend.trt.fail("engine_deserialize", code: code, message)
        let host = EngineHost(cache: cache)
        defer { host.close() }
        _ = host.request(request, session: nil)
        #expect(host.settles(timeout: 60))
        let snapshot = host.snapshot()
        let modified = try FileManager.default.attributesOfItem(atPath: plan.path)[.modificationDate] as? Date
        return (snapshot.state.rawValue, snapshot.detail, modified != old)
      }

      let notThePlan: [(code: Int32, message: String)] = [
        (Int32(JL_TRT_ERROR), "Requested amount of GPU memory (1718548480 bytes) could not be allocated"),
        (Int32(JL_TRT_CUDA_ERROR), "createInferRuntime: TensorRT returned no runtime"),
      ]
      for refusal in notThePlan {
        let kept = try attempt(refusal.code, refusal.message)
        #expect(kept.event == "failed" && kept.detail.contains(refusal.message), "\(kept)")
        #expect(!kept.rebuilt)
      }
      let replaced = try attempt(Int32(JL_TRT_ERROR), "The engine plan file is not compatible with this version of TensorRT")
      #expect(replaced.event == "ready" && replaced.rebuilt, "\(replaced)")
      #expect(!backend.trt.isSticky)
      #expect(backend.trt.live.allSatisfy { $0 == 0 }, "\(backend.trt.live)")
    }
  }
#endif
