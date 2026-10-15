import Foundation

/// The computer behind the page: the installer's answers and the runs that
/// change them, updates, restarts and keeping the box awake. JetlinkLinux's
/// `PageHost` on an installed Jetson or PC; none elsewhere.
public protocol PageSystem: AnyObject, Sendable {
  /// What the System and Settings screens show (`GET /api/system`).
  func info() -> [String: Any]
  /// Runs the installer with these answers, in its own words
  /// (`power: always`); the server restarts.
  func apply(_ settings: [String: String]) throws(PageSystemError)
  func perform(_ action: PageAction, seconds: Int?) throws(PageSystemError) -> [String: Any]
  /// The settings or update run, if any (`GET /api/task`).
  func task() -> [String: Any]
}

/// What the page's System screen can ask of the computer.
public enum PageAction: String, Sendable, CaseIterable {
  case restart, reboot, poweroff, update
  case checkUpdate = "check_update"
  case keepAwake = "keep_awake"

  /// Takes the server or the computer down: never while the car drives.
  public var interruptsComma: Bool {
    switch self {
    case .restart, .reboot, .poweroff, .update: true
    case .checkUpdate, .keepAwake: false
    }
  }
}

public enum PageSystemError: Error, Equatable, CustomStringConvertible {
  /// The request was understood and cannot be done now, or here.
  case refused(String)
  /// It was tried and failed.
  case failed(String)

  public var description: String {
    switch self {
    case .refused(let text), .failed(let text): text
    }
  }

  var status: Int {
    if case .refused = self { 409 } else { 500 }
  }
}
