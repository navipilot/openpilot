import Foundation
import Testing

@testable import JetlinkONNX

#if canImport(CryptoKit)
  import CryptoKit
#else
  import Crypto
#endif

/// The fp16 to fp32 widening heads_in_fp32 relies on, held to numpy's
/// `astype(np.float32)` bit for bit.
@Suite struct WidenTests {
  static func widen(_ bits: [UInt16]) -> [UInt32] {
    var source: [UInt8] = []
    for b in bits {
      source.append(UInt8(truncatingIfNeeded: b))
      source.append(UInt8(truncatingIfNeeded: b >> 8))
    }
    var out = [UInt8](repeating: 0, count: bits.count * 4)
    source.withUnsafeBytes { from in
      out.withUnsafeMutableBytes { to in
        PartWriter.widenBand(from.baseAddress!, to.baseAddress!, bits.count)
      }
    }
    return (0..<bits.count).map { i -> UInt32 in
      var v: UInt32 = 0
      for b in 0..<4 {
        v |= UInt32(out[i * 4 + b]) << UInt32(8 * b)
      }
      return v
    }
  }

  @Test func specialValues() {
    let cases: [(UInt16, UInt32)] = [
      (0x0000, 0x0000_0000),  // 0
      (0x8000, 0x8000_0000),  // -0
      (0x3c00, 0x3f80_0000),  // 1
      (0x3000, 0x3e00_0000),  // 1/8, the prescale constant
      (0x0001, 0x3380_0000),  // the smallest subnormal
      (0x03ff, 0x387f_c000),  // the largest subnormal
      (0x7bff, 0x477f_e000),  // the largest finite
      (0x7c00, 0x7f80_0000),  // inf
      (0xfc00, 0xff80_0000),  // -inf
      (0x7e00, 0x7fc0_0000),  // quiet NaN
      (0x7e01, 0x7fc0_2000),  // quiet NaN with a payload
      (0x7c01, 0x7fc0_2000),  // signalling NaN: quieted, payload kept
      (0xfc01, 0xffc0_2000),
    ]
    #expect(Self.widen(cases.map(\.0)) == cases.map(\.1))
  }

  /// Every fp16 bit pattern: the SHA-256 of numpy 2.5.3's table on Apple
  /// silicon (`np.arange(65536).astype(np.uint16).view(np.float16).astype(np.float32)`
  /// as little-endian bytes), 2026-09-27.
  @Test func everyPatternMatchesNumpy() {
    let all = Self.widen((0..<65536).map { UInt16($0) })
    var bytes: [UInt8] = []
    bytes.reserveCapacity(all.count * 4)
    for v in all {
      withUnsafeBytes(of: v.littleEndian) { bytes.append(contentsOf: $0) }
    }
    let digest = SHA256.hash(data: Data(bytes)).map { String(format: "%02x", $0) }.joined()
    #expect(digest == "b636c5716ff84d972782faf02d0194cb8951526bea4cc487082feb47b1860ddf")
  }

  @Test func widenedDataCountsFourBytesAnElement() {
    let w = Widen(elements: .owned([0, 0x3c, 0, 0x30]), count: 2)
    #expect(Tensor.Data.widened(w).count == 8)
    var e = Encoded()
    e.widened(w)
    #expect(e.count == 8)
  }
}
