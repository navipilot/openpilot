import AppKit
import Testing

@testable import Jetlink

@Suite("Updates")
struct UpdateStoreTests {
  private let notes = NSAttributedString(string: "Jetlink v0.9.0\nNew in 0.9.0\n\nJetlink v0.8.10\nNew in 0.8.10\n\nJetlink v0.8.1\nNew in 0.8.1\n")

  @Test("The notes stop at the installed release")
  func notesCut() {
    #expect(UpdateStore.notesNewer(than: "0.8.1", in: notes)?.string == "Jetlink v0.9.0\nNew in 0.9.0\n\nJetlink v0.8.10\nNew in 0.8.10")
    #expect(UpdateStore.notesNewer(than: "0.8.10", in: notes)?.string == "Jetlink v0.9.0\nNew in 0.9.0")
  }

  @Test("Notes without the installed release, or starting with it, stay whole")
  func notesKept() {
    #expect(UpdateStore.notesNewer(than: "0.7.0", in: notes) == nil)
    #expect(UpdateStore.notesNewer(than: "0.9.0", in: notes) == nil)
    #expect(UpdateStore.notesNewer(than: "0.8", in: notes) == nil)
  }
}
