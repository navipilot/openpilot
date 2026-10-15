import Foundation

/// What the USB loop and the sessions tell a host that suspends while the
/// comma is gone: the Python serve loop's `touch` and `idle` calls.
public enum GadgetIdleEvent: Sendable, Equatable {
  /// On the bus, whether or not it opens: the box must not sleep under it.
  case present
  /// No gadget and no comma served over any link: the only time to sleep.
  case absent
  /// A comma connected over any link, and that connection ended.
  case connected, disconnected
}

/// What the host running the server supplies besides the backend and the
/// gadget. The defaults are what the apps want; the Linux daemon fills them in.
public struct ServerHooks: Sendable {
  /// The host's sensors for WANT_STATE, the hello and STATE_RESP, `{}` with
  /// none, never zeros. Read on a thread of the server's own, so it may block.
  public var telemetry: @Sendable () -> [String: Any]
  /// "nominal", "fair", "serious" or "critical", for the benchmark's reports.
  public var thermal: @Sendable () -> String
  /// HELLO_RESP's `sleep_after`, the seconds without a gadget before this host
  /// suspends; 0, it never does, and the comma holds the gadget all park.
  public var sleepAfter: Double
  /// Hears every USB poll's presence and every connection's start and end.
  /// After `.absent` it may suspend, returning true once awake, and the loop
  /// then looks for the gadget at once: whatever woke the box is likely the comma.
  public var gadgetIdle: (@Sendable (GadgetIdleEvent) -> Bool)?
  /// A SHUTDOWN_REQ and its reason. nil, or a hook returning nil, refuses:
  /// ok:false, and the apps hear `.shutdownRequested`. A hook accepts by
  /// returning the power-off, which runs once the ok:true reply is written.
  public var shutdown: (@Sendable (_ reason: String) -> (@Sendable () -> Void)?)?
  /// After a frame answered INFER_FAILED for an error the backend marks fatal
  /// (`FatalEngineError`): the daemon exits 3 and systemd restarts it.
  public var fatal: (@Sendable (any Error) -> Void)?

  public init(
    telemetry: @escaping @Sendable () -> [String: Any] = { [:] },
    thermal: @escaping @Sendable () -> String = { platformThermal() },
    sleepAfter: Double = 0,
    gadgetIdle: (@Sendable (GadgetIdleEvent) -> Bool)? = nil,
    shutdown: (@Sendable (_ reason: String) -> (@Sendable () -> Void)?)? = nil,
    fatal: (@Sendable (any Error) -> Void)? = nil
  ) {
    self.telemetry = telemetry
    self.thermal = thermal
    self.sleepAfter = sleepAfter
    self.gadgetIdle = gadgetIdle
    self.shutdown = shutdown
    self.fatal = fatal
  }
}
