# Using Jetlink

## Start

Keep the computer powered, awake, and connected to the comma over USB.
On Mac, open Jetlink; closing its window keeps it running, quitting stops it.
On iPhone or iPad, keep Jetlink on screen and the device unlocked.
On Android, Jetlink keeps running while its notification shows.

## Check the comma's icon

| Icon | What to do |
| --- | --- |
| Pulsing | Wait for the model to download and prepare. |
| Green | Ready when parked; active when driving. |
| Dimmed green | Disengage fully to switch, as described below. |
| Orange | Read the home-screen alert. See [troubleshooting](troubleshooting.md). |

## What to expect when driving

The small model drives until the large model is ready. To switch, **turn cruise
fully off, including lateral control if it stays on without cruise**. You do
not need to stop the car.

Wait for **Big Model Active**, then engage again. The switch takes about a
second; **Big Model Loading** means cruise cannot engage yet.

**TAKE CONTROL: Big model lost** means the link dropped or lagged. Openpilot
stays engaged on the small model. **Be ready to take over**, especially during
the first few seconds. Jetlink reconnects automatically; switch back as above.

<a id="parking-and-waking-a-jetson"></a>

## Park

With a Jetson configured for **Always on**, leave power and USB connected.
It sleeps after a few minutes and the comma wakes it when the car starts.
After a battery-protection shutdown, press the Jetson's power button or
reconnect power; starting the car will not restart it.

<a id="choose-a-model"></a>
<a id="update-or-stop-using-jetlink"></a>

## Change a model, update, or stop

- [Choose or prepare a model](models.md).
- [Update or roll back](releasing.md).
- Stop using Jetlink: set **Settings > Models > Jetlink** to **Off**.

<details>
<summary>Use the Jetson or Linux PC web page</summary>

<a id="status-page"></a>

## Web page

Check the connection and frame times, manage models, or change settings from
your phone. Settings, updates, and model changes require the car to be parked.

1. Enable the page in the installer or `jetlink setup`. The default port is
   **5600**; **0** turns it off. Set a password or use the generated one.
2. Put the phone and computer on the same trusted network, such as the comma's
   hotspot or home Wi-Fi. The page uses plain HTTP.
3. Run `jetlink status` for the address. Open it in your browser and sign in.

To join the comma's hotspot, turn on tethering in its network settings, then
run on the Jetson or PC:

```bash
sudo nmcli dev wifi connect "<hotspot name>" password "<password>"
```

Forgot the password? Run `sudo jetlink password`.

</details>
