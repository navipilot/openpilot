#if os(macOS) || os(Linux)
  import ArgumentParser
  import Foundation
  import JetlinkStatusPage

  /// `jetlink-server web-password`: writes the web page's password file, for
  /// install.sh and `jetlink password`. A new file has a new key, so every
  /// device signed in under the old one is signed out.
  struct WebPassword: ParsableCommand {
    static let configuration = CommandConfiguration(
      commandName: "web-password",
      abstract: "Set the web page's password: from standard input, or a generated one printed on standard output.")

    @Option(help: "The password file to write, 0600.")
    var file: String
    @Flag(help: "Make up a password and print it, rather than reading one.")
    var generate = false

    func run() throws {
      let password: String
      if generate {
        password = GeneratedPassword.make()
      } else {
        // The first line; the password never goes on a command line, which
        // any user can read in ps.
        let input = FileHandle.standardInput.readDataToEndOfFile()
        password =
          String(decoding: input, as: UTF8.self).split(separator: "\n", maxSplits: 1, omittingEmptySubsequences: false).first.map {
            String($0.hasSuffix("\r") ? $0.dropLast() : $0)
          } ?? ""
        if let problem = GeneratedPassword.problem(with: password) {
          FileHandle.standardError.write(Data((problem + "\n").utf8))
          throw ExitCode.failure
        }
      }
      do {
        try AuthFile.make(password: password).write(URL(fileURLWithPath: file))
      } catch {
        FileHandle.standardError.write(Data("Could not write \(file): \(error.localizedDescription)\n".utf8))
        throw ExitCode.failure
      }
      if generate { print(password) }
    }
  }
#endif
