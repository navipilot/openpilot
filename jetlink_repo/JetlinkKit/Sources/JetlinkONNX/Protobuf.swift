import Foundation

/// An error reading or preparing an ONNX model. The message is written to be
/// shown as it is; where the Python preparation raises, it is the same text.
public struct OnnxError: Error, LocalizedError, CustomStringConvertible, Sendable, Equatable {
  public let message: String

  public init(_ message: String) {
    self.message = message
  }

  public var errorDescription: String? { message }
  public var description: String { message }
}

// MARK: reading

/// Protobuf's wire types. Groups are long deprecated and ONNX has none, but a
/// reader that meets one skips it rather than losing its place.
enum Wire: UInt8 {
  case varint = 0
  case fixed64 = 1
  case bytes = 2
  case startGroup = 3
  case endGroup = 4
  case fixed32 = 5
}

/// One field as it sits in the source. Offsets are into the whole file, so a
/// field can be copied out later without keeping anything else of it.
struct WireField {
  let number: Int
  let wire: Wire
  /// Where the tag starts.
  let start: Int
  /// Just past the field.
  let end: Int
  /// A varint's or fixed field's value; zero for the other wire types.
  let value: UInt64
  /// The bytes after the tag: a length-delimited field's content (after the
  /// length), or a varint's or fixed value's own bytes. Empty for a group.
  let payload: Range<Int>

  /// The whole field, tag included.
  var whole: Range<Int> { start..<end }
}

/// The file's bytes. Memory-mapped by whoever made it, and never copied: every
/// range the decoder keeps is an offset into this.
struct Source {
  let bytes: UnsafeRawBufferPointer

  var count: Int { bytes.count }

  func slice(_ range: Range<Int>) -> UnsafeRawBufferPointer {
    UnsafeRawBufferPointer(rebasing: bytes[range])
  }

  /// A string field. Names are compared and written back, so bytes that are
  /// not UTF-8 are refused rather than replaced: a replaced name would no
  /// longer match the tensor it names.
  func string(_ range: Range<Int>) throws -> String {
    guard let s = String(validating: slice(range), as: UTF8.self) else {
      throw OnnxError("malformed ONNX: a string at offset \(range.lowerBound) is not valid UTF-8")
    }
    return s
  }

  func reader(_ range: Range<Int>) -> ProtoReader {
    ProtoReader(bytes, range)
  }
}

/// Walks the fields of one message. Only tags and lengths are read; a
/// length-delimited field's payload is not touched unless the caller asks.
struct ProtoReader {
  private let buf: UnsafeRawBufferPointer
  private(set) var pos: Int
  let end: Int

  init(_ buf: UnsafeRawBufferPointer, _ range: Range<Int>) {
    self.buf = buf
    self.pos = range.lowerBound
    self.end = range.upperBound
  }

  var atEnd: Bool { pos >= end }

  mutating func next() throws -> WireField? {
    guard pos < end else { return nil }
    let start = pos
    let tag = try varint()
    let number = Int(tag >> 3)
    guard number > 0, number <= 536_870_911 else {
      throw OnnxError("malformed ONNX: field number \(number) at offset \(start)")
    }
    guard let wire = Wire(rawValue: UInt8(tag & 7)) else {
      throw OnnxError("malformed ONNX: wire type \(tag & 7) at offset \(start)")
    }
    let afterTag = pos
    switch wire {
    case .varint:
      let value = try varint()
      return WireField(number: number, wire: wire, start: start, end: pos, value: value, payload: afterTag..<pos)
    case .fixed64:
      let value = try fixed(8)
      return WireField(number: number, wire: wire, start: start, end: pos, value: value, payload: afterTag..<pos)
    case .fixed32:
      let value = try fixed(4)
      return WireField(number: number, wire: wire, start: start, end: pos, value: value, payload: afterTag..<pos)
    case .bytes:
      let length = try varint()
      guard length <= UInt64(end - pos) else {
        throw OnnxError("malformed ONNX: field \(number) at offset \(start) runs past the end of its message")
      }
      let payload = pos..<(pos + Int(length))
      pos = payload.upperBound
      return WireField(number: number, wire: wire, start: start, end: pos, value: 0, payload: payload)
    case .startGroup:
      try skipGroup(number)
      return WireField(number: number, wire: wire, start: start, end: pos, value: 0, payload: pos..<pos)
    case .endGroup:
      throw OnnxError("malformed ONNX: an end-group tag with no group at offset \(start)")
    }
  }

  mutating func varint() throws -> UInt64 {
    var result: UInt64 = 0
    var shift: UInt64 = 0
    while true {
      guard pos < end else { throw OnnxError("malformed ONNX: a varint runs past the end at offset \(pos)") }
      let byte = buf[pos]
      pos += 1
      result |= UInt64(byte & 0x7f) << shift
      if byte < 0x80 { return result }
      shift += 7
      guard shift < 64 else { throw OnnxError("malformed ONNX: a varint longer than ten bytes at offset \(pos)") }
    }
  }

  /// The next `size` bytes, for a fixed-width element of a packed run.
  mutating func take(_ size: Int) throws -> Range<Int> {
    guard end - pos >= size else { throw OnnxError("malformed ONNX: a fixed field runs past the end at offset \(pos)") }
    defer { pos += size }
    return pos..<(pos + size)
  }

  private mutating func fixed(_ size: Int) throws -> UInt64 {
    guard end - pos >= size else { throw OnnxError("malformed ONNX: a fixed field runs past the end at offset \(pos)") }
    var value: UInt64 = 0
    for i in 0..<size {
      value |= UInt64(buf[pos + i]) << (8 * UInt64(i))
    }
    pos += size
    return value
  }

  private mutating func skipGroup(_ number: Int) throws {
    while pos < end {
      let start = pos
      let tag = try varint()
      guard let wire = Wire(rawValue: UInt8(tag & 7)) else {
        throw OnnxError("malformed ONNX: wire type \(tag & 7) at offset \(start)")
      }
      switch wire {
      case .varint: _ = try varint()
      case .fixed64: _ = try fixed(8)
      case .fixed32: _ = try fixed(4)
      case .bytes:
        let length = try varint()
        guard length <= UInt64(end - pos) else { throw OnnxError("malformed ONNX: a group field runs past the end") }
        pos += Int(length)
      case .startGroup: try skipGroup(Int(tag >> 3))
      case .endGroup:
        guard Int(tag >> 3) == number else { throw OnnxError("malformed ONNX: mismatched end-group at offset \(start)") }
        return
      }
    }
    throw OnnxError("malformed ONNX: group \(number) is never closed")
  }
}

extension WireField {
  /// Throws unless the field has the wire type the schema declares for it.
  func expect(_ wire: Wire, _ what: String) throws {
    guard self.wire == wire else {
      throw OnnxError("malformed ONNX: \(what) (field \(number)) has wire type \(self.wire.rawValue) at offset \(start)")
    }
  }

  /// The values of a repeated varint field, which may be written packed or
  /// not whatever the schema says: parsers accept both, and so does this.
  func appendVarints(to values: inout [UInt64], _ src: Source, _ what: String) throws {
    switch wire {
    case .varint:
      values.append(value)
    case .bytes:
      var r = src.reader(payload)
      while !r.atEnd { values.append(try r.varint()) }
    default:
      throw OnnxError("malformed ONNX: \(what) (field \(number)) has wire type \(wire.rawValue) at offset \(start)")
    }
  }
}

// MARK: passthrough

/// A field kept exactly as the source wrote it, tag included.
struct RawField {
  let number: Int
  let range: Range<Int>
}

/// Writes a message's untouched fields back where Python's protobuf would put
/// them. Python (upb) writes the fields its schema knows in field-number
/// order and the ones it does not know after them, in the order it read them.
/// The interpreted fields are written by the caller between calls to `upTo`.
struct FieldCursor {
  /// The fields in the order they are written, each with its sort key: its
  /// number if the schema knows it, Int.max if not.
  private let fields: [(key: Int, field: RawField)]
  private var index = 0

  init(_ fields: [RawField], known: Set<Int>) {
    // A stable sort, so repeated fields keep their order.
    let keyed = fields.enumerated().map { (offset, field) in
      (key: known.contains(field.number) ? field.number : Int.max, offset: offset, field: field)
    }
    self.fields = keyed.sorted { ($0.key, $0.offset) < ($1.key, $1.offset) }.map { ($0.key, $0.field) }
  }

  /// Writes the known fields numbered below `number`.
  mutating func upTo(_ number: Int, _ out: inout Encoded, _ src: Source) {
    while index < fields.count, fields[index].key < number {
      out.source(fields[index].field.range, src)
      index += 1
    }
  }

  /// Writes everything left: the remaining known fields, then the unknown ones.
  mutating func rest(_ out: inout Encoded, _ src: Source) {
    while index < fields.count {
      out.source(fields[index].field.range, src)
      index += 1
    }
  }
}

// MARK: writing

/// A message being encoded, held as its bytes except for the weights. Small
/// fields are copied into owned bytes; a weight stays a range of the source,
/// and a transposed or widened weight stays a recipe, until the writer streams
/// it. Its size is always known, so a parent can write the length prefix
/// before the content without holding the content in memory.
struct Encoded {
  enum Piece {
    case bytes([UInt8])
    case source(Range<Int>)
    case transposed(Transpose)
    case widened(Widen)
  }

  /// Ranges of the source shorter than this are copied rather than referenced:
  /// names, attributes and shapes, not weights.
  static let copyBelow = 4096

  private(set) var pieces: [Piece] = []
  private(set) var tail: [UInt8] = []
  private(set) var count = 0

  mutating func byte(_ b: UInt8) {
    tail.append(b)
    count += 1
  }

  mutating func bytes<C: Collection>(_ bytes: C) where C.Element == UInt8 {
    tail.append(contentsOf: bytes)
    count += bytes.count
  }

  mutating func varint(_ value: UInt64) {
    var v = value
    while v >= 0x80 {
      byte(UInt8(truncatingIfNeeded: v) | 0x80)
      v >>= 7
    }
    byte(UInt8(v))
  }

  mutating func tag(_ number: Int, _ wire: Wire) {
    varint(UInt64(number) << 3 | UInt64(wire.rawValue))
  }

  mutating func varintField(_ number: Int, _ value: UInt64) {
    tag(number, .varint)
    varint(value)
  }

  /// An int64 or int32 field: negative values are sign-extended to ten bytes,
  /// as protobuf writes them.
  mutating func intField(_ number: Int, _ value: Int64) {
    varintField(number, UInt64(bitPattern: value))
  }

  mutating func stringField(_ number: Int, _ value: String) {
    tag(number, .bytes)
    let utf8 = value.utf8
    varint(UInt64(utf8.count))
    bytes(utf8)
  }

  mutating func bytesField(_ number: Int, _ value: [UInt8]) {
    tag(number, .bytes)
    varint(UInt64(value.count))
    bytes(value)
  }

  /// Bytes of the source, verbatim.
  mutating func source(_ range: Range<Int>, _ src: Source) {
    if range.count < Encoded.copyBelow {
      bytes(src.slice(range))
    } else {
      flush()
      pieces.append(.source(range))
      count += range.count
    }
  }

  mutating func transposed(_ t: Transpose) {
    flush()
    pieces.append(.transposed(t))
    count += t.byteCount
  }

  mutating func widened(_ w: Widen) {
    flush()
    pieces.append(.widened(w))
    count += w.byteCount
  }

  /// A nested message: its tag, its length, and then it.
  mutating func message(_ number: Int, _ child: Encoded) {
    tag(number, .bytes)
    varint(UInt64(child.count))
    append(child)
  }

  mutating func append(_ other: Encoded) {
    if other.pieces.isEmpty {
      bytes(other.tail)
      return
    }
    flush()
    pieces.append(contentsOf: other.pieces)
    tail = other.tail
    count += other.count
  }

  private mutating func flush() {
    if !tail.isEmpty {
      pieces.append(.bytes(tail))
      tail = []
    }
  }

  /// Every piece in order, the owned tail last.
  var allPieces: [Piece] {
    tail.isEmpty ? pieces : pieces + [.bytes(tail)]
  }
}

/// The size of a varint, for length prefixes.
func varintSize(_ value: UInt64) -> Int {
  var v = value
  var n = 1
  while v >= 0x80 {
    v >>= 7
    n += 1
  }
  return n
}
