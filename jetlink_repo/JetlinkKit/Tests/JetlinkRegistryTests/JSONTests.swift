import Foundation
import Testing

@testable import JetlinkRegistry

/// The JSON the state files go through behaves as Python's json module.
struct JSONTests {
  private func parse(_ text: String) throws -> JSON {
    try JSON.parse(Data(text.utf8))
  }

  @Test func keepsKeyOrderAsADictDoes() throws {
    let value = try parse(#"{"z": 1, "a": 2, "m": {"y": [1, 2.5, "x"], "b": null}}"#)
    #expect(value.object?.keys == ["z", "a", "m"])
    #expect(value["m"]?.object?.keys == ["y", "b"])
    #expect(value.serialized() == #"{"z": 1, "a": 2, "m": {"y": [1, 2.5, "x"], "b": null}}"#)
  }

  @Test func aRepeatedKeyTakesTheLastValueInTheFirstPlace() throws {
    let value = try parse(#"{"a": 1, "b": 2, "a": 3}"#)
    #expect(value.object?.keys == ["a", "b"])
    #expect(value["a"] == 3)
  }

  @Test func settingAKeyKeepsItsPlace() {
    var object: JSONObject = ["a": 1, "b": 2]
    object["a"] = 5
    object["c"] = 3
    #expect(object.keys == ["a", "b", "c"])
    object["a"] = nil
    #expect(object.keys == ["b", "c"])
    #expect(object == ["c": 3, "b": 2], "equality ignores order")
  }

  @Test func integersStayIntegers() throws {
    #expect(try parse("765953504") == .int(765_953_504))
    #expect(try parse("1.0") == .double(1.0))
    #expect(try parse("-2e3") == .double(-2000))
    #expect(try parse("123456789012345678901234567890") == .double(123456789012345678901234567890.0))
    #expect(JSON.double(1757440000.25).serialized() == "1757440000.25")
    #expect(JSON.double(548.9).serialized() == "548.9")
  }

  @Test func stringsEscapeAsDumpsDoes() throws {
    // Python writes non-ASCII as UTF-16 escapes; built from pieces so no editor folds them back
    let u = "\\u"
    let escaped = "\"a\\\"b\\\\c\\/d\\n" + u + "00e9" + u + "d83d" + u + "de97\""
    #expect(try parse(escaped) == .string("a\"b\\c/d\n\u{E9}\u{1F697}"))
    #expect(JSON.string("\u{E9}\u{1F697}\t\"\u{01}").serialized() == "\"" + u + "00e9" + u + "d83d" + u + "de97\\t\\\"" + u + "0001\"")
  }

  @Test(arguments: ["", "{", "[1,]", "{\"a\" 1}", "\"\u{01}\"", "tru", "1 2", "{'a': 1}", "[01]"])
  func refusesWhatJsonLoadsRefuses(text: String) {
    #expect(throws: JSONParseError.self) { try JSON.parse(Data(text.utf8)) }
  }

  @Test func refusesInvalidUTF8() {
    #expect(throws: JSONParseError.self) { try JSON.parse(Data([0x22, 0xFF, 0x22])) }
  }

  @Test func pythonCoercions() {
    #expect(JSON.string(" 19 ").pythonInt == 19)
    #expect(JSON.string("1_9").pythonInt == 19)
    #expect(JSON.string("19.0").pythonInt == nil)
    #expect(JSON.string("x").pythonInt == nil)
    #expect(JSON.double(19.7).pythonInt == 19)
    #expect(JSON.bool(true).pythonInt == 1)
    #expect(JSON.null.pythonInt == nil)
    #expect(JSON.string("").truthy == false && JSON.int(0).truthy == false && JSON.array([]).truthy == false)
    #expect(JSON.string("0").truthy == true)
    #expect(JSON.pythonRepr("nothex") == "'nothex'")
    #expect(JSON.pythonRepr("it's") == "\"it's\"")
  }
}
