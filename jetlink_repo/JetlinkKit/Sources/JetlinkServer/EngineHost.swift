import Foundation
import JetlinkKit
import JetlinkRegistry

/// An engine resident on the device, with the state that goes with it.
final class Loaded {
  let sha256: String
  let spec: ModelSpec
  let engine: any Engine
  let staging: any FrameStaging

  init(sha256: String, spec: ModelSpec, engine: any Engine, staging: any FrameStaging) {
    self.sha256 = sha256
    self.spec = spec
    self.engine = engine
    self.staging = staging
  }
}

/// One build or load in flight, or its outcome. Guarded by the host's lock.
final class Job: @unchecked Sendable {
  enum State: String { case building, ready, failed }

  let sha256: String
  let loadOnly: Bool
  var state: State = .building
  var detail = ""

  init(sha256: String, loadOnly: Bool) {
    self.sha256 = sha256
    self.loadOnly = loadOnly
  }
}

/// What a client asked for: enough to identify the model without the file.
struct Request: Equatable {
  let sha256: String
  let nbytes: Int64
  let frameSkip: Int

  init(sha256: String, nbytes: Int64, frameSkip: Int) throws {
    guard CacheLayout.isSHA256(sha256) else { throw HostError.invalid("model identity must be a lowercase SHA-256 digest") }
    guard nbytes >= 0, frameSkip > 0 else { throw HostError.invalid("invalid model size or frame skip") }
    self.sha256 = sha256
    self.nbytes = nbytes
    self.frameSkip = frameSkip
  }
}

package enum HostError: Error, CustomStringConvertible {
  case invalid(String)
  case failed(String)

  package var description: String {
    switch self {
    case .invalid(let detail), .failed(let detail): return detail
    }
  }
}

/// What the host tells its listeners: the control layer, and through it the app.
public enum HostEvent: Sendable {
  case progress(stage: String, frac: Double, msg: String)
  case engine(EngineEvent)
  case link(LinkEvent)
  /// Once a second while a comma is connected and sending.
  case stats(StatsEvent)
  /// A benchmark's progress and its report.
  case benchmark(BenchmarkEvent)
  /// The comma asked for a power-off, which this server refused: the host
  /// passed no shutdown hook, or its hook declined.
  case shutdownRequested(reason: String)
}

/// Process-wide owner of the loaded engine and of the build in flight: the
/// Swift form of `session.EngineHost`.
///
/// The engine outlives the connection. The comma reconnects at every
/// handover, and reloading a 770 MB model costs seconds of modeld's budget.
/// Everything that touches `loaded` holds `lock`: the job thread swaps engines
/// while the request loop runs frames.
public final class EngineHost: @unchecked Sendable {
  static let progressInterval: TimeInterval = 0.25

  let cache: ServerCache
  let lock = NSLock()
  var loaded: Loaded?
  var job: Job?
  /// Who hears about progress and completion over the link.
  var session: Session?
  /// A benchmark owns the engine: frames from a comma are answered NOT_READY.
  var benchmarking = false
  /// When the comma last did something, a frame, a hello or the load it
  /// asked for: `warmIfIdle` goes by it.
  var lastSeenAt: TimeInterval = 0
  private var warmFailed = false
  let frameStats = FrameStats()
  let log = ServerLog(category: "server")

  private var lastProgress: TimeInterval = 0
  private var lastStage: (stage: String?, frac: Double, msg: String) = (nil, 0, "")
  private var lastEngine: EngineEvent?
  private let listenersLock = NSLock()
  private var listeners: [@Sendable (HostEvent) -> Void] = []
  private let emitLock = NSLock()

  /// The host's telemetry, thermal state, sleep and power: `ServerHooks`.
  let hooks: ServerHooks
  /// `hooks.telemetry`, sampled off the frame path.
  let telemetry: TelemetrySampler

  public init(cache: ServerCache, hooks: ServerHooks = ServerHooks()) {
    self.cache = cache
    self.hooks = hooks
    telemetry = TelemetrySampler(source: hooks.telemetry)
  }

  deinit {
    telemetry.close()
  }

  var backend: any EngineBackend { cache.backend }

  // MARK: listeners

  public func subscribe(_ listener: @escaping @Sendable (HostEvent) -> Void) {
    listenersLock.lock()
    listeners.append(listener)
    listenersLock.unlock()
  }

  /// An engine event identical to the last one is dropped: a job finishing
  /// emits from the progress call and again from its end, and the two are the
  /// same event.
  func emit(_ event: HostEvent) {
    emitLock.lock()
    if case .engine(let snapshot) = event {
      if snapshot == lastEngine {
        emitLock.unlock()
        return
      }
      lastEngine = snapshot
    }
    emitLock.unlock()
    listenersLock.lock()
    let current = listeners
    listenersLock.unlock()
    for listener in current {
      listener(event)
    }
  }

  /// The `engine` event: what is loaded, or what is being prepared.
  public func snapshot() -> EngineEvent {
    lock.lock()
    defer { lock.unlock() }
    return snapshotLocked()
  }

  private func snapshotLocked() -> EngineEvent {
    let (stage, frac, msg) = lastStage
    if let loaded {
      let accelerator = loaded.engine.accelerator
      return EngineEvent(
        state: .ready, sha256: loaded.sha256, detail: "", stage: nil, frac: 1, msg: msg, loadOnly: job?.loadOnly ?? false,
        accelerator: accelerator.isEmpty ? nil : accelerator)
    }
    if let job, job.state == .building {
      return EngineEvent(
        state: job.loadOnly ? .loading : .building, sha256: job.sha256, detail: job.detail, stage: stage, frac: frac, msg: msg, loadOnly: job.loadOnly)
    }
    if let job, job.state == .failed {
      return EngineEvent(state: .failed, sha256: job.sha256, detail: job.detail, stage: "failed", frac: frac, msg: msg, loadOnly: job.loadOnly)
    }
    return .none
  }

  // MARK: what a client sees

  /// The engine state for one model, in the shape ENGINE_RESP carries. frame_skip
  /// is part of the identity: the spec handed back is stamped with it.
  func status(_ sha256: String?, frameSkip: Int? = nil) -> [String: Any] {
    lock.lock()
    defer { lock.unlock() }
    return statusLocked(sha256, frameSkip: frameSkip)
  }

  private func statusLocked(_ sha256: String?, frameSkip: Int? = nil) -> [String: Any] {
    let chunk = ModelConstants.chunk
    guard let sha256 else {
      return ["state": "none", "detail": "", "sha256": NSNull(), "chunk": chunk]
    }
    if let loaded, loaded.sha256 == sha256, frameSkip == nil || loaded.spec.frameSkip == frameSkip {
      return ["state": "ready", "detail": "", "sha256": sha256, "chunk": chunk, "spec": loaded.spec.dictionary()]
    }
    if let job, job.sha256 == sha256, job.state != .ready {
      return ["state": job.state.rawValue, "detail": job.detail, "sha256": sha256, "chunk": chunk]
    }
    if let job, job.state == .building {
      return ["state": "building", "sha256": sha256, "chunk": chunk, "detail": "another build is in progress (\(job.sha256.prefix(16)))"]
    }
    if let entry = try? cache.entry(sha256), cachedSpec(entry) != nil {
      // Built already, just not loaded. modeld, which never carries the ONNX,
      // would read need_upload as an engine that is gone.
      return ["state": "building", "sha256": sha256, "chunk": chunk, "detail": "engine cached, not loaded yet"]
    }
    return ["state": "need_upload", "sha256": sha256, "chunk": chunk, "detail": "have \(modelBytes(sha256)) of the model"]
  }

  public func loadedSHA() -> String? {
    lock.lock()
    defer { lock.unlock() }
    return loaded?.sha256
  }

  // MARK: requests

  /// Make `request` the model being served, starting whatever that takes.
  /// A control-channel prepare passes no session, and must not detach the
  /// comma's session from the progress it is waiting on.
  func request(_ request: Request, session: Session?) -> [String: Any] {
    lock.lock()
    if let session {
      self.session = session
    }
    if let loaded, loaded.sha256 == request.sha256, loaded.spec.frameSkip == request.frameSkip {
      let ready = readyStatus(loaded)
      lock.unlock()
      return ready
    }
    if let job, job.state == .building {
      let status = statusLocked(request.sha256, frameSkip: request.frameSkip)
      lock.unlock()
      return status
    }
    lock.unlock()
    let entry = cache.entry(request)
    let modelPath = cache.modelPath(request)
    let spec = specOnDisk(entry, modelPath: modelPath, request: request)
    if entry.exists, let spec {
      start(Job(sha256: request.sha256, loadOnly: true), request: request, entry: entry, modelPath: modelPath, spec: spec)
    } else if modelComplete(modelPath, nbytes: request.nbytes) {
      start(Job(sha256: request.sha256, loadOnly: false), request: request, entry: entry, modelPath: modelPath, spec: spec)
    } else {
      lock.lock()
      if let job, job.sha256 == request.sha256, job.state == .failed {
        // Whatever failed has left the disk; what the client needs now is need_upload.
        self.job = nil
      }
      lock.unlock()
    }
    return status(request.sha256, frameSkip: request.frameSkip)
  }

  private func readyStatus(_ loaded: Loaded) -> [String: Any] {
    ["state": "ready", "detail": "", "sha256": loaded.sha256, "chunk": ModelConstants.chunk, "spec": loaded.spec.dictionary()]
  }

  /// The spec a cached artifact's sidecar carries, if it has one.
  func cachedSpec(_ entry: CacheEntry) -> [String: Any]? {
    guard entry.exists else { return nil }
    let meta = Artifact.sidecar(entry.path)
    if meta.isEmpty {
      log.warning("unreadable sidecar for \(entry.path.lastPathComponent)")
    }
    return meta["spec"] as? [String: Any]
  }

  private func modelBytes(_ sha256: String) -> Int64 {
    (try? cache.modelPath(sha256)).map { Files.size(of: $0) } ?? 0
  }

  /// Start loading whatever was loaded last, before a client asks for it.
  public func preload() {
    guard let last = cache.lastLoaded() else { return }
    let (sha256, frameSkip) = (last.sha256, last.frameSkip)
    lock.lock()
    let busy = loaded != nil || job != nil
    lock.unlock()
    if busy { return }
    guard let request = try? Request(sha256: sha256, nbytes: 0, frameSkip: frameSkip) else { return }
    let entry = cache.entry(request)
    guard let d = cachedSpec(entry), let spec = try? ModelSpec.from(d).withFrameSkip(frameSkip) else { return }
    log.info("preloading the engine loaded last: \(entry.path.lastPathComponent)")
    start(Job(sha256: sha256, loadOnly: true), request: request, entry: entry, modelPath: cache.modelPath(request), spec: spec)
  }

  /// The spec for a cached artifact, from its sidecar or failing that the ONNX.
  /// Only a whole model file is parsed.
  private func specOnDisk(_ entry: CacheEntry, modelPath: URL, request: Request) -> ModelSpec? {
    if let d = cachedSpec(entry), let spec = try? ModelSpec.from(d) {
      return spec.withFrameSkip(request.frameSkip)
    }
    if modelComplete(modelPath, nbytes: request.nbytes) {
      do {
        return try backend.deriveSpec(model: modelPath, sha256: request.sha256, nbytes: request.nbytes, frameSkip: request.frameSkip)
      } catch {
        log.error("could not derive a spec from \(modelPath.lastPathComponent): \(String(describing: error))")
      }
    }
    return nil
  }

  // MARK: the worker

  private func start(_ job: Job, request: Request, entry: CacheEntry, modelPath: URL, spec: ModelSpec?) {
    job.detail = job.loadOnly ? "loading engine" : "building engine"
    lock.lock()
    self.job = job
    // The stage belongs to the job: a fresh job must not show the last one's 100 %.
    lastStage = (nil, 0, "")
    lock.unlock()
    let thread = Thread { [self] in run(job, request: request, entry: entry, modelPath: modelPath, spec: spec) }
    thread.name = "jetlink-build"
    thread.qualityOfService = .userInitiated
    thread.stackSize = 8 << 20
    thread.start()
    emit(.engine(snapshot()))
  }

  private func run(_ job: Job, request: Request, entry: CacheEntry, modelPath: URL, spec: ModelSpec?) {
    var engine: (any Engine)?
    var fatal: (any Error)?
    do {
      // One engine resident at a time: a build needs the memory.
      unload()
      var spec = spec
      if !job.loadOnly {
        spec = try buildJob(request, entry: entry, modelPath: modelPath, spec: spec)
      }
      guard var ready = spec else { throw HostError.failed("no model spec") }
      writeSpec(entry, ready)
      progress("load", 0, "deserializing engine", force: true)
      do {
        engine = try backend.load(artifact: entry.path, report: progressFn)
      } catch let invalid as ArtifactInvalid {
        // Wrong on disk, not wrong here, whichever backend says so. Replace it
        // once from the ONNX when that is on disk, else let the client upload
        // again. A preload names no size; a client's request does, and the
        // model has to match it.
        log.warning("discarding \(entry.path.lastPathComponent): \(invalid.description)")
        entry.remove()
        let size = Files.size(of: modelPath)
        let have = size > 0 && (request.nbytes == 0 || size == request.nbytes)
        if job.loadOnly && !have {
          throw HostError.failed("artifact invalid and the model is not on disk: \(invalid.description)")
        }
        ready = try buildJob(request, entry: entry, modelPath: modelPath, spec: ready)
        writeSpec(entry, ready)
        progress("load", 0, "deserializing engine", force: true)
        engine = try backend.load(artifact: entry.path, report: progressFn)
      }
      // A frame or two on zeros: minutes on a phone's CPU, so it ticks.
      let warming = engine!
      let loaded = try Ticker.during(interval: 1, Ticker.paced("warm", "warming up", took: 0, report: progressFn)) {
        try warm(warming, spec: ready)
      }
      engine = nil
      lock.lock()
      self.loaded = loaded
      lastSeenAt = ProcessInfo.processInfo.systemUptime
      warmFailed = false
      job.state = .ready
      job.detail = ""
      lock.unlock()
      cache.rememberLoaded(request.sha256, frameSkip: request.frameSkip)
      progress("load", 1, "ready", force: true)
      log.info("engine ready: \(entry.path.lastPathComponent)")
    } catch {
      log.error("engine preparation failed: \(String(describing: error))")
      if (error as? any FatalEngineError)?.isFatal == true {
        fatal = error
      }
      engine?.close()
      lock.lock()
      job.state = .failed
      job.detail = "\(type(of: error)): \(error)"
      lock.unlock()
      progress("failed", 1, job.detail, force: true)
    }
    lock.lock()
    let session = self.session
    lock.unlock()
    if let session {
      servePending(job, session: session)
      session.engineUpdate()
    }
    emit(.engine(snapshot()))
    // D15, as on the frame path: a device broken while building, loading or
    // warming up stays broken for every later job, so the host gets it once
    // the comma has heard the job failed.
    if let fatal {
      hooks.fatal?(fatal)
    }
  }

  /// Start what the client is still waiting for, now the device is free. A
  /// preload that guessed the wrong sha holds the device while the client waits.
  private func servePending(_ done: Job, session: Session) {
    guard let request = session.request, done.sha256 != request.sha256 else { return }
    _ = self.request(request, session: session)
  }

  private func buildJob(_ request: Request, entry: CacheEntry, modelPath: URL, spec: ModelSpec?) throws -> ModelSpec {
    var spec = spec
    if spec == nil {
      progress("parse", 0, "reading model metadata", force: true)
      spec = try backend.deriveSpec(model: modelPath, sha256: request.sha256, nbytes: request.nbytes, frameSkip: request.frameSkip)
    }
    try backend.build(model: modelPath, artifact: entry.path, report: progressFn, metaExtra: ["spec": spec!.dictionary()])
    cache.prune(protect: entry.path)
    cache.sweepTemp()
    return spec!
  }

  private func writeSpec(_ entry: CacheEntry, _ spec: ModelSpec) {
    var meta = Artifact.sidecar(entry.path)
    if meta["spec"] == nil {
      meta["spec"] = spec.dictionary()
      try? Artifact.writeSidecar(entry.path, meta)
    }
  }

  private func warm(_ engine: any Engine, spec: ModelSpec) throws -> Loaded {
    try checkShapes(engine, spec: spec)
    let staging = try Staging.forModel(spec, engine: engine)
    // Warm on zeros, so the first real frame pays for nothing lazy.
    try Self.stageZeros(staging, spec: spec)
    let warmed = try engine.warm()
    log.info("\(warmed)")
    staging.reset()
    return Loaded(sha256: spec.sha256, spec: spec, engine: engine, staging: staging)
  }

  private static func stageZeros(_ staging: any FrameStaging, spec: ModelSpec) throws {
    let warped = [UInt8](repeating: 0, count: spec.warpedBytes)
    let packed = [Float](repeating: 0, count: spec.packedCount)
    try warped.withUnsafeBytes { w in
      try packed.withUnsafeBytes { p in
        try staging.stage(warped: w.baseAddress!, packed: p.baseAddress!)
      }
    }
  }

  // MARK: keeping warm between sessions

  /// Warm runs start this long after the comma was last seen: a joined comma
  /// that hears nothing for 0.2 s hands back, so it has left, and a run on
  /// zeros stays out of a drive's history. A hello waits out the gap too, so
  /// its first frame never queues behind a warm run.
  static let warmIdleAfter: TimeInterval = 5
  /// And stop this long after: its rejoin backs off to 60 s, and a parked
  /// comma should not cost the battery all night.
  static let warmFor: TimeInterval = 120

  /// One run on zeros between sessions, at the server's tick, so the next
  /// join's first frame finds an accelerator that cools when idle awake: an
  /// iPad's Neural Engine left idle for 20 to 60 s took 67 to 123 ms on that
  /// frame against about 30 after it, and the comma holds a frame past 46 ms.
  /// Returns whether it ran.
  @discardableResult
  func warmIfIdle(now: TimeInterval = ProcessInfo.processInfo.systemUptime) -> Bool {
    lock.lock()
    defer { lock.unlock() }
    guard let loaded, loaded.engine.coolsWhenIdle, !benchmarking, !warmFailed,
      (Self.warmIdleAfter...Self.warmFor).contains(now - lastSeenAt)
    else { return false }
    do {
      try Self.stageZeros(loaded.staging, spec: loaded.spec)
      try loaded.engine.run()
    } catch {
      warmFailed = true
      log.error("stopped keeping the engine warm between sessions: \(String(describing: error))")
    }
    loaded.staging.reset()
    return !warmFailed
  }

  /// Release the engine on a client's say-so.
  public func unload() {
    lock.lock()
    let old = loaded
    loaded = nil
    lock.unlock()
    if let old {
      old.engine.close()
      log.info("engine \(old.sha256.prefix(16)) unloaded")
      emit(.engine(snapshot()))
    }
  }

  public func close() {
    unload()
    telemetry.close()
  }

  // MARK: talking back

  private var progressFn: ProgressFn {
    { [weak self] stage, frac, msg in self?.progress(stage, frac, msg) }
  }

  func progress(_ stage: String, _ frac: Double, _ msg: String, force: Bool = false) {
    let now = ProcessInfo.processInfo.systemUptime
    lock.lock()
    // Throttled within a stage only: a new stage's first word is never held
    // back, or the screen would show the last stage's until the next tick.
    if !force && frac < 1 && stage == lastStage.stage && now - lastProgress < EngineHost.progressInterval {
      lock.unlock()
      return
    }
    lastProgress = now
    lastStage = (stage, frac, msg)
    let session = self.session
    lock.unlock()
    emit(.progress(stage: stage, frac: pythonRound(frac, 4), msg: msg))
    session?.progress(stage, frac, msg)
  }
}

/// The engine is what will execute and the spec came from the file, so
/// disagreement means the artifact on disk is not this model's.
func checkShapes(_ engine: any Engine, spec: ModelSpec) throws {
  for (name, io) in engine.inputs {
    guard let want = spec.input(name) else {
      throw HostError.failed("engine input \(name) is not in the model spec")
    }
    if io.count != want.reduce(1, *) {
      throw HostError.failed("input \(name): engine \(io.shape) vs spec \(want)")
    }
  }
  let missing = Set(spec.inputShapes.map(\.name)).subtracting(engine.inputs.keys)
  if !missing.isEmpty {
    throw HostError.failed("engine has no input(s) \(missing.sorted()) the model spec declares")
  }
  guard let out = engine.outputs[ModelConstants.drivingOutput], out.count == spec.outputCount else {
    throw HostError.failed("output: engine \(engine.outputs[ModelConstants.drivingOutput]?.shape ?? []) vs spec \(spec.outputCount)")
  }
  let missingState = Set(spec.statePairs.map(\.output)).subtracting(engine.outputs.keys)
  if !missingState.isEmpty {
    throw HostError.failed("engine has no output(s) \(missingState.sorted()) to feed the state back from")
  }
}

/// Uploads land in place chunk by chunk, so the model file is only the model
/// once it is the size the client declared.
func modelComplete(_ url: URL, nbytes: Int64) -> Bool {
  Files.status(url).map { $0.isFile && $0.size == nbytes } ?? false
}
