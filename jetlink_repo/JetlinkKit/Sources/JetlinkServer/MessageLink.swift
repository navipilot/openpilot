import Foundation
import JetlinkKit

/// One framed, ordered message channel to one comma: a TCP connection, or bulk
/// transfers to the comma's USB gadget. The session speaks to either the same
/// way; only how the bytes are framed on the way differs.
public protocol MessageLink: AnyObject, Sendable {
  /// Who is on the other end, for the link event and the log.
  var peer: String { get }
  /// How the link is carried, as this end sees it; the comma's hello may say
  /// better (a phone's cable is TCP over USB).
  var medium: LinkMedium? { get }
  /// Is the link up as soon as it is open? A TCP connection is someone
  /// dialing; the USB gadget is on the bus whether or not anything on the
  /// comma serves it, so over USB the link is up on the comma's first message.
  var connectsOnOpen: Bool { get }
  /// The next message. Its payload is a view into the link's receive buffer,
  /// valid only until the next `recv()`.
  func recv() throws -> Message
  /// One message, `parts` concatenated as its payload, from the caller's storage.
  func sendParts(_ type: Wire.Msg, seq: UInt32, parts: UnsafeBufferPointer<UnsafeRawBufferPointer>, flags: Wire.Flag) throws
  /// Wakes a `recv()` blocked on another thread, which then throws.
  func shutdown()
  func close()
  /// A port and a new token for frames as datagrams, or nil where the link
  /// has none: only a phone's cable (`TCPTransport.offerDatagrams`).
  func offerDatagrams() -> (port: UInt16, token: UInt32)?
}

extension MessageLink {
  public func offerDatagrams() -> (port: UInt16, token: UInt32)? { nil }

  public func send(_ type: Wire.Msg, seq: UInt32, parts: [UnsafeRawBufferPointer] = [], flags: Wire.Flag = []) throws {
    try parts.withUnsafeBufferPointer { try sendParts(type, seq: seq, parts: $0, flags: flags) }
  }

  public func send(_ type: Wire.Msg, seq: UInt32, data: [Data], flags: Wire.Flag = []) throws {
    try withBuffers(data) { try send(type, seq: seq, parts: $0, flags: flags) }
  }

  public func sendJSON(_ type: Wire.Msg, seq: UInt32, _ object: [String: Any], flags: Wire.Flag = []) throws {
    try send(type, seq: seq, data: [JSONLine.encode(object)], flags: flags)
  }
}

/// `data`'s bytes as raw buffers, valid inside `body`.
func withBuffers<R>(_ data: [Data], _ body: ([UnsafeRawBufferPointer]) throws -> R) throws -> R {
  var buffers: [UnsafeRawBufferPointer] = []
  func recurse(_ index: Int) throws -> R {
    if index == data.count {
      return try body(buffers)
    }
    return try data[index].withUnsafeBytes { bytes in
      buffers.append(bytes)
      return try recurse(index + 1)
    }
  }
  return try recurse(0)
}

extension TCPTransport: MessageLink {
  public var medium: LinkMedium? { LinkMedium(tcpPeer: peer) }
  public var connectsOnOpen: Bool { true }
}
