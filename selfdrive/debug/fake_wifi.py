#!/usr/bin/env python3
import argparse
import time

from cereal import log, messaging
from openpilot.system.hardware import HARDWARE

NETWORK_TYPES = {
  "none": log.DeviceState.NetworkType.none,
  "wifi": log.DeviceState.NetworkType.wifi,
  "ethernet": log.DeviceState.NetworkType.ethernet,
  "cell2G": log.DeviceState.NetworkType.cell2G,
  "cell3G": log.DeviceState.NetworkType.cell3G,
  "cell4G": log.DeviceState.NetworkType.cell4G,
  "cell5G": log.DeviceState.NetworkType.cell5G,
}

NETWORK_STRENGTHS = {
  "unknown": log.DeviceState.NetworkStrength.unknown,
  "poor": log.DeviceState.NetworkStrength.poor,
  "moderate": log.DeviceState.NetworkStrength.moderate,
  "good": log.DeviceState.NetworkStrength.good,
  "great": log.DeviceState.NetworkStrength.great,
}


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Publish fake deviceState network info for desktop UI demos.")
  parser.add_argument("--network", choices=NETWORK_TYPES.keys(), default="wifi", help="network type to publish")
  parser.add_argument("--strength", choices=NETWORK_STRENGTHS.keys(), default="great", help="network strength to publish")
  parser.add_argument("--started", action="store_true", help="publish started=true (onroad)")
  parser.add_argument("--metered", action="store_true", help="publish networkMetered=true")
  parser.add_argument("--interval", type=float, default=0.2, help="publish interval in seconds")
  return parser.parse_args()


def main() -> None:
  args = parse_args()

  pm = messaging.PubMaster(["deviceState"])
  interval = max(args.interval, 0.05)
  network_type = NETWORK_TYPES[args.network]
  network_strength = NETWORK_STRENGTHS[args.strength]

  while True:
    msg = messaging.new_message("deviceState")
    ds = msg.deviceState

    ds.deviceType = HARDWARE.get_device_type()
    ds.started = args.started
    ds.networkType = network_type
    ds.networkStrength = network_strength
    ds.networkMetered = args.metered
    ds.lastAthenaPingTime = time.monotonic_ns()

    # Keep common sidebar values reasonable for desktop demos.
    ds.freeSpacePercent = 80
    ds.memoryUsagePercent = 35
    ds.maxTempC = 45

    pm.send("deviceState", msg)
    time.sleep(interval)


if __name__ == "__main__":
  main()
