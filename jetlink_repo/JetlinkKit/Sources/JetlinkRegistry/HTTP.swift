import Foundation

#if canImport(FoundationNetworking)
  import FoundationNetworking
#endif

/// The registry's small requests: a catalog, a pointer, a patch head, a tree
/// listing. Each failure is a RegistryError, and a 404 is `notFound` so a
/// caller can tell a missing file from an outage.
struct HTTP: Sendable {
  let session: URLSession

  enum Shape: Sendable {
    case object, array

    var pythonName: String { self == .object ? "dict" : "list" }
  }

  /// The body at `url`, at most `limit` bytes. Past the limit the request is
  /// cancelled rather than read: a pointer is 134 bytes, and a host that
  /// resolved the LFS filter for us would otherwise send the whole model.
  func get(_ url: String, timeout: TimeInterval, limit: Int? = nil) async throws(RegistryError) -> Data {
    guard let target = URL(string: url), target.scheme != nil else {
      throw .network("could not fetch \(url): unknown url type")
    }
    var request = URLRequest(url: target)
    // URLSession's request timeout is an idle timeout, as a socket timeout is
    // for urllib: a slow body that keeps arriving is not cut off.
    request.timeoutInterval = timeout
    do {
      guard let limit else {
        let (data, response) = try await session.data(for: request)
        try HTTP.check(response, url: url)
        return data
      }
      #if canImport(FoundationNetworking)
        // swift-corelibs-foundation has no bytes(for:), so it reads the whole
        // body and keeps the first `limit` bytes. Only small reads pass a
        // limit; a model downloads through Transfer, not here.
        let (whole, response) = try await session.data(for: request)
        try HTTP.check(response, url: url)
        return Data(whole.prefix(max(0, limit)))
      #else
        let (bytes, response) = try await session.bytes(for: request)
        defer { bytes.task.cancel() }
        try HTTP.check(response, url: url)
        var data = Data()
        data.reserveCapacity(min(limit, 64 << 10))
        if limit > 0 {
          for try await byte in bytes {
            data.append(byte)
            if data.count >= limit { break }
          }
        }
        return data
      #endif
    } catch let error as RegistryError {
      throw error
    } catch {
      throw .network("could not fetch \(url): \(HTTP.describe(error))")
    }
  }

  /// `get`, parsed, and of the shape asked for. A body that is not JSON is a
  /// network error: the server did not answer sensibly.
  func getJSON(_ url: String, timeout: TimeInterval, shape: Shape = .object) async throws(RegistryError) -> JSON {
    let body = try await get(url, timeout: timeout)
    let value: JSON
    do {
      value = try JSON.parse(body)
    } catch {
      throw .network("\(url) did not serve JSON: \(error)")
    }
    switch (shape, value) {
    case (.object, .object), (.array, .array): return value
    default: throw .network("\(url) did not serve a JSON \(shape.pythonName)")
    }
  }

  /// urllib raises for anything but a 2xx once redirects are followed, which
  /// URLSession has also done by now.
  static func check(_ response: URLResponse, url: String) throws(RegistryError) {
    guard let failure = statusFailure(response) else { return }
    let message = "could not fetch \(url): \(failure.text)"
    throw failure.status == 404 ? .notFound(message) : .network(message)
  }

  /// A status urllib would raise HTTPError for, as its text reads:
  /// "HTTP Error 404: not found". Nil for a 2xx or a response that is not HTTP.
  static func statusFailure(_ response: URLResponse) -> (status: Int, text: String)? {
    guard let http = response as? HTTPURLResponse, !(200..<300).contains(http.statusCode) else { return nil }
    return (http.statusCode, "HTTP Error \(http.statusCode): \(HTTPURLResponse.localizedString(forStatusCode: http.statusCode))")
  }

  static func describe(_ error: any Error) -> String {
    if let error = error as? URLError {
      return "\(error.localizedDescription) (URLError \(error.code.rawValue))"
    }
    return String(describing: error)
  }
}
