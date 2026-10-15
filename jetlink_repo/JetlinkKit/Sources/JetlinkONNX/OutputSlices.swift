import Foundation

/// One entry of openpilot's output_slices: where a named output sits in the
/// model's flat output vector, as Python's `slice` holds it. Either end may
/// be None, and either may count from the end.
public struct OutputSlice: Sendable, Equatable {
  public let name: String
  public let start: Int?
  public let stop: Int?

  public init(name: String, start: Int?, stop: Int?) {
    self.name = name
    self.start = start
    self.stop = stop
  }
}

/// Reads output_slices, which openpilot stores in metadata_props as base64 of
/// a Python pickle of `{str: slice(start, stop, None)}`.
///
/// This is a pickle VM for exactly that shape and nothing more: protocols 2
/// to 5, a dict of str keys to slices built by calling builtins.slice. Any
/// other opcode is refused by name rather than guessed at, and nothing a
/// pickle names is ever called; `slice` is recognised, not imported.
public enum OutputSlices {
  public static func decode(base64: String) throws -> [OutputSlice] {
    // codecs' base64 wraps lines every 76 characters; the newlines are not data.
    guard let data = Data(base64Encoded: base64, options: .ignoreUnknownCharacters) else {
      throw OnnxError("output_slices is not valid base64")
    }
    return try decode(pickle: data)
  }

  public static func decode(pickle: Data) throws -> [OutputSlice] {
    var vm = PickleVM(Array(pickle))
    let result = try vm.run()
    guard case .dict(let dict) = result else {
      throw OnnxError("output_slices pickle holds \(result.kind), not a dict")
    }
    return try dict.items.map { key, value in
      guard case .str(let name) = key else {
        throw OnnxError("output_slices pickle has a \(key.kind) key; expected str")
      }
      guard case .slice(let start, let stop, let step) = value else {
        throw OnnxError("output_slices entry \(name) is \(value.kind), not a slice")
      }
      func bound(_ value: PickleValue) throws -> Int? {
        switch value {
        case .int(let n): return n
        case .none: return nil
        default: throw OnnxError("output_slices entry \(name) has \(value.kind) for a bound; expected an int or None")
        }
      }
      guard step == .none || step == .int(1) else {
        throw OnnxError("output_slices entry \(name) has a step; expected None")
      }
      return OutputSlice(name: name, start: try bound(start), stop: try bound(stop))
    }
  }
}

// MARK: the VM

private final class PickleDict {
  var items: [(key: PickleValue, value: PickleValue)] = []

  /// A Python dict: a repeated key keeps its first position and takes the
  /// last value.
  func set(_ key: PickleValue, _ value: PickleValue) {
    if let i = items.firstIndex(where: { $0.key == key }) {
      items[i].value = value
    } else {
      items.append((key, value))
    }
  }
}

private indirect enum PickleValue: Equatable {
  case none
  case int(Int)
  case str(String)
  case global(module: String, name: String)
  case tuple([PickleValue])
  case slice(PickleValue, PickleValue, PickleValue)
  case dict(PickleDict)

  var kind: String {
    switch self {
    case .none: "None"
    case .int: "an int"
    case .str: "a str"
    case .global(let m, let n): "the global \(m).\(n)"
    case .tuple: "a tuple"
    case .slice: "a slice"
    case .dict: "a dict"
    }
  }

  var isSliceCallable: Bool {
    if case .global(let module, let name) = self {
      // Protocol 2 writes the Python 2 module name for builtins.
      return name == "slice" && (module == "builtins" || module == "__builtin__")
    }
    return false
  }

  static func == (a: PickleValue, b: PickleValue) -> Bool {
    switch (a, b) {
    case (.none, .none): true
    case (.int(let x), .int(let y)): x == y
    case (.str(let x), .str(let y)): x == y
    case (.global(let m1, let n1), .global(let m2, let n2)): m1 == m2 && n1 == n2
    case (.tuple(let x), .tuple(let y)): x == y
    case (.slice(let a1, let b1, let c1), .slice(let a2, let b2, let c2)): a1 == a2 && b1 == b2 && c1 == c2
    case (.dict(let x), .dict(let y)): x === y
    default: false
    }
  }
}

private struct PickleVM {
  private let bytes: [UInt8]
  private var pos = 0
  private var stack: [PickleValue] = []
  private var marks: [Int] = []
  private var memo: [Int: PickleValue] = [:]

  init(_ bytes: [UInt8]) {
    self.bytes = bytes
  }

  mutating func run() throws -> PickleValue {
    while pos < bytes.count {
      let at = pos
      let op = bytes[pos]
      pos += 1
      switch op {
      case 0x80:  // PROTO
        let version = try take(1)[0]
        guard (2...5).contains(version) else { throw fail("pickle protocol \(version); expected 2 to 5") }
      case 0x95:  // FRAME: a length hint for buffered readers, nothing to do
        _ = try take(8)
      case 0x7d:  // EMPTY_DICT
        stack.append(.dict(PickleDict()))
      case 0x28:  // MARK
        marks.append(stack.count)
      case 0x73:  // SETITEM
        let value = try pop()
        let key = try pop()
        try dict(at: stack.count - 1).set(key, value)
      case 0x75:  // SETITEMS
        let items = try popMark()
        guard items.count % 2 == 0 else { throw fail("SETITEMS with an odd number of items") }
        let d = try dict(at: stack.count - 1)
        for i in stride(from: 0, to: items.count, by: 2) {
          d.set(items[i], items[i + 1])
        }
      case 0x58:  // BINUNICODE
        stack.append(.str(try string(Int(try uint(4)))))
      case 0x8c:  // SHORT_BINUNICODE
        stack.append(.str(try string(Int(try uint(1)))))
      case 0x8d:  // BINUNICODE8
        let n = try uint(8)
        guard n <= UInt64(bytes.count) else { throw fail("a string longer than the pickle") }
        stack.append(.str(try string(Int(n))))
      case 0x63:  // GLOBAL: "module\nname\n"
        let module = try line()
        let name = try line()
        stack.append(.global(module: module, name: name))
      case 0x93:  // STACK_GLOBAL
        let name = try pop()
        let module = try pop()
        guard case .str(let n) = name, case .str(let m) = module else {
          throw fail("STACK_GLOBAL needs two strings")
        }
        stack.append(.global(module: m, name: n))
      case 0x4a:  // BININT: four bytes, signed
        stack.append(.int(Int(Int32(bitPattern: UInt32(try uint(4))))))
      case 0x4b:  // BININT1
        stack.append(.int(Int(try uint(1))))
      case 0x4d:  // BININT2
        stack.append(.int(Int(try uint(2))))
      case 0x8a:  // LONG1: a length byte, then two's complement little-endian
        let n = Int(try take(1)[0])
        stack.append(.int(try long(try take(n))))
      case 0x4e:  // NONE
        stack.append(.none)
      case 0x74:  // TUPLE
        stack.append(.tuple(try popMark()))
      case 0x85:  // TUPLE1
        stack.append(.tuple(try pop(1)))
      case 0x86:  // TUPLE2
        stack.append(.tuple(try pop(2)))
      case 0x87:  // TUPLE3
        stack.append(.tuple(try pop(3)))
      case 0x52:  // REDUCE
        let args = try pop()
        let callable = try pop()
        guard callable.isSliceCallable else { throw fail("REDUCE of \(callable.kind); only builtins.slice is allowed") }
        guard case .tuple(let a) = args else { throw fail("REDUCE with \(args.kind) for arguments") }
        switch a.count {
        case 1: stack.append(.slice(.none, a[0], .none))
        case 2: stack.append(.slice(a[0], a[1], .none))
        case 3: stack.append(.slice(a[0], a[1], a[2]))
        default: throw fail("slice() with \(a.count) arguments")
        }
      case 0x94:  // MEMOIZE
        guard let top = stack.last else { throw fail("MEMOIZE on an empty stack") }
        memo[memo.count] = top
      case 0x71:  // BINPUT
        guard let top = stack.last else { throw fail("BINPUT on an empty stack") }
        memo[Int(try uint(1))] = top
      case 0x72:  // LONG_BINPUT
        guard let top = stack.last else { throw fail("LONG_BINPUT on an empty stack") }
        memo[Int(try uint(4))] = top
      case 0x68:  // BINGET
        stack.append(try recall(Int(try uint(1))))
      case 0x6a:  // LONG_BINGET
        stack.append(try recall(Int(try uint(4))))
      case 0x2e:  // STOP
        guard stack.count == 1, marks.isEmpty else { throw fail("STOP with \(stack.count) values on the stack") }
        return stack[0]
      default:
        let printable = (0x21...0x7e).contains(op) ? " ('\(Character(UnicodeScalar(op)))')" : ""
        throw OnnxError("output_slices pickle: unsupported opcode 0x\(String(op, radix: 16))\(printable) at offset \(at)")
      }
    }
    throw fail("the pickle ends without STOP")
  }

  private func fail(_ message: String) -> OnnxError {
    OnnxError("output_slices pickle: \(message) (offset \(pos))")
  }

  private mutating func take(_ n: Int) throws -> [UInt8] {
    guard n >= 0, bytes.count - pos >= n else { throw fail("truncated") }
    defer { pos += n }
    return Array(bytes[pos..<(pos + n)])
  }

  private mutating func uint(_ n: Int) throws -> UInt64 {
    var value: UInt64 = 0
    for (i, b) in try take(n).enumerated() {
      value |= UInt64(b) << (8 * UInt64(i))
    }
    return value
  }

  private func long(_ digits: [UInt8]) throws -> Int {
    guard !digits.isEmpty else { return 0 }
    guard digits.count <= 8 else { throw fail("an int wider than 64 bits") }
    var value: UInt64 = 0
    for (i, b) in digits.enumerated() {
      value |= UInt64(b) << (8 * UInt64(i))
    }
    // Sign-extend from the top byte's high bit.
    let bits = UInt64(digits.count * 8)
    if bits < 64, digits.last! & 0x80 != 0 {
      value |= ~UInt64(0) << bits
    }
    return Int(Int64(bitPattern: value))
  }

  private mutating func string(_ n: Int) throws -> String {
    let raw = try take(n)
    guard let s = String(validating: raw, as: UTF8.self) else { throw fail("a string that is not UTF-8") }
    return s
  }

  private mutating func line() throws -> String {
    guard let nl = bytes[pos...].firstIndex(of: 0x0a) else { throw fail("GLOBAL without a newline") }
    let raw = bytes[pos..<nl]
    pos = nl + 1
    guard let s = String(validating: raw, as: UTF8.self) else { throw fail("a GLOBAL name that is not UTF-8") }
    return s
  }

  private mutating func pop() throws -> PickleValue {
    guard let v = stack.popLast() else { throw fail("pop from an empty stack") }
    if let m = marks.last, stack.count < m { throw fail("pop past a MARK") }
    return v
  }

  private mutating func pop(_ n: Int) throws -> [PickleValue] {
    guard stack.count >= n else { throw fail("pop from an empty stack") }
    if let m = marks.last, stack.count - n < m { throw fail("pop past a MARK") }
    let values = Array(stack.suffix(n))
    stack.removeLast(n)
    return values
  }

  private mutating func popMark() throws -> [PickleValue] {
    guard let m = marks.popLast() else { throw fail("no MARK to pop to") }
    let values = Array(stack[m...])
    stack.removeSubrange(m...)
    return values
  }

  private func recall(_ index: Int) throws -> PickleValue {
    guard let v = memo[index] else { throw fail("memo \(index) was never stored") }
    return v
  }

  private func dict(at index: Int) throws -> PickleDict {
    guard index >= 0, index < stack.count, case .dict(let d) = stack[index] else {
      throw fail("SETITEM on something that is not a dict")
    }
    return d
  }
}
