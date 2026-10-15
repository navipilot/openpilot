// The jetlink server as a command: the daemon on a Jetson or a Linux PC, and
// a Mac's server in a terminal. Only Linux and macOS build it; elsewhere this
// is an empty program, so the package still builds everything everywhere.
#if os(macOS) || os(Linux)
  import ArgumentParser
  import Foundation
  import JetlinkKit

  struct JetlinkServerCommand: ParsableCommand {
    static let configuration = CommandConfiguration(
      commandName: "jetlink-server",
      abstract: "Runs the comma's big model on this machine and serves it over USB or TCP.",
      version: productVersion(),
      subcommands: [Serve.self, Build.self, Spec.self, ListBackends.self, Bench.self, Models.self, WebPassword.self],
      defaultSubcommand: Serve.self)

    /// `main()`, except that a usage mistake exits 1 as any other error
    /// does: 2 and 3 mean a network failure and bytes that did not verify to
    /// `models`, and 3 a fatal CUDA error to `serve`.
    static func runAndExit() -> Never {
      do {
        var command = try parseAsRoot()
        try command.run()
        Foundation.exit(0)
      } catch {
        if exitCode(for: error) == .validationFailure {
          FileHandle.standardError.write(Data((fullMessage(for: error) + "\n").utf8))
          Foundation.exit(1)
        }
        exit(withError: error)
      }
    }
  }

  /// What `--version` prints: the VERSION a release tarball keeps beside
  /// bin/, which names a dev build's commit, else the version this source
  /// was pinned at.
  func productVersion(executable: URL? = executableURL()) -> String {
    let file = executable?.deletingLastPathComponent().deletingLastPathComponent().appending(path: "VERSION")
    if let file, let text = try? String(contentsOf: file, encoding: .utf8),
      let line = text.split(whereSeparator: \.isNewline).first?.trimmingCharacters(in: .whitespaces), !line.isEmpty
    {
      return line
    }
    return Pinned.productVersion
  }

  /// This binary, links resolved: the installed path runs through
  /// /opt/jetlink/current, a link to the version's directory.
  func executableURL() -> URL? {
    #if os(Linux)
      (try? FileManager.default.destinationOfSymbolicLink(atPath: "/proc/self/exe")).map { URL(fileURLWithPath: $0) }
    #else
      Bundle.main.executableURL?.resolvingSymlinksInPath()
    #endif
  }

  JetlinkServerCommand.runAndExit()
#endif
