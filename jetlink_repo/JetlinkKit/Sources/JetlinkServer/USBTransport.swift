import Foundation
import JetlinkKit

/// The two bulk endpoints of the comma's vendor interface: IOUSBHost on a Mac,
/// usbfs on Linux and Android, a fake in the tests. Both calls block.
protocol BulkPipes: AnyObject, Sendable {
  /// Reads up to `count` bytes of the stream into `buffer` (`ReadRing`), and
  /// returns how many arrived. A timeout is not an error: whatever did arrive
  /// is returned, or kept for the next read, because dropping it desyncs the
  /// stream. A `timeout` of 0 waits until data comes or the link goes. Throws
  /// `LinkError` when the link is gone or the transfer failed.
  func read(into buffer: UnsafeMutableRawPointer, count: Int, timeout: TimeInterval) throws -> Int
  /// Writes `count` bytes as one transfer and returns how many went out, also
  /// after a timeout, so the caller resends only the rest.
  func write(from buffer: UnsafeRawPointer, count: Int, timeout: TimeInterval) throws -> Int
  /// Ends any transfer in flight with an error, from another thread.
  func abort()
  /// Aborts and lets the interface go.
  func close()
}

/// Where the server finds the comma's gadget: IOKit on a Mac, a descriptor
/// the Android app hands over (UsbfsGadget), sysfs on Linux, a fake in the
/// tests.
public protocol GadgetSource: Sendable {
  /// Is the gadget on the bus? Cheap enough to poll twice a second.
  func present() -> Bool
  /// Opens the link interface's bulk pair.
  func open() throws -> any MessageLink
  /// A comma connected, over any link, and that session ended: what the
  /// server says as `.connected` and `.disconnected`, on the session's thread.
  func sessionStarted()
  func sessionEnded()
  /// A message arrived on the session, before it is handled (on the
  /// session's thread), or the server pushed a finished build to it (on the
  /// job's): a session outlives the comma's modeld while its gadget stays
  /// up, so a quiet one is told apart this way. Called for every frame, so
  /// it must return at once.
  func sessionHeard()
  /// The server is shutting down.
  func close()
}

extension GadgetSource {
  public func sessionStarted() {}
  public func sessionEnded() {}
  public func sessionHeard() {}
  public func close() {}
}

/// Framing over USB bulk transfers, the host's end: the Swift form of
/// `UsbBulkTransport` on `StreamTransport`, with the rules the bench taught.
///
/// - The gadget pads every message to `Wire.gadgetTxAlign` (16 KB), and the
///   pipes keep 16 KB reads posted ahead of this end (`ReadRing` says why
///   that is safe), so a message streams in without the host asking for it
///   piece by piece.
/// - What this end sends keeps the one-byte PADDED rule, and goes out as one
///   transfer: libusb and IOUSBHost have no vectored bulk write, and several
///   writes let the host scheduler interleave them.
/// - Reads wait without a deadline, as the Python server's do between
///   messages; `shutdown()` aborts them. Writes give up after
///   `writeTimeout` without progress.
/// - After a protocol error nothing resynchronises the stream: the link is
///   latched desynced, and `drain` swallows what the comma is still sending
///   so its own frame timeout fires instead of a half-written message
///   blocking it.
///
/// Every message lands at the start of the receive buffer, so the float32
/// arrays inside an INFER stay aligned, and the steady state allocates nothing.
final class USBTransport: MessageLink, @unchecked Sendable {
  static let writeTimeout: TimeInterval = 2

  let peer: String
  /// The USB generation the bus negotiated, as the host read it.
  let medium: LinkMedium?
  var connectsOnOpen: Bool { false }
  /// After a protocol error nothing resynchronises the stream, so `close`
  /// drains what the comma is still sending, unless the link was interrupted.
  static let drainTimeout: TimeInterval = 5.0
  private var interrupted = false
  private let pipes: any BulkPipes
  private let buffers: LinkBuffers
  private var reader: FrameReader { buffers.reader }
  var desynced: Bool { reader.desynced }
  private var sendLock: NSLock { buffers.sendLock }

  /// `buffers` may be the last session's on the same device, one session at
  /// a time.
  init(pipes: any BulkPipes, peer: String = "usb", medium: LinkMedium? = .usb, buffers: LinkBuffers = LinkBuffers()) {
    self.pipes = pipes
    self.peer = peer
    self.medium = medium
    self.buffers = buffers
    buffers.reader.reset()
  }

  deinit {
    pipes.close()
  }

  // MARK: receiving

  func recv() throws -> Message {
    try reader.recv(pad: { USBTransport.gadgetPad(Wire.headerSize + Int($0.length)) }) { into, missing in
      try pipes.read(into: into, count: missing, timeout: 0)
    }
  }

  /// The pad the gadget put after a message of `body` bytes.
  static func gadgetPad(_ body: Int) -> Int {
    (Wire.gadgetTxAlign - body % Wire.gadgetTxAlign) % Wire.gadgetTxAlign
  }

  /// After a desync, reads and drops what the comma is still sending until it
  /// goes quiet for `timeout` or the link drops. Reopening instead would
  /// drain a packet per session, and the comma's frame timeout would never
  /// fire. The Python server waits 5 s, longer than the client's own timeout.
  func drain(_ timeout: TimeInterval) {
    let size = ReadRing.depth * ReadRing.slotSize
    let scratch = UnsafeMutableRawPointer.allocate(byteCount: size, alignment: 64)
    defer { scratch.deallocate() }
    var last = ProcessInfo.processInfo.systemUptime
    while ProcessInfo.processInfo.systemUptime - last < timeout {
      guard let n = try? pipes.read(into: scratch, count: size, timeout: timeout) else { return }
      if n > 0 {
        last = ProcessInfo.processInfo.systemUptime
      }
    }
  }

  // MARK: sending

  func sendParts(_ type: Wire.Msg, seq: UInt32, parts: UnsafeBufferPointer<UnsafeRawBufferPointer>, flags: Wire.Flag) throws {
    sendLock.lock()
    defer { sendLock.unlock() }
    var flags = flags
    var length = 0
    for part in parts { length += part.count }
    let padded = Wire.needsPad(length)
    if padded {
      flags.insert(.padded)
    }
    let total = Wire.headerSize + length + (padded ? 1 : 0)
    let tx = buffers.tx(total)
    Wire.packHeader(Wire.Header(msgType: type.rawValue, seq: seq, flags: flags.rawValue, length: UInt32(length)), into: tx)
    var offset = Wire.headerSize
    for part in parts where part.count > 0 {
      (tx + offset).copyMemory(from: part.baseAddress!, byteCount: part.count)
      offset += part.count
    }
    if padded {
      tx.storeBytes(of: UInt8(0), toByteOffset: offset, as: UInt8.self)
    }
    var sent = 0
    while sent < total {
      let n = try pipes.write(from: tx + sent, count: total - sent, timeout: USBTransport.writeTimeout)
      if n <= 0 {
        throw LinkError.closed("peer went away during send")
      }
      sent += n
    }
  }

  // MARK: lifecycle

  func shutdown() {
    interrupted = true
    pipes.abort()
  }

  /// Closes the pipes. After a desync the comma is still mid-message; it is
  /// let finish and time out first, rather than reopened under.
  func close() {
    if desynced && !interrupted {
      drain(USBTransport.drainTimeout)
    }
    pipes.close()
  }
}

/// A USB link's receive and send buffers, kept by a device across its
/// sessions (`UsbfsGadget`), so a reopen neither allocates them nor faults
/// them in again.
final class LinkBuffers: @unchecked Sendable {
  let reader = FrameReader(capacity: 2 << 20)
  /// A build's progress can still be going out through the last session's
  /// transport, so sends share the lock with the buffer.
  let sendLock = NSLock()
  private var send = UnsafeMutableRawPointer.allocate(byteCount: 1 << 20, alignment: 64)
  private var sendCapacity = 1 << 20

  deinit {
    send.deallocate()
  }

  /// The send buffer, grown to `bytes` when it is smaller.
  func tx(_ bytes: Int) -> UnsafeMutableRawPointer {
    if bytes > sendCapacity {
      send.deallocate()
      sendCapacity = max(bytes, sendCapacity * 2)
      send = UnsafeMutableRawPointer.allocate(byteCount: sendCapacity, alignment: 64)
    }
    return send
  }
}
