from __future__ import annotations
from dataclasses import dataclass
import math
import time
import pyray as rl
from collections.abc import Callable
from openpilot.system.ui.lib.application import gui_app, FontWeight, MousePos
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.widgets import Widget, DialogResult
from openpilot.system.ui.widgets.label import gui_label
from openpilot.selfdrive.ui.layouts.settings.starpilot.asset_loader import starpilot_texture


GEOMETRY_OFFSET = 10
PLATE_TAU = 0.060
TILE_RADIUS = 0.25
TILE_SEGMENTS = 10
SLIDER_BUTTON_SIZE = 60


class SPACING:
  xs: int = 4
  sm: int = 8
  md: int = 12
  lg: int = 16
  xl: int = 24
  xxl: int = 32
  xxxl: int = 48

  tile_gap: int = 16
  tile_content: int = 16
  line_gap: int = 8
  section_gap: int = 24
  tab_height: int = 96
  tab_panel_gap: int = 16


def hex_to_color(hex_str: str) -> rl.Color:
  hex_str = hex_str.lstrip('#')
  return rl.Color(int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)


_SUBSTRATE_MAP = {
  "#E63956": hex_to_color("#5A0B1A"),
  "#3B82F6": hex_to_color("#0B1C4A"),
  "#10B981": hex_to_color("#033326"),
  "#D946EF": hex_to_color("#4A052E"),
  "#8B5CF6": hex_to_color("#1E0A4D"),
  "#64748B": hex_to_color("#252A33"),
}

_DEFAULT_SUBSTRATE = hex_to_color("#1E0A4D")


def _resolve_value(value, default=""):
  if callable(value):
    return value()
  return value if value is not None else default


def _with_alpha(color: rl.Color, alpha: int) -> rl.Color:
  return rl.Color(color.r, color.g, color.b, max(0, min(color.a, int(alpha))))


class AetherListColors:
  PANEL_BG = rl.Color(8, 8, 10, 255)
  PANEL_BORDER = rl.Color(255, 255, 255, 22)
  PANEL_GLOW = rl.Color(92, 116, 151, 34)
  HEADER = rl.Color(236, 242, 250, 255)
  SUBTEXT = rl.Color(164, 177, 196, 255)
  MUTED = rl.Color(126, 139, 158, 255)
  ROW_BG = rl.Color(255, 255, 255, 0)
  ROW_BORDER = rl.Color(255, 255, 255, 0)
  ROW_SEPARATOR = rl.Color(255, 255, 255, 16)
  ROW_HOVER = rl.Color(255, 255, 255, 8)
  CURRENT_BG = rl.Color(89, 116, 151, 18)
  CURRENT_BORDER = rl.Color(116, 136, 168, 44)
  ACTION_BG = rl.Color(255, 255, 255, 0)
  ACTION_SEPARATOR = rl.Color(255, 255, 255, 18)
  PRIMARY = hex_to_color("#597497")
  PRIMARY_SOFT = rl.Color(89, 116, 151, 48)
  DANGER = rl.Color(173, 78, 90, 255)
  DANGER_SOFT = rl.Color(173, 78, 90, 44)
  SUCCESS = rl.Color(94, 168, 130, 255)
  SUCCESS_SOFT = rl.Color(94, 168, 130, 44)
  WARNING = rl.Color(204, 158, 83, 255)
  SCROLL_TRACK = rl.Color(255, 255, 255, 10)
  SCROLL_THUMB = rl.Color(255, 255, 255, 68)


@dataclass(frozen=True)
class AetherListMetrics:
  max_content_width: int = 1560
  outer_margin_x: int = 18
  outer_margin_y: int = 24
  panel_padding_x: int = 16
  panel_padding_top: int = 28
  panel_padding_bottom: int = 22
  header_height: int = 210
  section_gap: int = 28
  section_header_height: int = 34
  section_header_gap: int = 12
  row_height: int = 122
  utility_row_height: int = 88
  row_radius: float = 0.12
  action_width: int = 188
  header_button_height: int = 58
  header_button_gap: int = 10
  fade_height: int = 24
  content_right_gutter: int = 18
  toggle_width: int = 78
  toggle_height: int = 42
  toggle_right_inset: int = 34
  utility_value_right: int = 270
  utility_value_width: int = 220
  utility_chevron_right: int = 62
  menu_button_font_size: int = 18
  menu_button_roundness: float = 0.35
  menu_button_segments: int = 12


@dataclass(frozen=True)
class AetherListFrame:
  shell: rl.Rectangle
  header: rl.Rectangle
  scroll: rl.Rectangle


AETHER_LIST_METRICS = AetherListMetrics()


def build_list_panel_frame(rect: rl.Rectangle, metrics: AetherListMetrics = AETHER_LIST_METRICS) -> AetherListFrame:
  shell_w = min(rect.width - metrics.outer_margin_x * 2, metrics.max_content_width)
  shell_x = rect.x + (rect.width - shell_w) / 2
  shell_y = rect.y + metrics.outer_margin_y
  shell_h = rect.height - metrics.outer_margin_y * 2
  shell_rect = rl.Rectangle(shell_x, shell_y, shell_w, shell_h)

  header_rect = rl.Rectangle(
    shell_x + metrics.panel_padding_x,
    shell_y + metrics.panel_padding_top,
    shell_w - metrics.panel_padding_x * 2,
    metrics.header_height,
  )

  scroll_rect = rl.Rectangle(
    shell_x + metrics.panel_padding_x,
    header_rect.y + header_rect.height,
    shell_w - metrics.panel_padding_x * 2,
    shell_h - metrics.header_height - metrics.panel_padding_top - metrics.panel_padding_bottom,
  )

  return AetherListFrame(shell_rect, header_rect, scroll_rect)


def draw_list_panel_shell(frame: AetherListFrame, *, bg: rl.Color = AetherListColors.PANEL_BG, border: rl.Color = AetherListColors.PANEL_BORDER, glow: rl.Color = AetherListColors.PANEL_GLOW):
  rl.draw_rectangle_rounded(frame.shell, 0.055, 18, bg)
  rl.draw_rectangle_rounded_lines_ex(frame.shell, 0.055, 18, 1, border)
  glow_rect = rl.Rectangle(frame.shell.x + 2, frame.shell.y + 2, frame.shell.width - 4, frame.shell.height - 4)
  rl.draw_rectangle_rounded_lines_ex(glow_rect, 0.055, 18, 1, glow)


def draw_soft_card(rect: rl.Rectangle, fill: rl.Color, border: rl.Color, radius: float = 0.08, segments: int = 18):
  rl.draw_rectangle_rounded(rect, radius, segments, fill)
  rl.draw_rectangle_rounded_lines_ex(rect, radius, segments, 1, border)


def draw_list_row_shell(
  rect: rl.Rectangle,
  *,
  current: bool = False,
  hovered: bool = False,
  pressed: bool = False,
  is_last: bool = False,
  alpha: int = 255,
  row_bg: rl.Color = AetherListColors.ROW_BG,
  row_border: rl.Color = AetherListColors.ROW_BORDER,
  row_separator: rl.Color = AetherListColors.ROW_SEPARATOR,
  row_hover: rl.Color = AetherListColors.ROW_HOVER,
  current_bg: rl.Color = AetherListColors.CURRENT_BG,
  current_border: rl.Color = AetherListColors.CURRENT_BORDER,
  row_radius: float = AetherListMetrics.row_radius,
  segments: int = 18,
  separator_inset: int = 22,
):
  bg = current_bg if current else row_bg
  border = current_border if current else row_border
  if hovered:
    bg = rl.Color(bg.r, bg.g, bg.b, min(bg.a + row_hover.a, 255))
  if pressed:
    bg = rl.Color(bg.r, bg.g, bg.b, min(bg.a + 8, 255))

  if bg.a > 0:
    rl.draw_rectangle_rounded(rect, row_radius, segments, _with_alpha(bg, alpha))
  if current and border.a > 0:
    rl.draw_rectangle_rounded_lines_ex(rect, row_radius, segments, 1, _with_alpha(border, alpha))
  if not is_last:
    line_y = int(rect.y + rect.height - 1)
    rl.draw_line(int(rect.x + separator_inset), line_y, int(rect.x + rect.width - separator_inset), line_y, _with_alpha(row_separator, alpha))


def draw_action_rail(
  rect: rl.Rectangle,
  action_width: int,
  *,
  current: bool = False,
  alpha: int = 255,
  fill: rl.Color = AetherListColors.ACTION_BG,
  current_fill: rl.Color = rl.Color(255, 255, 255, 6),
  separator: rl.Color = AetherListColors.ACTION_SEPARATOR,
  inset_y: int = 18,
):
  action_x = rect.x + rect.width - action_width
  action_rect = rl.Rectangle(action_x, rect.y, action_width, rect.height)
  action_fill = current_fill if current else fill
  if action_fill.a > 0:
    rl.draw_rectangle_rec(action_rect, _with_alpha(action_fill, alpha))
  rl.draw_line(int(action_x), int(rect.y + inset_y), int(action_x), int(rect.y + rect.height - inset_y), _with_alpha(separator, alpha))
  return action_rect


def draw_list_scroll_fades(
  scroll_rect: rl.Rectangle,
  content_height: float,
  scroll_offset: float,
  bg_color: rl.Color,
  *,
  fade_height: int = AETHER_LIST_METRICS.fade_height,
  right_trim: int = 12,
  threshold: int = 4,
):
  if content_height <= scroll_rect.height + threshold:
    return

  fade_h = min(fade_height, int(scroll_rect.height / 4))
  if scroll_offset < -threshold:
    rl.draw_rectangle_gradient_v(
      int(scroll_rect.x), int(scroll_rect.y), int(scroll_rect.width - right_trim), fade_h, _with_alpha(bg_color, 255), _with_alpha(bg_color, 0)
    )

  if (-scroll_offset + scroll_rect.height) < (content_height - threshold):
    bottom_y = int(scroll_rect.y + scroll_rect.height - fade_h)
    rl.draw_rectangle_gradient_v(
      int(scroll_rect.x), bottom_y, int(scroll_rect.width - right_trim), fade_h, _with_alpha(bg_color, 0), _with_alpha(bg_color, 255)
    )


def draw_busy_ring(
  center: rl.Vector2,
  phase: float,
  accent_color: rl.Color,
  *,
  track_color: rl.Color = rl.Color(255, 255, 255, 26),
  inner_radius: float = 20,
  outer_radius: float = 26,
  sweep: float = 260,
  thickness: int = 48,
):
  rl.draw_ring(center, inner_radius, outer_radius, 0, 360, thickness, track_color)
  rl.draw_ring(center, inner_radius, outer_radius, phase, phase + sweep, thickness, accent_color)


def draw_toggle_switch(
  rect: rl.Rectangle,
  enabled: bool,
  *,
  track_color: rl.Color = AetherListColors.PRIMARY,
  off_track_color: rl.Color = rl.Color(255, 255, 255, 24),
  knob_color: rl.Color = rl.WHITE,
  width: int = AETHER_LIST_METRICS.toggle_width,
  height: int = AETHER_LIST_METRICS.toggle_height,
  right_inset: int = AETHER_LIST_METRICS.toggle_right_inset,
  knob_offset: int = 20,
):
  toggle_rect = rl.Rectangle(rect.x + rect.width - width - right_inset, rect.y + (rect.height - height) / 2, width, height)
  track = track_color if enabled else off_track_color
  knob_x = toggle_rect.x + toggle_rect.width - knob_offset if enabled else toggle_rect.x + knob_offset
  rl.draw_rectangle_rounded(toggle_rect, 1.0, 16, track)
  rl.draw_circle(int(knob_x), int(toggle_rect.y + toggle_rect.height / 2), 16, knob_color)


def draw_action_pill(
  rect: rl.Rectangle,
  text: str,
  fill: rl.Color,
  border: rl.Color,
  text_color: rl.Color,
  *,
  font_size: int = AETHER_LIST_METRICS.menu_button_font_size,
  roundness: float = AETHER_LIST_METRICS.menu_button_roundness,
  segments: int = AETHER_LIST_METRICS.menu_button_segments,
):
  rl.draw_rectangle_rounded(rect, roundness, segments, fill)
  rl.draw_rectangle_rounded_lines_ex(rect, roundness, segments, 1, border)
  gui_label(rect, text, font_size, text_color, FontWeight.SEMI_BOLD, alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER)


def draw_status_led(center: rl.Vector2, enabled: bool):
  if enabled:
    led_color = rl.Color(110, 175, 245, 255)
    rl.draw_circle(int(center.x), int(center.y), 18, rl.Color(110, 175, 245, 18))
    rl.draw_circle(int(center.x), int(center.y), 12, rl.Color(110, 175, 245, 48))
    rl.draw_circle(int(center.x), int(center.y), 7, rl.Color(110, 175, 245, 100))
    rl.draw_circle(int(center.x), int(center.y), 6, led_color)
    rl.draw_circle(int(center.x - 2), int(center.y - 2), 2, rl.Color(210, 235, 255, 200))
  else:
    rl.draw_circle(int(center.x), int(center.y), 8, rl.Color(10, 10, 14, 230))
    rl.draw_circle(int(center.x), int(center.y), 6, rl.Color(35, 40, 50, 255))
    rl.draw_ring(center, 6, 7, 0, 360, 24, rl.Color(70, 78, 95, 140))


def draw_overflow_dots(center: rl.Vector2, color: rl.Color):
  dot_r = 4
  gap = 12
  for i in range(3):
    rl.draw_circle(int(center.x + (i - 1) * gap), int(center.y), dot_r, color)


def draw_trash_icon(center: rl.Vector2, color: rl.Color):
  bin_rect = rl.Rectangle(center.x - 12, center.y - 12, 24, 24)
  lid_rect = rl.Rectangle(center.x - 14, center.y - 18, 28, 5)
  handle_rect = rl.Rectangle(center.x - 4, center.y - 22, 8, 4)
  rl.draw_rectangle_rounded(bin_rect, 0.2, 8, color)
  rl.draw_rectangle_rounded(lid_rect, 0.5, 8, color)
  rl.draw_rectangle_rounded(handle_rect, 0.5, 8, color)
  stripe = _with_alpha(AetherListColors.PANEL_BG, 120)
  rl.draw_line(int(center.x - 6), int(center.y - 8), int(center.x - 6), int(center.y + 8), stripe)
  rl.draw_line(int(center.x), int(center.y - 8), int(center.x), int(center.y + 8), stripe)
  rl.draw_line(int(center.x + 6), int(center.y - 8), int(center.x + 6), int(center.y + 8), stripe)


def draw_heart_icon(center: rl.Vector2, color: rl.Color):
  rl.draw_circle(int(center.x - 5), int(center.y - 3), 7, color)
  rl.draw_circle(int(center.x + 5), int(center.y - 3), 7, color)
  rl.draw_triangle(
    rl.Vector2(center.x + 13, center.y + 1),
    rl.Vector2(center.x - 13, center.y + 1),
    rl.Vector2(center.x, center.y + 13),
    color,
  )


def draw_download_icon(center: rl.Vector2, color: rl.Color):
  shaft_top = rl.Vector2(center.x, center.y - 18)
  shaft_bottom = rl.Vector2(center.x, center.y + 8)
  left_head = rl.Vector2(center.x - 11, center.y - 2)
  right_head = rl.Vector2(center.x + 11, center.y - 2)
  tray_left = rl.Vector2(center.x - 14, center.y + 18)
  tray_right = rl.Vector2(center.x + 14, center.y + 18)
  rl.draw_line_ex(shaft_top, shaft_bottom, 4, color)
  rl.draw_line_ex(left_head, shaft_bottom, 4, color)
  rl.draw_line_ex(right_head, shaft_bottom, 4, color)
  rl.draw_line_ex(tray_left, tray_right, 4, color)


class AetherButton(Widget):
  def __init__(
    self,
    text: str | Callable[[], str],
    click_callback: Callable[[], None] | None = None,
    enabled: bool | Callable[[], bool] = True,
    emphasized: bool = False,
    font_size: int = 24,
  ):
    super().__init__()
    self._text = text
    self._emphasized = emphasized
    self._font_size = font_size
    self.set_click_callback(click_callback)
    self.set_enabled(enabled)

  @property
  def text(self) -> str:
    return str(_resolve_value(self._text, ""))

  def set_text(self, text: str | Callable[[], str]):
    self._text = text

  def set_emphasized(self, emphasized: bool):
    self._emphasized = emphasized

  def _render(self, rect: rl.Rectangle):
    enabled = self.enabled
    hovered = enabled and rl.check_collision_point_rec(gui_app.last_mouse_event.pos, rect)
    pressed = enabled and self.is_pressed

    if self._emphasized:
      bg = AetherListColors.PRIMARY if enabled else rl.Color(AetherListColors.PRIMARY.r, AetherListColors.PRIMARY.g, AetherListColors.PRIMARY.b, 80)
      border = _with_alpha(AetherListColors.PRIMARY, 190 if enabled else 70)
    else:
      bg = rl.Color(255, 255, 255, 10 if enabled else 5)
      border = rl.Color(255, 255, 255, 22 if enabled else 10)

    if hovered:
      bg = rl.Color(min(bg.r + 10, 255), min(bg.g + 10, 255), min(bg.b + 10, 255), bg.a)
    if pressed:
      bg = rl.Color(max(bg.r - 8, 0), max(bg.g - 8, 0), max(bg.b - 8, 0), bg.a)

    rl.draw_rectangle_rounded(rect, 0.32, 16, bg)
    rl.draw_rectangle_rounded_lines_ex(rect, 0.32, 16, 1, border)
    gui_label(
      rect,
      self.text,
      self._font_size,
      AetherListColors.HEADER if enabled else AetherListColors.MUTED,
      FontWeight.MEDIUM,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER,
    )


class AetherChip:
  def __init__(self, text: str | Callable[[], str], fill: rl.Color, border: rl.Color, text_color: rl.Color, pill: bool = False, font_size: int = 18):
    self._text = text
    self._fill = fill
    self._border = border
    self._text_color = text_color
    self._pill = pill
    self._font_size = font_size

  @property
  def text(self) -> str:
    return str(_resolve_value(self._text, ""))

  def render(self, rect: rl.Rectangle):
    roundness = 1.0 if self._pill else 0.4
    rl.draw_rectangle_rounded(rect, roundness, 18, self._fill)
    rl.draw_rectangle_rounded_lines_ex(rect, roundness, 18, 1, _with_alpha(self._border, 110))
    gui_label(rect, self.text, 20 if self._pill else self._font_size, self._text_color, FontWeight.MEDIUM, alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER)


class AetherScrollbar:
  def __init__(
    self,
    track_color: rl.Color = AetherListColors.SCROLL_TRACK,
    thumb_color: rl.Color = AetherListColors.SCROLL_THUMB,
    track_width: int = 4,
    track_inset_x: int = 7,
    track_inset_y: int = 8,
    min_thumb_height: float = 46.0,
  ):
    self._track_color = track_color
    self._thumb_color = thumb_color
    self._track_width = track_width
    self._track_inset_x = track_inset_x
    self._track_inset_y = track_inset_y
    self._min_thumb_height = min_thumb_height

  def render(self, rect: rl.Rectangle, content_height: float, scroll_offset: float):
    if content_height <= 0 or content_height <= rect.height:
      return

    track_rect = rl.Rectangle(rect.x + rect.width - self._track_inset_x, rect.y + self._track_inset_y, self._track_width, rect.height - self._track_inset_y * 2)
    rl.draw_rectangle_rounded(track_rect, 1.0, 12, self._track_color)

    max_scroll = max(content_height - rect.height, 1.0)
    thumb_height = max(self._min_thumb_height, track_rect.height * (rect.height / content_height))
    thumb_range = max(track_rect.height - thumb_height, 0.0)
    thumb_y = track_rect.y + (-scroll_offset / max_scroll) * thumb_range
    thumb_rect = rl.Rectangle(track_rect.x, thumb_y, track_rect.width, thumb_height)
    rl.draw_rectangle_rounded(thumb_rect, 1.0, 12, self._thumb_color)


class AetherTile(Widget):
  def __init__(self, surface_color: rl.Color | str | None = None, substrate_color: rl.Color | str | None = None, on_click: Callable | None = None):
    super().__init__()
    if isinstance(surface_color, str):
      self.surface_color = hex_to_color(surface_color)
      self.substrate_color = _SUBSTRATE_MAP.get(surface_color, _DEFAULT_SUBSTRATE)
    elif surface_color:
      self.surface_color = surface_color
      self.substrate_color = substrate_color or _DEFAULT_SUBSTRATE
    else:
      self.surface_color = hex_to_color("#3B82F6")
      self.substrate_color = hex_to_color("#0B1C4A")
    if isinstance(self.substrate_color, str):
      self.substrate_color = hex_to_color(self.substrate_color)
    self.on_click = on_click
    self._plate_offset: float = 0.0
    self._plate_target: float = 0.0

  @property
  def _hit_rect(self) -> rl.Rectangle:
    return rl.Rectangle(
      self._rect.x - GEOMETRY_OFFSET,
      self._rect.y - GEOMETRY_OFFSET,
      self._rect.width + 2 * GEOMETRY_OFFSET,
      self._rect.height + 2 * GEOMETRY_OFFSET,
    )

  def _handle_mouse_press(self, mouse_pos: MousePos):
    if rl.check_collision_point_rec(mouse_pos, self._hit_rect):
      self._is_pressed = True
      self._plate_target = 1.0

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._is_pressed:
      if rl.check_collision_point_rec(mouse_pos, self._hit_rect):
        self._plate_target = 0.0
        if self.on_click:
          self.on_click()
      else:
        self._plate_target = 0.0
      self._is_pressed = False

  def _handle_mouse_event(self, mouse_event):
    if not rl.check_collision_point_rec(mouse_event.pos, self._hit_rect):
      self._plate_target = 0.0

  def _animate_plate(self, dt: float):
    if self._plate_offset == self._plate_target:
      return
    self._plate_offset += (self._plate_target - self._plate_offset) * (1 - math.exp(-dt / PLATE_TAU))

  def _render_layers(self, rect: rl.Rectangle, radius: float = TILE_RADIUS, segments: int = TILE_SEGMENTS):
    self._animate_plate(rl.get_frame_time())
    self.set_rect(rect)

    extrusion_alpha = int(255 * (1.0 - 0.9 * self._plate_offset))
    substrate = rl.Color(self.substrate_color.r, self.substrate_color.g, self.substrate_color.b, extrusion_alpha)
    rl.draw_rectangle_rounded(rl.Rectangle(rect.x + GEOMETRY_OFFSET, rect.y + GEOMETRY_OFFSET, rect.width, rect.height), radius, segments, substrate)

    face_x = rect.x + GEOMETRY_OFFSET * self._plate_offset
    face_y = rect.y + GEOMETRY_OFFSET * self._plate_offset

    shadow_rect = rl.Rectangle(face_x, face_y, rect.width, rect.height)
    rl.draw_rectangle_rounded(shadow_rect, radius, segments, rl.Color(0, 0, 0, 80))

    hl_rect = rl.Rectangle(face_x, face_y, rect.width - 1.5, rect.height - 1.5)
    rl.draw_rectangle_rounded(hl_rect, radius, segments, rl.Color(255, 255, 255, 110))

    surf_rect = rl.Rectangle(face_x + 1.5, face_y + 1.5, rect.width - 3, rect.height - 3)
    rl.draw_rectangle_rounded(surf_rect, radius, segments, self.surface_color)

    glare_rect = rl.Rectangle(surf_rect.x, surf_rect.y, surf_rect.width, surf_rect.height * 0.45)
    rl.draw_rectangle_rounded(glare_rect, radius, segments, rl.Color(255, 255, 255, 15))

    return surf_rect

  def _draw_text_fit(
    self,
    font: rl.Font,
    text: str,
    pos: rl.Vector2,
    max_width: float,
    font_size: float,
    align_center: bool = False,
    align_right: bool = False,
    letter_spacing: float = 0,
    uppercase: bool = False,
  ):
    if uppercase:
      text = text.upper()
    spacing = round(letter_spacing if letter_spacing > 0 else font_size * 0.15)
    base_font_size = max(1, int(round(font_size)))
    size = measure_text_cached(font, text, base_font_size, spacing=spacing)
    actual_font_size = base_font_size
    if size.x > max_width:
      actual_font_size = max(1, int(round(font_size * (max_width / size.x))))
      while actual_font_size > 1 and measure_text_cached(font, text, actual_font_size, spacing=spacing).x > max_width:
        actual_font_size -= 1
      render_width = measure_text_cached(font, text, actual_font_size, spacing=spacing).x
    else:
      render_width = size.x
    nudge_y = (font_size - actual_font_size) / 2
    draw_x = pos.x
    if align_center:
      draw_x = pos.x + (max_width - render_width) / 2
    elif align_right:
      draw_x = pos.x + max_width - render_width
    shadow_pos = rl.Vector2(round(draw_x + 1), round(pos.y + nudge_y + 2))
    rl.draw_text_ex(font, text, shadow_pos, actual_font_size, spacing, rl.Color(0, 0, 0, 90))
    rl.draw_text_ex(font, text, rl.Vector2(round(draw_x), round(pos.y + nudge_y)), actual_font_size, spacing, rl.WHITE)

  def _centered_content(
    self, face: rl.Rectangle, icon: rl.Texture2D | None, icon_scale: float, title_font_size: float, text_lines: int, line_heights: list[float]
  ):
    line_spacing = SPACING.line_gap
    total_h = sum(line_heights) + line_spacing * (text_lines - 1)
    icon_w = icon.width * icon_scale if icon else 0
    icon_h = icon.height * icon_scale if icon else 0
    if icon:
      total_h += icon_h + line_spacing
    group_top = face.y + (face.height - total_h) / 2
    if icon:
      ix = face.x + (face.width - icon_w) / 2
      rl.draw_texture_pro(
        icon, rl.Rectangle(0, 0, icon.width, icon.height), rl.Rectangle(ix + 1, group_top + 2, icon_w, icon_h), rl.Vector2(0, 0), 0, rl.Color(0, 0, 0, 90)
      )
      rl.draw_texture_pro(icon, rl.Rectangle(0, 0, icon.width, icon.height), rl.Rectangle(ix, group_top, icon_w, icon_h), rl.Vector2(0, 0), 0, rl.WHITE)
      ty = group_top + icon_h + line_spacing
    else:
      ty = group_top
    return face, ty

  def _wrap_text(self, font: rl.Font, text: str, max_width: float, font_size: float, max_lines: int = 2) -> list[str]:
    spacing = font_size * 0.15
    words = text.upper().split()
    lines: list[str] = []
    current = ""
    for word in words:
      candidate = f"{current} {word}".strip() if current else word
      if measure_text_cached(font, candidate, int(font_size), spacing=spacing).x <= max_width:
        current = candidate
      else:
        if current:
          lines.append(current)
        current = word
        if len(lines) >= max_lines:
          break
    if current and len(lines) < max_lines:
      lines.append(current)
    return lines if lines else [text.upper()]

  def _render(self, rect: rl.Rectangle):
    pass


class HubTile(AetherTile):
  def __init__(
    self,
    title: str,
    desc: str,
    icon_path: str,
    on_click: Callable | None = None,
    starpilot_icon: bool = False,
    bg_color: rl.Color | str | None = None,
    get_status: Callable[[], str] | None = None,
  ):
    if bg_color:
      super().__init__(surface_color=bg_color, on_click=on_click)
    else:
      super().__init__(on_click=on_click)
    self.get_status = get_status
    self.title = title
    self.desc = desc
    if icon_path:
      if starpilot_icon:
        self._icon = starpilot_texture(icon_path, 100, 100)
      else:
        self._icon = gui_app.texture(icon_path, 100, 100)
    else:
      self._icon = None
    self._font_title = gui_app.font(FontWeight.BOLD)
    self._font_desc = gui_app.font(FontWeight.NORMAL)

  def _render(self, rect: rl.Rectangle):
    face = self._render_layers(rect)

    status_text = self.get_status() if self.get_status else ""
    if status_text:
      import re

      m = re.search(r'(\d+)%$', status_text)
      if m:
        ratio = min(1.0, max(0.0, float(m.group(1)) / 100.0))
        if ratio > 0.05:
          fill_rect = rl.Rectangle(face.x, face.y, face.width * ratio, face.height)
          rl.draw_rectangle_rounded(fill_rect, TILE_RADIUS, 10, rl.Color(255, 255, 255, 40))

    content_pad = SPACING.tile_content
    max_w = face.width - (content_pad * 2)
    lines = self._wrap_text(self._font_title, self.title, max_w, 30)
    line_heights = [30] * len(lines)
    _, ty = self._centered_content(face, self._icon, 0.75, 30, len(line_heights), line_heights)
    line_h = 30
    line_spacing = SPACING.line_gap
    for i, line in enumerate(lines):
      self._draw_text_fit(self._font_title, line, rl.Vector2(face.x + content_pad, ty + i * (line_h + line_spacing)), max_w, line_h, align_center=True)

    desc_to_render = status_text if status_text else self.desc
    if desc_to_render:
      desc_lines = self._wrap_text(self._font_desc, desc_to_render, max_w, 18, max_lines=3)
      desc_y = ty + len(lines) * (line_h + line_spacing) + SPACING.lg
      for i, line in enumerate(desc_lines):
        self._draw_text_fit(self._font_desc, line, rl.Vector2(face.x + content_pad, desc_y + i * 20), max_w, 18, align_center=True)


class ToggleTile(AetherTile):
  def __init__(
    self,
    title: str,
    get_state: Callable[[], bool],
    set_state: Callable[[bool], None],
    icon_path: str | None = None,
    bg_color: rl.Color | str | None = None,
    desc: str = "",
    is_enabled: Callable[[], bool] | None = None,
    disabled_label: str = "",
  ):
    if bg_color:
      super().__init__(surface_color=bg_color)
    else:
      super().__init__(surface_color=rl.Color(0, 163, 0, 255))
    self.title = title
    self.desc = desc
    self.get_state = get_state
    self.set_state = set_state
    self.set_enabled(is_enabled or True)
    self._icon = starpilot_texture(icon_path, 80, 80) if icon_path else None
    self._font = gui_app.font(FontWeight.BOLD)
    self._font_desc = gui_app.font(FontWeight.NORMAL)
    self._active_color = self.surface_color
    self._inactive_color = rl.Color(120, 120, 120, 255)
    self._disabled_color = rl.Color(75, 75, 75, 255)
    self._disabled_label = disabled_label

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._is_pressed:
      if rl.check_collision_point_rec(mouse_pos, self._hit_rect) and self.enabled:
        self.set_state(not self.get_state())
      self._plate_target = 0.0
      self._is_pressed = False

  def _render(self, rect: rl.Rectangle):
    enabled = self.enabled
    active = self.get_state()
    if enabled:
      self.surface_color = self._active_color if active else self._inactive_color
    else:
      self.surface_color = self._disabled_color
      self._plate_offset = 0.0
      self._plate_target = 0.0
    face = self._render_layers(rect)
    line_heights = [28, 30]
    _, ty = self._centered_content(face, self._icon, 0.75, 28, len(line_heights), line_heights)
    content_pad = SPACING.tile_content
    max_w = face.width - (content_pad * 2)
    self._draw_text_fit(self._font, self.title, rl.Vector2(face.x + content_pad, ty), max_w, 28, align_center=True, uppercase=True)
    if enabled:
      state_text = tr("ON") if active else tr("OFF")
    else:
      state_text = tr(self._disabled_label) if self._disabled_label else tr("LOCKED")
    self._draw_text_fit(self._font, state_text, rl.Vector2(face.x + content_pad, ty + 28 + SPACING.line_gap), max_w, 30, align_center=True, uppercase=True)

    if self.desc:
      self._draw_text_fit(self._font_desc, self.desc, rl.Vector2(face.x + content_pad, ty + 28 + SPACING.line_gap + 34), max_w, 18, align_center=True)


class ValueTile(AetherTile):
  def __init__(
    self,
    title: str,
    get_value: Callable[[], str],
    on_click: Callable,
    icon_path: str | None = None,
    bg_color: rl.Color | str | None = None,
    is_enabled: Callable[[], bool] | None = None,
    desc: str = "",
  ):
    super().__init__(surface_color=bg_color, on_click=on_click)
    self.title = title
    self.desc = desc
    self.get_value = get_value
    self._enabled = is_enabled or (lambda: True)
    self._icon = starpilot_texture(icon_path, 80, 80) if icon_path else None
    self._font = gui_app.font(FontWeight.BOLD)
    self._font_desc = gui_app.font(FontWeight.NORMAL)
    self._active_color = self.surface_color
    self._disabled_color = rl.Color(120, 120, 120, 255)

  def _render(self, rect: rl.Rectangle):
    enabled = self.enabled
    self.surface_color = self._active_color if enabled else self._disabled_color
    if not enabled:
      self._plate_offset = 0.0
      self._plate_target = 0.0
    face = self._render_layers(rect)
    line_heights = [28, 28]
    _, ty = self._centered_content(face, self._icon, 0.75, 28, len(line_heights), line_heights)
    content_pad = SPACING.tile_content
    max_w = face.width - (content_pad * 2)
    self._draw_text_fit(self._font, self.title, rl.Vector2(face.x + content_pad, ty), max_w, 28, align_center=True, uppercase=True)
    val_text = self.get_value()
    self._draw_text_fit(self._font, val_text, rl.Vector2(face.x + content_pad, ty + 28 + SPACING.line_gap), max_w, 28, align_center=True, uppercase=True)

    if self.desc:
      self._draw_text_fit(self._font_desc, self.desc, rl.Vector2(face.x + content_pad, ty + 28 + SPACING.line_gap + 34), max_w, 18, align_center=True)


class SliderTile(AetherTile):
    LONG_PRESS_THRESHOLD = 0.5
    DRAG_THRESHOLD = 10

    def __init__(
        self,
        title: str,
        get_value: Callable[[], float],
        set_value: Callable[[float], None],
        min_val: float,
        max_val: float,
        step: float,
        icon_path: str | None = None,
        bg_color: rl.Color | str | None = None,
        is_enabled: Callable[[], bool] | None = None,
        desc: str = "",
        unit: str = "",
        labels: dict[float, str] | None = None,
        on_test: Callable[[], None] | None = None,
    ):
        super().__init__(surface_color=bg_color)
        self.title = title
        self.desc = desc
        self.get_value = get_value
        self.set_value = set_value
        self.min_val = min_val
        self.max_val = max_val
        self.step = step
        self.unit = unit
        self.labels = labels or {}
        self.on_test = on_test
        self.set_enabled(is_enabled or (lambda: True))
        self._icon = starpilot_texture(icon_path, 80, 80) if icon_path else None
        self._font = gui_app.font(FontWeight.BOLD)
        self._font_desc = gui_app.font(FontWeight.NORMAL)
        self._active_color = self.surface_color
        self._disabled_color = rl.Color(120, 120, 120, 255)

        self._is_dragging = False
        self._last_mouse_x = 0.0
        self._velocity = 0.0
        self._smooth_value = get_value()
        self._press_start_x = 0.0
        self._press_start_time: float | None = None
        self._long_press_triggered = False

    def _handle_mouse_press(self, mouse_pos: MousePos):
        if rl.check_collision_point_rec(mouse_pos, self._hit_rect) and self.enabled:
            self._is_pressed = True
            self._last_mouse_x = mouse_pos.x
            self._velocity = 0.0
            self._press_start_x = mouse_pos.x
            self._press_start_time = time.monotonic()
            self._long_press_triggered = False

    def _handle_mouse_release(self, mouse_pos: MousePos):
        self._is_dragging = False
        self._is_pressed = False
        self._press_start_time = None

    def _handle_mouse_event(self, mouse_event):
        if not rl.check_collision_point_rec(mouse_event.pos, self._hit_rect):
            if not self._is_dragging and not self._press_start_time:
                self._plate_target = 0.0

        if self._press_start_time and not self._is_dragging and not self._long_press_triggered:
            dx = abs(mouse_event.pos.x - self._press_start_x)
            if dx > self.DRAG_THRESHOLD:
                self._is_dragging = True
                self._press_start_time = None
            else:
                elapsed = time.monotonic() - self._press_start_time
                if elapsed >= self.LONG_PRESS_THRESHOLD:
                    self._long_press_triggered = True
                    self._press_start_time = None
                    if self.on_test:
                        self.on_test()

        if self._is_dragging:
            dt = rl.get_frame_time()
            current_val = self.get_value()
            mouse_pos = mouse_event.pos
            dx = mouse_pos.x - self._last_mouse_x
            self._velocity = dx / max(dt, 0.001)
            self._last_mouse_x = mouse_pos.x

            rect_w = self._rect.width
            if rect_w > 0:
                val_range = self.max_val - self.min_val
                val_dx = (dx / rect_w) * val_range
                new_val = current_val + val_dx

                abs_vel = abs(self._velocity)
                snap_threshold = 800
                coarse_step = 10 if val_range >= 100 else self.step * 5
                dynamic_step = coarse_step if abs_vel > snap_threshold else self.step

                snapped = round(new_val / dynamic_step) * dynamic_step
                snapped = max(self.min_val, min(self.max_val, snapped))

                if abs(snapped - current_val) >= self.step:
                    self.set_value(float(snapped))

    def _render(self, rect: rl.Rectangle):
        enabled = self.enabled
        current_val = self.get_value()
        dt = rl.get_frame_time()

        self._smooth_value += (current_val - self._smooth_value) * (1 - math.exp(-dt / 0.1))

        self.surface_color = self._active_color if enabled else self._disabled_color
        if not enabled:
            self._plate_offset = 0.0
            self._plate_target = 0.0

        face = self._render_layers(rect)

        frac = (self._smooth_value - self.min_val) / (self.max_val - self.min_val)
        fill_w = face.width * frac
        if fill_w > 1:
            fill_rect = rl.Rectangle(face.x, face.y, fill_w, face.height)
            fill_color = rl.Color(
                min(self.surface_color.r + 30, 255),
                min(self.surface_color.g + 30, 255),
                min(self.surface_color.b + 30, 255),
                140,
            )
            rl.draw_rectangle_rounded(fill_rect, TILE_RADIUS, 10, fill_color)
            edge_x = face.x + fill_w
            rl.draw_rectangle_rec(rl.Rectangle(edge_x - 3, face.y, 3, face.height), rl.Color(255, 255, 255, 80))

        line_heights = [28, 28]
        _, ty = self._centered_content(face, self._icon, 0.75, 28, len(line_heights), line_heights)
        content_pad = SPACING.tile_content
        max_w = face.width - (content_pad * 2)

        self._draw_text_fit(self._font, self.title, rl.Vector2(face.x + content_pad, ty), max_w, 28, align_center=True, uppercase=True)

        val_str = self.labels.get(current_val, f"{int(current_val)}{self.unit}")
        self._draw_text_fit(self._font, val_str, rl.Vector2(face.x + content_pad, ty + 28 + SPACING.line_gap), max_w, 28, align_center=True, uppercase=True)

        if self.desc:
            self._draw_text_fit(self._font_desc, self.desc, rl.Vector2(face.x + content_pad, ty + 28 + SPACING.line_gap + 34), max_w, 18, align_center=True)


class AetherSlider(Widget):
  def __init__(
    self,
    min_val: float,
    max_val: float,
    step: float,
    current_val: float,
    on_change: Callable[[float], None],
    unit: str = "",
    labels: dict[float, str] | None = None,
    color: rl.Color = rl.Color(54, 77, 239, 255),
  ):
    super().__init__()
    self.min_val, self.max_val, self.step, self.current_val = min_val, max_val, step, current_val
    self.on_change, self.unit, self.labels, self.color = on_change, unit, labels or {}, color
    self._is_dragging = False
    self._font = gui_app.font(FontWeight.BOLD)
    self._thumb_offset: float = 0.0
    self._minus_offset: float = 0.0
    self._plus_offset: float = 0.0
    self._minus_pressed = False
    self._plus_pressed = False

  def _clamp_and_snap(self, val: float) -> float:
    snapped = round((val - self.min_val) / self.step) * self.step + self.min_val
    return max(self.min_val, min(self.max_val, snapped))

  def _get_thumb_x(self, rect: rl.Rectangle) -> float:
    track_x = rect.x + SLIDER_BUTTON_SIZE
    track_w = rect.width - 2 * SLIDER_BUTTON_SIZE
    frac = (self.current_val - self.min_val) / (self.max_val - self.min_val)
    return track_x + frac * track_w

  def _exponential_ease(self, current: float, target: float, dt: float) -> float:
    if current == target:
      return target
    return current + (target - current) * (1 - math.exp(-dt / PLATE_TAU))

  def _draw_slider_button(self, rect: rl.Rectangle, label: str):
    shadow_alpha = int(255 * (1.0 - 0.8 * self._minus_offset if label == "-" else 1.0 - 0.8 * self._plus_offset))
    offset = self._minus_offset if label == "-" else self._plus_offset
    substrate = rl.Color(100, 100, 100, shadow_alpha)
    rl.draw_rectangle_rounded(rl.Rectangle(rect.x + GEOMETRY_OFFSET, rect.y + GEOMETRY_OFFSET, rect.width, rect.height), 0.2, 10, substrate)
    face_x = rect.x + GEOMETRY_OFFSET * offset
    face_y = rect.y + GEOMETRY_OFFSET * offset
    face_rect = rl.Rectangle(face_x, face_y, rect.width, rect.height)
    btn_color = rl.Color(80, 80, 80, 255)
    rl.draw_rectangle_rounded(face_rect, 0.2, 10, btn_color)
    rl.draw_rectangle_rounded(rl.Rectangle(face_rect.x + 1, face_rect.y + 1, face_rect.width - 2, face_rect.height - 2), 0.2, 10, rl.Color(0, 0, 0, 80))
    rl.draw_rectangle_rounded(rl.Rectangle(face_rect.x, face_rect.y, face_rect.width - 1.5, face_rect.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110))
    ts = measure_text_cached(self._font, label, 35)
    label_pos = rl.Vector2(face_x + (rect.width - ts.x) / 2, face_y + (rect.height - ts.y) / 2)
    rl.draw_text_ex(self._font, label, rl.Vector2(round(label_pos.x + 2), round(label_pos.y + 2)), 35, 0, rl.Color(0, 0, 0, 70))
    rl.draw_text_ex(self._font, label, rl.Vector2(round(label_pos.x), round(label_pos.y)), 35, 0, rl.WHITE)

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    dt = rl.get_frame_time()
    if self._is_dragging:
      self._update_val_from_mouse(rl.get_mouse_position())
    self._minus_offset = self._exponential_ease(self._minus_offset, 1.0 if self._minus_pressed else 0.0, dt)
    self._plus_offset = self._exponential_ease(self._plus_offset, 1.0 if self._plus_pressed else 0.0, dt)
    minus_rect = rl.Rectangle(rect.x, rect.y, SLIDER_BUTTON_SIZE, rect.height)
    plus_rect = rl.Rectangle(rect.x + rect.width - SLIDER_BUTTON_SIZE, rect.y, SLIDER_BUTTON_SIZE, rect.height)
    self._draw_slider_button(minus_rect, "-")
    self._draw_slider_button(plus_rect, "+")
    track_x = rect.x + SLIDER_BUTTON_SIZE
    track_w = rect.width - 2 * SLIDER_BUTTON_SIZE
    track_h = 20
    track_rect = rl.Rectangle(track_x, rect.y + (rect.height - track_h) / 2, track_w, track_h)
    rl.draw_rectangle_rounded(track_rect, 1.0, 10, rl.Color(50, 50, 50, 255))
    frac = (self.current_val - self.min_val) / (self.max_val - self.min_val)
    fill_w = frac * track_w
    if fill_w > 0:
      rl.draw_rectangle_rounded(rl.Rectangle(track_x, track_rect.y, fill_w, track_h), 1.0, 10, self.color)
    n_steps = int(round((self.max_val - self.min_val) / self.step))
    for i in range(n_steps + 1):
      tick_x = track_x + (i / n_steps) * track_w
      tick_h = int(track_h * 0.6)
      tick_y = track_rect.y + (track_h - tick_h) / 2
      rl.draw_rectangle_rec(rl.Rectangle(tick_x - 1, tick_y, 2, tick_h), rl.Color(255, 255, 255, 60))
    thumb_w, thumb_h = 24, 44
    thumb_x = self._get_thumb_x(rect) - thumb_w / 2
    thumb_y = rect.y + (rect.height - thumb_h) / 2
    thumb_offset = GEOMETRY_OFFSET * self._thumb_offset
    t_substrate = rl.Color(180, 180, 180, int(255 * (1.0 - 0.8 * self._thumb_offset)))
    rl.draw_rectangle_rounded(rl.Rectangle(thumb_x + GEOMETRY_OFFSET, thumb_y + GEOMETRY_OFFSET, thumb_w, thumb_h), 0.2, 10, t_substrate)
    t_face_rect = rl.Rectangle(thumb_x + thumb_offset, thumb_y + thumb_offset, thumb_w, thumb_h)
    rl.draw_rectangle_rounded(t_face_rect, 0.2, 10, rl.WHITE)
    rl.draw_rectangle_rounded(rl.Rectangle(t_face_rect.x + 1, t_face_rect.y + 1, t_face_rect.width - 2, t_face_rect.height - 2), 0.2, 10, rl.Color(0, 0, 0, 80))
    rl.draw_rectangle_rounded(
      rl.Rectangle(t_face_rect.x, t_face_rect.y, t_face_rect.width - 1.5, t_face_rect.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110)
    )
    val_str = self.labels.get(self.current_val, f"{self.current_val:.2f}".rstrip('0').rstrip('.') + self.unit)
    ts = measure_text_cached(self._font, val_str, 35)
    val_pos = rl.Vector2(thumb_x + (thumb_w - ts.x) / 2, thumb_y - 45)
    rl.draw_text_ex(self._font, val_str, rl.Vector2(round(val_pos.x + 2), round(val_pos.y + 2)), 35, 0, rl.Color(0, 0, 0, 70))
    rl.draw_text_ex(self._font, val_str, rl.Vector2(round(val_pos.x), round(val_pos.y)), 35, 0, rl.WHITE)

  def _handle_mouse_press(self, mouse_pos: MousePos):
    if not rl.check_collision_point_rec(mouse_pos, self._rect):
      return
    minus_rect = rl.Rectangle(self._rect.x, self._rect.y, SLIDER_BUTTON_SIZE, self._rect.height)
    plus_rect = rl.Rectangle(self._rect.x + self._rect.width - SLIDER_BUTTON_SIZE, self._rect.y, SLIDER_BUTTON_SIZE, self._rect.height)
    if rl.check_collision_point_rec(mouse_pos, minus_rect):
      self._minus_pressed = True
      return
    if rl.check_collision_point_rec(mouse_pos, plus_rect):
      self._plus_pressed = True
      return
    thumb_w, thumb_h = 24, 44
    thumb_x = self._get_thumb_x(self._rect) - thumb_w / 2
    thumb_y = self._rect.y + (self._rect.height - thumb_h) / 2
    thumb_rect = rl.Rectangle(thumb_x - 8, thumb_y - 8, thumb_w + 16, thumb_h + 16)
    if rl.check_collision_point_rec(mouse_pos, thumb_rect):
      self._is_dragging = True
      self._thumb_offset = 1.0
    else:
      track_x = self._rect.x + SLIDER_BUTTON_SIZE
      track_w = self._rect.width - 2 * SLIDER_BUTTON_SIZE
      if track_w > 0:
        rel_x = max(0.0, min(1.0, (mouse_pos.x - track_x) / track_w))
        val = self.min_val + rel_x * (self.max_val - self.min_val)
        snapped = self._clamp_and_snap(val)
        if snapped != self.current_val:
          self.current_val = snapped
          self.on_change(self.current_val)

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._minus_pressed:
      self._minus_pressed = False
      if rl.check_collision_point_rec(mouse_pos, rl.Rectangle(self._rect.x, self._rect.y, SLIDER_BUTTON_SIZE, self._rect.height)):
        new_val = self._clamp_and_snap(self.current_val - self.step)
        if new_val != self.current_val:
          self.current_val = new_val
          self.on_change(self.current_val)
    if self._plus_pressed:
      self._plus_pressed = False
      if rl.check_collision_point_rec(
        mouse_pos, rl.Rectangle(self._rect.x + self._rect.width - SLIDER_BUTTON_SIZE, self._rect.y, SLIDER_BUTTON_SIZE, self._rect.height)
      ):
        new_val = self._clamp_and_snap(self.current_val + self.step)
        if new_val != self.current_val:
          self.current_val = new_val
          self.on_change(self.current_val)
    if self._is_dragging:
      self._is_dragging = False
      self._thumb_offset = 0.0

  def _update_val_from_mouse(self, mouse_pos: MousePos):
    track_x = self._rect.x + SLIDER_BUTTON_SIZE
    track_w = self._rect.width - 2 * SLIDER_BUTTON_SIZE
    if track_w <= 0:
      return
    rel_x = max(0.0, min(1.0, (mouse_pos.x - track_x) / track_w))
    val = self.min_val + rel_x * (self.max_val - self.min_val)
    snapped = self._clamp_and_snap(val)
    if snapped != self.current_val:
      self.current_val = snapped
      self.on_change(self.current_val)


class AetherSliderDialog(Widget):
  def __init__(
    self,
    title: str,
    min_val: float,
    max_val: float,
    step: float,
    current_val: float,
    on_close: Callable,
    unit: str = "",
    labels: dict[float, str] | None = None,
    color: rl.Color | str = "#F57371",
  ):
    super().__init__()
    self.title, self._user_callback = title, on_close
    self._color = hex_to_color(color) if isinstance(color, str) else color
    self._font_title, self._font_btn = gui_app.font(FontWeight.BOLD), gui_app.font(FontWeight.BOLD)
    self._slider = AetherSlider(min_val, max_val, step, current_val, self._on_slider_change, unit, labels, self._color)
    self._current_val, self._is_pressed_ok, self._is_pressed_cancel = current_val, False, False
    self._ok_offset: float = 0.0
    self._cancel_offset: float = 0.0
    self._ok_target: float = 0.0
    self._cancel_target: float = 0.0

  def _on_slider_change(self, val):
    self._current_val = val

  def _handle_mouse_press(self, mouse_pos: MousePos):
    self._slider._handle_mouse_press(mouse_pos)
    if rl.check_collision_point_rec(mouse_pos, self._ok_rect):
      self._is_pressed_ok = True
      self._ok_target = 1.0
    if rl.check_collision_point_rec(mouse_pos, self._cancel_rect):
      self._is_pressed_cancel = True
      self._cancel_target = 1.0

  def _handle_mouse_release(self, mouse_pos: MousePos):
    self._slider._handle_mouse_release(mouse_pos)
    if self._is_pressed_ok:
      self._ok_target = 0.0
      if rl.check_collision_point_rec(mouse_pos, self._ok_rect):
        gui_app.pop_widget()
        self._user_callback(DialogResult.CONFIRM, self._current_val)
      self._is_pressed_ok = False
    if self._is_pressed_cancel:
      self._cancel_target = 0.0
      if rl.check_collision_point_rec(mouse_pos, self._cancel_rect):
        gui_app.pop_widget()
        self._user_callback(DialogResult.CANCEL, self._current_val)
      self._is_pressed_cancel = False

  def _render(self, rect: rl.Rectangle):
    dt = rl.get_frame_time()
    self._ok_offset += (self._ok_target - self._ok_offset) * (1 - math.exp(-dt / PLATE_TAU))
    self._cancel_offset += (self._cancel_target - self._cancel_offset) * (1 - math.exp(-dt / PLATE_TAU))
    rl.draw_rectangle(0, 0, gui_app.width, gui_app.height, rl.Color(0, 0, 0, 160))
    dialog_w, dialog_h = 1000, 500
    dialog_margin = SPACING.xxl
    button_height = 80
    button_width = 350
    dx, dy = rect.x + (rect.width - dialog_w) / 2, rect.y + (rect.height - dialog_h) / 2
    self._ok_rect = rl.Rectangle(dx + dialog_w - button_width - SPACING.lg, dy + dialog_h - button_height - SPACING.lg, button_width, button_height)
    self._cancel_rect = rl.Rectangle(dx + SPACING.lg, dy + dialog_h - button_height - SPACING.lg, button_width, button_height)
    d_rect = rl.Rectangle(dx, dy, dialog_w, dialog_h)
    rl.draw_rectangle_rounded(d_rect, 0.05, 10, rl.Color(30, 30, 30, 255))
    rl.draw_rectangle_rounded_lines_ex(d_rect, 0.05, 10, 2, self._color)
    ts = measure_text_cached(self._font_title, self.title, 50)
    rl.draw_text_ex(self._font_title, self.title, rl.Vector2(round(dx + (dialog_w - ts.x) / 2), round(dy + SPACING.xxl)), 50, 0, rl.WHITE)
    slider_rect = rl.Rectangle(dx + SPACING.xxl, dy + 200, dialog_w - (SPACING.xxl * 2), 100)
    self._slider.render(slider_rect)
    c_shadow_alpha = int(255 * (1.0 - 0.9 * self._cancel_offset))
    rl.draw_rectangle_rounded(
      rl.Rectangle(self._cancel_rect.x + GEOMETRY_OFFSET, self._cancel_rect.y + GEOMETRY_OFFSET, button_width, button_height),
      0.2,
      10,
      rl.Color(30, 30, 30, c_shadow_alpha),
    )
    c_face_x = self._cancel_rect.x + GEOMETRY_OFFSET * self._cancel_offset
    c_face_y = self._cancel_rect.y + GEOMETRY_OFFSET * self._cancel_offset
    c_face = rl.Rectangle(c_face_x, c_face_y, button_width, button_height)
    rl.draw_rectangle_rounded(c_face, 0.2, 10, rl.Color(80, 80, 80, 255))
    rl.draw_rectangle_rounded(rl.Rectangle(c_face.x + 1, c_face.y + 1, c_face.width - 2, c_face.height - 2), 0.2, 10, rl.Color(0, 0, 0, 80))
    rl.draw_rectangle_rounded(rl.Rectangle(c_face.x, c_face.y, c_face.width - 1.5, c_face.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110))
    cts = measure_text_cached(self._font_btn, tr("CANCEL"), 35)
    cancel_text_pos = rl.Vector2(c_face_x + (button_width - cts.x) / 2, c_face_y + (button_height - cts.y) / 2)
    rl.draw_text_ex(self._font_btn, tr("CANCEL"), rl.Vector2(round(cancel_text_pos.x + 1), round(cancel_text_pos.y + 2)), 35, 0, rl.Color(0, 0, 0, 90))
    rl.draw_text_ex(self._font_btn, tr("CANCEL"), rl.Vector2(round(cancel_text_pos.x), round(cancel_text_pos.y)), 35, 0, rl.WHITE)
    o_shadow_alpha = int(255 * (1.0 - 0.9 * self._ok_offset))
    rl.draw_rectangle_rounded(
      rl.Rectangle(self._ok_rect.x + GEOMETRY_OFFSET, self._ok_rect.y + GEOMETRY_OFFSET, button_width, button_height),
      0.2,
      10,
      rl.Color(self._color.r, self._color.g, self._color.b, int(o_shadow_alpha * 0.4)),
    )
    o_face_x = self._ok_rect.x + GEOMETRY_OFFSET * self._ok_offset
    o_face_y = self._ok_rect.y + GEOMETRY_OFFSET * self._ok_offset
    o_face = rl.Rectangle(o_face_x, o_face_y, button_width, button_height)
    rl.draw_rectangle_rounded(o_face, 0.2, 10, self._color)
    rl.draw_rectangle_rounded(rl.Rectangle(o_face.x + 1, o_face.y + 1, o_face.width - 2, o_face.height - 2), 0.2, 10, rl.Color(0, 0, 0, 80))
    rl.draw_rectangle_rounded(rl.Rectangle(o_face.x, o_face.y, o_face.width - 1.5, o_face.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110))
    ots = measure_text_cached(self._font_btn, tr("OK"), 35)
    ok_text_pos = rl.Vector2(o_face_x + (button_width - ots.x) / 2, o_face_y + (button_height - ots.y) / 2)
    rl.draw_text_ex(self._font_btn, tr("OK"), rl.Vector2(round(ok_text_pos.x + 1), round(ok_text_pos.y + 2)), 35, 0, rl.Color(0, 0, 0, 90))
    rl.draw_text_ex(self._font_btn, tr("OK"), rl.Vector2(round(ok_text_pos.x), round(ok_text_pos.y)), 35, 0, rl.WHITE)
    return DialogResult.NO_ACTION


class RadioTileGroup(Widget):
  def __init__(self, title: str, options: list[str], current_index: int, on_change: Callable):
    super().__init__()
    self.title, self.options, self.current_index, self.on_change = title, options, current_index, on_change
    self._font, self._font_title = gui_app.font(FontWeight.BOLD), gui_app.font(FontWeight.NORMAL)
    self._active_color, self._inactive_color = rl.Color(54, 77, 239, 255), rl.Color(80, 80, 80, 255)
    self._pressed_index = -1
    self._option_rects: list[rl.Rectangle] = []
    self._option_offsets: list[float] = []
    self._option_targets: list[float] = []

  def set_index(self, index: int):
    self.current_index = index

  def _handle_mouse_press(self, mouse_pos: MousePos):
    for i, r in enumerate(self._option_rects):
      hit = rl.Rectangle(r.x - GEOMETRY_OFFSET, r.y - GEOMETRY_OFFSET, r.width + 2 * GEOMETRY_OFFSET, r.height + 2 * GEOMETRY_OFFSET)
      if rl.check_collision_point_rec(mouse_pos, hit):
        self._pressed_index = i
        if i < len(self._option_offsets):
          self._option_targets[i] = 1.0
        return

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._pressed_index != -1:
      r = self._option_rects[self._pressed_index]
      hit = rl.Rectangle(r.x - GEOMETRY_OFFSET, r.y - GEOMETRY_OFFSET, r.width + 2 * GEOMETRY_OFFSET, r.height + 2 * GEOMETRY_OFFSET)
      if rl.check_collision_point_rec(mouse_pos, hit):
        if self.current_index != self._pressed_index:
          self.current_index = self._pressed_index
          self.on_change(self.current_index)
      if self._pressed_index < len(self._option_targets):
        self._option_targets[self._pressed_index] = 0.0
      self._pressed_index = -1

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    self._option_rects.clear()
    dt = rl.get_frame_time()
    while len(self._option_offsets) < len(self.options):
      self._option_offsets.append(0.0)
      self._option_targets.append(0.0)
    for i in range(len(self._option_offsets)):
      self._option_offsets[i] += (self._option_targets[i] - self._option_offsets[i]) * (1 - math.exp(-dt / PLATE_TAU))
    gap = SPACING.lg
    option_w = 240 if len(self.options) <= 3 else 188
    total_width = len(self.options) * option_w + max(0, len(self.options) - 1) * gap
    if self.title:
      title_size = measure_text_cached(self._font_title, self.title, 40)
      rl.draw_text_ex(self._font_title, self.title, rl.Vector2(round(rect.x), round(rect.y + (rect.height - title_size.y) / 2)), 40, 0, rl.WHITE)
      start_x = rect.x + rect.width - total_width
    else:
      start_x = rect.x + (rect.width - total_width) / 2
    for i, opt in enumerate(self.options):
      r = rl.Rectangle(start_x + i * (option_w + gap), rect.y, option_w, rect.height)
      self._option_rects.append(r)
      is_active = i == self.current_index
      color = self._active_color if is_active else self._inactive_color
      offset = self._option_offsets[i] if i < len(self._option_offsets) else 0.0
      extrusion_alpha = int(255 * (1.0 - 0.9 * offset))
      rl.draw_rectangle_rounded(
        rl.Rectangle(r.x + GEOMETRY_OFFSET, r.y + GEOMETRY_OFFSET, r.width, r.height), TILE_RADIUS, 10, rl.Color(30, 30, 30, extrusion_alpha)
      )
      face_x = r.x + GEOMETRY_OFFSET * offset
      face_y = r.y + GEOMETRY_OFFSET * offset
      face_rect = rl.Rectangle(face_x, face_y, r.width, r.height)
      rl.draw_rectangle_rounded(face_rect, TILE_RADIUS, 10, color)
      rl.draw_rectangle_rounded(
        rl.Rectangle(face_rect.x + 1, face_rect.y + 1, face_rect.width - 2, face_rect.height - 2), TILE_RADIUS, 10, rl.Color(0, 0, 0, 80)
      )
      rl.draw_rectangle_rounded(
        rl.Rectangle(face_rect.x, face_rect.y, face_rect.width - 1.5, face_rect.height - 1.5), TILE_RADIUS, 10, rl.Color(255, 255, 255, 110)
      )
      font_size = 30
      spacing = round(font_size * 0.08)
      max_width = r.width - (SPACING.lg + SPACING.xs)
      ts = measure_text_cached(self._font, opt, font_size, spacing=spacing)
      while font_size > 22 and ts.x > max_width:
        font_size -= 1
        spacing = round(font_size * 0.08)
        ts = measure_text_cached(self._font, opt, font_size, spacing=spacing)
      text_pos = rl.Vector2(face_x + (r.width - ts.x) / 2, face_y + (r.height - ts.y) / 2)
      rl.draw_text_ex(self._font, opt, rl.Vector2(round(text_pos.x + 1), round(text_pos.y + 2)), font_size, spacing, rl.Color(0, 0, 0, 90))
      rl.draw_text_ex(self._font, opt, rl.Vector2(round(text_pos.x), round(text_pos.y)), font_size, spacing, rl.WHITE)


class TileGrid(Widget):
  def __init__(self, columns: int | None = None, padding: int | None = None, uniform_width: bool = False):
    super().__init__()
    self._columns = columns
    self._gap = padding if padding is not None else SPACING.tile_gap
    self.tiles = []
    self._uniform_width = uniform_width

  @property
  def gap(self) -> int:
    return self._gap

  def add_tile(self, tile: Widget):
    self.tiles.append(tile)

  def clear(self):
    self.tiles.clear()

  def get_column_count(self, tile_count: int | None = None) -> int:
    count = len(self.tiles) if tile_count is None else tile_count
    if count <= 0:
      return self._columns or 1
    if self._columns is not None:
      return self._columns
    if count == 1:
      return 1
    if count == 2:
      return 2
    if count == 3:
      return 3
    if count == 4:
      return 2
    if count <= 6:
      return 3
    return 4

  def get_row_count(self, tile_count: int | None = None) -> int:
    count = len(self.tiles) if tile_count is None else tile_count
    if count <= 0:
      return 0
    cols = self.get_column_count(count)
    return (count + cols - 1) // cols

  def get_internal_gap_height(self, tile_count: int | None = None) -> float:
    rows = self.get_row_count(tile_count)
    return self._gap * max(0, rows - 1)

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    if not self.tiles:
      return
    tiles_to_render = list(self.tiles)
    count = len(tiles_to_render)
    cols = self.get_column_count(count)
    rows = self.get_row_count(count)
    tile_h = (rect.height - (self._gap * (rows - 1))) / rows
    uniform_tile_w = (rect.width - (self._gap * (cols - 1))) / cols if self._uniform_width else 0
    tile_idx = 0
    for r in range(rows):
      remaining = count - tile_idx
      if remaining <= 0:
        break
      items_in_row = min(cols, remaining)
      if self._uniform_width:
        row_tile_w = uniform_tile_w
        row_width = (row_tile_w * items_in_row) + (self._gap * (items_in_row - 1))
        row_x = rect.x + (rect.width - row_width) / 2
      else:
        row_tile_w = (rect.width - (self._gap * (items_in_row - 1))) / items_in_row
        row_x = rect.x
      for c in range(items_in_row):
        tile = tiles_to_render[tile_idx]
        tile.render(rl.Rectangle(row_x + c * (row_tile_w + self._gap), rect.y + r * (tile_h + self._gap), row_tile_w, tile_h))
        tile_idx += 1
