# Source installs and testing

For normal installation, use the [setup guides](README.md#set-up).
This page covers terminal installs, WSL2, and testing without a comma.

<a id="before-running-from-source"></a>
<a id="mac-apple-silicon"></a>

## Mac (Apple silicon)

Use the [Mac app](macos-app.md) from
[Releases](https://github.com/zoompilot/jetlink/releases). The server is built
in.

### From a terminal

The app's server as a command. Needs Xcode 26:

```bash
git clone https://github.com/zoompilot/jetlink.git && cd jetlink
swift build -c release --package-path JetlinkKit --product jetlink-server
caffeinate -i JetlinkKit/.build/release/jetlink-server --usb
```

- Quit the app first: one server holds the comma at a time.
- `caffeinate -i` keeps the Mac awake while it serves.
- Models and prepared engines go in `~/Library/Caches/jetlink`, about 3 GB per
  model. `--cache DIR` moves them; the app's folder is
  `~/Library/Application Support/Jetlink/cache`.
- CoreML prepares a model in about 20 seconds the first time, then loads it in
  under a second to about 10 seconds.

Options (`jetlink-server --help` lists the rest):

```bash
# TCP for a bench tool instead of the comma (see Test without a comma)
JetlinkKit/.build/release/jetlink-server --listen

# the GPU only, if another app keeps the Neural Engine busy
JetlinkKit/.build/release/jetlink-server --usb --device coreml

# prepare a model ahead of time, then exit
JetlinkKit/.build/release/jetlink-server build /path/to/big_driving_supercombo.onnx
```

Every command and option: [the server command](installation-reference.md#the-server-command).
Performance: [backends](backends.md#mac-measured).

## Linux (NVIDIA GPU)

Use the [Linux PC installer guide](linux-pc.md). To build the server yourself,
see [From a checkout](#from-a-checkout).

<a id="docker-nvidia-laptops-and-desktops"></a>

## Windows (NVIDIA GPU)

Untested. Run the Linux installer in Ubuntu 22.04 or 24.04 on WSL2:

1. Update the NVIDIA driver in Windows to 580 or newer. Never install one
   inside WSL.
2. Turn on systemd in WSL: add these lines to `/etc/wsl.conf`, then run
   `wsl --shutdown` in Windows.

   ```
   [boot]
   systemd=true
   ```

3. Run the [Linux installer](linux-pc.md#1-install-jetlink) in Ubuntu.
4. Attach the comma to WSL with
   [usbipd-win](https://learn.microsoft.com/windows/wsl/connect-usb).

Start with a [TCP test](#test-without-a-comma).

<a id="without-docker"></a>

## From a checkout

For development on Linux. Build the server tarball (in a Docker or podman
container, or natively on a Jetson or PC), then install it:

```bash
scripts/build-linux.sh linux-x86_64       # linux-aarch64 for a Jetson
sudo ./install.sh --binary dist/jetlink-server-<version>-linux-x86_64.tar.gz
```

Or run it in a terminal without installing. The host needs:

- `libcurl4`, which the server links;
- TensorRT: JetPack's on a Jetson; on a PC, TensorRT 11.3.0.99's libraries
  named with `--tensorrt-libs` (an installed PC has them in
  `/opt/jetlink/tensorrt/11.3.0.99`; otherwise see
  [installing by hand](installation-reference.md#installing-by-hand));
- the udev rule, which grants USB access without root. Replug the comma after
  installing it.

```bash
sudo install -m 644 scripts/99-jetlink-host.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules
tar -xzf dist/jetlink-server-<version>-linux-x86_64.tar.gz
jetlink-server-<version>-linux-x86_64/bin/jetlink-server --usb --backend trt \
  --tensorrt-libs /opt/jetlink/tensorrt/11.3.0.99
```

On a Jetson, leave out `--tensorrt-libs`. Without root the server cannot turn
off USB 3 link power management on the comma's port, which costs about 2 ms a
frame; the log says so.

## CPU only

Checks the protocol and model loading without a GPU. Too slow for driving. The
tarball has no onnxruntime; the build script fetches it and prints its folder:

```bash
ORT=$(scripts/build-linux.sh linux-x86_64 ort)
LD_LIBRARY_PATH=$ORT/lib jetlink-server-<version>-linux-x86_64/bin/jetlink-server \
  --backend ort --device cpu --listen
```

On a Mac, `--device cpu` does the same.

## Test without a comma

You need a large driving-model ONNX file and a server listening on TCP: on an
installed Jetson or PC, `jetlink run --listen` (it stops the service while it
runs). In a second terminal, from a checkout, replace `/path/to/big_model.onnx`
with your model:

```bash
python3 -m venv .venv
source .venv/bin/activate
python3 -m pip install -e . onnx
python3 scripts/bench_link.py --host 127.0.0.1 --onnx /path/to/big_model.onnx --rate 20
```

- It uploads the model, waits for the build, and reports round-trip latency and
  frames over the 50 ms budget at 20 Hz.
- For a server on another machine, use its wired-network IP.
- TCP has no authentication: use a trusted network.
- Wi-Fi does not meet the frame budget.

## Troubleshooting

| Problem | Check |
| --- | --- |
| Installer says the GPU is too old or has no driver | A GeForce RTX 20 series or newer, and driver 580 or newer. |
| `jetlink status` says the server stopped | `jetlink logs`. `sudo /opt/jetlink/current/bin/jetlink-server backends --backend trt --tensorrt-libs /opt/jetlink/tensorrt/11.3.0.99` says whether TensorRT loads (on a Jetson, without `--tensorrt-libs`). |
| `libcurl.so.4: no version information available` in the log on Fedora or openSUSE | Harmless: the server is linked on Ubuntu, whose libcurl versions its symbols; theirs does not. |
| USB permission error | Install the udev rule, then replug the comma. |
| TCP connection refused | Start the server with `--listen`. Check the IP and allow port 5599 through the firewall. |
| Mac looks stuck loading | On an M1 Pro, CoreML prepares in about 20 seconds and loads in up to 10. If loading takes minutes, remove the prepared engine and prepare again. |
| Link drops when the laptop sleeps | Keep it awake, powered, and open. |

Cache folders: [model management](models.md#downloads-prepared-engines-and-disk-space).
Comma-side alerts: [troubleshooting](troubleshooting.md).
