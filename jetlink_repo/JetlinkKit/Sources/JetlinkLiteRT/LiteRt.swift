import CLiteRt
import Foundation
import JetlinkServer

#if canImport(Android)
  import Android
#endif

/// LiteRT, through the C shim in CLiteRt.
public enum LiteRtRuntime {
  /// The release jetlink builds against and ships: CLiteRt's headers, the
  /// Mac's ai-edge-litert wheel and Android's AAR. LiteRT's C API reports no
  /// version of its own.
  public static let version = "2.2.0"

  /// Where the tests and the command line find LiteRT's libraries when the
  /// host names none: the ai-edge-litert wheel's package directory, which
  /// holds libLiteRt and its GPU accelerator side by side.
  public static let directoryVariable = "JETLINK_LITERT_DIR"

  /// Opens LiteRT and makes the process's environment, or throws why it
  /// cannot, in words an app can show. `directory` holds LiteRT's
  /// libraries, as the Android app's nativeLibraryDir does. Nil falls back to
  /// $JETLINK_LITERT_DIR, then to the loader's path. The first open that
  /// succeeds decides for the process; later calls return at once.
  public static func load(directory: URL? = nil) throws {
    let path = directory?.path ?? ProcessInfo.processInfo.environment[directoryVariable]
    try LiteRtError.check(jl_litert_open(path))
  }

  /// Whether LiteRT's GPU accelerator loaded, and the names of every
  /// accelerator that did ("GPU Metal, CpuAccelerator" on a Mac). LiteRT
  /// must be open.
  public static func accelerators() throws -> (gpu: Bool, names: String) {
    var hardware: Int32 = 0
    var names = [CChar](repeating: 0, count: 256)
    try LiteRtError.check(jl_litert_accelerators(&hardware, &names, names.count))
    let text = String(decoding: names.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, as: UTF8.self)
    return (hardware & Int32(JL_LITERT_GPU) != 0, text)
  }

  #if os(Android)
    /// The names LiteRT's GPU accelerator looks for the vendor's OpenCL
    /// under, which the app's manifest declares.
    static let openCLLibraries = ["libOpenCL.so", "libOpenCL-pixel.so", "libOpenCL-car.so"]

    /// Whether this process can open the vendor's OpenCL, which LiteRT's GPU
    /// is pinned to on Android (the shim's GPU options): what a failed GPU
    /// compile is blamed on, and what the tests run the GPU's on.
    public static let hasOpenCL: Bool = openCLLibraries.contains(where: opens)

    /// The library in a Pixel's system that LiteRT's Google Tensor plugin
    /// hands a model to for compiling, and its dispatch library runs the
    /// compiled model through. Public to apps from Tensor G3 (Pixel 8) on;
    /// the app's manifest declares it.
    static let googleTensorSystemLibrary = "libedgetpu_litert.so"

    /// LiteRT's Google Tensor libraries, which the app ships beside
    /// libLiteRt.so: the dispatch library that runs a compiled model on the
    /// NPU, and the compiler plugin that compiles one on the phone.
    static let googleTensorLibraries = ["libLiteRtDispatch_GoogleTensor.so", "libLiteRtCompilerPlugin_google_tensor.so"]

    /// Whether this process can open the Google Tensor NPU's system library.
    public static let hasGoogleTensorNPU: Bool = opens(googleTensorSystemLibrary)

    private static func opens(_ name: String) -> Bool {
      guard let handle = dlopen(name, RTLD_NOW) else { return false }
      dlclose(handle)
      return true
    }
  #endif
}

public struct LiteRtError: Error, CustomStringConvertible {
  public let description: String

  init(_ description: String) {
    self.description = description
  }

  /// Throws the shim's message when there is one, and frees it.
  static func check(_ message: UnsafeMutablePointer<CChar>?) throws {
    guard let message else { return }
    defer { jl_litert_free(message) }
    throw LiteRtError(String(cString: message))
  }
}

/// LiteRT's lines in this process's log: the reason an NPU compile failed,
/// which LiteRT's Google Tensor plugin and the phone's compiler say only
/// there. The app's log, which a tester shares, gets a copy.
enum LiteRtLog {
  /// The lines of `logcat -v epoch` text logged at `since` or after, but for
  /// jetlink's own, as "W litert: message": the last `limit` of them.
  static func select(_ text: String, since: TimeInterval, limit: Int = 60) -> [String] {
    var lines: [String] = []
    for line in text.split(separator: "\n") {
      // "1791043556.899  3544  3782 W litert  : message"
      let fields = line.split(separator: " ", maxSplits: 4, omittingEmptySubsequences: true)
      guard fields.count == 5, let time = Double(fields[0]), time >= since, let colon = fields[4].range(of: ": ") else { continue }
      let tag = fields[4][..<colon.lowerBound].trimmingCharacters(in: .whitespaces)
      guard tag != "jetlink" else { continue }
      lines.append("\(fields[3]) \(tag): \(fields[4][colon.upperBound...])")
    }
    return Array(lines.suffix(limit))
  }

  /// What `lines` say kept the NPU from the model, in words for the log,
  /// when it is a reason jetlink knows: a Pixel's EdgeTPU service serves only
  /// the apps on Google's allowlist, by package and signing key ("error code
  /// 16", seen on a Pixel 10 Pro Fold, 2026-10-03).
  static func refusal(_ lines: [String]) -> String? {
    guard lines.contains(where: { $0.contains("not be allowed to access EdgeTPU") || $0.contains("not in the EdgeTPU allowed list") }) else {
      return nil
    }
    return "the phone's NPU serves only the apps on Google's EdgeTPU allowlist, and this build of Jetlink is not on it"
  }

  #if os(Android)
    /// What this process logged since `start`: LiteRT's info lines and up,
    /// everything else's warnings and up. An app may read its own.
    static func lines(since start: Date) -> [String] {
      guard let pipe = popen("logcat -d -v epoch --pid=\(getpid()) litert:I *:W", "r") else { return [] }
      defer { pclose(pipe) }
      var data = Data()
      var buffer = [UInt8](repeating: 0, count: 1 << 16)
      while true {
        let n = fread(&buffer, 1, buffer.count, pipe)
        if n <= 0 { break }
        data.append(contentsOf: buffer[..<n])
      }
      // a second's slack for the log's own clock
      return select(String(decoding: data, as: UTF8.self), since: start.timeIntervalSince1970 - 1)
    }
  #endif
}

/// How a model is compiled: which accelerator runs it and with what.
enum LiteRtCompileOptions: Sendable {
  /// The GPU alone, in fp16, so an op it cannot run fails the compile
  /// rather than quietly running on the CPU. `cache` is where it keeps the
  /// programs it compiled, and the key it files them under.
  case gpu(cache: (directory: URL, key: String)?)
  /// The NPU, the model compiled for it on the phone and kept in `compiled`,
  /// an existing directory, for the next load with the same one. Where the
  /// NPU's compiler cannot take the model, the GPU runs it as `.gpu(cache:)`
  /// would: `LiteRtModel.hardware` tells which one did.
  case npu(compiled: URL, gpu: (directory: URL, key: String)?)
  /// XNNPACK on the CPU, with a pool of `threads`.
  case cpu(threads: Int)
}

/// One compiled model: a .tflite, compiled for an accelerator, and the
/// inputs and outputs of its first signature, in signature order.
final class LiteRtModel: @unchecked Sendable {
  let pointer: OpaquePointer
  let inputs: [TensorSpec]
  let outputs: [TensorSpec]

  /// LiteRT must be open (`LiteRtRuntime.load`).
  init(model: URL, options: LiteRtCompileOptions) throws {
    var shim = jl_litert_options()
    var cache: (directory: URL, key: String)?
    var npuCache: URL?
    switch options {
    case .gpu(let programs):
      shim.gpu = 1
      cache = programs
    case .npu(let directory, let programs):
      shim.gpu = 1
      cache = programs
      npuCache = directory
    case .cpu(let threads):
      shim.cpu_threads = Int32(threads)
    }
    let cacheDirectory = cache.flatMap { strdup($0.directory.path) }
    let cacheKey = cache.flatMap { strdup($0.key) }
    let npuCacheDirectory = npuCache.flatMap { strdup($0.path) }
    defer {
      free(cacheDirectory)
      free(cacheKey)
      free(npuCacheDirectory)
    }
    shim.cache_dir = UnsafePointer(cacheDirectory)
    shim.cache_key = UnsafePointer(cacheKey)
    shim.npu_cache_dir = UnsafePointer(npuCacheDirectory)
    var compiled: OpaquePointer?
    try model.path.withCString { path in
      try LiteRtError.check(jl_litert_model_create(path, &shim, &compiled))
    }
    guard let compiled else { throw LiteRtError("LiteRT returned no compiled model for \(model.lastPathComponent)") }
    pointer = compiled
    do {
      inputs = try LiteRtModel.describe(compiled, output: false)
      outputs = try LiteRtModel.describe(compiled, output: true)
    } catch {
      jl_litert_model_release(compiled)
      throw error
    }
  }

  deinit {
    jl_litert_model_release(pointer)
  }

  /// Whether the accelerator asked for runs every op.
  var fullyAccelerated: Bool {
    get throws {
      var fully: Int32 = 0
      try LiteRtError.check(jl_litert_model_fully_accelerated(pointer, &fully))
      return fully != 0
    }
  }

  /// What runs the model: on an NPU compile, the NPU, or the GPU when the
  /// NPU's compiler could not take it.
  var hardware: LiteRtProfile {
    get throws { try reads(input: 0) }
  }

  /// Whether the accelerator reads input `index` straight from host memory,
  /// as the CPU does, where a GPU or an NPU wants its own.
  func readsHostMemory(input index: Int) throws -> Bool {
    try reads(input: index) == .cpu
  }

  /// Whose memory input `index` is read from best.
  private func reads(input index: Int) throws -> LiteRtProfile {
    var hardware: Int32 = 0
    try LiteRtError.check(jl_litert_model_input_hardware(pointer, index, &hardware))
    switch hardware {
    case Int32(JL_LITERT_NPU): return .npu
    case Int32(JL_LITERT_GPU): return .gpu
    default: return .cpu
    }
  }

  private static func describe(_ model: OpaquePointer, output: Bool) throws -> [TensorSpec] {
    try TensorSpec.described(count: jl_litert_model_io_count(model, output ? 1 : 0)) { index, name, type, dims, rank in
      try LiteRtError.check(jl_litert_model_io_info(model, output ? 1 : 0, index, &name, name.count, &type, &dims, dims.count, &rank))
    }
  }
}

/// One of LiteRT's tensor buffers: over memory the engine owns, or the
/// accelerator's own (GPU memory on a GPU).
final class LiteRtBuffer {
  let pointer: OpaquePointer

  /// A buffer for input or output `index` over `bytes` at `data`, 64-byte
  /// aligned, which must outlive it.
  init(_ model: LiteRtModel, output: Bool, index: Int, wrapping data: UnsafeMutableRawPointer, bytes: Int) throws {
    var buffer: OpaquePointer?
    try LiteRtError.check(jl_litert_buffer_wrap(model.pointer, output ? 1 : 0, index, data, bytes, &buffer))
    guard let buffer else { throw LiteRtError("LiteRT returned no buffer") }
    pointer = buffer
  }

  /// A zeroed buffer of the kind the accelerator works in for input or
  /// output `index`.
  init(_ model: LiteRtModel, output: Bool = false, index: Int) throws {
    var buffer: OpaquePointer?
    try LiteRtError.check(jl_litert_buffer_create(model.pointer, output ? 1 : 0, index, &buffer))
    guard let buffer else { throw LiteRtError("LiteRT returned no buffer") }
    pointer = buffer
  }

  deinit {
    jl_litert_buffer_release(pointer)
  }

  func zero() throws {
    try LiteRtError.check(jl_litert_buffer_write(pointer, nil, 0))
  }

  func read(into data: UnsafeMutableRawPointer, bytes: Int) throws {
    try LiteRtError.check(jl_litert_buffer_read(pointer, data, bytes))
  }

  func write(from data: UnsafeRawPointer, bytes: Int) throws {
    try LiteRtError.check(jl_litert_buffer_write(pointer, data, bytes))
  }
}

extension LiteRtModel {
  /// One run of the first signature, over buffers' pointers in signature
  /// order, which the caller keeps alive.
  func run(inputs: [OpaquePointer?], outputs: [OpaquePointer?]) throws {
    try LiteRtError.check(jl_litert_run(pointer, inputs, inputs.count, outputs, outputs.count))
  }
}
