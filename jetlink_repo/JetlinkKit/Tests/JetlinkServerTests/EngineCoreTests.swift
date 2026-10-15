import Foundation
import Testing

@testable import JetlinkServer

/// An allocator that fills what it hands out, as page-locked memory comes,
/// and keeps count of what is still out.
final class CountingAllocator: @unchecked Sendable {
  private let lock = NSLock()
  private var live: Set<UnsafeMutableRawPointer> = []

  var outstanding: Int { lock.withLock { live.count } }

  var hook: HostAllocator {
    HostAllocator(
      allocate: { [self] bytes in
        let buffer = UnsafeMutableRawPointer.allocate(byteCount: bytes, alignment: 64)
        buffer.initializeMemory(as: UInt8.self, repeating: 0xEE, count: bytes)
        lock.withLock { _ = live.insert(buffer) }
        return buffer
      },
      free: { [self] buffer in
        lock.withLock { _ = live.remove(buffer) }
        buffer.deallocate()
      })
  }
}

/// x and state_x in; out = x + state_x and next_state_x = state_x + 1.
class StepEngine: EngineCore, @unchecked Sendable {
  static let pair = (input: "state_x", output: "next_state_x")
  var resets = 0

  init(allocator: HostAllocator) throws {
    let specs = ["x", "state_x", "out", "next_state_x"].map { TensorSpec(name: $0, type: .float, shape: [2]) }
    try super.init(
      inputs: Dictionary(uniqueKeysWithValues: specs[..<2].map { ($0.name, $0) }),
      outputs: Dictionary(uniqueKeysWithValues: specs[2...].map { ($0.name, $0) }), allocator: allocator)
  }

  override func execute() throws {
    let x = buffer("x", parity: parity)!.assumingMemoryBound(to: Float.self)
    let state = buffer("state_x", parity: parity)!.assumingMemoryBound(to: Float.self)
    let out = buffer("out", parity: parity)!.assumingMemoryBound(to: Float.self)
    // looped, next_state_x writes the state_x buffer the next run reads
    let next = (looped.isEmpty ? buffer("next_state_x", parity: parity) : buffer("state_x", parity: parity ^ 1))!
      .assumingMemoryBound(to: Float.self)
    for i in 0..<2 {
      out[i] = x[i] + state[i]
      next[i] = state[i] + 1
    }
  }

  override func stateDidReset() {
    resets += 1
  }

  func stage(_ values: [Float]) {
    hostInput("x")!.copyMemory(from: values, byteCount: 8)
  }

  var out: [Float] {
    Array(UnsafeBufferPointer(start: output("out")!.assumingMemoryBound(to: Float.self), count: 2))
  }
}

/// onnxruntime's way: two host copies of the state, swapped each run.
final class DoubleBufferedEngine: StepEngine, @unchecked Sendable {
  override func bindLoop(_ pairs: [(input: String, output: String)]) throws {
    try doubleBuffer(pairs.map(\.input))
  }
}

/// TensorRT's way: the state stays on the device, so the host keeps none.
final class DeviceLoopEngine: StepEngine, @unchecked Sendable {
  override func bindLoop(_ pairs: [(input: String, output: String)]) throws {
    releaseHostBuffers(pairs.flatMap { [$0.input, $0.output] })
  }

  override func execute() throws {
    Thread.sleep(forTimeInterval: 0.002)
  }
}

@Suite("Engine core")
struct EngineCoreTests {
  @Test("Host buffers come zeroed from the allocator, and close gives every one back")
  func buffers() throws {
    let allocator = CountingAllocator()
    let engine = try StepEngine(allocator: allocator.hook)
    #expect(allocator.outstanding == 4)
    #expect(UnsafeRawBufferPointer(start: engine.hostInput("x"), count: 8).allSatisfy { $0 == 0 })
    #expect(engine.hostInput("out") == nil && engine.output("x") == nil)
    engine.close()
    engine.close()
    #expect(allocator.outstanding == 0)
    #expect(throws: HostError.self) { try engine.run() }
  }

  @Test("An engine that cannot loop refuses the pairs and keeps them all")
  func noLoop() throws {
    let engine = try StepEngine(allocator: .heap)
    defer { engine.close() }
    #expect(throws: HostError.self) { try engine.loopState([StepEngine.pair]) }
    #expect(engine.looped.isEmpty)
    #expect(engine.hostInputs == ["state_x", "x"])
    #expect(engine.hostOutputs == ["next_state_x", "out"])
    #expect(engine.output("next_state_x") != nil)
    #expect(engine.resets == 0)
  }

  @Test("A double-buffered loop feeds the state back without the host")
  func doubleBuffered() throws {
    let allocator = CountingAllocator()
    let engine = try DoubleBufferedEngine(allocator: allocator.hook)
    try engine.loopState([StepEngine.pair])
    #expect(allocator.outstanding == 5)
    #expect(engine.hostInputs == ["x"] && engine.hostOutputs == ["out"])
    #expect(engine.output("next_state_x") == nil)
    engine.stage([10, 20])
    var outs: [[Float]] = []
    for _ in 0..<3 {
      try engine.run()
      outs.append(engine.out)
    }
    #expect(outs == [[10, 20], [11, 21], [12, 22]])
    #expect(engine.parity == 1)
    engine.resetState()
    #expect(engine.parity == 0 && engine.resets == 2)
    try engine.run()
    #expect(engine.out == [10, 20])
    engine.close()
    #expect(allocator.outstanding == 0)
  }

  @Test("A loop on the device frees the pair's host buffers, and resets reach the engine")
  func deviceLoop() throws {
    let allocator = CountingAllocator()
    let engine = try DeviceLoopEngine(allocator: allocator.hook)
    defer { engine.close() }
    try engine.loopState([StepEngine.pair])
    #expect(allocator.outstanding == 2)
    #expect(engine.hostInput("state_x") == nil && engine.output("next_state_x") == nil)
    #expect(engine.hostOutputs == ["out"])
    let before = engine.resets
    engine.resetState()
    #expect(engine.resets == before + 1)
    _ = try engine.warm()
    #expect(engine.parity == 0)
    #expect(engine.lastGpuUs >= 2000 && engine.lastRunNanoseconds >= 2_000_000)
  }
}
