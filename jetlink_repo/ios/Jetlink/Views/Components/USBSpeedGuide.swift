import Foundation

/// Which iPhones and iPads have a USB 3 port, from Apple's tech specs: USB 3
/// on the iPhone 15 Pro and later Pro models, and on every iPad Pro, iPad Air
/// and iPad mini with USB-C; USB 2 on the other USB-C iPhones and on the iPad
/// (10th generation) and (A16). docs/iphone-app.md has the sources. One table
/// for the help screen and for the slow-link banner.
enum USBSpeedGuide {
  struct Row {
    let device: String
    let models: String
    let speed: String
  }

  static let rows = [
    Row(device: "iPhone", models: "iPhone 15 Pro and Later", speed: "USB 3"),
    Row(device: "iPhone", models: "Other iPhones", speed: "USB 2"),
    Row(device: "iPad", models: "iPad Pro, Air and mini", speed: "USB 3"),
    Row(device: "iPad", models: "Other iPads", speed: "USB 2"),
  ]

  /// The table as one sentence for the kind of device in hand ("iPhone" or
  /// "iPad"): "iPhone 15 Pro and Later are USB 3; other iPhones are USB 2."
  static func summary(for device: String) -> String {
    let own = rows.filter { $0.device == device }
    guard !own.isEmpty else { return "" }
    let parts = own.enumerated().map { i, row in
      "\(i == 0 ? row.models : row.models.lowercased()) are \(row.speed)"
    }
    return parts.joined(separator: "; ") + "."
  }
}
