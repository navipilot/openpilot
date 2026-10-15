import AppKit
import Observation
@preconcurrency import Sparkle
import os

/// Updates from the GitHub releases, through Sparkle: a check a day, Check for
/// Updates in the menus, and the two settings. Only a build with a feed has
/// them, and the release workflow gives one only to a build it signs with the
/// update key, so a `make app` build never offers to replace itself.
///
/// Nothing installs without the user: Sparkle asks, or with automatic updates
/// on, installs when Jetlink quits. A check that finds an update while a comma
/// is connected keeps it to the menu bar until the comma is gone, so a drive
/// never gets an update alert.
@MainActor
@Observable
final class UpdateStore: NSObject {
  var isAvailable: Bool { controller != nil }
  /// False while a check is running.
  private(set) var canCheckForUpdates = false
  /// The version a scheduled check found while a comma was connected. The
  /// menu bar offers it, and it comes up once the comma is gone and Jetlink is
  /// next in front.
  private(set) var heldUpdate: String?

  /// Sparkle's own settings, read live: the update window can change the
  /// second one too. Set only from the settings view, as Sparkle asks; writing
  /// one at launch would pin Info.plist's default into the user's defaults.
  var automaticallyChecks: Bool {
    get {
      access(keyPath: \.automaticallyChecks)
      return controller?.updater.automaticallyChecksForUpdates ?? false
    }
    set {
      withMutation(keyPath: \.automaticallyChecks) { controller?.updater.automaticallyChecksForUpdates = newValue }
    }
  }
  var automaticallyDownloads: Bool {
    get {
      access(keyPath: \.automaticallyDownloads)
      return controller?.updater.automaticallyDownloadsUpdates ?? false
    }
    set {
      withMutation(keyPath: \.automaticallyDownloads) { controller?.updater.automaticallyDownloadsUpdates = newValue }
    }
  }

  @ObservationIgnored private var controller: SPUStandardUpdaterController?
  @ObservationIgnored private let isCommaConnected: @MainActor () -> Bool
  @ObservationIgnored private var canCheckObservation: NSKeyValueObservation?
  @ObservationIgnored private let log = Logger(subsystem: "io.zoompilot.jetlink", category: "updates")

  init(isCommaConnected: @escaping @MainActor () -> Bool) {
    self.isCommaConnected = isCommaConnected
    super.init()
    let feed = Bundle.main.object(forInfoDictionaryKey: "SUFeedURL") as? String ?? ""
    let key = Bundle.main.object(forInfoDictionaryKey: "SUPublicEDKey") as? String ?? ""
    guard !feed.isEmpty, !key.isEmpty else {
      log.info("updates are off: this build has no update feed")
      return
    }
    let controller = SPUStandardUpdaterController(startingUpdater: false, updaterDelegate: nil, userDriverDelegate: self)
    do {
      try controller.updater.start()
    } catch {
      log.error("the updater did not start: \(error.localizedDescription, privacy: .public)")
      return
    }
    self.controller = controller
    canCheckObservation = controller.updater.observe(\.canCheckForUpdates, options: [.initial, .new]) { [weak self] _, change in
      let value = change.newValue ?? false
      MainActor.assumeIsolated { self?.canCheckForUpdates = value }
    }
    _ = NotificationCenter.default.addObserver(forName: NSApplication.didBecomeActiveNotification, object: nil, queue: .main) {
      [weak self] _ in
      MainActor.assumeIsolated { self?.showHeldUpdateIfClear() }
    }
  }

  /// Checks now, or brings an update already found to the front.
  func checkForUpdates() {
    controller?.checkForUpdates(nil)
  }

  private func showHeldUpdateIfClear() {
    guard heldUpdate != nil, !isCommaConnected() else { return }
    log.info("showing the update held while a comma was connected")
    checkForUpdates()
  }

  /// The feed's notes are the changelog from the new release back several
  /// releases, each under a `Jetlink vX.Y.Z` heading. Cut at the installed
  /// release's heading so only what is new shows; nil keeps them all, when
  /// that heading is not there or comes first.
  nonisolated static func notesNewer(than installed: String, in notes: NSAttributedString) -> NSAttributedString? {
    // the whole line, so v0.8.1 does not cut at v0.8.10, and the blank lines before it
    let heading = #"\s*^[ \t]*"# + NSRegularExpression.escapedPattern(for: "Jetlink v\(installed)") + #"[ \t]*$"#
    guard let regex = try? NSRegularExpression(pattern: heading, options: .anchorsMatchLines),
      let match = regex.firstMatch(in: notes.string, range: NSRange(location: 0, length: notes.length)),
      match.range.location > 0
    else { return nil }
    return notes.attributedSubstring(from: NSRange(location: 0, length: match.range.location))
  }
}

/// Sparkle's standard user driver calls these on the main thread.
extension UpdateStore: @preconcurrency SPUStandardUserDriverDelegate {
  var supportsGentleScheduledUpdateReminders: Bool { true }

  func standardUserDriverShouldHandleShowingScheduledUpdate(_ update: SUAppcastItem, andInImmediateFocus immediateFocus: Bool) -> Bool {
    !isCommaConnected()
  }

  func standardUserDriverWillHandleShowingUpdate(_ handleShowingUpdate: Bool, forUpdate update: SUAppcastItem, state: SPUUserUpdateState) {
    guard !handleShowingUpdate else { return }
    log.info("holding update \(update.displayVersionString, privacy: .public) while a comma is connected")
    heldUpdate = update.displayVersionString
  }

  func standardUserDriverDidReceiveUserAttention(forUpdate update: SUAppcastItem) {
    heldUpdate = nil
  }

  func standardUserDriverWillFinishUpdateSession() {
    heldUpdate = nil
  }

  func standardUserDriverWillShowReleaseNotesText(
    _ releaseNotesAttributedString: NSAttributedString, forUpdate update: SUAppcastItem, withBundleDisplayVersion bundleDisplayVersion: String,
    bundleVersion: String
  ) -> NSAttributedString? {
    UpdateStore.notesNewer(than: bundleDisplayVersion, in: releaseNotesAttributedString)
  }
}
