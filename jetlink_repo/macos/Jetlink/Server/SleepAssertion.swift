import Foundation
import IOKit.ps
import IOKit.pwr_mgt
import os

/// Keeps the Mac awake while the server is serving on AC power, the same rule
/// caffeinate -s follows, and with the lid closed when asked. No subprocess.
@MainActor
final class SleepAssertion {
  private var assertionID: IOPMAssertionID = IOPMAssertionID(0)
  private var isActive = false
  private var runLoopSource: CFRunLoopSource?
  private let log = Logger(subsystem: "io.zoompilot.jetlink", category: "sleep")

  /// Called on the main actor when the power source changes.
  var onPowerSourceChange: (@MainActor () -> Void)?

  /// The power source the Mac is running on. A desktop with no battery has no
  /// external adapter details, so ask which source is providing power instead,
  /// and treat an answer we cannot read as AC.
  var isOnACPower: Bool {
    guard let snapshot = IOPSCopyPowerSourcesInfo()?.takeRetainedValue() else { return true }
    guard let source = IOPSGetProvidingPowerSourceType(snapshot)?.takeUnretainedValue() else { return true }
    return (source as String) == kIOPMACPowerKey
  }

  var isHoldingAssertion: Bool { isActive }

  /// Saved because the kernel keeps the lid flag after this process exits:
  /// the first update after a crash clears what it left set.
  private var isLidSleepDisabled = UserDefaults.standard.bool(forKey: SleepAssertion.lidSleepDisabledKey)
  private static let lidSleepDisabledKey = "lidSleepDisabled"

  func setActive(_ active: Bool) {
    guard active != isActive else { return }
    if active {
      var identifier = IOPMAssertionID(0)
      let result = IOPMAssertionCreateWithName(
        kIOPMAssertionTypePreventUserIdleSystemSleep as CFString,
        IOPMAssertionLevel(kIOPMAssertionLevelOn),
        "Jetlink is serving the comma" as CFString,
        &identifier)
      guard result == kIOReturnSuccess else {
        log.error("could not take a sleep assertion, error \(result)")
        return
      }
      assertionID = identifier
      isActive = true
      log.info("holding a sleep assertion")
    } else {
      IOPMAssertionRelease(assertionID)
      assertionID = IOPMAssertionID(0)
      isActive = false
      log.info("released the sleep assertion")
    }
  }

  /// Keeps a lid close from sleeping the Mac: the IOPMrootDomain call behind
  /// Amphetamine's Closed-Display Mode, which needs no privileges.
  func setLidSleepDisabled(_ disabled: Bool) {
    // Sent again on every update while held: powerd clears the same flag
    // when an external display or charger comes and goes.
    guard disabled || isLidSleepDisabled else { return }
    guard Self.setClamshellSleepDisabled(disabled) else {
      log.error("could not change sleep on lid close")
      return
    }
    guard disabled != isLidSleepDisabled else { return }
    isLidSleepDisabled = disabled
    UserDefaults.standard.set(disabled, forKey: Self.lidSleepDisabledKey)
    log.info("\(disabled ? "keeping the Mac awake with the lid closed" : "a closed lid sleeps the Mac again", privacy: .public)")
  }

  private static func setClamshellSleepDisabled(_ disabled: Bool) -> Bool {
    let rootDomain = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("IOPMrootDomain"))
    guard rootDomain != IO_OBJECT_NULL else { return false }
    defer { IOObjectRelease(rootDomain) }
    var connect = io_connect_t(0)
    guard IOServiceOpen(rootDomain, mach_task_self_, 0, &connect) == KERN_SUCCESS else { return false }
    defer { IOServiceClose(connect) }
    var input: UInt64 = disabled ? 1 : 0
    return IOConnectCallScalarMethod(connect, UInt32(kPMSetClamshellSleepState), &input, 1, nil, nil) == KERN_SUCCESS
  }

  func startObservingPowerSource() {
    guard runLoopSource == nil else { return }
    let context = Unmanaged.passUnretained(self).toOpaque()
    let callback: IOPowerSourceCallbackType = { raw in
      guard let raw else { return }
      let assertion = Unmanaged<SleepAssertion>.fromOpaque(raw).takeUnretainedValue()
      Task { @MainActor in assertion.onPowerSourceChange?() }
    }
    guard let source = IOPSNotificationCreateRunLoopSource(callback, context)?.takeRetainedValue() else {
      log.error("could not observe power source changes")
      return
    }
    runLoopSource = source
    CFRunLoopAddSource(CFRunLoopGetMain(), source, CFRunLoopMode.defaultMode)
  }

  func stopObservingPowerSource() {
    guard let source = runLoopSource else { return }
    CFRunLoopRemoveSource(CFRunLoopGetMain(), source, CFRunLoopMode.defaultMode)
    runLoopSource = nil
  }

  /// Releases everything. Call before dropping the last reference.
  func invalidate() {
    setActive(false)
    stopObservingPowerSource()
    onPowerSourceChange = nil
  }
}
