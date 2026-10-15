# Set up Jetlink on Android

**Experimental.** Phone performance is not established. Run the benchmark
before use. Set up offroad with the comma and phone online.

<a id="requirements"></a>
<a id="which-phones"></a>

## What you need

- A recent flagship phone with **Android 12 or later** and **USB 3**.
- About 3 GB free per model.
- A comma 3X or comma 4, powered separately.
- A USB 3 hub with USB-C power pass-through, a charger, and a USB 3 A-to-C cable.

Leave **Processor** on **Automatic** for initial setup. GPU performance varies
by phone; a compatible phone may still be too slow. Pixel NPU access is not
available to Jetlink. Snapdragon NPU modes require output validation before
use; see [device validation](../android/README.md#on-a-phone).

<a id="install"></a>

## 1. Install Jetlink

1. On the phone, open [Releases](https://github.com/zoompilot/jetlink/releases/latest)
   and download `Jetlink-<version>-Android.apk` under **Assets**.
2. Open the download. Allow your browser or file manager to install unknown
   apps when Android asks, then tap **Install**.
3. Open Jetlink and allow notifications so it can keep running in the background.

There is no Play Store build.

<a id="connect-the-comma"></a>

## 2. Connect the comma

1. **Install zoompilot.** After resetting the comma, enter
   **`zoompilot/develop`** as the install URL. Already on zoompilot? Select
   **develop** in **Settings > Software > Target Branch > Non-Prebuilt Branches**.
   Wait for installation, rebooting, and building to finish.
2. Set **Settings > Models > Jetlink** to **USB**.
   Leave **Big Model** at its default.
3. Connect the phone to the hub and plug the charger into the hub.
4. Connect the hub's **USB-A** port to the comma's **USB-C** port.
5. When Android asks to open Jetlink, tap **OK** and select **Always open**.
6. Wait for **Connected over USB 3**.

Stay offroad and online until the comma's home-button icon turns **green**.
It pulses while the model downloads and prepares.

On OnePlus, OPPO, and realme phones, enable **OTG connection** in Settings.
It may turn itself off when unused.

<a id="benchmark"></a>

## 3. Check performance

1. Leave the model loaded and disconnect the comma.
2. Put the phone in its car mount with charging connected.
3. Open **Benchmark** and tap **1 Minute**.

**Fast Enough** leaves time for the cable and comma. **Tight** leaves little
margin; **Too Slow** cannot sustain 20 frames per second. Run **10 Minutes**
to check for slowdowns as the phone heats up. Repeat after changing the model
or processor.

The benchmark excludes cable latency. Passing does not validate operation
with the comma connected. Read [daily use](using-jetlink.md) before driving.

<a id="status"></a>
<a id="limits"></a>
<a id="heat"></a>

## While using Jetlink

Jetlink runs with the screen off while its notification shows. **Stop** in the
notification stops the server. Keep the phone charging, out of direct sun,
and out of thick cases.

<a id="prepare-a-model-before-you-drive"></a>

To download a model ahead of time, see [Models](models.md#prepare-ahead-of-time-optional).

## Troubleshooting

| Problem | First step |
| --- | --- |
| App stops in the background | Set its battery use to **Unrestricted** in Android settings. |
| Connection says **USB 2** | Check that the phone, hub, and cable all support USB 3. |
| Android keeps asking to open Jetlink | Select **Always open** in the USB prompt. |
| Slow frames | Let the phone cool, then repeat the 10-minute benchmark. |
| Release APK will not replace a self-built app | They use different signing keys. Uninstall the old app first; this deletes its models. |

<a id="logs"></a>

Logs: **Settings > Help > Logs**. Use the share button when
[asking for help](troubleshooting.md#get-help).

<details>
<summary>Optional settings</summary>

## Settings

**Keep NPU Awake** and **Keep CPU Awake** can reduce delays between frames but
use more power. Turn off **Keep NPU Awake** to reduce heat if performance allows.

Use **Restart Server** to restart with the same settings, or **Stop Server**
to stop until you choose **Start Server**.

Tap **Version** under **About** seven times to show **Developer**, with the
port and Wi-Fi address that bench tools on a Mac use. Jetlink listens on that
port only while **Developer** shows. The comma does not need it. Tap
**Version** seven times again to hide it.

</details>
