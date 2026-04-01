import pyray as rl
from openpilot.common.constants import CV
from openpilot.selfdrive.ui import UI_BORDER_SIZE
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.system.ui.lib.application import gui_app, FontWeight
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.lib.text_measure import measure_text_cached

# ── Constants (matching Qt exactly) ───────────────────────────────────

# Set speed rect layout (from hud_renderer.py UI_CONFIG)
SET_SPEED_X_OFFSET = 60
SET_SPEED_Y_OFFSET = 45
SET_SPEED_HEIGHT = 204
SET_SPEED_WIDTH_IMP = 172
SET_SPEED_WIDTH_MET = 200

# US MUTCD sign
US_SIGN_HEIGHT = 186
US_SIGN_RADIUS = 24
US_INNER_RADIUS = 16
US_BORDER_WIDTH = 6
US_INSET = 9

# EU Vienna sign
EU_SIGN_SIZE = 176
EU_SIGN_WIDTH = 176
RED_RING_WIDTH = 20

# Pending sign
PENDING_BLINK_MS = 500
PENDING_US_WIDE = 200

# Sources panel
SOURCE_ROW_W = 450
SOURCE_ROW_H = 60
SOURCE_ROW_GAP = UI_BORDER_SIZE // 2

# Fonts
FONT_LABEL = 28
FONT_SPEED = 70
FONT_OFFSET = 50
FONT_EU_LARGE = 70
FONT_EU_SMALL = 60
FONT_EU_OFFSET = 40

# Layout
SIGN_MARGIN = 12


# ── State ─────────────────────────────────────────────────────────────

def _get_slc_state():
  """Extract SLC state from SubMaster. Returns dict or None if stale."""
  sm = ui_state.sm
  if sm.recv_frame["starpilotPlan"] < ui_state.started_frame:
    return None

  plan = sm["starpilotPlan"]
  speed_limit_changed = plan.speedLimitChanged

  show_slc = ui_state.params.get_bool("ShowSpeedLimits") or ui_state.params.get_bool("SpeedLimitController")
  hide_sl = ui_state.params.get_bool("HideSpeedLimit")
  hide = not speed_limit_changed and hide_sl

  if not show_slc and not speed_limit_changed:
    return None

  speed_conversion = CV.MS_TO_KPH if ui_state.is_metric else CV.MS_TO_MPH
  show_offset = ui_state.params.get_bool("ShowSLCOffset")

  dashboard_sl = sm["starpilotCarState"].dashboardSpeedLimit if sm.valid.get("starpilotCarState", False) else 0.0
  vision_sl = ui_state.params_memory.get_float("VisionSpeedLimit") if ui_state.params.get_bool("VisionSpeedLimitDetection") else 0.0

  slc_overridden_speed = plan.slcOverriddenSpeed
  speed_limit = plan.slcSpeedLimit

  # Offset calculation (matches Qt lines 199-204)
  if slc_overridden_speed == 0 and not show_offset:
    speed_limit += plan.slcSpeedLimitOffset
  speed_limit *= speed_conversion

  speed_limit_offset = plan.slcSpeedLimitOffset * speed_conversion
  offset_str = f"{'+' if speed_limit_offset > 0 else '-'}{abs(int(round(speed_limit_offset)))}" if speed_limit_offset != 0 else "\u2013"

  return {
    'speed_limit': speed_limit,
    'speed_limit_str': "\u2013" if speed_limit <= 1 else str(int(round(speed_limit))),
    'slc_overridden_speed': slc_overridden_speed,
    'speed_limit_source': plan.slcSpeedLimitSource,
    'unconfirmed_speed_limit': max(0.0, plan.unconfirmedSlcSpeedLimit * speed_conversion),
    'speed_limit_changed': speed_limit_changed,
    'hide': hide,
    'show_offset': show_offset,
    'use_vienna': ui_state.params.get_bool("UseVienna"),
    'offset_str': offset_str,
    'speed_conversion': speed_conversion,
    'show_sources': ui_state.params.get_bool("SpeedLimitSources"),
    # Per-source raw values
    'dashboard_sl': max(0.0, dashboard_sl * speed_conversion),
    'map_sl': max(0.0, plan.slcMapSpeedLimit * speed_conversion),
    'vision_sl': max(0.0, vision_sl * speed_conversion),
    'mapbox_sl': max(0.0, plan.slcMapboxSpeedLimit * speed_conversion),
    'next_sl': max(0.0, plan.slcNextSpeedLimit * speed_conversion),
  }


# ── Fonts ─────────────────────────────────────────────────────────────

_font_bold = None
_font_semi_bold = None

def _get_bold():
  global _font_bold
  if _font_bold is None:
    _font_bold = gui_app.font(FontWeight.BOLD)
  return _font_bold

def _get_semi_bold():
  global _font_semi_bold
  if _font_semi_bold is None:
    _font_semi_bold = gui_app.font(FontWeight.SEMI_BOLD)
  return _font_semi_bold


# ── US MUTCD Sign ─────────────────────────────────────────────────────

def _draw_us_sign(x: float, y: float, sign_width: float, speed_text: str, offset_str: str,
                  alpha: int, show_offset: bool):
  """Draw US-style speed limit sign at (x, y) with given width."""
  sign_rect = rl.Rectangle(x, y, sign_width, US_SIGN_HEIGHT)
  border_rect = rl.Rectangle(x + US_INSET, y + US_INSET,
                              sign_width - 2 * US_INSET,
                              US_SIGN_HEIGHT - 2 * US_INSET)

  # White background
  rl.draw_rectangle_rounded(sign_rect, US_SIGN_RADIUS / US_SIGN_HEIGHT, 16,
                             rl.Color(255, 255, 255, alpha))
  # Black border
  rl.draw_rectangle_rounded_lines_ex(border_rect, US_INNER_RADIUS / (US_SIGN_HEIGHT - 18), 16,
                                      US_BORDER_WIDTH, rl.Color(0, 0, 0, alpha))

  font_bold = _get_bold()
  font_semi = _get_semi_bold()
  text_color = rl.Color(0, 0, 0, alpha)
  cx = x + sign_width / 2

  if show_offset:
    # Offset ON: "LIMIT" at y=22, speed at y=51, offset at y=120
    limit_size = measure_text_cached(font_semi, tr("LIMIT"), FONT_LABEL)
    rl.draw_text_ex(font_semi, tr("LIMIT"), rl.Vector2(cx - limit_size.x / 2, y + 22), FONT_LABEL, 0, text_color)

    speed_size = measure_text_cached(font_bold, speed_text, FONT_SPEED)
    rl.draw_text_ex(font_bold, speed_text, rl.Vector2(cx - speed_size.x / 2, y + 51), FONT_SPEED, 0, text_color)

    offset_size = measure_text_cached(font_semi, offset_str, FONT_OFFSET)
    rl.draw_text_ex(font_semi, offset_str, rl.Vector2(cx - offset_size.x / 2, y + 120), FONT_OFFSET, 0, text_color)
  else:
    # Offset OFF: "SPEED" at y=22, "LIMIT" at y=51, speed at y=85
    speed_label_size = measure_text_cached(font_semi, tr("SPEED"), FONT_LABEL)
    rl.draw_text_ex(font_semi, tr("SPEED"), rl.Vector2(cx - speed_label_size.x / 2, y + 22), FONT_LABEL, 0, text_color)

    limit_size = measure_text_cached(font_semi, tr("LIMIT"), FONT_LABEL)
    rl.draw_text_ex(font_semi, tr("LIMIT"), rl.Vector2(cx - limit_size.x / 2, y + 51), FONT_LABEL, 0, text_color)

    speed_size = measure_text_cached(font_bold, speed_text, FONT_SPEED)
    rl.draw_text_ex(font_bold, speed_text, rl.Vector2(cx - speed_size.x / 2, y + 85), FONT_SPEED, 0, text_color)


# ── EU Vienna Sign ────────────────────────────────────────────────────

def _draw_eu_sign(x: float, y: float, speed_text: str, offset_str: str,
                  text_alpha: int, show_offset: bool):
  """Draw EU-style speed limit sign at (x, y)."""
  center_x = x + EU_SIGN_SIZE / 2
  center_y = y + EU_SIGN_SIZE / 2
  radius = EU_SIGN_SIZE / 2

  # White circle (full opacity)
  rl.draw_circle(int(center_x), int(center_y), radius, rl.Color(255, 255, 255, 255))
  # Red ring (full opacity, static)
  rl.draw_ring(rl.Vector2(center_x, center_y), radius - RED_RING_WIDTH, radius,
               0, 360, 64, rl.Color(201, 34, 49, 255))

  font_bold = _get_bold()
  font_semi = _get_semi_bold()
  text_color = rl.Color(0, 0, 0, text_alpha)

  eu_font = FONT_EU_LARGE if len(speed_text) <= 2 else FONT_EU_SMALL

  if show_offset:
    # Offset ON: speed shifted up, offset below
    speed_size = measure_text_cached(font_bold, speed_text, eu_font)
    speed_pos = rl.Vector2(center_x - speed_size.x / 2, center_y - speed_size.y / 2 - 25)
    rl.draw_text_ex(font_bold, speed_text, speed_pos, eu_font, 0, text_color)

    offset_size = measure_text_cached(font_semi, offset_str, FONT_EU_OFFSET)
    offset_pos = rl.Vector2(center_x - offset_size.x / 2, y + 100)
    rl.draw_text_ex(font_semi, offset_str, offset_pos, FONT_EU_OFFSET, 0, text_color)
  else:
    # Centered speed value
    speed_size = measure_text_cached(font_bold, speed_text, eu_font)
    speed_pos = rl.Vector2(center_x - speed_size.x / 2, center_y - speed_size.y / 2)
    rl.draw_text_ex(font_bold, speed_text, speed_pos, eu_font, 0, text_color)


# ── Main Speed Limit Sign ────────────────────────────────────────────

def _draw_speed_limit_sign(state: dict, sign_x: float, sign_y: float, sign_width: float):
  """Draw the main speed limit sign (US or EU based on setting)."""
  speed_text = state['speed_limit_str']
  offset_str = state['offset_str']
  alpha = 72 if state['slc_overridden_speed'] != 0 else 255

  if state['use_vienna']:
    _draw_eu_sign(sign_x, sign_y, speed_text, offset_str, alpha, state['show_offset'])
  else:
    _draw_us_sign(sign_x, sign_y, sign_width, speed_text, offset_str, alpha, state['show_offset'])


# ── Pending Speed Limit Sign ─────────────────────────────────────────

def _draw_pending_sign(state: dict, pending_x: float, pending_y: float, sign_width: float):
  """Draw the blinking pending speed limit sign."""
  pending_speed = state['unconfirmed_speed_limit']
  if pending_speed <= 0:
    return

  speed_text = "\u2013" if pending_speed <= 1 else str(int(round(pending_speed)))
  use_vienna = state['use_vienna']
  blink_on = int(rl.get_time() * 1000) % (PENDING_BLINK_MS * 2) < PENDING_BLINK_MS

  if use_vienna:
    # EU pending: white circle, STATIC red ring, TEXT blinks black/red
    size = EU_SIGN_SIZE
    center_x = pending_x + size / 2
    center_y = pending_y + size / 2
    radius = size / 2

    rl.draw_circle(int(center_x), int(center_y), radius, rl.Color(255, 255, 255, 255))
    rl.draw_ring(rl.Vector2(center_x, center_y), radius - RED_RING_WIDTH, radius,
                 0, 360, 64, rl.Color(201, 34, 49, 255))

    eu_font = FONT_EU_LARGE if len(speed_text) <= 2 else FONT_EU_SMALL
    text_color = rl.Color(0, 0, 0) if blink_on else rl.Color(201, 34, 49)
    font_bold = _get_bold()
    speed_size = measure_text_cached(font_bold, speed_text, eu_font)
    speed_pos = rl.Vector2(center_x - speed_size.x / 2, center_y - speed_size.y / 2)
    rl.draw_text_ex(font_bold, speed_text, speed_pos, eu_font, 0, text_color)

  else:
    # US pending: white rounded rect, BORDER blinks black/red
    w = PENDING_US_WIDE if len(speed_text) >= 3 else sign_width
    h = US_SIGN_HEIGHT
    sign_rect = rl.Rectangle(pending_x, pending_y, w, h)
    border_rect = rl.Rectangle(pending_x + US_INSET, pending_y + US_INSET,
                                w - 2 * US_INSET, h - 2 * US_INSET)

    rl.draw_rectangle_rounded(sign_rect, US_SIGN_RADIUS / h, 16, rl.Color(255, 255, 255, 255))

    border_color = rl.Color(0, 0, 0) if blink_on else rl.Color(201, 34, 49)
    rl.draw_rectangle_rounded_lines_ex(border_rect, US_INNER_RADIUS / (h - 18), 16,
                                        US_BORDER_WIDTH, border_color)

    font_bold = _get_bold()
    font_semi = _get_semi_bold()
    black = rl.Color(0, 0, 0, 255)
    cx = pending_x + w / 2

    pending_label = measure_text_cached(font_semi, tr("PENDING"), FONT_LABEL)
    rl.draw_text_ex(font_semi, tr("PENDING"),
                     rl.Vector2(cx - pending_label.x / 2, pending_y + 22),
                     FONT_LABEL, 0, black)

    limit_label = measure_text_cached(font_semi, tr("LIMIT"), FONT_LABEL)
    rl.draw_text_ex(font_semi, tr("LIMIT"),
                     rl.Vector2(cx - limit_label.x / 2, pending_y + 51),
                     FONT_LABEL, 0, black)

    speed_size = measure_text_cached(font_bold, speed_text, FONT_SPEED)
    rl.draw_text_ex(font_bold, speed_text,
                     rl.Vector2(cx - speed_size.x / 2, pending_y + 85),
                     FONT_SPEED, 0, black)


# ── Sources Panel ─────────────────────────────────────────────────────

def _draw_sources_panel(state: dict, panel_x: float, panel_y: float):
  """Draw the 5 source indicator rows below the speed limit sign."""
  font_bold = _get_bold()
  font_semi = _get_semi_bold()
  active_source = state['speed_limit_source']

  sources = [
    ("Dashboard", state['dashboard_sl']),
    ("Map Data", state['map_sl']),
    ("Vision", state['vision_sl']),
    ("Mapbox", state['mapbox_sl']),
    ("Upcoming", state['next_sl']),
  ]

  y = panel_y
  for title, value in sources:
    is_active = active_source == title and value != 0

    rect = rl.Rectangle(panel_x, y, SOURCE_ROW_W, SOURCE_ROW_H)

    if is_active:
      bg = rl.Color(201, 34, 49, 166)
      border = rl.Color(201, 34, 49, 255)
      text_font = font_bold
    else:
      bg = rl.Color(0, 0, 0, 166)
      border = rl.Color(0, 0, 0, 255)
      text_font = font_semi

    rl.draw_rectangle_rounded(rect, 24 / SOURCE_ROW_H, 16, bg)
    rl.draw_rectangle_rounded_lines_ex(rect, 24 / SOURCE_ROW_H, 16, 10, border)

    speed_text = f"{int(round(value))}" if value != 0 else "N/A"
    full_text = f"{tr(title)} - {speed_text}"

    if is_active:
      # Draw with black stroke outline
      text_pos = rl.Vector2(rect.x + 20, rect.y + (SOURCE_ROW_H - 35) / 2)
      rl.draw_text_ex(text_font, full_text, text_pos, 35, 0, rl.Color(255, 255, 255, 255))
    else:
      text_pos = rl.Vector2(rect.x + 20, rect.y + (SOURCE_ROW_H - 35) / 2)
      rl.draw_text_ex(text_font, full_text, text_pos, 35, 0, rl.Color(255, 255, 255, 255))

    y += SOURCE_ROW_H + SOURCE_ROW_GAP


# ── Sign Rect Calculations ───────────────────────────────────────────

def _calc_sign_rect(sign_x: float, sign_y: float, sign_width: float, use_vienna: bool) -> rl.Rectangle:
  if use_vienna:
    return rl.Rectangle(sign_x, sign_y, EU_SIGN_SIZE, EU_SIGN_SIZE)
  return rl.Rectangle(sign_x, sign_y, sign_width, US_SIGN_HEIGHT)


def _calc_pending_rect(sign_x: float, sign_y: float, sign_width: float,
                       unconfirmed_speed: float, use_vienna: bool) -> rl.Rectangle:
  # Pending sign is adjacent to main sign (Qt: translated by sign.width + UI_BORDER_SIZE)
  pending_x = sign_x + sign_width + UI_BORDER_SIZE

  if use_vienna:
    return rl.Rectangle(pending_x, sign_y, EU_SIGN_SIZE, EU_SIGN_SIZE)

  speed_text = str(int(round(unconfirmed_speed)))
  w = PENDING_US_WIDE if len(speed_text) >= 3 else sign_width
  return rl.Rectangle(pending_x, sign_y, w, US_SIGN_HEIGHT)


# ── Click Handling ────────────────────────────────────────────────────

def handle_slc_click(mouse_pos, sign_x: float, sign_y: float, sign_width: float):
  """Handle mouse click for accepting pending speed limit. Call from _handle_mouse_press."""
  state = _get_slc_state()
  if state is None or not state['speed_limit_changed']:
    return

  pending_rect = _calc_pending_rect(sign_x, sign_y, sign_width,
                                     state['unconfirmed_speed_limit'], state['use_vienna'])
  if (pending_rect.x <= mouse_pos.x <= pending_rect.x + pending_rect.width and
      pending_rect.y <= mouse_pos.y <= pending_rect.y + pending_rect.height):
    from openpilot.common.params import Params
    Params(memory=True).put_bool("SpeedLimitAccepted", True)


# ── Public API ────────────────────────────────────────────────────────

def render_speed_limit(content_rect: rl.Rectangle):
  """Render SLC speed limit signs. Call from onroad view render loop."""
  state = _get_slc_state()
  if state is None:
    return

  # Compute sign rect matching Qt:
  # signRect = QRect(setSpeedRect.x() + signMargin, setSpeedRect.bottom() + signMargin,
  #                  setSpeedRect.width() - 2*signMargin, usSignHeight)
  ss_width = SET_SPEED_WIDTH_MET if ui_state.is_metric else SET_SPEED_WIDTH_IMP
  ss_x = content_rect.x + SET_SPEED_X_OFFSET + (SET_SPEED_WIDTH_IMP - ss_width) // 2
  ss_y = content_rect.y + SET_SPEED_Y_OFFSET

  sign_width = ss_width - 2 * SIGN_MARGIN
  sign_x = ss_x + SIGN_MARGIN
  sign_y = ss_y + SET_SPEED_HEIGHT + SIGN_MARGIN

  use_vienna = state['use_vienna']

  # 1. Pending sign (always drawn when speedLimitChanged, even if main sign hidden)
  if state['speed_limit_changed']:
    pending_rect = _calc_pending_rect(sign_x, sign_y, sign_width,
                                       state['unconfirmed_speed_limit'], use_vienna)
    _draw_pending_sign(state, pending_rect.x, pending_rect.y, sign_width)

  # 2. Main speed limit sign (unless hidden)
  if not state['hide']:
    _draw_speed_limit_sign(state, sign_x, sign_y, sign_width)

  # 3. Sources panel
  if state['show_sources']:
    sign_rect = _calc_sign_rect(sign_x, sign_y, sign_width, use_vienna)
    sources_x = sign_rect.x - SIGN_MARGIN
    sources_y = sign_rect.y + sign_rect.height + UI_BORDER_SIZE
    _draw_sources_panel(state, sources_x, sources_y)
