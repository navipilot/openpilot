import Foundation

/// The bytes a string of hex digits spells.
public func hex(_ s: String) -> [UInt8] {
  var out: [UInt8] = []
  var chars = Array(s)
  while chars.count >= 2 {
    out.append(UInt8(String(chars[0..<2]), radix: 16)!)
    chars.removeFirst(2)
  }
  return out
}

/// Bytes as lowercase hex digits.
public func hexString(_ bytes: [UInt8]) -> String {
  bytes.map { String(format: "%02x", $0) }.joined()
}
