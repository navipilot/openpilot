import CryptoExtras
import Foundation
import JetlinkRegistry

#if canImport(Glibc)
  import Glibc
#elseif canImport(Darwin)
  import Darwin
#endif

#if canImport(CryptoKit)
  import CryptoKit
#else
  import Crypto
#endif

/// The web page's password, as `/etc/jetlink/web-auth.json` keeps it: a
/// PBKDF2-HMAC-SHA256 hash with its salt and rounds, and the key that signs
/// the sign-in cookies. Root's alone (0600): the key lets anyone who reads it
/// make a cookie.
public struct AuthFile: Codable, Equatable, Sendable {
  public static let version = 1
  /// swift-crypto's floor for PBKDF2-SHA256: about 20 ms on an M1 core.
  public static let rounds = 210_000

  public var version: Int
  public var kdf: String
  public var iterations: Int
  public var salt: Data
  public var hash: Data
  public var key: Data

  /// A new file for `password`: a fresh salt and a fresh key, so every
  /// cookie signed under the old one stops working.
  public static func make(password: String, iterations: Int = AuthFile.rounds) -> AuthFile {
    let salt = Random.bytes(16)
    return AuthFile(
      version: version, kdf: "pbkdf2-sha256", iterations: iterations, salt: salt,
      hash: pbkdf2(password, salt: salt, rounds: iterations, length: 32), key: Random.bytes(32))
  }

  public func verify(_ password: String) -> Bool {
    guard kdf == "pbkdf2-sha256", iterations > 0, !hash.isEmpty else { return false }
    return Equal.constantTime(AuthFile.pbkdf2(password, salt: salt, rounds: iterations, length: hash.count), hash)
  }

  /// The same password under a new key: signs everyone out.
  public func rotated() -> AuthFile {
    var copy = self
    copy.key = Random.bytes(32)
    return copy
  }

  /// RFC 8018's PBKDF2 with HMAC-SHA256, the platform's own: CommonCrypto on
  /// Apple's, BoringSSL elsewhere.
  static func pbkdf2(_ password: String, salt: Data, rounds: Int, length: Int) -> Data {
    // Only a bad length or round count throws, and both are this file's.
    let key = try! KDF.Insecure.PBKDF2.deriveKey(
      from: Data(password.utf8), salt: salt, using: .sha256, outputByteCount: length, unsafeUncheckedRounds: max(rounds, 1))
    return key.withUnsafeBytes { Data($0) }
  }

  public static func read(_ url: URL) throws -> AuthFile {
    let file = try JSONDecoder().decode(AuthFile.self, from: Data(contentsOf: url))
    guard file.version == version, file.key.count >= 32 else {
      throw CocoaError(.fileReadCorruptFile, userInfo: [NSFilePathErrorKey: url.path])
    }
    return file
  }

  /// Written beside itself and renamed over, 0600 from the start, so no
  /// reader ever sees half a file or a moment of a wider mode.
  public func write(_ url: URL) throws {
    let encoder = JSONEncoder()
    encoder.outputFormatting = [.sortedKeys, .prettyPrinted]
    let data = try encoder.encode(self) + Data("\n".utf8)
    let directory = url.deletingLastPathComponent()
    try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
    let temporary = directory.appending(path: ".\(url.lastPathComponent).\(ProcessInfo.processInfo.processIdentifier).tmp")
    let fd = open(temporary.path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0o600)
    guard fd >= 0 else { throw CocoaError(.fileWriteNoPermission, userInfo: [NSFilePathErrorKey: temporary.path]) }
    let handle = FileHandle(fileDescriptor: fd, closeOnDealloc: true)
    do {
      try handle.write(contentsOf: data)
      try handle.synchronize()
      try handle.close()
    } catch {
      unlink(temporary.path)
      throw error
    }
    guard rename(temporary.path, url.path) == 0 else {
      unlink(temporary.path)
      throw CocoaError(.fileWriteUnknown, userInfo: [NSFilePathErrorKey: url.path])
    }
  }
}

/// A password for someone to type on a phone: 12 characters in groups of
/// four, from letters and digits that do not look alike. About 59 bits.
public enum GeneratedPassword {
  static let alphabet = Array("23456789abcdefghjkmnpqrstuvwxyz")

  public static func make() -> String {
    var generator = SystemRandomNumberGenerator()
    let characters = (0..<12).map { _ in alphabet[Int.random(in: 0..<alphabet.count, using: &generator)] }
    return stride(from: 0, to: 12, by: 4).map { String(characters[$0..<$0 + 4]) }.joined(separator: "-")
  }

  /// What a person may choose: 8 to 128 characters, no control characters.
  /// The rule the page and `jetlink password` go by.
  public static func problem(with password: String) -> String? {
    if password.count < 8 { return "The password needs at least 8 characters." }
    if password.count > 128 { return "The password can have at most 128 characters." }
    if password.unicodeScalars.contains(where: { $0.properties.generalCategory == .control }) {
      return "The password cannot contain control characters."
    }
    return nil
  }
}

/// The sign-in cookie's value: `v1.<expiry>.<nonce>.<mac>`, the MAC an
/// HMAC-SHA256 of the rest under the file's key. Nothing is kept on the
/// server, so a restart or a reboot signs nobody out, and a new key signs
/// everybody out.
enum SessionToken {
  static func issue(key: Data, expires: Date) -> String {
    let body = "v1.\(Int(expires.timeIntervalSince1970)).\(Base64URL.encode(Random.bytes(16)))"
    let mac = HMAC<SHA256>.authenticationCode(for: Data(body.utf8), using: SymmetricKey(data: key))
    return body + "." + Base64URL.encode(Data(mac))
  }

  /// When `token` expires, if `key` signed it and it has not.
  static func verify(_ token: String, key: Data, now: Date) -> Date? {
    let parts = token.split(separator: ".", omittingEmptySubsequences: false)
    guard parts.count == 4, parts[0] == "v1", let seconds = Int(parts[1]), let given = Base64URL.decode(String(parts[3])) else { return nil }
    let body = Data(parts[0...2].joined(separator: ".").utf8)
    guard HMAC<SHA256>.isValidAuthenticationCode(given, authenticating: body, using: SymmetricKey(data: key)) else { return nil }
    let expires = Date(timeIntervalSince1970: TimeInterval(seconds))
    return expires > now ? expires : nil
  }
}

enum Equal {
  /// Compares in a time that depends only on the lengths.
  static func constantTime(_ a: Data, _ b: Data) -> Bool {
    guard a.count == b.count else { return false }
    var difference: UInt8 = 0
    for (x, y) in zip(a, b) { difference |= x ^ y }
    return difference == 0
  }
}

enum Random {
  static func bytes(_ count: Int) -> Data {
    var generator = SystemRandomNumberGenerator()
    return Data((0..<count).map { _ in UInt8.random(in: 0...255, using: &generator) })
  }
}

enum Base64URL {
  static func encode(_ data: Data) -> String {
    data.base64EncodedString().replacingOccurrences(of: "+", with: "-").replacingOccurrences(of: "/", with: "_")
      .replacingOccurrences(of: "=", with: "")
  }

  static func decode(_ text: String) -> Data? {
    var base64 = text.replacingOccurrences(of: "-", with: "+").replacingOccurrences(of: "_", with: "/")
    base64 += String(repeating: "=", count: (4 - base64.count % 4) % 4)
    return Data(base64Encoded: base64)
  }
}

/// Who may try a password, and when: five tries an address, then a wait that
/// doubles with each failure up to five minutes; and at most two checks a
/// second from everyone together, so guessing cannot keep a core hashing.
final class LoginThrottle: @unchecked Sendable {
  static let freeTries = 5
  static let longestWait: TimeInterval = 300
  static let checksPerSecond = 2
  /// Addresses remembered; past it the stalest goes.
  static let addresses = 256

  private struct Record {
    var failures = 0
    var until: TimeInterval = 0
    var last: TimeInterval = 0
  }

  private let lock = NSLock()
  private var records: [String: Record] = [:]
  private var checks: [TimeInterval] = []

  /// Nil to go ahead with a check now (it is counted), else the seconds to wait.
  func admit(_ address: String, now: TimeInterval) -> TimeInterval? {
    lock.withLock {
      if let record = records[address], record.until > now { return record.until - now }
      checks.removeAll { $0 <= now - 1 }
      if checks.count >= LoginThrottle.checksPerSecond { return max(0.1, (checks.first ?? now) + 1 - now) }
      checks.append(now)
      return nil
    }
  }

  /// After a check: a success forgets the address, a failure counts against it.
  /// Returns the wait the failure starts, if any.
  @discardableResult
  func record(_ address: String, success: Bool, now: TimeInterval) -> TimeInterval? {
    lock.withLock {
      if success {
        records[address] = nil
        return nil
      }
      var record = records[address] ?? Record()
      record.failures += 1
      record.last = now
      var wait: TimeInterval?
      if record.failures >= LoginThrottle.freeTries {
        let doubled = pow(2, Double(record.failures - LoginThrottle.freeTries))
        wait = min(LoginThrottle.longestWait, doubled)
        record.until = now + wait!
      }
      records[address] = record
      if records.count > LoginThrottle.addresses, let stalest = records.min(by: { $0.value.last < $1.value.last })?.key {
        records[stalest] = nil
      }
      return wait
    }
  }
}

/// The page's sign-in: the password file, read again whenever it changes
/// (`jetlink password` writes it under a running server), the cookies it
/// signs and the throttle in front of it. A request reads `file` once and
/// hands it to the rest.
public final class WebAuth: @unchecked Sendable {
  /// Chrome keeps a cookie at most 400 days; the page renews it on use.
  static let lifetime: TimeInterval = 400 * 86400
  /// A cookie is renewed once it is a day old.
  static let renewAfter: TimeInterval = 86400
  static let cookieName = "jetlink_session"
  static let clearCookie = "\(cookieName)=; Max-Age=0; Path=/; HttpOnly; SameSite=Strict"

  public let url: URL
  let throttle = LoginThrottle()
  private let clock: @Sendable () -> Date
  private let lock = NSLock()
  /// One password check at a time: the throttle bounds how often they
  /// start, this how many cores they take.
  private let checking = NSLock()
  private var cached: (file: AuthFile, size: Int64, modified: Double)?

  public init(url: URL, clock: @escaping @Sendable () -> Date = { Date() }) {
    self.url = url
    self.clock = clock
  }

  /// The file as it is now, or nil when there is none (or it is unreadable,
  /// which the page treats the same: no controls).
  var file: AuthFile? {
    lock.withLock {
      guard let status = Files.status(url) else {
        cached = nil
        return nil
      }
      if let cached, cached.size == status.size, cached.modified == status.modified { return cached.file }
      guard let file = try? AuthFile.read(url) else {
        cached = nil
        return nil
      }
      cached = (file, status.size, status.modified)
      return file
    }
  }

  public var isConfigured: Bool { file != nil }

  /// What a password check came to.
  enum Check: Equatable {
    case right
    case wrong(retryAfter: TimeInterval?)
    /// The throttle did not let it run.
    case wait(TimeInterval)
  }

  private func check(_ password: String, against file: AuthFile, address: String) -> Check {
    if let wait = throttle.admit(address, now: clock().timeIntervalSince1970) { return .wait(wait) }
    let ok = checking.withLock { file.verify(password) }
    let wait = throttle.record(address, success: ok, now: clock().timeIntervalSince1970)
    return ok ? .right : .wrong(retryAfter: wait)
  }

  enum Login: Equatable {
    case signedIn(cookie: String)
    case refused(Check)
  }

  /// Checks `password` for `address`, at most as often as the throttle lets.
  func login(_ password: String, file: AuthFile, address: String) -> Login {
    let result = check(password, against: file, address: address)
    return result == .right ? .signedIn(cookie: cookie(key: file.key)) : .refused(result)
  }

  /// A signed-in request: its cookie verified under the file's key now.
  struct Session: Equatable {
    /// A fresh cookie to send, once this one is a day old.
    let renewal: String?
  }

  func session(cookieHeader: String?, file: AuthFile) -> Session? {
    guard let token = WebAuth.cookieValue(cookieHeader) else { return nil }
    let now = clock()
    guard let expires = SessionToken.verify(token, key: file.key, now: now) else { return nil }
    let issued = expires.addingTimeInterval(-WebAuth.lifetime)
    return Session(renewal: now.timeIntervalSince(issued) >= WebAuth.renewAfter ? cookie(key: file.key) : nil)
  }

  enum Change: Equatable {
    case changed(cookie: String)
    case refused(Check)
    case rejected(String)
    case failed(String)
  }

  /// A new password, if `current` is right; everyone else is signed out.
  func changePassword(current: String, new: String, file: AuthFile, address: String) -> Change {
    if let problem = GeneratedPassword.problem(with: new) { return .rejected(problem) }
    let result = check(current, against: file, address: address)
    guard result == .right else { return .refused(result) }
    let made = AuthFile.make(password: new, iterations: file.iterations)
    do {
      try store(made)
    } catch {
      return .failed("Could not save the password: \(error.localizedDescription)")
    }
    return .changed(cookie: cookie(key: made.key))
  }

  /// A new key: every cookie stops working but the one returned.
  func signOutEverywhere(file: AuthFile) throws -> String {
    let made = file.rotated()
    try store(made)
    return cookie(key: made.key)
  }

  private func store(_ file: AuthFile) throws {
    try file.write(url)
    lock.withLock { cached = Files.status(url).map { (file, $0.size, $0.modified) } }
  }

  private func cookie(key: Data) -> String {
    let token = SessionToken.issue(key: key, expires: clock().addingTimeInterval(WebAuth.lifetime))
    return "\(WebAuth.cookieName)=\(token); Max-Age=\(Int(WebAuth.lifetime)); Path=/; HttpOnly; SameSite=Strict"
  }

  static func cookieValue(_ header: String?) -> String? {
    guard let header else { return nil }
    for pair in header.split(separator: ";") {
      let trimmed = pair.trimmingCharacters(in: .whitespaces)
      guard let equals = trimmed.firstIndex(of: "=") else { continue }
      if trimmed[..<equals] == cookieName {
        let value = String(trimmed[trimmed.index(after: equals)...])
        return value.isEmpty ? nil : value
      }
    }
    return nil
  }
}
