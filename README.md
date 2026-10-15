![](https://user-images.githubusercontent.com/47793918/233812617-beab2e71-57b9-479e-8bff-c3931347ca40.png)

## 🌞 What is sunnypilot?
[sunnypilot](https://github.com/sunnyhaibin/sunnypilot) is a fork of comma.ai's openpilot, an open source driver assistance system. sunnypilot offers the user a unique driving experience for over 300+ supported car makes and models with modified behaviors of driving assist engagements. sunnypilot complies with comma.ai's safety rules as accurately as possible.

## 💭 Join our Community Forum
Join the official sunnypilot community forum to stay up to date with all the latest features and be a part of shaping the future of sunnypilot!
* https://community.sunnypilot.ai/

## Documentation
https://docs.sunnypilot.ai/ is your one stop shop for everything from features to installation to FAQ about the sunnypilot

## 🚘 Running on a dedicated device in a car
First, check out this list of items you'll need to [get started](https://community.sunnypilot.ai/t/getting-started-using-sunnypilot-in-your-supported-car/251).

## Installation
### navipilot `jetlink` source branch

This branch contains the complete source snapshot of
[`moumou758/openpilot`, `dev261007XL-onemiless-jetlink`](https://github.com/moumou758/openpilot/tree/ed2edf926af0877b480652c448292e314272b410).
Dependencies are vendored as ordinary directories, not Git submodules. Their
licenses and original executable permissions are preserved.

| Dependency | Imported upstream base commit |
| --- | --- |
| `jetlink_repo` | `f843d92c0d645c1c1132295aa276a09f3d91da21` |
| `msgq_repo` | `e7396e76dadbb49e374d4b664ff6bbb43a39bcb0` |
| `opendbc_repo` | `55e4f4f6e5b78470eb82c33dc713eb35de2c5e72` |
| `panda` | `8e724b8fd62a06b8fa896d071563fa229b5ee729` |
| `rednose_repo` | `28d4a7f69e80e1c3e0d24ca0733d7daeaeade3d0` |
| `teleoprtc_repo` | `1aa8fc433bef1519a95c0700c96258c3be6dfb34` |
| `tinygrad_repo` | `fe5d3169ba4f41d0947ad174925f413cbea9d056` |
| `openpilot/sunnypilot/neural_network_data` | `03cac2d30e111e0689c0429cb8c1fe6cb5a905af` |

Models and other LFS assets are hosted in **this repository's GitHub LFS
storage**. Do not skip LFS downloads when installing; a pointer file is not a
usable model. The only fetch exclusion is Jetlink's
`tests/fixtures/pointer_f877d7a0.txt`: it is intentional pointer text used by
offline registry tests, not a model asset. For a development checkout:

```sh
git lfs install
git clone --branch jetlink --single-branch https://github.com/navipilot/openpilot.git
cd openpilot
git lfs pull
git lfs fsck
```

This is a **source branch, not a prebuilt device release**. The existing device
launcher builds it on first boot, expects the dependencies for the pinned
`AGNOS_VERSION` in `launch_env.sh`, and creates the import links to the vendored
packages. The upstream [2026-10-10 release notes](CHANGELOG.md) describe the
original C3XL-only snapshot and Tesla-specific experimental features.

This branch additionally restores **comma 3 (`tici`) code support**, including
the AR0231 camera driver, its image-processing configuration, in-phase camera
synchronization and STM32F4/DOS Panda firmware with USB communication,
MCU-specific flashing and recovery. Native device identification distinguishes `tici` from
`tizi`. Existing C3X/C3XL OX03C10/OS04C10 probing and staggered camera timing are
retained. Driving/driver-monitoring models, Jetlink selection and vehicle safety
policies are not replaced by the reference fork's versions.

**C3 hardware validation is still required.** Successful source builds and host
regressions are not evidence that a physical C3 has installed, maintained 20 Hz
camera/model operation, connected to the vehicle or driven safely. This is not a
validated device release or a blanket expansion of vehicle compatibility.
Jetlink acceleration additionally requires a compatible external
server; see [`jetlink_repo/README.md`](jetlink_repo/README.md).
Source integrity checks and desktop tests do not establish device compatibility
or driving safety. Validate installation, compilation, cameras, Panda, model
startup and Jetlink connectivity on the intended comma hardware and supported
vehicle before use; retain a known-working installation for recovery.

The AR0231 driver/register configuration was restored from
`navipilot/openpilot:Carrot` at
`31254e6538aa4a7b863077d9bcb3ef1f275cbcbd`, without importing that branch's
models, Bluetooth/AGNOS changes or vehicle-control experiments. The missing
Panda F4 platform was adapted from its Panda snapshot
`c61d998ff8fdda9cbb8a1a6d5b5d0b198085b9c4`; existing H7 firmware and the
branch's own opendbc safety code are retained. See [`panda/README.md`](panda/README.md)
for firmware build/verification commands. On AGNOS,
native camera regressions can be built and run with:

```sh
scons --minimal openpilot/system/camerad/test/test_sensors openpilot/system/camerad/camerad
openpilot/system/camerad/test/test_sensors
```

Host camera builds additionally need Clang and the SDM845 kernel's installed
camera UAPI headers. The device-only `openpilot/system/camerad/test/test_camerad.py`
checks camera liveness, exposure, frame continuity and sensor-specific phase;
run it on the actual target device before on-road use.

Next, refer to the sunnypilot community forum for [installation instructions](https://community.sunnypilot.ai/t/read-before-installing-sunnypilot/254), as well as a complete list of [Recommended Branch Installations](https://community.sunnypilot.ai/t/recommended-branch-installations/235).

## 🎆 Pull Requests
We welcome both pull requests and issues on GitHub. Bug fixes are encouraged.

Pull requests should be against the most current `master` branch.

## 📊 User Data

By default, sunnypilot uploads the driving data to comma servers. You can also access your data through [comma connect](https://connect.comma.ai/).

sunnypilot is open source software. The user is free to disable data collection if they wish to do so.

sunnypilot logs the road-facing camera, CAN, GPS, IMU, magnetometer, thermal sensors, crashes, and operating system logs.
The driver-facing camera and microphone are only logged if you explicitly opt-in in settings.

By using this software, you understand that use of this software or its related services will generate certain types of user data, which may be logged and stored at the sole discretion of comma. By accepting this agreement, you grant an irrevocable, perpetual, worldwide right to comma for the use of this data.

## Licensing

sunnypilot is released under the [MIT License](LICENSE). This repository includes original work as well as significant portions of code derived from [openpilot by comma.ai](https://github.com/commaai/openpilot), which is also released under the MIT license with additional disclaimers.

The original openpilot license notice, including comma.ai’s indemnification and alpha software disclaimer, is reproduced below as required:

> openpilot is released under the MIT license. Some parts of the software are released under other licenses as specified.
>
> Any user of this software shall indemnify and hold harmless Comma.ai, Inc. and its directors, officers, employees, agents, stockholders, affiliates, subcontractors and customers from and against all allegations, claims, actions, suits, demands, damages, liabilities, obligations, losses, settlements, judgments, costs and expenses (including without limitation attorneys’ fees and costs) which arise out of, relate to or result from any use of this software by user.
>
> **THIS IS ALPHA QUALITY SOFTWARE FOR RESEARCH PURPOSES ONLY. THIS IS NOT A PRODUCT.
> YOU ARE RESPONSIBLE FOR COMPLYING WITH LOCAL LAWS AND REGULATIONS.
> NO WARRANTY EXPRESSED OR IMPLIED.**

For full license terms, please see the [`LICENSE`](LICENSE) file.

## 💰 Support sunnypilot
If you find any of the features useful, consider becoming a [sponsor on GitHub](https://github.com/sponsors/sunnyhaibin) to support future feature development and improvements.


By becoming a sponsor, you will gain access to exclusive content, early access to new features, and the opportunity to directly influence the project's development.


<h3>GitHub Sponsor</h3>

<a href="https://github.com/sponsors/sunnyhaibin">
  <img src="https://user-images.githubusercontent.com/47793918/244135584-9800acbd-69fd-4b2b-bec9-e5fa2d85c817.png" alt="Become a Sponsor" width="300" style="max-width: 100%; height: auto;">
</a>
<br>

<h3>PayPal</h3>

<a href="https://paypal.me/sunnyhaibin0850" target="_blank">
<img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif" alt="PayPal this" title="PayPal - The safer, easier way to pay online!" border="0" />
</a>
<br></br>

Your continuous love and support are greatly appreciated! Enjoy 🥰

<span>-</span> Jason, Founder of sunnypilot
