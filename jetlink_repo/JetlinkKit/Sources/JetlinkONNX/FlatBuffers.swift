import Foundation

/// A FlatBuffers builder, enough of one to write a TFLite model: tables with
/// scalar, offset and union fields, vectors of scalars and of offsets, and
/// strings. It is written back to front, as the reference builders are, so a
/// child is always finished before the table that points at it and every
/// offset points forward. Vtables that come out the same are written once.
///
/// The format (flatbuffers.dev, "FlatBuffers internals"): little-endian
/// scalars, each aligned to its own size; a table starts with the signed
/// distance back to its vtable, which lists each field's position in the
/// table (0 for a field left out, which then reads as its default); a vector
/// or string is its 32-bit length followed by its elements (and a string by
/// a NUL); a reference is an unsigned 32-bit distance forward from where it
/// is stored. The buffer opens with the reference to the root table and the
/// four-byte file identifier.
struct FlatBufferBuilder {
  /// Where something already written sits, as its distance from the end of
  /// the buffer. That distance does not change as the buffer grows at the
  /// front, which is why everything is measured from the end.
  struct Offset: Hashable {
    let value: UInt32
  }

  /// The bytes live at the end of `storage`, from `head` on.
  private var storage: [UInt8]
  private var head: Int
  /// The largest alignment anything asked for; the finished buffer's length
  /// is a multiple of it, so alignment from the end is alignment from the start.
  private(set) var minAlign = 1
  /// The open table's fields: where each one was written, or 0.
  private var fields: [UInt32] = []
  private var tableStart: UInt32?
  /// Every vtable written so far, by its bytes.
  private var vtables: [[UInt16]: UInt32] = [:]

  init(capacity: Int = 1 << 16) {
    storage = [UInt8](repeating: 0, count: max(capacity, 64))
    head = storage.count
  }

  /// Bytes written so far.
  var size: Int { storage.count - head }

  private var offsetNow: UInt32 { UInt32(size) }

  // MARK: bytes

  /// Makes room for `n` more bytes in front, doubling the storage as needed.
  private mutating func reserve(_ n: Int) {
    guard head < n else { return }
    let used = size
    var capacity = storage.count
    while capacity - used < n { capacity *= 2 }
    var grown = [UInt8](repeating: 0, count: capacity)
    grown.withUnsafeMutableBytes { to in
      storage.withUnsafeBytes { from in
        to.baseAddress!.advanced(by: capacity - used).copyMemory(from: from.baseAddress!.advanced(by: head), byteCount: used)
      }
    }
    storage = grown
    head = capacity - used
  }

  /// Pads so that, once `additional` more bytes are written, what comes next
  /// starts on a multiple of `alignment`.
  mutating func prep(_ alignment: Int, _ additional: Int) {
    minAlign = max(minAlign, alignment)
    let padding = (~(size + additional) + 1) & (alignment - 1)
    reserve(padding + alignment + additional)
    for _ in 0..<padding {
      head -= 1
      storage[head] = 0
    }
  }

  /// Writes a scalar in the room `prep` made.
  private mutating func place<T: FixedWidthInteger>(_ value: T) {
    let n = MemoryLayout<T>.size
    head -= n
    storage.withUnsafeMutableBytes { $0.storeBytes(of: value.littleEndian, toByteOffset: head, as: T.self) }
  }

  mutating func push<T: FixedWidthInteger>(_ value: T) {
    prep(MemoryLayout<T>.size, 0)
    place(value)
  }

  /// A reference to `target`, stored here.
  mutating func push(_ target: Offset) {
    prep(4, 0)
    precondition(target.value <= offsetNow, "a FlatBuffers reference has to point at something already written")
    place(offsetNow + 4 - target.value)
  }

  // MARK: tables

  mutating func startTable(fields count: Int) {
    precondition(tableStart == nil, "a table is already open")
    fields = [UInt32](repeating: 0, count: count)
    tableStart = offsetNow
  }

  /// A scalar field, left out when it equals the schema's default, as the
  /// reference builders do; `force` writes it anyway.
  mutating func add<T: FixedWidthInteger>(_ slot: Int, _ value: T, default fallback: T = 0, force: Bool = false) {
    guard force || value != fallback else { return }
    push(value)
    fields[slot] = offsetNow
  }

  mutating func add(_ slot: Int, _ value: Bool, default fallback: Bool = false) {
    add(slot, UInt8(value ? 1 : 0), default: fallback ? 1 : 0)
  }

  mutating func add(_ slot: Int, _ value: Float, default fallback: Float = 0) {
    guard value != fallback else { return }
    push(value.bitPattern)
    fields[slot] = offsetNow
  }

  /// A reference field; nil leaves it out.
  mutating func add(_ slot: Int, _ target: Offset?) {
    guard let target else { return }
    push(target)
    fields[slot] = offsetNow
  }

  mutating func endTable() -> Offset {
    guard let start = tableStart else { preconditionFailure("no table is open") }
    // Where the vtable distance goes; filled in once the vtable has a place.
    push(Int32(0))
    let table = offsetNow
    var count = fields.count
    while count > 0, fields[count - 1] == 0 { count -= 1 }
    var vtable: [UInt16] = [UInt16((count + 2) * 2), UInt16(table - start)]
    for slot in 0..<count {
      vtable.append(fields[slot] == 0 ? 0 : UInt16(table - fields[slot]))
    }
    let at: UInt32
    if let existing = vtables[vtable] {
      at = existing
    } else {
      for entry in vtable.reversed() { push(entry) }
      at = offsetNow
      vtables[vtable] = at
    }
    // The table's first word is its position minus its vtable's.
    let distance = Int32(Int64(at) - Int64(table))
    let position = storage.count - Int(table)
    storage.withUnsafeMutableBytes { $0.storeBytes(of: distance.littleEndian, toByteOffset: position, as: Int32.self) }
    tableStart = nil
    fields = []
    return Offset(value: table)
  }

  // MARK: vectors and strings

  /// A vector of scalars. `alignment` is for the elements, for a schema field
  /// declared with force_align.
  mutating func vector<T: FixedWidthInteger>(_ values: [T], alignment: Int = MemoryLayout<T>.size) -> Offset {
    let bytes = values.count * MemoryLayout<T>.size
    prep(4, bytes)
    prep(max(alignment, MemoryLayout<T>.size), bytes)
    for v in values.reversed() { place(v) }
    place(UInt32(values.count))
    return Offset(value: offsetNow)
  }

  /// A vector of bytes copied in one go: a buffer's data.
  mutating func bytes(_ data: UnsafeRawBufferPointer, alignment: Int = 1) -> Offset {
    prep(4, data.count)
    prep(alignment, data.count)
    head -= data.count
    if data.count > 0 {
      storage.withUnsafeMutableBytes { $0.baseAddress!.advanced(by: head).copyMemory(from: data.baseAddress!, byteCount: data.count) }
    }
    place(UInt32(data.count))
    return Offset(value: offsetNow)
  }

  /// A vector of references.
  mutating func vector(_ targets: [Offset]) -> Offset {
    prep(4, targets.count * 4)
    for t in targets.reversed() {
      place(offsetNow + 4 - t.value)
    }
    place(UInt32(targets.count))
    return Offset(value: offsetNow)
  }

  mutating func string(_ s: String) -> Offset {
    let utf8 = Array(s.utf8)
    prep(4, utf8.count + 1)
    place(UInt8(0))
    utf8.withUnsafeBytes { buf in
      head -= buf.count
      if buf.count > 0 {
        storage.withUnsafeMutableBytes { $0.baseAddress!.advanced(by: head).copyMemory(from: buf.baseAddress!, byteCount: buf.count) }
      }
    }
    place(UInt32(utf8.count))
    return Offset(value: offsetNow)
  }

  /// The finished buffer: the root reference, the file identifier, and
  /// everything written.
  mutating func finish(_ root: Offset, identifier: String) -> [UInt8] {
    let id = Array(identifier.utf8)
    precondition(id.count == 4, "a FlatBuffers file identifier is four bytes")
    prep(minAlign, 8)
    for b in id.reversed() { place(b) }
    push(root)
    return Array(storage[head...])
  }
}
