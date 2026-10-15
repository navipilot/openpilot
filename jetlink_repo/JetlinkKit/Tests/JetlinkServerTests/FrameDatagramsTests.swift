import Foundation
import JetlinkKit
import JetlinkTestSupport
import Testing

@testable import JetlinkServer

#if canImport(Glibc)
  import Glibc
#elseif canImport(Android)
  import Android
#endif

extension Wire {
  /// What `protocol.pack_datagram_header` packs: the comma's side, which the
  /// server never needs, to check the reading side against.
  static func packDatagramHeader(_ header: DatagramHeader, into out: UnsafeMutableRawPointer) {
    out.storeBytes(of: datagramMagic.littleEndian, toByteOffset: 0, as: UInt32.self)
    out.storeBytes(of: header.token.littleEndian, toByteOffset: 4, as: UInt32.self)
    out.storeBytes(of: header.seq.littleEndian, toByteOffset: 8, as: UInt32.self)
    out.storeBytes(of: header.offset.littleEndian, toByteOffset: 12, as: UInt32.self)
    out.storeBytes(of: header.total.littleEndian, toByteOffset: 16, as: UInt32.self)
  }

  /// How the comma cuts a message of `total` bytes: `protocol.datagram_pieces`.
  static func datagramPieces(_ total: Int) -> [(offset: Int, size: Int)] {
    let count = max(1, (total + datagramPayload - 1) / datagramPayload)
    var pieces: [(offset: Int, size: Int)] = []
    var offset = 0
    for index in 0..<count {
      let size = total / count + (index < total % count ? 1 : 0)
      pieces.append((offset, size))
      offset += size
    }
    return pieces
  }
}

/// Datagrams to a `FrameDatagrams` on loopback, as the comma sends them over
/// the cable.
final class DatagramSender {
  let fd: Int32
  let to: sockaddr_in

  init(port: UInt16) throws {
    fd = socket(AF_INET, Sys.datagram, 0)
    guard fd >= 0 else { throw TestError("socket failed") }
    // Darwin refuses a datagram bigger than the send buffer, 9 KB by default
    Sys.set(fd, SOL_SOCKET, SO_SNDBUF, 4 << 20)
    var to = sockaddr_in()
    #if canImport(Darwin)
      to.sin_len = UInt8(MemoryLayout<sockaddr_in>.size)
    #endif
    to.sin_family = sa_family_t(AF_INET)
    to.sin_port = port.bigEndian
    to.sin_addr.s_addr = inet_addr("127.0.0.1")
    self.to = to
  }

  deinit { Sys.close(fd) }

  func send(_ bytes: [UInt8]) {
    var to = to
    _ = bytes.withUnsafeBytes { body in
      withUnsafePointer(to: &to) {
        $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
          sendto(fd, body.baseAddress, body.count, 0, $0, socklen_t(MemoryLayout<sockaddr_in>.size))
        }
      }
    }
  }

  /// One piece of a message: header for `token` and `seq`, then `message[offset..<offset+size]`.
  func send(token: UInt32, seq: UInt32, message: [UInt8], offset: Int, size: Int, magic: Bool = true) {
    var datagram = [UInt8](repeating: 0, count: Wire.datagramHeaderSize + size)
    datagram.withUnsafeMutableBytes {
      Wire.packDatagramHeader(.init(token: token, seq: seq, offset: UInt32(offset), total: UInt32(message.count)), into: $0.baseAddress!)
    }
    if !magic { datagram[0] ^= 0xFF }
    datagram.replaceSubrange(Wire.datagramHeaderSize..., with: message[offset..<offset + size])
    send(datagram)
  }

  /// Every piece of `message` as the comma cuts it, in `order` (an index list).
  func sendAll(token: UInt32, seq: UInt32, message: [UInt8], order: [Int]? = nil) {
    let pieces = Wire.datagramPieces(message.count)
    for index in order ?? Array(pieces.indices) {
      send(token: token, seq: seq, message: message, offset: pieces[index].offset, size: pieces[index].size)
    }
  }
}

/// What `read()` finishes, waiting up to a second for loopback to deliver.
private func whole(_ datagrams: FrameDatagrams, wait: TimeInterval = 1.0) -> [UInt8]? {
  var got: [UInt8]?
  _ = eventually(timeout: wait) {
    got = datagrams.read().map(Array.init)
    return got != nil
  }
  return got
}

private func loopback() throws -> FrameDatagrams {
  let local = in_addr(s_addr: inet_addr("127.0.0.1"))
  guard let datagrams = FrameDatagrams(local: local, comma: local) else { throw TestError("no datagram socket") }
  return datagrams
}

/// A message of `count` bytes, every one different from its neighbours.
private func message(_ count: Int, seed: Int = 0) -> [UInt8] {
  (0..<count).map { UInt8(($0 * 13 + seed * 7) % 251) }
}

@Suite("Conformance: frames as datagrams, as protocol.py and TcpTransport send them")
struct DatagramConformanceTests {
  @Test("Datagram headers pack and unpack as protocol.pack_datagram_header does")
  func headers() throws {
    let datagrams = try Conformance.json("wire.json")["datagrams"] as! [String: Any]
    for h in datagrams["headers"] as! [[String: Any]] {
      let header = Wire.DatagramHeader(
        token: UInt32(int(h["token"])), seq: UInt32(int(h["seq"])), offset: UInt32(int(h["offset"])), total: UInt32(int(h["total"])))
      var packed = [UInt8](repeating: 0xEE, count: Wire.datagramHeaderSize)
      packed.withUnsafeMutableBytes { Wire.packDatagramHeader(header, into: $0.baseAddress!) }
      #expect(packed == hex(h["hex"] as! String))
      #expect(hex(h["hex"] as! String).withUnsafeBytes { Wire.unpackDatagramHeader($0.baseAddress!) } == header)
    }
  }

  @Test("Messages are cut as protocol.datagram_pieces cuts them")
  func pieces() throws {
    let datagrams = try Conformance.json("wire.json")["datagrams"] as! [String: Any]
    for case let p as [String: Any] in datagrams["pieces"] as! [Any] {
      let want = (p["pieces"] as! [[NSNumber]]).map { [$0[0].intValue, $0[1].intValue] }
      #expect(Wire.datagramPieces(int(p["total"])).map { [$0.offset, $0.size] } == want, "\(p["total"]!)")
    }
  }

  @Test("What TcpTransport.send_datagrams sends is put back together, in any order", arguments: [false, true])
  func reassembles(reversed: Bool) throws {
    let frame = (try Conformance.json("wire.json")["datagrams"] as! [String: Any])["frame"] as! [String: Any]
    let blob = [UInt8](try Conformance.data(frame["file"] as! String))
    var sent: [[UInt8]] = []
    var at = 0
    for size in (frame["sizes"] as! [NSNumber]).map(\.intValue) {
      sent.append(Array(blob[at..<at + size]))
      at += size
    }
    let datagrams = try loopback()
    _ = datagrams.renew(UInt32(int(frame["token"])))
    let sender = try DatagramSender(port: datagrams.port)
    for datagram in reversed ? sent.reversed() : sent {
      sender.send(datagram)
    }
    let bytes = try #require(whole(datagrams))
    let message = try #require(bytes.withUnsafeBytes { TCPTransport.message($0).map { ($0.msgType, $0.seq, $0.flags, Array($0.payload)) } })
    let want = WireMessage(["type": frame["type"]!, "seq": frame["seq"]!, "flags": frame["flags"]!, "parts": frame["parts"]!])
    #expect(message.0 == want.type.rawValue && message.1 == want.seq)
    #expect(Wire.Flag(rawValue: message.2).subtracting(.padded) == want.flags)
    #expect(message.3 == [UInt8](want.payload))
    #expect(datagrams.completed == 1 && datagrams.dropped == 0)
  }
}

@Suite("FrameDatagrams: a frame whole, or dropped")
struct FrameDatagramsTests {
  @Test("A newer frame drops one still missing a piece")
  func newerDropsIncomplete() throws {
    let datagrams = try loopback()
    let token = datagrams.renew(7)
    let sender = try DatagramSender(port: datagrams.port)
    let first = message(150_000, seed: 1)
    let second = message(150_000, seed: 2)
    sender.sendAll(token: token, seq: 5, message: first, order: [0, 2])
    #expect(whole(datagrams, wait: 0.2) == nil)
    sender.sendAll(token: token, seq: 6, message: second)
    #expect(whole(datagrams) == second)
    #expect(datagrams.dropped == 1 && datagrams.completed == 1)
  }

  @Test("Stragglers of an older frame, and a frame seen whole again, count for nothing")
  func stragglersAndRepeats() throws {
    let datagrams = try loopback()
    let token = datagrams.renew(7)
    let sender = try DatagramSender(port: datagrams.port)
    let frame = message(100_000)
    sender.sendAll(token: token, seq: 9, message: frame)
    #expect(whole(datagrams) == frame)
    sender.sendAll(token: token, seq: 9, message: frame)
    sender.sendAll(token: token, seq: 8, message: frame)
    #expect(whole(datagrams, wait: 0.2) == nil)
    #expect(datagrams.completed == 1 && datagrams.dropped == 0)
  }

  @Test("Another session's token, or something not ours, is ignored")
  func foreign() throws {
    let datagrams = try loopback()
    let token = datagrams.renew(7)
    let sender = try DatagramSender(port: datagrams.port)
    let frame = message(1000)
    sender.sendAll(token: token + 1, seq: 1, message: frame)
    sender.send(token: token, seq: 1, message: frame, offset: 0, size: frame.count, magic: false)
    sender.send([1, 2, 3])
    #expect(whole(datagrams, wait: 0.2) == nil)
    sender.sendAll(token: token, seq: 1, message: frame)
    #expect(whole(datagrams) == frame)
  }

  @Test("A piece that arrives twice does not finish a frame missing another")
  func duplicates() throws {
    let datagrams = try loopback()
    let token = datagrams.renew(7)
    let sender = try DatagramSender(port: datagrams.port)
    let frame = message(200_000)
    sender.sendAll(token: token, seq: 3, message: frame, order: [0, 0, 1, 1, 2])
    #expect(whole(datagrams, wait: 0.2) == nil)
    sender.sendAll(token: token, seq: 3, message: frame, order: [3])
    #expect(whole(datagrams) == frame)
  }

  @Test("A new token drops whatever was in flight")
  func renewDrops() throws {
    let datagrams = try loopback()
    let old = datagrams.renew(7)
    let sender = try DatagramSender(port: datagrams.port)
    let frame = message(150_000)
    sender.sendAll(token: old, seq: 4, message: frame, order: [0, 1])
    #expect(whole(datagrams, wait: 0.2) == nil)
    let new = datagrams.renew(8)
    sender.sendAll(token: old, seq: 4, message: frame, order: [2])
    #expect(whole(datagrams, wait: 0.2) == nil)
    // the new session's seqs start again
    sender.sendAll(token: new, seq: 1, message: frame)
    #expect(whole(datagrams) == frame)
  }
}
