import Foundation
import Observation

/// The server's output, as a Logs view sees it. The Mac fills it from the
/// Python server's stderr, the iPhone from the in-process server's log sink.
@MainActor
@Observable
public final class LogBuffer {
  public static let capacity = 5000
  public static let trimChunk = 500

  public private(set) var lines: [String] = []
  /// One integer a view can observe instead of the whole array.
  public private(set) var revision: Int = 0

  public init() {}

  public func append(_ line: String) {
    lines.append(line)
    if lines.count > LogBuffer.capacity {
      lines.removeFirst(LogBuffer.trimChunk)
    }
    revision += 1
  }

  public func clear() {
    lines.removeAll(keepingCapacity: true)
    revision += 1
  }

  /// The last `count` lines, for a crash report in the Status view.
  public func tail(_ count: Int) -> [String] {
    Array(lines.suffix(count))
  }

  public static func preview(lines: [String]) -> LogBuffer {
    let buffer = LogBuffer()
    for line in lines { buffer.append(line) }
    return buffer
  }
}
