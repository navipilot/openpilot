import Foundation
import JetlinkServer
import JetlinkTestSupport
import Testing

@testable import JetlinkStatusPage

@Suite("Web page sign-in")
struct WebAuthTests {
  @Test("PBKDF2-HMAC-SHA256 gives RFC 7914's and the usual vectors, more than one block included")
  func pbkdf2() {
    #expect(
      hexString(Array(AuthFile.pbkdf2("passwd", salt: Data("salt".utf8), rounds: 1, length: 64)))
        == "55ac046e56e3089fec1691c22544b605f94185216dde0465e68b9d57c20dacbc49ca9cccf179b645991664b39d77ef317c71b845b1e30bd509112041d3a19783")
    #expect(
      hexString(Array(AuthFile.pbkdf2("password", salt: Data("salt".utf8), rounds: 4096, length: 32)))
        == "c5e478d59288c841aa530db6845c4c8d962893a001ce4e11a4963873aa98134a")
  }

  @Test("A file verifies its own password and no other; a rotated one keeps it under a new key")
  func file() {
    let file = AuthFile.make(password: "correct horse", iterations: 1000)
    #expect(file.verify("correct horse"))
    #expect(!file.verify("correct horsf") && !file.verify("") && !file.verify("correct horse "))
    #expect(file.salt.count == 16 && file.key.count == 32 && file.hash.count == 32)
    let other = AuthFile.make(password: "correct horse", iterations: 1000)
    #expect(other.salt != file.salt && other.hash != file.hash && other.key != file.key)
    let rotated = file.rotated()
    #expect(rotated.verify("correct horse") && rotated.key != file.key && rotated.hash == file.hash)
  }

  @Test("Written root's alone, read back the same; a file of another shape is refused")
  func readWrite() throws {
    let scratch = try TemporaryDirectory()
    let url = scratch.url.appending(path: "etc/web-auth.json")
    let file = AuthFile.make(password: "correct horse", iterations: 1000)
    try file.write(url)
    let mode = try FileManager.default.attributesOfItem(atPath: url.path)[.posixPermissions] as? NSNumber
    #expect(mode?.intValue == 0o600)
    #expect(try AuthFile.read(url) == file)
    let text = try String(contentsOf: url, encoding: .utf8)
    #expect(text.contains("\"kdf\" : \"pbkdf2-sha256\"") && text.contains("\"iterations\" : 1000") && !text.contains("correct horse"))
    // nothing left beside it
    #expect(try FileManager.default.contentsOfDirectory(atPath: url.deletingLastPathComponent().path) == ["web-auth.json"])
    try Data(#"{"version":2}"#.utf8).write(to: url)
    #expect(throws: (any Error).self) { try AuthFile.read(url) }
  }

  @Test("A generated password: three groups of four, letters and digits that do not look alike")
  func generated() {
    let made = (0..<50).map { _ in GeneratedPassword.make() }
    #expect(Set(made).count == 50)
    for password in made {
      #expect(password.range(of: "^[2-9a-hjkmnp-z]{4}-[2-9a-hjkmnp-z]{4}-[2-9a-hjkmnp-z]{4}$", options: .regularExpression) != nil, "\(password)")
      #expect(GeneratedPassword.problem(with: password) == nil)
    }
    #expect(GeneratedPassword.problem(with: "short") != nil)
    #expect(GeneratedPassword.problem(with: String(repeating: "a", count: 129)) != nil)
    #expect(GeneratedPassword.problem(with: "eight\u{7}chars") != nil)
    #expect(GeneratedPassword.problem(with: "naïve ünïcode ok") == nil)
  }

  @Test("A session token is good until it expires, under the key that signed it")
  func token() {
    let key = Random.bytes(32)
    let now = Date(timeIntervalSince1970: 1_790_000_000)
    let token = SessionToken.issue(key: key, expires: now.addingTimeInterval(60))
    #expect(SessionToken.verify(token, key: key, now: now) == Date(timeIntervalSince1970: 1_790_000_060))
    #expect(SessionToken.verify(token, key: key, now: now.addingTimeInterval(61)) == nil)
    #expect(SessionToken.verify(token, key: Random.bytes(32), now: now) == nil)
    // a later expiry in the same token does not verify
    var parts = token.split(separator: ".").map(String.init)
    parts[1] = String(Int(parts[1])! + 86400)
    #expect(SessionToken.verify(parts.joined(separator: "."), key: key, now: now) == nil)
    for junk in ["", "v1", "v1.1.2", "v2.1.2.3", "v1.x.nonce.mac", token + "."] {
      #expect(SessionToken.verify(junk, key: key, now: now) == nil, "\(junk)")
    }
  }

  @Test("The cookie's value is found among others")
  func cookieValue() {
    #expect(WebAuth.cookieValue("a=1; jetlink_session=v1.2.3.4; b=2") == "v1.2.3.4")
    #expect(WebAuth.cookieValue("jetlink_session=") == nil)
    #expect(WebAuth.cookieValue("xjetlink_session=1") == nil)
    #expect(WebAuth.cookieValue(nil) == nil)
  }

  @Test("Five free tries an address, then a doubling wait; a success forgets it")
  func throttle() {
    let throttle = LoginThrottle()
    var now: TimeInterval = 1000
    for _ in 0..<4 {
      #expect(throttle.admit("a", now: now) == nil)
      #expect(throttle.record("a", success: false, now: now) == nil)
      now += 1
    }
    #expect(throttle.admit("a", now: now) == nil)
    #expect(throttle.record("a", success: false, now: now) == 1)
    #expect(throttle.admit("a", now: now + 0.5) != nil)
    // another address is not held up by this one
    #expect(throttle.admit("b", now: now + 0.5) == nil)
    now += 1
    #expect(throttle.admit("a", now: now) == nil)
    #expect(throttle.record("a", success: false, now: now) == 2)
    now += 2
    #expect(throttle.admit("a", now: now) == nil)
    #expect(throttle.record("a", success: true, now: now) == nil)
    now += 1
    #expect(throttle.admit("a", now: now) == nil)
    #expect(throttle.record("a", success: false, now: now) == nil)
  }

  @Test("No more than two checks a second from everyone together")
  func throttleOverall() {
    let throttle = LoginThrottle()
    #expect(throttle.admit("a", now: 10) == nil)
    #expect(throttle.admit("b", now: 10.1) == nil)
    let wait = throttle.admit("c", now: 10.2)
    #expect(wait != nil && wait! <= 0.81)
    #expect(throttle.admit("c", now: 11.05) == nil)
  }

  @Test("The waits stop at five minutes")
  func longestWait() {
    let throttle = LoginThrottle()
    var last: TimeInterval?
    for n in 0..<20 {
      last = throttle.record("a", success: false, now: TimeInterval(n * 1000))
    }
    #expect(last == LoginThrottle.longestWait)
  }

  @Test("Sign-in: the file is read again when it changes; a cookie is renewed once a day old")
  func signIn() throws {
    let scratch = try TemporaryDirectory()
    let url = scratch.url.appending(path: "web-auth.json")
    let clock = Locked(Date(timeIntervalSince1970: 1_790_000_000))
    let auth = WebAuth(url: url) { clock.value }
    #expect(!auth.isConfigured)
    try AuthFile.make(password: "first password", iterations: 100).write(url)
    let file = try #require(auth.file)
    guard case .signedIn(let cookie) = auth.login("first password", file: file, address: "1.2.3.4") else {
      Issue.record("not signed in")
      return
    }
    #expect(cookie.hasPrefix("jetlink_session=v1.") && cookie.hasSuffix("; Max-Age=34560000; Path=/; HttpOnly; SameSite=Strict"))
    let header = String(cookie.split(separator: ";")[0])
    #expect(auth.session(cookieHeader: header, file: file) == WebAuth.Session(renewal: nil))
    clock.value += 86400 + 1
    let renewed = try #require(auth.session(cookieHeader: header, file: file)?.renewal)
    #expect(renewed != cookie)
    clock.value += 400 * 86400
    #expect(auth.session(cookieHeader: header, file: file) == nil)

    // jetlink password writes a new file under a running server
    clock.value = Date(timeIntervalSince1970: 1_790_000_000)
    try AuthFile.make(password: "second password", iterations: 100).write(url)
    let second = try #require(auth.file)
    #expect(second != file)
    #expect(auth.session(cookieHeader: header, file: second) == nil)
    clock.value += 1
    #expect(auth.login("first password", file: second, address: "1.2.3.4") == .refused(.wrong(retryAfter: nil)))
    try FileManager.default.removeItem(at: url)
    #expect(auth.file == nil && !auth.isConfigured)
  }

  @Test("A new password needs the current one, and signs every other device out")
  func change() throws {
    let scratch = try TemporaryDirectory()
    let url = scratch.url.appending(path: "web-auth.json")
    try AuthFile.make(password: "first password", iterations: 100).write(url)
    // a second between checks: no more than two go through a second
    let clock = Locked(Date(timeIntervalSince1970: 1_790_000_000))
    let auth = WebAuth(url: url) { clock.value }
    let file = try #require(auth.file)
    guard case .signedIn(let old) = auth.login("first password", file: file, address: "a") else {
      Issue.record("not signed in")
      return
    }
    clock.value += 1
    #expect(auth.changePassword(current: "wrong one!", new: "second password", file: file, address: "a") == .refused(.wrong(retryAfter: nil)))
    clock.value += 1
    guard case .rejected = auth.changePassword(current: "first password", new: "short", file: file, address: "a") else {
      Issue.record("a short password was taken")
      return
    }
    guard case .changed(let new) = auth.changePassword(current: "first password", new: "second password", file: file, address: "a") else {
      Issue.record("not changed")
      return
    }
    let changed = try #require(auth.file)
    #expect(auth.session(cookieHeader: String(old.split(separator: ";")[0]), file: changed) == nil)
    #expect(auth.session(cookieHeader: String(new.split(separator: ";")[0]), file: changed) != nil)
    #expect(changed.verify("second password") && changed.iterations == 100)

    let mine = try auth.signOutEverywhere(file: changed)
    let rotated = try #require(auth.file)
    #expect(auth.session(cookieHeader: String(new.split(separator: ";")[0]), file: rotated) == nil)
    #expect(auth.session(cookieHeader: String(mine.split(separator: ";")[0]), file: rotated) != nil)
    #expect(rotated.verify("second password"))
  }

  @Test("A password change is a check too: it counts against the two a second")
  func changeThrottled() throws {
    let scratch = try TemporaryDirectory()
    let url = scratch.url.appending(path: "web-auth.json")
    try AuthFile.make(password: "first password", iterations: 100).write(url)
    let clock = Locked(Date(timeIntervalSince1970: 1_790_000_000))
    let auth = WebAuth(url: url) { clock.value }
    let file = try #require(auth.file)
    _ = auth.login("first password", file: file, address: "a")
    _ = auth.login("first password", file: file, address: "b")
    guard case .refused(.wait) = auth.changePassword(current: "first password", new: "second password", file: file, address: "a") else {
      Issue.record("a third check in a second went through")
      return
    }
    #expect(try AuthFile.read(url).verify("first password"))
  }
}
