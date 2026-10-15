import Foundation
import Synchronization

#if canImport(CryptoKit)
  import CryptoKit
#else
  import Crypto
#endif
#if canImport(FoundationNetworking)
  import FoundationNetworking
#endif

extension LFS {
  /// Streams one LFS object to `<dest>.part`, hashing as it arrives, and only
  /// then gives it the name. A half-written model must never sit where the
  /// next start would hand it to a backend to build from.
  ///
  /// The body goes from URLSession's delegate callbacks straight into a file
  /// handle and the hash, a callback's worth at a time, so a 766 MB model is
  /// never in memory. The .part is removed on any failure.
  static func download(
    href: String, pointer: Pointer, dest: URL, session: URLSession,
    progress: @escaping @Sendable (Double) -> Void,
    shouldStop: @escaping @Sendable () -> Bool
  ) async throws(RegistryError) -> URL {
    let directory = dest.deletingLastPathComponent()
    try? Files.makeDirectory(directory)
    guard let free = Files.freeBytes(at: directory) else {
      throw .registry("could not tell how much space is free in \(directory.path(percentEncoded: false))")
    }
    if free < pointer.size + freeSlack {
      throw .registry("need \(pointer.size >> 20) MB for the model, \(free >> 20) MB free")
    }
    let short = String(pointer.oid.prefix(16))
    guard let url = URL(string: href) else {
      throw .network("could not download \(short): \(href) is not a URL")
    }

    let part = dest.deletingLastPathComponent().appending(path: dest.lastPathComponent + ".part")
    let outcome: (written: Int64, digest: String)
    do {
      guard FileManager.default.createFile(atPath: part.path(percentEncoded: false), contents: nil),
        let handle = FileHandle(forWritingAtPath: part.path(percentEncoded: false))
      else {
        throw RegistryError.network("could not download \(short): cannot write \(part.path(percentEncoded: false))")
      }
      var request = URLRequest(url: url)
      request.timeoutInterval = connectTimeout
      let transfer = Transfer(pointer: pointer, handle: handle, progress: progress, shouldStop: shouldStop)
      outcome = try await transfer.run(request, session: session).get()
    } catch {
      try? Files.removeFile(part)
      throw error as? RegistryError ?? .network("could not download \(short): \(error)")
    }

    if outcome.written != pointer.size {
      try? Files.removeFile(part)
      throw .verify("\(short) is \(outcome.written) bytes, expected \(pointer.size)")
    }
    if outcome.digest != pointer.oid {
      try? Files.removeFile(part)
      throw .verify("downloaded bytes hash to \(outcome.digest.prefix(16)), expected \(short)")
    }
    do {
      try Files.replace(part, with: dest)
    } catch {
      try? Files.removeFile(part)
      throw .network("could not download \(short): \(error)")
    }
    progress(1.0)
    return dest
  }
}

/// One download's delegate. URLSession calls it on the session's serial
/// delegate queue; the lock is for the cancellation handler, which runs
/// wherever the cancelling task is.
private final class Transfer: NSObject, URLSessionDataDelegate, Sendable {
  private struct State {
    var handle: FileHandle?
    var hasher = SHA256()
    var written: Int64 = 0
    var reported = -1
    var failure: RegistryError?
    var continuation: CheckedContinuation<Void, Never>?
  }

  private let state: Mutex<State>
  private let pointer: Pointer
  private let progress: @Sendable (Double) -> Void
  private let shouldStop: @Sendable () -> Bool
  private var short: String { String(pointer.oid.prefix(16)) }

  init(pointer: Pointer, handle: FileHandle, progress: @escaping @Sendable (Double) -> Void, shouldStop: @escaping @Sendable () -> Bool) {
    self.pointer = pointer
    self.progress = progress
    self.shouldStop = shouldStop
    self.state = Mutex(State(handle: handle))
  }

  func run(_ request: URLRequest, session: URLSession) async -> Result<(written: Int64, digest: String), RegistryError> {
    // A session of its own with this as the session's delegate:
    // swift-corelibs-foundation never calls a task's own delegate (a download
    // sat at 0 bytes on Android), and a model is one transfer anyway.
    let own = URLSession(configuration: session.configuration, delegate: self, delegateQueue: nil)
    defer { own.finishTasksAndInvalidate() }
    let task = own.dataTask(with: request)
    await withTaskCancellationHandler {
      await withCheckedContinuation { continuation in
        state.withLock { $0.continuation = continuation }
        task.resume()
      }
    } onCancel: {
      fail(.cancelled("download cancelled"), task)
    }
    return state.withLock { state in
      if let failure = state.failure { return .failure(failure) }
      let digest = state.hasher.finalize().map { String(format: "%02x", $0) }.joined()
      return .success((state.written, digest))
    }
  }

  /// Keeps the first failure, which is the one that explains the rest, and
  /// stops the transfer.
  private func fail(_ error: RegistryError, _ task: URLSessionTask) {
    state.withLock { state in
      if state.failure == nil { state.failure = error }
    }
    task.cancel()
  }

  func urlSession(
    _ session: URLSession, dataTask: URLSessionDataTask, didReceive response: URLResponse,
    completionHandler: @escaping @Sendable (URLSession.ResponseDisposition) -> Void
  ) {
    // The href is left out of the message, as Python's is: it is a signed
    // URL, and noise in a message about a model.
    if let failure = HTTP.statusFailure(response) {
      state.withLock { if $0.failure == nil { $0.failure = .network("could not download \(short): \(failure.text)") } }
      completionHandler(.cancel)
      return
    }
    // Python asks before every read, the first included.
    if shouldStop() {
      state.withLock { if $0.failure == nil { $0.failure = .cancelled("download cancelled") } }
      completionHandler(.cancel)
      return
    }
    completionHandler(.allow)
  }

  func urlSession(_ session: URLSession, dataTask: URLSessionDataTask, didReceive data: Data) {
    if state.withLock({ $0.failure != nil }) { return }
    if shouldStop() {
      fail(.cancelled("download cancelled"), dataTask)
      return
    }
    // Whole percent only: a gigabyte arrives in thousands of callbacks, and
    // the callback may write a param or a socket line.
    let report: Double? = state.withLock { state in
      do {
        try state.handle?.write(contentsOf: data)
      } catch {
        state.failure = .network("could not download \(short): \(error.localizedDescription)")
        return nil
      }
      state.hasher.update(data: data)
      state.written += Int64(data.count)
      guard pointer.size > 0 else { return nil }
      let fraction = Double(state.written) / Double(pointer.size)
      let percent = Int(Double(100 * state.written) / Double(pointer.size))
      guard percent != state.reported else { return nil }
      state.reported = percent
      return min(1.0, fraction)
    }
    if state.withLock({ $0.failure != nil }) {
      dataTask.cancel()
      return
    }
    if let report { progress(report) }
  }

  func urlSession(_ session: URLSession, task: URLSessionTask, didCompleteWithError error: (any Error)?) {
    // Python asks once more before the read that finds the end.
    let stopped = error == nil && shouldStop()
    let continuation = state.withLock { state -> CheckedContinuation<Void, Never>? in
      try? state.handle?.close()
      state.handle = nil
      if state.failure == nil {
        if let error {
          state.failure = .network("could not download \(short): \(HTTP.describe(error))")
        } else if stopped {
          state.failure = .cancelled("download cancelled")
        }
      }
      defer { state.continuation = nil }
      return state.continuation
    }
    continuation?.resume()
  }
}
