import Foundation
import Observation

/// Where the server is in its life. On a Mac it is a process the app starts;
/// on an iPhone it runs inside the app, and only starts, serves and fails.
public enum ServerRunState: Equatable, Sendable {
  case stopped
  case starting
  case serving
  case stopping
  case failed(String)
}

/// One `stats` event and when it arrived, for the frame time chart.
public struct StatsSample: Identifiable, Equatable, Sendable {
  public var id: Date { at }
  public let at: Date
  public let stats: StatsEvent

  public init(at: Date, stats: StatsEvent) {
    self.at = at
    self.stats = stats
  }

  /// Two minutes at one summary a second.
  public static let historyLength = 120

  /// `history` with `stats` appended, trimmed to the last two minutes.
  public static func appending(_ stats: StatsEvent, at date: Date = Date(), to history: [StatsSample]) -> [StatsSample] {
    var history = history
    history.append(StatsSample(at: date, stats: stats))
    if history.count > historyLength {
      history.removeFirst(history.count - historyLength)
    }
    return history
  }
}

/// The server as the stores see it, whichever process runs it: the Mac app's
/// Python server behind a socket, or the iPhone app's own.
@MainActor
public protocol ServerControlling: AnyObject, Observable {
  var link: LinkEvent { get }
  var engine: EngineEvent { get }
  /// Every event a `ModelStore` cares about: inventory, catalog, download, import.
  var modelEvents: AsyncStream<ControlEvent> { get }
  func send(_ command: ControlCommand) async throws -> ReplyEvent
  /// Starts the server if it is not serving, and waits until it is.
  func startIfNeeded() async throws
}

/// A server with fixed state and nothing behind it, for previews and tests.
@MainActor
@Observable
public final class PreviewServer: ServerControlling {
  public var link: LinkEvent
  public var engine: EngineEvent
  public let modelEvents: AsyncStream<ControlEvent>

  public init(link: LinkEvent = .waiting, engine: EngineEvent = .none) {
    self.link = link
    self.engine = engine
    self.modelEvents = AsyncStream { $0.finish() }
  }

  public func send(_ command: ControlCommand) async throws -> ReplyEvent {
    ReplyEvent(id: nil, ok: true, error: nil)
  }

  public func startIfNeeded() async throws {}
}
