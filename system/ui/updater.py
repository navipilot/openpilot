#!/usr/bin/env python3
from openpilot.system.hardware import HARDWARE
import openpilot.system.ui.tici_updater as tici_updater
import openpilot.system.ui.mici_updater as mici_updater


def main():
  if HARDWARE.get_device_type() in ("tici", "tizi"):
    tici_updater.main()
  else:
    mici_updater.main()


if __name__ == "__main__":
  main()
