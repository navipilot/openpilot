import Foundation
import JetlinkKit
import JetlinkONNX
import JetlinkRegistry
import JetlinkServer

#if canImport(CryptoKit)
  import CryptoKit
#else
  import Crypto
#endif

/// Where LiteRT runs a model. The raw value is the device the Android app's
/// start config and `--device` name beside the backend, "litert", and the
/// device part of the artifact's tag.
public enum LiteRtProfile: String, CaseIterable, Sendable {
  /// The whole graph on the GPU in fp16: OpenCL on Android, Metal on a Mac.
  /// A model the GPU cannot run whole fails to build rather than run partly
  /// on the CPU at a fraction of the speed.
  case gpu
  /// XNNPACK on the CPU: for tests, the emulator, and phones LiteRT drives no
  /// GPU on. Nowhere near the frame budget with a real model.
  case cpu
  /// A Google Tensor's NPU (G3, the Pixel 8, and later), the model compiled
  /// for it on the phone: LiteRT's Google Tensor plugin hands the converted
  /// model to the compiler in the phone's system, and LiteRT keeps what it
  /// compiles with the artifact. A model the NPU's compiler cannot take runs
  /// on the GPU instead, as the gpu profile runs it. Android only.
  case npu

  /// The accelerator, as messages name it.
  var name: String { rawValue.uppercased() }

  /// What the log and the benchmark's report call it on `chip`.
  func label(chip: String) -> String {
    switch self {
    case .gpu: "GPU(fp16)"
    case .cpu: "CPU(\(HostChip.cpuThreads) threads)"
    case .npu: "NPU(\(chip))"
    }
  }

  /// A first compile with no earlier build to go by. The GPU's: 7 to 12 s
  /// for Cinque Terre V3 on an M1 Pro's Metal; a phone's OpenCL compile is
  /// unmeasured. The NPU's is unmeasured on a phone; Google's compiler for
  /// Tensor took minutes for Cinque Terre V3 on a PC.
  var expectedCompileSeconds: Double {
    switch self {
    case .gpu: 60
    case .cpu: 0
    case .npu: 300
    }
  }

  /// How the profile compiles a model; on the GPU and the NPU, the GPU's
  /// programs cached in `cache` when there is one. The NPU keeps the model
  /// it compiled in `compiled`, an existing directory.
  func options(cache: (directory: URL, key: String)? = nil, compiled: URL) -> LiteRtCompileOptions {
    switch self {
    case .gpu: .gpu(cache: cache)
    case .cpu: .cpu(threads: HostChip.cpuThreads)
    case .npu: .npu(compiled: compiled, gpu: cache)
    }
  }
}

/// LiteRT in process, on the GPU, the CPU or a Google Tensor's NPU: the
/// backend for phones onnxruntime's QNN does not drive. The artifact is a
/// directory holding the converted model, the GPU's compile cache and the
/// model compiled for the NPU (LiteRtArtifact).
///
/// Nothing here has run on a phone yet: the GPU path is Metal's on a Mac,
/// Android's OpenCL one follows LiteRT 2.2.0's documentation and source, and
/// the NPU's follows LiteRT's source, the Pixel system library it calls
/// (seen in Pixel 8, 9 and 10 Pro Fold factory images), and Google's
/// compiler for Tensor, which compiles every op of Cinque Terre V3 as the
/// conversion writes it for a G5.
public final class LiteRtBackend: EngineBackend {
  /// What JetlinkONNX's LiteRTPreparation writes now: an artifact converted
  /// under another version rebuilds.
  static let conversionVersion = 1

  public let name = "litert"
  /// A directory: the converted model and the GPU's compile cache.
  public let suffix = ".litertcache"
  public let profile: LiteRtProfile
  /// Where LiteRT's libraries are; nil for $JETLINK_LITERT_DIR or the
  /// loader's path (`LiteRtRuntime.load`).
  public let libraries: URL?
  private let preparer: any ModelPreparer
  private let chip: String
  /// The phone's build fingerprint, which a model compiled for the NPU is
  /// good for; "" where the app names none.
  private let firmware: String
  private let log = ServerLog(category: "litert")

  /// `libraries` is the directory holding libLiteRt and its GPU
  /// accelerator, the Android app's nativeLibraryDir; nil reads
  /// $JETLINK_LITERT_DIR. `chip` names what the artifacts are valid for: on
  /// Android the SoC's model, "Tensor G5" (Build.SOC_MODEL), which a GPU's
  /// compiled programs are for; nil is HostChip's name for this machine.
  /// `firmware` is Android's Build.FINGERPRINT: the NPU's compiler is part
  /// of the system, so the NPU's artifacts are for one system build.
  public init(profile: LiteRtProfile, preparer: any ModelPreparer, libraries: URL? = nil, chip: String? = nil, firmware: String? = nil) {
    self.profile = profile
    self.preparer = preparer
    self.libraries = libraries
    self.chip = HostChip.resolve(chip)
    self.firmware = firmware ?? ""
  }

  /// The GPU accelerator's library, which LiteRT opens from beside its own.
  static var gpuLibrary: String {
    #if os(macOS) || os(iOS)
      "libLiteRtMetalAccelerator.dylib"
    #else
      "libLiteRtClGlAccelerator.so"
    #endif
  }

  /// Opens LiteRT from `libraries`, and for the GPU checks its accelerator
  /// loaded: both are what an app shows when they fail. Whoever makes the
  /// backend calls it, before any build or load: the app as its server
  /// starts, so a phone LiteRT cannot run on says so at once rather than
  /// after converting a model.
  public func open() throws {
    if profile == .npu {
      try LiteRtBackend.checkNPU(libraries: libraries)
    }
    try LiteRtRuntime.load(directory: libraries)
    // the NPU's profile runs on the GPU what the NPU cannot take
    if profile != .cpu {
      let (gpu, names) = try LiteRtRuntime.accelerators()
      guard gpu else {
        throw LiteRtError(
          "LiteRT's GPU accelerator did not load (\(LiteRtBackend.gpuLibrary) beside libLiteRt), so only the CPU can run LiteRT here; "
            + "it has \(names.isEmpty ? "no accelerators" : names)")
      }
    }
  }

  /// Throws what keeps this process from the NPU: LiteRT's Google Tensor
  /// libraries missing beside libLiteRt, or no Tensor NPU library in the
  /// phone's system.
  static func checkNPU(libraries: URL?) throws {
    #if os(Android)
      guard let libraries else { throw LiteRtError("the NPU's libraries are looked for beside libLiteRt, and the app named no directory") }
      if let missing = LiteRtRuntime.googleTensorLibraries.first(where: { !FileManager.default.fileExists(atPath: libraries.appending(path: $0).path) }) {
        throw LiteRtError("\(missing) is not in the app's libraries, so LiteRT cannot reach the NPU")
      }
      guard LiteRtRuntime.hasGoogleTensorNPU else {
        throw LiteRtError(
          "this phone's system gives apps no \(LiteRtRuntime.googleTensorSystemLibrary), the Google Tensor NPU's library (Pixel 8 and later); "
            + "the GPU profile runs on any phone")
      }
    #else
      throw LiteRtError("LiteRT's NPU profile runs on Android, on a Google Tensor G3 (Pixel 8) or later")
    #endif
  }

  public var runtimeVersion: String { LiteRtRuntime.version }

  /// The NPU's names the system build too: a system update brings another
  /// compiler, so the model is prepared again.
  public func deviceTag() -> String {
    guard profile == .npu, !firmware.isEmpty else { return sanitize("\(profile.rawValue)-\(chip)") }
    let build = SHA256.hash(data: Data(firmware.utf8)).prefix(4).map { String(format: "%02x", $0) }.joined()
    return sanitize("\(profile.rawValue)-\(chip)-\(build)")
  }

  public func deriveSpec(model: URL, sha256: String, nbytes: Int64, frameSkip: Int) throws -> ModelSpec {
    try preparer.readSpec(model: model, sha256: sha256, nbytes: nbytes, frameSkip: frameSkip)
  }

  /// The model in `directory`, an artifact or its staging, compiled for
  /// `target`: the profile, or for the NPU's the GPU alone. On the GPU or
  /// the NPU every op must be the accelerator's: a model that runs partly on
  /// the CPU is a build that failed.
  func engine(_ directory: URL, cacheKey: String, on target: LiteRtProfile) throws -> LiteRtEngine {
    let engine: LiteRtEngine
    do {
      engine = try LiteRtEngine(
        model: directory.appending(path: LiteRtArtifact.model),
        options: target.options(
          cache: (directory.appending(path: LiteRtArtifact.cache, directoryHint: .isDirectory), cacheKey),
          compiled: directory.appending(path: LiteRtArtifact.npuCache, directoryHint: .isDirectory)),
        device: deviceTag(), label: { $0.label(chip: chip) })
    } catch let error as LiteRtError where target != .cpu {
      #if os(Android)
        if !LiteRtRuntime.hasOpenCL {
          throw LiteRtError(
            "no OpenCL: LiteRT's GPU runs on OpenCL here, and \(chip) gives apps none of "
              + "\(LiteRtRuntime.openCLLibraries.joined(separator: ", ")) (\(error.description)); the CPU profile runs every op")
        }
      #endif
      // A GPU-only compile fails outright on an op the GPU cannot run.
      throw LiteRtError(
        "LiteRT could not compile the model for the GPU on \(chip) (\(error.description)); LiteRT's log names any op the GPU cannot run, "
          + "and the CPU profile runs every op")
    }
    if target != .cpu && !engine.fullyAccelerated {
      let what = engine.hardware.name
      engine.close()
      throw LiteRtError("LiteRT's \(what) on \(chip) cannot run every op of this model; LiteRT's log names them, and the CPU profile runs every op")
    }
    return engine
  }

  // MARK: build

  public func build(model: URL, artifact: URL, report: @escaping ProgressFn, metaExtra: [String: Any]) throws {
    let started = Date()
    let expect = Artifact.sidecar(artifact)
    try Artifact.build(artifact, metaExtra: metaExtra, report: report) { staged in
      try FileManager.default.createDirectory(at: staged, withIntermediateDirectories: true)
      report("convert", 0, "converting the model for LiteRT")
      let convertStarted = Date()
      let convertTook = (expect["convert_seconds"] as? NSNumber)?.doubleValue ?? 0
      let converted = try Ticker.during(interval: 1, Ticker.paced("convert", "converting the model for LiteRT", took: convertTook, report: report)) {
        try LiteRTPreparation.prepare(source: model, into: staged)
      }
      let convertSeconds = Date().timeIntervalSince(convertStarted)
      log.info("converted \(model.lastPathComponent): \(converted.summary)")
      report("convert", 1, "converted in \(Int(convertSeconds.rounded())) s")

      guard converted.url.lastPathComponent == LiteRtArtifact.model else {
        throw HostError.failed("the conversion wrote \(converted.url.lastPathComponent), expected \(LiteRtArtifact.model)")
      }
      for cache in [LiteRtArtifact.cache, LiteRtArtifact.npuCache] {
        try FileManager.default.createDirectory(at: staged.appending(path: cache, directoryHint: .isDirectory), withIntermediateDirectories: true)
      }

      // One compile on the profile's accelerator proves the model runs there
      // whole, and leaves the GPU's cache or the NPU's model for every load
      // after. No run: the compile writes the cache (Metal's whole-graph one
      // appears before any run), and the host's warm-up runs the loaded
      // model anyway. A compile for the NPU that killed the app last time is
      // not tried again: the GPU's alone instead.
      let attempt = NPUAttempt(artifact: artifact, stage: .compile)
      let target = profile == .npu && attempt.diedBefore ? .gpu : profile
      if target != profile {
        log.warning("the last compile for the NPU never finished, so \(model.lastPathComponent) runs on the GPU")
      }
      let what = "compiling for the \(target.name)"
      let took = (expect["compile_seconds"] as? NSNumber)?.doubleValue ?? target.expectedCompileSeconds
      report("compile", 0, what)
      let compileStarted = Date()
      let compile = {
        try Ticker.during(interval: 1, Ticker.paced("compile", what, took: took, report: report)) {
          let engine = try self.engine(staged, cacheKey: LiteRtArtifact.cacheKey(artifact), on: target)
          defer { engine.close() }
          return engine.hardware
        }
      }
      let hardware: LiteRtProfile
      do {
        hardware = try target == .npu ? attempt.during(compile) : compile()
      } catch {
        if target == .npu { copyLiteRtLog(since: compileStarted) }
        throw error
      }
      let compileSeconds = Date().timeIntervalSince(compileStarted)
      report("compile", 1, "compiled in \(Int(compileSeconds.rounded())) s")
      if hardware != target {
        log.warning("the NPU's compiler could not take \(model.lastPathComponent), so it runs on the GPU; LiteRT's lines below say why")
        copyLiteRtLog(since: compileStarted)
      }
      var meta = LiteRtArtifact.meta(self, model: model, started: started)
      meta["accelerator"] = hardware.label(chip: chip)
      meta["convert_seconds"] = pythonRound(convertSeconds, 1)
      meta["compile_seconds"] = pythonRound(compileSeconds, 1)
      meta["artifact_bytes"] = Files.size(of: staged)
      if hardware == .npu {
        meta["hardware"] = hardware.rawValue
        // LiteRT keeps no copy of a compile it cannot serialize, and then
        // compiles again at every load.
        meta["npu_kept"] = LiteRtArtifact.keptNPUModel(staged)
      }
      return meta
    }
  }

  // MARK: load

  public func load(artifact: URL, report: @escaping ProgressFn) throws -> any Engine {
    let meta = try LiteRtArtifact.open(artifact)
    // What the build found runs the model: the NPU, or for the NPU's profile
    // the GPU when it could not take it, which a load does not ask again.
    let target = profile == .npu && meta["hardware"] as? String != LiteRtProfile.npu.rawValue ? .gpu : profile
    // LiteRT reads the compiled model back rather than compile again. A load
    // that never finished prepares the model again, whose own compile is
    // marked: one that got the app killed then leaves the NPU to the GPU.
    let attempt = NPUAttempt(artifact: artifact, stage: .load)
    if target == .npu {
      if meta["npu_kept"] as? Bool == true && !LiteRtArtifact.keptNPUModel(artifact) {
        throw ArtifactInvalid("\(artifact.lastPathComponent): the model compiled for the NPU is missing")
      }
      if attempt.diedBefore {
        throw ArtifactInvalid("\(artifact.lastPathComponent): the last load for the NPU never finished")
      }
    }
    let loadStarted = Date()
    let (engine, seconds): (LiteRtEngine, TimeInterval)
    do {
      (engine, seconds) = try Artifact.load(artifact, meta: meta, what: "the model", report: report) {
        let load = { try self.engine(artifact, cacheKey: LiteRtArtifact.cacheKey(artifact), on: target) }
        return try target == .npu ? attempt.during(load) : load()
      }
    } catch {
      if target == .npu { copyLiteRtLog(since: loadStarted) }
      throw error
    }
    if engine.hardware != target {
      log.warning("\(artifact.lastPathComponent) was compiled for the NPU but loaded on the GPU; LiteRT's lines below say why")
      copyLiteRtLog(since: loadStarted)
    }
    log.info("LiteRT on \(profile.rawValue) in \(String(format: "%.1f", seconds)) s: \(engine.label)")
    return engine
  }
}

extension LiteRtBackend {
  /// LiteRT's lines since `start` into jetlink's log, which a tester shares:
  /// why the NPU took no model, which LiteRT says only in Android's log.
  func copyLiteRtLog(since start: Date) {
    #if os(Android)
      let lines = LiteRtLog.lines(since: start)
      for line in lines {
        log.warning("litert: \(line)")
      }
      if let refusal = LiteRtLog.refusal(lines) {
        log.warning("why the NPU took no model: \(refusal)")
      }
    #endif
  }
}

/// A compile or a load for the NPU under way, marked by a file beside the
/// artifact, removed when it returns. The phone's compiler runs in the app's
/// process: one that runs out of memory gets the app killed, and Android
/// starts it again into the same build. A compile's file still there says
/// so, and the next build runs on the GPU rather than die again. Removing
/// the model's prepared files, or a system update (another artifact name,
/// `deviceTag`), tries the NPU again.
struct NPUAttempt {
  enum Stage: String {
    case compile = "npu-compiling"
    case load = "npu-loading"
  }

  let url: URL

  init(artifact: URL, stage: Stage) {
    url = Artifact.beside(artifact, stage.rawValue)
  }

  /// Whether one never returned.
  var diedBefore: Bool { FileManager.default.fileExists(atPath: url.path) }

  /// Runs `body` marked, the mark removed once it returns or throws.
  func during<T>(_ body: () throws -> T) throws -> T {
    try Data().write(to: url)
    defer { try? FileManager.default.removeItem(at: url) }
    return try body()
  }
}
