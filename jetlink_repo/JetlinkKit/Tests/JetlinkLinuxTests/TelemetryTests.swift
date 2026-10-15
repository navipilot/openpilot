#if os(Linux)
  import Foundation
  import JetlinkServer
  import Testing

  @testable import JetlinkLinux

  @Suite("Tegra telemetry")
  struct TegraTelemetryTests {
    @Test("The bench Jetson's sysfs gives what the Python server sent")
    func capture() {
      let sample = TegraTelemetry(root: jetson).read()
      // tj-thermal 58937, soc0-thermal 57343, VDD_IN 4952 mV at 1744 mA,
      // GPU idle at 1020 MHz, the fan at 2367 rpm.
      #expect(sample["temp_c"] as? Double == 58.9)
      #expect(sample["memory_temp_c"] as? Double == 57.3)
      #expect(sample["power_w"] as? Double == 8.64)
      #expect(sample["power_limit_w"] as? Double == 25.0)
      #expect(sample["gpu_load_pct"] as? Int == 0)
      #expect(sample["gpu_clock_mhz"] as? Int == 1020)
      #expect(sample["fan_rpm"] as? Int == 2367)
      #expect(sample["supply_mv"] as? Int == 4952)
      #expect(sample["supply_ma"] as? Int == 1744)
      #expect(sample.count == 9)
    }

    @Test("The cv zones that cannot be read are listed without a temperature")
    func unreadableZones() {
      let zones = ThermalZone.all(jetson)
      #expect(
        zones.map(\.type) == [
          "cpu-thermal", "gpu-thermal", "cv0-thermal", "cv1-thermal", "cv2-thermal", "soc0-thermal", "soc1-thermal", "soc2-thermal", "tj-thermal",
        ])
      #expect(zones.filter { $0.celsius() == nil }.map(\.type) == ["cv0-thermal", "cv1-thermal", "cv2-thermal"])
      #expect(zones.first { $0.zone == "thermal_zone8" }?.celsius() == 58.937)
    }

    @Test("hwmon devices by name")
    func hwmons() {
      #expect(hwmon(named: "ina3221", jetson)?.hasSuffix("/sys/class/hwmon/hwmon4") == true)
      #expect(hwmon(named: "pwm_tach", jetson)?.hasSuffix("/sys/class/hwmon/hwmon2") == true)
      #expect(hwmon(named: "nothing", jetson) == nil)
    }

    @Test("Without tj or soc0 it falls back to gpu and cpu, as a zone reading 0 does")
    func fallbacks() {
      let tree = Tree.jetsonCopy()
      tree.remove("/sys/devices/virtual/thermal/thermal_zone8")
      tree.write("/sys/devices/virtual/thermal/thermal_zone5/temp", "0\n")
      let sample = TegraTelemetry(root: tree.root).read()
      #expect(sample["temp_c"] as? Double == 58.8)
      #expect(sample["memory_temp_c"] as? Double == 58.3)
    }

    @Test("A busy GPU, rounded down and held to 100")
    func load() {
      let tree = Tree.jetsonCopy()
      tree.write("/sys/devices/platform/bus@0/17000000.gpu/load", "999\n")
      tree.write("/sys/class/devfreq/17000000.gpu/cur_freq", "624750000\n")
      #expect(TegraTelemetry(root: tree.root).read()["gpu_load_pct"] as? Int == 99)
      #expect(TegraTelemetry(root: tree.root).read()["gpu_clock_mhz"] as? Int == 624)
      tree.write("/sys/devices/platform/bus@0/17000000.gpu/load", "1200\n")
      #expect(TegraTelemetry(root: tree.root).read()["gpu_load_pct"] as? Int == 100)
    }

    @Test("Every file missing reads zeros, with every key still there")
    func nothing() {
      let tree = Tree()
      let sample = TegraTelemetry(root: tree.root).read()
      #expect(sample.count == 9)
      #expect(sample["temp_c"] as? Double == 0)
      #expect(sample["power_w"] as? Double == 0)
      #expect(sample["fan_rpm"] as? Int == 0)
      #expect(sample["power_limit_w"] as? Double == 25)
    }
  }

  /// NVML as the driver's library answers, in C functions the table hands
  /// out: they cannot capture, so they read this.
  nonisolated(unsafe) var nvml = FakeNvml()

  struct FakeNvml {
    var initStatus: Int32 = 0
    var devices = 1
    var index: UInt32?
    var temperature: UInt32 = 61
    var temperatureV: Int32 = 63
    var temperatureVStatus: Int32 = 0
    var temperatureStatus: Int32 = 0
    var powerMilliwatts: UInt32 = 87_654
    var powerStatus: Int32 = 0
    var limitMilliwatts: UInt32 = 170_000
    var utilization: (gpu: UInt32, memory: UInt32) = (37, 12)
    var clock: UInt32 = 2505
    var fan: UInt32 = 44
    /// NVML_ERROR_NOT_SUPPORTED, as a laptop answers for its fan.
    var fanStatus: Int32 = 0
    var name = "NVIDIA GeForce RTX 4070"
    var versionSeen: Int32 = 0
    var missing: Set<String> = []
  }

  @Suite("NVML telemetry", .serialized)
  struct NvmlTelemetryTests {
    typealias Device = OpaquePointer?

    nonisolated(unsafe) static let table: [String: UnsafeMutableRawPointer] = [
      "nvmlInit_v2": unsafeBitCast(({ nvml.initStatus } as @convention(c) () -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetHandleByIndex_v2": unsafeBitCast(
        ({
          nvml.index = $0
          guard Int($0) < nvml.devices else { return 2 }
          $1.pointee = OpaquePointer(bitPattern: 0x1000 + Int($0))
          return 0
        } as @convention(c) (UInt32, UnsafeMutablePointer<Device>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetTemperatureV": unsafeBitCast(
        ({
          nvml.versionSeen = $1[0]
          guard nvml.temperatureVStatus == 0 else { return nvml.temperatureVStatus }
          $1[2] = nvml.temperatureV
          return 0
        } as @convention(c) (Device, UnsafeMutablePointer<Int32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetTemperature": unsafeBitCast(
        ({
          $2.pointee = nvml.temperature
          return nvml.temperatureStatus
        } as @convention(c) (Device, UInt32, UnsafeMutablePointer<UInt32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetPowerUsage": unsafeBitCast(
        ({
          $1.pointee = nvml.powerMilliwatts
          return nvml.powerStatus
        } as @convention(c) (Device, UnsafeMutablePointer<UInt32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetEnforcedPowerLimit": unsafeBitCast(
        ({
          $1.pointee = nvml.limitMilliwatts
          return 0
        } as @convention(c) (Device, UnsafeMutablePointer<UInt32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetUtilizationRates": unsafeBitCast(
        ({
          $1[0] = nvml.utilization.gpu
          $1[1] = nvml.utilization.memory
          return 0
        } as @convention(c) (Device, UnsafeMutablePointer<UInt32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetClockInfo": unsafeBitCast(
        ({
          // NVML_CLOCK_GRAPHICS
          guard $1 == 0 else { return 2 }
          $2.pointee = nvml.clock
          return 0
        } as @convention(c) (Device, UInt32, UnsafeMutablePointer<UInt32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetFanSpeed": unsafeBitCast(
        ({
          $1.pointee = nvml.fan
          return nvml.fanStatus
        } as @convention(c) (Device, UnsafeMutablePointer<UInt32>) -> Int32), to: UnsafeMutableRawPointer.self),
      "nvmlDeviceGetName": unsafeBitCast(
        ({
          let bytes = Array(nvml.name.utf8CString.prefix(Int($2)))
          for (offset, byte) in bytes.enumerated() { $1[offset] = byte }
          return 0
        } as @convention(c) (Device, UnsafeMutablePointer<CChar>, UInt32) -> Int32), to: UnsafeMutableRawPointer.self),
    ]

    func open(_ index: Int = 0) -> Result<NvmlTelemetry, NvmlUnavailable> {
      NvmlTelemetry.make(index: index) { nvml.missing.contains($0) ? nil : NvmlTelemetryTests.table[$0] }
    }

    init() {
      nvml = FakeNvml()
    }

    @Test("A desktop GPU's sample, in the Python keys")
    func sample() throws {
      let telemetry = try open().get()
      let sample = telemetry.read()
      #expect(sample["temp_c"] as? Double == 63)
      #expect(sample["power_w"] as? Double == 87.65)
      #expect(sample["power_limit_w"] as? Double == 170)
      #expect(sample["gpu_load_pct"] as? Int == 37)
      #expect(sample["gpu_clock_mhz"] as? Int == 2505)
      #expect(sample["fan_pct"] as? Int == 44)
      #expect(sample.count == 6)
      #expect(telemetry.name == "NVIDIA GeForce RTX 4070")
      #expect(nvml.versionSeen == 0x0100_000C)
    }

    @Test("The device index is --device's, not always 0")
    func index() throws {
      nvml.devices = 2
      _ = try open(1).get()
      #expect(nvml.index == 1)
      guard case .failure(let why) = open(2) else {
        Issue.record("device 2 of 2 opened")
        return
      }
      #expect(why.reason.contains("no NVML device 2"))
    }

    @Test("Without nvmlDeviceGetTemperatureV, or when it fails, the older call")
    func olderTemperature() throws {
      nvml.missing = ["nvmlDeviceGetTemperatureV"]
      #expect(try open().get().read()["temp_c"] as? Double == 61)
      nvml.missing = []
      // NVML_ERROR_ARGUMENT_VERSION_MISMATCH, from a driver older than the struct
      nvml.temperatureVStatus = 25
      #expect(try open().get().read()["temp_c"] as? Double == 61)
    }

    @Test("A query the GPU refuses reads 0 and the rest still count")
    func refused() throws {
      nvml.fanStatus = 3
      nvml.powerStatus = 3
      nvml.temperatureVStatus = 3
      nvml.temperatureStatus = 3
      nvml.missing = ["nvmlDeviceGetUtilizationRates"]
      let sample = try open().get().read()
      #expect(sample["fan_pct"] as? Int == 0)
      #expect(sample["power_w"] as? Double == 0)
      #expect(sample["temp_c"] as? Double == 0)
      #expect(sample["gpu_load_pct"] as? Int == 0)
      #expect(sample["gpu_clock_mhz"] as? Int == 2505)
    }

    @Test("No NVML: no init, or an init that fails")
    func unavailable() {
      nvml.missing = ["nvmlInit_v2"]
      #expect((try? open().get()) == nil)
      nvml.missing = []
      // NVML_ERROR_DRIVER_NOT_LOADED
      nvml.initStatus = 9
      guard case .failure(let why) = open() else {
        Issue.record("opened with a failing init")
        return
      }
      #expect(why.reason.contains("nvmlInit_v2 failed"))
    }

    @Test("A Tegra takes its sysfs and never asks NVML; a PC asks NVML for --device's GPU")
    func picks() {
      let asked = Locked<[Int]>([])
      let fake: (Int) -> Result<NvmlTelemetry, NvmlUnavailable> = { index in
        asked.value.append(index)
        return self.open(index)
      }
      let lines = Lines()
      let tegra = LinuxHost.telemetry(tegra: true, root: jetson, gpu: 0, nvml: fake, log: lines.log)
      #expect(tegra?.read()["supply_mv"] as? Int == 4952)
      #expect(tegra?.tegra == true && tegra?.name == nil)
      #expect(asked.value.isEmpty)
      nvml.devices = 3
      let pc = LinuxHost.telemetry(tegra: false, root: jetson, gpu: 2, nvml: fake, log: lines.log)
      #expect(pc?.read()["fan_pct"] as? Int == 44)
      #expect(pc?.tegra == false && pc?.name == "NVIDIA GeForce RTX 4070")
      #expect(asked.value == [2])
      nvml.initStatus = 9
      #expect(LinuxHost.telemetry(tegra: false, root: jetson, gpu: 0, nvml: fake, log: lines.log) == nil)
      #expect(lines.count("no telemetry on this host") == 1)
    }
  }
#endif
