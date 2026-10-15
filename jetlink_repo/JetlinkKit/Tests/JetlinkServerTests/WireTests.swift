import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkServer

@Suite("Wire protocol")
struct WireTests {
  @Test("A closed listener frees its port at once, even with a thread waiting to accept")
  func listenerFreesItsPort() throws {
    let first = try TCPListener(host: "127.0.0.1", port: 0)
    let port = first.port
    let waiting = Thread { _ = first.accept() }
    waiting.start()
    Thread.sleep(forTimeInterval: 0.05)  // the accept is in its poll now
    first.close()
    // what a restart does next
    let second = try TCPListener(host: "127.0.0.1", port: port)
    second.close()
  }

  /// protocol.pack_header(INFER_REQ, 7, 1234, WANT_STATE | PADDED), from Python.
  @Test("A header is the bytes Python packs")
  func headerBytes() throws {
    var bytes = [UInt8](repeating: 0xFF, count: Wire.headerSize)
    bytes.withUnsafeMutableBytes {
      Wire.packHeader(
        Wire.Header(msgType: Wire.Msg.inferReq.rawValue, seq: 7, flags: (Wire.Flag.wantState.union(.padded)).rawValue, length: 1234), into: $0.baseAddress!)
    }
    #expect(hexString(bytes) == "4a4c4e4b030008000700000082000000d2040000000000000000000000000000")
    let header = try bytes.withUnsafeBytes { try Wire.unpackHeader($0.baseAddress!) }
    #expect(header.msgType == Wire.Msg.inferReq.rawValue)
    #expect(header.seq == 7)
    #expect(header.length == 1234)
  }

  /// protocol.pack_infer_resp(42, NOT_FINITE, 28000, 130, 29000), from Python.
  @Test("An inference reply is the bytes Python packs")
  func inferRespBytes() {
    #expect(
      hexString(Wire.inferResp(frameID: 42, status: .notFinite, gpuUs: 28000, queueUs: 130, totalUs: 29000)) == "2a00000004000000606d00008200000048710000")
  }

  @Test("A header from another peer or another version is refused, not misread")
  func refusesStrangers() {
    var bytes = [UInt8](repeating: 0, count: Wire.headerSize)
    #expect(throws: Wire.ProtocolError.self) { try bytes.withUnsafeBytes { try Wire.unpackHeader($0.baseAddress!) } }
    bytes.withUnsafeMutableBytes { Wire.packHeader(Wire.Header(msgType: 1, seq: 1, flags: 0, length: 0), into: $0.baseAddress!) }
    for version: UInt8 in [1, 2, 4] {
      bytes[4] = version
      #expect(throws: Wire.ProtocolError.self) { try bytes.withUnsafeBytes { try Wire.unpackHeader($0.baseAddress!) } }
    }
  }
}
