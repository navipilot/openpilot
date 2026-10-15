import Foundation
import JetlinkKit
import JetlinkTestSupport
import Testing

@testable import JetlinkLiteRT
@testable import JetlinkServer

/// LiteRT's libraries, where $JETLINK_LITERT_DIR names them: the
/// ai-edge-litert 2.2.0 wheel's package directory on a Mac. Without them the
/// LiteRT suite skips, so a machine without LiteRT stays green.
enum LiteRtLibraries {
  static var available: Bool {
    !(ProcessInfo.processInfo.environment[LiteRtRuntime.directoryVariable] ?? "").isEmpty
  }

  /// The profiles a test can run here: on Android the GPU only with OpenCL,
  /// which LiteRT's GPU is pinned to there. Without it the app's server gets
  /// a clean "no OpenCL" (seen on the emulator), but a test runner, a shell
  /// process, dies inside LiteRT's OpenCL loader on a null string.
  static var profiles: [LiteRtProfile] {
    #if os(Android)
      LiteRtRuntime.hasOpenCL ? [.cpu, .gpu] : [.cpu]
    #else
      [.cpu, .gpu]
    #endif
  }

  /// The bench's directory, $JETLINK_LITERT_BENCH (LiteRtBenchTests).
  static var bench: URL? {
    ProcessInfo.processInfo.environment["JETLINK_LITERT_BENCH"].flatMap { $0.isEmpty ? nil : URL(fileURLWithPath: $0, isDirectory: true) }
  }
}

/// A graph LiteRT's GPU cannot run: y = x + x on a rank-5 float tensor,
/// which the conversion keeps rank 5 (its rewrites take only the driving
/// models' rank-5 patterns down to rank 4). A run-time GATHER would not do:
/// Metal runs it. The protobuf is written here field by field, as
/// onnx.helper would write it.
enum RankFiveAdd {
  static func write(to url: URL) throws {
    let node = string(1, "x") + string(1, "x") + string(2, "y") + string(3, "add") + string(4, "Add")
    let graph = message(1, node) + string(2, "add") + value(11, "x", 1, [1, 2, 2, 2, 2]) + value(12, "y", 1, [1, 2, 2, 2, 2])
    let model = varint(1, 8) + message(7, graph) + message(8, string(1, "") + varint(2, 17))
    try Data(model).write(to: url)
  }

  /// A ValueInfoProto: a tensor of ONNX element type `type` and a static shape.
  private static func value(_ field: Int, _ name: String, _ type: Int, _ dims: [Int]) -> [UInt8] {
    let shape = dims.flatMap { message(1, varint(1, $0)) }
    return message(field, string(1, name) + message(2, message(1, varint(1, type) + message(2, shape))))
  }

  private static func varint(_ field: Int, _ value: Int) -> [UInt8] {
    leb(field << 3) + leb(value)
  }

  private static func message(_ field: Int, _ bytes: [UInt8]) -> [UInt8] {
    leb(field << 3 | 2) + leb(bytes.count) + bytes
  }

  private static func string(_ field: Int, _ text: String) -> [UInt8] {
    message(field, Array(text.utf8))
  }

  private static func leb(_ value: Int) -> [UInt8] {
    var bytes: [UInt8] = []
    var v = UInt64(value)
    repeat {
      bytes.append(UInt8(v & 0x7f) | (v > 0x7f ? 0x80 : 0))
      v >>= 7
    } while v > 0
    return bytes
  }
}

/// LiteRT behind the whole server: the conversion on the device, the
/// compile, the load, the state loop on the GPU's memory or the CPU's, and
/// the replies against Python's outputs. On a Mac the GPU is Metal's.
@Suite(
  "LiteRT backend", .serialized,
  .enabled(if: LiteRtLibraries.available, "LiteRT's libraries are not in $\(LiteRtRuntime.directoryVariable)"))
struct LiteRtBackendTests {
  /// Opened, as the app and the command line open theirs.
  func backend(_ profile: LiteRtProfile) throws -> LiteRtBackend {
    let backend = LiteRtBackend(profile: profile, preparer: ONNXPreparer())
    try backend.open()
    return backend
  }

  @Test("A comma is served Python's outputs from LiteRT", arguments: LiteRtLibraries.profiles, ["tiny_queued", "tiny_stateful"])
  func servesGoldenFrames(_ profile: LiteRtProfile, _ name: String) throws {
    let golden = try Golden(name)
    try serve(backend: try backend(profile)) { server, client in
      // The weights are fp16 and the GPU computes in fp16: held to what
      // verify_parity asks of a phone, not to Python's bits.
      let (hello, count) = try client.replay(golden, exact: false)
      #expect(hello["backend"] as? String == "litert")
      #expect(hello["runtime_version"] as? String == LiteRtRuntime.version)
      #expect((hello["device"] as? String)?.hasPrefix("\(profile.rawValue)-") == true)
      #expect(count == 8)
      #expect(eventually { server.framesServed == count })
      let engine = server.host.lock.withLock { server.host.loaded?.engine } as? LiteRtEngine
      #expect(engine?.fullyAccelerated == true || profile == .cpu)
      // The GPU keeps the queues in its own memory; the CPU reads the host's.
      #expect(engine?.stateOnDevice == (profile == .gpu && name == "tiny_stateful"))

      // The artifact: the model, the GPU's cache directory, a sidecar.
      let engines = server.cache.layout.engines
      let artifacts = try FileManager.default.contentsOfDirectory(atPath: engines.path).filter { $0.hasSuffix(".litertcache") }
      #expect(artifacts.count == 1)
      let artifact = engines.appending(path: artifacts[0])
      #expect(FileManager.default.fileExists(atPath: artifact.appending(path: "model.tflite").path))
      #expect(FileManager.default.fileExists(atPath: artifact.appending(path: "gpu-cache").path))
      let meta = try LiteRtArtifact.open(artifact)
      #expect(meta["backend"] as? String == "litert")
      #expect(meta["litert"] as? String == LiteRtRuntime.version)
      #expect(meta["accelerator"] as? String == profile.label(chip: HostChip.name()))
      #expect(meta["prepare"] as? Int == LiteRtBackend.conversionVersion)
      #expect(((meta["artifact_bytes"] as? NSNumber)?.int64Value ?? 0) > 0)
    }
  }

  @Test("Reset empties the looped state, in the GPU's memory as in the host's", arguments: LiteRtLibraries.profiles)
  func resetsState(_ profile: LiteRtProfile) throws {
    let temp = try TemporaryDirectory()
    let backend = try backend(profile)
    let artifact = temp.url.appending(path: "tiny.litertcache")
    try backend.build(model: TinyModel.stateful, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
    let engine = try #require(try backend.load(artifact: artifact, report: { _, _, _ in }) as? LiteRtEngine)
    defer { engine.close() }
    let pairs = ["img_q", "desire_q", "feat_q"].map { (input: "state_\($0)", output: "next_state_\($0)") }
    try engine.loopState(pairs)
    #expect(engine.stateOnDevice == (profile == .gpu))
    #expect(pairs.allSatisfy { engine.hostInput($0.input) == nil || !engine.stateOnDevice })

    // The same frame three times: the queues fill, and the output moves with them.
    for name in engine.hostInputs {
      let spec = engine.inputs[name]!
      let input = engine.hostInput(name)!
      if spec.type == .uint8 {
        input.initializeMemory(as: UInt8.self, repeating: 200, count: spec.count)
      } else {
        input.initializeMemory(as: Float.self, repeating: 0.5, count: spec.count)
      }
    }
    let count = engine.outputs["outputs"]!.count
    func run() throws -> [Float] {
      try engine.run()
      return Array(UnsafeBufferPointer(start: engine.output("outputs")!.assumingMemoryBound(to: Float.self), count: count))
    }
    let first = try run()
    _ = try run()
    #expect(try run() != first)
    let queue = engine.inputs["state_img_q"]!
    var bytes = [UInt8](repeating: 0, count: queue.byteCount)
    try engine.state("state_img_q", into: &bytes)
    #expect(bytes.contains(200))

    engine.resetState()
    try engine.state("state_img_q", into: &bytes)
    #expect(!bytes.contains { $0 != 0 })
    let again = try run()
    #expect(Golden.correlation(Data(bytes: again, count: count * 4), Data(bytes: first, count: count * 4)) > 0.99999)
  }

  @Test(
    "A model the GPU cannot run whole fails to build, and says so; the CPU builds it",
    .enabled(if: LiteRtLibraries.profiles.contains(.gpu), "no OpenCL for LiteRT's GPU here"))
  func gpuRunsEveryOpOrNothing() throws {
    let temp = try TemporaryDirectory()
    let model = temp.url.appending(path: "add.onnx")
    try RankFiveAdd.write(to: model)
    let artifact = temp.url.appending(path: "add.litertcache")
    #expect {
      try backend(.gpu).build(model: model, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
    } throws: { error in
      (error as? LiteRtError)?.description.contains("could not compile the model for the GPU") == true
    }
    #expect(!FileManager.default.fileExists(atPath: artifact.path))
    try backend(.cpu).build(model: model, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
    #expect(FileManager.default.fileExists(atPath: artifact.appending(path: "model.tflite").path))
  }

  @Test("An artifact from another conversion is rebuilt")
  func conversionVersion() throws {
    let temp = try TemporaryDirectory()
    let artifact = temp.url.appending(path: "tiny.litertcache")
    try backend(.cpu).build(model: TinyModel.queued, artifact: artifact, report: { _, _, _ in }, metaExtra: [:])
    try backend(.cpu).load(artifact: artifact, report: { _, _, _ in }).close()
    var meta = Artifact.sidecar(artifact)
    meta["prepare"] = LiteRtBackend.conversionVersion + 1
    try Artifact.writeSidecar(artifact, meta)
    #expect(throws: ArtifactInvalid.self) { try backend(.cpu).load(artifact: artifact, report: { _, _, _ in }) }
    #expect(throws: ArtifactInvalid.self) { try backend(.cpu).load(artifact: temp.url.appending(path: "missing.litertcache"), report: { _, _, _ in }) }
  }
}

/// What needs no library: the profiles and the tags.
@Suite("LiteRT profiles")
struct LiteRtProfileTests {
  @Test("The profiles are the apps' device names, and the tag names the chip")
  func profiles() {
    #expect(LiteRtProfile.allCases.map(\.rawValue) == ["gpu", "cpu", "npu"])
    let backend = LiteRtBackend(profile: .gpu, preparer: ONNXPreparer(), chip: "Tensor G5")
    #expect(backend.name == "litert" && backend.suffix == ".litertcache")
    #expect(backend.deviceTag() == "gpu-Tensor_G5")
    #expect(backend.tag() == "litert2.2.0.gpu-Tensor_G5")
    #expect(LiteRtBackend(profile: .cpu, preparer: ONNXPreparer()).deviceTag() == sanitize("cpu-\(HostChip.name())"))
    #expect(LiteRtBackend(profile: .gpu, preparer: ONNXPreparer(), chip: "").deviceTag() == "gpu-unknown")
    #expect(LiteRtBackend(profile: .npu, preparer: ONNXPreparer(), chip: "Tensor G5").deviceTag() == "npu-Tensor_G5")
  }

  @Test("The NPU's artifacts are for one system build, the GPU's for any")
  func npuTagNamesTheSystemBuild() {
    let before = "google/rango/rango:17/CP3A.260905.009/14256703:user/release-keys"
    let after = "google/rango/rango:17/CP3A.261005.001/14398812:user/release-keys"
    let tag = LiteRtBackend(profile: .npu, preparer: ONNXPreparer(), chip: "Tensor G5", firmware: before).deviceTag()
    #expect(tag.hasPrefix("npu-Tensor_G5-") && tag.count == "npu-Tensor_G5-".count + 8)
    #expect(tag != LiteRtBackend(profile: .npu, preparer: ONNXPreparer(), chip: "Tensor G5", firmware: after).deviceTag())
    #expect(LiteRtBackend(profile: .gpu, preparer: ONNXPreparer(), chip: "Tensor G5", firmware: before).deviceTag() == "gpu-Tensor_G5")
  }

  @Test("The NPU's options keep its compiled model where the GPU's and the CPU's do not look")
  func npuOptions() {
    let compiled = URL(fileURLWithPath: "/tmp/\(LiteRtArtifact.npuCache)")
    guard case .npu(let directory, let gpu) = LiteRtProfile.npu.options(compiled: compiled) else {
      Issue.record("not the NPU's options")
      return
    }
    #expect(directory == compiled && gpu == nil)
    guard case .gpu = LiteRtProfile.gpu.options(compiled: compiled) else {
      Issue.record("not the GPU's options")
      return
    }
  }

  @Test("LiteRT's copy of the model compiled for the NPU is found where its cache files it")
  func keptNPUModel() throws {
    let temp = try TemporaryDirectory()
    #expect(!LiteRtArtifact.keptNPUModel(temp.url))
    // litert/core/cache/compilation_cache.cc: <dir>/<model name>/<content hash>/<config hash>.tflite
    let filed = temp.url.appending(path: "\(LiteRtArtifact.npuCache)/model/1234", directoryHint: .isDirectory)
    try FileManager.default.createDirectory(at: filed, withIntermediateDirectories: true)
    #expect(!LiteRtArtifact.keptNPUModel(temp.url))
    try Data([0]).write(to: filed.appending(path: "5678.tflite"))
    #expect(LiteRtArtifact.keptNPUModel(temp.url))
  }

  @Test("An NPU attempt's lines come out of logcat's, newest last, jetlink's own left out")
  func logLines() {
    // `logcat -v epoch` from the Pixel 10 Pro Fold that refused the NPU (2026-10-03)
    let text = """
      1791043558.844  3544  5944 I litert  : [compiler_plugin.cc:433] Loading Google Tensor Compiler Adapter
      1791043559.501  3544  5944 E libedgetpu_client: Compile failed, error code 16, message: Current application should not be allowed to access EdgeTPU.
      1791043559.616  3544  5944 E litert  : [compiler_plugin.cc:876] Failed to compile model: Compile failed, error code 16
      1791043566.223  3544  5944 W jetlink : the NPU's compiler could not take 404a18cfd86d2963.onnx
      --------- beginning of main
      """
    let lines = LiteRtLog.select(text, since: 1_791_043_559)
    #expect(
      lines == [
        "E libedgetpu_client: Compile failed, error code 16, message: Current application should not be allowed to access EdgeTPU.",
        "E litert: [compiler_plugin.cc:876] Failed to compile model: Compile failed, error code 16",
      ])
    #expect(LiteRtLog.select(text, since: 0, limit: 1) == ["E litert: [compiler_plugin.cc:876] Failed to compile model: Compile failed, error code 16"])
    #expect(LiteRtLog.refusal(lines)?.contains("allowlist") == true)
    #expect(LiteRtLog.refusal(["I litert: [compiler_plugin.cc:433] Loading Google Tensor Compiler Adapter"]) == nil)
  }

  @Test("What runs the model names the chip for the NPU")
  func labels() {
    #expect(LiteRtProfile.npu.label(chip: "Tensor G4") == "NPU(Tensor G4)")
    #expect(LiteRtProfile.gpu.label(chip: "Tensor G4") == "GPU(fp16)")
    #expect(LiteRtProfile.npu.name == "NPU")
  }

  @Test("The NPU's profile says where it runs, and that a Mac is not it")
  func npuOpens() {
    #if !os(Android)
      #expect(throws: LiteRtError.self) { try LiteRtBackend(profile: .npu, preparer: ONNXPreparer()).open() }
    #endif
  }

  @Test("A compile for the NPU that never returned is remembered beside the artifact")
  func npuAttempt() throws {
    let temp = try TemporaryDirectory()
    let artifact = temp.url.appending(path: "abc.litert2.2.0.npu-Tensor_G5-1a2b3c4d.litertcache")
    let attempt = NPUAttempt(artifact: artifact, stage: .compile)
    #expect(attempt.url.lastPathComponent == "abc.litert2.2.0.npu-Tensor_G5-1a2b3c4d.npu-compiling")
    #expect(!attempt.diedBefore)
    // a compile that returns leaves no mark, even one that throws
    #expect(try attempt.during { attempt.diedBefore })
    #expect(!attempt.diedBefore)
    #expect(throws: LiteRtError.self) { try attempt.during { throw LiteRtError("no") } }
    #expect(!attempt.diedBefore)
    // one that killed the app leaves it; a load's mark is its own
    try Data().write(to: attempt.url)
    #expect(attempt.diedBefore)
    #expect(!NPUAttempt(artifact: artifact, stage: .load).diedBefore)
  }
}

/// A real model on LiteRT in a closed loop, where $JETLINK_LITERT_BENCH names
/// a directory holding model.tflite and, for each input the host writes,
/// `<name>.bin`: its frames back to back. Logs the compile, the first frame
/// and the frame time ($JETLINK_LITERT_BENCH_FRAMES of them, 200 by default),
/// and writes the driving output of each recorded frame to
/// outputs.litert.bin for a parity check against onnxruntime's.
/// $JETLINK_LITERT_BENCH_DEVICE is gpu (the default) or cpu.
@Suite(
  "LiteRT bench", .serialized,
  .enabled(if: LiteRtLibraries.available && LiteRtLibraries.bench != nil, "no LiteRT, or no model in $JETLINK_LITERT_BENCH"))
struct LiteRtBenchTests {
  @Test("A model in a closed loop: compile, first frame, frame time, outputs")
  func closedLoop() throws {
    let environment = ProcessInfo.processInfo.environment
    let directory = try #require(LiteRtLibraries.bench)
    let profile = LiteRtProfile(rawValue: environment["JETLINK_LITERT_BENCH_DEVICE"] ?? "") ?? .gpu
    let total = Int(environment["JETLINK_LITERT_BENCH_FRAMES"] ?? "") ?? 200
    try LiteRtRuntime.load()
    let compiled = directory.appending(path: LiteRtArtifact.npuCache, directoryHint: .isDirectory)
    try FileManager.default.createDirectory(at: compiled, withIntermediateDirectories: true)
    let compileStarted = DispatchTime.now()
    let engine = try LiteRtEngine(
      model: directory.appending(path: "model.tflite"), options: profile.options(compiled: compiled), device: "bench", label: { $0.rawValue })
    defer { engine.close() }
    let compileMs = Double(DispatchTime.now().uptimeNanoseconds - compileStarted.uptimeNanoseconds) / 1e6
    #expect(engine.fullyAccelerated || profile == .cpu)
    try engine.loopState(
      engine.inputs.keys.filter { $0.hasPrefix("state_") && engine.outputs["next_\($0)"] != nil }.map { (input: $0, output: "next_\($0)") })

    let frames = try Dictionary(uniqueKeysWithValues: engine.hostInputs.map { ($0, try Data(contentsOf: directory.appending(path: "\($0).bin"))) })
    let recorded = engine.hostInputs.map { frames[$0]!.count / engine.inputs[$0]!.byteCount }.min() ?? 0
    #expect(recorded > 0)
    let output = try #require(engine.outputs["outputs"])
    var outputs = Data(capacity: recorded * output.byteCount)
    var scratch = Data(count: output.byteCount)
    var times: [Double] = []
    for i in 0..<max(total, recorded) {
      let frame = i % recorded
      let started = DispatchTime.now()
      for name in engine.hostInputs {
        let bytes = engine.inputs[name]!.byteCount
        frames[name]!.withUnsafeBytes { engine.hostInput(name)!.copyMemory(from: $0.baseAddress! + frame * bytes, byteCount: bytes) }
      }
      try engine.run()
      scratch.withUnsafeMutableBytes { $0.baseAddress!.copyMemory(from: engine.output("outputs")!, byteCount: output.byteCount) }
      times.append(Double(DispatchTime.now().uptimeNanoseconds - started.uptimeNanoseconds) / 1e6)
      if i < recorded { outputs.append(scratch) }
    }
    try outputs.write(to: directory.appending(path: "outputs.litert.bin"))
    let steady = Array(times.dropFirst(2))
    let stats = BenchmarkStats.of(steady)
    print(
      String(
        format:
          "LiteRT bench %@ (%@): compile %.0f ms, first frame %.1f ms, then mean %.2f ms p50 %.2f ms p99 %.2f ms max %.2f ms over %d frames; %d recorded",
        profile.rawValue, engine.notes, compileMs, times[0], stats.mean, stats.p50, stats.p99, stats.max, steady.count, recorded))
  }
}
