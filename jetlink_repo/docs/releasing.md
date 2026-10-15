# Updates and rollback

Update offroad with both devices powered and online. If the release notes say
the protocol changed, update the comma and Jetlink together. Otherwise, they
can be updated separately.

## Updating

| Device | What to do |
| --- | --- |
| Comma | Update in **Settings > Software** and let it reboot. |
| Jetson or Linux PC | Run `jetlink update`. Settings are kept; a failed update restores the previous server. |
| Mac | Jetlink offers each new release and installs it when you agree. **Check for Updates…** looks now. Version 0.8.1 and earlier: quit Jetlink, replace it with the new release, and reopen it. |
| iPhone or iPad | Install the newest build in TestFlight, or enable automatic updates. |
| Android | Install the new release's APK over the old one. Models and settings are kept. |

Connect the comma offroad and wait for the green icon. An update may need to
prepare the model again.

## Rolling back

To stop using Jetlink, set **Settings > Models > Jetlink** to **Off**.
Across a protocol change, roll back both the comma and server. Keep the model cache.

On an installed Jetson or Linux PC, replace `v0.7.0` with the release you need:

```bash
jetlink update --ref v0.7.0
```

It stays on that release until `jetlink update --ref latest`.

<details>
<summary>Version 0.6.0 and source installs</summary>

Updating from 0.6.0 or earlier replaces the Docker server and keeps your models.
Rolling back to 0.6.0 restores Docker. To update again from 0.6.0, use:

```bash
curl -fsSL https://raw.githubusercontent.com/zoompilot/jetlink/main/install.sh | bash -s -- --update --ref latest
```

For a source install, run `git pull`, then rebuild and install using the
[developer guide](development.md).

</details>

Manual rollback: [installation reference](installation-reference.md#versions-and-manual-rollback).

<a id="which-jetlink-to-run"></a>
<a id="maintainer-reference"></a>
<a id="installing-the-app"></a>
<a id="the-container-images"></a>
<a id="signing-secrets"></a>
