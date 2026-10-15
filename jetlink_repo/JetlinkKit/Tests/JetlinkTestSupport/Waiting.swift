import Foundation

/// Whether `condition` holds within `timeout`, looking every 5 ms.
public func eventually(timeout: TimeInterval = 2, _ condition: () -> Bool) -> Bool {
  let deadline = Date().addingTimeInterval(timeout)
  while !condition() {
    if Date() >= deadline { return false }
    Thread.sleep(forTimeInterval: 0.005)
  }
  return true
}

/// Values appended from any thread, and a wait for them.
public final class Recorded<T>: @unchecked Sendable {
  private let condition = NSCondition()
  private var values: [T] = []

  public init() {}

  public func append(_ value: T) {
    condition.lock()
    values.append(value)
    condition.broadcast()
    condition.unlock()
  }

  public var all: [T] {
    condition.lock()
    defer { condition.unlock() }
    return values
  }

  /// Whether the values come to satisfy `done` within `timeout`. Each append
  /// wakes it, so the bound is only for a test that fails: a loaded CI
  /// runner can take seconds to get a thread to the event.
  public func wait(timeout: TimeInterval = 10, until done: ([T]) -> Bool) -> Bool {
    let deadline = Date().addingTimeInterval(timeout)
    condition.lock()
    defer { condition.unlock() }
    while !done(values) {
      if !condition.wait(until: deadline) { return done(values) }
    }
    return true
  }
}

/// A test's own failure, with what it saw.
public struct TestError: Error, CustomStringConvertible {
  public let description: String

  public init(_ description: String) {
    self.description = description
  }
}
