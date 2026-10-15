// PageServer is built for Linux and macOS only.
#if canImport(Darwin) || canImport(Glibc)
  import Foundation
  import JetlinkKit
  import JetlinkTestSupport
  import Testing

  @testable import JetlinkServer
  @testable import JetlinkStatusPage

  @Suite("Web page sign-in and controls", .serialized)
  struct PageAuthTests {
    /// A page with a password file, its password "bench password".
    final class Signed {
      let scratch: TemporaryDirectory
      let auth: WebAuth
      let system = FakeSystem()
      let page: RunningPage

      init(file: Bool = true) throws {
        scratch = try TemporaryDirectory()
        auth = WebAuth(url: scratch.url.appending(path: "web-auth.json"))
        if file { try AuthFile.make(password: "bench password", iterations: 100).write(auth.url) }
        page = try RunningPage(auth: auth, system: system, keepalive: 0.2)
      }

      var port: UInt16 { page.port }

      func signIn() throws -> String {
        let reply = try Client.post(port, "/api/login", ["password": "bench password"])
        #expect(reply.status == 200)
        return try #require(reply.cookie)
      }
    }

    @Test("Without a sign-in only the page and the sign-in answer")
    func gated() throws {
      let signed = try Signed()
      #expect(try Client.fetch(signed.port, "/").status == 200)
      let session = try Client.fetch(signed.port, "/api/session")
      #expect(session.status == 200 && session.json["auth"] as? String == "required" && session.json["signed_in"] as? Bool == false)
      for path in ["/events", "/logs", "/api/system", "/api/task"] {
        #expect(try Client.fetch(signed.port, path).status == 401, "\(path)")
      }
      for path in ["/api/command", "/api/settings", "/api/action", "/api/password", "/api/logout-all"] {
        #expect(try Client.post(signed.port, path, ["cmd": "inventory", "action": "restart"]).status == 401, "\(path)")
      }
      #expect(signed.system.asked.all.isEmpty)
      // a cookie that does not verify is cleared
      let stale = try Client.fetch(signed.port, "/api/session", cookie: "jetlink_session=v1.1.2.3")
      #expect(stale.setCookie?.hasPrefix("jetlink_session=; Max-Age=0") == true)
    }

    @Test("The right password signs in for 400 days; the cookie opens everything")
    func signIn() throws {
      let signed = try Signed()
      let wrong = try Client.post(signed.port, "/api/login", ["password": "nope nope"])
      #expect(wrong.status == 401 && wrong.cookie == nil && wrong.json["error"] as? String == "That password is not right.")
      let reply = try Client.post(signed.port, "/api/login", ["password": "bench password"])
      #expect(reply.status == 200)
      #expect(reply.setCookie?.contains("; Max-Age=34560000; Path=/; HttpOnly; SameSite=Strict") == true)
      let cookie = try #require(reply.cookie)
      let session = try Client.fetch(signed.port, "/api/session", cookie: cookie)
      #expect(session.json["signed_in"] as? Bool == true)
      #expect(try Client.fetch(signed.port, "/logs", cookie: cookie).status == 200)
      let system = try Client.fetch(signed.port, "/api/system", cookie: cookie)
      #expect(system.status == 200 && system.json["installed"] as? Bool == true)
      #expect(system.head.contains("Cache-Control: no-store"))
      let stream = try Client(port: signed.port)
      stream.send("GET /events HTTP/1.1\r\nCookie: \(cookie)\r\n\r\n")
      #expect(stream.read { $0.contains("retry: 2000") }.hasPrefix("HTTP/1.1 200 OK\r\n"))
      // signing out clears it here; the server keeps nothing, so the old value still works elsewhere until it expires
      let out = try Client.post(signed.port, "/api/logout", cookie: cookie)
      #expect(out.status == 200 && out.setCookie?.hasPrefix("jetlink_session=; Max-Age=0") == true)
    }

    @Test("Guessing is held up: five tries, then a wait the reply names")
    func throttled() throws {
      let signed = try Signed()
      var last = Reply("")
      for n in 0..<6 {
        // two checks a second at most, from everyone together
        if n > 0 { Thread.sleep(forTimeInterval: 0.55) }
        last = try Client.post(signed.port, "/api/login", ["password": "guess \(n)"])
      }
      #expect(last.status == 429)
      #expect(last.json["retry_after"] as? Int == 1)
      #expect((last.json["error"] as? String)?.hasPrefix("Too many tries.") == true)
    }

    @Test("A POST from another site, or without the page's header, is refused before anything is read")
    func crossSite() throws {
      let signed = try Signed()
      let cookie = try signed.signIn()
      let plain = try Client.post(signed.port, "/api/action", ["action": "restart"], cookie: cookie, headers: ["Content-Type: application/json"])
      #expect(plain.status == 403)
      let form = try Client.post(signed.port, "/api/action", ["action": "restart"], cookie: cookie, headers: ["X-Jetlink: 1", "Content-Type: text/plain"])
      #expect(form.status == 415)
      let elsewhere = try Client.post(
        signed.port, "/api/action", ["action": "restart"], cookie: cookie,
        headers: ["X-Jetlink: 1", "Content-Type: application/json", "Origin: http://evil.example"])
      #expect(elsewhere.status == 403)
      let fetchSite = try Client.post(
        signed.port, "/api/action", ["action": "restart"], cookie: cookie,
        headers: ["X-Jetlink: 1", "Content-Type: application/json", "Sec-Fetch-Site: cross-site"])
      #expect(fetchSite.status == 403)
      let same = try Client.post(
        signed.port, "/api/action", ["action": "restart"], cookie: cookie,
        headers: ["X-Jetlink: 1", "Content-Type: application/json", "Origin: http://jetlink.local", "Sec-Fetch-Site: same-origin"])
      #expect(same.status == 200)
      #expect(signed.system.asked.all == ["restart"])
    }

    @Test("A body is read to its length, and refused past 16 KB or when it is not an object")
    func bodies() throws {
      let signed = try Signed()
      let cookie = try signed.signIn()
      let big = try Client.post(signed.port, "/api/settings", ["pad": String(repeating: "a", count: 17000)], cookie: cookie)
      #expect(big.status == 413)
      let client = try Client(port: signed.port)
      client.send("POST /api/settings HTTP/1.1\r\nX-Jetlink: 1\r\nContent-Type: application/json\r\nCookie: \(cookie)\r\nContent-Length: 2\r\n\r\n[]")
      #expect(Reply(client.read()).status == 400)
      let none = try Client(port: signed.port)
      none.send("POST /api/settings HTTP/1.1\r\nX-Jetlink: 1\r\nContent-Type: application/json\r\nCookie: \(cookie)\r\n\r\n")
      #expect(Reply(none.read()).status == 411)
      // a body that comes in pieces
      let slow = try Client(port: signed.port)
      slow.send("POST /api/settings HTTP/1.1\r\nX-Jetlink: 1\r\nContent-Type: application/json\r\nCookie: \(cookie)\r\nContent-Length: 18\r\n\r\n{\"power\":")
      Thread.sleep(forTimeInterval: 0.2)
      slow.send("\"always\"}")
      #expect(Reply(slow.read()).status == 200)
      #expect(signed.system.asked.all == ["apply power=always"])
    }

    @Test("While the car drives, nothing that restarts, builds, downloads or deletes starts")
    func driving() async throws {
      let signed = try Signed()
      let cookie = try signed.signIn()
      signed.page.feed.publish(.link(LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3")))
      signed.page.feed.publish(stats(frames: 20))
      signed.page.feed.flush()
      for action in PageAction.allCases where action.interruptsComma {
        let reply = try Client.post(signed.port, "/api/action", ["action": action.rawValue], cookie: cookie)
        #expect(reply.status == 409 && (reply.json["error"] as? String)?.contains("driving") == true, "\(action)")
      }
      #expect(try Client.post(signed.port, "/api/settings", ["power": "switched"], cookie: cookie).status == 409)
      for command: [String: Any] in [
        ["cmd": "prepare", "sha256": String(repeating: "a", count: 64)], ["cmd": "download", "ref": "x"],
        ["cmd": "forget", "sha256": String(repeating: "a", count: 64)], ["cmd": "benchmark"],
      ] {
        #expect(try Client.post(signed.port, "/api/command", command, cookie: cookie).status == 409, "\(command)")
      }
      // what costs the frames nothing still goes
      #expect(try Client.post(signed.port, "/api/action", ["action": "keep_awake", "seconds": 900], cookie: cookie).status == 200)
      #expect(signed.system.asked.all == ["keep_awake 900"])
      // the comma connected but parked is not driving
      signed.page.feed.publish(.link(LinkEvent(state: .waiting, detail: "", peer: nil, medium: nil)))
      signed.page.feed.flush()
      #expect(try Client.post(signed.port, "/api/action", ["action": "restart"], cookie: cookie).status == 200)
    }

    @Test("Commands go to the controller; imports, unloads and shutdowns are the apps' alone")
    func commands() async throws {
      let signed = try Signed()
      let cookie = try signed.signIn()
      #expect(try Client.post(signed.port, "/api/command", ["cmd": "inventory"], cookie: cookie).status == 503)
      let scratch = try TemporaryDirectory()
      let server = try Server(
        configuration: Server.Configuration(host: "127.0.0.1", port: 0, cacheRoot: scratch.url, preload: false, listen: true), backend: NamingBackend())
      let registry = CountingRegistry()
      let controller = ServerController(server: server, registry: registry)
      signed.page.server.watch(controller, version: "test")
      let ok = try Client.post(signed.port, "/api/command", ["cmd": "inventory"], cookie: cookie)
      #expect(ok.status == 200 && ok.json["ok"] as? Bool == true && ok.json["id"] == nil)
      for name in ["import", "unload", "shutdown"] {
        let reply = try Client.post(signed.port, "/api/command", ["cmd": name, "path": "/etc/shadow"], cookie: cookie)
        #expect(reply.status == 403, "\(name)")
      }
      #expect(try Client.post(signed.port, "/api/command", ["cmd": "nonsense"], cookie: cookie).status == 400)
      // the controller's own refusals come back as such
      let refused = try Client.post(signed.port, "/api/command", ["cmd": "benchmark"], cookie: cookie)
      #expect(refused.status == 409 && refused.json["ok"] as? Bool == false && (refused.json["error"] as? String)?.contains("no model is loaded") == true)
      #expect(registry.count("import") == 0)
      signed.page.server.stop()
      server.stop()
    }

    @Test("A new password signs everyone else out; signing the others out keeps this device")
    func password() throws {
      let signed = try Signed()
      // apart, as people are: no more than two password checks go through a second
      let apart = { Thread.sleep(forTimeInterval: 0.55) }
      let phone = try signed.signIn()
      apart()
      let laptop = try signed.signIn()
      apart()
      let wrong = try Client.post(signed.port, "/api/password", ["current": "not it at all", "new": "a new password"], cookie: phone)
      #expect(wrong.status == 401 && wrong.json["error"] as? String == "The current password is not right.")
      let short = try Client.post(signed.port, "/api/password", ["current": "bench password", "new": "short"], cookie: phone)
      #expect(short.status == 400 && short.json["error"] as? String == "The password needs at least 8 characters.")
      apart()
      let changed = try Client.post(signed.port, "/api/password", ["current": "bench password", "new": "a new password"], cookie: phone)
      #expect(changed.status == 200)
      let phoneAgain = try #require(changed.cookie)
      #expect(try Client.fetch(signed.port, "/logs", cookie: laptop).status == 401)
      #expect(try Client.fetch(signed.port, "/logs", cookie: phone).status == 401)
      #expect(try Client.fetch(signed.port, "/logs", cookie: phoneAgain).status == 200)
      apart()
      #expect(try Client.post(signed.port, "/api/login", ["password": "a new password"]).status == 200)

      let others = try Client.post(signed.port, "/api/logout-all", cookie: phoneAgain)
      let kept = try #require(others.cookie)
      #expect(try Client.fetch(signed.port, "/logs", cookie: phoneAgain).status == 401)
      #expect(try Client.fetch(signed.port, "/logs", cookie: kept).status == 200)
    }

    @Test("Without a password file the page is the read-only one: it watches, and every control is refused")
    func readOnly() throws {
      let signed = try Signed(file: false)
      let session = try Client.fetch(signed.port, "/api/session")
      #expect(session.json["auth"] as? String == "none")
      #expect(try Client.fetch(signed.port, "/logs").status == 200)
      let stream = try Client(port: signed.port)
      stream.send("GET /events HTTP/1.1\r\n\r\n")
      #expect(stream.read { $0.contains("retry: 2000") }.hasPrefix("HTTP/1.1 200 OK\r\n"))
      #expect(try Client.post(signed.port, "/api/login", ["password": "anything at all"]).status == 403)
      let action = try Client.post(signed.port, "/api/action", ["action": "reboot"])
      #expect(action.status == 403 && (action.json["error"] as? String)?.contains("sudo jetlink password") == true)
      #expect(try Client.fetch(signed.port, "/api/system").status == 403)
      #expect(signed.system.asked.all.isEmpty)

      // `jetlink password` under a running server: sign-in from then on
      try AuthFile.make(password: "bench password", iterations: 100).write(signed.auth.url)
      #expect(try Client.fetch(signed.port, "/logs").status == 401)
      _ = try signed.signIn()
    }

    @Test("What the computer refuses comes back as a refusal, what fails as a failure")
    func systemErrors() throws {
      let signed = try Signed()
      let cookie = try signed.signIn()
      signed.system.refuse.value = .refused("Not here.")
      let refused = try Client.post(signed.port, "/api/action", ["action": "update"], cookie: cookie)
      #expect(refused.status == 409 && refused.json["error"] as? String == "Not here.")
      signed.system.refuse.value = .failed("It broke.")
      #expect(try Client.post(signed.port, "/api/settings", ["power": "always"], cookie: cookie).status == 500)
      #expect(try Client.post(signed.port, "/api/action", [:], cookie: cookie).status == 400)
    }
  }
#endif
