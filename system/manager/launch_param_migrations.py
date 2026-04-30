#!/usr/bin/env python3
from __future__ import annotations

import sys

from pathlib import Path
from typing import Protocol

LONG_PITCH_KEY = "LongPitch"
STEER_KP_KEY = "SteerKP"
STEER_KP_STOCK_KEY = "SteerKPStock"

DEFAULT_STEER_KP = 0.6
LEGACY_STEER_KP = 0.7
QT_STEER_KP_PLACEHOLDER = 1.0

LAUNCH_PARAM_MIGRATION_MARKER = ".starpilot_launch_param_migrations_v2"


class ParamsLike(Protocol):
  def get_param_path(self, key: str = "") -> str: ...
  def get_bool(self, key: str) -> bool: ...
  def get_float(self, key: str) -> float: ...
  def put_bool(self, key: str, value: bool) -> None: ...
  def put_float(self, key: str, value: float) -> None: ...


def _approx_equal(lhs: float, rhs: float, tolerance: float = 1e-6) -> bool:
  return abs(lhs - rhs) <= tolerance


def _default_marker_path(params: ParamsLike) -> Path:
  return Path(params.get_param_path()) / LAUNCH_PARAM_MIGRATION_MARKER


def apply_launch_param_migrations(params: ParamsLike, marker_path: Path | None = None) -> None:
  marker = marker_path or _default_marker_path(params)
  if marker.exists():
    return

  marker.parent.mkdir(parents=True, exist_ok=True)

  if not params.get_bool(LONG_PITCH_KEY):
    params.put_bool(LONG_PITCH_KEY, True)

  steer_kp = params.get_float(STEER_KP_KEY)
  if _approx_equal(steer_kp, 0.0) or _approx_equal(steer_kp, LEGACY_STEER_KP):
    params.put_float(STEER_KP_KEY, DEFAULT_STEER_KP)

  steer_kp_stock = params.get_float(STEER_KP_STOCK_KEY)
  if (_approx_equal(steer_kp_stock, 0.0) or
      _approx_equal(steer_kp_stock, LEGACY_STEER_KP) or
      _approx_equal(steer_kp_stock, QT_STEER_KP_PLACEHOLDER)):
    params.put_float(STEER_KP_STOCK_KEY, DEFAULT_STEER_KP)

  # Initialize UsePrebuilt to True if never explicitly set, so the UI default
  # matches the shell script's default of USE_PREBUILT=1.
  if not Path(params.get_param_path("UsePrebuilt")).exists():
    params.put_bool("UsePrebuilt", True)

  marker.touch()


def main() -> int:
  try:
    from openpilot.common.params import Params

    apply_launch_param_migrations(Params())
  except Exception as exc:
    print(f"launch_param_migrations.py failed: {exc}", file=sys.stderr)
    return 1
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
