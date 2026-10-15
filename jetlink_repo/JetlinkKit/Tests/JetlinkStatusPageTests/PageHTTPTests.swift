import Foundation
import Testing

@testable import JetlinkStatusPage

@Suite("Status page requests")
struct PageHTTPTests {
  func parse(_ text: String, limit: Int = 8192) -> PageRequest {
    PageRequest.parse(Array(text.utf8), limit: limit)
  }

  /// The request line alone, for the tests that only look at that.
  func line(_ text: String, limit: Int = 8192) -> String? {
    guard case .request(let head) = parse(text, limit: limit) else { return nil }
    return head.method + " " + head.path
  }

  @Test("A request line, its headers and the blank line")
  func request() {
    #expect(line("GET /events HTTP/1.1\r\nHost: jetlink.local\r\nAccept: text/event-stream\r\n\r\n") == "GET /events")
    #expect(line("GET /?reload=1 HTTP/1.0\n\n") == "GET /")
    #expect(line("POST /logs HTTP/1.1\r\n\r\n") == "POST /logs")
  }

  @Test("Headers by lowercased name; cookies joined; where the body starts")
  func headers() throws {
    let text =
      "POST /api/login HTTP/1.1\r\nHost: jetlink.local:5600\r\nCONTENT-TYPE: application/json\r\nContent-Length: 17\r\n"
      + "Cookie: a=1\r\ncookie: jetlink_session=x\r\nX-Pad:\tspaced \r\n\r\n{\"password\":\"p\"}"
    guard case .request(let head) = parse(text) else {
      Issue.record("not a request")
      return
    }
    #expect(head.headers["host"] == "jetlink.local:5600")
    #expect(head.headers["content-type"] == "application/json")
    #expect(head.contentLength == 17)
    #expect(head.headers["cookie"] == "a=1; jetlink_session=x")
    #expect(head.headers["x-pad"] == "spaced")
    #expect(String(decoding: Array(text.utf8)[head.length...], as: UTF8.self) == "{\"password\":\"p\"}")
    guard case .request(let bare) = parse("GET / HTTP/1.1\nContent-Length: x\n\nrest") else {
      Issue.record("not a request")
      return
    }
    #expect(bare.contentLength == -1)
    #expect(String(decoding: Array("GET / HTTP/1.1\nContent-Length: x\n\nrest".utf8)[bare.length...], as: UTF8.self) == "rest")
    #expect(parse("GET / HTTP/1.1\r\nno colon here\r\n\r\n") == .bad)
    #expect(parse("GET / HTTP/1.1\r\nBad Name: x\r\n\r\n") == .bad)
    #expect(parse("GET / HTTP/1.1\r\n: empty name\r\n\r\n") == .bad)
  }

  @Test("Nothing is decided before the blank line")
  func incomplete() {
    #expect(parse("") == .incomplete)
    #expect(parse("GET / HTTP/1.1\r\n") == .incomplete)
    #expect(parse("GET / HTTP/1.1\r\nHost: x\r\n") == .incomplete)
  }

  @Test("The request line and headers are capped, blank line included")
  func capped() {
    let head = "GET / HTTP/1.1\r\nX-Pad: "
    let fits = head + String(repeating: "a", count: 8192 - head.utf8.count - 4) + "\r\n\r\n"
    #expect(fits.utf8.count == 8192)
    #expect(line(fits) == "GET /")
    let over = head + String(repeating: "a", count: 8192 - head.utf8.count - 3) + "\r\n\r\n"
    #expect(parse(over) == .tooLarge)
    #expect(parse(String(repeating: "a", count: 9000)) == .tooLarge)
  }

  @Test("A malformed request line is refused")
  func bad() {
    #expect(parse("HELLO\r\n\r\n") == .bad)
    #expect(parse("GET /\r\n\r\n") == .bad)
    #expect(parse("GET http://jetlink.local/ HTTP/1.1\r\n\r\n") == .bad)
    #expect(parse("GET / SPDY/3\r\n\r\n") == .bad)
    #expect(PageRequest.parse(Array("GET /\u{1} HTTP/1.1\r\n\r\n".utf8), limit: 8192) == .bad)
  }

  @Test("A reply closes its connection and says how long it is")
  func reply() {
    let text = String(decoding: PageResponse.text(404, "not found\n"), as: UTF8.self)
    #expect(text.hasPrefix("HTTP/1.1 404 Not Found\r\n"))
    #expect(text.contains("Content-Length: 10\r\n"))
    #expect(text.contains("Connection: close\r\n"))
    #expect(text.contains("X-Content-Type-Options: nosniff\r\n") && text.contains("Referrer-Policy: no-referrer\r\n"))
    #expect(text.hasSuffix("\r\n\r\nnot found\n"))
  }

  @Test("The page may not be framed and loads nothing from elsewhere; JSON is never stored")
  func policies() {
    let replies = PageResponse.page(Data("<p>".utf8), etag: "\"abc\"")
    let page = String(decoding: replies.whole, as: UTF8.self)
    #expect(page.contains("Content-Security-Policy: default-src 'none'; ") && page.contains("frame-ancestors 'none'"))
    #expect(page.contains("X-Frame-Options: DENY\r\n") && page.contains("ETag: \"abc\"\r\n") && page.hasSuffix("\r\n\r\n<p>"))
    let unchanged = String(decoding: replies.unchanged, as: UTF8.self)
    #expect(unchanged.hasPrefix("HTTP/1.1 304 Not Modified\r\n") && unchanged.hasSuffix("\r\n\r\n"))
    let json = String(decoding: PageResponse.json(401, ["error": "no"], cookie: "c=1"), as: UTF8.self)
    #expect(json.hasPrefix("HTTP/1.1 401 Unauthorized\r\n"))
    #expect(json.contains("Cache-Control: no-store\r\n") && json.contains("Set-Cookie: c=1\r\n"))
    #expect(json.hasSuffix("{\"error\":\"no\"}"))
  }
}
