import Foundation
import JetlinkKit
import Testing

/// How the comma's link is carried, as the apps show it.
struct LinkMediumTests {
  @Test(
    "A USB speed as Linux names it gives its generation",
    arguments: [
      ("super-speed-plus", LinkMedium.usb3), ("super-speed", .usb3), ("high-speed", .usb2), ("full-speed", .usb1), ("UNKNOWN", .usb),
    ])
  func usbSpeeds(speed: String, medium: LinkMedium) {
    #expect(LinkMedium(usbSpeed: speed) == medium)
  }

  @Test("A hello's link names the medium, or nothing")
  func helloLinks() {
    #expect(LinkMedium(link: ["kind": "cable", "usb_speed": "high-speed"]) == .usb2)
    #expect(LinkMedium(link: ["kind": "usb", "usb_speed": "super-speed"]) == .usb3)
    #expect(LinkMedium(link: ["kind": "usb"]) == .usb)
    // before a hello: the comma's cable address at the far end is the cable
    #expect(LinkMedium(tcpPeer: "192.168.60.1:5599") == .usb)
    #expect(LinkMedium(tcpPeer: "10.0.0.5:40000") == .tcp)
    #expect(LinkMedium(tcpPeer: "192.168.60.4:50000") == .tcp)
    #expect(LinkMedium(link: ["kind": "tcp"]) == .tcp)
    #expect(LinkMedium(link: ["kind": "pigeon"]) == nil)
    #expect(LinkMedium(link: nil) == nil)
  }

  @Test("Only USB 2 and 1 are slow enough to warn about")
  func slowLinks() {
    #expect(LinkMedium.allCases.filter(\.isSlow) == [.usb2, .usb1])
    #expect(LinkMedium.usb2.advice?.hasPrefix("USB 2 costs") == true)
    #expect(LinkMedium.usb3.advice == nil)
  }

  @Test("A link event from a server that predates the field has no medium")
  func olderServers() throws {
    let old = try JSONDecoder().decode(LinkEvent.self, from: Data(#"{"state":"connected","detail":"","peer":"usb"}"#.utf8))
    #expect(old.linkMedium == nil)
    let new = try JSONDecoder().decode(LinkEvent.self, from: Data(#"{"state":"connected","detail":"","peer":"usb","medium":"usb2"}"#.utf8))
    #expect(new.linkMedium == .usb2)
  }
}
