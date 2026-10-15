#if os(macOS) || os(Linux)
  import ArgumentParser
  import Foundation
  import JetlinkKit
  import JetlinkRegistry
  import JetlinkServer
  import JetlinkTestSupport
  import Testing

  @testable import jetlink_server

  struct ParsingTests {
    private func root(_ arguments: [String]) throws -> any ParsableCommand {
      try JetlinkServerCommand.parseAsRoot(arguments)
    }

    @Test func serveIsTheDefaultAndTakesWhatTheInstalledUnitPasses() throws {
      let command = try root([
        "--usb", "--backend", "trt", "--cache", "/var/lib/jetlink", "--sleep-after", "900", "--status-port", "5600", "--poweroff",
      ])
      let serve = try #require(command as? Serve)
      #expect(serve.usb && !serve.listen && serve.poweroff)
      #expect(serve.chosen.backend == .trt && serve.chosen.device == nil)
      #expect(serve.cache.root.path == "/var/lib/jetlink")
      #expect(serve.sleepAfter == 900 && serve.statusPort == 5600)
      #expect(try #require(try root(["--device", "auto"]) as? Serve).chosen.options().device == nil, "auto is each backend's default")
      let libs = try #require(try root(["bench", "--tensorrt-libs", "/opt/jetlink/tensorrt/11.3.0.99"]) as? Bench)
      #expect(libs.chosen.options().tensorrtLibs == "/opt/jetlink/tensorrt/11.3.0.99")
    }

    @Test func litertIsNamedAndTakesGpuOrCpu() throws {
      let serve = try #require(try root(["--backend", "litert", "--device", "cpu"]) as? Serve)
      #expect(serve.chosen.backend == .litert && serve.chosen.device == "cpu")
      #expect { try BackendOptions(device: "tpu").litert() } throws: { "\($0)".contains("gpu or cpu or npu") }
      // the NPU's profile is a phone's
      #expect { try BackendOptions(device: "npu").litert() } throws: { "\($0)".contains("runs on Android") }
    }

    @Test func tensorrtDefaultsToLibBesideBinWhenThere() throws {
      let tmp = try TemporaryDirectory()
      let binary = tmp.url.appending(path: "bin/jetlink-server")
      #expect(bundledTensorRT(executable: binary) == nil, "none there: the loader path")
      try FileManager.default.createDirectory(at: tmp.url.appending(path: "lib/tensorrt"), withIntermediateDirectories: true)
      #expect(bundledTensorRT(executable: binary) == tmp.url.appending(path: "lib/tensorrt").path)
      #expect(bundledTensorRT(executable: nil) == nil)
    }

    @Test func usageMistakesAreRefused() {
      for arguments in [
        ["--sleep-after", "900"],  // without --usb
        ["--sleep-after", "-1", "--usb"],
        ["--dial", "a:b:c"],
        ["--status-port", "70000"],
        ["--backend", "cuda"],
        ["--bogus"],
        ["build", "m.onnx", "--frame-skip", "0"],
        ["bench", "--seconds", "0"],
        ["bench", "--sha256", "nothex"],
        ["models", "fetch"],
      ] {
        #expect(throws: (any Error).self, "\(arguments)") { try root(arguments) }
      }
    }
  }

  struct VersionTests {
    @Test func aVersionFileBesideBinWins() throws {
      let tmp = try TemporaryDirectory()
      let binary = tmp.url.appending(path: "bin/jetlink-server")
      #expect(productVersion(executable: binary) == Pinned.productVersion)
      try "0.7.0-dev.8b8268e\n".write(to: tmp.url.appending(path: "VERSION"), atomically: true, encoding: .utf8)
      #expect(productVersion(executable: binary) == "0.7.0-dev.8b8268e")
      try "\n".write(to: tmp.url.appending(path: "VERSION"), atomically: true, encoding: .utf8)
      #expect(productVersion(executable: binary) == Pinned.productVersion)
      #expect(productVersion(executable: nil) == Pinned.productVersion)
    }
  }

  /// The Linux rules are JetlinkLinux's Platform, tested there.
  struct CacheDefaultTests {
    @Test func jetlinkCacheWins() {
      #expect(defaultCache(environment: ["JETLINK_CACHE": "/somewhere"]).path == "/somewhere")
    }

    #if os(macOS)
      @Test func elseTheUsersCacheDirectory() {
        let home = FileManager.default.homeDirectoryForCurrentUser.path
        #expect(defaultCache(environment: [:]).path == home + "/Library/Caches/jetlink")
      }
    #endif
  }

  struct BuildTests {
    /// onnxruntime's CPU provider, which every platform's tests have.
    private func cpu() throws -> any EngineBackend {
      try BackendOptions(device: "cpu").ort()
    }

    @Test func buildsOnceCarriesTheSpecAndRecordsThePreload() async throws {
      let tmp = try TemporaryDirectory()
      let backend = try cpu()
      let queued = try TinyModel.sha256(TinyModel.queued)
      try await buildEngine(model: TinyModel.queued, frameSkip: 2, backend: backend, root: tmp.url)
      let cache = try ServerCache(root: tmp.url, backend: backend)
      let entry = try cache.entry(queued)
      #expect(entry.exists)
      // taken into the cache, where a stale engine is rebuilt from
      #expect(FileManager.default.fileExists(atPath: try cache.modelPath(queued).path))
      let meta = try #require(try JSONSerialization.jsonObject(with: Data(contentsOf: entry.metaPath)) as? [String: Any])
      let spec = try #require(meta["spec"] as? [String: Any])
      #expect(spec["sha256"] as? String == queued && (spec["frame_skip"] as? NSNumber)?.intValue == 2)
      #expect(cache.lastLoaded()?.sha256 == queued && cache.lastLoaded()?.frameSkip == 2)

      // Built already: loaded, not built again. The model built last is the one preloaded.
      let built = try FileManager.default.attributesOfItem(atPath: entry.path.path)[.modificationDate] as? Date
      try await buildEngine(model: TinyModel.queued, frameSkip: 2, backend: backend, root: tmp.url)
      #expect(try FileManager.default.attributesOfItem(atPath: entry.path.path)[.modificationDate] as? Date == built)
      try await buildEngine(model: TinyModel.stateful, frameSkip: 4, backend: backend, root: tmp.url)
      #expect(try cache.entry(try TinyModel.sha256(TinyModel.stateful)).exists)
      #expect(cache.lastLoaded()?.sha256 == (try TinyModel.sha256(TinyModel.stateful)))
    }

    @Test func aModelThatIsNotThereFails() async throws {
      let tmp = try TemporaryDirectory()
      await #expect(throws: ExitCode.failure) {
        try await buildEngine(model: tmp.url.appending(path: "missing.onnx"), frameSkip: 4, backend: try cpu(), root: tmp.url)
      }
    }

    @Test func theStageLogKeepsEveryTwoPercentOfEachStage() {
      let lines = Locked<[String]>([])
      let report = stageLog { line in lines.withLock { $0.append(line) } }
      for (stage, frac) in [("build", 0.0), ("build", 0.01), ("build", 0.021), ("build", 0.03), ("build", 1.0), ("load", 0.5), ("load", 0.51)] {
        report(stage, frac, "working")
      }
      #expect(
        lines.withLock { $0 } == [
          "build      0.0%  working", "build      2.1%  working", "build    100.0%  working", "load      50.0%  working",
        ])
    }
  }
#endif
