from __future__ import annotations

import json
from pathlib import Path

import pyray as rl

from openpilot.common.basedir import BASEDIR

ACTIVE_THEME_COLORS_PATH = Path(BASEDIR) / "starpilot/assets/active_theme/colors/colors.json"
STOCK_THEME_COLORS_PATH = Path(BASEDIR) / "starpilot/assets/stock_theme/colors/colors.json"

_FALLBACK_THEME_COLORS = {
  "LaneLines": (255, 255, 255, 178),
  "LeadMarker": (201, 34, 49, 255),
  "Path": (48, 255, 156, 255),
  "PathEdge": (38, 209, 125, 255),
  "Sidebar1": (255, 255, 255, 178),
  "Sidebar2": (255, 255, 255, 255),
  "Sidebar3": (255, 255, 255, 255),
}

_THEME_COLOR_CACHE: dict[str, object] = {
  "stamp": None,
  "colors": None,
}


def _file_stamp(path: Path) -> tuple[str, int | None, int | None]:
  try:
    stat = path.stat()
    return (str(path), stat.st_mtime_ns, stat.st_size)
  except OSError:
    return (str(path), None, None)


def _clamp_channel(value: object, fallback: int) -> int:
  try:
    return max(0, min(255, int(value)))
  except (TypeError, ValueError):
    return fallback


def _load_color_map(path: Path) -> dict[str, dict[str, int]]:
  try:
    with path.open() as f:
      data = json.load(f)
    if isinstance(data, dict):
      return data
  except (OSError, ValueError, TypeError):
    pass
  return {}


def _build_color(entry: object, fallback: tuple[int, int, int, int]) -> rl.Color:
  if isinstance(entry, dict):
    red, green, blue, alpha = fallback
    return rl.Color(
      _clamp_channel(entry.get("red"), red),
      _clamp_channel(entry.get("green"), green),
      _clamp_channel(entry.get("blue"), blue),
      _clamp_channel(entry.get("alpha"), alpha),
    )
  return rl.Color(*fallback)


def _load_theme_colors() -> dict[str, rl.Color]:
  stamp = (_file_stamp(STOCK_THEME_COLORS_PATH), _file_stamp(ACTIVE_THEME_COLORS_PATH))
  if stamp == _THEME_COLOR_CACHE["stamp"] and _THEME_COLOR_CACHE["colors"] is not None:
    return _THEME_COLOR_CACHE["colors"]  # type: ignore[return-value]

  stock_colors = _load_color_map(STOCK_THEME_COLORS_PATH)
  active_colors = _load_color_map(ACTIVE_THEME_COLORS_PATH)

  colors = {}
  for key, fallback in _FALLBACK_THEME_COLORS.items():
    colors[key] = _build_color(active_colors.get(key, stock_colors.get(key)), fallback)

  _THEME_COLOR_CACHE["stamp"] = stamp
  _THEME_COLOR_CACHE["colors"] = colors
  return colors


def get_theme_color(key: str, fallback: rl.Color | None = None) -> rl.Color:
  color = _load_theme_colors().get(key)
  if color is not None:
    return color
  return fallback if fallback is not None else rl.WHITE


def with_alpha(color: rl.Color, alpha: int) -> rl.Color:
  return rl.Color(color.r, color.g, color.b, max(0, min(255, int(alpha))))
