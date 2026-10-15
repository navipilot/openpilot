#!/usr/bin/env bash
# Compiles and links the iPhone app's sources for arm64 iOS through SwiftPM,
# against the iPhone SDK Xcode ships with. Xcode itself will not build for iOS
# until its iOS platform component is installed (about 8 GB, with the
# simulator); this needs none of that, so it also suits CI.
set -euo pipefail

IOS_DIR="$(cd "$(dirname "$0")/.." && pwd)"
REPO="$(cd "$IOS_DIR/.." && pwd)"
WORK="$IOS_DIR/build/typecheck"
mkdir -p "$WORK/Sources/JetlinkApp"
rm -f "$WORK/Sources/JetlinkApp/"*.swift

cat > "$WORK/Package.swift" <<SWIFT
// swift-tools-version: 6.2
import PackageDescription

let package = Package(
  name: "typecheck",
  platforms: [.iOS("26.1"), .macOS(.v15)],
  dependencies: [.package(path: "$REPO/JetlinkKit")],
  targets: [
    .executableTarget(name: "JetlinkApp", dependencies: [
      .product(name: "JetlinkKit", package: "JetlinkKit"),
      .product(name: "JetlinkUI", package: "JetlinkKit"),
      .product(name: "JetlinkRegistry", package: "JetlinkKit"),
      .product(name: "JetlinkServer", package: "JetlinkKit"),
      .product(name: "JetlinkORT", package: "JetlinkKit"),
    ], swiftSettings: [.swiftLanguageMode(.v6)]),
  ]
)
SWIFT

find "$IOS_DIR/Jetlink" -name '*.swift' -exec ln -sf {} "$WORK/Sources/JetlinkApp/" \;
cd "$WORK"
swift build --triple arm64-apple-ios26.1 --sdk "$(xcrun --sdk iphoneos --show-sdk-path)"
echo "the iPhone app compiles and links for arm64 iOS 26.1"
