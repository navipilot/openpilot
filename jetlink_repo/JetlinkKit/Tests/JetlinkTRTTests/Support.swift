import CTrt
import Foundation
import JetlinkLog
import JetlinkTestSupport

@testable import JetlinkServer
@testable import JetlinkTRT

/// The fake plan's lines for the server tests' tiny stateful model
/// (`TinyModel.stateful`), as the TensorRT build of its patched ONNX declares
/// them: the image fp16, each next_state_ fed from its state_.
let tinyPlanLines = """
  input new_img float16 2 6 8 16
  input desire float32 8
  input traffic_convention float32 1 2
  input action_t float32 1 2
  input state_img_q float16 2 5 6 8 16
  input state_desire_q float16 6 1 8
  input state_feat_q float16 4 1 16
  output outputs float32 1 64
  output next_state_img_q float16 2 5 6 8 16 from state_img_q
  output next_state_desire_q float16 6 1 8 from state_desire_q
  output next_state_feat_q float16 4 1 16 from state_feat_q

  """

func tinySpec() throws -> ModelSpec {
  try ModelSpec.from(TinyModel.spec(TinyModel.stateful))
}

/// How many lines the CUDA-event timing has logged for a block of `frames`
/// ("7 frames"), from the ring every server line goes to.
func timingLines(_ frames: String) -> Int {
  LogRing.shared.lines().filter { $0.contains("jetlink.trt: gpu (cuda events), \(frames): mean") }.count
}

#if JL_TRT_FAKE
  struct FakeOpenFailed: Error {
    let message: String
  }

  /// The fake shim with the defaults (TensorRT 10.3.0.30 on an Orin, sm87)
  /// and `configure` on top.
  func fakeTensorRT(device: String = "Orin", _ configure: (inout jl_trt_fake_config) -> Void = { _ in }) throws -> TensorRT {
    var config = jl_trt_fake_config()
    jl_trt_fake_defaults(&config)
    configure(&config)
    var handle: OpaquePointer?
    var err = [CChar](repeating: 0, count: 256)
    let rc = device.withCString { name in
      config.device_name = name
      return jl_trt_fake_open(&config, &handle, &err, err.count)
    }
    guard rc == JL_TRT_OK, let handle else { throw FakeOpenFailed(message: string(err)) }
    return TensorRT(handle: handle)
  }

  extension TensorRT {
    var stats: jl_trt_fake_stats {
      var stats = jl_trt_fake_stats()
      jl_trt_fake_get_stats(handle, &stats)
      return stats
    }

    /// Every live object the fake counts: all 0 once everything is closed.
    var live: [Int64] {
      let s = stats
      return [s.device_allocs, s.host_allocs, s.streams, s.events, s.graphs, s.graph_execs, s.engines, s.contexts, s.builds]
    }

    /// The stream work done so far: h2d, d2h, d2d, memsets, enqueues, graph launches.
    var work: [UInt64] {
      let s = stats
      return [s.h2d, s.d2h, s.d2d, s.memsets, s.enqueues, s.graph_launches]
    }

    func fail(_ call: String, nth: Int = 1, code: Int32, _ message: String = "injected") {
      jl_trt_fake_fail(handle, call, Int32(nth), code, message)
    }

    func setBuild(_ lines: String, layers: Int = 3) {
      jl_trt_fake_set_build(handle, lines, Int32(layers))
    }
  }

  /// A text plan the fake loads.
  func fakePlan(_ lines: String, built: String? = nil) -> String {
    "jl_trt_fake_plan 1\n" + (built.map { "built \($0)\n" } ?? "") + lines
  }

  /// The fake's own model: y = x + state, next_state = state + 1.
  let defaultLines = """
    input x float16 1 8
    input state float16 1 8
    output y float32 1 8
    output next_state float16 1 8 from state

    """

  extension TrtEngine {
    func setInput(_ name: String, _ values: [Float]) {
      let spec = inputs[name]!
      let buffer = hostInput(name)!
      for (i, v) in values.enumerated() {
        switch spec.type {
        case .float16: buffer.storeBytes(of: Float16(v).bitPattern, toByteOffset: i * 2, as: UInt16.self)
        default: buffer.storeBytes(of: v, toByteOffset: i * 4, as: Float.self)
        }
      }
    }

    func floats(_ name: String) -> [Float] {
      let spec = outputs[name]!
      let buffer = output(name)!
      return (0..<spec.count).map { i in
        spec.type == .float16
          ? Float(Float16(bitPattern: buffer.load(fromByteOffset: i * 2, as: UInt16.self))) : buffer.load(fromByteOffset: i * 4, as: Float.self)
      }
    }
  }
#endif
