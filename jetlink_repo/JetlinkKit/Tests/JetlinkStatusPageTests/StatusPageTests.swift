import Foundation
import Testing

@testable import JetlinkStatusPage

@Suite("Status page")
struct StatusPageTests {
  // The page ships where it is served, beside jetlink-server on Linux and
  // macOS; an Android test runner has no bundle beside it.
  #if canImport(Darwin) || canImport(Glibc)
    @Test("The page is bundled with the module")
    func bundled() throws {
      let page = String(decoding: try StatusPage.page(), as: UTF8.self)
      #expect(page.hasPrefix("<!doctype html>"))
    }

    @Test("The page needs nothing but this server: the comma's hotspot has no internet")
    func offline() throws {
      let page = String(decoding: try StatusPage.page(), as: UTF8.self)
      // Nothing loaded from elsewhere: every src and href is this server's or data.
      #expect(page.range(of: #"(src|href)\s*=\s*["']?(https?:)?//"#, options: .regularExpression) == nil)
      #expect(!page.contains("//cdn") && !page.contains("@import") && !page.contains("url(http"))
      #expect(page.contains("new EventSource('events')") && page.contains("fetch('logs'"))
      // It reads every event the feed relays, and the host's two.
      for name in ["host", "hello", "server", "link", "engine", "inventory", "stats", "hw", "catalog", "download", "benchmark"] {
        #expect(page.contains("case '\(name)':"), "\(name)")
      }
      #expect(!page.contains("\u{2014}"))
    }
  #endif

  @Test("A binary without the page's bundle is told so rather than stopped")
  func missing() {
    let nowhere = FileManager.default.temporaryDirectory.appendingPathComponent("jetlink-no-page-\(UUID().uuidString)")
    let error = #expect(throws: StatusPage.Unavailable.self) { try StatusPage.page(searching: [nowhere]) }
    #expect(error?.searched == [nowhere.path])
  }
}
