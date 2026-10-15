import Foundation
import JetlinkKit

/// A request's line and headers, as far as the page reads them.
struct PageHead: Equatable {
  var method: String
  /// Without its query.
  var path: String
  /// Names lowercased; a repeated header's values joined as HTTP joins them.
  var headers: [String: String] = [:]
  /// The request line and headers with the blank line: the body starts here.
  var length = 0

  /// nil with no Content-Length; -1 for one that is not a number.
  var contentLength: Int? {
    guard let value = headers["content-length"] else { return nil }
    return Int(value.trimmingCharacters(in: .whitespaces)).flatMap { $0 >= 0 ? $0 : nil } ?? -1
  }
}

/// A request as far as the page reads one: its request line and headers,
/// with the whole capped in size. The body, if any, is the server's to read.
enum PageRequest: Equatable {
  /// No blank line yet: read more.
  case incomplete
  /// The request line and headers passed the cap before their blank line.
  case tooLarge
  case bad
  case request(PageHead)

  /// The request in `bytes`, whose request line and headers may take up to
  /// `limit` bytes with the blank line that ends them.
  static func parse(_ bytes: [UInt8], limit: Int) -> PageRequest {
    let window = bytes.prefix(limit)
    guard let (end, blank) = headerEnd(window) else {
      return bytes.count >= limit ? .tooLarge : .incomplete
    }
    let lines = window[..<end].split(separator: 0x0A, omittingEmptySubsequences: false).map { line in
      line.last == 0x0D ? line.dropLast() : line
    }
    guard let first = lines.first, isText(first) else { return .bad }
    let parts = String(decoding: first, as: UTF8.self).split(separator: " ", omittingEmptySubsequences: false)
    guard parts.count == 3, parts[2].hasPrefix("HTTP/1."), !parts[0].isEmpty, parts[1].hasPrefix("/") else { return .bad }
    let target = parts[1]
    let path = target[..<(target.firstIndex { $0 == "?" || $0 == "#" } ?? target.endIndex)]
    var head = PageHead(method: String(parts[0]), path: String(path), length: end + blank)
    for line in lines.dropFirst() where !line.isEmpty {
      guard isText(line, tabs: true), let colon = line.firstIndex(of: 0x3A), colon > line.startIndex else { return .bad }
      let name = String(decoding: line[..<colon], as: UTF8.self).lowercased()
      guard !name.contains(" "), !name.contains("\t") else { return .bad }
      let value = String(decoding: line[line.index(after: colon)...], as: UTF8.self).trimmingCharacters(in: .whitespaces)
      if let earlier = head.headers[name] {
        head.headers[name] = earlier + (name == "cookie" ? "; " : ", ") + value
      } else {
        head.headers[name] = value
      }
    }
    return .request(head)
  }

  /// Printable ASCII, and tabs in a header line; anything else is no request.
  private static func isText(_ line: ArraySlice<UInt8>, tabs: Bool = false) -> Bool {
    line.allSatisfy { $0 >= 0x20 && $0 < 0x7F || tabs && $0 == 0x09 }
  }

  /// Where the head ends and how long the blank line after it is, CRLF or
  /// bare LF.
  private static func headerEnd(_ bytes: ArraySlice<UInt8>) -> (Int, Int)? {
    var index = bytes.startIndex
    while index < bytes.endIndex {
      if bytes[index] == 0x0A {
        let next = bytes.index(after: index)
        if next < bytes.endIndex && bytes[next] == 0x0A { return (index, 2) }
        if next + 1 < bytes.endIndex && bytes[next] == 0x0D && bytes[next + 1] == 0x0A { return (index, 3) }
      }
      index += 1
    }
    return nil
  }
}

/// A whole response: status, a few headers, the body. Every connection
/// closes after its one response; an event stream is one long response.
enum PageResponse {
  static let reasons = [
    200: "OK", 304: "Not Modified", 400: "Bad Request", 401: "Unauthorized", 403: "Forbidden", 404: "Not Found", 408: "Request Timeout",
    409: "Conflict", 411: "Length Required", 413: "Content Too Large", 415: "Unsupported Media Type", 429: "Too Many Requests",
    431: "Request Header Fields Too Large", 500: "Internal Server Error", 503: "Service Unavailable", 504: "Gateway Timeout",
  ]

  /// The page itself: its script and styles are inline, it talks only to
  /// this server and no other site may frame it.
  static let pagePolicy =
    "default-src 'none'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; img-src data:; connect-src 'self'; "
    + "manifest-src 'self'; base-uri 'none'; form-action 'none'; frame-ancestors 'none'"

  /// `cache`: no-cache for the page and the stream, which a browser may keep
  /// but must ask again for; no-store for what a sign-in guards.
  static func head(_ status: Int, type: String, length: Int? = nil, cache: String = "no-cache", headers: [(String, String)] = []) -> Data {
    var text = "HTTP/1.1 \(status) \(reasons[status] ?? "Error")\r\nContent-Type: \(type)\r\n"
    if let length { text += "Content-Length: \(length)\r\n" }
    text += "Cache-Control: \(cache)\r\nX-Content-Type-Options: nosniff\r\nReferrer-Policy: no-referrer\r\n"
    for (name, value) in headers { text += "\(name): \(value)\r\n" }
    text += "Connection: close\r\n\r\n"
    return Data(text.utf8)
  }

  static func whole(_ status: Int, type: String, body: Data, cache: String = "no-cache", headers: [(String, String)] = []) -> Data {
    head(status, type: type, length: body.count, cache: cache, headers: headers) + body
  }

  static func text(_ status: Int, _ body: String, cache: String = "no-cache") -> Data {
    whole(status, type: "text/plain; charset=utf-8", body: Data(body.utf8), cache: cache)
  }

  /// The page and the 304 for a browser that has it, made once: `etag` names
  /// this build's page.
  static func page(_ body: Data, etag: String) -> (whole: Data, unchanged: Data) {
    let headers = [("ETag", etag), ("Content-Security-Policy", pagePolicy), ("X-Frame-Options", "DENY")]
    return (whole(200, type: "text/html; charset=utf-8", body: body, headers: headers), head(304, type: "text/html; charset=utf-8", headers: headers))
  }

  /// A JSON reply, never cached. `cookie` sets or clears the sign-in.
  static func json(_ status: Int, _ object: [String: Any], cookie: String? = nil) -> Data {
    let body = ControlJSON.data(object) ?? Data(#"{"error":"the reply is not JSON"}"#.utf8)
    return whole(status, type: "application/json", body: body, cache: "no-store", headers: cookie.map { [("Set-Cookie", $0)] } ?? [])
  }

  static func error(_ status: Int, _ message: String) -> Data {
    json(status, ["error": message])
  }
}
