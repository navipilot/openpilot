import Foundation

/// Released once, waited on by any number of threads: the accept loop and
/// the dial loop can both wait for the same session to end.
final class Latch: @unchecked Sendable {
  private let condition = NSCondition()
  private var released = false

  func release() {
    condition.lock()
    released = true
    condition.broadcast()
    condition.unlock()
  }

  func wait() {
    condition.lock()
    while !released {
      condition.wait()
    }
    condition.unlock()
  }
}

/// A value behind a lock that escaping closures can share, which Mutex,
/// being noncopyable, cannot be.
package final class Locked<Value>: @unchecked Sendable {
  private let lock = NSLock()
  private var stored: Value

  package init(_ value: Value) {
    stored = value
  }

  package func withLock<R>(_ body: (inout Value) throws -> R) rethrows -> R {
    lock.lock()
    defer { lock.unlock() }
    return try body(&stored)
  }

  package var value: Value {
    get { withLock { $0 } }
    set { withLock { $0 = newValue } }
  }
}

/// Waits on the calling thread for async work, for a synchronous caller: a
/// command run on the main thread, a JNI call from a Kotlin worker.
package func blocking<T: Sendable>(_ work: @escaping @Sendable () async throws -> T) throws -> T {
  let result = Locked<Result<T, any Error>?>(nil)
  let done = DispatchSemaphore(value: 0)
  Task.detached {
    let outcome: Result<T, any Error>
    do {
      outcome = .success(try await work())
    } catch {
      outcome = .failure(error)
    }
    result.withLock { $0 = outcome }
    done.signal()
  }
  done.wait()
  return try result.withLock { $0! }.get()
}

/// `blocking` for a caller that cannot wait for ever, a page's request: nil
/// once `timeout` passes, while the work goes on.
package func blocking<T: Sendable>(timeout: TimeInterval, _ work: @escaping @Sendable () async -> T) -> T? {
  let result = Locked<T?>(nil)
  let done = DispatchSemaphore(value: 0)
  Task.detached {
    let value = await work()
    result.value = value
    done.signal()
  }
  return done.wait(timeout: .now() + timeout) == .success ? result.value : nil
}
