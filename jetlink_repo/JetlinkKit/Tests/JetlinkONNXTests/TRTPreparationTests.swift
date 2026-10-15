import Foundation
import JetlinkTestSupport
import Testing

@testable import JetlinkONNX

/// The `.plain` layout held to Python's `onnx_patch.patch_file`, the call the
/// TensorRT build made: trt.json has the counts or the error Python gave for
/// each graph, and `<graph>.trt.model.expected.onnx` the file it wrote, made
/// once before the Python server went. The files must match byte for byte.
@Suite struct TRTPreparationTests {
  static let python = PreparationTests.results("trt.json")

  static let cases: [String] = python.keys.map { String($0.split(separator: ".")[0]) }.sorted()

  @Test func everyGraphHasAResult() {
    #expect(Self.cases.count == 18)
    #expect(Self.python.values.filter { $0.error != nil }.count == 6)
  }

  @Test(arguments: cases)
  func matchesPython(_ fixture: String) throws {
    let expected = try #require(Self.python["\(fixture).trt"])
    let out = try TemporaryDirectory()
    let prepare = {
      try CoreMLPreparation.prepare(source: Fixtures.url("\(fixture).onnx"), into: out.url, layout: .plain, cacheKey: { _ in "unused" })
    }
    if let message = expected.error {
      #expect(throws: OnnxError(message)) { try prepare() }
      #expect(try FileManager.default.contentsOfDirectory(atPath: out.url.path).isEmpty)
      return
    }
    let report = try prepare()
    #expect(report.stripped == expected.stripped)
    #expect(report.retypedImages == expected.retypedImages)
    #expect((report.gathers, report.gemms, report.tiles, report.norms, report.heads) == (0, 0, 0, 0, 0))
    #expect(report.parts.map(\.name) == ["model"])
    let mine = try Data(contentsOf: report.parts[0].url)
    let theirs = try Data(contentsOf: Fixtures.url("\(fixture).trt.model.expected.onnx"))
    #expect(mine == theirs, "\(fixture): \(mine.count) bytes against Python's \(theirs.count)")
  }

  /// No COREML_CACHE_KEY, and the model's own props as they were, the
  /// export's CACHE_KEY included.
  @Test func keepsTheModelsProps() throws {
    let out = try TemporaryDirectory()
    let report = try CoreMLPreparation.prepare(source: Fixtures.url("queued.onnx"), into: out.url, layout: .plain, cacheKey: { $0 })
    let meta = try OnnxMeta.read(contentsOf: report.parts[0].url)
    let source = try OnnxMeta.read(contentsOf: Fixtures.url("queued.onnx"))
    #expect(meta.props == source.props)
    #expect(meta.props.map(\.key) == ["output_slices", "model_checkpoint", "CACHE_KEY"])
    #expect(meta.inputs.first { $0.name == "img" }?.elemType == 10)
  }
}
