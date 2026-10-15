import Foundation
import JetlinkUI

/// The Mac's own sample values, beside the events and rows JetlinkUI shares
/// with the iPhone: the server's description and its log. Nothing here is used
/// by a running app.
extension PreviewData {
  // MARK: Server

  static let serverInfo = ServerInfo(
    version: "0.5.0",
    choice: .coreml,
    runtimeVersion: "1.29.0",
    device: "coreml-Apple_M1_Pro",
    cache: "/Users/me/Library/Application Support/Jetlink/cache",
    port: nil
  )

  // MARK: Logs

  static let logLines = [
    "2026-09-09 20:14:02 INFO    jetlink.server.main: jetlink server 0.2.0, backend ort 1.29.0 on coreml-Apple_M1_Pro",
    "2026-09-09 20:14:02 INFO    jetlink.server.main: cache /Users/me/Library/Application Support/Jetlink/cache",
    "2026-09-09 20:14:03 INFO    jetlink.server.main: waiting for a jetlink gadget at 1209:0001",
    "2026-09-09 20:14:31 WARNING jetlink.server.session: no engine loaded, the comma will use its small model",
    "2026-09-09 20:15:00 INFO    jetlink.server.session: engine ready, a086d5249fc308bb, 548.9 s",
    "2026-09-09 20:15:11 ERROR   jetlink.server.session: link dropped after 0 frames",
  ]
}
