// The preparation reads and writes float16 weights through Swift's Float16,
// which Intel Macs do not have; the apps target the Neural Engine anyway. The
// app's release build passes ARCHS=arm64 for the package's targets too
// (macos/scripts/build-app.sh), since Xcode otherwise builds them universal.
#if os(macOS) && arch(x86_64)
  #error("JetlinkONNX needs Apple silicon: Float16 does not exist on Intel Macs")
#endif
