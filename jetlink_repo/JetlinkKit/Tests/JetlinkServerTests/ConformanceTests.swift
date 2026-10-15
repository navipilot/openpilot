import Foundation
import JetlinkKit
import JetlinkORT
import JetlinkTestSupport
import Testing

@testable import JetlinkServer

#if canImport(Glibc)
  import Glibc
#elseif canImport(Android)
  import Android
#endif

/// What the Python does, as JetlinkKit/Scripts/make_conformance_fixtures.py
/// wrote it into Fixtures/conformance. docs/conformance.md has the story.
enum Conformance {
  static func url(_ name: String) -> URL { Fixture.url("conformance/\(name)") }
  static func data(_ name: String) throws -> Data { try Data(contentsOf: url(name)) }
  static func json(_ name: String) throws -> [String: Any] {
    try JSONSerialization.jsonObject(with: data(name)) as! [String: Any]
  }
}

func int(_ value: Any?) -> Int { (value as! NSNumber).intValue }
func double(_ value: Any?) -> Double { (value as! NSNumber).doubleValue }

/// "HELLO_REQ" as Swift spells a case: "helloReq".
func camel(_ name: String) -> String {
  let words = name.lowercased().split(separator: "_")
  return words.enumerated().map { $0.offset == 0 ? String($0.element) : $0.element.prefix(1).uppercased() + $0.element.dropFirst() }.joined()
}

/// A message of wire.json, with its payload made the way the generator made it.
struct WireMessage {
  let type: Wire.Msg
  let seq: UInt32
  let flags: Wire.Flag
  let parts: [Data]
  var payload: Data { parts.reduce(Data(), +) }

  init(_ d: [String: Any]) {
    type = Wire.Msg(rawValue: UInt16(int(d["type"])))!
    seq = UInt32(int(d["seq"]))
    flags = Wire.Flag(rawValue: UInt32(int(d["flags"])))
    if let json = d["json"] as? String {
      parts = [Data(json.utf8)]
    } else {
      let lengths = (d["parts"] as! [NSNumber]).map(\.intValue)
      let seq = Int(seq)
      // In steps: as one expression Linux's type checker gives up on it.
      let total: Int = lengths.reduce(0, +)
      var bytes = [UInt8](repeating: 0, count: total)
      for i in 0..<total {
        bytes[i] = UInt8((seq * 31 + i * 7) % 251)
      }
      let body = Data(bytes)
      var at = 0
      var parts: [Data] = []
      for n in lengths {
        parts.append(body.subdata(in: at..<at + n))
        at += n
      }
      self.parts = parts
    }
  }

  static func all() throws -> [WireMessage] {
    (try Conformance.json("wire.json")["messages"] as! [[String: Any]]).map(WireMessage.init)
  }

  func send(over link: any MessageLink) throws {
    try link.send(type, seq: seq, data: parts, flags: flags)
  }

  func matches(_ message: Message) -> Bool {
    message.msgType == type.rawValue && message.seq == seq && Data(message.payload) == payload
  }
}

@Suite("Conformance: the constants pinned to the Python")
struct PinnedConstantTests {
  @Test("The wire's constants are protocol.py's")
  func wire() {
    #expect(Wire.magic == Pinned.magic)
    #expect(Wire.version == Pinned.protocolVersion)
    #expect(Wire.headerSize == Pinned.headerSize)
    #expect(Wire.packetMultiple == Pinned.packetMultiple)
    #expect(Wire.gadgetTxAlign == Pinned.gadgetTxAlign)
    #expect(Wire.maxMessage == Pinned.maxMessage)
    #expect(Wire.inferReqSize == Pinned.inferReqSize)
    #expect(Wire.inferRespSize == Pinned.inferRespSize)
    #expect(LeaveReason.allCases.map(\.rawValue) == Pinned.leaveReasons)
    #expect(Wire.defaultPort == Pinned.defaultPort)
  }

  @Test("Every message, flag and status has Python's number and name, and no more")
  func numbering() {
    for (name, value) in Pinned.messageTypes {
      let message = Wire.Msg(rawValue: value)
      #expect(message.map { String(describing: $0) } == camel(name), "message \(name) = \(value)")
    }
    #expect((0...UInt16(255)).compactMap(Wire.Msg.init(rawValue:)).count == Pinned.messageTypes.count)
    let flags: [String: Wire.Flag] = ["RESET_QUEUES": .resetQueues, "WANT_STATE": .wantState, "WANT_HIDDEN": .wantHidden, "PADDED": .padded]
    #expect(Set(flags.keys) == Set(Pinned.flags.map(\.name)))
    for (name, value) in Pinned.flags {
      #expect(flags[name]?.rawValue == value, "flag \(name)")
    }
    for (name, value) in Pinned.statuses {
      #expect(Wire.Status(rawValue: value).map { String(describing: $0) } == camel(name), "status \(name) = \(value)")
    }
    #expect((0...UInt32(255)).compactMap(Wire.Status.init(rawValue:)).count == Pinned.statuses.count)
  }

  @Test("The model constants are the Python's")
  func model() {
    #expect(ModelConstants.runFrequency == Pinned.modelRunFrequency)
    #expect(ModelConstants.contextFrequency == Pinned.modelContextFrequency)
    #expect(ModelConstants.defaultFrameSkip == Pinned.defaultFrameSkip)
    #expect(ModelConstants.chunk == Pinned.uploadChunk)
  }

  @Test("Builds run on the onnxruntime the fixtures pin")
  func onnxruntime() {
    #expect(OrtRuntime.version == Pinned.onnxruntimeVersion)
  }
}

@Suite("Conformance: wire bytes against protocol.py and StreamTransport")
struct WireConformanceTests {
  @Test("Headers pack and unpack as protocol.pack_header does")
  func headers() throws {
    for h in try Conformance.json("wire.json")["headers"] as! [[String: Any]] {
      let header = Wire.Header(
        msgType: UInt16(int(h["msg_type"])), seq: UInt32(int(h["seq"])), flags: UInt32(int(h["flags"])), length: UInt32(int(h["length"])),
        reserved: (h["reserved"] as! NSNumber).uint64Value)
      var packed = [UInt8](repeating: 0xEE, count: Wire.headerSize)
      packed.withUnsafeMutableBytes { Wire.packHeader(header, into: $0.baseAddress!) }
      #expect(packed == hex(h["hex"] as! String), "\(h["name"]!)")
      let read = try hex(h["hex"] as! String).withUnsafeBytes { try Wire.unpackHeader($0.baseAddress!) }
      #expect(read == header, "\(h["name"]!)")
    }
  }

  @Test("INFER bodies are the bytes protocol.py packs")
  func inferBodies() throws {
    let wire = try Conformance.json("wire.json")
    for r in wire["infer_req"] as! [[String: Any]] {
      let raw = hex(r["hex"] as! String)
      raw.withUnsafeBytes {
        #expect(UInt32(littleEndian: $0.loadUnaligned(as: UInt32.self)) == UInt32(int(r["frame_id"])))
        #expect(UInt32(littleEndian: $0.loadUnaligned(fromByteOffset: 4, as: UInt32.self)) == UInt32(int(r["flags"])))
      }
    }
    for r in wire["infer_resp"] as! [[String: Any]] {
      let packed = Wire.inferResp(
        frameID: UInt32(int(r["frame_id"])), status: Wire.Status(rawValue: UInt32(int(r["status"])))!, gpuUs: UInt32(int(r["gpu_us"])),
        queueUs: UInt32(int(r["queue_us"])), totalUs: UInt32(int(r["total_us"])))
      #expect(packed == hex(r["hex"] as! String))
    }
  }

  /// A connected pair of stream sockets: `a` for the transport, `b` for the test.
  private func socketPair() throws -> (Int32, Int32) {
    var fds: [Int32] = [0, 0]
    guard socketpair(AF_UNIX, Sys.stream, 0, &fds) == 0 else { throw TestError("socketpair failed") }
    return (fds[0], fds[1])
  }

  @Test("TCP sends what TcpTransport sends")
  func tcpSends() throws {
    let expected = try Conformance.data("wire.tcp.bin")
    let (a, b) = try socketPair()
    let transport = TCPTransport(fd: a, peer: "fixture")
    let reader = Thread.detachNewThreadAndCollect(fd: b, count: expected.count)
    for message in try WireMessage.all() {
      try message.send(over: transport)
    }
    let got = reader.wait()
    transport.close()
    close(b)
    #expect(got == expected)
  }

  @Test("TCP reads what TcpTransport sends")
  func tcpReads() throws {
    let stream = try Conformance.data("wire.tcp.bin")
    let (a, b) = try socketPair()
    let transport = TCPTransport(fd: a, peer: "fixture")
    let writer = Thread {
      stream.withUnsafeBytes { raw in
        var at = 0
        while at < raw.count {
          let n = write(b, raw.baseAddress! + at, raw.count - at)
          if n <= 0 { break }
          at += n
        }
      }
    }
    writer.start()
    for message in try WireMessage.all() {
      let got = try transport.recv()
      #expect(message.matches(got), "\(message.type) seq \(message.seq)")
    }
    transport.close()
    close(b)
  }

  @Test("A USB host sends what UsbBulkTransport sends")
  func usbHostSends() throws {
    let kernel = FakeUsbfs()
    let transport = USBTransport(pipes: UsbfsPipes(device: UsbfsDevice(kernel: kernel), inEndpoint: 0x81, outEndpoint: 0x01))
    for message in try WireMessage.all() {
      try message.send(over: transport)
    }
    #expect(kernel.written == (try Conformance.data("wire.usb_host.bin")))
  }

  @Test("A USB host reads the gadget's padded stream")
  func usbHostReads() throws {
    let kernel = FakeUsbfs()
    kernel.feed([UInt8](try Conformance.data("wire.usb_gadget.bin")))
    let transport = USBTransport(pipes: UsbfsPipes(device: UsbfsDevice(kernel: kernel), inEndpoint: 0x81, outEndpoint: 0x01))
    for message in try WireMessage.all() {
      let got = try transport.recv()
      #expect(message.matches(got), "\(message.type) seq \(message.seq)")
    }
    #expect(kernel.buffered == 0)
  }
}

extension Thread {
  /// Reads `count` bytes from `fd` on a thread of its own; `wait()` returns them.
  static func detachNewThreadAndCollect(fd: Int32, count: Int) -> Collector {
    let collector = Collector()
    let thread = Thread {
      var out = Data()
      var buffer = [UInt8](repeating: 0, count: 1 << 16)
      while out.count < count {
        let n = buffer.withUnsafeMutableBytes { read(fd, $0.baseAddress!, $0.count) }
        if n <= 0 { break }
        out.append(contentsOf: buffer[0..<n])
      }
      collector.finish(out)
    }
    thread.start()
    return collector
  }

  final class Collector: @unchecked Sendable {
    private let condition = NSCondition()
    private var data: Data?

    func finish(_ value: Data) {
      condition.lock()
      data = value
      condition.broadcast()
      condition.unlock()
    }

    func wait() -> Data {
      condition.lock()
      defer { condition.unlock() }
      while data == nil { condition.wait() }
      return data!
    }
  }
}

@Suite("Conformance: reply layout")
struct LayoutConformanceTests {
  /// A reply the two ends size differently is refused on the comma, every
  /// frame: the hidden_state slice must resolve alike on both.
  @Test("hidden_state's ends resolve as the comma's spec resolves them, and the queues take what Python's take")
  func hiddenState() throws {
    let fixture = try Conformance.json("layout.json")
    let cases = fixture["cases"] as! [[String: Any]]
    #expect(cases.count == 12)
    for entry in cases {
      var d = fixture["spec"] as! [String: Any]
      var slices: [String: Any] = ["plan": [0, 16]]
      if let bounds = entry["hidden_state"] as? [Any] { slices["hidden_state"] = bounds }
      d["output_slices"] = slices
      let spec = try ModelSpec.from(d)
      let said = "\(entry["hidden_state"] ?? "none")"
      let expected = (entry["hidden_range"] as? [NSNumber]).map { $0[0].intValue..<$0[1].intValue }
      #expect(spec.hiddenRange == expected, "\(said)")
      #expect(spec.replyCount == int(entry["reply_nelem"]), "\(said)")
      #expect(spec.inferRespBytes == int(entry["infer_resp_nbytes"]), "\(said)")
      let engine = StagingEngine(spec.inputShapes.map { TensorSpec(name: $0.name, type: .float16, shape: $0.shape) })
      let queues = try? PolicyQueues(spec: spec, engine: engine)
      #expect((queues != nil) == (entry["feeds_back"] as! Bool), "\(said)")
    }
  }
}

/// An engine that is only its input buffers, for staging without a runtime.
final class StagingEngine: Engine {
  let inputs: [String: TensorSpec]
  let outputs: [String: TensorSpec] = [:]
  let lastGpuUs: UInt32 = 0
  private var buffers: [String: UnsafeMutableRawPointer] = [:]

  init(_ specs: [TensorSpec]) {
    inputs = Dictionary(uniqueKeysWithValues: specs.map { ($0.name, $0) })
    for spec in specs {
      let buffer = UnsafeMutableRawPointer.allocate(byteCount: spec.byteCount, alignment: 64)
      buffer.initializeMemory(as: UInt8.self, repeating: 0xAB, count: spec.byteCount)
      buffers[spec.name] = buffer
    }
  }

  deinit {
    for buffer in buffers.values { buffer.deallocate() }
  }

  func hostInput(_ name: String) -> UnsafeMutableRawPointer? { buffers[name] }
  func output(_ name: String) -> UnsafeRawPointer? { nil }
  func loopState(_ pairs: [(input: String, output: String)]) throws {}
  func resetState() {}
  func run() throws {}
  func warm() throws -> String { "" }
  func close() {}
}

/// One case of the staging fixture: frames as the comma sends them, each with
/// the output its run returned, and the tensors Python's queues fed for them.
struct StagingCase {
  let spec: ModelSpec
  let inputs: [TensorSpec]
  let frames: Data
  let staged: Data
  let count: Int
  let resetBefore: Int
  let helloBefore: Int

  init(frameSkip: Int, type: (String) -> ElementType = { _ in .float16 }) throws {
    let manifest = try Conformance.json("staging.json")
    let entry = try #require((manifest["cases"] as! [[String: Any]]).first { int($0["frame_skip"]) == frameSkip })
    spec = try ModelSpec.from(Conformance.json(entry["spec"] as! String))
    inputs = (entry["inputs"] as! [[String: Any]]).map {
      let name = $0["name"] as! String
      return TensorSpec(name: name, type: type(name), shape: ($0["shape"] as! [NSNumber]).map(\.intValue))
    }
    frames = try Conformance.data(entry["frames"] as! String)
    staged = try Conformance.data(entry["staged"] as! String)
    count = int(manifest["frames"])
    resetBefore = int(manifest["reset_before"])
    helloBefore = int(manifest["hello_before"])
  }

  /// The frame: warped, packed, then the output its run returned.
  var frameBytes: Int { spec.warpedBytes + spec.packedBytes + spec.outputCount * 4 }
  var stagedBytes: Int { inputs.reduce(0) { $0 + $1.byteCount } }

  /// Frame `frame` as the session takes it: the reset or the hello before
  /// it, staged from `request` (the frame copied there, aligned as the
  /// caller likes), and its output kept for the next when it is all finite.
  func stage(_ frame: Int, into staging: any FrameStaging, request: UnsafeMutableRawPointer) throws {
    if frame == resetBefore { staging.reset() }
    if frame == helloBefore { staging.newClient() }
    frames.withUnsafeBytes { request.copyMemory(from: $0.baseAddress! + frame * frameBytes, byteCount: frameBytes) }
    try staging.stage(warped: request, packed: request + spec.warpedBytes)
    let output = UnsafeMutablePointer<Float>.allocate(capacity: spec.outputCount)
    defer { output.deallocate() }
    UnsafeMutableRawPointer(output).copyMemory(from: request + spec.warpedBytes + spec.packedBytes, byteCount: spec.outputCount * 4)
    if Convert.allFinite(output, count: spec.outputCount) {
      staging.keep(outputs: output, type: .float)
    }
  }

  /// Each input `engine` holds against what Python staged for `frame`.
  func check(_ frame: Int, _ engine: StagingEngine, _ label: String) {
    var offset = frame * stagedBytes
    for input in inputs {
      let got = Data(bytes: engine.hostInput(input.name)!, count: input.byteCount)
      #expect(got == staged.subdata(in: offset..<offset + input.byteCount), "frame \(frame) \(input.name) \(label)")
      offset += input.byteCount
    }
  }
}

@Suite("Conformance: queue staging against queues.PolicyQueues")
struct StagingConformanceTests {
  /// The hidden state each frame returned is fed into the next, as modeld
  /// fed it back through prev_feat: the generator checks Python's queues
  /// against protocol 2's staging, and this the Swift against Python's. The
  /// frame sits one byte off alignment, as nothing in the receive buffer
  /// promises the packed floats theirs.
  @Test("Each frame stages the tensors Python's queues feed", arguments: [1, 2, 4])
  func staging(frameSkip: Int) throws {
    let fixture = try StagingCase(frameSkip: frameSkip)
    #expect(fixture.spec.frameSkip == frameSkip)
    #expect(fixture.frames.count == fixture.count * fixture.frameBytes)
    #expect(fixture.staged.count == fixture.count * fixture.stagedBytes)
    let engine = StagingEngine(fixture.inputs)
    let staging = try PolicyQueues(spec: fixture.spec, engine: engine)
    let request = UnsafeMutableRawPointer.allocate(byteCount: fixture.frameBytes + 1, alignment: 16) + 1
    defer { (request - 1).deallocate() }
    for frame in 0..<fixture.count {
      try fixture.stage(frame, into: staging, request: request)
      fixture.check(frame, engine, "at frame_skip \(frameSkip), one byte off")
    }
  }
}

@Suite("Conformance: frame statistics against session.FrameStats")
struct StatsConformanceTests {
  @Test("The stats event is the one FrameStats.summary makes")
  func summaries() throws {
    let fixture = try Conformance.json("stats.json")
    #expect(int(fixture["slow_frame_us"]) == Int(FrameStats.slowUs))
    for c in fixture["cases"] as! [[String: Any]] {
      let name = c["name"] as! String
      let clock = Clock()
      let stats = FrameStats(now: clock.read)
      for sample in c["samples"] as! [[NSNumber]] {
        clock.value = sample[0].doubleValue
        stats.record(totalUs: sample[1].uint32Value, gpuUs: sample[2].uint32Value, queueUs: sample[3].uint32Value, sendUs: sample[4].uint32Value)
      }
      clock.value = double(c["now"])
      let summary = stats.summary(window: double(c["window"]), framesTotal: int(c["frames_total"]))
      guard let expected = c["expected"] as? [String: Any] else {
        #expect(summary == nil, "\(name)")
        continue
      }
      let got = try #require(summary, "\(name)")
      let served = expected["served_ms"] as! [String: Any]
      let stages = expected["stages_ms"] as! [String: Any]
      #expect(got.frames == int(expected["frames"]), "\(name)")
      #expect(got.fps == double(expected["fps"]), "\(name) fps")
      #expect(got.servedMs == StatsEvent.Total(mean: double(served["mean"]), p99: double(served["p99"]), max: double(served["max"])), "\(name) served")
      let wantStages = StatsEvent.Stages(
        queue: double(stages["queue"]), gpu: double(stages["gpu"]), other: double(stages["other"]), send: double(stages["send"]))
      #expect(got.stagesMs == wantStages, "\(name) stages")
      #expect(got.slow == int(expected["slow"]), "\(name) slow")
      #expect(got.windowS == double(expected["window_s"]), "\(name) window")
      let total = expected["total_ms"] as! [String: Any]
      #expect(got.totalMs == StatsEvent.Total(mean: double(total["mean"]), p99: double(total["p99"]), max: double(total["max"])), "\(name) total")
      #expect(got.gpuMs == StatsEvent.Mean(mean: double((expected["gpu_ms"] as! [String: Any])["mean"])), "\(name) gpu")
    }
  }

  final class Clock: @unchecked Sendable {
    var value: TimeInterval = 0
    var read: @Sendable () -> TimeInterval { { [unowned self] in self.value } }
  }
}
