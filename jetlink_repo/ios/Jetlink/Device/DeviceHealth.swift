import Foundation
import JetlinkUI
import SwiftUI

/// What the phone itself is doing, for the tiles under the frame budget. A
/// phone in a car mount in the sun throttles, and a throttled phone misses
/// frames long before the numbers above say why.
struct DeviceHealth: Equatable, Sendable {
  enum Power: Equatable, Sendable {
    case unknown, unplugged, charging, full
  }

  var thermal: ThermalLevel = .nominal
  /// 0 to 1, nil where the level is not known.
  var batteryLevel: Double?
  var power: Power = .unknown
  var lowPowerMode = false
  /// What the app may still allocate before iOS ends it, in bytes; nil
  /// where the system does not say. A model and its CoreML compile take
  /// most of the phone's share.
  var availableMemory: Int64?

  static let lowMemory: Int64 = 1 << 30

  /// "2.4", in GB, beside the unit; "--" when unknown.
  var memoryValue: String {
    guard let availableMemory else { return "--" }
    return (Double(availableMemory) / 1e9).formatted(.number.precision(.fractionLength(1)))
  }

  var memoryNote: String? {
    guard let availableMemory else { return nil }
    return availableMemory < DeviceHealth.lowMemory ? "Low" : "Free"
  }

  var memoryTone: Color {
    guard let availableMemory else { return .secondary }
    return availableMemory < DeviceHealth.lowMemory ? .orange : .secondary
  }

  /// "82", the unit set beside it; "--" when unknown.
  var batteryValue: String {
    guard let batteryLevel else { return "--" }
    return Int((batteryLevel * 100).rounded()).formatted()
  }

  var powerText: String {
    switch power {
    case .unknown: "Unknown"
    case .unplugged: lowPowerMode ? "Low Power Mode" : "Not Charging"
    case .charging: "Charging"
    case .full: "Charged"
    }
  }

  var batterySymbol: String {
    switch power {
    case .charging, .full: return "battery.100percent.bolt"
    case .unknown: return "battery.0percent"
    case .unplugged:
      let level = batteryLevel ?? 0
      if level > 0.75 { return "battery.100percent" }
      if level > 0.5 { return "battery.75percent" }
      if level > 0.25 { return "battery.50percent" }
      if level > 0.1 { return "battery.25percent" }
      return "battery.0percent"
    }
  }

  /// A phone running a model twenty times a second belongs on power.
  var batteryTone: Color {
    switch power {
    case .charging, .full, .unknown: .secondary
    case .unplugged: (batteryLevel ?? 1) < 0.2 ? .red : .orange
    }
  }
}
