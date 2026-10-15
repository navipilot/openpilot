import Foundation
import Testing

@testable import JetlinkONNX

@Suite struct OutputSlicesTests {
  /// output_slices as the three real models carry it (Cinque Terre V3
  /// 404a18cfd86d2963 and the queued 09d080f36965bb2a and a086d5249fc308bb):
  /// the three strings are identical, byte for byte. A protocol 4 pickle.
  static let real = """
    gASVkwEAAAAAAAB9lCiMCmxhbmVfbGluZXOUjAhidWlsdGluc5SMBXNsaWNllJOUSwBNEAJOh5RS
    lIwPbGFuZV9saW5lc19wcm9ilGgETRACTRgCToeUUpSMCnJvYWRfZWRnZXOUaARNGAJNIANOh5RS
    lIwEbWV0YZRoBE0gA01XA06HlFKUjAtkZXNpcmVfcHJlZJRoBE1XA013A06HlFKUjARwb3NllGgE
    TXcDTYMDToeUUpSMFndpZGVfZnJvbV9kZXZpY2VfZXVsZXKUaARNgwNNiQNOh5RSlIwOcm9hZF90
    cmFuc2Zvcm2UaARNiQNNlQNOh5RSlIwEcGxhbpRoBE2VA01zB06HlFKUjARsZWFklGgETXMHTQMI
    ToeUUpSMCWxlYWRfcHJvYpRoBE0DCE0GCE6HlFKUjAxkZXNpcmVfc3RhdGWUaARNBghNDghOh5RS
    lIwGYWN0aW9ulGgETQ4ITRIIToeUUpSMDGhpZGRlbl9zdGF0ZZRoBE0SCE0SSE6HlFKUjANwYWSU
    aARNEkhNFEhOh5RSlHUu

    """

  @Test func realModels() throws {
    let slices = try OutputSlices.decode(base64: Self.real)
    let expected: [(String, Int, Int)] = [
      ("lane_lines", 0, 528), ("lane_lines_prob", 528, 536), ("road_edges", 536, 800), ("meta", 800, 855),
      ("desire_pred", 855, 887), ("pose", 887, 899), ("wide_from_device_euler", 899, 905),
      ("road_transform", 905, 917), ("plan", 917, 1907), ("lead", 1907, 2051), ("lead_prob", 2051, 2054),
      ("desire_state", 2054, 2062), ("action", 2062, 2066), ("hidden_state", 2066, 18450), ("pad", 18450, 18452),
    ]
    #expect(slices == expected.map { OutputSlice(name: $0.0, start: $0.1, stop: $0.2) })
  }

  /// Lebowski's pad is `slice(-2, None)`: `pickle.dumps({'hidden_state':
  /// slice(2066, 18450), 'pad': slice(-2, None)}, protocol=4)`. And
  /// `slice(7)`, through TUPLE1, has no start.
  @Test func openAndNegativeEnds() throws {
    let slices = try OutputSlices.decode(
      base64: "gASVRwAAAAAAAAB9lCiMDGhpZGRlbl9zdGF0ZZSMCGJ1aWx0aW5zlIwFc2xpY2WUk5RNEghNEkhOh5RSlIwDcGFklGgESv7///9OToeUUpR1Lg==")
    #expect(slices == [OutputSlice(name: "hidden_state", start: 2066, stop: 18450), OutputSlice(name: "pad", start: -2, stop: nil)])
    #expect(try OutputSlices.decode(base64: "gAJ9WAEAAABiY19fYnVpbHRpbl9fCnNsaWNlCksHhVJzLg==") == [OutputSlice(name: "b", start: nil, stop: 7)])
  }

  /// `pickle.dumps({'lane_lines': slice(0, 528), 'wide': slice(200, 70000),
  /// 'far': slice(-3, 2**40), 'neg': slice(-100000, -5)}, protocol=p)`:
  /// BININT1, BININT2, BININT (negative too) and LONG1, and the memo:
  /// BINPUT/BINGET in 2 and 3, MEMOIZE/BINGET in 4 and 5; GLOBAL with
  /// __builtin__ in 2 and builtins in 3, STACK_GLOBAL and FRAME in 4 and 5.
  static let protocols: [Int: String] = [
    2:
      "gAJ9cQAoWAoAAABsYW5lX2xpbmVzcQFjX19idWlsdGluX18Kc2xpY2UKcQJLAE0QAk6HcQNScQRYBAAAAHdpZGVxBWgCS8hKcBEBAE6HcQZScQdYAwAAAGZhcnEIaAJK/f///4oGAAAAAAABTodxCVJxClgDAAAAbmVncQtoAkpgef7/Svv///9Oh3EMUnENdS4=",
    3:
      "gAN9cQAoWAoAAABsYW5lX2xpbmVzcQFjYnVpbHRpbnMKc2xpY2UKcQJLAE0QAk6HcQNScQRYBAAAAHdpZGVxBWgCS8hKcBEBAE6HcQZScQdYAwAAAGZhcnEIaAJK/f///4oGAAAAAAABTodxCVJxClgDAAAAbmVncQtoAkpgef7/Svv///9Oh3EMUnENdS4=",
    4:
      "gASVdwAAAAAAAAB9lCiMCmxhbmVfbGluZXOUjAhidWlsdGluc5SMBXNsaWNllJOUSwBNEAJOh5RSlIwEd2lkZZRoBEvISnARAQBOh5RSlIwDZmFylGgESv3///+KBgAAAAAAAU6HlFKUjANuZWeUaARKYHn+/0r7////ToeUUpR1Lg==",
    5:
      "gAWVdwAAAAAAAAB9lCiMCmxhbmVfbGluZXOUjAhidWlsdGluc5SMBXNsaWNllJOUSwBNEAJOh5RSlIwEd2lkZZRoBEvISnARAQBOh5RSlIwDZmFylGgESv3///+KBgAAAAAAAU6HlFKUjANuZWeUaARKYHn+/0r7////ToeUUpR1Lg==",
  ]

  @Test(arguments: [2, 3, 4, 5]) func protocolVersions(_ p: Int) throws {
    let slices = try OutputSlices.decode(base64: Self.protocols[p]!)
    #expect(
      slices == [
        OutputSlice(name: "lane_lines", start: 0, stop: 528),
        OutputSlice(name: "wide", start: 200, stop: 70_000),
        OutputSlice(name: "far", start: -3, stop: 1 << 40),
        OutputSlice(name: "neg", start: -100_000, stop: -5),
      ])
  }

  /// One item goes through SETITEM rather than SETITEMS.
  @Test func singleItem() throws {
    let slices = try OutputSlices.decode(base64: "gAJ9cQBYBAAAAG9ubHlxAWNfX2J1aWx0aW5fXwpzbGljZQpxAksBSwJOh3EDUnEEcy4=")
    #expect(slices == [OutputSlice(name: "only", start: 1, stop: 2)])
  }

  /// Written by hand, since Python only writes these opcodes for huge
  /// objects: BINUNICODE8.
  @Test func binunicode8() throws {
    let slices = try OutputSlices.decode(base64: "gAR9KI0BAAAAAAAAAGGMCGJ1aWx0aW5zjAVzbGljZZNLAEsBTodSdS4=")
    #expect(slices == [OutputSlice(name: "a", start: 0, stop: 1)])
  }

  /// By hand: LONG_BINPUT, LONG_BINGET, a MARK-delimited TUPLE and TUPLE2.
  @Test func longMemoAndTuples() throws {
    let slices = try OutputSlices.decode(
      base64: "gAJ9cgAAAAAoWAEAAABhY19fYnVpbHRpbl9fCnNsaWNlCnIBAAAAKEsASwV0UlgBAAAAY2oBAAAASwFLAoZSdS4=")
    #expect(slices == [OutputSlice(name: "a", start: 0, stop: 5), OutputSlice(name: "c", start: 1, stop: 2)])
  }

  @Test(arguments: [
    // protocol 0: its text opcodes, DICT first
    (
      "KGRwMApWbGFuZV9saW5lcwpwMQpjX19idWlsdGluX18Kc2xpY2UKcDIKKEkwCkk1MjgKTnRwMwpScDQKc1Z3aWRlCnA1CmcyCihJMjAwCkk3MDAwMApOdHA2ClJwNwpzVmZhcgpwOApnMgooSS0zCkwxMDk5NTExNjI3Nzc2TApOdHA5ClJwMTAKc1ZuZWcKcDExCmcyCihJLTEwMDAwMApJLTUKTnRwMTIKUnAxMwpzLg==",
      "unsupported opcode 0x64 ('d')"
    ),
    // a list, not a dict
    ("gASVIgAAAAAAAABdlIwIYnVpbHRpbnOUjAVzbGljZZSTlEsASwFOh5RSlGEu", "unsupported opcode 0x5d (']')"),
    // {'a': 1}
    ("gASVCgAAAAAAAAB9lIwBYZRLAXMu", "output_slices entry a is an int, not a slice"),
    // {'a': slice(0, 10, 2)}
    ("gASVJwAAAAAAAAB9lIwBYZSMCGJ1aWx0aW5zlIwFc2xpY2WUk5RLAEsKSwKHlFKUcy4=", "output_slices entry a has a step; expected None"),
    // collections.OrderedDict(a=slice(0, 1)): a REDUCE of anything but slice
    (
      "gASVRQAAAAAAAACMC2NvbGxlY3Rpb25zlIwLT3JkZXJlZERpY3SUk5QpUpSMAWGUjAhidWlsdGluc5SMBXNsaWNllJOUSwBLAU6HlFKUcy4=",
      "unsupported opcode 0x29 (')')"
    ),
    ("!!!", "output_slices is not valid base64"),
    // valid base64 of a pickle cut short
    ("gASVdwAAAAAAAAB9lCiMCmxhbmVf", "output_slices pickle: truncated"),
  ])
  func refused(_ base64: String, _ message: String) {
    do {
      _ = try OutputSlices.decode(base64: base64)
      Issue.record("decoded without an error")
    } catch let error as OnnxError {
      #expect(error.message.contains(message), "\(error.message)")
    } catch {
      Issue.record("unexpected \(error)")
    }
  }

  @Test func reduceOfAnotherGlobalIsRefused() throws {
    // {'a': collections.OrderedDict(1)} by hand: GLOBAL, TUPLE1, REDUCE. The
    // callable is only named, never called, so this is refused by name.
    let bytes = Data(
      [0x80, 0x02, 0x7d, 0x58, 0x01, 0, 0, 0, 0x61]
        + Array("ccollections\nOrderedDict\n".utf8) + [0x4b, 0x01, 0x85, 0x52, 0x73, 0x2e])
    let error = try #require(throws: OnnxError.self) { try OutputSlices.decode(pickle: bytes) }
    #expect(error.message.contains("REDUCE of the global collections.OrderedDict; only builtins.slice is allowed"))
  }
}
