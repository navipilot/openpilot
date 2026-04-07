#!/usr/bin/env python3
import argparse
import math
from dataclasses import dataclass

import numpy as np

from openpilot.tools.lib.logreader import LogReader, ReadMode
from openpilot.selfdrive.locationd.torqued import TorqueEstimator
from opendbc.car.gm.interface import NON_LINEAR_TORQUE_PARAMS


@dataclass
class ControlSample:
  v_ego: float
  steering_pressed: bool
  lat_active: bool
  saturated: bool
  actual_la: float
  desired_la: float
  desired_jerk: float
  p_term: float
  i_term: float
  f_term: float
  torque_cmd: float


def siglin_torque(lat_accel: float, params: dict[str, list[float]]) -> float:
  side = "left" if lat_accel >= 0.0 else "right"
  a, b, c, d = params[side]
  sig_input = a * lat_accel
  sig = math.copysign((1.0 / (1.0 + math.exp(-abs(sig_input))) - 0.5), sig_input)
  return (sig * b) + (lat_accel * c) + d


def summarize_control_samples(samples: list[ControlSample]) -> None:
  if not samples:
    print("No lateral torque samples found.")
    return

  v = np.array([s.v_ego for s in samples])
  steering_pressed = np.array([s.steering_pressed for s in samples], dtype=bool)
  lat_active = np.array([s.lat_active for s in samples], dtype=bool)
  saturated = np.array([s.saturated for s in samples], dtype=bool)
  actual = np.array([s.actual_la for s in samples])
  desired = np.array([s.desired_la for s in samples])
  jerk = np.array([s.desired_jerk for s in samples])
  p_term = np.array([s.p_term for s in samples])
  i_term = np.array([s.i_term for s in samples])
  f_term = np.array([s.f_term for s in samples])
  torque_cmd = np.array([s.torque_cmd for s in samples])

  base = lat_active & (~steering_pressed) & (v > 8.0)
  masks = (
    ("all", base),
    ("all_non_sat", base & (~saturated)),
    ("left", base & (~saturated) & (desired >= 0.1)),
    ("right", base & (~saturated) & (desired <= -0.1)),
    ("center", base & (~saturated) & (np.abs(desired) < 0.1)),
    ("steady_left", base & (~saturated) & (desired >= 0.1) & (np.abs(jerk) < 0.2)),
    ("steady_right", base & (~saturated) & (desired <= -0.1) & (np.abs(jerk) < 0.2)),
  )

  print("\nControlsState tracking:")
  for name, mask in masks:
    if not np.any(mask):
      continue
    print(
      f"  {name:12s} n={int(mask.sum()):5d} "
      f"mae={np.mean(np.abs(desired[mask] - actual[mask])):.4f} "
      f"bias={np.mean(actual[mask] - desired[mask]):+.4f} "
      f"|p|={np.mean(np.abs(p_term[mask])):.4f} "
      f"|i|={np.mean(np.abs(i_term[mask])):.4f} "
      f"|f|={np.mean(np.abs(f_term[mask])):.4f} "
      f"torque={np.mean(torque_cmd[mask]):+.4f}"
    )


def summarize_torque_points(car_fingerprint: str, points: np.ndarray) -> None:
  if points.size == 0:
    print("No torque-estimator points found.")
    return

  params = NON_LINEAR_TORQUE_PARAMS.get(car_fingerprint)
  if params is None:
    print(f"No siglin torque params configured for {car_fingerprint}.")
    return

  steer = points[:, 0]
  lat = points[:, 1]
  pred = np.array([siglin_torque(x, params) for x in lat])
  err = pred - steer

  print("\nTorque map residuals:")
  print(f"  all          n={points.shape[0]:5d} mae={np.mean(np.abs(err)):.4f} bias={np.mean(err):+.4f}")
  for name, mask in (("left", lat >= 0.0), ("right", lat < 0.0), ("small", np.abs(lat) < 0.3), ("mid", (np.abs(lat) >= 0.3) & (np.abs(lat) < 0.8))):
    if np.any(mask):
      print(f"  {name:12s} n={int(mask.sum()):5d} mae={np.mean(np.abs(err[mask])):.4f} bias={np.mean(err[mask]):+.4f}")

  print("\nLinearized correction against current siglin:")
  for name, mask in (("all", np.ones_like(lat, dtype=bool)), ("left", lat >= 0.0), ("right", lat < 0.0)):
    x = np.column_stack([pred[mask], np.ones(mask.sum())])
    y = steer[mask]
    scale, offset = np.linalg.lstsq(x, y, rcond=None)[0]
    fit = scale * pred[mask] + offset
    print(
      f"  {name:12s} scale={scale:.4f} offset={offset:+.4f} "
      f"mae_fit={np.mean(np.abs(fit - y)):.4f}"
    )


def main() -> None:
  parser = argparse.ArgumentParser(description="Analyze a Bolt route for lateral tuning opportunities.")
  parser.add_argument("route", help="Route name, e.g. dongle/route")
  parser.add_argument("--mode", choices=("auto", "qlog", "rlog"), default="auto")
  args = parser.parse_args()

  mode_map = {
    "auto": ReadMode.AUTO,
    "qlog": ReadMode.QLOG,
    "rlog": ReadMode.RLOG,
  }
  log_reader = LogReader(args.route, default_mode=mode_map[args.mode], sort_by_time=True)

  car_params = None
  live_torque_snapshots = []
  torque_estimator = None
  latest = {}
  control_samples: list[ControlSample] = []

  for msg in log_reader:
    which = msg.which()
    if which == "carParams" and car_params is None:
      car_params = msg.carParams
      torque_estimator = TorqueEstimator(car_params, track_all_points=True)
      continue

    if car_params is None:
      continue

    if which in ("carState", "carControl"):
      latest[which] = getattr(msg, which)
    elif which == "controlsState" and "carState" in latest and "carControl" in latest:
      lateral_state = msg.controlsState.lateralControlState
      if lateral_state.which() == "torqueState":
        torque_state = lateral_state.torqueState
        control_samples.append(ControlSample(
          v_ego=latest["carState"].vEgo,
          steering_pressed=latest["carState"].steeringPressed,
          lat_active=latest["carControl"].latActive,
          saturated=torque_state.saturated,
          actual_la=torque_state.actualLateralAccel,
          desired_la=torque_state.desiredLateralAccel,
          desired_jerk=torque_state.desiredLateralJerk,
          p_term=torque_state.p,
          i_term=torque_state.i,
          f_term=torque_state.f,
          torque_cmd=latest["carControl"].actuators.torque,
        ))
    elif which == "liveTorqueParameters" and len(live_torque_snapshots) < 8:
      live_torque_snapshots.append(msg.liveTorqueParameters)

    if torque_estimator is not None and which in ("carControl", "carOutput", "carState", "liveCalibration", "livePose", "liveDelay"):
      torque_estimator.handle_log(msg.logMonoTime / 1e9, which, getattr(msg, which))

  if car_params is None:
    raise RuntimeError("No carParams found in route.")

  torque_tune = car_params.lateralTuning.torque
  print(f"carFingerprint={car_params.carFingerprint}")
  print(f"steerRatio={car_params.steerRatio:.4f} steerActuatorDelay={car_params.steerActuatorDelay:.4f}")
  print(
    "torqueTune="
    f"latAccelFactor={torque_tune.latAccelFactor:.4f} "
    f"friction={torque_tune.friction:.4f} "
    f"latAccelOffset={torque_tune.latAccelOffset:.4f} "
    f"kp={getattr(torque_tune, 'kpDEPRECATED', 0.0):.4f} "
    f"ki={getattr(torque_tune, 'kiDEPRECATED', 0.0):.4f} "
    f"kd={getattr(torque_tune, 'kdDEPRECATED', 0.0):.4f} "
    f"kf={getattr(torque_tune, 'kfDEPRECATED', 0.0):.4f}"
  )

  if live_torque_snapshots:
    last = live_torque_snapshots[-1]
    print(
      "liveTorqueFiltered="
      f"latAccelFactor={last.latAccelFactorFiltered:.4f} "
      f"latAccelOffset={last.latAccelOffsetFiltered:.4f} "
      f"friction={last.frictionCoefficientFiltered:.4f} "
      f"useParams={last.useParams} liveValid={last.liveValid}"
    )

  summarize_control_samples(control_samples)

  points = np.array(torque_estimator.all_torque_points) if torque_estimator is not None else np.empty((0, 2))
  if torque_estimator is not None and torque_estimator.filtered_points.is_calculable():
    slope, offset, friction = torque_estimator.estimate_params()
    print(
      "\nTorqueEstimator fit:"
      f" latAccelFactor={slope:.4f}"
      f" latAccelOffset={offset:.4f}"
      f" friction={friction:.4f}"
      f" bucket_points={len(torque_estimator.filtered_points)}"
    )
  summarize_torque_points(car_params.carFingerprint, points)


if __name__ == "__main__":
  main()
