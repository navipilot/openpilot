import SwiftUI

/// How the comma and the iPhone or iPad meet: one USB-C cable, in a
/// few steps for a reader standing at the car. docs/iphone-app.md has the why.
struct ConnectHelpScreen: View {
  static let guide = URL(string: "https://github.com/zoompilot/jetlink/blob/main/docs/iphone-app.md#connect-the-comma")!

  var body: some View {
    let device = ThisDevice.name
    List {
      Section {
        step(1, "On the comma, set Jetlink to iOS.")
        step(2, "Open Jetlink and allow Local Network access.")
        step(3, "Connect your \(device) to the comma with a USB 3 USB-C cable.")
        step(4, "Wait for Connected.")
      } footer: {
        Text("A powered USB-C hub keeps your \(device) charging.")
      }
      Section {
        ForEach(USBSpeedGuide.rows, id: \.models) { row in
          LabeledContent(row.models, value: row.speed)
        }
      } header: {
        Text("Speed")
      } footer: {
        Text("Your iPhone or iPad needs a USB-C port. USB 2 works, but leaves less time for each frame.")
      }
      Section {
        Link("Learn More", destination: ConnectHelpScreen.guide)
      }
    }
    .readableWidth()
    .navigationTitle("Connecting the Comma")
    .navigationBarTitleDisplayMode(.inline)
  }

  private func step(_ number: Int, _ text: String) -> some View {
    HStack(alignment: .firstTextBaseline, spacing: 12) {
      Text(number.formatted())
        .font(.body.weight(.semibold).monospacedDigit())
        .foregroundStyle(.secondary)
        .frame(width: 18, alignment: .trailing)
      Text(text)
    }
    .accessibilityElement(children: .combine)
  }
}

#Preview {
  NavigationStack {
    ConnectHelpScreen()
  }
}
