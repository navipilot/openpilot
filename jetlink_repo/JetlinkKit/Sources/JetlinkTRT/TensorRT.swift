import CTrt
import Foundation
import JetlinkServer

/// A shim call that failed: TensorRT's reason, or CUDA's error name and
/// description. A sticky CUDA error breaks the context for the life of the
/// process, so it is fatal (D15): the session answers the frame, then the
/// daemon exits for systemd to start it again.
public struct TrtError: FatalEngineError, CustomStringConvertible, Equatable {
  /// A JL_TRT_ code: JL_TRT_UNAVAILABLE when TensorRT cannot run here at all.
  public let code: Int32
  public let description: String

  public init(code: Int32 = Int32(JL_TRT_ERROR), _ description: String) {
    self.code = code
    self.description = description
  }

  public var isFatal: Bool { code == JL_TRT_CUDA_STICKY }
}

private let trtLog = ServerLog(category: "tensorrt")

/// TensorRT's log lines, from any of its threads, one at a time.
private func forwardLog(_ ctx: UnsafeMutableRawPointer?, _ severity: Int32, _ message: UnsafePointer<CChar>?) {
  guard let message else { return }
  let line = String(cString: message)
  switch severity {
  case ...Int32(JL_TRT_LOG_ERROR): trtLog.error(line)
  case Int32(JL_TRT_LOG_WARNING): trtLog.warning(line)
  default: trtLog.info(line)
  }
}

/// CUDA and TensorRT, opened once for the process: the libraries, the device
/// and its primary context. Engines and builds keep it alive, since their
/// handles must go before it does.
public final class TensorRT: @unchecked Sendable {
  let handle: OpaquePointer
  /// The CUDA device index.
  public let device: Int
  public let major: Int
  public let minor: Int
  public let patch: Int
  public let build: Int
  public let deviceName: String
  public let computeCapability: (major: Int, minor: Int)
  public let stronglyTyped: Bool
  public let plugins: Bool
  /// cuDriverGetVersion: 12060 for CUDA 12.6.
  public let cudaDriver: Int
  /// The libnvinfer loaded, as a path.
  public let library: String

  /// Opens `device`, and checks it answers. TensorRT comes from `libraries`,
  /// a directory holding a self-contained copy, else from the loader path.
  /// Throws JL_TRT_UNAVAILABLE on a machine without a driver, a TensorRT of
  /// the shim's major, or the device, and always on the fake shim: a build
  /// without TensorRT's headers never runs a model.
  public convenience init(device: Int = 0, libraries: String? = nil) throws(TrtError) {
    let unavailable = Int32(JL_TRT_UNAVAILABLE)
    guard let index = Int32(exactly: device), index >= 0 else { throw TrtError(code: unavailable, "no CUDA device \(device)") }
    var handle: OpaquePointer?
    var err = [CChar](repeating: 0, count: 512)
    guard jl_trt_open(index, libraries, &handle, &err, err.count) == JL_TRT_OK, let handle else {
      throw TrtError(code: unavailable, string(err))
    }
    self.init(handle: handle, device: device)
    do {
      var free = 0
      var total = 0
      try check { jl_trt_mem_info(handle, &free, &total, $0, $1) }
    } catch {
      throw TrtError(code: unavailable, "the GPU is not usable: \(error)")
    }
  }

  /// Takes over an open handle, jl_trt_open's (or jl_trt_fake_open's in the
  /// tests), and closes it when the last engine and backend let go.
  package init(handle: OpaquePointer, device: Int = 0) {
    self.handle = handle
    self.device = device
    var info = jl_trt_info()
    jl_trt_get_info(handle, &info)
    major = Int(info.major)
    minor = Int(info.minor)
    patch = Int(info.patch)
    build = Int(info.build)
    deviceName = info.device_name.map { String(cString: $0) } ?? ""
    computeCapability = (Int(info.cc_major), Int(info.cc_minor))
    stronglyTyped = info.strongly_typed != 0
    plugins = info.plugins != 0
    cudaDriver = Int(info.cuda_driver)
    library = info.library.map { String(cString: $0) } ?? ""
    // Python's trt.Logger(WARNING). Set before any engine or build, as the
    // shim asks; it never logs the plugin registration, which came before.
    jl_trt_set_logger(handle, Int32(JL_TRT_LOG_WARNING), forwardLog, nil)
    trtLog.info(
      "TensorRT \(fullVersion) on \(deviceName) (sm\(computeCapability.major)\(computeCapability.minor)), CUDA driver "
        + "\(cudaDriver / 1000).\(cudaDriver % 1000 / 10), \(plugins ? "plugins registered" : "no plugin library"), from \(library)")
  }

  deinit {
    jl_trt_close(handle)
  }

  /// "10.16.2.10": the build a plan or a timing cache is valid for.
  public var fullVersion: String { "\(major).\(minor).\(patch).\(build)" }

  /// The release as Python's `tensorrt.__version__` printed it, which the
  /// cache tags and sidecars carry: "10.3.0" for JetPack 6, whose wheel left
  /// the build out, and all four parts on every other ("10.16.2.10" on
  /// JetPack 7, "11.3.0.99" on a PC), so the plans Python built load here
  /// without a rebuild.
  public var version: String { Self.pythonVersion(major: major, minor: minor, patch: patch, build: build) }

  static func pythonVersion(major: Int, minor: Int, patch: Int, build: Int) -> String {
    major == 10 && minor == 3 ? "\(major).\(minor).\(patch)" : "\(major).\(minor).\(patch).\(build)"
  }

  /// A sticky CUDA error has been seen on this handle.
  public var isSticky: Bool { jl_trt_sticky(handle) != 0 }

  /// Page-locked host memory, which the staging writes into and the copies
  /// move without a bounce buffer.
  var pinned: HostAllocator {
    HostAllocator(
      allocate: { [self] bytes in
        var pointer: UnsafeMutableRawPointer?
        try check { jl_trt_host_alloc(handle, bytes, &pointer, $0, $1) }
        return pointer!
      },
      free: { [self] in jl_trt_host_free(handle, $0) })
  }

  /// One shim call, with room for its reason; anything but JL_TRT_OK throws.
  @inline(__always)
  func check(capacity: Int = 512, _ call: (UnsafeMutablePointer<CChar>, Int) -> Int32) throws(TrtError) {
    try withUnsafeTemporaryAllocation(of: CChar.self, capacity: capacity) { (err) throws(TrtError) in
      err[0] = 0
      let rc = call(err.baseAddress!, err.count)
      guard rc == JL_TRT_OK else {
        let message = String(cString: err.baseAddress!)
        throw TrtError(code: rc, message.isEmpty ? "TensorRT shim error \(rc)" : message)
      }
    }
  }
}

/// A shim's reason from a buffer it NUL terminated.
func string(_ buffer: [CChar]) -> String {
  String(decoding: buffer.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, as: UTF8.self)
}
