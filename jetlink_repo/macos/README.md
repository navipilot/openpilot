<a id="jetlink-for-mac"></a>

# Jetlink for Mac development

To install and use the app, follow the [Mac user guide](../docs/macos-app.md).
This page covers building, testing, and signing it from source.
Run the commands from the repository root.

## Build

```
brew install xcodegen
make -C macos app
```

`make app` generates the Xcode project, builds Release for Apple silicon, and
signs the app ad hoc. The result is `macos/build/Jetlink.app`. The server is
the Swift one in `JetlinkKit` (every platform runs the same code), linked into
the app with onnxruntime's static xcframework, which Xcode downloads (61 MB)
the first time it resolves the package.

Check the built app with `make -C macos smoke`: it launches the bundle with TCP
on a free port and a cache of its own, says hello and pings over the wire
protocol, and stops it. Any `python3` runs the client; it uses only
`jetlink/protocol.py` and the TCP transport from this checkout.

## Develop

```
make -C macos dev
```

This opens `Jetlink.xcodeproj`. The Swift server itself lives in
`JetlinkKit/Sources/JetlinkServer`; `swift test --package-path JetlinkKit`
runs its tests, and `jetlink-server` runs it without the app
([from a terminal](../docs/platforms.md#from-a-terminal)).

`make -C macos project` regenerates the project from `project.yml` alone;
`make -C macos test` runs the Swift Testing suites. The test host starts no
server. The generated `Jetlink.xcodeproj` is committed, so
`open macos/Jetlink.xcodeproj` works without xcodegen installed.

## Sign, notarize, release

For CI releases and signing secrets, see [publishing](../docs/publishing.md).

```
SIGN_IDENTITY="Developer ID Application: Name (TEAMID)" make -C macos app
NOTARY_KEY_ID=... NOTARY_ISSUER_ID=... NOTARY_KEY_PATH=AuthKey_XXXX.p8 make -C macos notarize
SIGN_IDENTITY="Developer ID Application: Name (TEAMID)" make -C macos dmg
SPARKLE_ED_KEY_FILE=jetlink-ed25519-private.key make -C macos appcast
```

`SIGN_IDENTITY` defaults to `-` (ad hoc), which is all a machine without a
Developer ID certificate can do; such a build runs locally but Gatekeeper will
not accept it on another Mac. The hardened runtime is on either way. An ad hoc
build, and a Debug build from Xcode or `make test`, takes
`Resources/Jetlink-AdHoc.entitlements`, which turn library validation off: it
checks that the app and Sparkle share a Team ID, and an ad hoc signature has
none.

## Updates

The app updates itself from GitHub releases with
[Sparkle](https://sparkle-project.org) (`Jetlink/App/UpdateStore.swift`,
pinned in `project.yml`). Only a build with a feed checks, and
`JETLINK_UPDATE_FEED_URL` is empty unless the release workflow signs with the
update key, so a `make app` build shows **This build does not update itself**
in Settings.

`make appcast` runs Sparkle's `generate_appcast` on the DMG and writes
`build/appcast.xml`, the feed the release carries
([publishing](../docs/publishing.md#mac-updates)). To try an update locally,
point two builds at a feed of your own and a throwaway key, sign both with a
Team ID (an Apple Development identity will do; that is how it was tried), and
serve the folder:

```
export JETLINK_UPDATE_FEED_URL=http://127.0.0.1:8765/appcast.xml JETLINK_UPDATE_PUBLIC_KEY=<test public key>
JETLINK_VERSION=0.8.0 JETLINK_BUILD=9000 SIGN_IDENTITY="Apple Development: ..." make -C macos app   # copy it out: the old one
export JETLINK_VERSION=0.8.1 JETLINK_BUILD=9001
SIGN_IDENTITY="Apple Development: ..." make -C macos app dmg
SPARKLE_ED_KEY_FILE=test.key UPDATE_DOWNLOAD_BASE=http://127.0.0.1:8765 make -C macos appcast
```

Then put the DMG and `appcast.xml` in one folder, serve it with
`python3 -m http.server 8765 --bind 127.0.0.1`, and open the old copy. A
scheduled check shows its window only once the app is in front, as Sparkle
does for any app with a Dock icon. The new build's version has to have a
`CHANGELOG.md` section for the notes to show; without one the window links
to the release page. `defaults delete io.zoompilot.jetlink SULastCheckTime`
makes the next launch check again.

## Outputs

Everything lands in `macos/build/`: `Jetlink.app`, `DerivedData/`, from
`make dmg` the `Jetlink-<version>-macOS.dmg` and `SHA256SUMS` (plus a `.zip`
for an ad hoc build), and from `make appcast` the `appcast.xml`. None of it is
committed.

The app icon is generated once by `scripts/make-icon.swift` and its output is
committed under `Resources/Assets.xcassets`.
