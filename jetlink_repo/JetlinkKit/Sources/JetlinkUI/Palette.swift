#if canImport(SwiftUI)
  import SwiftUI

  #if canImport(AppKit)
    import AppKit
  #else
    import UIKit
  #endif

  extension Color {
    /// A colour with its own step in dark mode, as the chart palette specifies
    /// them, rather than one colour flipped automatically.
    public init(light: UInt32, dark: UInt32) {
      #if canImport(AppKit)
        self.init(
          nsColor: NSColor(name: nil) { appearance in
            let isDark = appearance.bestMatch(from: [.aqua, .darkAqua]) == .darkAqua
            return NSColor(srgbHex: isDark ? dark : light)
          })
      #else
        self.init(
          uiColor: UIColor { traits in
            UIColor(srgbHex: traits.userInterfaceStyle == .dark ? dark : light)
          })
      #endif
    }
  }

  #if canImport(AppKit)
    extension NSColor {
      fileprivate convenience init(srgbHex hex: UInt32) {
        self.init(
          srgbRed: CGFloat((hex >> 16) & 0xFF) / 255,
          green: CGFloat((hex >> 8) & 0xFF) / 255,
          blue: CGFloat(hex & 0xFF) / 255,
          alpha: 1)
      }
    }
  #else
    extension UIColor {
      fileprivate convenience init(srgbHex hex: UInt32) {
        self.init(
          red: CGFloat((hex >> 16) & 0xFF) / 255,
          green: CGFloat((hex >> 8) & 0xFF) / 255,
          blue: CGFloat(hex & 0xFF) / 255,
          alpha: 1)
      }
    }
  #endif
#endif
