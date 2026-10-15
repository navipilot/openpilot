import Darwin
import Foundation
import JetlinkKit
import Network
import Observation

/// The phone's IPv4 addresses, and what each one is for.
///
/// Over one cable the comma is a USB network adapter to the phone, and hands
/// it a 192.168.60.x address by DHCP; the app dials the comma over it, and
/// nobody types anything. en0 is Wi-Fi, where bench tools on a Mac reach the
/// server, and bridge100 the Personal Hotspot. The cable comes first.
@MainActor
@Observable
final class NetworkInterfaces {
  /// The comma's network over the cable, and its own address on it.
  static let cableNetwork = LinkMedium.cableNetwork
  static let commaAddress = Pinned.cableAddress

  struct Address: Identifiable, Equatable, Sendable {
    enum Kind: Int, Comparable, Sendable {
      case cable = 0, hotspot, wifi, other

      static func < (a: Kind, b: Kind) -> Bool { a.rawValue < b.rawValue }

      var title: String {
        switch self {
        case .cable: "USB"
        case .hotspot: "Personal Hotspot"
        case .wifi: "Wi-Fi"
        case .other: "Other"
        }
      }

      var symbol: String {
        switch self {
        case .cable: "cable.connector"
        case .hotspot: "personalhotspot"
        case .wifi: "wifi"
        case .other: "network"
        }
      }
    }

    var id: String { "\(interface)-\(address)" }
    let interface: String
    let address: String
    let kind: Kind
  }

  private(set) var addresses: [Address] = []
  @ObservationIgnored private let monitor = NWPathMonitor()

  init() {
    refresh()
    monitor.pathUpdateHandler = { [weak self] _ in
      Task { @MainActor in self?.refresh() }
    }
    monitor.start(queue: DispatchQueue(label: "io.zoompilot.jetlink.network"))
  }

  /// The phone's address on the comma's cable network, while the cable is in.
  var cable: Address? {
    addresses.first { $0.kind == .cable }
  }

  var wifi: Address? {
    addresses.first { $0.kind == .wifi }
  }

  func refresh() {
    addresses = NetworkInterfaces.read()
  }

  static func read() -> [Address] {
    var head: UnsafeMutablePointer<ifaddrs>?
    guard getifaddrs(&head) == 0, let first = head else { return [] }
    defer { freeifaddrs(head) }
    var found: [Address] = []
    var cursor: UnsafeMutablePointer<ifaddrs>? = first
    while let entry = cursor {
      defer { cursor = entry.pointee.ifa_next }
      let flags = Int32(entry.pointee.ifa_flags)
      guard flags & IFF_UP != 0, flags & IFF_LOOPBACK == 0,
        let socket = entry.pointee.ifa_addr, socket.pointee.sa_family == UInt8(AF_INET)
      else { continue }
      let name = String(cString: entry.pointee.ifa_name)
      var host = [CChar](repeating: 0, count: Int(NI_MAXHOST))
      guard getnameinfo(socket, socklen_t(socket.pointee.sa_len), &host, socklen_t(host.count), nil, 0, NI_NUMERICHOST) == 0 else { continue }
      let text = String(decoding: host.prefix { $0 != 0 }.map { UInt8(bitPattern: $0) }, as: UTF8.self)
      let kind: Address.Kind
      if text.hasPrefix(NetworkInterfaces.cableNetwork) {
        kind = .cable  // the comma's lease, whatever iOS names the interface
      } else if name == "en0" {
        kind = .wifi
      } else if name.hasPrefix("bridge") {
        kind = .hotspot
      } else if name.hasPrefix("pdp_ip") || name.hasPrefix("utun") || name.hasPrefix("ipsec") || name.hasPrefix("awdl") || name.hasPrefix("llw") {
        continue  // cellular and tunnels: never where a comma is
      } else {
        kind = .other
      }
      found.append(Address(interface: name, address: text, kind: kind))
    }
    return found.sorted { ($0.kind, $0.interface) < ($1.kind, $1.interface) }
  }
}
