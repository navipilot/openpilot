import Foundation
import JetlinkKit

/// The wire protocol, byte for byte what `jetlink/protocol.py` defines.
///
/// Every message is a 32-byte header and an opaque payload. INFER is a fixed
/// struct plus two raw arrays sized at handshake, not json: one vectored write
/// out, one read into a preallocated buffer back. The numbers come from
/// `Pinned`, which make_pins.py writes from the Python; the enums below are
/// checked against it in ConformanceTests.
public enum Wire {
  public static let magic = Pinned.magic  // b'JLNK'
  /// The one version this server speaks: the comma package and the server
  /// are updated together, and a header of any other is a broken stream. 3
  /// keeps the hidden state here, so INFER_REQ has no prev_feat and INFER_RESP
  /// no hidden_state slice.
  public static let version = Pinned.protocolVersion
  public static let headerSize = Pinned.headerSize
  /// A bulk transfer ends on a short packet, so a message that is an exact
  /// multiple of the packet size gets a pad byte and Flag.padded. 512, a
  /// high-speed packet, which divides a SuperSpeed one: a USB 2 link (every
  /// Android phone, a cable that fell back) must end each message short too.
  /// TCP keeps the rule so one client speaks to every transport the same way.
  public static let packetMultiple = Pinned.packetMultiple
  /// The gadget pads every message it sends to a whole burst, so none ends on
  /// a short packet: dwc3 flushed its TX FIFO past one about once in 400
  /// frames. A USB host reads each message to that boundary. Host to device
  /// keeps the one-byte pad instead. `protocol.GADGET_TX_ALIGN`.
  public static let gadgetTxAlign = Pinned.gadgetTxAlign

  /// The pad byte a received message's header says follows its payload.
  static func pad(_ header: Header) -> Int {
    Flag(rawValue: header.flags).contains(.padded) ? 1 : 0
  }

  /// Does a sent message with a `length` byte payload need the pad byte and
  /// Flag.padded? The rule every sender but the gadget keeps.
  static func needsPad(_ length: Int) -> Bool {
    (headerSize + length) % packetMultiple == 0
  }
  /// Stops a corrupt length field making the receive buffer allocate
  /// gigabytes. A big model's request is 393 KB.
  public static let maxMessage = Pinned.maxMessage
  public static let defaultPort = Pinned.defaultPort

  public enum Msg: UInt16, Sendable {
    case helloReq = 1
    case helloResp = 2
    case engineReq = 3
    case engineResp = 4
    case uploadChunk = 5
    case uploadDone = 6
    case progress = 7
    case inferReq = 8
    case inferResp = 9
    case stateReq = 12
    case stateResp = 13
    case error = 14
    case ping = 15
    case pong = 16
    case shutdownReq = 17
    case shutdownResp = 18
    /// The client stops using the link and says why, with what it measured;
    /// no reply. The connection may stay for a later hello.
    case leave = 19
  }

  public struct Flag: OptionSet, Sendable {
    public let rawValue: UInt32
    public init(rawValue: UInt32) { self.rawValue = rawValue }
    /// On INFER_REQ: warm-start, clear history before this frame.
    public static let resetQueues = Flag(rawValue: 1 << 0)
    /// On INFER_REQ: append telemetry json to the response.
    public static let wantState = Flag(rawValue: 1 << 1)
    /// On INFER_REQ: keep hidden_state in the response, for a comma logging
    /// the whole output vector.
    public static let wantHidden = Flag(rawValue: 1 << 2)
    /// One pad byte follows the payload; see packetMultiple.
    public static let padded = Flag(rawValue: 1 << 7)
  }

  public enum Status: UInt32, Sendable {
    case ok = 0
    case notReady = 1
    case badShape = 2
    case inferFailed = 3
    /// The model produced NaN or Inf; the comma falls back.
    case notFinite = 4
  }

  /// INFER_REQ: frame_id, flags.
  public static let inferReqSize = Pinned.inferReqSize
  /// INFER_RESP: frame_id, status, gpu_us, queue_us, total_us.
  public static let inferRespSize = Pinned.inferRespSize

  public struct Header: Equatable, Sendable {
    public var msgType: UInt16
    public var seq: UInt32
    public var flags: UInt32
    public var length: UInt32
    public var reserved: UInt64 = 0

    public init(msgType: UInt16, seq: UInt32, flags: UInt32, length: UInt32, reserved: UInt64 = 0) {
      self.msgType = msgType
      self.seq = seq
      self.flags = flags
      self.length = length
      self.reserved = reserved
    }
  }

  public enum ProtocolError: Error, CustomStringConvertible {
    case badMagic(UInt32)
    case badVersion(UInt16)
    case tooLong(UInt32)

    public var description: String {
      switch self {
      case .badMagic(let magic):
        return "bad magic 0x\(String(magic, radix: 16)) (link desynced or not a jetlink peer)"
      case .badVersion(let version):
        return "peer speaks protocol v\(version), we speak v\(Wire.version)"
      case .tooLong(let length):
        return "message claims \(length) bytes, over the \(Wire.maxMessage) cap"
      }
    }
  }

  /// '<IHHIIIQ4x': magic, version, msg_type, seq, flags, length, reserved, 4 pad.
  public static func packHeader(_ header: Header, into out: UnsafeMutableRawPointer) {
    out.storeBytes(of: magic.littleEndian, toByteOffset: 0, as: UInt32.self)
    out.storeBytes(of: version.littleEndian, toByteOffset: 4, as: UInt16.self)
    out.storeBytes(of: header.msgType.littleEndian, toByteOffset: 6, as: UInt16.self)
    out.storeBytes(of: header.seq.littleEndian, toByteOffset: 8, as: UInt32.self)
    out.storeBytes(of: header.flags.littleEndian, toByteOffset: 12, as: UInt32.self)
    out.storeBytes(of: header.length.littleEndian, toByteOffset: 16, as: UInt32.self)
    out.storeBytes(of: header.reserved.littleEndian, toByteOffset: 20, as: UInt64.self)
    out.storeBytes(of: UInt32(0), toByteOffset: 28, as: UInt32.self)
  }

  public static func unpackHeader(_ buffer: UnsafeRawPointer) throws -> Header {
    let magicRead = UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 0, as: UInt32.self))
    guard magicRead == magic else { throw ProtocolError.badMagic(magicRead) }
    let versionRead = UInt16(littleEndian: buffer.loadUnaligned(fromByteOffset: 4, as: UInt16.self))
    guard versionRead == version else { throw ProtocolError.badVersion(versionRead) }
    return Header(
      msgType: UInt16(littleEndian: buffer.loadUnaligned(fromByteOffset: 6, as: UInt16.self)),
      seq: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 8, as: UInt32.self)),
      flags: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 12, as: UInt32.self)),
      length: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 16, as: UInt32.self)),
      reserved: UInt64(littleEndian: buffer.loadUnaligned(fromByteOffset: 20, as: UInt64.self)))
  }

  // MARK: frames as datagrams

  /// A frame over the phone's cable as UDP datagrams (`FrameDatagrams`): the
  /// INFER_REQ's stream bytes cut into equal pieces of at most
  /// `datagramPayload`, each behind a datagram header. `protocol.DATAGRAM_MAGIC`.
  public static let datagramMagic = Pinned.datagramMagic  // b'JFRM'
  public static let datagramHeaderSize = Pinned.datagramHeaderSize
  public static let datagramPayload = Pinned.datagramPayload

  /// '<IIIII': magic, then where this piece goes in a message of `total` bytes,
  /// for the session `token` the server's hello gave.
  struct DatagramHeader: Equatable, Sendable {
    var token: UInt32
    var seq: UInt32
    var offset: UInt32
    var total: UInt32
  }

  /// The header, or nil for a datagram that is not ours.
  static func unpackDatagramHeader(_ buffer: UnsafeRawPointer) -> DatagramHeader? {
    guard UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 0, as: UInt32.self)) == datagramMagic else { return nil }
    return DatagramHeader(
      token: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 4, as: UInt32.self)),
      seq: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 8, as: UInt32.self)),
      offset: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 12, as: UInt32.self)),
      total: UInt32(littleEndian: buffer.loadUnaligned(fromByteOffset: 16, as: UInt32.self)))
  }

  public static func packInferResp(frameID: UInt32, status: Status, gpuUs: UInt32, queueUs: UInt32, totalUs: UInt32, into out: UnsafeMutableRawPointer) {
    out.storeBytes(of: frameID.littleEndian, toByteOffset: 0, as: UInt32.self)
    out.storeBytes(of: status.rawValue.littleEndian, toByteOffset: 4, as: UInt32.self)
    out.storeBytes(of: gpuUs.littleEndian, toByteOffset: 8, as: UInt32.self)
    out.storeBytes(of: queueUs.littleEndian, toByteOffset: 12, as: UInt32.self)
    out.storeBytes(of: totalUs.littleEndian, toByteOffset: 16, as: UInt32.self)
  }

  public static func inferResp(frameID: UInt32 = 0, status: Status, gpuUs: UInt32 = 0, queueUs: UInt32 = 0, totalUs: UInt32 = 0) -> [UInt8] {
    var bytes = [UInt8](repeating: 0, count: inferRespSize)
    bytes.withUnsafeMutableBytes {
      packInferResp(frameID: frameID, status: status, gpuUs: gpuUs, queueUs: queueUs, totalUs: totalUs, into: $0.baseAddress!)
    }
    return bytes
  }
}
