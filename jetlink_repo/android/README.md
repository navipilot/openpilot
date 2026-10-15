# Jetlink for Android development

To install and use the app, follow the [Android user guide](../docs/android-app.md):
each release carries the APK. This page covers building and testing it from
source. Run the commands from the repository root.

## Layout

The app is a Kotlin shell over the same Swift server every platform runs.
`JetlinkKit` builds for Android with the Swift SDK for Android, into one
native library, `libjetlink.so`:

| Part | What it is |
| --- | --- |
| `JetlinkKit` | The server, the model registry and the ONNX preparation, shared with every platform |
| `JetlinkORT/OrtBackend.swift` | onnxruntime's profiles; `htp`, `htp-whole` and `gpu` use its QNN provider on a Snapdragon's NPU and GPU |
| `JetlinkLiteRT`, `CLiteRt` | LiteRT's profiles `gpu`, `cpu` and `npu`, through LiteRT's C API opened at run time: the GPU path every phone can take, and a Google Tensor's NPU, compiled for on the phone |
| `JetlinkONNX/LiteRTPreparation.swift`, `LiteRTLowering.swift`, `LiteRTOps.swift` | The conversion to `.tflite` on the phone, in the forms LiteRT's GPU needs (fp16-safe LayerNorm, no rank-5 tensors, constant gathers as slices) |
| `JetlinkServer/UsbfsPipes.swift`, `CUsbfs` | The comma's bulk pair through usbdevfs, on the descriptor the app opened |
| `JetlinkKit/AppSnapshot.swift` | The app's state as the screens draw it, from the server's events |
| `JetlinkAndroid` | The JNI functions `io.zoompilot.jetlink.server.Native` calls |
| `android/app` | Kotlin and Compose: the screens, the foreground service, USB permission, the phone's health |

Kotlin holds no server logic. It sends control commands in the control
protocol's JSON ([control-protocol.md](../docs/control-protocol.md)) and draws the
snapshot the server hands back, whose Models rows come from the same
`ModelRowBuilder` as the other apps. `JetlinkKit/Tests/JetlinkKitTests/Fixtures/android_snapshot.json`
pins that JSON: the Swift tests write and check it, and the app's unit tests
parse it.

## Setup

- JDK 17 or newer, and the Android SDK with platform 37 and NDK 30.0.16248370
  (Android Studio installs them; `sdkmanager "platforms;android-37.0"
  "ndk;30.0.16248370"` does too).
- The open-source Swift 6.4.0 toolchain and the Swift SDK for Android of the
  same version. Xcode's Swift cannot use the SDK.

```
swiftly install 6.4.0
swift sdk install https://download.swift.org/swift-6.4.0-release/android-sdk/swift-6.4.0-RELEASE/swift-6.4.0-RELEASE_android.artifactbundle.tar.gz \
  --checksum 21fb555122a3d801ad943d48df7ebffdd8824de61c25c180bb792d3edaee0b43
```

Without swiftly, the toolchain's installer package from swift.org installs for
your user with `installer -pkg swift-6.4.0-RELEASE-osx.pkg -target CurrentUserHomeDirectory`.
`android/scripts/swift-build.sh` finds either, links the SDK to the NDK on its
first run, and says what is missing.

## Build

```
cd android
./gradlew :app:assembleRelease
adb install -r app/build/outputs/apk/release/app-release.apk
```

The `swiftBuild` task runs `scripts/swift-build.sh`, which cross-compiles
`JetlinkKit`'s `jetlink` library with the Swift runtime linked in, always
optimized. `-Pjetlink.prebuiltSwift=DIR` packages `DIR/arm64-v8a/libjetlink.so`
instead, for work on the Kotlin side without the Swift toolchain. The APK is
arm64 only, as the QNN runtime is.

Release builds are signed with the debug key so they install over debug ones;
the app is sideloaded, never on a store. The APK on the releases page is signed
with the project's [release key](../docs/publishing.md#android-release-key)
instead. Android installs neither over the other, so uninstall the released app
before you install your own build, and the other way round.

`python3 android/scripts/make-icon.py` makes the launcher and notification icons
from the iPhone app's, with Pillow. The outputs are committed.

## Test

```
cd JetlinkKit && swift test          # the server, on the Mac
cd android && ./gradlew :app:testDebugUnitTest
```

The Swift suites cover the server as on the other platforms, plus the usbfs
pipes against a fake kernel and the snapshot's JSON. The LiteRT suites run on
the Mac when `JETLINK_LITERT_DIR` names the `ai_edge_litert` directory of an
`ai-edge-litert` 2.2.0 wheel (it holds `libLiteRt.dylib` and the Metal GPU
accelerator); they skip without it. `jetlink-server --backend litert --device
gpu` serves through the same code on the Mac's GPU, for `verify_parity.py`. The Kotlin tests parse the
snapshot fixture and check the gadget's IDs, the port and the onnxruntime
release against `Pinned.swift`.

The same Swift suites run on Android itself, on a device or the emulator, with
their fixtures pushed beside them:

```
android/scripts/swift-test-device.sh path/to/onnxruntime-android-qnn-1.29.0.aar
```

On the emulator they pass, the whole server included, on onnxruntime's CPU
provider (`LITERT_AAR=path/to/litert-2.2.0.aar` adds the LiteRT suites). Its Android build runs an fp16 graph's MatMul in fp16, so the fp16
golden model lands within a few percent of the golden frames rather than bit
for bit, and is held to `verify_parity`'s correlation there instead.

## The emulator

An Android emulator on an Apple silicon Mac runs arm64, so the APK runs there on
the CPU (Settings > Processor > CPU). It has no USB host, so a bench tool on the
Mac stands in for the comma over TCP. Jetlink listens on the port only with the
developer setting on, which the emulator starts with (tap Version in Settings >
About seven times to toggle it):

```
adb forward tcp:5599 tcp:5599
python3 scripts/bench_link.py --host 127.0.0.1 --onnx big_driving_supercombo.onnx --rate 20
```

`adb logcat -s jetlink` shows the server's log. Launch extras open a tab or run
a benchmark, as the iPhone app's launch arguments do:

```
adb shell am start -n io.zoompilot.jetlink.android/io.zoompilot.jetlink.MainActivity -e tab models
adb shell am start -n io.zoompilot.jetlink.android/io.zoompilot.jetlink.MainActivity --ei benchmark 60
```

## On a phone

Nothing of this has run on a phone yet. What to check first, in order:

1. The gadget enumerates, Android asks to open Jetlink, and `adb logcat -s
   jetlink` shows the server's `comma attached` with the endpoints and a hello.
2. A model prepares on **GPU**: the log says what the conversion rewrote, how
   long LiteRT's GPU compile took, and that it ran whole on the GPU. A phone
   whose GPU cannot run it all fails the build with that said, rather than fall
   back to the CPU.
3. Benchmark 1 Minute, then 10 Minutes while charging in the car mount.
4. `scripts/verify_parity.py` from a Mac on the same Wi-Fi (developer setting
   on, see above, for the port), which must end with
   OK before a drive, then the comma's live bench
   (`jetlink_repo/scripts/comma/jetlink_live_bench.sh 180`).
5. On a Snapdragon, the same for **NPU + GPU**: the log names the sessions it
   built (`QNN(htp) then QNN(gpu)`) and how long the NPU compile took. It runs
   in float16 on the NPU without the GPU path's LayerNorm rewrite, so
   `verify_parity.py` decides whether it can be the default there.
6. On a Pixel 8 or later, the same for **NPU** (Automatic's pick there). The
   log says `compiling for the NPU`, how long it took and what runs the model:
   `NPU(Tensor G5)`, or a warning that the NPU's compiler could not take it and
   the GPU runs it, followed by LiteRT's own lines from logcat (`litert:` in
   Jetlink's log), so a shared log carries the reason. Settings > Processor and
   the Benchmark then say GPU. Watch the
   app's memory during that compile (`adb shell dumpsys meminfo
   io.zoompilot.jetlink.android`): Google's compiler for Tensor needed about
   11 times the model's weights on a PC, 8 GB and more for a big model, and a
   compile that gets the app killed is not tried again on that system build
   (the `.npu-compiling` file beside the artifact says so).

The first Pixel to try it (a Pixel 10 Pro Fold, 2026-10-03) refused: the
EdgeTPU service in `/system_ext` serves only the apps on Google's allowlist, by
package name and signing certificate, which Google delivers as the
`edgetpu_native` device config flags (`persist.device_config.edgetpu_native.allowlist_*`).
Jetlink logs "is not in the EdgeTPU allowed list" and "error code 16", and runs
on the GPU. Google adds an app; the release key's certificate is the one to
give them (docs/publishing.md#android-release-key).

The NPU path follows LiteRT 2.2.0's source. LiteRT's Google Tensor plugin
(`libLiteRtCompilerPlugin_google_tensor.so`, which the APK carries) hands the
converted model to `EdgeTpuCompilerCompileFlatbuffer` in the Pixel's
`/vendor/lib64/libedgetpu_litert.so`. The September 2026 system images of the
Pixel 8, 9 and 10 Pro Fold all export it, and list the library as public to
apps. LiteRT keeps the compiled model in the artifact's `npu-cache/`, keyed by
the phone's build fingerprint, and the artifact's name carries a digest of the
fingerprint, so a system update prepares the model again.

The first phone a user ran it on was a Pixel 10 Pro Fold (Google Tensor G5,
2026-10-01). QNN cannot drive a Tensor: every op fell to onnxruntime's CPU
provider on one thread, minutes a frame, so QNN's choices are a Snapdragon's
only, a Pixel 8 or later defaults to its Tensor NPU through LiteRT, and every other phone to LiteRT's GPU. onnxruntime 1.29's Android CPU provider also runs
an fp16 Gemm with a transposed weight on one thread, about 100 times slower than
the same product as a MatMul (8 s against 0.09 s for 256x1024x1024 on the
emulator), so the CPU profile prepares the graph without CoreML's Gemm rewrite.
The QNN profiles keep it: check on a Snapdragon that the NPU takes every Gemm,
since one left to the CPU costs seconds.

## Checking a model against Google's compiler for Tensor

A phone compiles the model for its NPU itself, so nothing here needs Google's
Tensor SDK. With access to the SDK's beta (an x86-64 Linux compiler), its
ahead-of-time compiler shows on a PC whether a converted model compiles for a
Tensor G3 to G6 at all:

```
android/scripts/tensor-compile-check.sh path/to/litert_plugin_compiler.tar.gz model.tflite Tensor_G5
```

`model.tflite` is the one in a LiteRT artifact (`jetlink-server build ONNX
--backend litert` on a Mac writes one). The script runs the compiler in Docker
for linux/amd64. It needs about 11 times the model's weights in memory: 8 GB
and more for a big model.

## Licenses

The APK carries LiteRT's C library and GPU accelerator (Apache 2.0, from Maven,
`com.google.ai.edge.litert:litert`), LiteRT's Google Tensor dispatch library
and compiler plugin (from LiteRT's v2.2.0 GitHub release,
`litert_npu_runtime_libraries_jit.zip`; their source directory in LiteRT
carries Google's Tensor SDK terms beside the Apache license, read them before
you share a build), onnxruntime (MIT) and Qualcomm's QNN runtime libraries from
Maven (`com.qualcomm.qti:qnn-runtime`, which `onnxruntime-android-qnn`
depends on), under Qualcomm's AI Stack License: redistributable only inside an
app, not on their own. That license also advises against what it calls
high-risk applications, those making consequential decisions; read it before
you share a build.
