#if os(macOS) || os(Linux)
  import Foundation
  import JetlinkRegistry
  import JetlinkServer
  import JetlinkTestSupport
  import Testing

  @testable import jetlink_server

  #if canImport(FoundationNetworking)
    import FoundationNetworking
  #endif

  /// What a models command wrote, for a test to read.
  final class Captured: Sendable {
    private let text = Locked<(out: String, err: String)>(("", ""))

    var console: Console {
      Console(
        out: { line in self.text.withLock { $0.out += line + "\n" } },
        err: { chunk in self.text.withLock { $0.err += chunk } })
    }

    var out: String { text.withLock { $0.out } }
    var err: String { text.withLock { $0.err } }
  }

  /// A models command as the command line gives it, run against `net`.
  func models<Command: ModelsCommand>(
    _ type: Command.Type, _ arguments: [String], cache: TemporaryDirectory, net: MockNet = MockNet()
  ) async throws -> (code: Int32, out: String, err: String) {
    let command = try Command.parse(arguments + ["--cache", cache.path])
    let captured = Captured()
    let code = await command.execute(Registry(layout: CacheLayout(root: cache.url), session: net.session), captured.console)
    return (code, captured.out, captured.err)
  }
#endif
