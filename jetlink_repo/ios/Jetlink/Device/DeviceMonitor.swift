import Foundation
import JetlinkUI
import Observation
import UIKit
import os

/// Keeps `DeviceHealth` current: the thermal state, the battery, Low Power
/// Mode, and once a second the memory left to the app.
@MainActor
@Observable
final class DeviceMonitor {
  private(set) var health = DeviceHealth()
  /// Told when the phone is in trouble the logs should record: memory
  /// pressure, and heat that throttles.
  @ObservationIgnored var warn: (String) -> Void = { _ in }
  @ObservationIgnored private var observers: [NSObjectProtocol] = []
  @ObservationIgnored private var memoryTask: Task<Void, Never>?

  init() {
    UIDevice.current.isBatteryMonitoringEnabled = true
    refresh()
    let center = NotificationCenter.default
    observers.append(
      center.addObserver(forName: UIApplication.didReceiveMemoryWarningNotification, object: nil, queue: .main) { [weak self] _ in
        MainActor.assumeIsolated { self?.warn("the system reports memory pressure; a model being prepared may not fit") }
      })
    memoryTask = Task { [weak self] in
      while !Task.isCancelled {
        try? await Task.sleep(for: .seconds(1))
        self?.pollMemory()
      }
    }
    for name in [
      ProcessInfo.thermalStateDidChangeNotification,
      Notification.Name.NSProcessInfoPowerStateDidChange,
      UIDevice.batteryLevelDidChangeNotification,
      UIDevice.batteryStateDidChangeNotification,
    ] {
      observers.append(
        center.addObserver(forName: name, object: nil, queue: .main) { [weak self] _ in
          MainActor.assumeIsolated { self?.refresh() }
        })
    }
  }

  private func pollMemory() {
    let available = Int64(os_proc_available_memory())
    guard available > 0, health.availableMemory != available else { return }
    health.availableMemory = available
  }

  func refresh() {
    let device = UIDevice.current
    let power: DeviceHealth.Power =
      switch device.batteryState {
      case .charging: .charging
      case .full: .full
      case .unplugged: .unplugged
      case .unknown: .unknown
      @unknown default: .unknown
      }
    let thermal = ThermalLevel(ProcessInfo.processInfo.thermalState)
    if thermal.note != nil && health.thermal.note == nil {
      warn("the \(ThisDevice.name) is \(thermal.title.lowercased()); the system slows the chip to cool it, and frames may miss 50 ms")
    }
    let lowPower = ProcessInfo.processInfo.isLowPowerModeEnabled
    if lowPower && !health.lowPowerMode {
      warn("Low Power Mode is on; the system slows the chip to save power, and frames may miss 50 ms")
    }
    health = DeviceHealth(
      thermal: thermal,
      batteryLevel: device.batteryLevel >= 0 ? Double(device.batteryLevel) : nil,
      power: power,
      lowPowerMode: lowPower,
      availableMemory: health.availableMemory)
  }
}

/// The device in hand. Jetlink serves the comma the same way from an iPhone
/// and an iPad; the words that name the device follow it.
@MainActor
enum ThisDevice {
  /// "iPhone" or "iPad": "your iPad", and the Status tab's section.
  static var name: String {
    isPad ? "iPad" : "iPhone"
  }

  static var isPad: Bool {
    UIDevice.current.userInterfaceIdiom == .pad
  }
}
