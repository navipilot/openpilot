import Foundation
import Synchronization

#if canImport(CryptoKit)
  import CryptoKit
#else
  import Crypto
#endif
#if canImport(FoundationNetworking)
  import FoundationNetworking
#endif
#if canImport(os)
  import os
#else
  import JetlinkLog
#endif

/// A large model's ONNX, named by its git-lfs oid.
public struct Pointer: Sendable, Equatable, Hashable {
  /// The ONNX's SHA-256, which is the model's identity.
  public let oid: String
  public let size: Int64

  public init(oid: String, size: Int64) {
    self.oid = oid
    self.size = size
  }
}

/// Getting a large model's ONNX by its git-lfs oid.
///
/// comma overwrites one file per model, so the commit is the only name a big
/// model's ONNX has. GitHub's raw host serves the LFS pointer for any commit
/// it holds, merged or not, and that pointer carries the oid and size the
/// comma will ask a jetlink server for. The bytes themselves are on comma's
/// LFS servers, which are asked in turn because which one has an object
/// varies with the model's age.
///
/// Since openpilot #38930 (2026-09-16) a commit can ship a precompiled
/// tinygrad pkl instead, with no ONNX in the tree at all; Cinque Terre V3 is
/// one. Its subject names the export ("Use f78ed37d for the precompiled eGPU
/// driving model") and the ONNX is in comma's HuggingFace model repo, in the
/// folder that id starts. That repo speaks the LFS batch protocol too, so it
/// is one more endpoint to ask.
///
/// Not every such commit names its export. ResAction's pull request (#39037,
/// 2026-10-05) adds the ONNX in one commit and replaces it with the pkl in the
/// next, subject "compiled", and a squash merge's subject only names the pull
/// request. The pointer is still in a diff: the one the commit's own patch
/// deletes, or the last one its pull request's patch carries.
///
/// This mirrors `jetlink/registry/lfs.py`: same URLs, same endpoint order,
/// same verify rules.
public enum LFS {
  public static let bigONNX = "big_driving_supercombo.onnx"
  public static let pointerURLTemplate = "https://raw.githubusercontent.com/commaai/openpilot/{ref}/openpilot/selfdrive/modeld/models/" + bigONNX
  /// A commit's subject without the API, whose anonymous limit a car behind CGNAT shares.
  public static let commitPatchURLTemplate = "https://github.com/commaai/openpilot/commit/{ref}.patch"
  /// Every commit of a pull request, one patch after another, for a squash merge's subject.
  public static let pullPatchURLTemplate = "https://github.com/commaai/openpilot/pull/{number}.patch"
  /// How much of a patch is read for the pointers in its diffs; a model commit's is 3 KB.
  public static let patchMax = 256 << 10
  public static let drivingModelsRepo = "commaai/openpilot_driving_models"
  public static let drivingModelsTreeURL = "https://huggingface.co/api/models/\(drivingModelsRepo)/tree/main"
  public static let endpoints = [
    "https://gitlab.com/commaai/openpilot-lfs.git/info/lfs",  // every object, older and PR-branch models included
    "https://huggingface.co/commaai/openpilot-lfs.git/info/lfs",  // where comma is moving them; the current ones
    "https://huggingface.co/\(drivingModelsRepo).git/info/lfs",  // the exports behind a precompiled pkl
  ]
  public static let mediaType = "application/vnd.git-lfs+json"
  public static let pointerTimeout: TimeInterval = 10
  public static let connectTimeout: TimeInterval = 30
  /// A pointer is 134 bytes. Anything larger is the ONNX itself, served by a
  /// host that resolved the LFS filter for us, and reading a gigabyte to find
  /// that out is not on.
  public static let pointerMax = 4096
  /// Room left on the volume beyond the model itself.
  public static let freeSlack: Int64 = 64 << 20

  public static func pointerURL(ref: String) -> String {
    pointerURLTemplate.replacingOccurrences(of: "{ref}", with: ref)
  }

  public static func commitPatchURL(ref: String) -> String {
    commitPatchURLTemplate.replacingOccurrences(of: "{ref}", with: ref)
  }

  public static func pullPatchURL(number: String) -> String {
    pullPatchURLTemplate.replacingOccurrences(of: "{number}", with: number)
  }

  static let log = Logger(subsystem: "io.zoompilot.jetlink", category: "registry")

  // MARK: pointers

  /// The oid and size in a git-lfs pointer's text, or nil if it is not one.
  public static func parsePointer(_ text: String) -> Pointer? {
    guard text.utf8.count <= pointerMax else { return nil }
    var oid: String?
    var size: Int64?
    for line in pythonLines(text) {
      let key: Substring
      let value: Substring
      if let space = line.firstIndex(of: " ") {
        key = line[..<space]
        value = line[line.index(after: space)...]
      } else {
        key = line[...]
        value = ""
      }
      if key == "oid" {
        let body = value.hasPrefix("sha256:") ? value.dropFirst("sha256:".count) : value
        oid = String(body).trimmingCharacters(in: .whitespacesAndNewlines)
      } else if key == "size" {
        guard let parsed = JSON.parsePythonInt(String(value)) else { return nil }
        size = parsed
      }
    }
    guard let oid, CacheLayout.isSHA256(oid), let size, size > 0 else { return nil }
    return Pointer(oid: oid, size: size)
  }

  /// The oid and size of the ONNX at a comma commit: in its tree, or for a
  /// commit that ships a precompiled pkl instead, the ONNX it was built from.
  /// Only a 404 falls back; an outage is an outage.
  static func fetchPointer(ref: String, http: HTTP, timeout: TimeInterval = pointerTimeout) async throws(RegistryError) -> Pointer {
    let body: Data
    do {
      body = try await http.get(pointerURL(ref: ref), timeout: timeout, limit: pointerMax)
    } catch  where error.kind == .notFound {
      return try await fetchExportPointer(ref: ref, http: http, timeout: timeout)
    }
    guard let pointer = parsePointer(String(decoding: body, as: UTF8.self)) else {
      throw .registry("\(ref.prefix(10)) did not serve an lfs pointer")
    }
    return pointer
  }

  /// Every commit's subject in a patch, in order: one for a commit's, one per
  /// commit for a pull request's.
  static func patchSubjects(_ text: String) -> [String] {
    let lines = pythonLines(text)
    var subjects: [String] = []
    for (i, line) in lines.enumerated() where line.hasPrefix("Subject:") {
      var subject = [line.dropFirst("Subject:".count).trimmingCharacters(in: .whitespacesAndNewlines)]
      // A long subject is folded onto indented lines; the headers end at a blank one.
      for continuation in lines[(i + 1)...] {
        guard let first = continuation.first, first.isWhitespace,
          !continuation.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
        else { break }
        subject.append(continuation.trimmingCharacters(in: .whitespacesAndNewlines))
      }
      subjects.append(stripPatchTag(subject.joined(separator: " ")))
    }
    return subjects
  }

  /// The big ONNX's pointer in a patch's diffs, the last one wins: the pointer a
  /// diff gives the file, or for a diff that deletes it, the pointer it had. A
  /// commit that swaps the ONNX for a pkl deletes the ONNX the pkl was built from.
  public static func diffPointer(_ text: String) -> Pointer? {
    var found: Pointer?
    var section: [Substring]?
    func close() {
      guard let lines = section else { return }
      let added = lines.filter { $0.hasPrefix("+") && !$0.hasPrefix("+++") }.map { $0.dropFirst() }
      let removed = lines.filter { $0.hasPrefix("-") && !$0.hasPrefix("---") }.map { $0.dropFirst() }
      found = parsePointer(added.joined(separator: "\n")) ?? parsePointer(removed.joined(separator: "\n")) ?? found
    }
    for line in pythonLines(text) {
      // Each diff, and each commit of a pull request's patch, ends the one before.
      if line.hasPrefix("diff --git ") || line.hasPrefix("From ") {
        close()
        section = isONNXDiff(line) ? [] : nil
      } else {
        section?.append(line)
      }
    }
    close()
    return found
  }

  /// `^diff --git a/\S*selfdrive/modeld/models/big_driving_supercombo\.onnx `:
  /// the big ONNX's diff header, wherever the tree keeps selfdrive.
  static func isONNXDiff(_ line: Substring) -> Bool {
    guard line.hasPrefix("diff --git a/") else { return false }
    let rest = line.dropFirst("diff --git a/".count)
    let path = rest.prefix { !$0.isWhitespace }
    return path.hasSuffix("selfdrive/modeld/models/" + bigONNX) && rest.dropFirst(path.count).first == " "
  }

  /// `re.search(r'\(#([0-9]+)\)$', subject)`: the pull request a squash merge's subject ends with.
  static func pullNumber(in subject: String) -> String? {
    guard subject.hasSuffix(")"), let open = subject.range(of: "(#", options: .backwards) else { return nil }
    let digits = subject[open.upperBound..<subject.index(before: subject.endIndex)]
    return !digits.isEmpty && digits.allSatisfy({ $0.isASCII && $0.isNumber }) ? String(digits) : nil
  }

  /// A comma commit's subject line, from the head of its patch.
  static func commitSubject(ref: String, http: HTTP, timeout: TimeInterval = pointerTimeout) async throws(RegistryError) -> String {
    let url = commitPatchURL(ref: ref)
    let head = try await http.get(url, timeout: timeout, limit: pointerMax)
    guard let subject = patchSubjects(String(decoding: head, as: UTF8.self)).first else {
      throw .registry("\(url) has no subject line")
    }
    return subject
  }

  /// `re.sub(r'^\[PATCH[^\]]*\]\s*', '', subject)`.
  static func stripPatchTag(_ subject: String) -> String {
    guard subject.hasPrefix("[PATCH"), let close = subject.firstIndex(of: "]") else { return subject }
    return String(subject[subject.index(after: close)...].drop { $0.isWhitespace })
  }

  /// The eight-character hex ids a subject names, first mention first:
  /// `\b[0-9a-f]{8}\b`, which is a whole word of exactly eight lowercase hex digits.
  static func exportIDs(in subject: String) -> [String] {
    var ids: [String] = []
    var word = ""
    func close() {
      if CacheLayout.isLowercaseHex(word, count: 8), !ids.contains(word) { ids.append(word) }
      word = ""
    }
    for character in subject {
      if character.isLetter || character.isNumber || character == "_" {
        word.append(character)
      } else {
        close()
      }
    }
    close()
    return ids
  }

  /// A listing of comma's model repo, or of one folder of it, recursively.
  static func tree(_ path: String, http: HTTP, timeout: TimeInterval) async throws(RegistryError) -> [JSONObject] {
    var url = drivingModelsTreeURL
    if !path.isEmpty {
      url += "/\(pythonQuote(path))?recursive=true"
    }
    let listing = try await http.getJSON(url, timeout: timeout, shape: .array)
    return (listing.array ?? []).compactMap { entry in
      guard let object = entry.object, object["path"]?.string != nil else { return nil }
      return object
    }
  }

  /// `urllib.parse.quote(path)`: everything but letters, digits, `_.-~` and `/`.
  static func pythonQuote(_ path: String) -> String {
    var allowed = CharacterSet()
    allowed.insert(charactersIn: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.-~/")
    return path.addingPercentEncoding(withAllowedCharacters: allowed) ?? path
  }

  /// The big ONNX a precompiled-pkl commit was built from. In turn: the export
  /// its subject names, in comma's model repo; the pointer its own diff deletes;
  /// and for a squash merge, the last pointer its pull request carried, or the
  /// export one of that pull request's commits names.
  static func fetchExportPointer(ref: String, http: HTTP, timeout: TimeInterval = pointerTimeout) async throws(RegistryError) -> Pointer {
    let url = commitPatchURL(ref: ref)
    let patch = String(decoding: try await http.get(url, timeout: timeout, limit: patchMax), as: UTF8.self)
    guard let subject = patchSubjects(patch).first else {
      throw .registry("\(url) has no subject line")
    }
    let short = ref.prefix(10)
    let ids = exportIDs(in: subject)
    if !ids.isEmpty, let pointer = try await exportPointer(ref: ref, ids: ids, subject: subject, http: http, timeout: timeout) {
      return pointer
    }
    var tried = [ids.isEmpty ? "its subject names no export" : "no folder in \(drivingModelsRepo) for \(ids.joined(separator: ", "))"]
    if let pointer = diffPointer(patch) {
      log.info("\(short, privacy: .public) deletes \(bigONNX, privacy: .public) \(pointer.oid.prefix(16), privacy: .public)")
      return pointer
    }
    tried.append("its diff has no \(bigONNX)")
    if let number = pullNumber(in: subject) {
      let pullPatch = String(decoding: try await http.get(pullPatchURL(number: number), timeout: timeout, limit: patchMax), as: UTF8.self)
      if let pointer = diffPointer(pullPatch) {
        let oid = pointer.oid.prefix(16)
        log.info("\(short, privacy: .public) merged #\(number, privacy: .public), whose last \(bigONNX, privacy: .public) is \(oid, privacy: .public)")
        return pointer
      }
      let pullSubjects = patchSubjects(pullPatch).joined(separator: " ")
      let pullIDs = exportIDs(in: pullSubjects).filter { !ids.contains($0) }
      if !pullIDs.isEmpty, let pointer = try await exportPointer(ref: ref, ids: pullIDs, subject: pullSubjects, http: http, timeout: timeout) {
        return pointer
      }
      tried.append("nor does pull request #\(number)")
    }
    throw .registry("\(short) has no \(bigONNX): \(tried.joined(separator: "; ")) (\(JSON.pythonRepr(subject)))")
  }

  /// The ONNX in the first export folder one of `ids` starts, or nil when none does.
  static func exportPointer(ref: String, ids: [String], subject: String, http: HTTP, timeout: TimeInterval) async throws(RegistryError) -> Pointer? {
    let short = ref.prefix(10)
    let folders = try await tree("", http: http, timeout: timeout)
      .filter { $0["type"]?.string == "directory" }
      .compactMap { $0["path"]?.string }
    for export in ids {
      let matches = folders.filter { $0.hasPrefix(export) }
      if matches.isEmpty { continue }
      if matches.count > 1 {
        throw .registry("\(short): \(export) starts \(matches.count) folders in \(drivingModelsRepo)")
      }
      var files = try await tree(matches[0], http: http, timeout: timeout).filter { entry in
        guard entry["type"]?.string == "file", let path = entry["path"]?.string else { return false }
        return lastComponent(path) == bigONNX
      }
      if files.count > 1 {
        // A subject that names the checkpoint, as '1a421175-.../12864' does, picks one.
        let named = files.filter { subject.contains(parentPath($0["path"]?.string ?? "")) }
        if !named.isEmpty { files = named }
      }
      guard files.count == 1 else {
        throw .registry("\(short): \(files.count) copies of \(bigONNX) under \(matches[0])")
      }
      let file = files[0]
      let path = file["path"]?.string ?? ""
      let lfs = file["lfs"]?.object
      guard let oid = lfs?["oid"]?.string, CacheLayout.isSHA256(oid), let size = lfs?["size"]?.pythonIntValue, size > 0 else {
        throw .registry("\(path) is not an lfs object")
      }
      log.info("\(short, privacy: .public) names export \(export, privacy: .public): \(path, privacy: .public)")
      return Pointer(oid: oid, size: size)
    }
    return nil
  }

  /// `path.rsplit('/', 1)[-1]`.
  static func lastComponent(_ path: String) -> String {
    guard let slash = path.lastIndex(of: "/") else { return path }
    return String(path[path.index(after: slash)...])
  }

  /// `path.rsplit('/', 1)[0]`, which is the whole path when it has no slash.
  static func parentPath(_ path: String) -> String {
    guard let slash = path.lastIndex(of: "/") else { return path }
    return String(path[..<slash])
  }

  /// `str.splitlines()`: every line break Python knows, the break dropped.
  static func pythonLines(_ text: String) -> [Substring] {
    let breaks: Set<Character> = ["\n", "\r", "\r\n", "\u{0B}", "\u{0C}", "\u{1C}", "\u{1D}", "\u{1E}", "\u{85}", "\u{2028}", "\u{2029}"]
    var lines = text.split(omittingEmptySubsequences: false) { breaks.contains($0) }
    // A trailing break ends the last line rather than starting an empty one.
    if let last = lines.last, last.isEmpty, let final = text.last, breaks.contains(final) {
      lines.removeLast()
    }
    if text.isEmpty { return [] }
    return lines
  }

  // MARK: the batch API

  /// Asks one LFS server for a download href, or nil if it does not have it.
  ///
  /// A server that is down is not different from a server that lacks the
  /// object: either way the caller moves to the next one, so nothing throws.
  static func resolve(endpoint: String, pointer: Pointer, session: URLSession, timeout: TimeInterval = connectTimeout) async -> String? {
    guard let url = URL(string: "\(endpoint)/objects/batch") else { return nil }
    let body: JSON = [
      "operation": "download",
      "transfers": ["basic"],
      "objects": [["oid": .string(pointer.oid), "size": .int(pointer.size)]],
    ]
    var request = URLRequest(url: url)
    request.httpMethod = "POST"
    request.httpBody = body.data()
    request.setValue(mediaType, forHTTPHeaderField: "Accept")
    request.setValue(mediaType, forHTTPHeaderField: "Content-Type")
    request.timeoutInterval = timeout

    let payload: JSON
    do {
      let (data, response) = try await session.data(for: request)
      try HTTP.check(response, url: url.absoluteString)
      payload = try JSON.parse(data)
    } catch {
      log.warning("lfs batch failed at \(endpoint, privacy: .public): \(String(describing: error), privacy: .public)")
      return nil
    }

    for object in payload["objects"]?.array ?? [] {
      guard let object = object.object, object["oid"]?.string == pointer.oid else { continue }
      if object.contains("error") {
        let message = object["error"]?["message"]?.pythonString ?? "None"
        log.warning("\(endpoint, privacy: .public) has no \(pointer.oid.prefix(16), privacy: .public) (\(message, privacy: .public))")
        return nil
      }
      if let href = object["actions"]?["download"]?["href"], href.truthy {
        return href.pythonString
      }
    }
    return nil
  }
}
