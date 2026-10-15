import Foundation

/// Compares two JSON values as JSONSerialization reads them. A key the Swift
/// side leaves out matches a Python null.
public struct JSONComparison {
  public var differences: [String] = []

  public init() {}

  public mutating func compare(python: Any, swift: Any, at path: String) {
    switch (python, swift) {
    case (let p as [String: Any], let s as [String: Any]):
      for (key, value) in p where s[key] == nil {
        let child = path.isEmpty ? key : "\(path)/\(key)"
        if !(value is NSNull) {
          differences.append("only Python sends \(child)")
        }
      }
      for (key, value) in s {
        let child = path.isEmpty ? key : "\(path)/\(key)"
        guard let theirs = p[key] else {
          differences.append("only Swift has \(child)")
          continue
        }
        compare(python: theirs, swift: value, at: child)
      }
    case (let p as [Any], let s as [Any]):
      guard p.count == s.count else {
        differences.append("\(path): \(p.count) items in Python, \(s.count) in Swift")
        return
      }
      for (index, pair) in zip(p, s).enumerated() {
        compare(python: pair.0, swift: pair.1, at: "\(path)/\(index)")
      }
    default:
      if !JSONComparison.leavesEqual(python, swift) {
        differences.append("\(path): Python \(python), Swift \(swift)")
      }
    }
  }

  public static func leavesEqual(_ a: Any, _ b: Any) -> Bool {
    if a is NSNull || b is NSNull { return a is NSNull && b is NSNull }
    if let x = a as? String, let y = b as? String { return x == y }
    if let x = a as? NSNumber, let y = b as? NSNumber {
      // Outside Apple's Foundation a long number can parse as a Decimal, whose
      // doubleValue is not the nearest double; its own digits parse exactly.
      return x.doubleValue == y.doubleValue || (Double(x.description).map { $0 == Double(y.description) } ?? false)
    }
    if let x = a as? Bool, let y = b as? Bool { return x == y }
    return false
  }
}
