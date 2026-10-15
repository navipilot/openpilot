#if os(Linux)
  import Foundation
  import Glibc

  /// Where this host's kernel files are: `/` on a real host, a captured or
  /// made-up tree in the tests. Every path the Linux host reads or writes
  /// goes through one of these.
  public struct HostRoot: Sendable, Equatable {
    /// Put in front of every absolute path: "" for the real host.
    public let prefix: String

    public static let system = HostRoot("")

    public init(_ directory: String) {
      var prefix = directory
      while prefix.hasSuffix("/") { prefix.removeLast() }
      self.prefix = prefix
    }

    public func path(_ absolute: String) -> String {
      prefix + absolute
    }

    /// The names in a directory, sorted as Python's `sorted(glob)` sorts
    /// them; empty when it cannot be listed.
    public func list(_ absolute: String) -> [String] {
      ((try? FileManager.default.contentsOfDirectory(atPath: path(absolute))) ?? []).sorted()
    }

    public func exists(_ absolute: String) -> Bool {
      FileManager.default.fileExists(atPath: path(absolute))
    }
  }

  /// A kernel file or call that failed, and its errno: which says, for one,
  /// whether a suspend is worth trying again (sleep.py's split).
  public struct KernelError: Error, CustomStringConvertible {
    public let what: String
    public let errno: Int32

    public var description: String { "\(what): \(String(cString: strerror(errno)))" }
  }

  /// Small kernel files, read and written with one call each and no
  /// Foundation in between: the sampler reads a handful every second, and a
  /// write's errno decides what the sleeper does next.
  public enum Sysfs {
    /// The file's text without the whitespace around it, or nil on any
    /// error: a Jetson's cv* thermal zones answer ENODATA.
    public static func read(_ path: String) -> String? {
      bytes(path) { String(decoding: $0, as: UTF8.self) }
    }

    public static func readInt(_ path: String) -> Int? {
      bytes(path) { text -> Int? in
        var digits = text[...]
        let negative = digits.first == UInt8(ascii: "-")
        if negative { digits = digits.dropFirst() }
        guard !digits.isEmpty else { return nil }
        var value = 0
        for byte in digits {
          guard byte >= UInt8(ascii: "0") && byte <= UInt8(ascii: "9") else { return nil }
          let (times, overflow) = value.multipliedReportingOverflow(by: 10)
          let (sum, carry) = times.addingReportingOverflow(Int(byte - UInt8(ascii: "0")))
          guard !overflow && !carry else { return nil }
          value = sum
        }
        return negative ? -value : value
      } ?? nil
    }

    /// The file's bytes, whitespace trimmed, handed to `body` from the stack.
    /// sysfs and procfs make a small file whole at the first read, so a read
    /// that comes back short is the end, with no second one to see EOF; only
    /// a file that fills the buffer is read on to its end. (A seq file of
    /// many records, /proc/cpuinfo, can stop short at a record: the page reads
    /// only its first.)
    private static func bytes<T>(_ path: String, _ body: (UnsafeBufferPointer<UInt8>) -> T) -> T? {
      let fd = open(path, O_RDONLY | O_CLOEXEC)
      guard fd >= 0 else { return nil }
      defer { close(fd) }
      return withUnsafeTemporaryAllocation(of: UInt8.self, capacity: 256) { buffer -> T? in
        guard let first = fill(fd, buffer) else { return nil }
        if first < buffer.count {
          return body(trimmed(UnsafeBufferPointer(rebasing: buffer[..<first])))
        }
        var whole = Array(buffer)
        var chunk = [UInt8](repeating: 0, count: 4096)
        while true {
          guard let n = chunk.withUnsafeMutableBufferPointer({ fill(fd, $0) }) else { return nil }
          whole += chunk[..<n]
          if n < chunk.count { break }
        }
        return whole.withUnsafeBufferPointer { body(trimmed($0)) }
      }
    }

    /// One read into `buffer`, or nil on an error.
    private static func fill(_ fd: Int32, _ buffer: UnsafeMutableBufferPointer<UInt8>) -> Int? {
      while true {
        let n = Glibc.read(fd, buffer.baseAddress, buffer.count)
        if n >= 0 { return n }
        if errno != EINTR { return nil }
      }
    }

    private static func trimmed(_ bytes: UnsafeBufferPointer<UInt8>) -> UnsafeBufferPointer<UInt8> {
      let space: (UInt8) -> Bool = { $0 == 0x20 || (0x09...0x0D).contains($0) }
      guard let start = bytes.firstIndex(where: { !space($0) }), let end = bytes.lastIndex(where: { !space($0) }) else {
        return UnsafeBufferPointer(rebasing: bytes[0..<0])
      }
      return UnsafeBufferPointer(rebasing: bytes[start...end])
    }

    /// Writes `text` to an existing file in one write, as the kernel wants a
    /// sysfs attribute written; throws with the errno.
    public static func write(_ path: String, _ text: String) throws(KernelError) {
      let fd = open(path, O_WRONLY | O_TRUNC | O_CLOEXEC)
      guard fd >= 0 else { throw KernelError(what: path, errno: errno) }
      defer { close(fd) }
      let bytes = Array(text.utf8)
      let n = bytes.withUnsafeBytes { Glibc.write(fd, $0.baseAddress, $0.count) }
      if n < 0 { throw KernelError(what: path, errno: errno) }
    }
  }

  /// The few things the server has to know about the machine it runs on
  /// (Python's `jetlink/server/platform.py`). Each fails open: a wrong guess
  /// costs a cache directory in an odd place, never a refusal to serve.
  public enum Platform {
    /// Where a Jetson keeps the cache: its data partition.
    public static let jetsonCache = "/mnt/data/jetlink"

    /// Any one of these says Tegra: the device tree's compatible string, the
    /// L4T release file, or the Orin's GPU node.
    public static func isTegra(_ root: HostRoot = .system) -> Bool {
      for file in ["/sys/firmware/devicetree/base/compatible", "/proc/device-tree/compatible"] {
        if let data = FileManager.default.contents(atPath: root.path(file)), String(decoding: data, as: UTF8.self).lowercased().contains("tegra") {
          return true
        }
      }
      return ["/etc/nv_tegra_release", "/sys/devices/platform/bus@0/17000000.gpu"].contains { root.exists($0) }
    }

    /// /proc/meminfo's fields in bytes ("MemAvailable", "SwapFree", ...);
    /// empty when it cannot be read.
    public static func meminfo(_ root: HostRoot = .system) -> [String: Int] {
      var bytes: [String: Int] = [:]
      for line in (Sysfs.read(root.path("/proc/meminfo")) ?? "").split(separator: "\n") {
        let fields = line.split(separator: " ")
        if fields.count >= 2, let kb = Int(fields[1]) { bytes[String(fields[0].dropLast())] = kb * 1024 }
      }
      return bytes
    }

    /// MemAvailable, or 0 for "no idea". Swap does not count: on a Tegra the
    /// GPU's memory is pinned system RAM and cannot page out.
    public static func memAvailableBytes(_ root: HostRoot = .system) -> Int {
      meminfo(root)["MemAvailable"] ?? 0
    }

    /// The Jetson's data partition when it exists or this is a Tegra, else
    /// /var/lib/jetlink for root, where the installed unit keeps it, else
    /// ${XDG_CACHE_HOME:-~/.cache}/jetlink. $JETLINK_CACHE comes first
    /// (jetlink-server's --cache).
    public static func defaultCache(
      environment: [String: String] = ProcessInfo.processInfo.environment, root: HostRoot = .system, asRoot: Bool = geteuid() == 0
    ) -> URL {
      var isDirectory: ObjCBool = false
      if FileManager.default.fileExists(atPath: root.path(jetsonCache), isDirectory: &isDirectory) && isDirectory.boolValue || isTegra(root) {
        return URL(fileURLWithPath: jetsonCache, isDirectory: true)
      }
      if asRoot {
        return URL(fileURLWithPath: "/var/lib/jetlink", isDirectory: true)
      }
      let base =
        environment["XDG_CACHE_HOME"].flatMap { $0.isEmpty ? nil : URL(fileURLWithPath: $0, isDirectory: true) }
        ?? FileManager.default.homeDirectoryForCurrentUser.appending(path: ".cache", directoryHint: .isDirectory)
      return base.appending(path: "jetlink", directoryHint: .isDirectory)
    }

    /// Whether --sleep-after has a kernel to ask.
    public static func canSuspend(_ root: HostRoot = .system) -> Bool {
      root.exists("/sys/power/state")
    }
  }
#endif
