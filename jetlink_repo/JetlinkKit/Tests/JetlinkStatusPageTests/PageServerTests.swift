// PageServer is built for Linux and macOS only.
#if canImport(Darwin) || canImport(Glibc)
  import Foundation
  import JetlinkKit
  import JetlinkLog
  import JetlinkTestSupport
  import Testing

  @testable import JetlinkServer
  @testable import JetlinkStatusPage

  @Suite("Status page server", .serialized)
  struct PageServerTests {
    @Test("GET / is the page; anything else but the events and the log is not found")
    func paths() throws {
      let page = try RunningPage()
      let root = try Client.get(page.port, "/")
      #expect(root.hasPrefix("HTTP/1.1 200 OK\r\n"))
      #expect(root.contains("Content-Type: text/html; charset=utf-8\r\n"))
      #expect(root.hasSuffix("\r\n\r\n<!doctype html><p>page</p>"))
      // a browser that has this build's page is told so, and sent nothing
      let etag = try #require(root.components(separatedBy: "\r\n").first { $0.hasPrefix("ETag: ") }).dropFirst(6)
      #expect(try Client.get(page.port, "/", headers: ["If-None-Match: \(etag)"]).hasPrefix("HTTP/1.1 304 Not Modified\r\n"))
      #expect(try Client.get(page.port, "/", headers: ["If-None-Match: \"other\""]).hasPrefix("HTTP/1.1 200 OK\r\n"))
      for path in ["/nope", "/index.html", "/events/x", "/logs/"] {
        #expect(try Client.get(page.port, path).hasPrefix("HTTP/1.1 404 Not Found\r\n"), "\(path)")
      }
      for method in ["POST", "HEAD", "PUT"] {
        #expect(try Client.get(page.port, "/", method: method).hasPrefix("HTTP/1.1 404 Not Found\r\n"), "\(method)")
      }
    }

    @Test("/logs is the last 300 lines of the log")
    func logs() throws {
      let lines = (1...350).map { "line \($0)" }
      let page = try RunningPage(logs: { lines })
      let reply = try Client.get(page.port, "/logs")
      #expect(reply.hasPrefix("HTTP/1.1 200 OK\r\n"))
      #expect(reply.contains("Content-Type: text/plain; charset=utf-8\r\n"))
      let body = try #require(reply.components(separatedBy: "\r\n\r\n").last)
      #expect(body == (51...350).map { "line \($0)\n" }.joined())
    }

    @Test("The log ring keeps the newest lines, oldest first, each after its time")
    func ring() {
      let ring = LogRing(capacity: 3)
      #expect(ring.lines().isEmpty)
      for n in 1...5 {
        ring.append("INFO jetlink.server: \(n)", at: Date(timeIntervalSince1970: 1_790_000_000 + Double(n)))
      }
      let lines = ring.lines()
      #expect(
        lines.map { $0.split(separator: " ", maxSplits: 2).last.map(String.init) } == [
          "INFO jetlink.server: 3", "INFO jetlink.server: 4", "INFO jetlink.server: 5",
        ])
      #expect(lines.allSatisfy { $0.range(of: #"^\d{4}-\d\d-\d\d \d\d:\d\d:\d\d\.\d{3} "#, options: .regularExpression) != nil })
    }

    @Test("A request that never ends its headers is answered 408 after the header timeout")
    func headerTimeout() throws {
      let page = try RunningPage(headerTimeout: 0.3)
      let client = try Client(port: page.port)
      client.send("GET / HTTP/1.1\r\nHost: jetlink.local\r\n")
      let started = Date()
      let reply = client.read(timeout: 5)
      #expect(reply.hasPrefix("HTTP/1.1 408 Request Timeout\r\n"))
      #expect(client.closed)
      #expect(Date().timeIntervalSince(started) < 3)
    }

    @Test("Headers past 8 KB are refused with 431; a garbled request with 400")
    func headerLimits() throws {
      let page = try RunningPage()
      let client = try Client(port: page.port)
      client.send("GET / HTTP/1.1\r\nX-Pad: " + String(repeating: "a", count: 9000) + "\r\n\r\n")
      #expect(client.read().hasPrefix("HTTP/1.1 431 Request Header Fields Too Large\r\n"))
      let garbled = try Client(port: page.port)
      garbled.send("HELLO\r\n\r\n")
      #expect(garbled.read().hasPrefix("HTTP/1.1 400 Bad Request\r\n"))
    }

    @Test("An event stream: its headers, then data lines, then a comment when quiet")
    func stream() throws {
      let page = try RunningPage(keepalive: 0.2)
      let link = ControlEvent.link(LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3"))
      page.feed.publish(link)
      let client = try Client(port: page.port)
      client.send("GET /events HTTP/1.1\r\nAccept: text/event-stream\r\n\r\n")
      let text = client.read { $0.contains(": keepalive\n\n") }
      // Split rather than counted: a Character is a whole "\r\n".
      let parts = text.components(separatedBy: "\r\n\r\n")
      let head = parts[0]
      #expect(head.hasPrefix("HTTP/1.1 200 OK\r\n"))
      #expect(head.contains("Content-Type: text/event-stream\r\n"))
      #expect(head.contains("Cache-Control: no-cache\r\n"))
      #expect(!head.contains("Content-Length"))
      let body = parts.dropFirst().joined(separator: "\r\n\r\n")
      #expect(body.hasPrefix("retry: 2000\n\ndata: {"))
      let events = dataEvents(body)
      #expect(events.count == 1)
      #expect(events.first?["event"] as? String == "link" && events.first?["medium"] as? String == "usb3")
      #expect(events.first?["t"] is Double)
      #expect(body.hasSuffix("\n\n: keepalive\n\n"))

      // Live events keep coming after the replay.
      page.feed.publish(stats(frames: 7))
      #expect(names(client.read { names($0).count == 2 }).last == "stats")
    }

    @Test("At most eight event streams; a ninth page is told to wait, and a closed one frees its place")
    func streamCap() throws {
      let page = try RunningPage(keepalive: 0.1)
      var clients: [Client] = []
      for _ in 0..<8 {
        let client = try Client(port: page.port)
        client.send("GET /events HTTP/1.1\r\n\r\n")
        #expect(client.read { $0.contains("retry: 2000\n\n") }.hasPrefix("HTTP/1.1 200 OK\r\n"))
        clients.append(client)
      }
      let ninth = try Client(port: page.port)
      ninth.send("GET /events HTTP/1.1\r\n\r\n")
      #expect(ninth.read().hasPrefix("HTTP/1.1 503 Service Unavailable\r\n"))
      // The page and the log still load while the streams are full.
      #expect(try Client.get(page.port, "/").hasPrefix("HTTP/1.1 200 OK\r\n"))

      // Its thread sees the page gone at its next keepalive.
      clients.removeFirst()
      #expect(
        eventually(timeout: 5) {
          guard let tenth = try? Client(port: page.port) else { return false }
          tenth.send("GET /events HTTP/1.1\r\n\r\n")
          return tenth.read { $0.contains("\r\n\r\n") }.hasPrefix("HTTP/1.1 200 OK\r\n")
        })
    }

    @Test("A page's socket gives up on a vanished phone in about half a minute")
    func keepalive() throws {
      let fd = socket(AF_INET, Sys.stream, 0)
      #if canImport(Glibc)
        let idle = TCP_KEEPIDLE
      #else
        let idle = TCP_KEEPALIVE
      #endif
      try #require(fd >= 0)
      defer { close(fd) }
      PageServer.tune(fd)
      func option(_ level: Int32, _ name: Int32) -> Int32 {
        var value: Int32 = -1
        var length = socklen_t(MemoryLayout<Int32>.size)
        return getsockopt(fd, level, name, &value, &length) == 0 ? value : -1
      }
      let tcp = Int32(IPPROTO_TCP)
      #expect(option(SOL_SOCKET, SO_KEEPALIVE) != 0)
      #expect(option(tcp, idle) == 15 && option(tcp, TCP_KEEPINTVL) == 5 && option(tcp, TCP_KEEPCNT) == 3)
      #expect(option(tcp, TCP_NODELAY) != 0)
      #if canImport(Glibc)
        #expect(option(tcp, TCP_USER_TIMEOUT) == 30_000)
      #endif
    }

    @Test("Stopping ends every stream after telling it the server is stopping")
    func stop() throws {
      let page = try RunningPage()
      page.feed.publish(.server(ServerEvent(state: "serving", detail: "", backend: "trt", runtimeVersion: "10.16.2.10", device: "Orin")))
      let client = try Client(port: page.port)
      client.send("GET /events HTTP/1.1\r\n\r\n")
      client.read { names($0) == ["server"] }
      page.server.stop()
      let text = client.read(timeout: 3)
      #expect(client.closed)
      #expect(dataEvents(text).map { $0["state"] as? String } == ["serving", "stopping"])
      #expect(throws: (any Error).self) { try Client.get(page.port, "/") }
    }

    /// A browser with the page open holds its event stream, and often a spare
    /// connection it has sent nothing on: answering that one 408 and waiting
    /// for it to close held SIGTERM up a second on the bench Jetson.
    @Test("Stopping closes a connection that sent nothing, and a reply's linger, without waiting on them")
    func stopPromptly() throws {
      let page = try RunningPage()
      let stream = try Client(port: page.port)
      stream.send("GET /events HTTP/1.1\r\n\r\n")
      stream.read { $0.contains("\r\n\r\n") }
      let spare = try Client(port: page.port)
      // a reply the client never reads to the end nor closes: the server lingers on it
      let lingering = try Client(port: page.port)
      lingering.send("GET /nope HTTP/1.1\r\n\r\n")
      Thread.sleep(forTimeInterval: 0.1)
      let started = Date()
      page.server.stop()
      // It waited out its one-second bound before; now a poll or two.
      let took = Date().timeIntervalSince(started)
      #expect(took < 0.6, "stop took \(took) s")
      #expect(spare.read(timeout: 3).isEmpty && spare.closed)
      _ = stream.read(timeout: 3)
      #expect(stream.closed)
    }

    @Test("The page only watches the controller: no catalog fetch, no download, no build")
    func readOnly() async throws {
      let scratch = try TemporaryDirectory()
      let server = try Server(
        configuration: Server.Configuration(host: "127.0.0.1", port: 0, cacheRoot: scratch.url, preload: false, listen: true), backend: NamingBackend())
      let registry = CountingRegistry()
      let controller = ServerController(server: server, registry: registry)
      let page = try RunningPage()
      page.server.watch(controller, version: "0.7.0-test")

      let client = try Client(port: page.port)
      client.send("GET /events HTTP/1.1\r\n\r\n")
      let first = dataEvents(client.read { names($0).count >= 5 })
      #expect(first.compactMap { $0["event"] as? String } == ["hello", "server", "link", "engine", "inventory"])
      #expect(first[0]["version"] as? String == "0.7.0-test")
      #expect(first[1]["state"] as? String == "serving" && first[1]["backend"] as? String == "ort")

      // What the server's host says reaches the page through the controller.
      server.host.emit(.link(LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3")))
      server.host.emit(
        .stats(
          StatsEvent(frames: 3, fps: 20, servedMs: .init(mean: 1, p99: 2, max: 3), stagesMs: .init(queue: 0, gpu: 1, other: 0, send: 0), slow: 0, windowS: 1)))
      let live = dataEvents(client.read { names($0).count >= 7 })
      #expect(live.suffix(2).compactMap { $0["event"] as? String } == ["link", "stats"])
      #expect(live.last?["frames"] as? Int == 3)

      // Every path a page takes, a few times over.
      for _ in 0..<3 {
        #expect(try Client.get(page.port, "/").hasPrefix("HTTP/1.1 200 OK"))
        #expect(try Client.get(page.port, "/logs").hasPrefix("HTTP/1.1 200 OK"))
        let again = try Client(port: page.port)
        again.send("GET /events HTTP/1.1\r\n\r\n")
        again.read { names($0).count >= 6 }
      }
      try await Task.sleep(for: .milliseconds(200))
      #expect(registry.reachedOut == 0)
      #expect(registry.count("catalog") == 0 && registry.count("fetch") == 0 && registry.count("resolvePointer") == 0)
      // Only the seed read the disk: every later page is given what the feed kept.
      #expect(registry.count("inventory") == 1)

      // An observing controller makes no stream, so it buffers nothing.
      controller.finish()
      var buffered = 0
      for await _ in controller.events { buffered += 1 }
      #expect(buffered == 0)
      page.server.stop()
      server.stop()
    }
  }
#endif
