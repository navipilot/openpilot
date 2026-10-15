#if os(Linux)
  import Foundation
  import Glibc
  import JetlinkServer

  /// Powering the box off on the comma's say-so (D9). hardwared shuts the
  /// comma down below 11.8 V or after 30 hours parked, and a Jetson on an
  /// always-on feed draws even asleep, so it goes too. Off is off: only a DC
  /// cycle or the button brings it back, so it takes --poweroff, which the
  /// installer passes only on a Jetson whose owner said yes.
  public enum PowerOff {
    /// `ServerHooks.shutdown` on Linux. Every request is answered ok, as the
    /// Python server did, so the comma sees what it always saw; once the
    /// reply is written the box syncs, then powers off when `enabled`.
    public static func hook(enabled: Bool) -> @Sendable (String) -> (@Sendable () -> Void)? {
      hook(enabled: enabled, powerOff: systemctlPowerOff, log: ServerLog(category: "power"))
    }

    static func hook(enabled: Bool, powerOff: @escaping @Sendable (ServerLog) -> Void, log: ServerLog)
      -> @Sendable (String) -> (@Sendable () -> Void)?
    {
      { _ in
        {
          sync()
          if enabled {
            log.warning("powering off")
            powerOff(log)
          } else {
            log.warning("staying up: this server runs without --poweroff")
          }
        }
      }
    }

    static func systemctlPowerOff(_ log: ServerLog) {
      do {
        let pid = try spawn("systemctl", ["poweroff"])
        // Reaped off the session thread: systemd may stop this process
        // before systemctl returns.
        Thread.detachNewThread {
          let status = exitStatus(of: pid)
          if status != 0 { log.error("systemctl poweroff exited with status \(status)") }
        }
      } catch {
        log.error("cannot run systemctl poweroff: \(error)")
      }
    }

    /// Starts `path`, looked up on PATH when it has no slash, with
    /// `arguments` and this process's environment. The daemon blocks SIGINT
    /// and SIGTERM on every thread and ignores SIGPIPE, and a child inherits
    /// both through exec: the child gets an empty mask and those signals'
    /// defaults back. With `output`, the child's stdout and stderr go there
    /// and its stdin is /dev/null.
    static func spawn(_ path: String, _ arguments: [String], output: Int32? = nil) throws(KernelError) -> pid_t {
      var actions = posix_spawn_file_actions_t()
      posix_spawn_file_actions_init(&actions)
      defer { posix_spawn_file_actions_destroy(&actions) }
      if let output {
        posix_spawn_file_actions_addopen(&actions, 0, "/dev/null", O_RDONLY, 0)
        posix_spawn_file_actions_adddup2(&actions, output, 1)
        posix_spawn_file_actions_adddup2(&actions, output, 2)
      }
      var attributes = posix_spawnattr_t()
      posix_spawnattr_init(&attributes)
      defer { posix_spawnattr_destroy(&attributes) }
      var empty = sigset_t()
      sigemptyset(&empty)
      var defaults = sigset_t()
      sigemptyset(&defaults)
      for signal in [SIGINT, SIGTERM, SIGPIPE, SIGHUP] {
        sigaddset(&defaults, signal)
      }
      posix_spawnattr_setsigmask(&attributes, &empty)
      posix_spawnattr_setsigdefault(&attributes, &defaults)
      posix_spawnattr_setflags(&attributes, Int16(POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF))
      let argv = ([path] + arguments).map { strdup($0) } + [nil]
      defer { argv.forEach { free($0) } }
      var pid: pid_t = 0
      let error =
        output == nil
        ? posix_spawnp(&pid, path, nil, &attributes, argv, environ) : posix_spawnp(&pid, path, &actions, &attributes, argv, environ)
      guard error == 0 else { throw KernelError(what: path, errno: error) }
      return pid
    }

    /// The child's exit status, or -1 when it did not exit normally.
    static func exitStatus(of pid: pid_t) -> Int32 {
      var status: Int32 = 0
      while waitpid(pid, &status, 0) < 0 {
        guard errno == EINTR else { return -1 }
      }
      // WIFEXITED and WEXITSTATUS, which are macros.
      return status & 0x7f == 0 ? (status >> 8) & 0xff : -1
    }
  }
#endif
