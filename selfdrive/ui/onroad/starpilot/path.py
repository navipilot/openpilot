from __future__ import annotations

import colorsys

import numpy as np
import pyray as rl

from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.system.ui.lib.shader_polygon import draw_polygon, Gradient

_METRICS_FONT = None
_METRICS_FONT_SIZE = 45


def _get_metrics_font():
  global _METRICS_FONT
  if _METRICS_FONT is None or _METRICS_FONT.baseSize != _METRICS_FONT_SIZE:
    if _METRICS_FONT is not None:
      rl.unload_font(_METRICS_FONT)
    _METRICS_FONT = rl.load_font_ex("fonts/Inter-SemiBold.ttf", _METRICS_FONT_SIZE, None, 256)
  return _METRICS_FONT


def _hsla_to_color(h: float, s: float, l: float, a: float) -> rl.Color:
  rgb = colorsys.hls_to_rgb(h, l, s)
  return rl.Color(int(rgb[0] * 255), int(rgb[1] * 255), int(rgb[2] * 255), int(a * 255))


def _make_gradient(hue: float) -> Gradient:
  return Gradient(
    start=(0.0, 1.0),
    end=(0.0, 0.0),
    colors=[
      _hsla_to_color(hue, 0.75, 0.5, 0.4),
      _hsla_to_color(hue, 0.75, 0.5, 0.35),
      _hsla_to_color(hue, 0.75, 0.5, 0.0),
    ],
    stops=[0.0, 0.5, 1.0],
  )


def _draw_text_with_outline(text: str, x: float, y: float, font, font_size: int):
  pos = rl.Vector2(x, y)
  rl.draw_text_ex(font, text, rl.Vector2(pos.x - 1, pos.y - 1), font_size, 0, rl.BLACK)
  rl.draw_text_ex(font, text, rl.Vector2(pos.x + 1, pos.y - 1), font_size, 0, rl.BLACK)
  rl.draw_text_ex(font, text, rl.Vector2(pos.x - 1, pos.y + 1), font_size, 0, rl.BLACK)
  rl.draw_text_ex(font, text, rl.Vector2(pos.x + 1, pos.y + 1), font_size, 0, rl.BLACK)
  rl.draw_text_ex(font, text, pos, font_size, 0, rl.WHITE)


def render_adjacent_paths(renderer) -> None:
  """Draw left and right adjacent lane paths with gradient coloring.

  Qt reference: paintAdjacentPaths in starpilot_annotated_camera.cc:337-392

  Gradient color is based on lane width ratio compared to LaneDetectionWidth toggle.
  hue = (ratio^2) * (120/360), where ratio = laneWidth / lane_detection_width
  Gradient: HSL(hue, 0.75, 0.5, 0.4) at bottom, 0.35 at mid, 0.0 at top.

  If AdjacentPathMetrics is enabled, show lane width text on each path.
  """
  sm = ui_state.sm
  if sm.recv_frame["starpilotPlan"] < ui_state.started_frame:
    return

  plan = sm["starpilotPlan"]
  lane_width_left = float(plan.laneWidthLeft)
  lane_width_right = float(plan.laneWidthRight)
  lane_detection_width = renderer._params.get_float("LaneDetectionWidth") if renderer._params.get("LaneDetectionWidth") else 3.5

  vertices = renderer._adjacent_path_vertices
  if not vertices or len(vertices) < 2:
    return

  show_metrics = renderer._params.get_bool("AdjacentPathMetrics")
  rect = renderer._rect

  distance_conversion = 3.28084 if not ui_state.is_metric else 1.0
  unit = "ft" if not ui_state.is_metric else "m"
  font = _get_metrics_font()

  for i, (verts, lane_width) in enumerate(zip(vertices, [lane_width_left, lane_width_right], strict=True)):
    if verts.size < 4 or lane_width == 0.0:
      continue

    ratio = np.clip(lane_width / lane_detection_width, 0.0, 1.0)
    hue = (ratio * ratio) * (120.0 / 360.0)
    gradient = _make_gradient(hue)
    draw_polygon(rect, verts, gradient=gradient)

    if show_metrics:
      is_left = i == 0
      mid_index = len(verts) // 2
      anchor_idx = mid_index // 2 if is_left else mid_index + (len(verts) - mid_index) // 2
      anchor = verts[anchor_idx]

      text = f"{lane_width * distance_conversion:.2f}{unit}"
      text_width = rl.measure_text_ex(font, text, _METRICS_FONT_SIZE, 0).x
      text_height = rl.measure_text_ex(font, text, _METRICS_FONT_SIZE, 0).y

      text_x = anchor[0] - text_width if is_left else anchor[0]
      text_y = anchor[1] - text_height / 2 + text_height * 0.75
      _draw_text_with_outline(text, text_x, text_y, font, _METRICS_FONT_SIZE)


def render_blind_spot_path(renderer) -> None:
  """Draw red gradient overlay on adjacent lanes when vehicle detected in blind spot.

  Qt reference: paintBlindSpotPath in starpilot_annotated_camera.cc:394-411

  Red gradient: HSL(0, 0.75, 0.5, 0.4) at bottom, 0.35 at mid, 0.0 at top.
  Only draws on the side where blind spot is active.
  """
  sm = ui_state.sm
  if sm.recv_frame["carState"] < ui_state.started_frame:
    return

  if not renderer._params.get_bool("BlindSpotPath"):
    return

  car_state = sm["carState"]
  blindspot_left = bool(car_state.leftBlindspot)
  blindspot_right = bool(car_state.rightBlindspot)

  if not blindspot_left and not blindspot_right:
    return

  vertices = renderer._adjacent_path_vertices
  if not vertices or len(vertices) < 2:
    return

  gradient = _make_gradient(0.0)
  rect = renderer._rect

  if blindspot_left and vertices[0].size >= 4:
    draw_polygon(rect, vertices[0], gradient=gradient)

  if blindspot_right and vertices[1].size >= 4:
    draw_polygon(rect, vertices[1], gradient=gradient)


def render_path_edges(renderer) -> None:
  """Draw colored edge strips along the driving path.

  Qt reference: paintPathEdges in starpilot_annotated_camera.cc:732-769

  Path edges are the area between track_edge_vertices (outer) and track_vertices (inner).
  Color is based on current mode:
  - If switchback_mode_enabled: use STATUS_SWITCHBACK_MODE_ENABLED color
  - If always_on_lateral_active: use STATUS_ALWAYS_ON_LATERAL_ACTIVE color
  - If conditional_status == 1: use STATUS_CEM_DISABLED color
  - If experimental_mode: use STATUS_EXPERIMENTAL_MODE_ENABLED color
  - If traffic_mode_enabled: use STATUS_TRAFFIC_MODE_ENABLED color
  - If color_scheme != "stock" and path_edges_color is set: use that color
  - Else: stock green gradient HSL(148/360, 0.94, 0.41, 0.4) → HSL(112/360, 1.0, 0.54, 0.35) → transparent

  The path_edges_color param is a hex string like "#178644".
  """
  outer = renderer._track_edge_vertices
  inner = renderer._path.projected_points

  if outer.size < 4 or inner.size < 4:
    return

  edge_strip = np.vstack([outer, inner[::-1]])

  if ui_state.switchback_mode_enabled:
    base_color = rl.Color(139, 108, 197, 241)
  elif ui_state.always_on_lateral_active:
    base_color = rl.Color(10, 186, 181, 241)
  elif ui_state.conditional_status == 1:
    base_color = rl.Color(255, 255, 0, 241)
  elif renderer._experimental_mode:
    base_color = rl.Color(218, 111, 37, 241)
  elif ui_state.traffic_mode_enabled:
    base_color = rl.Color(201, 34, 49, 241)
  else:
    color_scheme = renderer._params.get("ColorScheme")
    if color_scheme and color_scheme.decode("utf-8") != "stock":
      path_edges_color = renderer._params.get("PathEdgesColor")
      if path_edges_color:
        hex_str = path_edges_color.decode("utf-8").lstrip("#")
        if len(hex_str) == 6:
          r = int(hex_str[0:2], 16)
          g = int(hex_str[2:4], 16)
          b = int(hex_str[4:6], 16)
          base_color = rl.Color(r, g, b, 241)
        else:
          base_color = rl.Color(23, 134, 68, 241)
      else:
        base_color = rl.Color(23, 134, 68, 241)
    else:
      base_color = None

  if base_color is not None:
    gradient = Gradient(
      start=(0.0, 1.0),
      end=(0.0, 0.0),
      colors=[
        rl.Color(base_color.r, base_color.g, base_color.b, int(255 * 0.4)),
        rl.Color(base_color.r, base_color.g, base_color.b, int(255 * 0.35)),
        rl.Color(base_color.r, base_color.g, base_color.b, 0),
      ],
      stops=[0.0, 0.5, 1.0],
    )
  else:
    gradient = Gradient(
      start=(0.0, 1.0),
      end=(0.0, 0.0),
      colors=[
        _hsla_to_color(148.0 / 360.0, 0.94, 0.41, 0.4),
        _hsla_to_color(112.0 / 360.0, 1.0, 0.54, 0.35),
        _hsla_to_color(112.0 / 360.0, 1.0, 0.54, 0.0),
      ],
      stops=[0.0, 0.5, 1.0],
    )

  draw_polygon(renderer._rect, edge_strip, gradient=gradient)
