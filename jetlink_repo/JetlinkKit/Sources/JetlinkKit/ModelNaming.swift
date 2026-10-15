import Foundation

extension ModelRow {
  /// The name without the trailing parenthesised build date the catalog carries,
  /// so "BMRLNAP Model v4 (August 30, 2026)" reads as "BMRLNAP Model v4". A
  /// parenthesis that is not a date is part of the name and stays.
  public var displayName: String {
    guard let date = name.range(of: ModelRow.trailingDatePattern, options: .regularExpression) else { return name }
    return String(name[name.startIndex..<date.lowerBound])
  }

  public static let trailingDatePattern = " \\([A-Za-z]+ \\d{1,2}, \\d{4}\\)$"
}

/// Build times as the catalog reports them, ISO-8601 in UTC.
public enum BuildTime {
  public static func date(_ text: String?) -> Date? {
    guard let text, !text.isEmpty else { return nil }
    let withFraction = ISO8601DateFormatter()
    withFraction.formatOptions = [.withInternetDateTime, .withFractionalSeconds]
    if let date = withFraction.date(from: text) { return date }
    let plain = ISO8601DateFormatter()
    plain.formatOptions = [.withInternetDateTime]
    return plain.date(from: text)
  }

  /// An abbreviated date, or nothing at all when the build time is unknown.
  public static func text(_ text: String?) -> String {
    guard let date = date(text) else { return "" }
    return date.formatted(Date.FormatStyle(date: .abbreviated, time: .omitted))
  }
}
