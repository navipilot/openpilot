#if os(Linux)
  import Foundation
  import Glibc
  import JetlinkKit

  /// A PC's NVIDIA GPU through NVML, opened at run time from the driver's
  /// libnvidia-ml.so.1 (Python's `NvmlTelemetry`, without pynvml). The same
  /// keys as a Jetson's where the meaning matches; fan speed is a percentage
  /// and there is no memory temperature, so those are what they are. A query
  /// the GPU refuses (a laptop's fan, much of it under WSL2) reads 0; the
  /// rest still count.
  ///
  /// Never on a Tegra: NVML initializes there and every query is
  /// NotSupported, which would tell the comma a cold board drawing nothing.
  public struct NvmlTelemetry: @unchecked Sendable {
    /// dlsym over the opened library, or a test's table.
    typealias Lookup = (String) -> UnsafeMutableRawPointer?

    typealias Status = Int32
    typealias Device = OpaquePointer?
    private typealias InitFn = @convention(c) () -> Status
    private typealias HandleFn = @convention(c) (UInt32, UnsafeMutablePointer<Device>) -> Status
    private typealias UIntFn = @convention(c) (Device, UnsafeMutablePointer<UInt32>) -> Status
    private typealias WithKindFn = @convention(c) (Device, UInt32, UnsafeMutablePointer<UInt32>) -> Status
    /// nvmlTemperature_v1_t is {version, sensorType, temperature}: three
    /// 32-bit fields, passed as an array so the layout is C's.
    private typealias TemperatureVFn = @convention(c) (Device, UnsafeMutablePointer<Int32>) -> Status
    private typealias NameFn = @convention(c) (Device, UnsafeMutablePointer<CChar>, UInt32) -> Status

    static let success: Status = 0
    static let temperatureGPU: UInt32 = 0
    static let clockGraphics: UInt32 = 0
    /// NVML_STRUCT_VERSION(Temperature, 1): the struct's size, and 1 << 24.
    static let temperatureV1: Int32 = 12 | (1 << 24)

    private let device: Device
    private let temperatureV: TemperatureVFn?
    private let temperature: WithKindFn?
    private let powerUsage: UIntFn?
    private let utilization: UIntFn?
    private let clock: WithKindFn?
    private let fanSpeed: UIntFn?
    public let powerLimitW: Double
    /// The GPU's marketing name, for the status page.
    public let name: String?

    /// NVML on the driver's library for GPU `index`, or nil with why.
    public static func open(index: Int) -> Result<NvmlTelemetry, NvmlUnavailable> {
      guard let library = dlopen("libnvidia-ml.so.1", RTLD_NOW | RTLD_LOCAL) else {
        return .failure(NvmlUnavailable(reason: dlerror().map { String(cString: $0) } ?? "cannot open libnvidia-ml.so.1"))
      }
      // Never closed: the process keeps NVML for as long as it runs.
      return NvmlTelemetry.make(index: index) { dlsym(library, $0) }
    }

    static func make(index: Int, lookup: Lookup) -> Result<NvmlTelemetry, NvmlUnavailable> {
      func symbol<T>(_ name: String, _: T.Type) -> T? {
        lookup(name).map { unsafeBitCast($0, to: T.self) }
      }
      guard let initialize = symbol("nvmlInit_v2", InitFn.self), let handle = symbol("nvmlDeviceGetHandleByIndex_v2", HandleFn.self) else {
        return .failure(NvmlUnavailable(reason: "libnvidia-ml.so.1 has no nvmlInit_v2"))
      }
      let status = initialize()
      guard status == success else { return .failure(NvmlUnavailable(reason: "nvmlInit_v2 failed with NVML error \(status)")) }
      var device: Device = nil
      let found = handle(UInt32(clamping: index), &device)
      guard found == success, device != nil else {
        return .failure(NvmlUnavailable(reason: "no NVML device \(index) (NVML error \(found))"))
      }
      var milliwatts: UInt32 = 0
      let limit = symbol("nvmlDeviceGetEnforcedPowerLimit", UIntFn.self).map { $0(device, &milliwatts) == success ? Double(milliwatts) / 1000 : 0 }
      let name = symbol("nvmlDeviceGetName", NameFn.self).flatMap { call in
        // NVML_DEVICE_NAME_V2_BUFFER_SIZE
        var buffer = [CChar](repeating: 0, count: 96)
        return buffer.withUnsafeMutableBufferPointer { text in
          call(device, text.baseAddress!, UInt32(text.count - 1)) == success ? String(cString: text.baseAddress!) : nil
        }
      }
      return .success(
        NvmlTelemetry(
          device: device, temperatureV: symbol("nvmlDeviceGetTemperatureV", TemperatureVFn.self),
          temperature: symbol("nvmlDeviceGetTemperature", WithKindFn.self), powerUsage: symbol("nvmlDeviceGetPowerUsage", UIntFn.self),
          utilization: symbol("nvmlDeviceGetUtilizationRates", UIntFn.self), clock: symbol("nvmlDeviceGetClockInfo", WithKindFn.self),
          fanSpeed: symbol("nvmlDeviceGetFanSpeed", UIntFn.self), powerLimitW: limit ?? 0, name: name))
    }

    public func read() -> [String: Any] {
      var util: [UInt32] = [0, 0]
      let busy = utilization.map { call in util.withUnsafeMutableBufferPointer { call(device, $0.baseAddress!) } == NvmlTelemetry.success } ?? false
      return [
        "temp_c": Double(gpuTemperature()),
        "power_w": pythonRound(Double(value(powerUsage)) / 1000, 2),
        "power_limit_w": powerLimitW,
        "gpu_load_pct": busy ? Int(util[0]) : 0,
        "gpu_clock_mhz": Int(value(clock, NvmlTelemetry.clockGraphics)),
        "fan_pct": Int(value(fanSpeed)),
      ]
    }

    /// nvmlDeviceGetTemperatureV where the driver has it: the older call is
    /// deprecated in CUDA 13 and answers NVML_ERROR_DEPRECATED from 14.
    private func gpuTemperature() -> Int {
      if let temperatureV {
        var reading: [Int32] = [NvmlTelemetry.temperatureV1, Int32(NvmlTelemetry.temperatureGPU), 0]
        if reading.withUnsafeMutableBufferPointer({ temperatureV(device, $0.baseAddress!) }) == NvmlTelemetry.success {
          return Int(reading[2])
        }
      }
      return Int(value(temperature, NvmlTelemetry.temperatureGPU))
    }

    private func value(_ call: UIntFn?) -> UInt32 {
      var out: UInt32 = 0
      guard let call, call(device, &out) == NvmlTelemetry.success else { return 0 }
      return out
    }

    private func value(_ call: WithKindFn?, _ kind: UInt32) -> UInt32 {
      var out: UInt32 = 0
      guard let call, call(device, kind, &out) == NvmlTelemetry.success else { return 0 }
      return out
    }
  }

  /// Why this host has no NVML telemetry, for the log.
  public struct NvmlUnavailable: Error, CustomStringConvertible {
    public let reason: String
    public var description: String { reason }
  }
#endif
