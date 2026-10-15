import Foundation

/// Python's `round(value, digits)`: the decimal nearest the exact binary value,
/// halves to even. `(value * 100).rounded() / 100` is not the same: it rounds
/// halves away from zero, after a multiply that can itself land on a half,
/// so 1.125 became 1.13 and 2.675 became 2.68 where Python says 1.12 and 2.67.
/// The events both servers publish have to read the same, so the Swift one
/// rounds here. C's `%.*f` is correctly rounded on both Darwin and glibc,
/// which the conformance tests check against Python's own numbers.
public func pythonRound(_ value: Double, _ digits: Int) -> Double {
  guard value.isFinite else { return value }
  return Double(String(format: "%.\(digits)f", value)) ?? value
}
