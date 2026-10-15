#!/usr/bin/env bash
#
# Write the update feed for the DMG with Sparkle's generate_appcast:
#
#   build/appcast.xml   one signed item: this version's DMG on its GitHub release
#
# The release workflow attaches the feed to the release, and installed copies
# read it from releases/latest/download/appcast.xml. Run it after make-dmg.sh:
# stapling changes the DMG, so it is signed for Sparkle once it is final.
#
#   SPARKLE_ED_PRIVATE_KEY=<base64 seed> scripts/make-appcast.sh build/Jetlink.app
#   SPARKLE_ED_KEY_FILE=path/to/key scripts/make-appcast.sh build/Jetlink.app
#
# The notes are CHANGELOG.md's, this release and the ones before it
# (scripts/changelog.py history), which the app cuts at the version it has. The
# download URL is GITHUB_REPOSITORY's release (zoompilot/jetlink's outside CI);
# UPDATE_DOWNLOAD_BASE puts the DMG somewhere else, for a test feed.
#
# Gotchas:
#   - generate_appcast only warns when the key is not the other half of the
#     app's SUPublicEDKey, and leaves the DMG unsigned. The signature it wrote
#     is checked against that key here, so the release stops instead of
#     publishing an update every installed copy refuses.
#   - generate_appcast reads a whole folder, so it gets one holding only this
#     DMG and its notes. It is the copy the Sparkle package brought into
#     DerivedData, the version the app embeds: `make app` has to have run.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MACOS_DIR="$(dirname "$SCRIPT_DIR")"
REPO_ROOT="$(dirname "$MACOS_DIR")"

APP="${1:-$MACOS_DIR/build/Jetlink.app}"
[ -d "$APP" ] || { echo "error: no app bundle at $APP" >&2; exit 1; }

VERSION="$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$APP/Contents/Info.plist")"
PUBLIC_KEY="$(/usr/libexec/PlistBuddy -c 'Print :SUPublicEDKey' "$APP/Contents/Info.plist")"
TAG="v$VERSION"
DMG="$MACOS_DIR/build/Jetlink-$VERSION-macOS.dmg"
OUT="$MACOS_DIR/build/appcast.xml"
REPO="${GITHUB_REPOSITORY:-zoompilot/jetlink}"
DOWNLOAD_BASE="${UPDATE_DOWNLOAD_BASE:-https://github.com/$REPO/releases/download/$TAG}"
GENERATE_APPCAST="$MACOS_DIR/build/DerivedData/SourcePackages/artifacts/sparkle/Sparkle/bin/generate_appcast"

[ -f "$DMG" ] || { echo "error: no $DMG; run make dmg first" >&2; exit 1; }
[ -x "$GENERATE_APPCAST" ] || { echo "error: no generate_appcast at $GENERATE_APPCAST; run make app first" >&2; exit 1; }
if [ -z "${SPARKLE_ED_PRIVATE_KEY:-}" ] && [ -z "${SPARKLE_ED_KEY_FILE:-}" ]; then
  echo "error: set SPARKLE_ED_PRIVATE_KEY or SPARKLE_ED_KEY_FILE" >&2
  exit 1
fi

STAGE="$(mktemp -d -t jetlink-appcast)"
trap 'rm -rf "$STAGE"' EXIT
cp "$DMG" "$STAGE/"
NOTES="$STAGE/$(basename "$DMG" .dmg).md"
python3 "$REPO_ROOT/scripts/changelog.py" history "$TAG" > "$NOTES"
if [ ! -s "$NOTES" ]; then
  echo "warning: CHANGELOG.md has no $TAG section; the update window links to the release page instead"
  printf 'See the [release notes](https://github.com/%s/releases/tag/%s).\n' "$REPO" "$TAG" > "$NOTES"
fi

echo "==> writing $OUT"
rm -f "$OUT"
# The key goes in on stdin, never on a command line.
if [ -n "${SPARKLE_ED_PRIVATE_KEY:-}" ]; then
  printf '%s\n' "$SPARKLE_ED_PRIVATE_KEY"
else
  cat "$SPARKLE_ED_KEY_FILE"
fi | "$GENERATE_APPCAST" --ed-key-file - --embed-release-notes \
  --download-url-prefix "$DOWNLOAD_BASE/" \
  --link "https://github.com/$REPO/releases/tag/$TAG" \
  --full-release-notes-url "https://github.com/$REPO/releases" \
  -o "$OUT" "$STAGE"

echo "==> checking the DMG's signature against the app's SUPublicEDKey"
SIGNATURE="$(sed -n 's/.*sparkle:edSignature="\([^"]*\)".*/\1/p' "$OUT")"
if [ -z "$SIGNATURE" ]; then
  echo "error: the feed has no signature for the DMG; is the private key the other half of the app's SUPublicEDKey?" >&2
  exit 1
fi
xcrun swift "$SCRIPT_DIR/check-update-signature.swift" "$PUBLIC_KEY" "$SIGNATURE" "$DMG"
