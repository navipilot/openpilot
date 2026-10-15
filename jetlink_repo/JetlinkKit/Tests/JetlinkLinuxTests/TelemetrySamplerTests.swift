import Foundation
import JetlinkServer
import JetlinkTestSupport
import Testing

/// The sampler on a clock the test moves, over a source that counts its reads.
@Suite("Telemetry sampler")
struct TelemetrySamplerTests {
  final class Source: @unchecked Sendable {
    let reads = Locked(0)
    let gate = DispatchSemaphore(value: 0)
    let gated: Bool

    init(gated: Bool = false) {
      self.gated = gated
    }

    func read() -> [String: Any] {
      if gated { gate.wait() }
      reads.value += 1
      return ["temp_c": 40.5, "read": reads.value]
    }
  }

  let clock = Locked(100.0)

  func sampler(_ source: Source) -> TelemetrySampler {
    let clock = clock
    return TelemetrySampler(clock: { clock.value }, source: { source.read() })
  }

  @Test("The first sample is taken at the start, and read hands it back")
  func firstSample() {
    let source = Source()
    let sampler = sampler(source)
    defer { sampler.close() }
    #expect(eventually { sampler.read()["read"] as? Int == 1 })
    #expect(sampler.read()["temp_c"] as? Double == 40.5)
  }

  @Test("A read while the sensor is stuck returns at once, with nothing")
  func neverBlocks() {
    let source = Source(gated: true)
    let sampler = sampler(source)
    defer {
      sampler.close()
      source.gate.signal()
    }
    let start = Date()
    for _ in 0..<100 {
      #expect(sampler.read().isEmpty)
    }
    #expect(Date().timeIntervalSince(start) < 0.5)
    source.gate.signal()
    #expect(eventually { !sampler.read().isEmpty })
  }

  @Test("The sensor is read at most once a period, and only when someone reads")
  func oncePerPeriod() {
    let source = Source()
    let sampler = sampler(source)
    defer { sampler.close() }
    #expect(eventually { source.reads.value == 1 })
    clock.value += 0.05
    _ = sampler.read()
    Thread.sleep(forTimeInterval: 0.05)
    #expect(source.reads.value == 1)
    clock.value += 0.1
    _ = sampler.read()
    #expect(eventually { source.reads.value == 2 })
    // Nobody reads: the time passes without a sample.
    clock.value += 10
    Thread.sleep(forTimeInterval: 0.05)
    #expect(source.reads.value == 2)
  }

  @Test("A sample older than a second reads as none, never as an old value")
  func stale() {
    let source = Source(gated: true)
    let sampler = sampler(source)
    defer {
      sampler.close()
      source.gate.signal()
    }
    source.gate.signal()
    #expect(eventually { sampler.read()["read"] as? Int == 1 })
    // Asks for the next sample, which the sensor holds up.
    clock.value += 1.0
    #expect(sampler.read()["read"] as? Int == 1)
    clock.value += 0.01
    #expect(sampler.read().isEmpty)
    // Fresh from when it was taken, not from when it came back.
    source.gate.signal()
    #expect(eventually { sampler.read()["read"] as? Int == 2 })
  }

  @Test("After close it reads nothing and samples no more")
  func closed() {
    let source = Source()
    let sampler = sampler(source)
    #expect(eventually { source.reads.value == 1 })
    sampler.close()
    clock.value += 5
    #expect(sampler.read().isEmpty)
    Thread.sleep(forTimeInterval: 0.05)
    #expect(source.reads.value == 1)
  }
}
