# Publish a release

Updating an installed server: [updates and rollback](releasing.md). This page:
publishing a release.

A pushed `v*` tag runs the Release workflow: the macOS app, the Android app and
the Linux server for Jetsons and PCs, then the iPhone app on TestFlight.

Cut one when something under `JetlinkKit`, `macos`, `ios`, `android`,
`install.sh` or the wire (`jetlink/protocol.py`, `jetlink/transport`,
`jetlink/spec.py`, `jetlink/registry`) changed. A change on the comma's side
alone ships as a zoompilot pin of the `main` commit (driven on
`danger-unstable` first) and waits for the next release's notes: a tag builds
and publishes every app, and the installer moves every Jetson and PC to it.

1. Set `__version__` in `jetlink/__init__.py` (`pyproject.toml` reads it), run
   `.venv/bin/python JetlinkKit/Scripts/make_pins.py` so `Pinned.swift` carries
   the new version, add a `Jetlink vX.Y.Z` section at the top of
   `CHANGELOG.md`, and commit.
   - The tag must match `__version__`; `macos/scripts/check-version.sh` checks
     before the build.
   - The section becomes the release notes: write what installers will notice,
     not how it was done. Without one, GitHub generates the notes. Include
     what changed on the comma since the last release, and say whether the
     protocol version moved; if it did, the comma and the server update
     together.
2. Tag and push (replace `0.7.0`):

```bash
git tag v0.7.0
git push origin v0.7.0
```

3. Watch **Actions > Release**. The macOS job builds, smoke-tests and notarizes
   the app; the Android jobs build `libjetlink.so` and the APK as CI does and
   sign it with the [release key](#android-release-key); each Linux server
   builds on a native runner for its architecture with
   `scripts/build-linux.sh`.
4. Check the release page: `Jetlink-0.7.0-macOS.dmg`, `appcast.xml` (the
   Mac app's [update feed](#mac-updates)), `Jetlink-0.7.0-Android.apk`,
   `SHA256SUMS` (both apps), `jetlink-server-0.7.0-linux-aarch64.tar.gz` and
   `-linux-x86_64.tar.gz` with their `.sha256`, and notes made of the
   changelog section and the install commands.
5. The **TestFlight** job starts once the release is out and waits while
   Apple processes the upload, usually 5 to 30 minutes. Then the build is in
   the internal group and waiting for Beta App Review in the public one
   ([below](#iphone-app-on-testflight)).

- The release waits for both Linux servers: the installer takes the newest
  release, so one without them would stop every install and update.
- Each push to `main` refreshes the `edge` prerelease with
  `jetlink-server-edge-linux-aarch64.tar.gz` and `-x86_64`, which the installer
  takes with `--ref main`. A push that changes only the comma's side (the
  `changes` job in `.github/workflows/ci.yml` lists the paths) builds nothing:
  the comma takes jetlink as a git pin, and edge keeps the server it has.
- Prereleases: a hyphen (`v0.7.0-rc1`) or a PEP 440 suffix (`v0.7.0a1`,
  `v0.7.0b2`, `v0.7.0rc1`) publishes as a prerelease. Use the same version in
  `jetlink/__init__.py` and the tag, minus the leading `v`.

## Installing the app

Open the DMG and drag Jetlink to Applications. The APK installs as in
[Jetlink for Android](android-app.md#install). Verify a download against
`SHA256SUMS`, in the folder it is in:

```bash
shasum -a 256 -c --ignore-missing SHA256SUMS
```

## Signing secrets

Releases are signed with a Developer ID and notarized, and the APK with the
Android release key. A fork without these secrets gets an ad hoc signed ZIP
and no DMG, and an APK signed with the runner's debug key; the workflow steps
"Report the signing mode" say which mode ran. zoompilot/jetlink's release stops
instead of shipping a debug-signed APK.

| Secret | What |
| --- | --- |
| `MACOS_CERTIFICATE_P12_BASE64` | the Developer ID Application certificate with its private key, exported from Keychain Access as a .p12 and base64 encoded |
| `MACOS_CERTIFICATE_PASSWORD` | the .p12 password |
| `KEYCHAIN_PASSWORD` | any random string; it locks the temporary keychain the runner builds in |
| `NOTARY_KEY_ID` | the App Store Connect API key id |
| `NOTARY_ISSUER_ID` | the issuer id of that key |
| `NOTARY_PRIVATE_KEY_P8_BASE64` | the key's .p8 file, base64 encoded |
| `APPLE_TEAM_ID` | the team ID the iPhone app is signed for; without it, nothing goes to TestFlight |
| `ANDROID_KEYSTORE_BASE64` | the Android release keystore, base64 encoded ([below](#android-release-key)) |
| `ANDROID_KEYSTORE_PASSWORD` | its password, which is also the key's |
| `ANDROID_KEY_ALIAS` | the key's alias in it |
| `SPARKLE_ED_PRIVATE_KEY` | the Mac app's update signing key ([below](#mac-update-key)) |

- Notary key: a Team key made under **Users and Access > Integrations** in App
  Store Connect. The Developer role notarizes; give it Admin if the same key
  uploads the iPhone app (below).
- Encode a certificate with `base64 -i cert.p12 | pbcopy`; for
  `NOTARY_PRIVATE_KEY_P8_BASE64` encode the `.p8` file instead.
- The p12 can come from a key and CSR made with openssl instead of Keychain
  Access: upload the CSR under **Certificates > Developer ID Application**
  (G2 Sub-CA), then join the key and the downloaded `.cer` with
  `openssl pkcs12 -export`.

### Android release key

Android installs an update only over an app signed with the same key, so every
release's APK has to be signed with this one. Make it once, with the JDK's
keytool, which asks for a password (the store's and the key's):

```bash
keytool -genkeypair -v -keystore jetlink-release.keystore -storetype PKCS12 \
  -alias jetlink -keyalg RSA -keysize 4096 -validity 10000 \
  -dname "CN=Jetlink, O=zoompilot"
```

Then set the three secrets (`gh secret set` without `--body` asks for the
value):

```bash
base64 -i jetlink-release.keystore | gh secret set ANDROID_KEYSTORE_BASE64 --repo zoompilot/jetlink
gh secret set ANDROID_KEYSTORE_PASSWORD --repo zoompilot/jetlink
gh secret set ANDROID_KEY_ALIAS --repo zoompilot/jetlink --body jetlink
```

- Keep the keystore and its password backed up outside the repository. A lost
  key means a new one, and every phone then has to uninstall Jetlink, models
  and all, to take the next release.
- The workflow's "Check the APK's key" step fails a release whose APK is not
  signed with this key's certificate, `RELEASE_CERT_SHA256` in
  `.github/workflows/release.yml` (`keytool -list -v` prints it). A new key
  means a new value there, and every phone uninstalling.
- To sign a local build with it, set `JETLINK_ANDROID_KEYSTORE` (an absolute
  path), `JETLINK_ANDROID_KEYSTORE_PASSWORD` and `JETLINK_ANDROID_KEY_ALIAS`
  before `./gradlew :app:assembleRelease`.

### Mac update key

The Mac app takes an update only when the DMG and the feed are signed with
this Ed25519 key. Its public half is `JETLINK_UPDATE_PUBLIC_KEY` in
`macos/project.yml`, built into every copy as `SUPublicEDKey`; the private
half is the secret, base64 of the 32-byte seed (the format of Sparkle's
`generate_keys -x`):

```bash
gh secret set SPARKLE_ED_PRIVATE_KEY --repo zoompilot/jetlink < jetlink-ed25519-private.key
```

- Keep the key backed up outside the repository. Losing it is slow to
  recover from: the feed is signed too, so a release signed with a new key
  reaches installed copies only after 20 days of failed checks, and without
  its notes. Sparkle then takes the new key because the DMG keeps the same
  Developer ID; it never accepts a new key and a new Developer ID at once.
- "Write the update feed" checks the signature against the public key inside
  the app it built, so a secret that is not that key's other half stops the
  release instead of shipping an update no copy accepts.
- Sparkle's `generate_keys` makes a key in the keychain; `generate_keys -x`
  exports it in the secret's format.

## Mac updates

The Mac app checks `releases/latest/download/appcast.xml` once a day, so the
newest release that is not a prerelease is the one it offers.
`macos/scripts/make-appcast.sh` writes that feed during the release with
Sparkle's `generate_appcast`: one item, the DMG, with the release notes taken
from `CHANGELOG.md`, this release's section and the ones before it. The app shows only the sections
newer than the version it has, so write each section as the notes for people
updating from the one before.

- Only a build the release signs with both the Developer ID and the update
  key gets a feed. Any other build, a local one or a fork's without the key,
  does not update itself. A fork with a key of its own also sets
  `JETLINK_UPDATE_PUBLIC_KEY` in `macos/project.yml`, or the check stops its
  release.
- To stop a bad release reaching more Macs, delete its `appcast.xml` asset
  (`gh release delete-asset v0.7.0 appcast.xml`). Copies that already updated
  keep it; Sparkle never offers an older version, so the fix is a new release.

## iPhone app on TestFlight

The Release workflow ends with `.github/workflows/testflight.yml`, after the
GitHub release, for every tag but a prerelease (the App Store takes `X.Y.Z`
only). It archives the app, uploads it, waits while Apple processes it, sets
What to Test (a link to the release notes and how to try the app), adds the
build to the **Public** group and submits it for Beta App Review.

- It needs `APPLE_TEAM_ID` and the notary key, which needs the Admin role:
  xcodebuild makes the distribution certificate and profile with it, so no
  Mac signs in to Xcode.
- xcodebuild also makes an Apple Development certificate for the archive,
  since the runner has no key of its own; the job revokes it at the end.
- The version is `jetlink.__version__`, the build number the commit count.
- **Rerun failed jobs** after a failed review step publishes the build
  already uploaded rather than making another.
- **Actions > TestFlight > Run workflow** runs it on any branch. Unticked,
  **upload** only archives and exports, which checks the signing on the
  runner; ticked, it uploads a build of that branch. Clear **group** to keep
  the build to the internal group.
- The internal group (**Jetlink team**) gets every build once Apple has
  processed it.
- The public link (`https://testflight.apple.com/join/DAsYk5sP`, in the README,
  the iPhone guide, the release notes and the site) is the **Public** group. A
  build reaches it once it passes Beta App Review; later builds of a version
  usually pass at once. The job ends at the submission, so a rejection shows
  under **TestFlight** in App Store Connect, not in the run.
- Once per team: register the App ID `io.zoompilot.jetlink` with the
  **Increased Memory Limit** capability, and create the app in App Store
  Connect with that bundle ID. App Store Connect has no API for the second.

The same script uploads from a Mac:

```bash
JETLINK_TEAM=ABCDE12345 ASC_KEY_ID=... ASC_ISSUER_ID=... ASC_KEY_PATH=AuthKey_XXXX.p8 \
  TESTFLIGHT_GROUP=Public ios/scripts/testflight.sh
```

Without `TESTFLIGHT_GROUP` the build goes to the internal group alone.
`JETLINK_BUILD` overrides the build number when it is already taken, and
`TESTFLIGHT_NOTES` the What to Test text. `ios/scripts/asc.py` is what the
script asks of App Store Connect after the upload; `asc.py wait VERSION BUILD`
shows Apple's verdict on an upload, ITMS errors included, which the builds
list in App Store Connect leaves out.

<a id="the-container-images"></a>

## Comma and server release versions

The comma takes Jetlink as a git pin: zoompilot's `develop`,
`danger-unstable` and `jetson-trt` pin a commit on `main`, driven on
`danger-unstable` first, and it may lie between releases. A `v*` release is
cut when the apps, the server or the wire changed, not for the comma's side
alone; its notes list what changed on the comma since the last release too.

Releasing: [release steps](#publish-a-release). A version bump in
`jetlink/__init__.py` means running `JetlinkKit/Scripts/make_pins.py` again;
each release attaches the Linux server tarballs the installer downloads.
