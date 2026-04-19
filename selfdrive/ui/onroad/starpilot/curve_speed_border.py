import math
import pyray as rl
from openpilot.selfdrive.ui import UI_BORDER_SIZE
from openpilot.selfdrive.ui.ui_state import ui_state

# Amber palette
AMBER = rl.Color(251, 191, 36, 255)

# Filament
_FILAMENT_WIDTH = 3.0
_FILAMENT_PASSES = 3
_FILAMENT_SPEED_BASE = 2.0
_FILAMENT_SEGMENTS = 120

# Glow
_GLOW_LAYERS = 8
_GLOW_MAX_ALPHA = 60
_BREATHING_PERIOD = 4.5  # seconds — resting heart rate ~0.22 Hz

# Curvature mapping
_CURVATURE_MIN = 0.001
_CURVATURE_MAX = 0.1


# ── State ──────────────────────────────────────────────────────────────

def _csc_state():
  """Read CSC state from starpilotPlan. Returns dict or None if stale/hidden."""
  sm = ui_state.sm
  if sm.recv_frame["starpilotPlan"] < ui_state.started_frame:
    return None
  if sm.recv_frame["carState"] < ui_state.started_frame:
    return None
  if sm.recv_frame["controlsState"] < ui_state.started_frame:
    return None

  plan = sm["starpilotPlan"]
  if plan.speedLimitChanged or not ui_state.params.get_bool("ShowCSCStatus"):
    return None

  car_state = sm["carState"]
  v_cruise = car_state.vCruiseCluster
  if v_cruise == 0.0:
    v_cruise = sm["controlsState"].vCruiseDEPRECATED
  is_cruise_set = 0 < v_cruise < 255

  return {
    'training': plan.cscTraining,
    'active': is_cruise_set and plan.cscControllingSpeed,
    'curvature': plan.roadCurvature,
  }


# ── Math ───────────────────────────────────────────────────────────────

def _intensity(curvature: float) -> float:
  """Map abs(curvature) from [CURVATURE_MIN, CURVATURE_MAX] → [0, 1]."""
  return max(0.0, min(1.0, (abs(curvature) - _CURVATURE_MIN) / (_CURVATURE_MAX - _CURVATURE_MIN)))


def _perimeter(r: rl.Rectangle, roundness: float) -> list[tuple[float, float]]:
  """Generate evenly-spaced points around a rounded rectangle perimeter."""
  rx = roundness * min(r.width, r.height) / 2.0
  rx = min(rx, r.width / 2.0, r.height / 2.0)
  x, y, w, h = r.x, r.y, r.width, r.height

  # Corner centers
  corners = [
    (x + w - rx, y + rx),    # top-right
    (x + w - rx, y + h - rx), # bottom-right
    (x + rx, y + h - rx),    # bottom-left
    (x + rx, y + rx),         # top-left
  ]

  # Straight edge lengths (connecting corner tangent points)
  edges = [
    w - 2 * rx,  # top
    h - 2 * rx,  # right
    w - 2 * rx,  # bottom
    h - 2 * rx,  # left
  ]

  arc_len = math.pi * rx / 2.0
  total = 4 * arc_len + sum(edges)
  if total <= 0:
    return [(x, y)] * _FILAMENT_SEGMENTS

  points = []
  for i in range(_FILAMENT_SEGMENTS):
    d = (i / _FILAMENT_SEGMENTS) * total
    # Walk corners and edges in order
    for ci in range(4):
      if d < arc_len:
        frac = d / arc_len
        # Sweep 90° clockwise: 270→0, 0→90, 90→180, 180→270
        angle = math.radians(270 + ci * 90 + frac * 90)
        cx, cy = corners[ci]
        points.append((cx + rx * math.cos(angle), cy + rx * math.sin(angle)))
        break
      d -= arc_len

      edge = edges[ci]
      if d < edge:
        frac = d / edge
        cx, cy = corners[ci]
        # Next corner in CW order
        nx, ny = corners[(ci + 1) % 4]
        # Tangent exit point from current corner
        dx = (1.0, 0.0, -1.0, 0.0)[ci]
        dy = (0.0, 1.0, 0.0, -1.0)[ci]
        ex, ey = cx + rx * dx, cy + rx * dy
        # Tangent entry point to next corner
        nnx, nny = nx - rx * dx, ny - rx * dy
        points.append((ex + (nnx - ex) * frac, ey + (nny - ey) * frac))
        break
      d -= edge
    else:
      points.append((x, y))

  return points


# ── Public API ─────────────────────────────────────────────────────────

def render_glow(border_rect: rl.Rectangle, border_width: float = UI_BORDER_SIZE):
  """Layer 3: Amber glow behind the standard border. Call BEFORE drawing standard border."""
  state = _csc_state()
  if state is None or not state['active']:
    return

  intensity = _intensity(state['curvature'])
  if intensity <= 0.0:
    return

  phase = (rl.get_time() % _BREATHING_PERIOD) / _BREATHING_PERIOD
  breath = 0.5 + 0.5 * math.sin(phase * 2 * math.pi)
  alpha = _GLOW_MAX_ALPHA * intensity * (0.5 + 0.5 * breath)

  for i in range(_GLOW_LAYERS):
    inset = (border_width / _GLOW_LAYERS) * i
    falloff = 1.0 - (i / _GLOW_LAYERS)
    a = int(alpha * falloff * falloff)
    if a < 2:
      continue

    glow_rect = rl.Rectangle(
      border_rect.x + inset,
      border_rect.y + inset,
      border_rect.width - 2 * inset,
      border_rect.height - 2 * inset,
    )
    rl.draw_rectangle_rounded(glow_rect, 0.12, 10, rl.Color(251, 191, 36, a))


def render_filament(border_rect: rl.Rectangle, border_width: float = UI_BORDER_SIZE):
  """Layer 5: Amber filament on top of the standard border. Call AFTER drawing standard border."""
  state = _csc_state()
  if state is None:
    return

  if not state['active'] and not state['training']:
    return

  curvature = state['curvature']
  inner = rl.Rectangle(
    border_rect.x + border_width,
    border_rect.y + border_width,
    border_rect.width - 2 * border_width,
    border_rect.height - 2 * border_width,
  )

  points = _perimeter(inner, 0.12)
  n = len(points)

  direction = -1.0 if curvature < 0 else 1.0
  speed = _FILAMENT_SPEED_BASE + _intensity(curvature) * 3.0
  time_offset = rl.get_time() * speed * direction

  base_alpha = 120 if state['training'] else 200

  for pass_idx in range(_FILAMENT_PASSES):
    width = _FILAMENT_WIDTH + pass_idx * 1.5
    alpha_scale = 1.0 - (pass_idx / _FILAMENT_PASSES) * 0.5

    for i in range(n):
      j = (i + 1) % n
      t = ((i + time_offset) / n) % 1.0

      # Single pulse: ramp up 0→25%, hold 25→50%, ramp down 50→75%, off 75→100%
      if t < 0.25:
        a = t / 0.25
      elif t < 0.5:
        a = 1.0
      elif t < 0.75:
        a = (0.75 - t) / 0.25
      else:
        continue

      alpha = int(base_alpha * a * alpha_scale)
      if alpha < 2:
        continue

      rl.draw_line_ex(
        rl.Vector2(points[i][0], points[i][1]),
        rl.Vector2(points[j][0], points[j][1]),
        width,
        rl.Color(251, 191, 36, alpha),
      )
