import Foundation
import JetlinkKit

#if canImport(Darwin)
  import Darwin
#elseif canImport(Glibc)
  import Glibc
#elseif canImport(Android)
  import Android
#endif

/// Frames the comma sends over a phone's cable as UDP datagrams, put back
/// together: the Swift form of what `TcpTransport.send_datagrams` sends.
///
/// The comma's kernel does its per-packet work once a datagram where TCP does
/// it once a segment, 7.35 to 4.3 ms a frame on the bench. A frame that loses
/// a piece is dropped when the next one starts; the comma, which never gets
/// its reply, publishes its last output again. Only datagrams from the comma's
/// address with this session's token count. Not thread-safe: the session's
/// loop owns it.
final class FrameDatagrams {
  let fd: Int32
  let port: UInt16
  /// What the kernel granted of the receive buffer asked for.
  let receiveBuffer: Int
  private(set) var token: UInt32 = 0
  private let comma: in_addr_t

  /// The message being put back together, and the pieces in it so far.
  private var buffer: UnsafeMutableRawPointer
  private var capacity: Int
  private var current: (seq: UInt32, total: Int)?
  private var got = 0
  private var offsets: [UInt32] = []
  /// The last seq completed: anything at or below it is late or repeated.
  private var done: UInt32 = 0
  /// One datagram, header and piece, before it is copied into place.
  private let scratch: UnsafeMutableRawPointer
  private static let scratchSize = 1 << 16

  private(set) var completed = 0
  private(set) var dropped = 0
  private let log = ServerLog(category: "session")

  /// A socket on `local`, the cable address this end of the TCP link has,
  /// taking datagrams from `comma` alone. Nil when the socket cannot be made.
  init?(local: in_addr, comma: in_addr) {
    let fd = socket(AF_INET, Sys.datagram, 0)
    guard fd >= 0 else { return nil }
    // As much as the kernel allows of a few frames; iOS caps it lower than Linux.
    var granted: Int32 = 0
    for size: Int32 in [8 << 20, 4 << 20, 2 << 20, 1 << 20] {
      var value = size
      if setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &value, socklen_t(MemoryLayout<Int32>.size)) == 0 {
        var length = socklen_t(MemoryLayout<Int32>.size)
        _ = getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &granted, &length)
        break
      }
    }
    var address = sockaddr_in()
    #if canImport(Darwin)
      address.sin_len = UInt8(MemoryLayout<sockaddr_in>.size)
    #endif
    address.sin_family = sa_family_t(AF_INET)
    address.sin_addr = local
    address.sin_port = 0
    let bound = withUnsafePointer(to: &address) {
      $0.withMemoryRebound(to: sockaddr.self, capacity: 1) { bind(fd, $0, socklen_t(MemoryLayout<sockaddr_in>.size)) }
    }
    var length = socklen_t(MemoryLayout<sockaddr_in>.size)
    let named = withUnsafeMutablePointer(to: &address) {
      $0.withMemoryRebound(to: sockaddr.self, capacity: 1) { getsockname(fd, $0, &length) }
    }
    guard bound == 0, named == 0 else {
      Sys.close(fd)
      return nil
    }
    _ = fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK)
    self.fd = fd
    port = UInt16(bigEndian: address.sin_port)
    receiveBuffer = Int(granted)
    self.comma = comma.s_addr
    capacity = 1 << 19
    buffer = .allocate(byteCount: capacity, alignment: 64)
    scratch = .allocate(byteCount: FrameDatagrams.scratchSize, alignment: 16)
  }

  deinit {
    close()
    buffer.deallocate()
    scratch.deallocate()
  }

  private var closed = false

  /// Says what the datagrams did, for a link that had any.
  func close() {
    guard !closed else { return }
    closed = true
    Sys.close(fd)
    if completed + dropped > 0 {
      log.info(summary)
    }
  }

  /// A new session's token, `fixed` in a test: whatever is in flight for the
  /// old one is dropped.
  func renew(_ fixed: UInt32? = nil) -> UInt32 {
    token = fixed ?? UInt32.random(in: 1...UInt32.max)
    current = nil
    got = 0
    done = 0
    return token
  }

  /// Reads what has arrived, without waiting, until a message is whole: then
  /// its bytes, header and pad included, valid until the next call. Nil when
  /// what arrived completes nothing.
  func read() -> UnsafeRawBufferPointer? {
    while true {
      var from = sockaddr_in()
      var length = socklen_t(MemoryLayout<sockaddr_in>.size)
      let n = withUnsafeMutablePointer(to: &from) {
        $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
          recvfrom(fd, scratch, FrameDatagrams.scratchSize, 0, $0, &length)
        }
      }
      if n < 0 {
        if errno == EINTR { continue }
        return nil  // EAGAIN: nothing more for now
      }
      if let whole = take(n, from: from.sin_addr.s_addr) {
        return whole
      }
    }
  }

  /// One datagram of `n` bytes in `scratch`, put in place.
  private func take(_ n: Int, from source: in_addr_t) -> UnsafeRawBufferPointer? {
    guard source == comma, n > Wire.datagramHeaderSize, let header = Wire.unpackDatagramHeader(scratch),
      header.token == token, token != 0
    else { return nil }
    let size = n - Wire.datagramHeaderSize
    let total = Int(header.total)
    let offset = Int(header.offset)
    guard total >= Wire.headerSize, total <= Wire.maxMessage, offset + size <= total, header.seq > done else { return nil }
    if let current, current.seq != header.seq {
      if header.seq < current.seq { return nil }  // an older message's straggler
      dropped += 1  // a newer frame started: this one lost a piece
      self.current = nil
    }
    if current == nil {
      reserve(total)
      current = (header.seq, total)
      got = 0
      offsets.removeAll(keepingCapacity: true)
    }
    guard current?.total == total, !offsets.contains(header.offset) else { return nil }
    offsets.append(header.offset)
    (buffer + offset).copyMemory(from: scratch + Wire.datagramHeaderSize, byteCount: size)
    got += size
    guard got == total else { return nil }
    done = header.seq
    current = nil
    completed += 1
    return UnsafeRawBufferPointer(start: buffer, count: total)
  }

  private func reserve(_ total: Int) {
    guard total > capacity else { return }
    buffer.deallocate()
    capacity = max(total, capacity * 2)
    buffer = .allocate(byteCount: capacity, alignment: 64)
  }

  private var summary: String {
    "frames as datagrams: \(completed) whole, \(dropped) dropped incomplete (receive buffer \(receiveBuffer >> 10) KB)"
  }
}
