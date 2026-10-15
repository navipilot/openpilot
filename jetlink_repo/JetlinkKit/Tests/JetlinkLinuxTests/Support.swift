#if os(Linux)
  import Foundation
  import Glibc
  import JetlinkLog
  import JetlinkServer
  import JetlinkTestSupport

  @testable import JetlinkLinux

  /// The Orin Nano Super's sysfs as the bench Jetson had it (JetPack 7.2,
  /// kernel 6.8), trimmed to what the Linux host reads. The cv0-2 thermal
  /// zones have no temp file: on that kernel reading it fails with ENODATA.
  /// The comma is 2-1.3, behind the carrier's Realtek hub 2-1.
  let jetson = HostRoot(SourceTree.root().appending(path: "JetlinkKit/Tests/JetlinkLinuxTests/Fixtures/jetson").path)

  /// A kernel tree of the test's own in a temporary directory, removed with it.
  final class Tree: @unchecked Sendable {
    let url: URL
    var root: HostRoot { HostRoot(url.path) }

    init() {
      url = FileManager.default.temporaryDirectory.appending(path: "jetlink-linux-\(UUID().uuidString)", directoryHint: .isDirectory)
      try! FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
    }

    /// A copy of the Jetson capture, to write to.
    static func jetsonCopy() -> Tree {
      let tree = Tree()
      for name in ["sys", "proc", "etc"] {
        try! FileManager.default.copyItem(atPath: jetson.path("/\(name)"), toPath: tree.url.appending(path: name).path)
      }
      return tree
    }

    deinit {
      try? FileManager.default.removeItem(at: url)
    }

    func path(_ absolute: String) -> String {
      root.path(absolute)
    }

    func write(_ absolute: String, _ text: String) {
      let file = URL(fileURLWithPath: path(absolute))
      try! FileManager.default.createDirectory(at: file.deletingLastPathComponent(), withIntermediateDirectories: true)
      try! Data(text.utf8).write(to: file)
    }

    func write(_ absolute: String, bytes: [UInt8]) {
      let file = URL(fileURLWithPath: path(absolute))
      try! FileManager.default.createDirectory(at: file.deletingLastPathComponent(), withIntermediateDirectories: true)
      try! Data(bytes).write(to: file)
    }

    func read(_ absolute: String) -> String? {
      try? String(contentsOfFile: path(absolute), encoding: .utf8)
    }

    func remove(_ absolute: String) {
      try? FileManager.default.removeItem(atPath: path(absolute))
    }

    func makeDirectory(_ absolute: String) {
      try! FileManager.default.createDirectory(atPath: path(absolute), withIntermediateDirectories: true)
    }

    /// A symlink at `absolute` to `destination`, as sysfs links a device to its port.
    func link(_ absolute: String, to destination: String) {
      try! FileManager.default.createSymbolicLink(atPath: path(absolute), withDestinationPath: destination)
    }
  }

  /// A logger of the test's own, and what it wrote, read back from the ring
  /// every server line goes to.
  final class Lines: Sendable {
    let log = ServerLog(category: "test-\(UUID().uuidString.prefix(8))")

    /// (LEVEL, message), oldest first.
    var all: [(level: String, message: String)] {
      let tag = " jetlink.\(log.category): "
      return LogRing.shared.lines().compactMap { line in
        guard let at = line.range(of: tag) else { return nil }
        return (line[..<at.lowerBound].split(separator: " ").last.map(String.init) ?? "", String(line[at.upperBound...]))
      }
    }

    func count(_ fragment: String) -> Int {
      all.filter { $0.message.contains(fragment) }.count
    }

    func has(_ level: Log.Level, _ fragment: String) -> Bool {
      all.contains { $0.level == level.name && $0.message.contains(fragment) }
    }
  }
#endif
