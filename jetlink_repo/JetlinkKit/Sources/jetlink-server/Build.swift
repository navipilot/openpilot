#if os(macOS) || os(Linux)
  import ArgumentParser
  import Foundation
  import JetlinkKit
  import JetlinkRegistry
  import JetlinkServer
  #if os(Linux)
    import JetlinkLinux
    import JetlinkTRT
  #endif

  /// `jetlink-server build ONNX`: the engine a comma would get, built ahead
  /// of the drive (Python's --build).
  struct Build: ParsableCommand {
    static let configuration = CommandConfiguration(
      abstract: "Build this backend's engine for a model, or say it is built already.",
      discussion: "Stop a running server on the same cache first: both would build into it.")

    @Argument(help: "The model's ONNX.")
    var onnx: String
    @Option(help: "Model frames per camera frame the comma sends; the spec in the sidecar and the preload use it.")
    var frameSkip = Pinned.defaultFrameSkip
    @OptionGroup var chosen: BackendArguments
    @OptionGroup var cache: CacheArguments
    @Option(help: "debug, info, warning or error.")
    var logLevel = Log.Level.info

    func validate() throws {
      guard frameSkip > 0 else { throw ValidationError("--frame-skip must be at least 1") }
    }

    func run() throws {
      setUpLogging(logLevel)
      let backend = try chosen.pick()
      let (model, frameSkip, root) = (URL(fileURLWithPath: onnx), frameSkip, cache.root)
      try blocking { try await buildEngine(model: model, frameSkip: frameSkip, backend: backend, root: root) }
    }
  }

  /// Builds `model`'s engine into the cache at `root` unless it is there,
  /// and loads it once, through the controller's prepare: the one path that
  /// builds and loads an engine, as a comma's request does. The model is
  /// taken into the cache first, where a plan that goes stale is rebuilt
  /// from, unless `sha256` says it is there already. The sidecar carries the
  /// spec, and the model is what the next server start here preloads.
  /// Throws ExitCode.failure after logging why.
  func buildEngine(model: URL, sha256 known: String? = nil, frameSkip: Int, backend: any EngineBackend, root: URL) async throws {
    let log = ServerLog(category: "main")
    do {
      log.info("backend \(backend.name) \(backend.runtimeVersion) on \(backend.deviceTag()), cache \(root.path)")
      let server = try Server(configuration: Server.Configuration(cacheRoot: root, preload: false, listen: false), backend: backend)
      defer { server.shutdown() }
      var sha256 = known
      if sha256 == nil {
        sha256 = try await Registry(layout: server.cache.layout).importModel(at: model).sha256
      }
      let entry = try server.cache.entry(sha256!)
      let built = entry.exists
      log.info("model \(sha256!.prefix(16)) -> \(entry.path.path)")
      try await prepare(sha256!, frameSkip: frameSkip, on: server)
      log.info("\(built ? "already built" : "built"): \(sidecar(entry))")
    } catch {
      log.error("could not build \(model.path): \(error)")
      throw ExitCode.failure
    }
  }

  /// Loads `sha256` through the controller's prepare, building its engine
  /// first when there is none, and waits for it, logging each stage.
  func prepare(_ sha256: String, frameSkip: Int, on server: Server) async throws {
    let log = ServerLog(category: "main")
    let progress = stageLog { log.info($0) }
    let (ended, ending) = AsyncStream<Void>.makeStream()
    server.host.subscribe { event in
      switch event {
      case .progress(let stage, let frac, let msg): progress(stage, frac, msg)
      case .engine(let engine) where engine.sha256 == sha256 && (engine.state == .ready || engine.state == .failed): ending.yield()
      default: break
      }
    }
    let controller = ServerController(server: server, registry: Registry(layout: server.cache.layout))
    let reply = await controller.handle(.prepare(sha256: sha256, frameSkip: frameSkip))
    guard reply.ok else { throw HostError.failed(reply.error ?? "could not load \(sha256.prefix(16))") }
    let started = server.host.snapshot()
    guard started.sha256 == sha256, started.state != .none else {
      throw HostError.failed("could not load \(sha256.prefix(16)): its sidecar has no spec and the model is not here")
    }
    for await _ in ended { break }
    let engine = server.host.snapshot()
    guard engine.state == .ready, engine.sha256 == sha256 else {
      throw HostError.failed("could not load \(sha256.prefix(16)): \(engine.detail)")
    }
  }

  /// A build's or a load's progress as log lines: each stage's start, every
  /// 2 % of it, and its end. Per stage, since each runs 0 to 1, and a
  /// fraction carried over from the last would hide the next one's lines.
  func stageLog(_ write: @escaping @Sendable (String) -> Void) -> ProgressFn {
    let seen = Locked<(stage: String?, frac: Double)>((nil, 0))
    return { stage, frac, msg in
      let due = seen.withLock { seen in
        if stage != seen.stage { seen = (stage, 0) }
        guard frac - seen.frac >= 0.02 || frac >= 1 || frac == 0 else { return false }
        seen.frac = frac
        return true
      }
      if due {
        write("\(stage.padding(toLength: max(8, stage.count), withPad: " ", startingAt: 0)) \(String(format: "%5.1f", frac * 100))%  \(msg)")
      }
    }
  }

  /// The sidecar on one line, as the journal keeps a line.
  private func sidecar(_ entry: CacheEntry) -> String {
    (try? Data(contentsOf: entry.metaPath)).flatMap { try? JSONSerialization.jsonObject(with: $0) }.flatMap(jsonText) ?? "(no sidecar)"
  }

  /// `jetlink-server spec ONNX`: the ModelSpec JSON the comma is sent for a
  /// model (Python's --dump-spec).
  struct Spec: ParsableCommand {
    static let configuration = CommandConfiguration(abstract: "Print the spec the comma is sent for a model, as JSON.")

    @Argument(help: "The model's ONNX.")
    var onnx: String

    func run() throws {
      let model = URL(fileURLWithPath: onnx)
      let (sha256, nbytes) = try Registry.hashFile(model)
      let spec = try ONNXPreparer().readSpec(model: model, sha256: sha256, nbytes: nbytes, frameSkip: Pinned.defaultFrameSkip)
      print(jsonText(spec.dictionary()) ?? "{}")
    }
  }

  /// One line of JSON, keys sorted so two runs print the same.
  func jsonText(_ object: Any) -> String? {
    ControlJSON.data(object).map { String(decoding: $0, as: UTF8.self) }
  }

  /// `jetlink-server bench`: a built engine at the comma's pace, 20 frames a
  /// second through the real queues, with no comma and no link, so the
  /// server's own share of a frame can be read at home.
  struct Bench: ParsableCommand {
    static let configuration = CommandConfiguration(
      abstract: "Run a built engine at the comma's pace with no comma, and report its times.",
      discussion: "Stop a running server first: the engine needs the device to itself.")

    @Option(help: "How long to run.")
    var seconds = 60.0
    @Option(help: "The model to run. Default: the one loaded last.")
    var sha256: String?
    @Option(help: "Model frames per camera frame. Default: what it was loaded with last, else 4.")
    var frameSkip: Int?
    @OptionGroup var chosen: BackendArguments
    @OptionGroup var cache: CacheArguments
    @Option(help: "debug, info, warning or error.")
    var logLevel = Log.Level.info

    func validate() throws {
      guard seconds > 0 && seconds <= 3600 else { throw ValidationError("--seconds must be above 0 and at most 3600") }
      if let sha256, !CacheLayout.isSHA256(sha256) { throw ValidationError("--sha256 wants a 64 character sha256, not \(sha256)") }
      if let frameSkip, frameSkip < 1 { throw ValidationError("--frame-skip must be at least 1") }
    }

    func run() throws {
      setUpLogging(logLevel)
      let log = ServerLog(category: "main")
      let backend = try chosen.pick()
      do {
        var hooks = ServerHooks()
        #if os(Linux)
          // The report's temperature, from the GPU's
          hooks.thermal = LinuxHost.thermal(LinuxHost.telemetry(gpu: (backend as? TrtBackend)?.trt.device ?? 0))
        #endif
        let server = try Server(
          configuration: Server.Configuration(cacheRoot: cache.root, preload: false, listen: false), backend: backend, hooks: hooks)
        defer { server.shutdown() }
        let last = server.cache.lastLoaded()
        guard let wanted = sha256 ?? last?.sha256 else {
          throw HostError.failed("no model has been loaded here: name one with --sha256")
        }
        guard try server.cache.entry(wanted).exists else {
          throw HostError.failed("\(wanted.prefix(16)) is not built for \(backend.tag()): build it first")
        }
        let frameSkip = frameSkip ?? (last?.sha256 == wanted ? last!.frameSkip : Pinned.defaultFrameSkip)
        try blocking { try await prepare(wanted, frameSkip: frameSkip, on: server) }
        // The report goes to stdout alone, where a script reads it.
        let report = try server.host.benchmark(seconds: seconds, run: BenchmarkRun())
        print(report.text)
      } catch let exit as ExitCode {
        throw exit
      } catch {
        log.error("\(error)")
        throw ExitCode.failure
      }
    }
  }
#endif
