#if canImport(SwiftUI)
  import SwiftUI

  /// A log line's colour in either app's Logs view: errors red, warnings orange.
  /// Both servers write "time LEVEL name: message", the Python one padding the
  /// level to seven characters, so a warning is " WARNING" and an error " ERROR ".
  public enum LogTone {
    public static func color(for line: String) -> Color {
      if line.contains(" ERROR ") { return .red }
      if line.contains(" WARNING") { return .orange }
      return .primary
    }
  }
#endif
