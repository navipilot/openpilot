import Foundation
import JetlinkKit
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

/// A model imported from disk rather than fetched from the catalog.
public struct LocalModel: Sendable, Equatable {
  public let sha256: String
  public let bytes: Int64
  public let name: String
  /// Unix seconds, as the Python registry records it.
  public let addedAt: Double

  public init(sha256: String, bytes: Int64, name: String, addedAt: Double) {
    self.sha256 = sha256
    self.bytes = bytes
    self.name = name
    self.addedAt = addedAt
  }
}

/// Which large models exist, which one is which file, and how to get the
/// bytes. A port of `jetlink/registry`: the same state files, URLs, rules and
/// payloads, so the iPhone's server answers the control protocol as the
/// Python one does.
///
/// State lives in `<cache>/registry/`:
///
///     catalog.json       {"fetched_at", "url", "raw"} as fetched, refreshed hourly
///     pointers.json      {ref: {"oid", "size"}}; a commit's tree never changes, so this is kept forever
///     local-models.json  [{"sha256", "bytes", "name", "added_at"}] for models imported from disk
///
/// Each is written atomically, and every read-modify-write holds a lock,
/// because the control layer, a download and a catalog refresh can all be
/// writing while a comma is asking for a model. Calls are safe from any number
/// of tasks at once.
public final class Registry: Sendable {
  /// The suffix each backend gives its artifact, so a sidecar can be paired
  /// with the thing it describes without the backend (and so a Jetson's
  /// sidecars read on a phone).
  public static let artifactSuffixes = ["trt": ".plan", "tinygrad": ".pkl", "ort": ".ortcache", "litert": ".litertcache", "fake": ".fake"]

  static let hashChunk = 1 << 20
  static let copyChunk = 4 << 20

  public let layout: CacheLayout
  let session: URLSession
  let http: HTTP
  private let lock = Mutex(())
  private static let log = Logger(subsystem: "io.zoompilot.jetlink", category: "registry")

  /// Opens the registry of a cache, making its registry and models directories.
  public init(layout: CacheLayout, session: URLSession = .shared) {
    self.layout = layout
    self.session = session
    self.http = HTTP(session: session)
    layout.makeRegistryDirectories()
  }

  // MARK: - the catalog

  /// The `catalog` event from disk alone, never the network: what a client
  /// gets the moment it connects. Nil when nothing usable is cached; send
  /// `Registry.emptyCatalog` then and refresh off the connect path.
  public func cachedCatalog() -> CatalogEvent? {
    guard let cached = Files.readJSON(layout.catalogURL)?.object, let raw = cached["raw"], raw != .null else { return nil }
    return payload(raw: raw, fetchedAt: cached["fetched_at"]?.pythonNumber, error: nil)
  }

  /// The payload for a cache with no catalog in it yet.
  public static func emptyCatalog(error: String? = nil) -> CatalogEvent {
    CatalogEvent(fetchedAt: nil, url: Catalog.url, defaultRef: Catalog.defaultBigModelRef, error: error, models: [])
  }

  /// The `catalog` event, from the cache when it is fresh enough.
  ///
  /// Pointers are not resolved here: that is one request per model, thirteen
  /// of them today, and `resolveMissing` does it. A failed refresh is not
  /// fatal; the previous list comes back with `error` set.
  public func catalog(refresh: Bool = false, maxAge: TimeInterval = Catalog.maxAge) async -> CatalogEvent {
    let cached = Files.readJSON(layout.catalogURL)?.object
    var raw = cached?["raw"].flatMap { $0 == .null ? nil : $0 }
    var fetchedAt = cached?["fetched_at"]?.pythonNumber
    var error: String?

    if refresh || raw == nil || fetchedAt == nil || Date().timeIntervalSince1970 - (fetchedAt ?? 0) > maxAge {
      do {
        // The pinned catalog and whatever sunnypilot has published since: the
        // server runs any of their commits. Then zoompilot's extra models.
        let pinned = try await fetchCatalog(Catalog.url)
        let newer = try await newerCatalogs(after: Catalog.url)
        let extra = try await extraCatalog()
        let fresh = JSON.object(Catalog.merge([pinned] + newer + extra))
        let now = Date().timeIntervalSince1970
        raw = fresh
        fetchedAt = now
        do {
          try Files.writeJSON(["fetched_at": .double(now), "url": .string(Catalog.url), "raw": fresh], to: layout.catalogURL)
        } catch {
          Registry.log.warning("could not keep the catalog: \(String(describing: error), privacy: .public)")
        }
      } catch let failure {
        error = failure.message
      }
    }
    return payload(raw: raw, fetchedAt: fetchedAt, error: error)
  }

  private func payload(raw: JSON?, fetchedAt: Double?, error: String?) -> CatalogEvent {
    let pointers = Dictionary(pointers().map { ($0.ref, $0.pointer) }, uniquingKeysWith: { first, _ in first })
    let source: JSON = raw.map { $0.truthy ? $0 : [:] } ?? [:]
    let models = Catalog.parse(source).map { entry in
      let pointer = pointers[entry.ref]
      return CatalogModel(
        name: entry.name, shortName: entry.shortName, ref: entry.ref, buildTime: entry.buildTime, index: entry.index,
        sha256: pointer?.oid, bytes: pointer?.size)
    }
    return CatalogEvent(fetchedAt: fetchedAt, url: Catalog.url, defaultRef: Catalog.defaultBigModelRef, error: error, models: models)
  }

  /// The catalog JSON. Every failure, transport or content, is a network error.
  func fetchCatalog(_ url: String) async throws(RegistryError) -> JSONObject {
    try await http.getJSON(url, timeout: Catalog.timeout, shape: .object).object ?? JSONObject()
  }

  /// zoompilot's extra list, or nothing when it is not there. Any other
  /// failure throws, as `newerCatalogs` does.
  func extraCatalog() async throws(RegistryError) -> [JSONObject] {
    do {
      return [try await fetchCatalog(Catalog.extraURL)]
    } catch  where error.kind == .notFound {
      return []
    }
  }

  /// Every catalog sunnypilot has published after the one at `url`, oldest
  /// first. Stops at the first version that is not there. Any other failure
  /// throws: a short list would drop a model found only past it from the kept
  /// catalog, so the caller keeps its last complete one instead.
  func newerCatalogs(after url: String, limit: Int = Catalog.probeLimit) async throws(RegistryError) -> [JSONObject] {
    guard let version = Catalog.version(of: url), limit > 0 else { return [] }
    var found: [JSONObject] = []
    for next in (version + 1)...(version + limit) {
      do {
        found.append(try await fetchCatalog(Catalog.url(version: next)))
      } catch  where error.kind == .notFound {
        break
      }
    }
    return found
  }

  // MARK: - pointers

  /// The pointer behind a catalog ref, fetched the first time and kept for good.
  public func resolve(ref: String) async throws(RegistryError) -> Pointer {
    guard CacheLayout.isRef(ref) else {
      throw .registry("\(JSON.pythonRepr(ref)) is not a 40 character commit")
    }
    if let known = pointers().first(where: { $0.ref == ref }) {
      return known.pointer
    }
    let pointer = try await LFS.fetchPointer(ref: ref, http: http)
    savePointers([(ref, pointer)])
    Registry.log.info("\(ref.prefix(10), privacy: .public) is \(pointer.oid.prefix(16), privacy: .public), \(pointer.size >> 20) MB")
    return pointer
  }

  /// Resolves several refs at once, `workers` at a time, and keeps what
  /// resolved. A failure comes back per ref: one bad ref must not lose the
  /// other twelve.
  @discardableResult
  public func resolveMissing(_ refs: [String], workers: Int = 8) async -> [String: Result<Pointer, RegistryError>] {
    let known = Dictionary(pointers().map { ($0.ref, $0.pointer) }, uniquingKeysWith: { first, _ in first })
    var out: [String: Result<Pointer, RegistryError>] = [:]
    var todo: [String] = []
    var seen = Set<String>()
    for ref in refs where seen.insert(ref).inserted {
      if !CacheLayout.isRef(ref) {
        out[ref] = .failure(.registry("\(JSON.pythonRepr(ref)) is not a 40 character commit"))
      } else if let pointer = known[ref] {
        out[ref] = .success(pointer)
      } else {
        todo.append(ref)
      }
    }
    guard !todo.isEmpty else { return out }

    let http = self.http
    let results = await withTaskGroup(of: (Int, Result<Pointer, RegistryError>).self) { group in
      var results = [Result<Pointer, RegistryError>?](repeating: nil, count: todo.count)
      var next = 0
      let width = max(1, min(workers, todo.count))
      while next < width {
        let (index, ref) = (next, todo[next])
        group.addTask { (index, await Registry.fetchPointerResult(ref, http: http)) }
        next += 1
      }
      for await (index, result) in group {
        results[index] = result
        if next < todo.count {
          let (index, ref) = (next, todo[next])
          group.addTask { (index, await Registry.fetchPointerResult(ref, http: http)) }
          next += 1
        }
      }
      return results
    }

    var fresh: [(String, Pointer)] = []
    for (ref, result) in zip(todo, results) {
      guard let result else { continue }
      out[ref] = result
      if case .success(let pointer) = result { fresh.append((ref, pointer)) }
    }
    if !fresh.isEmpty { savePointers(fresh) }
    return out
  }

  private static func fetchPointerResult(_ ref: String, http: HTTP) async -> Result<Pointer, RegistryError> {
    do {
      return .success(try await LFS.fetchPointer(ref: ref, http: http))
    } catch {
      return .failure(error)
    }
  }

  /// A human name and the catalog ref for a model identity; either may be nil.
  public func name(for sha256: String) -> (name: String?, ref: String?) {
    nameLookup()(sha256)
  }

  /// `name(for:)` with the pointers, the cached catalog and the local models
  /// read once, for a loop over many models.
  private func nameLookup() -> (String) -> (name: String?, ref: String?) {
    let pointers = pointers()
    let raw = Files.readJSON(layout.catalogURL)?["raw"] ?? [:]
    let entries = Catalog.parse(raw.truthy ? raw : [:])
    let locals = localModels()
    return { sha256 in
      if let match = pointers.first(where: { $0.pointer.oid == sha256 }) {
        return (entries.first { $0.ref == match.ref }?.name, match.ref)
      }
      if let local = locals.first(where: { $0.sha256 == sha256 }) {
        return (local.name, nil)
      }
      return (nil, nil)
    }
  }

  /// The catalog ref whose model has this identity, from the pointers or the
  /// cached list. Disk only.
  public func ref(for sha256: String) -> String? {
    if let ref = name(for: sha256).ref { return ref }
    return cachedCatalog()?.models.first { $0.sha256 == sha256 }?.ref
  }

  /// pointers.json, in file order, without the entries the Python registry
  /// skips (no oid or no size).
  private func pointerTable() -> JSONObject {
    guard let known = Files.readJSON(layout.pointersURL)?.object else { return JSONObject() }
    var out = JSONObject()
    for (ref, entry) in known {
      guard let object = entry.object, object["oid"]?.truthy == true, object["size"]?.truthy == true else { continue }
      out[ref] = entry
    }
    return out
  }

  private func pointers() -> [(ref: String, pointer: Pointer)] {
    pointerTable().compactMap { ref, entry in
      guard let oid = entry["oid"]?.pythonString, let size = entry["size"]?.pythonInt else { return nil }
      return (ref, Pointer(oid: oid, size: size))
    }
  }

  private func savePointers(_ fresh: [(String, Pointer)]) {
    lock.withLock { _ in
      var known = pointerTable()
      for (ref, pointer) in fresh {
        known[ref] = ["oid": .string(pointer.oid), "size": .int(pointer.size)]
      }
      do {
        try Files.writeJSON(.object(known), to: layout.pointersURL)
      } catch {
        Registry.log.warning("could not keep the pointers: \(String(describing: error), privacy: .public)")
      }
    }
  }

  // MARK: - models on disk

  /// Where the server looks for a model's ONNX.
  public func modelPath(sha256: String) throws(RegistryError) -> URL {
    try layout.modelPath(sha256: sha256)
  }

  /// Materialises one model's ONNX, from whichever LFS server has it.
  ///
  /// A ref is resolved first (and kept); a sha256 must already be in a
  /// pointer, because an LFS object is an oid plus a size. The servers are
  /// asked in order and the first with the object serves it. `progress` is
  /// called whenever the whole percent changes, and once with 1.0 at the end;
  /// `shouldStop` is asked between chunks.
  public func fetch(
    _ refOrSHA256: String,
    progress: @escaping @Sendable (Double) -> Void = { _ in },
    shouldStop: @escaping @Sendable () -> Bool = { false }
  ) async throws(RegistryError) -> URL {
    let pointer = try await pointer(for: refOrSHA256)
    let dest = try layout.modelPath(sha256: pointer.oid)
    if let status = Files.status(dest), status.isFile, status.size == pointer.size {
      return dest
    }
    for endpoint in LFS.endpoints {
      guard let href = await LFS.resolve(endpoint: endpoint, pointer: pointer, session: session) else { continue }
      Registry.log.info("fetching \(pointer.oid.prefix(16), privacy: .public) (\(pointer.size >> 20) MB) from \(endpoint, privacy: .public)")
      return try await LFS.download(href: href, pointer: pointer, dest: dest, session: session, progress: progress, shouldStop: shouldStop)
    }
    throw .network("no LFS server has \(pointer.oid.prefix(16))")
  }

  private func pointer(for refOrSHA256: String) async throws(RegistryError) -> Pointer {
    if CacheLayout.isRef(refOrSHA256) {
      return try await resolve(ref: refOrSHA256)
    }
    if CacheLayout.isSHA256(refOrSHA256) {
      if let known = pointers().first(where: { $0.pointer.oid == refOrSHA256 }) {
        return Pointer(oid: refOrSHA256, size: known.pointer.size)
      }
      throw .registry("size for \(refOrSHA256.prefix(16)) unknown; fetch by catalog ref")
    }
    throw .registry("\(JSON.pythonRepr(refOrSHA256)) is neither a 40 character ref nor a 64 character sha256")
  }

  /// Takes a model from disk into the cache under its own identity.
  ///
  /// Progress covers the whole operation: the hashing pass is the first half
  /// and the copy the second, because both read the file end to end. A model
  /// already in the cache is not copied again, and a second import of it only
  /// renames it. The reading runs on a dispatch queue, off the task pool.
  public func importModel(
    at url: URL, name: String? = nil,
    progress: @escaping @Sendable (Double) -> Void = { _ in },
    shouldStop: @escaping @Sendable () -> Bool = { false }
  ) async throws(RegistryError) -> LocalModel {
    let fileName = url.lastPathComponent
    guard PythonPath.suffix(fileName).lowercased() == ".onnx" else {
      throw .registry("\(fileName) is not an .onnx file")
    }
    guard let status = Files.status(url), status.isFile else {
      throw .registry("\(url.path(percentEncoded: false)) does not exist")
    }
    let total = status.size
    let layout = self.layout
    let (sha256, nbytes) = try await Files.offload { () -> Result<(String, Int64), RegistryError> in
      do throws(RegistryError) {
        let (sha256, nbytes) = try Registry.hashFile(url, total: total, progress: progress, shouldStop: shouldStop)
        let dest = try layout.modelPath(sha256: sha256)
        if !(Files.status(dest).map { $0.isFile && $0.size == nbytes } ?? false) {
          try Registry.copyFile(url, to: dest, total: total, progress: progress, shouldStop: shouldStop)
        }
        return .success((sha256, nbytes))
      } catch {
        return .failure(error)
      }
    }.get()
    progress(1.0)

    let chosen = name.flatMap { $0.isEmpty ? nil : $0 } ?? PythonPath.stem(fileName)
    let local = LocalModel(sha256: sha256, bytes: nbytes, name: chosen, addedAt: Date().timeIntervalSince1970)
    try lock.withLock { _ throws(RegistryError) in
      var records = localRecords().filter { $0["sha256"] != .string(sha256) }
      records.append(["sha256": .string(local.sha256), "bytes": .int(local.bytes), "name": .string(local.name), "added_at": .double(local.addedAt)])
      do {
        try Files.writeJSON(.array(records.map(JSON.object)), to: layout.localModelsURL)
      } catch {
        throw .registry("could not record \(fileName): \(error.localizedDescription)")
      }
    }
    return local
  }

  public func localModels() -> [LocalModel] {
    localRecords().compactMap { record in
      guard let sha256 = record["sha256"]?.pythonString, let bytes = record["bytes"]?.pythonInt,
        let addedAt = Registry.pythonFloat(record["added_at"].flatMap { $0.truthy ? $0 : nil } ?? .double(0))
      else { return nil }
      let name = record["name"].flatMap { $0.truthy ? $0.pythonString : nil } ?? ""
      return LocalModel(sha256: sha256, bytes: bytes, name: name, addedAt: addedAt)
    }
  }

  private func localRecords() -> [JSONObject] {
    (Files.readJSON(layout.localModelsURL)?.array ?? []).compactMap(\.object)
  }

  /// `float(x)`, which also takes a numeric string.
  private static func pythonFloat(_ value: JSON) -> Double? {
    if let number = value.pythonNumber { return number }
    if let text = value.string { return Double(text.trimmingCharacters(in: .whitespacesAndNewlines)) }
    return nil
  }

  /// The file's SHA-256 and size, read a megabyte at a time: an import's
  /// first pass, and the server's check of an upload.
  package static func hashFile(
    _ url: URL, total: Int64 = 0, progress: @Sendable (Double) -> Void = { _ in }, shouldStop: @Sendable () -> Bool = { false }
  ) throws(RegistryError) -> (String, Int64) {
    guard let handle = try? FileHandle(forReadingFrom: url) else {
      throw .registry("could not read \(url.path(percentEncoded: false))")
    }
    defer { try? handle.close() }
    var hasher = SHA256()
    var read: Int64 = 0
    while true {
      let chunk: Data
      do {
        chunk = try handle.read(upToCount: hashChunk) ?? Data()
      } catch {
        throw .registry("could not read \(url.path(percentEncoded: false)): \(error.localizedDescription)")
      }
      if chunk.isEmpty { break }
      if shouldStop() { throw .cancelled("import cancelled") }
      hasher.update(data: chunk)
      read += Int64(chunk.count)
      if total > 0 { progress(0.5 * min(1.0, Double(read) / Double(total))) }
    }
    return (hasher.finalize().map { String(format: "%02x", $0) }.joined(), read)
  }

  static func copyFile(
    _ source: URL, to dest: URL, total: Int64, progress: @Sendable (Double) -> Void, shouldStop: @Sendable () -> Bool
  ) throws(RegistryError) {
    let part = dest.deletingLastPathComponent().appending(path: dest.lastPathComponent + ".part")
    let fail = { (detail: String) -> RegistryError in
      .registry("could not copy \(source.path(percentEncoded: false)) into the cache: \(detail)")
    }
    do {
      guard FileManager.default.createFile(atPath: part.path(percentEncoded: false), contents: nil) else {
        throw fail("cannot create \(part.lastPathComponent)")
      }
      let input = try FileHandle(forReadingFrom: source)
      defer { try? input.close() }
      let output = try FileHandle(forWritingTo: part)
      defer { try? output.close() }
      var written: Int64 = 0
      while let chunk = try input.read(upToCount: copyChunk), !chunk.isEmpty {
        if shouldStop() { throw RegistryError.cancelled("import cancelled") }
        try output.write(contentsOf: chunk)
        written += Int64(chunk.count)
        if total > 0 { progress(0.5 + 0.5 * min(1.0, Double(written) / Double(total))) }
      }
    } catch {
      try? Files.removeFile(part)
      throw error as? RegistryError ?? fail(error.localizedDescription)
    }
    do {
      try Files.replace(part, with: dest)
    } catch {
      try? Files.removeFile(part)
      throw fail(error.localizedDescription)
    }
  }

  // MARK: - inventory and removal

  /// The `inventory` event: what is on disk and what it belongs to.
  ///
  /// A model file is named by the first 16 characters of its identity, so a
  /// file whose full identity is in no pointer, no local record and no sidecar
  /// is listed with a 16 character `sha256`: the prefix is all anyone knows.
  /// Artifacts of every backend are listed; `current` marks the one whose key
  /// is `<sha16>.<artifactTag>` with `artifactSuffix`, the one this server
  /// would load. With no tag nothing is current.
  public func inventory(artifactTag: String?, artifactSuffix: String, loaded: String?) -> InventoryEvent {
    let known = knownSHAs()
    let name = nameLookup()
    var models: [InventoryModel] = []
    var modelsBytes: Int64 = 0
    for fileName in Files.names(in: layout.models) where fileName.hasSuffix(".onnx") {
      let stem = PythonPath.stem(fileName)
      guard CacheLayout.isLowercaseHex(stem, count: 16) else { continue }
      let url = layout.models.appending(path: fileName)
      guard let status = Files.status(url) else { continue }
      let sha256 = known[stem] ?? stem
      let (modelName, ref) = sha256.utf8.count == 64 ? name(sha256) : (nil, nil)
      models.append(InventoryModel(sha256: sha256, bytes: status.size, path: url.path(percentEncoded: false), name: modelName, ref: ref))
      modelsBytes += status.size
    }

    var artifacts: [InventoryArtifact] = []
    var enginesBytes: Int64 = 0
    let engineNames = Files.names(in: layout.engines)
    for fileName in engineNames where fileName.hasSuffix(".json") {
      guard let entry = artifactEntry(sidecar: fileName, engineNames: engineNames, tag: artifactTag, suffix: artifactSuffix) else {
        continue
      }
      artifacts.append(entry)
      enginesBytes += entry.bytes
    }

    return InventoryEvent(
      loaded: loaded,
      lastLoaded: layout.lastLoaded()?.sha256,
      models: models,
      artifacts: artifacts,
      disk: InventoryDisk(modelsBytes: modelsBytes, enginesBytes: enginesBytes, freeBytes: Files.freeBytes(at: layout.root) ?? 0))
  }

  private func artifactEntry(sidecar: String, engineNames: [String], tag: String?, suffix: String) -> InventoryArtifact? {
    let metaPath = layout.engines.appending(path: sidecar)
    guard let meta = Files.readJSON(metaPath)?.object, let sha256 = meta["spec"]?["sha256"]?.string,
      CacheLayout.isSHA256(sha256)
    else { return nil }
    let key = PythonPath.stem(sidecar)
    let backend = meta["backend"].flatMap { $0.truthy ? $0.pythonString : nil } ?? ""
    guard let artifact = artifactPath(key: key, backend: backend, sidecar: sidecar, engineNames: engineNames) else { return nil }

    // Python's `or` chain: the first of these that is truthy, else trt_version
    // as it stands, which is how a Jetson's sidecar still reports its runtime.
    // LiteRT's is Swift's alone.
    let runtime = [meta["onnxruntime"], meta["tinygrad"], meta["litert"]].lazy.compactMap { $0 }.first { $0.truthy } ?? meta["trt_version"]
    let spec = meta["spec"].flatMap { $0.truthy ? $0 : nil }
    let current = tag.map { key == "\(sha256.prefix(16)).\($0)" && artifact.lastPathComponent.hasSuffix(suffix) } ?? false
    return InventoryArtifact(
      sha256: sha256,
      key: key,
      path: artifact.path(percentEncoded: false),
      bytes: Files.size(of: artifact),
      backend: backend,
      runtimeVersion: Registry.optionalString(runtime),
      device: meta["device"].flatMap { $0.truthy ? $0.pythonString : nil } ?? "",
      builtAt: Registry.optionalString(meta["built_at"]),
      buildSeconds: meta["build_seconds"]?.pythonNumber,
      checkpoint: Registry.optionalString(spec?["checkpoint"]),
      current: current)
  }

  /// The artifact a sidecar describes: the backend's own suffix first, then
  /// anything else under the same key.
  private func artifactPath(key: String, backend: String, sidecar: String, engineNames: [String]) -> URL? {
    if let suffix = Registry.artifactSuffixes[backend] {
      let candidate = layout.engines.appending(path: key + suffix)
      if Files.exists(candidate) { return candidate }
    }
    let name = engineNames.first { $0 != sidecar && $0.hasPrefix(key + ".") }
    return name.map { layout.engines.appending(path: $0) }
  }

  /// A JSON value where the event has an optional string: null and absent are nil.
  private static func optionalString(_ value: JSON?) -> String? {
    guard let value, value != .null else { return nil }
    return value.pythonString
  }

  /// prefix16 to full identity, from every place a full identity is written down.
  private func knownSHAs() -> [String: String] {
    var out: [String: String] = [:]
    for (_, pointer) in pointers() where CacheLayout.isSHA256(pointer.oid) {
      out[String(pointer.oid.prefix(16))] = pointer.oid
    }
    for local in localModels() where CacheLayout.isSHA256(local.sha256) {
      out[String(local.sha256.prefix(16))] = local.sha256
    }
    for fileName in Files.names(in: layout.engines) where fileName.hasSuffix(".json") {
      guard let sha256 = Files.readJSON(layout.engines.appending(path: fileName))?["spec"]?["sha256"]?.string,
        CacheLayout.isSHA256(sha256)
      else { continue }
      out[String(sha256.prefix(16))] = sha256
    }
    return out
  }

  /// Deletes what was asked for. A file that is already gone is not an error.
  ///
  /// The loaded engine is not this class's business: the control layer
  /// unloads first. Removing the artifacts also drops last-loaded.json when it
  /// names this model, and removing the model drops its import record, which
  /// would otherwise name a file that is gone.
  public func remove(sha256: String, artifacts: Bool, model: Bool) throws(RegistryError) {
    try CacheLayout.validate(sha256: sha256)
    let prefix = "\(sha256.prefix(16))."
    if artifacts {
      for fileName in Files.names(in: layout.engines) where fileName.hasPrefix(prefix) {
        let url = layout.engines.appending(path: fileName)
        if Files.isDirectory(url) {
          try? FileManager.default.removeItem(at: url)
        } else {
          do {
            try Files.removeFile(url)
          } catch {
            Registry.log.warning("could not remove \(url.path(percentEncoded: false), privacy: .public)")
          }
        }
      }
      if layout.lastLoaded()?.sha256 == sha256 {
        try? Files.removeFile(layout.lastLoadedURL)
      }
    }
    if model {
      let path = try layout.modelPath(sha256: sha256)
      do {
        try Files.removeFile(path)
        try Files.removeFile(path.deletingLastPathComponent().appending(path: path.lastPathComponent + ".part"))
      } catch {
        throw .registry("could not remove \(path.path(percentEncoded: false)): \(error.localizedDescription)")
      }
      try lock.withLock { _ throws(RegistryError) in
        let records = localRecords()
        let kept = records.filter { $0["sha256"] != .string(sha256) }
        guard kept.count != records.count else { return }
        do {
          try Files.writeJSON(.array(kept.map(JSON.object)), to: layout.localModelsURL)
        } catch {
          throw .registry("could not update the import records: \(error.localizedDescription)")
        }
      }
    }
  }
}
