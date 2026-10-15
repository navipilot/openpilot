# Cables and power

## USB connection

Use USB 3 throughout: the computer port, cable, adapters, and any hub.
Charge-only cables do not work; USB 2 adds latency.

| Computer | Connection to the comma's USB-C port |
| --- | --- |
| Jetson or Linux PC | USB-A to USB-C, from the computer's USB-A port |
| Mac | USB-C to USB-C |
| iPhone or iPad | Powered USB 3 USB-C hub and data cable; direct USB-C can be unreliable |
| Android | USB-A to USB-C, from a USB 3 hub with USB-C power pass-through |

A USB-C to USB-A adapter with a USB-A to USB-C cable is an alternative for a
phone, but does not keep it charging. The comma's USB-C port cannot serve
Jetlink and chestnut at the same time.

On a direct cable the comma does not charge an iPhone unless you turn it on:
some iPhones lose the link once the comma powers them. To try it, run
`echo -n 1 > /data/params/d/JetlinkChargePhone` on the comma and replug the
cable. If the link goes down while the phone charges, the comma stops
charging it until the next plug. A powered hub charges the phone without it.

<a id="what-the-comma-presents"></a>

## Connection setting

Set **Settings > Models > Jetlink** while offroad:

- **USB:** Jetson, Linux PC, Mac, or Android.
- **iOS:** iPhone or iPad.

<a id="bus-speed"></a>

## Power requirements

Power the Jetson and comma separately. The Jetson's supply and cable must
deliver at least 25 W and tolerate voltage drops at engine start. **Always on**
or **Switched**: see the [power setup table](jetson.md#1-choose-your-power-setup).

### Recommended Jetson power setup

Orin Nano Super devkit: a **straight 12 V-to-DC barrel adapter** on a supply
that **stays on when the ignition is off** (such as an always-on 12 V accessory
socket), with Jetson **deep sleep** enabled.

<img src="images/jetson-12v-dc-adapter.jpg" width="320" alt="Example of a 12 V car accessory socket plug to DC barrel adapter cable">

- Plug: **5.5 mm outer / 2.5 mm inner, center-positive**. Check this when
  buying; the photo shows the style only. Other carrier boards may differ.
- Confirm the socket stays powered after ignition off, including after any
  delayed shutoff.
- Choose **Always on** in the installer for deep sleep; `jetlink setup` changes
  an existing setup.

<a id="always-on-supply-and-suspend"></a>

<a id="powering-off-with-the-comma"></a>

Parking, starting, and how battery-protection shutdown differs from sleep:
[Choose your power setup](jetson.md#1-choose-your-power-setup).

<a id="tcp"></a>
<a id="link-protocol"></a>
<a id="custom-usb-integrations"></a>

## Advanced reference

[TCP testing](platforms.md#test-without-a-comma) ·
[Link protocol](link-protocol.md) ·
[Custom USB integrations](installation-reference.md#custom-usb-integrations)
