#if !canImport(os)
  import Foundation

  /// `os.Logger`'s shape where there is no unified log: Linux and Android. The
  /// same call sites compile, privacy arguments included, and write to
  /// standard error. On Apple platforms this file is empty and the modules
  /// import `os` itself.
  public struct Logger: Sendable {
    public enum Level: Int, Comparable, Sendable {
      case debug, info, notice, warning, error

      public static func < (a: Level, b: Level) -> Bool { a.rawValue < b.rawValue }
    }

    /// The least severe level written, for every logger in the process:
    /// `Log.threshold` sets it. Set once at startup, before any thread logs.
    nonisolated(unsafe) public static var threshold = Level.info

    private let label: String

    public init(subsystem: String, category: String) {
      label = "\(subsystem).\(category)"
    }

    public func debug(_ message: LogMessage) { write(.debug, "DEBUG", message) }
    public func info(_ message: LogMessage) { write(.info, "INFO", message) }
    public func notice(_ message: LogMessage) { write(.notice, "NOTICE", message) }
    public func warning(_ message: LogMessage) { write(.warning, "WARNING", message) }
    public func error(_ message: LogMessage) { write(.error, "ERROR", message) }

    private func write(_ level: Level, _ name: String, _ message: LogMessage) {
      guard level >= Logger.threshold else { return }
      let line = "\(name) \(label): \(message.text)"
      FileHandle.standardError.write(Data((line + "\n").utf8))
    }
  }

  /// What `os.Logger` takes: a string whose interpolations may name a privacy.
  public struct LogMessage: ExpressibleByStringInterpolation, Sendable {
    public let text: String

    public init(stringLiteral value: String) {
      text = value
    }

    public init(stringInterpolation: Interpolation) {
      text = stringInterpolation.text
    }

    public struct Interpolation: StringInterpolationProtocol {
      var text = ""

      public init(literalCapacity: Int, interpolationCount: Int) {
        text.reserveCapacity(literalCapacity)
      }

      public mutating func appendLiteral(_ literal: String) {
        text += literal
      }

      public mutating func appendInterpolation<T>(_ value: T, privacy: LogPrivacy = .auto) {
        text += String(describing: value)
      }
    }
  }

  /// `OSLogPrivacy`'s cases, which mean nothing on standard error.
  public enum LogPrivacy: Sendable {
    case auto
    case `public`
    case `private`
  }
#endif
