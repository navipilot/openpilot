import Foundation
import JetlinkLog

#if canImport(os)
  import os
#endif

/// The server's log, the same on every platform: a line at or above
/// `threshold` goes to the unified log (standard error without one), to
/// `LogRing.shared`, and to the sink an app installs to show the lines itself.
public enum Log {
  public enum Level: String, Comparable, Sendable {
    case debug, info, warning, error

    public static func < (a: Level, b: Level) -> Bool { a.rank < b.rank }

    private var rank: Int {
      switch self {
      case .debug: 0
      case .info: 1
      case .warning: 2
      case .error: 3
      }
    }

    /// As Python's logging names it.
    public var name: String { rawValue.uppercased() }
  }

  /// The least severe level written: a daemon's --log-level.
  public static var threshold: Level {
    get { lock.withLock { least } }
    set {
      lock.withLock { least = newValue }
      #if !canImport(os)
        // The modules that log without the server's categories.
        Logger.threshold =
          switch newValue {
          case .debug: .debug
          case .info: .info
          case .warning: .warning
          case .error: .error
          }
      #endif
    }
  }

  /// Called for every line written, on the thread that logged it, with the
  /// level, the category (server, session, usb, ...) and the message.
  public static var sink: (@Sendable (Level, String, String) -> Void)? {
    get { lock.withLock { stored } }
    set { lock.withLock { stored = newValue } }
  }

  private static let lock = NSLock()
  nonisolated(unsafe) private static var stored: (@Sendable (Level, String, String) -> Void)?
  nonisolated(unsafe) private static var least = Level.info

  /// A line into the ring and the sink, for an app writing beside the server.
  public static func write(_ level: Level, _ category: String, _ message: String) {
    let (threshold, sink) = lock.withLock { (Log.least, Log.stored) }
    guard level >= threshold else { return }
    LogRing.shared.append("\(level.name) jetlink.\(category): \(message)")
    sink?(level, category, message)
  }
}

/// A category's logger: the unified log, and `Log.write`. Messages are
/// public: nothing the server logs is a person's.
package struct ServerLog: Sendable {
  package let category: String
  private let logger: Logger

  package init(category: String) {
    self.category = category
    logger = Logger(subsystem: "io.zoompilot.jetlink", category: category)
  }

  package func debug(_ message: String) { write(.debug, message) }
  package func info(_ message: String) { write(.info, message) }
  package func warning(_ message: String) { write(.warning, message) }
  package func error(_ message: String) { write(.error, message) }

  package func write(_ level: Log.Level, _ message: String) {
    guard level >= Log.threshold else { return }
    switch level {
    case .debug: logger.debug("\(message, privacy: .public)")
    case .info: logger.info("\(message, privacy: .public)")
    case .warning: logger.warning("\(message, privacy: .public)")
    case .error: logger.error("\(message, privacy: .public)")
    }
    Log.write(level, category, message)
  }
}
