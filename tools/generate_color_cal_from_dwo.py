#!/usr/bin/env python3
import argparse
import hashlib
import math
import struct
from pathlib import Path


EPS = 1e-9
COLOR_CAL_FMT = "<13e"
IGNORED_TAIL = b"\x00\x00\x00\x00\x00"
SAMPLES = [i / 47.0 for i in range(1, 48)]
WEIGHTS = [0.6 + 1.8 * x + (8.0 if x >= 0.985 else 0.0) for x in SAMPLES]


def parse_dwo_gamma_curves(path: str) -> list[list[float]]:
  rows: list[list[float]] = []
  with open(path) as f:
    for line in f:
      stripped = line.strip()
      if not stripped:
        continue
      values = [int(x, 16) for x in stripped.split()]
      if len(values) < 2:
        raise ValueError(f"{path}: gamma row must contain at least two values")
      if values[0] > values[-1]:
        values.reverse()
      max_value = max(values)
      if max_value <= 0:
        raise ValueError(f"{path}: gamma row max must be positive")
      rows.append([v / max_value for v in values])

  if len(rows) != 3:
    raise ValueError(f"{path}: expected 3 gamma rows, got {len(rows)}")

  row_len = len(rows[0])
  if any(len(row) != row_len for row in rows):
    raise ValueError(f"{path}: gamma rows must all have the same length")

  return rows


def interp(curve: list[float], x: float) -> float:
  x = min(max(x, 0.0), 1.0)
  pos = x * (len(curve) - 1)
  lo = int(pos)
  hi = min(lo + 1, len(curve) - 1)
  frac = pos - lo
  return curve[lo] * (1.0 - frac) + curve[hi] * frac


def invert_interp(curve: list[float], y: float) -> float:
  y = min(max(y, 0.0), 1.0)
  if y <= curve[0]:
    return 0.0
  if y >= curve[-1]:
    return 1.0
  lo = 0
  hi = len(curve) - 1
  while hi - lo > 1:
    mid = (lo + hi) // 2
    if curve[mid] < y:
      lo = mid
    else:
      hi = mid
  y0 = curve[lo]
  y1 = curve[hi]
  if abs(y1 - y0) < EPS:
    return lo / (len(curve) - 1)
  frac = (y - y0) / (y1 - y0)
  return (lo + frac) / (len(curve) - 1)


def apply_target_panel(curve: list[float], a: float, gamma: float, x: float) -> float:
  corrected = min(max(a * (x ** gamma), 0.0), 1.0)
  return interp(curve, corrected)


def fit_channel_amplitude(reference_curve: list[float], target_curve: list[float], gamma: float) -> tuple[float, float]:
  # Search for the per-channel amplitude that makes the target panel resemble the
  # reference panel once Weston applies a common gamma pre-distortion.
  lo = 0.85
  hi = 1.15
  best_a = 1.0
  best_err = float("inf")
  for _ in range(5):
    steps = 60
    step = (hi - lo) / steps
    for idx in range(steps + 1):
      a = lo + idx * step
      err = 0.0
      for x, w in zip(SAMPLES, WEIGHTS):
        ref = interp(reference_curve, x)
        actual = apply_target_panel(target_curve, a, gamma, x)
        diff = actual - ref
        err += w * diff * diff
      if err < best_err:
        best_err = err
        best_a = a
    lo = max(0.6, best_a - step * 1.5)
    hi = min(1.4, best_a + step * 1.5)
  return best_a, best_err


def fit_color_cal(reference_curves: list[list[float]], target_curves: list[list[float]]) -> tuple[float, list[float], list[float]]:
  best_gamma = 1.0
  best_amps = [1.0, 1.0, 1.0]
  best_err = float("inf")

  gamma_values = [0.82 + 0.004 * i for i in range(96)]  # 0.82 .. 1.20
  for gamma in gamma_values:
    amps: list[float] = []
    err = 0.0
    for ref_curve, target_curve in zip(reference_curves, target_curves):
      amp, channel_err = fit_channel_amplitude(ref_curve, target_curve, gamma)
      amps.append(amp)
      err += channel_err
    # Bias toward identity when multiple fits are similar so a "good" panel stays unchanged.
    identity_penalty = sum((amp - 1.0) ** 2 for amp in amps) * 3.0 + (gamma - 1.0) ** 2 * 12.0
    err += identity_penalty
    if err < best_err:
      best_err = err
      best_gamma = gamma
      best_amps = amps

  # The calibration file stores white-balance gains; Weston multiplies by their inverse.
  wb_gains = []
  for amp in best_amps:
    amp = max(amp, 0.6)
    gain = amp ** (-2.2 / best_gamma)
    wb_gains.append(min(max(gain, 0.7), 1.45))

  return best_gamma, [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0], wb_gains


def write_color_cal(path: str, gamma: float, ccm: list[float], wb_gains: list[float]) -> None:
  payload = struct.pack(COLOR_CAL_FMT, gamma, *ccm, *wb_gains)
  Path(path).write_bytes(payload + IGNORED_TAIL)


def build_parser() -> argparse.ArgumentParser:
  parser = argparse.ArgumentParser(description="Generate a Weston color_cal blob from dwo_gamma_curves.")
  parser.add_argument("--reference", required=True, help="Reference dwo_gamma_curves file from a good panel")
  parser.add_argument("--input", required=True, help="Device dwo_gamma_curves file to adapt")
  parser.add_argument("--output", required=True, help="Output color_cal path")
  parser.add_argument("--print-only", action="store_true", help="Print the generated parameters instead of writing")
  return parser


def main() -> int:
  args = build_parser().parse_args()
  reference_curves = parse_dwo_gamma_curves(args.reference)
  input_curves = parse_dwo_gamma_curves(args.input)
  gamma, ccm, wb_gains = fit_color_cal(reference_curves, input_curves)

  if args.print_only:
    print(f"gamma={gamma:.6f}")
    print("ccm=" + " ".join(f"{v:.6f}" for v in ccm))
    print("wb=" + " ".join(f"{v:.6f}" for v in wb_gains))
  else:
    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)
    write_color_cal(str(output), gamma, ccm, wb_gains)

  digest = hashlib.sha256(Path(args.input).read_bytes()).hexdigest()[:12]
  print(f"source_sha256={digest}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
