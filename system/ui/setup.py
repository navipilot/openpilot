#!/usr/bin/env python3
from openpilot.system.hardware import HARDWARE
import openpilot.system.ui.tici_setup as tici_setup
import openpilot.system.ui.mici_setup as mici_setup


def main():
  if HARDWARE.get_device_type() in ("tici", "tizi"):
    tici_setup.main()
  else:
    mici_setup.main()


if __name__ == "__main__":
  main()
