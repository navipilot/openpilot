import Foundation

/// sunnypilot's big-model catalog: which large models exist and which commit
/// each one is.
///
/// The catalog is the same JSON the comma's model manager caches for its
/// chestnut slot, so a model picked here is a model the comma can ask for. A
/// bundle's artifacts are tinygrad pkls for a GPU we do not have; the only
/// field that matters is `ref`, the comma openpilot commit the bundle was
/// compiled from, because that commit's ONNX is what a jetlink server runs.
public enum Catalog {
  public static let urlTemplate =
    "https://raw.githubusercontent.com/sunnypilot/sunnypilot-models/refs/heads/gh-pages/docs/driving_models_chestnut_v{version}.json"
  /// v26 is v25 plus Cinque Terre V3, at the same selector version. v27
  /// onwards are selector 20 and a newer tinygrad, for sunnypilot's next sync.
  public static let version = 26
  public static let url = url(version: version)
  /// sunnypilot publishes a new catalog version for new models or a new
  /// runtime, and keeps the old ones. The versions after the pinned one are
  /// probed up to the first that is not there, so a model published later is
  /// listed without a release of this package. See `merge`.
  public static let probeLimit = 10
  /// zoompilot's own big models: ones sunnypilot does not list (yet), such as
  /// previews from comma's open pull requests, in the catalog's shape. Merged
  /// after sunnypilot's catalogs, so their entry for the same commit wins. An
  /// edit on main reaches every server within the hour, with no release.
  public static let extraURL = "https://raw.githubusercontent.com/zoompilot/jetlink/refs/heads/main/catalog/extra_big_models.json"
  /// The selector version the fork requires (REQUIRED_JSON_VERSION on the
  /// comma). It is a string in the JSON; bundles at any other version describe
  /// fields we would misread.
  public static let requiredSelectorVersion = 19
  /// Cinque Terre V3 Model (September 17, 2026).
  public static let defaultBigModelRef = "bf3e3631b3f91d92a1020a5e0dd4298b93ff4244"
  public static let timeout: TimeInterval = 10
  /// How long a fetched catalog is served before `catalog()` fetches again.
  public static let maxAge: TimeInterval = 3600

  public static func url(version: Int) -> String {
    urlTemplate.replacingOccurrences(of: "{version}", with: String(version))
  }

  /// The version a catalog URL names, or nil for one not in sunnypilot's scheme.
  package static func version(of url: String) -> Int? {
    guard url.hasSuffix(".json") else { return nil }
    let stem = url.dropLast(".json".count)
    let digits = stem.reversed().prefix { $0.isASCII && $0.isNumber }
    guard !digits.isEmpty, stem.dropLast(digits.count).hasSuffix("_v") else { return nil }
    return Int(String(digits.reversed()))
  }

  /// One big-model bundle, before its pointer is known.
  struct Entry: Sendable, Equatable {
    let name: String
    let shortName: String
    let ref: String
    let buildTime: String
    let index: Int
  }

  /// The big-model bundles, newest first. No IO, and no bundle can fail the
  /// rest: a malformed entry in a catalog served to every comma costs that
  /// entry and nothing else.
  static func parse(_ data: JSON) -> [Entry] {
    var found: [Entry] = []
    var seen = Set<String>()
    for bundle in bundles(data) {
      guard let ref = bundle["ref"]?.string, CacheLayout.isRef(ref), !seen.contains(ref) else { continue }
      guard selector(bundle) == requiredSelectorVersion, bundle["is_big"]?.truthy == true else { continue }
      // int(bundle.get('index', 0)): a missing index is 0, one that int()
      // refuses drops the bundle.
      let raw: Int64? = bundle.contains("index") ? bundle["index"]?.pythonInt : 0
      guard let raw, let index = Int(exactly: raw) else { continue }
      seen.insert(ref)
      found.append(
        Entry(
          name: text(bundle["display_name"]) ?? String(ref.prefix(10)),
          shortName: text(bundle["short_name"]) ?? "",
          ref: ref,
          buildTime: text(bundle["build_time"]) ?? "",
          index: index))
    }
    // sorted(key=index, reverse=True) is stable: equal indexes keep catalog order.
    return found.enumerated()
      .sorted { $0.element.index != $1.element.index ? $0.element.index > $1.element.index : $0.offset < $1.offset }
      .map(\.element)
  }

  /// One catalog in the first one's shape, listing every big model of them all.
  ///
  /// A model some catalog lists at `selector` comes through as published, so
  /// a chestnut can still fetch its build. One listed only at another selector
  /// version, which is where sunnypilot puts every model once it moves
  /// runtimes, comes from the newest catalog that has it, retyped to
  /// `selector` and with no artifacts: an accelerator runs the commit's ONNX
  /// and needs nothing else from the entry, and a chestnut has nothing to
  /// download.
  static func merge(_ catalogs: [JSONObject], selector: Int = requiredSelectorVersion) -> JSONObject {
    guard let first = catalogs.first else { return JSONObject() }
    var kept = JSONObject()
    var others = JSONObject()
    for data in catalogs {
      for bundle in bundles(.object(data)) {
        guard let ref = bundle["ref"]?.string, CacheLayout.isRef(ref) else { continue }
        if self.selector(bundle) == selector {
          if !kept.contains(ref) { kept[ref] = .object(bundle) }
        } else if bundle["is_big"]?.truthy == true {
          others[ref] = .object(bundle)  // the newest catalog's entry wins, in the first one's place
        }
      }
    }
    var merged: [JSON] = kept.map(\.value)
    for (ref, bundle) in others where !kept.contains(ref) {
      guard var retyped = bundle.object else { continue }
      retyped["minimum_selector_version"] = .string(String(selector))
      retyped["models"] = .array([])
      merged.append(.object(retyped))
    }
    var out = first
    out["bundles"] = .array(merged)
    return out
  }

  static func bundles(_ data: JSON) -> [JSONObject] {
    (data["bundles"]?.array ?? []).compactMap(\.object)
  }

  /// `int(bundle.get('minimum_selector_version', 0))`, nil where int() refuses.
  static func selector(_ bundle: JSONObject) -> Int? {
    guard let value = bundle["minimum_selector_version"] else { return 0 }
    return value.pythonInt.flatMap { Int(exactly: $0) }
  }

  /// `str(value or '')`, with nil standing for the empty fallback.
  private static func text(_ value: JSON?) -> String? {
    guard let value, value.truthy else { return nil }
    return value.pythonString
  }
}
