#if os(Android)
  import Foundation
  import Testing

  @testable import JetlinkORT

  /// The hint API is found at run time; this checks the calls' shapes where
  /// the device has it. An emulator's power HAL may not, and then there is
  /// nothing to call.
  struct PerformanceHintTests {
    @Test("Hint sessions open, take reports from two threads, and close")
    func reports() throws {
      guard let hint = PerformanceHint.make() else { return }
      for _ in 0..<5 { hint.report(12_000_000) }
      let other = Thread { hint.report(8_000_000) }
      other.start()
      Thread.sleep(forTimeInterval: 0.2)
      hint.report(12_000_000)
      hint.close()
      hint.close()
    }
  }
#endif
