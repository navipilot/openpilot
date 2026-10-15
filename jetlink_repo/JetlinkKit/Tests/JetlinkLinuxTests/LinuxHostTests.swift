#if os(Linux)
  import Foundation
  import JetlinkServer
  import Testing

  @testable import JetlinkLinux

  @Suite("Linux host")
  struct LinuxHostTests {
    func hooks(_ tree: Tree, sleepAfter: Double, lines: Lines = Lines()) -> ServerHooks {
      LinuxHost.hooks(sleepAfter: sleepAfter, poweroff: false, telemetry: nil, root: tree.root, log: lines.log)
    }

    @Test("A Jetson suspends when asked to, and the hello says so")
    func sleeps() {
      let tree = Tree.jetsonCopy()
      tree.makeDirectory("/run")
      let lines = Lines()
      let hooks = hooks(tree, sleepAfter: 900, lines: lines)
      #expect(hooks.sleepAfter == 900)
      #expect(hooks.gadgetIdle?(.present) == false)
      #expect(lines.has(.info, "will suspend after 900 s without a gadget"))
      #expect(FileManager.default.fileExists(atPath: tree.path(Sleeper.awakeLock)))
    }

    @Test("Without /sys/power/state, or with --sleep-after 0, the hello says it never sleeps")
    func neverSleeps() {
      let tree = Tree()
      let lines = Lines()
      let hooks = hooks(tree, sleepAfter: 900, lines: lines)
      #expect(hooks.sleepAfter == 0)
      #expect(hooks.gadgetIdle == nil)
      #expect(lines.has(.warning, "needs /sys/power/state"))
      #expect(self.hooks(Tree.jetsonCopy(), sleepAfter: 0).sleepAfter == 0)
    }

    @Test("The benchmark's thermal state comes from the GPU's temperature")
    func thermal() {
      let bench = LinuxHost.hooks(
        sleepAfter: 0, poweroff: false, telemetry: GPUTelemetry(read: { TegraTelemetry(root: jetson).read() }, name: nil, tegra: true), root: Tree().root)
      // tj-thermal reads 58.9 °C
      #expect(bench.thermal() == "nominal")
      for (celsius, state) in [(0.0, "unknown"), (79.9, "nominal"), (80, "fair"), (90, "serious"), (99, "critical")] {
        #expect(LinuxHost.thermal(GPUTelemetry(read: { ["temp_c": celsius] }, name: nil, tegra: true))() == state, "\(celsius)")
      }
      #expect(LinuxHost.thermal(nil)() == "unknown")
    }
  }

  @Suite("Platform")
  struct PlatformTests {
    @Test("The bench Jetson is a Tegra; any one sign says so; an empty host is not")
    func tegra() {
      #expect(Platform.isTegra(jetson))
      #expect(!Platform.isTegra(Tree().root))
      for sign in ["/proc/device-tree/compatible", "/etc/nv_tegra_release", "/sys/devices/platform/bus@0/17000000.gpu/load"] {
        let tree = Tree()
        tree.write(sign, sign.hasSuffix("compatible") ? "nvidia,p3768-0000\0nvidia,tegra234\0" : "")
        #expect(Platform.isTegra(tree.root), "\(sign)")
      }
      let pc = Tree()
      pc.write("/sys/firmware/devicetree/base/compatible", "linux,dummy-virt\0")
      #expect(!Platform.isTegra(pc.root))
    }

    @Test("MemAvailable in bytes, 0 when unknown")
    func memory() {
      #expect(Platform.memAvailableBytes(jetson) == 5_362_572 * 1024)
      #expect(Platform.meminfo(jetson)["SwapFree"] == 10_481_400 * 1024)
      #expect(Platform.memAvailableBytes(Tree().root) == 0)
    }

    @Test("The cache: the Jetson's partition, else root's, else XDG's")
    func cache() {
      let pc = Tree().root
      #expect(Platform.defaultCache(environment: [:], root: jetson, asRoot: true).path == "/mnt/data/jetlink")
      #expect(Platform.defaultCache(environment: ["XDG_CACHE_HOME": "/xdg"], root: pc, asRoot: true).path == "/var/lib/jetlink")
      #expect(Platform.defaultCache(environment: ["XDG_CACHE_HOME": "/xdg"], root: pc, asRoot: false).path == "/xdg/jetlink")
      #expect(Platform.defaultCache(environment: ["XDG_CACHE_HOME": ""], root: pc, asRoot: false).path.hasSuffix("/.cache/jetlink"))
    }

    @Test("Suspend needs /sys/power/state")
    func suspend() {
      #expect(Platform.canSuspend(jetson))
      #expect(!Platform.canSuspend(Tree().root))
    }

    @Test("Kernel files: trimmed text, integers, and nil for what cannot be read")
    func sysfs() throws {
      #expect(Sysfs.read(jetson.path("/sys/power/mem_sleep")) == "s2idle [deep]")
      #expect(Sysfs.readInt(jetson.path("/sys/class/hwmon/hwmon2/rpm")) == 2367)
      #expect(Sysfs.read(jetson.path("/sys/devices/virtual/thermal/thermal_zone2/temp")) == nil)
      #expect(Sysfs.readInt(jetson.path("/sys/power/mem_sleep")) == nil)
      let tree = Tree()
      // Past the first read's buffer, and every integer shape a kernel writes.
      let long = (0..<200).map { "line \($0)" }.joined(separator: "\n")
      tree.write("/long", "\t" + long + "\n\n")
      #expect(Sysfs.read(tree.path("/long")) == long)
      let cases: [(String, Int?)] = [
        ("-40000\n", -40000), ("0", 0), (" 17 \n", 17), ("", nil), ("-", nil), ("12a", nil), ("99999999999999999999", nil),
      ]
      for (text, value) in cases {
        tree.write("/int", text)
        #expect(Sysfs.readInt(tree.path("/int")) == value, "\(text)")
      }
      tree.write("/attr", "old value\n")
      try Sysfs.write(tree.path("/attr"), "new")
      #expect(tree.read("/attr") == "new")
      let error = #expect(throws: KernelError.self) { try Sysfs.write(tree.path("/missing"), "x") }
      #expect(error?.errno == ENOENT)
    }
  }
#endif
