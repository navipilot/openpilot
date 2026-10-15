import CTrt
import Foundation
import Testing

@testable import JetlinkTRT

@Suite("TensorRT shim")
struct ShimTests {
  #if JL_TRT_FAKE
    @Test("A binary built on the fake never finds TensorRT, and says so")
    func fakeIsUnavailable() {
      for device in [0, -1] {
        #expect {
          try TensorRT(device: device)
        } throws: {
          ($0 as? TrtError)?.code == Int32(JL_TRT_UNAVAILABLE)
        }
      }
    }

    @Test("A TensorRT library directory reaches jl_trt_open, and the loaded library is reported")
    func libraryDirectory() throws {
      #expect {
        try TensorRT(device: 0, libraries: "/opt/jetlink/tensorrt/11.3.0.99")
      } throws: {
        String(describing: $0).hasSuffix("asked for it in /opt/jetlink/tensorrt/11.3.0.99")
      }
      #expect {
        try TensorRT(device: 0)
      } throws: {
        !String(describing: $0).contains("asked for it")
      }
      #expect(try fakeTensorRT().library == "the fake shim")
    }
  #endif
}
