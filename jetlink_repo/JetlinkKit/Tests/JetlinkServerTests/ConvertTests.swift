import Foundation
import Testing

@testable import JetlinkServer

/// Every conversion path against the element loops the server began with,
/// which are what the Python queues compute: numpy's casts, round to nearest
/// even. Bit for bit, so NaN payloads, signs of zero and subnormals count.
/// The paths only vectorize in an optimized build, so run these once with
/// `swift test -c release -Xswiftc -enable-testing --filter Convert` after
/// changing one; JETLINK_EXHAUSTIVE=1 adds every float32 there is.
@Suite("Convert")
struct ConvertTests {
  /// Convert as this platform builds it, and each implementation behind it,
  /// all of which compile everywhere Float16 does.
  enum Path: String, CaseIterable, CustomTestStringConvertible {
    case convert, native, integer
    #if canImport(Accelerate)
      case vImage
    #endif

    var testDescription: String { rawValue }

    func u8ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
      switch self {
      case .convert: Convert.u8ToF16(source, destination, count: count)
      case .native: preconditionFailure("bytes have no native path")
      case .integer: IntegerHalf.u8ToF16(source, destination, count: count)
      #if canImport(Accelerate)
        case .vImage: VImageHalf.u8ToF16(source, destination, count: count)
      #endif
      }
    }

    func f32ToF16(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
      switch self {
      case .convert: Convert.f32ToF16(source, destination, count: count)
      case .native: NativeHalf.f32ToF16(source, destination, count: count)
      case .integer: IntegerHalf.f32ToF16(source, destination, count: count)
      #if canImport(Accelerate)
        case .vImage: VImageHalf.f32ToF16(source, destination, count: count)
      #endif
      }
    }

    func f16ToF32(_ source: UnsafeRawPointer, _ destination: UnsafeMutableRawPointer, count: Int) {
      switch self {
      case .convert: Convert.f16ToF32(source, destination, count: count)
      case .native: NativeHalf.f16ToF32(source, destination, count: count)
      case .integer: IntegerHalf.f16ToF32(source, destination, count: count)
      #if canImport(Accelerate)
        case .vImage: VImageHalf.f16ToF32(source, destination, count: count)
      #endif
      }
    }
  }

  // The element loops, kept here as the reference.
  private static func referenceF32ToF16(_ bits: UInt32) -> UInt16 {
    Float16(Float(bitPattern: bits)).bitPattern
  }

  private static func referenceF16ToF32(_ bits: UInt16) -> UInt32 {
    Float(Float16(bitPattern: bits)).bitPattern
  }

  /// Every float32 whose rounding to float16 is delicate: for each float16,
  /// the float32 on it, the ones either side of the halfway point to the next
  /// float16, and the halfway point itself, which ties to even; the same in
  /// the subnormal range, where the halfway points fall elsewhere; every
  /// exponent (float32 subnormals, overflow, infinities, NaN payloads quiet
  /// and signaling) with fractions that stress the rounding; both signs of
  /// each; then random bit patterns and random values in float16's range.
  static let f32Cases: [UInt32] = {
    var positive: [UInt32] = []
    for h in UInt32(0x400)..<0x7C00 {
      let exact = ((h >> 10) + 112) << 23 | (h & 0x3FF) << 13
      positive += [exact, exact + 0x0FFF, exact + 0x1000, exact + 0x1001]
    }
    let tiny = Float(Float16.leastNonzeroMagnitude)
    for m in 0...1024 {
      let tie = (Float(m) + 0.5) * tiny
      positive += [(Float(m) * tiny).bitPattern, tie.bitPattern, tie.nextUp.bitPattern, tie.nextDown.bitPattern]
    }
    for exponent in UInt32(0)...255 {
      for fraction: UInt32 in [0, 1, 0x0FFF, 0x1000, 0x1001, 0x1FFF, 0x2000, 0x3F_E000, 0x3F_FFFF, 0x40_0000, 0x40_1000, 0x7F_E000, 0x7F_FFFF] {
        positive.append(exponent << 23 | fraction)
      }
    }
    var cases = positive + positive.map { $0 | 0x8000_0000 }
    var generator = SystemRandomNumberGenerator()
    for _ in 0..<100_000 {
      cases.append(UInt32.random(in: 0...UInt32.max, using: &generator))
    }
    for _ in 0..<10_000 {
      cases.append(Float.random(in: -70000...70000, using: &generator).bitPattern)
    }
    return cases
  }()

  static let f16Cases: [UInt16] = Array(0...UInt16.max)

  @Test("uint8 to float16 is exact for every byte, whatever the length and alignment", arguments: Path.allCases.filter { $0 != .native })
  func u8ToF16(path: Path) {
    let src = (0..<1027).map { UInt8(truncatingIfNeeded: $0 * 7 + $0 / 256) }
    #expect(Set(src).count == 256)
    let reference = src.map { Float16(Float($0)).bitPattern }
    for (offset, count) in [(0, 1027), (1, 1026), (3, 17), (5, 1), (0, 0)] {
      var out = [UInt16](repeating: 0xFFFF, count: 1027)
      src.withUnsafeBytes { s in out.withUnsafeMutableBytes { d in path.u8ToF16(s.baseAddress! + offset, d.baseAddress!, count: count) } }
      #expect(Array(out[..<count]) == Array(reference[offset..<offset + count]), "offset \(offset) count \(count)")
      #expect(out[count...].allSatisfy { $0 == 0xFFFF }, "wrote past \(count)")
    }
  }

  @Test("float32 to float16 rounds as the element loop, edges included", arguments: Path.allCases)
  func f32ToF16(path: Path) {
    let src = ConvertTests.f32Cases
    var out = [UInt16](repeating: 0xFFFF, count: src.count)
    src.withUnsafeBytes { s in out.withUnsafeMutableBytes { d in path.f32ToF16(s.baseAddress!, d.baseAddress!, count: src.count) } }
    let differences = zip(src, out).filter { ConvertTests.referenceF32ToF16($0) != $1 }
    #expect(
      differences.isEmpty,
      "\(differences.count) differ, first \(differences.first.map { "0x\(String($0, radix: 16)) -> 0x\(String($1, radix: 16))" } ?? "")")
  }

  @Test("float32 to float16 reads an unaligned source and stops at count", arguments: Path.allCases)
  func f32ToF16Unaligned(path: Path) {
    let src = Array(ConvertTests.f32Cases.prefix(1037))
    var bytes = [UInt8](repeating: 0, count: src.count * 4 + 1)
    bytes.withUnsafeMutableBytes { b in
      src.withUnsafeBytes { s in b.baseAddress!.advanced(by: 1).copyMemory(from: s.baseAddress!, byteCount: s.count) }
    }
    var out = [UInt16](repeating: 0xFFFF, count: src.count + 1)
    bytes.withUnsafeBytes { b in out.withUnsafeMutableBytes { d in path.f32ToF16(b.baseAddress! + 1, d.baseAddress!, count: src.count) } }
    #expect(Array(out.prefix(src.count)) == src.map(ConvertTests.referenceF32ToF16))
    #expect(out.last == 0xFFFF)
  }

  @Test("float16 to float32 is exact for every float16 bit pattern, aligned or not", arguments: Path.allCases)
  func f16ToF32(path: Path) {
    let src = ConvertTests.f16Cases
    let reference = src.map(ConvertTests.referenceF16ToF32)
    var out = [UInt32](repeating: 0, count: src.count)
    src.withUnsafeBytes { s in out.withUnsafeMutableBytes { d in path.f16ToF32(s.baseAddress!, d.baseAddress!, count: src.count) } }
    let differences = zip(src, out).filter { ConvertTests.referenceF16ToF32($0) != $1 }
    #expect(
      differences.isEmpty,
      "\(differences.count) differ, first \(differences.first.map { "0x\(String($0, radix: 16)) -> 0x\(String($1, radix: 16))" } ?? "")")

    var bytes = [UInt8](repeating: 0, count: 2 * 1031 + 1)
    bytes.withUnsafeMutableBytes { b in
      src.withUnsafeBytes { s in b.baseAddress!.advanced(by: 1).copyMemory(from: s.baseAddress! + 2 * 0x7BF0, byteCount: 2 * 1031) }
    }
    var shifted = [UInt32](repeating: 0, count: 1032)
    bytes.withUnsafeBytes { b in shifted.withUnsafeMutableBytes { d in path.f16ToF32(b.baseAddress! + 1, d.baseAddress!, count: 1031) } }
    #expect(Array(shifted.prefix(1031)) == Array(reference[0x7BF0..<0x7BF0 + 1031]))
    #expect(shifted.last == 0)
  }

  @Test("An empty conversion touches nothing", arguments: Path.allCases)
  func empty(path: Path) {
    var out: [UInt32] = [7]
    let src: [UInt32] = [0x3F80_0000]
    src.withUnsafeBytes { s in
      out.withUnsafeMutableBytes { d in
        if path != .native { path.u8ToF16(s.baseAddress!, d.baseAddress!, count: 0) }
        path.f32ToF16(s.baseAddress!, d.baseAddress!, count: 0)
        path.f16ToF32(s.baseAddress!, d.baseAddress!, count: 0)
      }
    }
    #expect(out == [7])
  }

  @Test("float16 back to uint8 is exact for every byte an image queue holds, and stops at count")
  func f16ToU8() {
    let bytes = (0...255).map { UInt8($0) }
    var halves = [UInt16](repeating: 0, count: 256)
    bytes.withUnsafeBytes { s in halves.withUnsafeMutableBytes { d in Convert.u8ToF16(s.baseAddress!, d.baseAddress!, count: 256) } }
    var back = [UInt8](repeating: 7, count: 257)
    halves.withUnsafeBufferPointer { h in back.withUnsafeMutableBytes { d in Convert.f16ToU8(h.baseAddress!, d.baseAddress!, count: 256) } }
    #expect(Array(back.prefix(256)) == bytes)
    #expect(back[256] == 7)
  }

  @Test("allFinite finds any NaN or infinity, wherever it is")
  func allFinite() {
    let finite: [Float] = (0..<1029).map { Float($0) * 1.5e35 - 7.7e37 } + [.greatestFiniteMagnitude, -.greatestFiniteMagnitude, .leastNonzeroMagnitude, -0.0]
    let check = { (values: [Float], count: Int) in values.withUnsafeBytes { Convert.allFinite($0.baseAddress!, count: count) } }
    #expect(check(finite, finite.count))
    #expect(check([], 0))
    let bad: [Float] = [.infinity, -.infinity, .nan, -.nan, .signalingNaN, Float(bitPattern: 0x7F80_0001), Float(bitPattern: 0xFFFF_FFFF)]
    for value in bad {
      for index in [0, 1, 15, 16, 17, 512, finite.count - 1] {
        var values = finite
        values[index] = value
        #expect(!check(values, values.count), "\(value) at \(index)")
        #expect(check(values, index), "\(value) just past count \(index)")
      }
    }
    // Unaligned, as a reply buffer never is, but the loop does not care.
    var bytes = [UInt8](repeating: 0, count: 4 * 33 + 1)
    bytes[1 + 4 * 32 + 3] = 0x7F
    bytes[1 + 4 * 32 + 2] = 0x80
    bytes.withUnsafeBytes { b in
      #expect(!Convert.allFinite(b.baseAddress! + 1, count: 33))
      #expect(Convert.allFinite(b.baseAddress! + 1, count: 32))
    }
  }

  @Test(
    "float32 to float16 matches the element loop for every float32",
    .enabled(if: ProcessInfo.processInfo.environment["JETLINK_EXHAUSTIVE"] != nil), arguments: Path.allCases)
  func exhaustive(path: Path) {
    let chunk = 1 << 24
    let src = UnsafeMutablePointer<UInt32>.allocate(capacity: chunk)
    let out = UnsafeMutablePointer<UInt16>.allocate(capacity: chunk)
    defer {
      src.deallocate()
      out.deallocate()
    }
    var differences = 0
    for base in stride(from: 0, to: 1 << 32, by: chunk) {
      for i in 0..<chunk { src[i] = UInt32(base + i) }
      path.f32ToF16(src, out, count: chunk)
      for i in 0..<chunk where out[i] != ConvertTests.referenceF32ToF16(src[i]) {
        differences += 1
      }
    }
    #expect(differences == 0)
  }
}
