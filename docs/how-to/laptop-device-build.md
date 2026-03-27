# Laptop Device Build (Mac/Linux)

This flow builds **device-target (`larch64`) binaries on your laptop** using a Linux/aarch64 container and a synced comma sysroot.

For the full StarPilot branch workflow, including host-native shorthand tools such as `./dev`, `./c3`, `./c4`, and `./raybig`, see [tools/STARPILOT_DEVELOPMENT.md](../../tools/STARPILOT_DEVELOPMENT.md).

## Prerequisites

- Docker Desktop (or Podman) with Linux/aarch64 support.
- `rsync` and `ssh` on your laptop.
- Either:
  - access to a comma device over SSH, or
  - internet access to download AGNOS system image for sysroot extraction.

## One-time setup

Fast path (no physical comma):

```bash
cd /path/to/frogpilot
scripts/laptop_device_build.sh setup
```

Equivalent wrapper:

```bash
scripts/starpilot_build_flow.sh laptop-setup
```

### Option A: no physical comma (AGNOS-based)

```bash
cd /path/to/frogpilot
scripts/laptop_device_build.sh build-image
scripts/laptop_device_build.sh setup-sysroot-agnos
```

### Option B: copy sysroot from a comma device

```bash
cd /path/to/frogpilot
scripts/laptop_device_build.sh setup-sysroot <device-ip> comma 22
scripts/laptop_device_build.sh build-image
```

Optional all-in-one with physical device sysroot:

```bash
scripts/laptop_device_build.sh setup <device-ip> comma 22
```

## Build device-compatible artifacts

```bash
cd /path/to/frogpilot
scripts/laptop_device_build.sh build
```

Equivalent wrapper:

```bash
scripts/starpilot_build_flow.sh laptop-device
```

This runs:

- `uv sync` into `/work/.venv-linux-arm64` (inside container)
- full `scons` with:
  - `SP_FORCE_ARCH=larch64`
  - `SP_FORCE_TICI=1`
  - `SP_TICI_SYSROOT=/opt/tici-sysroot`
- `touch prebuilt`

To run `scons` targets explicitly in the same device-compatible environment:

```bash
scripts/laptop_device_build.sh scons selfdrive/ui/ui
```

On macOS, once `.comma_sysroot` is present, plain `scons ...` auto-routes to this containerized device build.
Set `SP_DISABLE_AUTO_DEVICE_SCONS=1` to force native host `scons`.

## Quick checks

```bash
scripts/laptop_device_build.sh doctor
```

If `doctor` fails, fix the missing runtime/sysroot step before running `build`.

## One-command manager launch on desktop

`./launch_openpilot.sh` now auto-routes desktop/mac launches to the containerized larch64 manager path:

- runs `scripts/laptop_device_build.sh doctor`
- runs `setup` automatically if prerequisites are missing
- runs container manager launch, auto-building missing runtime artifacts first

Useful overrides:

- `SP_SKIP_DOCKER_AUTO_BUILD=1 ./launch_openpilot.sh` to skip auto-build.
- `SP_DOCKER_BUILD_JOBS=12 ./launch_openpilot.sh` to control build parallelism.
- `SP_FORCE_DEVICE_LAUNCH=1 ./launch_openpilot.sh` to force legacy device launch path.

Note:

- `manager` mode requires the container runtime machine to be `aarch64`.
- If your built image runs as `x86_64`, build mode still works for artifact generation, but manager launch in-container is not supported.

Preferred host-side shorthand commands on this branch:

```bash
./c3
./c4
./raybig
./dev replay
./dev cabana
./dev plotjuggler
```

These commands use the isolated `.host_runtime` cache so host-native artifacts do not churn the main tree.

Legacy direct script entrypoints still exist if needed:

```bash
scripts/launch_ui_desktop.sh
scripts/launch_ui_c4_desktop.sh
scripts/launch_ui_raybig_desktop.sh
```
