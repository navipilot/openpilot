import Foundation
import JetlinkKit
import JetlinkServer
import JetlinkTestSupport
import Testing

@testable import JetlinkStatusPage

@Suite("Status page feed")
struct PageFeedTests {
  let link = ControlEvent.link(LinkEvent(state: .connected, detail: "", peer: "usb", medium: "usb3"))

  /// The events a page is sent next, as it parses them.
  func take(_ feed: PageFeed, _ stream: PageFeed.Stream) -> [[String: Any]] {
    (feed.next(stream, timeout: 0.01) ?? []).flatMap { dataEvents(String(decoding: $0, as: UTF8.self)) }
  }

  @Test("An event is one data line, as jsonLine() writes it, then a blank line")
  func framing() throws {
    let line = link.jsonLine(at: Date(timeIntervalSince1970: 1_790_000_000.5))
    #expect(PageFeed.sse(line) == Data("data: ".utf8) + line.dropLast() + Data("\n\n".utf8))
    #expect(!line.dropLast().contains(0x0A))
    let host = String(decoding: PageFeed.frame("host", ["hostname": "jetlink", "jetson": true], at: Date(timeIntervalSince1970: 2.5)), as: UTF8.self)
    #expect(host == #"data: {"event":"host","hostname":"jetlink","jetson":true,"t":2.5}"# + "\n\n")
    // What JSONSerialization cannot write is an error event, not a crash.
    let broken = String(decoding: PageFeed.frame("hw", ["when": Date()]), as: UTF8.self)
    #expect(broken.hasPrefix(#"data: {"error":"#) && broken.hasSuffix("\n\n"))
  }

  @Test("A page that opens late gets the latest of each and the last two minutes of stats")
  func replay() throws {
    let clock = Locked<TimeInterval>(1000)
    let feed = PageFeed(hardware: nil, clock: { clock.value })
    feed.publish(.hello(HelloEvent(version: "0.7.0")))
    feed.publish(.server(ServerEvent(state: "serving", detail: "", backend: "trt", runtimeVersion: "10.16.2.10", device: "Orin")))
    feed.publish(.link(.waiting))
    feed.publish(.engine(.none))
    feed.publish(
      .inventory(InventoryEvent(loaded: nil, lastLoaded: nil, models: [], artifacts: [], disk: InventoryDisk(modelsBytes: 0, enginesBytes: 0, freeBytes: 0))))
    feed.publish(.catalog(CatalogEvent(fetchedAt: nil, url: "", defaultRef: "", error: nil, models: [])))
    feed.publish(.download(DownloadEvent(sha256: "a", ref: nil, state: "started", frac: 0, bytes: 0, total: 1, rateBps: 0, detail: "", source: nil)))
    feed.publish(stats(frames: 1))
    for (at, frames) in [(1030.0, 2), (1070, 3), (1110, 4), (1128, 5)] {
      feed.flush()
      clock.value = at
      feed.publish(stats(frames: frames))
    }
    feed.publish(link)
    feed.flush()
    clock.value = 1130

    let stream = try #require(feed.open())
    let replayed = take(feed, stream)
    // the catalog and a download only come after a signed-in page asked for them
    #expect(
      replayed.map { $0["event"] as? String } == ["hello", "server", "link", "engine", "inventory", "catalog", "download", "stats", "stats", "stats", "stats"])
    #expect(replayed[2]["state"] as? String == "connected" && replayed[2]["medium"] as? String == "usb3")
    // the first stats came 130 s before the page: past the chart's two minutes
    #expect(replayed.compactMap { $0["frames"] as? Int } == [2, 3, 4, 5])

    feed.publish(stats(frames: 6))
    feed.flush()
    #expect(take(feed, stream).map { $0["event"] as? String } == ["stats"])
  }

  @Test("The controller's state fills in only what no event has said")
  func seed() throws {
    let feed = PageFeed(hardware: nil)
    feed.publish(link)
    feed.seed([.server(ServerEvent(state: "serving", detail: "", backend: nil, runtimeVersion: nil, device: nil)), .link(.waiting), .engine(.none)])
    let stream = try #require(feed.open())
    let replayed = take(feed, stream)
    #expect(replayed.map { $0["event"] as? String } == ["server", "link", "engine"])
    #expect(replayed[1]["state"] as? String == "connected")
  }

  @Test("A page that falls too far behind is dropped, to reconnect")
  func behind() throws {
    let feed = PageFeed(hardware: nil, limits: PageFeed.Limits(behind: 4))
    let stream = try #require(feed.open())
    for n in 0..<5 { feed.publish(stats(frames: n)) }
    feed.flush()
    #expect(feed.next(stream, timeout: 0.01) == nil)
  }

  @Test("No sampling before a page opens; it stops a grace after the last one closes")
  func sampling() throws {
    let clock = Locked<TimeInterval>(0)
    let hardware = FakeHardware()
    let feed = PageFeed(hardware: hardware, limits: PageFeed.Limits(grace: 60, period: 0.01), clock: { clock.value })
    let host = { (frames: [Data]) in frames.contains { String(decoding: $0, as: UTF8.self).contains(#""event":"host""#) } }
    feed.publish(link)
    feed.flush()
    Thread.sleep(forTimeInterval: 0.1)
    #expect(hardware.samples == 0 && hardware.hosts == 0)

    let stream = try #require(feed.open())
    #expect(eventually { hardware.samples >= 3 })
    #expect(hardware.hosts == 1)
    #expect(host(feed.next(stream, timeout: 1) ?? []))

    clock.value = 10
    feed.close(stream)
    clock.value = 69
    let before = hardware.samples
    #expect(eventually { hardware.samples > before + 2 })

    clock.value = 71
    #expect(eventually { !feed.isSampling })
    let stopped = hardware.samples
    Thread.sleep(forTimeInterval: 0.1)
    #expect(hardware.samples == stopped)

    // A page after the pause: sampled again, the host not read again, and
    // the CPU counters forgotten so the first load is not a pause's average.
    let again = try #require(feed.open())
    #expect(eventually { hardware.samples > stopped })
    #expect(hardware.hosts == 1 && hardware.resets == 2)
    #expect(host(feed.next(again, timeout: 1) ?? []))
    feed.stop()
    #expect(eventually { !feed.isSampling })
  }

  // Lowered where the page is served: Linux and macOS.
  #if canImport(Darwin) || canImport(Glibc)
    @Test("The page's threads run at the lowest priority, and only they do")
    func priority() {
      let seen = Locked<Int32?>(nil)
      PageThread.start("jetlink-page-test") {
        #if os(Linux)
          let nice = getpriority(__priority_which_t(PRIO_PROCESS.rawValue), 0)
        #else
          let nice: Int32 = Thread.current.qualityOfService == .background ? 19 : 0
        #endif
        seen.value = nice
      }
      #expect(eventually { seen.value != nil })
      #expect(seen.value == 19)
      #if os(Linux)
        #expect(getpriority(__priority_which_t(PRIO_PROCESS.rawValue), 0) != 19)
      #endif
    }
  #endif
}
