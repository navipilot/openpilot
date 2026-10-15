# Set up Jetlink on a Linux PC

Set up offroad. Keep the comma and PC online, and the PC powered and awake.

## What you need

- An NVIDIA GeForce RTX 20 series or newer GPU with driver 580 or newer.
- Ubuntu 22.04 or 24.04. Other supported distributions are listed below.
- A comma 3X or comma 4, powered separately.
- A USB 3 A-to-C data cable and a USB-A port on the PC.

## 1. Install Jetlink

Open a terminal on the PC and run:

```bash
curl -fsSL https://raw.githubusercontent.com/zoompilot/jetlink/main/install.sh | bash
```

Allow 10–30 minutes. Choose whether Jetlink should start with the PC.
On Ubuntu and Arch, the installer can install the NVIDIA driver. If prompted,
restart and run the installer again. On other distributions, follow the driver
instructions it prints, then rerun it.

To use the web page from your phone, choose a port and password when prompted.
Port **5600** is the default; **0** disables it.

## 2. Connect the comma

1. **Install zoompilot.** After resetting the comma, enter
   **`zoompilot/develop`** as the install URL. Already on zoompilot? Select
   **develop** in **Settings > Software > Target Branch > Non-Prebuilt Branches**.
   Wait for installation, rebooting, and building to finish.
2. Set **Settings > Models > Jetlink** to **USB**.
   Leave **Big Model** at its default.
3. Connect the PC's **USB-A** port to the comma's **USB-C** port.

Stay offroad and online until the comma's home-button icon turns **green**.
It pulses while the model downloads and prepares.

Run `jetlink status` to check the connection. Keep the PC awake while driving.
Read [daily use](using-jetlink.md) before driving.

## Troubleshooting

| Problem | First step |
| --- | --- |
| Installer stopped | Follow its error message, then run it again. |
| GPU or driver rejected | Check for an RTX 20 series or newer GPU and driver 580 or newer. |
| Server stopped | Run `jetlink logs` and check the error. |
| Comma does not connect | Run `jetlink status`, then check the cable and the comma's **Jetlink** setting. |

[More troubleshooting](troubleshooting.md) · [Updates](releasing.md) ·
[Web page](using-jetlink.md#web-page)

<details>
<summary>Other Linux distributions</summary>

The installer also supports Debian 12, Fedora, Arch, and openSUSE Tumbleweed.
These have not been tested on hardware. Derivatives follow their base distribution.

You need systemd and glibc 2.35 or newer; Debian 11 and RHEL 9 are too old.
The installer uses apt, dnf, pacman, or zypper. With another package manager,
install `curl`, `git`, `unzip`, and libcurl first.

For WSL2 or a source install, see [advanced setup](platforms.md).

</details>
