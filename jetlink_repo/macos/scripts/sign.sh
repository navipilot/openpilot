#!/usr/bin/env bash
#
# Sign Jetlink.app.
#
#   scripts/sign.sh macos/build/Jetlink.app
#   SIGN_IDENTITY="Developer ID Application: Name (TEAMID)" scripts/sign.sh ...
#
# Gotchas:
#   - codesign --deep is not used for signing: it would sign any nested code
#     with the app's entitlements. Everything under Contents/Frameworks (the
#     Swift server's onnxruntime, Sparkle and its helpers) is signed here
#     first, inside out; nested code anywhere else stops the script.
#   - Ad hoc ("-") cannot carry a secure timestamp, so --timestamp is dropped
#     in that case. --options runtime stays on either way so a local build
#     behaves like a release one.
#   - The hardened runtime's library validation lets the app load Sparkle only
#     when both carry the same Team ID, and an ad hoc signature has none. An
#     ad hoc app takes Jetlink-AdHoc.entitlements, which turn it off; a
#     Developer ID one keeps it.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
MACOS_DIR="$(dirname "$SCRIPT_DIR")"

APP="${1:-$MACOS_DIR/build/Jetlink.app}"
SIGN_IDENTITY="${SIGN_IDENTITY:--}"
APP_ENTITLEMENTS="$MACOS_DIR/Resources/Jetlink.entitlements"

[ -d "$APP" ] || { echo "error: no app bundle at $APP" >&2; exit 1; }

TIMESTAMP=--timestamp
if [ "$SIGN_IDENTITY" = "-" ]; then
  TIMESTAMP=--timestamp=none
  echo "==> signing ad hoc; this build is for local use only"
  APP_ENTITLEMENTS="$MACOS_DIR/Resources/Jetlink-AdHoc.entitlements"
fi

NESTED="$(find "$APP/Contents" \( -path "$APP/Contents/MacOS" -o -path "$APP/Contents/Frameworks" \) -prune -o -type f \( -name '*.dylib' -o -name '*.so' \) -print)"
if [ -n "$NESTED" ]; then
  echo "error: nested code this script does not sign yet:" >&2
  printf '  %s\n' "$NESTED" >&2
  exit 1
fi

FRAMEWORKS="$APP/Contents/Frameworks"
if [ -d "$FRAMEWORKS" ]; then
  # The app is built ad hoc (build-app.sh), so the code it embeds is signed
  # here with the app's identity. `find -depth` lists a bundle's contents
  # before the bundle, so each loose dylib or executable (Sparkle's Autoupdate,
  # its Updater.app) is signed before the .app, .xpc or .framework that seals
  # it. An XPC service keeps its own entitlements, as Sparkle asks.
  echo "==> signing the frameworks"
  while IFS= read -r -d '' f; do
    case "$f" in
      *.xpc) codesign --force --options runtime "$TIMESTAMP" --preserve-metadata=entitlements --sign "$SIGN_IDENTITY" "$f" ;;
      *) codesign --force --options runtime "$TIMESTAMP" --sign "$SIGN_IDENTITY" "$f" ;;
    esac
  done < <(find "$FRAMEWORKS" -depth \( -type d \( -name '*.framework' -o -name '*.app' -o -name '*.xpc' \) -o -type f \( -name '*.dylib' -o -perm -u+x \) \) -print0)
fi
echo "==> signing the app"
codesign --force --options runtime "$TIMESTAMP" \
  --entitlements "$APP_ENTITLEMENTS" --sign "$SIGN_IDENTITY" "$APP"

echo "==> verifying"
codesign --verify --deep --strict --verbose=2 "$APP"

if [ "$SIGN_IDENTITY" != "-" ]; then
  # Before notarization this reports "Unnotarized Developer ID". That is
  # expected; notarize.sh is the next step.
  spctl --assess --type execute --verbose "$APP" || true
fi
