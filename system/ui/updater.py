#!/usr/bin/env python3
from pathlib import Path

from openpilot.system.hardware import HARDWARE


def _framebuffer_size() -> tuple[int, int] | None:
  fb_path = Path("/sys/class/graphics/fb0/virtual_size")
  if not fb_path.is_file():
    return None

  try:
    raw = fb_path.read_text().strip().replace(" ", "")
    width_s, height_s = raw.split(",", 1)
    return int(width_s), int(height_s)
  except Exception:
    return None


def _ui_device_type() -> str:
  reported_type = HARDWARE.get_device_type()
  fb_size = _framebuffer_size()
  if fb_size is not None and max(fb_size) < 1000:
    return "mici"
  return reported_type


def main():
  device_type = _ui_device_type()

  # The updater stack imports application sizing during module import, so patch the
  # hardware probe before importing either UI implementation.
  HARDWARE.get_device_type = lambda: device_type

  if device_type in ("tici", "tizi"):
    import openpilot.system.ui.tici_updater as updater_impl
  else:
    import openpilot.system.ui.mici_updater as updater_impl

  updater_impl.main()


if __name__ == "__main__":
  main()
