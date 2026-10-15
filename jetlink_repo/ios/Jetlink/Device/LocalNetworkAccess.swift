import Foundation
import Network
import Observation

/// Whether iOS lets Jetlink talk to the local network, which is where the comma
/// is. Without it the comma's connection never arrives and nothing says why.
///
/// iOS asks the first time an app uses the local network. A short Bonjour
/// browse asks at launch, while someone is looking at the phone, rather than
/// when the comma first connects; its outcome also says whether access was
/// refused, which nothing else reports.
@MainActor
@Observable
final class LocalNetworkAccess {
  enum State: Equatable {
    case unknown, granted, denied
  }

  static let serviceType = "_jetlink._tcp"

  private(set) var state: State = .unknown
  @ObservationIgnored private var browser: NWBrowser?

  func check() {
    browser?.cancel()
    let browser = NWBrowser(for: .bonjour(type: LocalNetworkAccess.serviceType, domain: nil), using: .tcp)
    browser.stateUpdateHandler = { [weak self] update in
      Task { @MainActor in self?.apply(update) }
    }
    self.browser = browser
    browser.start(queue: .main)
    // A browse with nothing to find only needs to run long enough to be answered.
    Task { @MainActor [weak self] in
      try? await Task.sleep(for: .seconds(10))
      if self?.browser === browser {
        browser.cancel()
        self?.browser = nil
      }
    }
  }

  private func apply(_ update: NWBrowser.State) {
    switch update {
    case .ready:
      state = .granted
    case .waiting(let error), .failed(let error):
      if case .dns(let code) = error, code == DNSServiceErrorType(kDNSServiceErr_PolicyDenied) {
        state = .denied
      }
    default:
      break
    }
  }
}
