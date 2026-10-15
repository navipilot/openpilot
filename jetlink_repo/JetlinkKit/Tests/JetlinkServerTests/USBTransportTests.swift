import Foundation
import JetlinkKit
import JetlinkTestSupport
import Testing

@testable import JetlinkServer

/// What the Mac sent, message by message: the host keeps the one-byte PADDED rule.
struct HostFrames {
  struct Frame {
    let type: UInt16
    let seq: UInt32
    let flags: UInt32
    let payload: Data
  }

  static func parse(_ data: Data) throws -> [Frame] {
    var frames: [Frame] = []
    var offset = 0
    while offset + Wire.headerSize <= data.count {
      let header = try data.withUnsafeBytes { try Wire.unpackHeader($0.baseAddress! + offset) }
      let pad = Wire.Flag(rawValue: header.flags).contains(.padded) ? 1 : 0
      let end = offset + Wire.headerSize + Int(header.length)
      guard end + pad <= data.count else { break }
      frames.append(Frame(type: header.msgType, seq: header.seq, flags: header.flags, payload: data.subdata(in: (offset + Wire.headerSize)..<end)))
      offset = end + pad
    }
    return frames
  }
}

/// The host's end of the framing, over usbfs pipes on a fake kernel.
@Suite("USB framing")
struct USBTransportTests {
  let kernel = FakeUsbfs()

  func transport() -> USBTransport {
    USBTransport(pipes: UsbfsPipes(device: UsbfsDevice(kernel: kernel), inEndpoint: 0x81, outEndpoint: 0x01))
  }

  @Test("What the host sends keeps the one-byte PADDED rule, in one transfer a message")
  func sendsHostFrames() throws {
    let transport = transport()
    let exact = Data(repeating: 1, count: Wire.packetMultiple - Wire.headerSize)
    try transport.send(.pong, seq: 3)
    try exact.withUnsafeBytes { try transport.send(.inferResp, seq: 4, parts: [$0]) }
    #expect(kernel.writes == 2)
    let frames = try HostFrames.parse(kernel.written)
    #expect(frames.count == 2)
    #expect(frames[0].type == Wire.Msg.pong.rawValue && frames[0].payload.isEmpty && frames[0].flags == 0)
    #expect(frames[1].payload == exact)
    #expect(Wire.Flag(rawValue: frames[1].flags).contains(.padded))
    #expect(kernel.written.count == Wire.headerSize + Wire.headerSize + exact.count + 1)
  }

  @Test("A message that fills whole high-speed packets is padded, so a USB 2 link ends it short too")
  func padsForHighSpeed() {
    for total in [512, 1024, 1536, 1024 * 5 + 512, 16384] {
      #expect(Wire.needsPad(total - Wire.headerSize), "\(total)")
    }
    for total in [Wire.headerSize, 511, 513, 1023, 1025, 1537] {
      #expect(!Wire.needsPad(total - Wire.headerSize), "\(total)")
    }
  }

  @Test("A write the bus takes in pieces still sends the message once, whole")
  func partialWrites() throws {
    kernel.writeLimit = 1000
    let transport = transport()
    let payload = Data((0..<74_000).map { UInt8(truncatingIfNeeded: $0) })
    try payload.withUnsafeBytes { try transport.send(.inferResp, seq: 9, parts: [$0]) }
    let frames = try HostFrames.parse(kernel.written)
    #expect(frames.count == 1)
    #expect(frames.first?.payload == payload)
    #expect(kernel.writes > 1)
  }

  @Test("A write that moves nothing is a lost link")
  func stalledWrite() {
    kernel.writeLimit = 0
    let transport = transport()
    #expect(throws: LinkError.self) { try transport.send(.pong, seq: 1) }
  }

  @Test("A message bigger than the receive buffer grows it")
  func growsForBigMessages() throws {
    let transport = transport()
    let payload = Data((0..<(4 << 20) + 8).map { UInt8(truncatingIfNeeded: $0 &* 7) })
    let reader = Background { () -> (Data, UInt16) in
      let big = Data(try transport.recv().payload)
      return (big, try transport.recv().msgType)
    }
    kernel.feed(FakeUsbfs.gadgetFrame(.uploadChunk, seq: 1, payload: payload))
    kernel.feed(FakeUsbfs.gadgetFrame(.ping, seq: 2))
    let (big, next) = try #require(reader.join(timeout: 30)).get()
    #expect(big == payload)
    #expect(next == Wire.Msg.ping.rawValue)
  }

  @Test("shutdown wakes a receive blocked on another thread")
  func shutdownWakesRecv() throws {
    let transport = transport()
    let reader = Background { try transport.recv() }
    Thread.sleep(forTimeInterval: 0.1)
    transport.shutdown()
    #expect(throws: LinkError.self) { try #require(reader.join()).get() }
  }
}

/// The comma's gadget for the server's USB loop: a queue of comma ends, one
/// per open, and a count of opens.
final class FakeGadget: GadgetSource, @unchecked Sendable {
  private let lock = NSLock()
  private var pending: [FakeUsbfs]
  private var opened = 0
  private let make: () -> FakeUsbfs

  init(_ ends: [FakeUsbfs] = [], then make: @escaping () -> FakeUsbfs = FakeUsbfs.unserved) {
    pending = ends
    self.make = make
  }

  var opens: Int { lock.withLock { opened } }

  func present() -> Bool { true }

  func open() throws -> any MessageLink {
    let end: FakeUsbfs = lock.withLock {
      opened += 1
      return pending.isEmpty ? make() : pending.removeFirst()
    }
    return USBTransport(pipes: UsbfsPipes(device: UsbfsDevice(kernel: end), inEndpoint: 0x81, outEndpoint: 0x01), medium: .usb3)
  }
}

/// The comma's gadget as the kernel shows one that left and came back: its
/// URBs die before its sysfs entry goes, so it stays present and the next
/// opens claim the stale device and fail. `script` is one comma end per open,
/// nil for an open that fails; an unserved end once it runs out.
final class StaleGadget: GadgetSource, @unchecked Sendable {
  private let lock = NSLock()
  private var script: [FakeUsbfs?]
  private var tries: [(at: TimeInterval, opened: Bool)] = []

  init(_ script: [FakeUsbfs?]) {
    self.script = script
  }

  /// Every open, when it came and whether it opened.
  var attempts: [(at: TimeInterval, opened: Bool)] { lock.withLock { tries } }

  func present() -> Bool { true }

  func open() throws -> any MessageLink {
    let end: FakeUsbfs? = lock.withLock {
      let next = script.isEmpty ? FakeUsbfs.unserved() : script.removeFirst()
      tries.append((ProcessInfo.processInfo.systemUptime, next != nil))
      return next
    }
    guard let end else { throw LinkError.closed("claiming the gadget: ENODEV: the device is gone") }
    return USBTransport(pipes: UsbfsPipes(device: UsbfsDevice(kernel: end), inEndpoint: 0x81, outEndpoint: 0x01), medium: .usb3)
  }
}

/// The comma's gadget on a fake usbfs descriptor: every open is a session's
/// pipes, and their read ring, on the one device.
final class UsbfsFakeGadget: GadgetSource, @unchecked Sendable {
  let kernel: FakeUsbfs
  let device: UsbfsDevice

  init() {
    kernel = FakeUsbfs()
    device = UsbfsDevice(kernel: kernel)
  }

  func present() -> Bool { !device.isGone }

  func open() throws -> any MessageLink {
    USBTransport(pipes: UsbfsPipes(device: device, inEndpoint: 0x81, outEndpoint: 0x01), medium: .usb3)
  }
}

/// A `FakeGadget` that also keeps what the server tells it about sessions.
final class ListeningGadget: GadgetSource, @unchecked Sendable {
  private let gadget: FakeGadget
  let heard = Recorded<String>()

  init(_ ends: [FakeUsbfs]) {
    gadget = FakeGadget(ends)
  }

  func present() -> Bool { gadget.present() }
  func open() throws -> any MessageLink { try gadget.open() }
  func sessionStarted() { heard.append("started") }
  func sessionEnded() { heard.append("ended") }
  func sessionHeard() { heard.append("message") }
}

/// The comma over a fake USB link.
final class GadgetClient: CommaClient {
  let pipes: FakeUsbfs
  private var seq: UInt32 = 0
  private var seen = 0

  init(_ pipes: FakeUsbfs) {
    self.pipes = pipes
  }

  /// Messages sent so far, each with the next seq.
  var sent: Int { Int(seq) }

  func sendMessage(_ type: Wire.Msg, _ payload: Data, flags: Wire.Flag, seq explicit: UInt32?) throws -> UInt32 {
    if explicit == nil { seq += 1 }
    let seq = explicit ?? self.seq
    pipes.feed(FakeUsbfs.gadgetFrame(type, seq: seq, payload: payload, flags: flags))
    return seq
  }

  /// The next message the server wrote to the pipes, waiting up to a minute.
  func recv() throws -> Reply {
    let deadline = Date().addingTimeInterval(60)
    while Date() < deadline {
      let frames = try HostFrames.parse(pipes.written)
      if seen < frames.count {
        let frame = frames[seen]
        seen += 1
        return Reply(type: frame.type, seq: frame.seq, payload: frame.payload)
      }
      _ = pipes.waitForWritten(pipes.written.count + 1, timeout: 0.2)
    }
    throw TestError("nothing from the server in 60 s")
  }
}

@Suite("Server over USB", .serialized)
struct ServerUSBTests {
  func makeServer(_ cache: TemporaryDirectory, gadget: any GadgetSource) throws -> Server {
    let server = try Server(
      configuration: Server.Configuration(host: "127.0.0.1", port: 0, cacheRoot: cache.url, preload: false, listen: false, usb: true),
      backend: cpuBackend())
    server.gadget = gadget
    return server
  }

  @Test("A gadget nothing on the comma serves is retried quietly, with no link events")
  func unservedGadget() throws {
    let cache = try TemporaryDirectory()
    let gadget = FakeGadget()
    let server = try makeServer(cache, gadget: gadget)
    let links = Recorded<LinkEvent>()
    server.host.subscribe { if case .link(let link) = $0 { links.append(link) } }
    try server.start()
    defer { server.shutdown() }
    #expect(server.port == nil, "a USB server opens no port")
    let deadline = Date().addingTimeInterval(10)
    while gadget.opens < 3 && Date() < deadline {
      Thread.sleep(forTimeInterval: 0.05)
    }
    #expect(gadget.opens >= 3)
    #expect(links.all.allSatisfy { $0.state == .waiting }, "\(links.all)")
  }

  @Test("The comma over USB is served as over TCP; the link is up on its first message and down when it goes")
  func servesOverUSB() throws {
    let cache = try TemporaryDirectory()
    let comma = FakeUsbfs()
    let gadget = FakeGadget([.unserved(), comma])
    let server = try makeServer(cache, gadget: gadget)
    let links = Recorded<LinkEvent>()
    server.host.subscribe { if case .link(let link) = $0 { links.append(link) } }
    try server.start()
    defer { server.shutdown() }
    let client = GadgetClient(comma)
    let hello = try client.hello(name: "modeld")
    #expect(hello["protocol"] as? Int == Int(Wire.version))
    try client.send(.ping)
    _ = try client.recv(.pong)
    #expect(links.all.contains { $0.state == .connected && $0.peer == "usb" && $0.linkMedium == .usb3 })
    comma.unplug()
    let deadline = Date().addingTimeInterval(5)
    while !links.all.contains(where: { $0.state == .disconnected }) && Date() < deadline {
      Thread.sleep(forTimeInterval: 0.05)
    }
    #expect(links.all.filter { $0.state == .connected }.count == 1)
    #expect(links.all.contains { $0.state == .disconnected })
  }

  /// Over USB the hello is the first message: the session is announced once,
  /// with the medium settled. install.sh waits for the "client connected" line.
  @Test("A USB session is announced once: the hello's speed, or the host's when the hello names none")
  func announcedOnce() throws {
    for (link, expected) in [(["kind": "usb"], LinkMedium.usb3), (["kind": "usb", "usb_speed": "high-speed"], .usb2)] {
      let cache = try TemporaryDirectory()
      let comma = FakeUsbfs()
      let server = try makeServer(cache, gadget: FakeGadget([comma]))
      let links = Recorded<LinkEvent>()
      server.host.subscribe { if case .link(let link) = $0 { links.append(link) } }
      try server.start()
      defer { server.shutdown() }
      _ = try GadgetClient(comma).hello(name: "modeld", link: link)
      let connected = links.all.filter { $0.state == .connected }
      #expect(connected.map(\.linkMedium) == [expected], "\(link)")
    }
  }

  /// Linux keeps USB 3 link power management off only while the comma talks.
  @Test("The gadget hears a session start, each message before it is answered, a finished build's push, and the end")
  func gadgetHearsTheSession() throws {
    let golden = try Golden("tiny_stateful")
    let cache = try TemporaryDirectory()
    let comma = FakeUsbfs()
    let gadget = ListeningGadget([comma])
    let server = try makeServer(cache, gadget: gadget)
    try server.start()
    defer { server.shutdown() }
    let client = GadgetClient(comma)
    _ = try client.hello(name: "modeld")
    #expect(gadget.heard.all == ["started", "message"])
    try client.send(.ping)
    _ = try client.recv(.pong)
    #expect(gadget.heard.all == ["started", "message", "message"])
    // a comma waiting on a build says nothing; the server's ready push counts
    _ = try client.ensureEngine(model: golden.model, sha256: golden.sha256)
    #expect(gadget.heard.all.filter { $0 == "message" }.count == client.sent + 1)
    comma.unplug()
    #expect(gadget.heard.wait { $0.last == "ended" })
    #expect(gadget.heard.all.filter { $0 == "started" || $0 == "ended" } == ["started", "ended"])
  }

  @Test("A comma that left and came back is opened again a poll after a stale open fails, not the quiet retry later")
  func rejoinsAfterAStaleOpen() throws {
    let cache = try TemporaryDirectory()
    let before = FakeUsbfs()
    let after = FakeUsbfs()
    let gadget = StaleGadget([before, nil, nil, after])
    let server = try makeServer(cache, gadget: gadget)
    try server.start()
    defer { server.shutdown() }
    _ = try GadgetClient(before).hello(name: "modeld")
    before.unplug()
    _ = try GadgetClient(after).hello(name: "modeld")
    let tries = gadget.attempts
    try #require(tries.count >= 4)
    #expect(tries.prefix(4).map { $0.opened } == [true, false, false, true])
    for failed in 1...2 {
      let gap = tries[failed + 1].at - tries[failed].at
      #expect(gap < Server.usbQuietRetry * 0.75, "the open after failure \(failed) came \(gap) s later")
    }
  }

  @Test("The golden frames through usbfs and its read ring, as a Jetson or a phone serves them", arguments: ["tiny_queued", "tiny_stateful"])
  func servesGoldenFramesOverUsbfs(_ name: String) throws {
    let golden = try Golden(name)
    let cache = try TemporaryDirectory()
    let gadget = UsbfsFakeGadget()
    gadget.kernel.shuffleReaps = true
    let server = try makeServer(cache, gadget: gadget)
    try server.start()
    defer { server.shutdown() }
    let (_, count) = try GadgetClient(gadget.kernel).replay(golden)
    #expect(eventually { server.framesServed == count })
    #expect(gadget.kernel.discards == 0, "the ring left the grid")
  }

  /// The comma's keep-alive between its join and the swap is a ping, over
  /// endpoints that outlive a server restart. Its next ping reaches a session
  /// with no hello and no model request, and must fail there, not at the swap.
  @Test("A comma still pinging a server that restarted under it is told to say hello again, and rejoins")
  func restartedServerRefusesThePing() throws {
    let golden = try Golden("tiny_stateful")
    let cache = try TemporaryDirectory()
    let gadget = UsbfsFakeGadget()
    let client = GadgetClient(gadget.kernel)
    let before = try makeServer(cache, gadget: gadget)
    try before.start()
    _ = try client.hello(name: "modeld")
    _ = try client.ensureEngine(model: golden.model, sha256: golden.sha256)
    try client.send(.ping)
    _ = try client.recv(.pong)
    before.shutdown()

    let after = try makeServer(cache, gadget: gadget)
    try after.start()
    defer { after.shutdown() }
    let ping = try client.send(.ping)
    let refused = try client.recv()
    #expect(refused.type == Wire.Msg.error.rawValue && refused.seq == ping, "\(refused.type)")
    #expect(refused.json["error"] as? String == "no_hello")
    _ = try client.hello(name: "modeld")
    _ = try client.ensureEngine(model: golden.model, sha256: golden.sha256)
    try client.send(.ping)
    _ = try client.recv(.pong)
  }
}
