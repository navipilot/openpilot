#if os(Linux)
  import Foundation
  import Glibc
  import JetlinkServer
  import Testing

  @testable import JetlinkLinux

  @Suite("Power off")
  struct PowerOffTests {
    func run(enabled: Bool) -> (calls: Int, lines: Lines) {
      let calls = Locked(0)
      let lines = Lines()
      let hook = PowerOff.hook(enabled: enabled, powerOff: { _ in calls.value += 1 }, log: lines.log)
      // Always accepted: the reply says ok and "powering off" either way.
      let action = hook("car battery")
      #expect(action != nil)
      #expect(calls.value == 0)
      action?()
      return (calls.value, lines)
    }

    @Test("With --poweroff the box powers off once the reply is out")
    func enabled() {
      let (calls, lines) = run(enabled: true)
      #expect(calls == 1)
      #expect(lines.has(.warning, "powering off"))
    }

    @Test("Without it the box stays up")
    func disabled() {
      let (calls, lines) = run(enabled: false)
      #expect(calls == 0)
      #expect(lines.has(.warning, "staying up: this server runs without --poweroff"))
    }

    @Test("The child starts with no signal blocked and the stop signals at their defaults")
    func childSignals() throws {
      let tree = Tree()
      let out = tree.path("/status")
      // As the daemon's threads are: SIGINT and SIGTERM blocked, SIGPIPE ignored.
      var held = sigset_t()
      sigemptyset(&held)
      sigaddset(&held, SIGINT)
      sigaddset(&held, SIGTERM)
      var previous = sigset_t()
      pthread_sigmask(SIG_BLOCK, &held, &previous)
      defer { pthread_sigmask(SIG_SETMASK, &previous, nil) }
      // Left ignored: other suites write to sockets in this process.
      signal(SIGPIPE, SIG_IGN)

      let pid = try PowerOff.spawn("/bin/sh", ["-c", "grep -E '^Sig(Blk|Ign)' /proc/self/status > \(out)"])
      #expect(PowerOff.exitStatus(of: pid) == 0)
      let status = try String(contentsOfFile: out, encoding: .utf8)
      let masks = Dictionary(
        uniqueKeysWithValues: status.split(separator: "\n").map { line in
          let parts = line.split(separator: ":")
          return (String(parts[0]), UInt64(parts[1].trimmingCharacters(in: .whitespaces), radix: 16)!)
        })
      #expect(masks["SigBlk"] == 0)
      for number in [SIGINT, SIGTERM, SIGPIPE] {
        #expect(masks["SigIgn"]! & (1 << UInt64(number - 1)) == 0)
      }
    }

    @Test("A failing child's status comes back, a name is found on PATH")
    func exitStatus() throws {
      #expect(PowerOff.exitStatus(of: try PowerOff.spawn("/bin/sh", ["-c", "exit 3"])) == 3)
      #expect(PowerOff.exitStatus(of: try PowerOff.spawn("sh", ["-c", "exit 4"])) == 4)
      #expect(throws: KernelError.self) { try PowerOff.spawn("/nonexistent/systemctl", ["poweroff"]) }
    }
  }
#endif
