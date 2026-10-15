#if os(Linux)
  import Foundation
  import Glibc
  import JetlinkKit
  import JetlinkTestSupport
  import Testing

  @testable import JetlinkLinux
  @testable import JetlinkServer

  /// Descriptors as a usbfs node reads them back.
  enum Descriptors {
    static let device: [UInt8] = [18, 1, 0x20, 0x03, 0, 0, 0, 9, 0x09, 0x12, 0x01, 0x00, 0x00, 0x00, 1, 2, 0, 1]

    static func endpoint(_ address: UInt8, bulk: Bool = true) -> [UInt8] {
      // wMaxPacketSize 1024, and the SuperSpeed companion a 5 Gb/s link adds.
      [7, 5, address, bulk ? 0x02 : 0x03, 0x00, 0x04, 0] + [6, 0x30, 0, 0, 0, 0]
    }

    static func interface(_ number: UInt8, alternate: UInt8 = 0, kind: [UInt8] = [0xff, 0xff, 0xff], _ endpoints: [UInt8]...) -> [UInt8] {
      [9, 4, number, alternate, UInt8(endpoints.count)] + kind + [0] + endpoints.flatMap { $0 }
    }

    static func configuration(_ value: UInt8, _ interfaces: [UInt8]...) -> [UInt8] {
      let body = interfaces.flatMap { $0 }
      let total = 9 + body.count
      let count = UInt8(interfaces.count)
      return [9, 2, UInt8(total & 0xff), UInt8(total >> 8), count, value, 0, 0x80, 50] + body
    }

    /// The comma's jetlink gadget as the Jetson saw it: one FF/FF/FF
    /// interface with bulk 81 in and 01 out.
    static let comma = device + configuration(1, interface(0, endpoint(0x81), endpoint(0x01)))
  }

  /// usbdevfs where the node is a file holding descriptors: opens and reads
  /// are real, the ioctls are the test's.
  final class FakeNode: UsbfsNode, @unchecked Sendable {
    let calls = Locked<[String]>([])
    let bound = Locked<String?>(nil)
    let claimError = Locked<Int32>(0)
    let descriptorsOpen = Locked<Set<Int32>>([])

    func open(_ path: String) throws(KernelError) -> Int32 {
      let fd = try DevUsbfs().open(path)
      calls.value.append("open \(URL(fileURLWithPath: path).pathComponents.suffix(2).joined(separator: "/"))")
      descriptorsOpen.value.insert(fd)
      return fd
    }

    func descriptors(_ fd: Int32) throws(KernelError) -> [UInt8] {
      try DevUsbfs().descriptors(fd)
    }

    func driver(_ fd: Int32, interface: UInt8) throws(KernelError) -> String? {
      bound.value
    }

    func disconnect(_ fd: Int32, interface: UInt8) throws(KernelError) {
      calls.value.append("disconnect \(interface)")
      bound.value = nil
    }

    func claim(_ fd: Int32, interface: UInt8) throws(KernelError) {
      if claimError.value != 0 { throw KernelError(what: "claiming interface \(interface)", errno: claimError.value) }
      calls.value.append("claim \(interface)")
    }

    func release(_ fd: Int32, interface: UInt8) {
      calls.value.append("release \(interface)")
    }

    func close(_ fd: Int32) {
      calls.value.append("close")
      descriptorsOpen.value.remove(fd)
      DevUsbfs().close(fd)
    }
  }

  /// The server's end of the claim: what it was handed, and a device that
  /// goes when told, as ENODEV from a reap makes `UsbfsGadget`'s go.
  final class FakeTarget: UsbfsTarget, @unchecked Sendable {
    let attached = Locked<(fd: Int32, input: UInt8, output: UInt8)?>(nil)
    let events = Locked<[String]>([])
    let gone = Locked(false)
    let attachError = Locked<Bool>(false)

    func attach(fd: Int32, inEndpoint: UInt8, outEndpoint: UInt8) throws {
      if attachError.value { throw LinkError.closed("could not create the USB wake event") }
      attached.value = (fd, inEndpoint, outEndpoint)
      gone.value = false
      events.value.append(String(format: "attach %02x %02x", inEndpoint, outEndpoint))
    }

    func detach() {
      attached.value = nil
      events.value.append("detach")
    }

    func present() -> Bool {
      attached.value != nil && !gone.value
    }

    func open() throws -> any MessageLink {
      guard present() else { throw LinkError.closed("no jetlink gadget is attached") }
      return StubLink()
    }
  }

  final class StubLink: MessageLink {
    var peer: String { "usb" }
    var medium: LinkMedium? { nil }
    var connectsOnOpen: Bool { false }
    func recv() throws -> Message { throw LinkError.closed("stub") }
    func sendParts(_ type: Wire.Msg, seq: UInt32, parts: UnsafeBufferPointer<UnsafeRawBufferPointer>, flags: Wire.Flag) throws {}
    func shutdown() {}
    func close() {}
  }

  /// A USB bus in a temporary tree: sysfs entries and usbfs nodes.
  final class Bus {
    let tree = Tree()
    let node = FakeNode()
    let target = FakeTarget()
    let lines = Lines()
    var environment: [String: String] = [:]
    /// Whether a permit write reaches the device: not while the bus is
    /// suspended, when the kernel still reports success.
    let lpmTakes = Locked(true)
    /// What was written to the comma's usb3_lpm_permit, in order.
    let permits = Locked<[String]>([])
    /// How long each permit write takes: the hub's control transfers.
    let writeTakes = Locked<TimeInterval>(0)
    /// How long a session may go without a message.
    var quietAfter: TimeInterval = 30
    lazy var gadget = SysfsGadget(
      root: tree.root, node: node, target: target, environment: environment,
      write: { [tree, lpmTakes, permits, writeTakes] path, text throws(KernelError) in
        try Sysfs.write(path, text)
        guard path.hasSuffix("/usb3_lpm_permit") else { return }
        if writeTakes.value > 0 { Thread.sleep(forTimeInterval: writeTakes.value) }
        permits.value.append(text)
        // The kernel applies a permit to the port's child at once, if it has one.
        let child = "/sys/bus/usb/devices/2-1.3/power"
        guard lpmTakes.value, tree.read("\(child)/usb3_hardware_lpm_u1") != nil else { return }
        for state in ["u1", "u2"] {
          tree.write("\(child)/usb3_hardware_lpm_\(state)", text == "0" ? "disabled\n" : "enabled\n")
        }
      }, log: lines.log, quietAfter: quietAfter)

    /// The comma's port, which outlives the comma.
    static let port = "/sys/devices/usb2/2-1/2-1:1.0/2-1-port3/usb3_lpm_permit"

    init() {
      // A hub and its interface, which the scan passes over.
      device("2-1", vendor: "0bda", product: "0489", bus: 2, device: 2)
      tree.write("/sys/bus/usb/devices/2-1:1.0/bInterfaceClass", "09\n")
      // Its port 3, whose permit outlives the devices plugged into it.
      tree.write(Bus.port, "u1_u2\n")
    }

    func device(
      _ name: String, vendor: String, product: String, bus: Int, device: Int, speed: String = "5000", descriptors: [UInt8] = Descriptors.comma
    ) {
      let base = "/sys/bus/usb/devices/\(name)"
      tree.write("\(base)/idVendor", "\(vendor)\n")
      tree.write("\(base)/idProduct", "\(product)\n")
      tree.write("\(base)/busnum", "\(bus)\n")
      tree.write("\(base)/devnum", "\(device)\n")
      tree.write("\(base)/speed", "\(speed)\n")
      tree.write(String(format: "/dev/bus/usb/%03d/%03d", bus, device), bytes: descriptors)
    }

    /// The comma's gadget plugs in behind the hub; with `lpm`, as a USB 3
    /// device the kernel runs with U1 and U2 enabled.
    func plug(device number: Int = 3, speed: String = "5000", lpm: Bool = false, descriptors: [UInt8] = Descriptors.comma) {
      device("2-1.3", vendor: "1209", product: "0001", bus: 2, device: number, speed: speed, descriptors: descriptors)
      guard lpm else { return }
      tree.link("/sys/bus/usb/devices/2-1.3/port", to: tree.path("/sys/devices/usb2/2-1/2-1:1.0/2-1-port3"))
      tree.write("/sys/bus/usb/devices/2-1.3/power/usb3_hardware_lpm_u1", "enabled\n")
      tree.write("/sys/bus/usb/devices/2-1.3/power/usb3_hardware_lpm_u2", "enabled\n")
    }

    func unplug(device number: Int = 3) {
      tree.remove("/sys/bus/usb/devices/2-1.3")
      tree.remove(String(format: "/dev/bus/usb/002/%03d", number))
    }

    func lpm(_ state: String) -> String? {
      tree.read("/sys/bus/usb/devices/2-1.3/power/usb3_hardware_lpm_\(state)")
    }

    var permit: String? { tree.read(Bus.port) }

    /// Waits for the link power writes queued so far.
    func settle() {
      gadget.power.sync {}
    }

    /// A session over any link, as the server says it.
    func started() {
      gadget.sessionStarted()
      settle()
    }

    func ended() {
      gadget.sessionEnded()
      settle()
    }

    /// Plugged, claimed and waiting for the comma's first message.
    func claimed(lpm: Bool = true, speed: String = "5000") throws {
      plug(speed: speed, lpm: lpm)
      #expect(gadget.present())
      _ = try gadget.open()
      settle()
    }
  }

  @Suite("Sysfs gadget")
  struct SysfsGadgetTests {
    @Test("The journal line install.sh and jetlink status wait for is unchanged")
    func waitingLine() {
      #expect(Server.usbWaiting == "waiting for a jetlink gadget at 1209:0001")
    }

    @Test("Found in the bench Jetson's sysfs: 2-1.3, bus 2 device 3")
    func capture() {
      let gadget = SysfsGadget(root: jetson, node: FakeNode(), target: FakeTarget(), log: Lines().log)
      #expect(gadget.scan() == SysfsGadget.Found(name: "2-1.3", bus: 2, device: 3, speed: "5000"))
    }

    @Test("Nothing on the bus: not present, and nothing opened")
    func absent() {
      let bus = Bus()
      #expect(!bus.gadget.present())
      #expect(throws: LinkError.self) { try bus.gadget.open() }
      #expect(bus.node.calls.value.isEmpty)
    }

    @Test("First sight claims the vendor interface of the node sysfs names; the descriptor stays open across the comma's per-run reopens")
    func claims() throws {
      let bus = Bus()
      bus.plug()
      #expect(bus.gadget.present())
      // Presence alone opens nothing: a comma that cannot be claimed is still there.
      #expect(bus.node.calls.value.isEmpty)
      for _ in 0..<5 {
        _ = try bus.gadget.open()
        #expect(bus.gadget.present())
      }
      #expect(bus.node.calls.value == ["open 002/003", "claim 0"])
      #expect(bus.target.events.value == ["attach 81 01"])
      #expect(bus.lines.has(.info, "claimed the comma's gadget at 2-1.3"))
    }

    @Test("Gone from sysfs: detached, released, closed")
    func unplugged() throws {
      let bus = Bus()
      bus.plug()
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.unplug()
      #expect(!bus.gadget.present())
      #expect(bus.node.calls.value == ["open 002/003", "claim 0", "release 0", "close"])
      #expect(bus.target.events.value == ["attach 81 01", "detach"])
      #expect(bus.node.descriptorsOpen.value.isEmpty)
      #expect(bus.lines.has(.info, "released the comma's gadget at 2-1.3: it left the bus"))
    }

    @Test("Re-enumerated between polls: the old claim goes, the new device is claimed")
    func reenumerated() throws {
      let bus = Bus()
      bus.plug(device: 3)
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.unplug(device: 3)
      bus.plug(
        device: 7,
        descriptors: Descriptors.device + Descriptors.configuration(1, Descriptors.interface(0, Descriptors.endpoint(0x82), Descriptors.endpoint(0x03))))
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      #expect(bus.node.calls.value == ["open 002/003", "claim 0", "release 0", "close", "open 002/007", "claim 0"])
      // The endpoints are read afresh: FunctionFS numbers them at every bind.
      #expect(bus.target.events.value == ["attach 81 01", "detach", "attach 82 03"])
    }

    @Test("ENODEV under the transfers with sysfs not caught up yet: released, and claimed again when it opens")
    func goneUnderTransfers() throws {
      let bus = Bus()
      bus.plug()
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.target.gone.value = true
      #expect(bus.gadget.present())
      #expect(bus.target.events.value == ["attach 81 01", "detach"])
      _ = try bus.gadget.open()
      #expect(bus.node.calls.value == ["open 002/003", "claim 0", "release 0", "close", "open 002/003", "claim 0"])
    }

    @Test("A kernel driver on the interface is detached first")
    func drivers() throws {
      let bus = Bus()
      bus.plug()
      bus.node.bound.value = "cdc_ncm"
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      #expect(bus.node.calls.value == ["open 002/003", "disconnect 0", "claim 0"])
    }

    enum Refusal: String, CaseIterable {
      case unopenable, anotherProgram, claimBusy, attachFails
    }

    @Test("A claim that fails is thrown for the USB loop to log, lets go of what it took, and is tried again", arguments: Refusal.allCases)
    func refused(_ refusal: Refusal) throws {
      let bus = Bus()
      bus.plug()
      switch refusal {
      case .unopenable: bus.tree.remove("/dev/bus/usb/002/003")
      // another program's usbfs claim, taken, would break it mid-transfer
      case .anotherProgram: bus.node.bound.value = "usbfs"
      case .claimBusy: bus.node.claimError.value = EBUSY
      case .attachFails: bus.target.attachError.value = true
      }
      #expect(bus.gadget.present())
      let error = #expect(throws: (any Error).self) { try bus.gadget.open() }
      let calls: [Refusal: [String]] = [
        .unopenable: [], .anotherProgram: ["open 002/003", "close"], .claimBusy: ["open 002/003", "close"],
        .attachFails: ["open 002/003", "claim 0", "release 0", "close"],
      ]
      #expect(bus.node.calls.value == calls[refusal])
      #expect(bus.node.descriptorsOpen.value.isEmpty && bus.target.attached.value == nil)
      if refusal == .unopenable {
        #expect(String(describing: error!).hasSuffix("/dev/bus/usb/002/003: No such file or directory"))
        return
      }
      bus.node.bound.value = nil
      bus.node.claimError.value = 0
      bus.target.attachError.value = false
      _ = try bus.gadget.open()
      #expect(bus.target.events.value.last == "attach 81 01")
    }

    @Test("With the real UsbfsGadget behind it, the claim serves a USB link")
    func realGadget() throws {
      let bus = Bus()
      bus.plug()
      let usbfs = UsbfsGadget()
      let gadget = SysfsGadget(root: bus.tree.root, node: bus.node, target: usbfs, log: bus.lines.log)
      #expect(!usbfs.present())
      #expect(gadget.present())
      let link = try gadget.open()
      #expect(link is USBTransport)
      #expect(usbfs.present())
      link.close()
      bus.unplug()
      #expect(!gadget.present())
      #expect(!usbfs.present())
      #expect(bus.node.descriptorsOpen.value.isEmpty)
    }
  }

  @Suite("USB 3 link power management")
  struct LinkPowerManagementTests {
    @Test("Left alone at the claim; off for a session, read back, once; stock when it ends or the server stops, and not off after")
    func session() throws {
      let bus = Bus()
      try bus.claimed()
      #expect(bus.permits.value.isEmpty)
      bus.started()
      #expect(bus.permits.value == ["0"] && bus.permit == "0")
      #expect(bus.lpm("u1") == "disabled\n" && bus.lpm("u2") == "disabled\n")
      #expect(bus.lines.count("usb 3 link power management off for the comma's session") == 1)
      // The comma's per-run reopens change nothing.
      _ = try bus.gadget.open()
      #expect(bus.permits.value == ["0"])
      bus.ended()
      #expect(bus.permits.value == ["0", "u1_u2"] && bus.permit == "u1_u2")
      #expect(bus.lpm("u1") == "enabled\n" && bus.lpm("u2") == "enabled\n")
      #expect(bus.lines.count("usb 3 link power management back to stock for the comma's link") == 1)
      // A second end says nothing: there is nothing to put back.
      bus.ended()
      #expect(bus.permits.value == ["0", "u1_u2"])
      bus.started()
      bus.gadget.close()
      #expect(bus.permit == "u1_u2" && bus.lpm("u1") == "enabled\n")
      bus.started()
      #expect(bus.permits.value == ["0", "u1_u2", "0", "u1_u2"])
    }

    @Test("The comma re-enumerates mid-session: off again at its claim, stock through its port once it is gone, off for the next session")
    func replugged() throws {
      let bus = Bus()
      try bus.claimed()
      bus.started()
      bus.unplug()
      #expect(!bus.gadget.present())
      // It comes back with U1/U2 on, as after a write made while it was away.
      bus.plug(device: 8, lpm: true)
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.settle()
      #expect(bus.permits.value == ["0", "0"])
      #expect(bus.lpm("u1") == "disabled\n" && bus.lpm("u2") == "disabled\n")
      // Written to the port itself: the comma's `port` link left with it.
      bus.unplug(device: 8)
      bus.ended()
      #expect(bus.permit == "u1_u2")
      #expect(bus.lines.count("back to stock for the comma's link") == 1)
      bus.plug(device: 9, lpm: true)
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.started()
      #expect(bus.permits.value == ["0", "0", "u1_u2", "0"])
      #expect(bus.lpm("u1") == "disabled\n")
    }

    @Test("A write that does not take is a warning with what the device reads")
    func notTaken() throws {
      let bus = Bus()
      try bus.claimed()
      bus.lpmTakes.value = false
      bus.started()
      #expect(bus.lines.has(.warning, "is still on for the comma's session after writing 0"))
      #expect(bus.lines.has(.warning, "u1 enabled, u2 enabled"))
      #expect(bus.lines.count("off for the comma's session") == 0)
    }

    @Test("A port an earlier server left off goes back to stock at the claim")
    func leftOff() throws {
      let bus = Bus()
      bus.tree.write(Bus.port, "0\n")
      bus.plug(lpm: true)
      bus.tree.write("/sys/bus/usb/devices/2-1.3/power/usb3_hardware_lpm_u1", "disabled\n")
      bus.tree.write("/sys/bus/usb/devices/2-1.3/power/usb3_hardware_lpm_u2", "disabled\n")
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.settle()
      #expect(bus.permit == "u1_u2")
      #expect(bus.lpm("u1") == "enabled\n")
      #expect(bus.lines.has(.info, "back to stock for the comma's link (an earlier server left it off)"))
    }

    @Test("Nothing on a USB 2 link, for a device the kernel runs without LPM, or with no comma claimed")
    func skipped() throws {
      let usb2 = Bus()
      try usb2.claimed(speed: "480")
      usb2.started()
      usb2.ended()
      #expect(usb2.permits.value.isEmpty)

      let bare = Bus()
      try bare.claimed(lpm: false)
      bare.started()
      bare.ended()
      bare.gadget.close()
      #expect(bare.permits.value.isEmpty)
      #expect(bare.lines.count("link power management") == 0)

      // A comma over TCP, with no gadget on the bus.
      let tcp = Bus()
      tcp.started()
      tcp.ended()
      #expect(tcp.permits.value.isEmpty)
    }

    @Test("A session quiet for a while is back at stock until the comma talks, and the message that turns it off does not wait for the write")
    func quiet() throws {
      let bus = Bus()
      bus.quietAfter = 0.5
      try bus.claimed()
      bus.started()
      #expect(bus.permits.value == ["0"])
      // modeld exits at ignition off; the owner keeps the gadget, and so the session
      #expect(eventually { bus.permits.value == ["0", "u1_u2"] })
      bus.settle()
      #expect(bus.lpm("u1") == "enabled\n" && bus.lpm("u2") == "enabled\n")
      #expect(bus.lines.has(.info, "usb 3 link power management back to stock for the comma's link (nothing from the comma for 0.5 s)"))

      // The next message queues the write and goes on to be handled: here the
      // write takes half a second, as a hub's control transfers never should.
      bus.writeTakes.value = 0.5
      let started = DispatchTime.now().uptimeNanoseconds
      bus.gadget.sessionHeard()
      let heardMs = Double(DispatchTime.now().uptimeNanoseconds - started) / 1e6
      #expect(heardMs < 50, "the message waited \(heardMs) ms for sysfs")
      for _ in 0..<20 {
        let each = DispatchTime.now().uptimeNanoseconds
        bus.gadget.sessionHeard()
        #expect(Double(DispatchTime.now().uptimeNanoseconds - each) / 1e6 < 50)
      }
      bus.settle()
      bus.writeTakes.value = 0
      #expect(bus.permits.value == ["0", "u1_u2", "0"])
      #expect(bus.lpm("u1") == "disabled\n" && bus.lpm("u2") == "disabled\n")
      #expect(bus.lines.has(.info, "usb 3 link power management off for the comma's session (the comma is sending again)"))

      // A comma that keeps talking, as frames or its 10 s keep-alive do, keeps it off.
      for _ in 0..<10 {
        Thread.sleep(forTimeInterval: 0.1)
        bus.gadget.sessionHeard()
      }
      bus.settle()
      #expect(bus.permits.value == ["0", "u1_u2", "0"])

      // Quiet again, then the session ends: nothing is left to put back, and
      // no check runs after the end.
      #expect(eventually { bus.permits.value == ["0", "u1_u2", "0", "u1_u2"] })
      bus.settle()
      bus.ended()
      bus.gadget.sessionHeard()
      Thread.sleep(forTimeInterval: 0.7)
      bus.settle()
      #expect(bus.permits.value == ["0", "u1_u2", "0", "u1_u2"])
      #expect(bus.lines.count("back to stock") == 2 && bus.lines.count("off for the comma's session") == 2)
    }

    @Test("A quiet session's comma that re-enumerates comes back at stock, and off at its next message")
    func quietReplugged() throws {
      let bus = Bus()
      bus.quietAfter = 0.2
      try bus.claimed()
      bus.started()
      #expect(eventually { bus.permits.value == ["0", "u1_u2"] })
      bus.settle()
      bus.unplug()
      #expect(!bus.gadget.present())
      bus.plug(device: 8, lpm: true)
      #expect(bus.gadget.present())
      _ = try bus.gadget.open()
      bus.settle()
      #expect(bus.permits.value == ["0", "u1_u2"])
      #expect(bus.lpm("u1") == "enabled\n")
      bus.gadget.sessionHeard()
      bus.settle()
      #expect(bus.permits.value == ["0", "u1_u2", "0"])
      #expect(bus.lpm("u1") == "disabled\n")
      bus.gadget.close()
      #expect(bus.permit == "u1_u2")
    }

    @Test("JETLINK_USB_LPM=1 keeps it on for the session, putting back a port left off")
    func keptOn() throws {
      let bus = Bus()
      bus.environment = ["JETLINK_USB_LPM": "1"]
      bus.tree.write(Bus.port, "0\n")
      try bus.claimed()
      bus.started()
      #expect(bus.permits.value == ["u1_u2"])
      #expect(bus.lpm("u1") == "enabled\n" && bus.lpm("u2") == "enabled\n")
      #expect(bus.lines.has(.info, "usb 3 link power management left on for the comma's session (JETLINK_USB_LPM=1)"))
      bus.ended()
      bus.gadget.close()
      #expect(bus.permits.value == ["u1_u2"])
    }

    @Test("JETLINK_USB_LPM=1 through a quiet spell: nothing more written or said")
    func keptOnQuiet() throws {
      let bus = Bus()
      bus.environment = ["JETLINK_USB_LPM": "1"]
      bus.quietAfter = 0.2
      try bus.claimed()
      bus.started()
      Thread.sleep(forTimeInterval: 0.4)
      bus.gadget.sessionHeard()
      bus.settle()
      #expect(bus.permits.value == ["u1_u2"])
      #expect(bus.lines.count("link power management") == 1)
    }
  }

  @Suite("Gadget descriptors")
  struct GadgetDescriptorsTests {
    typealias D = Descriptors

    @Test("The comma's one interface: 81 in, 01 out")
    func comma() throws {
      #expect(try GadgetDescriptors.pick(D.comma) == GadgetDescriptors(interface: 0, inEndpoint: 0x81, outEndpoint: 0x01))
    }

    @Test("A composite gadget: the FF/FF/FF interface, wherever it is")
    func composite() throws {
      let ncm = D.interface(0, kind: [0x02, 0x0d, 0x00], D.endpoint(0x83, bulk: false))
      let data = D.interface(1, kind: [0x0a, 0x00, 0x01], D.endpoint(0x81), D.endpoint(0x01))
      let vendor = D.interface(2, D.endpoint(0x82), D.endpoint(0x02))
      #expect(
        try GadgetDescriptors.pick(D.device + D.configuration(1, ncm, data, vendor)) == GadgetDescriptors(interface: 2, inEndpoint: 0x82, outEndpoint: 0x02))
    }

    @Test("No vendor interface: interface 0; alternate settings and interrupt endpoints are passed over")
    func fallback() throws {
      let zero = D.interface(0, kind: [0x08, 0x06, 0x50], D.endpoint(0x85, bulk: false), D.endpoint(0x84), D.endpoint(0x04))
      let alternate = D.interface(1, alternate: 1, D.endpoint(0x86), D.endpoint(0x06))
      #expect(
        try GadgetDescriptors.pick(D.device + D.configuration(1, zero, alternate)) == GadgetDescriptors(interface: 0, inEndpoint: 0x84, outEndpoint: 0x04))
    }

    @Test("The active configuration, when sysfs names one")
    func configurations() throws {
      let first = D.configuration(1, D.interface(0, D.endpoint(0x81), D.endpoint(0x01)))
      let second = D.configuration(2, D.interface(0, D.endpoint(0x83), D.endpoint(0x03)))
      #expect(try GadgetDescriptors.pick(D.device + first + second).inEndpoint == 0x81)
      #expect(try GadgetDescriptors.pick(D.device + first + second, configuration: 2).inEndpoint == 0x83)
    }

    @Test("Short, truncated, or without a bulk pair: an error, never a crash")
    func malformed() {
      #expect(throws: LinkError.self) { try GadgetDescriptors.pick([]) }
      #expect(throws: LinkError.self) { try GadgetDescriptors.pick(Array(D.comma.prefix(30))) }
      #expect(throws: LinkError.self) { try GadgetDescriptors.pick(D.device + [0, 2, 0, 0]) }
      #expect(throws: LinkError.self) { try GadgetDescriptors.pick(D.device + D.configuration(1, D.interface(0, D.endpoint(0x81)))) }
    }

    @Test("Read back through jl_usbfs_descriptors from a node's bytes")
    func read() throws {
      let tree = Tree()
      tree.write("/node", bytes: D.comma)
      let node = DevUsbfs()
      let fd = try node.open(tree.path("/node"))
      defer { node.close(fd) }
      #expect(try node.descriptors(fd) == D.comma)
      // Again from the start, as a second claim on the same descriptor reads.
      #expect(try node.descriptors(fd) == D.comma)
      // Not a usbfs node: the ioctls say so with their errno.
      #expect(throws: KernelError.self) { try node.claim(fd, interface: 0) }
    }
  }
#endif
