# Troubleshooting

**TAKE CONTROL while engaged:** be ready to take over. The comma stays engaged
on the small model. Investigate the connection while offroad.

| Symptom | First step | If it continues |
| --- | --- | --- |
| No **Jetlink** setting | Older builds call it **Accelerator Link**. Check **Settings > Software** for the `develop` branch. | Follow your [setup guide](README.md#set-up). |
| Waiting for comma; icon never pulses | Check Jetlink is running and the comma's **Jetlink** setting is **USB** (**iOS** for iPhone or iPad). | Try another USB 3 data cable. Use a USB-A port on Jetson or Linux PC, and a powered hub for a phone. |
| Pulsing icon | Wait while the model downloads and prepares. | Check internet access and [logs](#get-help). |
| Dimmed green icon | Disengage cruise and lateral control fully to let the model switch. | Wait for **Big Model Active**, then engage again. See [daily use](using-jetlink.md#what-to-expect-when-driving). |
| Orange icon or setup alert | Read the alert and check internet access. | Set **Jetlink** to **Off**, then back to **USB** or **iOS**. |
| Model list is empty | Connect to the internet and choose **Refresh Model List**. | In the Mac app, use **Models > Refresh**. |
| **no warp built for this camera** | Update or reinstall the `develop` branch. | Include the exact alert when asking for help. |
| Repeated link drops | Check the cable and separate power supplies. | Check cooling and sleep settings; keep the iPhone app on screen. |
| Small model only after an update | Check the release notes for a protocol change. | [Update both the comma and Jetlink](releasing.md) if required. |

Platform-specific help: [Jetson](jetson.md#troubleshooting),
[Mac](macos-app.md#troubleshooting), [Linux PC](linux-pc.md#troubleshooting),
[iPhone and iPad](iphone-app.md#troubleshooting), [Android](android-app.md#troubleshooting).

## Get help

Include your computer or phone model, comma model, driving model, exact alert,
and when the problem happened.

| Platform | Logs |
| --- | --- |
| Jetson or Linux PC | Copy `jetlink status` output and relevant lines from `jetlink logs`. Installer log: `/var/log/jetlink-install.log`. |
| Mac | Open **Logs** in Jetlink. File: `~/Library/Logs/Jetlink/server.log`. |
| iPhone, iPad, or Android | Open **Settings > Help > Logs** and use the share button. |

<details>
<summary>Extra diagnostics for a phone connection</summary>

For repeated fallbacks, also collect the comma's log over SSH:

```bash
tail -n 200 /data/log/jetlink-owner.log
```

For a direct iPhone cable that will not connect, leave it plugged in for
30 seconds, then run over SSH on the comma:

```bash
sudo /data/openpilot/jetlink_repo/scripts/comma/jetlink-root.sh check
grep 'USB-C' /data/log/jetlink-owner.log | tail -n 40
sudo dmesg | grep -iE 'usbpd|type-?c|swap|weak charger|reverse boost' | tail -n 60
```

Include the cable model and whether the comma restarted. Repeat with the
cable reversed.

</details>
