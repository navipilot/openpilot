#!/usr/bin/env python3
from openpilot.system.hardware import HARDWARE
import openpilot.system.ui.tici_reset as tici_reset
import openpilot.system.ui.mici_reset as mici_reset


def main():
  # Use actual hardware type, not UI scale/env flags, to choose reset UI.
  # This prevents mici devices from launching tici reset layouts.
  if HARDWARE.get_device_type() in ("tici", "tizi"):
    tici_reset.main()
  else:
    mici_reset.main()


if __name__ == "__main__":
  main()
