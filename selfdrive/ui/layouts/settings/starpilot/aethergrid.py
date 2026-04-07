from __future__ import annotations
import math
import pyray as rl
from collections.abc import Callable
from openpilot.system.ui.lib.application import gui_app, FontWeight, MousePos
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.widgets import Widget, DialogResult


GEOMETRY_OFFSET = 10
PLATE_TAU = 0.060
TILE_RADIUS = 0.25
TILE_SEGMENTS = 10
TILE_PADDING = 20
SLIDER_BUTTON_SIZE = 60


def hex_to_color(hex_str: str) -> rl.Color:
  hex_str = hex_str.lstrip('#')
  return rl.Color(int(hex_str[0:2], 16), int(hex_str[2:4], 16), int(hex_str[4:6], 16), 255)


_SURFACE_MAP = {
  "#E63956": hex_to_color("#E63956"),
  "#3B82F6": hex_to_color("#3B82F6"),
  "#10B981": hex_to_color("#10B981"),
  "#D946EF": hex_to_color("#D946EF"),
  "#8B5CF6": hex_to_color("#8B5CF6"),
  "#64748B": hex_to_color("#64748B"),
}

_SUBSTRATE_MAP = {
  "#E63956": hex_to_color("#5A0B1A"),
  "#3B82F6": hex_to_color("#0B1C4A"),
  "#10B981": hex_to_color("#033326"),
  "#D946EF": hex_to_color("#4A052E"),
  "#8B5CF6": hex_to_color("#1E0A4D"),
  "#64748B": hex_to_color("#252A33"),
}

_DEFAULT_SUBSTRATE = hex_to_color("#1E0A4D")


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
      self._rect.x - GEOMETRY_OFFSET, self._rect.y - GEOMETRY_OFFSET,
      self._rect.width + 2 * GEOMETRY_OFFSET, self._rect.height + 2 * GEOMETRY_OFFSET,
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

  def _draw_text_fit(self, font: rl.Font, text: str, pos: rl.Vector2, max_width: float, font_size: float, align_center: bool = False, align_right: bool = False, letter_spacing: float = 0, uppercase: bool = False):
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

  def _centered_content(self, face: rl.Rectangle, icon: rl.Texture2D | None, icon_scale: float, title_font_size: float, text_lines: int, line_heights: list[float]):
    line_spacing = 8
    total_h = sum(line_heights) + line_spacing * (text_lines - 1)
    icon_w = icon.width * icon_scale if icon else 0
    icon_h = icon.height * icon_scale if icon else 0
    if icon:
      total_h += icon_h + line_spacing
    group_top = face.y + (face.height - total_h) / 2
    if icon:
      ix = face.x + (face.width - icon_w) / 2
      rl.draw_texture_pro(icon, rl.Rectangle(0, 0, icon.width, icon.height), rl.Rectangle(ix + 1, group_top + 2, icon_w, icon_h), rl.Vector2(0, 0), 0, rl.Color(0, 0, 0, 90))
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
  def __init__(self, title: str, desc: str, icon_path: str, on_click: Callable | None = None, starpilot_icon: bool = False, bg_color: rl.Color | str | None = None):
    if bg_color:
      super().__init__(surface_color=bg_color, on_click=on_click)
    else:
      super().__init__(on_click=on_click)
    self.title = title
    self.desc = desc
    if icon_path:
      if starpilot_icon: self._icon = gui_app.starpilot_texture(icon_path, 100, 100)
      else: self._icon = gui_app.texture(icon_path, 100, 100)
    else: self._icon = None
    self._font_title = gui_app.font(FontWeight.BOLD)
    self._font_desc = gui_app.font(FontWeight.NORMAL)

  def _render(self, rect: rl.Rectangle):
    face = self._render_layers(rect)
    max_w = face.width - 40
    lines = self._wrap_text(self._font_title, self.title, max_w, 30)
    line_heights = [30] * len(lines)
    _, ty = self._centered_content(face, self._icon, 0.75, 30, len(line_heights), line_heights)
    line_h = 30
    line_spacing = 8
    for i, line in enumerate(lines):
      self._draw_text_fit(self._font_title, line, rl.Vector2(face.x + 20, ty + i * (line_h + line_spacing)), max_w, line_h, align_center=True)

    if self.desc:
      desc_lines = self._wrap_text(self._font_desc, self.desc, max_w, 18, max_lines=3)
      desc_y = ty + len(lines) * (line_h + line_spacing) + 18
      for i, line in enumerate(desc_lines):
        self._draw_text_fit(self._font_desc, line, rl.Vector2(face.x + 20, desc_y + i * 20), max_w, 18, align_center=True)


class ToggleTile(AetherTile):
  def __init__(self, title: str, get_state: Callable[[], bool], set_state: Callable[[bool], None], icon_path: str | None = None,
               bg_color: rl.Color | str | None = None, desc: str = "", is_enabled: Callable[[], bool] | None = None, disabled_label: str = ""):
    if bg_color: super().__init__(surface_color=bg_color)
    else: super().__init__(surface_color=rl.Color(0, 163, 0, 255))
    self.title = title
    self.desc = desc
    self.get_state = get_state
    self.set_state = set_state
    self.set_enabled(is_enabled or True)
    self._icon = gui_app.starpilot_texture(icon_path, 80, 80) if icon_path else None
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
    max_w = face.width - 40
    self._draw_text_fit(self._font, self.title, rl.Vector2(face.x + 20, ty), max_w, 28, align_center=True, uppercase=True)
    if enabled:
      state_text = tr("ON") if active else tr("OFF")
    else:
      state_text = tr(self._disabled_label) if self._disabled_label else tr("LOCKED")
    self._draw_text_fit(self._font, state_text, rl.Vector2(face.x + 20, ty + 28 + 8), max_w, 30, align_center=True, uppercase=True)

    if self.desc:
      self._draw_text_fit(self._font_desc, self.desc, rl.Vector2(face.x + 20, ty + 28 + 8 + 34), max_w, 18, align_center=True)


class ValueTile(AetherTile):
  def __init__(self, title: str, get_value: Callable[[], str], on_click: Callable, icon_path: str | None = None,
               bg_color: rl.Color | str | None = None, is_enabled: Callable[[], bool] | None = None, desc: str = ""):
    super().__init__(surface_color=bg_color, on_click=on_click)
    self.title = title
    self.desc = desc
    self.get_value = get_value
    self._enabled = is_enabled or (lambda: True)
    self._icon = gui_app.starpilot_texture(icon_path, 80, 80) if icon_path else None
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
    max_w = face.width - 40
    self._draw_text_fit(self._font, self.title, rl.Vector2(face.x + 20, ty), max_w, 28, align_center=True, uppercase=True)
    val_text = self.get_value()
    self._draw_text_fit(self._font, val_text, rl.Vector2(face.x + 20, ty + 28 + 8), max_w, 28, align_center=True, uppercase=True)

    if self.desc:
      self._draw_text_fit(self._font_desc, self.desc, rl.Vector2(face.x + 20, ty + 28 + 8 + 34), max_w, 18, align_center=True)


class AetherSlider(Widget):
  def __init__(self, min_val: float, max_val: float, step: float, current_val: float, on_change: Callable[[float], None], unit: str = "", labels: dict[float, str] | None = None, color: rl.Color = rl.Color(54, 77, 239, 255)):
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
    rl.draw_rectangle_rounded(rl.Rectangle(t_face_rect.x, t_face_rect.y, t_face_rect.width - 1.5, t_face_rect.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110))
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
      if rl.check_collision_point_rec(mouse_pos, rl.Rectangle(self._rect.x + self._rect.width - SLIDER_BUTTON_SIZE, self._rect.y, SLIDER_BUTTON_SIZE, self._rect.height)):
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
  def __init__(self, title: str, min_val: float, max_val: float, step: float, current_val: float, on_close: Callable, unit: str = "", labels: dict[float, str] | None = None, color: rl.Color | str = "#F57371"):
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
        self._user_callback(DialogResult.CONFIRM, self._current_val)
        gui_app.set_modal_overlay(None)
      self._is_pressed_ok = False
    if self._is_pressed_cancel:
      self._cancel_target = 0.0
      if rl.check_collision_point_rec(mouse_pos, self._cancel_rect):
        self._user_callback(DialogResult.CANCEL, self._current_val)
        gui_app.set_modal_overlay(None)
      self._is_pressed_cancel = False

  def _render(self, rect: rl.Rectangle):
    dt = rl.get_frame_time()
    self._ok_offset += (self._ok_target - self._ok_offset) * (1 - math.exp(-dt / PLATE_TAU))
    self._cancel_offset += (self._cancel_target - self._cancel_offset) * (1 - math.exp(-dt / PLATE_TAU))
    rl.draw_rectangle(0, 0, gui_app.width, gui_app.height, rl.Color(0, 0, 0, 160))
    dialog_w, dialog_h = 1000, 500
    dx, dy = rect.x + (rect.width - dialog_w) / 2, rect.y + (rect.height - dialog_h) / 2
    self._ok_rect = rl.Rectangle(dx + dialog_w - 450, dy + dialog_h - 120, 350, 80)
    self._cancel_rect = rl.Rectangle(dx + 100, dy + dialog_h - 120, 350, 80)
    d_rect = rl.Rectangle(dx, dy, dialog_w, dialog_h)
    rl.draw_rectangle_rounded(d_rect, 0.05, 10, rl.Color(30, 30, 30, 255))
    rl.draw_rectangle_rounded_lines_ex(d_rect, 0.05, 10, 2, self._color)
    ts = measure_text_cached(self._font_title, self.title, 50)
    rl.draw_text_ex(self._font_title, self.title, rl.Vector2(round(dx + (dialog_w - ts.x) / 2), round(dy + 40)), 50, 0, rl.WHITE)
    slider_rect = rl.Rectangle(dx + 100, dy + 200, dialog_w - 200, 100)
    self._slider.render(slider_rect)
    c_shadow_alpha = int(255 * (1.0 - 0.9 * self._cancel_offset))
    rl.draw_rectangle_rounded(rl.Rectangle(self._cancel_rect.x + GEOMETRY_OFFSET, self._cancel_rect.y + GEOMETRY_OFFSET, 350, 80), 0.2, 10, rl.Color(30, 30, 30, c_shadow_alpha))
    c_face_x = self._cancel_rect.x + GEOMETRY_OFFSET * self._cancel_offset
    c_face_y = self._cancel_rect.y + GEOMETRY_OFFSET * self._cancel_offset
    c_face = rl.Rectangle(c_face_x, c_face_y, 350, 80)
    rl.draw_rectangle_rounded(c_face, 0.2, 10, rl.Color(80, 80, 80, 255))
    rl.draw_rectangle_rounded(rl.Rectangle(c_face.x + 1, c_face.y + 1, c_face.width - 2, c_face.height - 2), 0.2, 10, rl.Color(0, 0, 0, 80))
    rl.draw_rectangle_rounded(rl.Rectangle(c_face.x, c_face.y, c_face.width - 1.5, c_face.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110))
    cts = measure_text_cached(self._font_btn, tr("CANCEL"), 35)
    cancel_text_pos = rl.Vector2(c_face_x + (350 - cts.x) / 2, c_face_y + (80 - cts.y) / 2)
    rl.draw_text_ex(self._font_btn, tr("CANCEL"), rl.Vector2(round(cancel_text_pos.x + 1), round(cancel_text_pos.y + 2)), 35, 0, rl.Color(0, 0, 0, 90))
    rl.draw_text_ex(self._font_btn, tr("CANCEL"), rl.Vector2(round(cancel_text_pos.x), round(cancel_text_pos.y)), 35, 0, rl.WHITE)
    o_shadow_alpha = int(255 * (1.0 - 0.9 * self._ok_offset))
    rl.draw_rectangle_rounded(rl.Rectangle(self._ok_rect.x + GEOMETRY_OFFSET, self._ok_rect.y + GEOMETRY_OFFSET, 350, 80), 0.2, 10, rl.Color(self._color.r, self._color.g, self._color.b, int(o_shadow_alpha * 0.4)))
    o_face_x = self._ok_rect.x + GEOMETRY_OFFSET * self._ok_offset
    o_face_y = self._ok_rect.y + GEOMETRY_OFFSET * self._ok_offset
    o_face = rl.Rectangle(o_face_x, o_face_y, 350, 80)
    rl.draw_rectangle_rounded(o_face, 0.2, 10, self._color)
    rl.draw_rectangle_rounded(rl.Rectangle(o_face.x + 1, o_face.y + 1, o_face.width - 2, o_face.height - 2), 0.2, 10, rl.Color(0, 0, 0, 80))
    rl.draw_rectangle_rounded(rl.Rectangle(o_face.x, o_face.y, o_face.width - 1.5, o_face.height - 1.5), 0.2, 10, rl.Color(255, 255, 255, 110))
    ots = measure_text_cached(self._font_btn, tr("OK"), 35)
    ok_text_pos = rl.Vector2(o_face_x + (350 - ots.x) / 2, o_face_y + (80 - ots.y) / 2)
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

  def set_index(self, index: int): self.current_index = index

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
    padding = 20
    option_w = 240 if len(self.options) <= 3 else 200
    total_width = len(self.options) * option_w + max(0, len(self.options) - 1) * padding
    if self.title:
      title_size = measure_text_cached(self._font_title, self.title, 40)
      rl.draw_text_ex(self._font_title, self.title, rl.Vector2(round(rect.x), round(rect.y + (rect.height - title_size.y) / 2)), 40, 0, rl.WHITE)
      start_x = rect.x + rect.width - total_width
    else:
      start_x = rect.x + (rect.width - total_width) / 2
    for i, opt in enumerate(self.options):
      r = rl.Rectangle(start_x + i * (option_w + padding), rect.y, option_w, rect.height)
      self._option_rects.append(r)
      is_active = i == self.current_index
      color = self._active_color if is_active else self._inactive_color
      offset = self._option_offsets[i] if i < len(self._option_offsets) else 0.0
      extrusion_alpha = int(255 * (1.0 - 0.9 * offset))
      rl.draw_rectangle_rounded(rl.Rectangle(r.x + GEOMETRY_OFFSET, r.y + GEOMETRY_OFFSET, r.width, r.height), TILE_RADIUS, 10, rl.Color(30, 30, 30, extrusion_alpha))
      face_x = r.x + GEOMETRY_OFFSET * offset
      face_y = r.y + GEOMETRY_OFFSET * offset
      face_rect = rl.Rectangle(face_x, face_y, r.width, r.height)
      rl.draw_rectangle_rounded(face_rect, TILE_RADIUS, 10, color)
      rl.draw_rectangle_rounded(rl.Rectangle(face_rect.x + 1, face_rect.y + 1, face_rect.width - 2, face_rect.height - 2), TILE_RADIUS, 10, rl.Color(0, 0, 0, 80))
      rl.draw_rectangle_rounded(rl.Rectangle(face_rect.x, face_rect.y, face_rect.width - 1.5, face_rect.height - 1.5), TILE_RADIUS, 10, rl.Color(255, 255, 255, 110))
      ts = measure_text_cached(self._font, opt.upper(), 35)
      text_pos = rl.Vector2(face_x + (r.width - ts.x) / 2, face_y + (r.height - ts.y) / 2)
      rl.draw_text_ex(self._font, opt.upper(), rl.Vector2(round(text_pos.x + 1), round(text_pos.y + 2)), 35, 0, rl.Color(0, 0, 0, 90))
      rl.draw_text_ex(self._font, opt.upper(), rl.Vector2(round(text_pos.x), round(text_pos.y)), 35, 0, rl.WHITE)


class TileGrid(Widget):
  def __init__(self, columns: int | None = None, padding: int = 20, uniform_width: bool = False):
    super().__init__()
    self._columns, self.padding, self.tiles = columns, padding, []
    self._uniform_width = uniform_width

  def add_tile(self, tile: Widget): self.tiles.append(tile)

  def clear(self): self.tiles.clear()

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    if not self.tiles:
      return
    tiles_to_render = list(self.tiles)
    count = len(tiles_to_render)
    if self._columns is not None:
      cols = self._columns
    else:
      if count == 1: cols = 1
      elif count == 2: cols = 2
      elif count == 3: cols = 3
      elif count == 4: cols = 2
      elif count <= 6: cols = 3
      else: cols = 4
    rows = (count + cols - 1) // cols
    tile_h = (rect.height - (self.padding * (rows - 1))) / rows
    uniform_tile_w = (rect.width - (self.padding * (cols - 1))) / cols if self._uniform_width else 0
    tile_idx = 0
    for r in range(rows):
      remaining = count - tile_idx
      if remaining <= 0: break
      items_in_row = min(cols, remaining)
      if self._uniform_width:
        row_tile_w = uniform_tile_w
        row_width = (row_tile_w * items_in_row) + (self.padding * (items_in_row - 1))
        row_x = rect.x + (rect.width - row_width) / 2
      else:
        row_tile_w = (rect.width - (self.padding * (items_in_row - 1))) / items_in_row
        row_x = rect.x
      for c in range(items_in_row):
        tile = tiles_to_render[tile_idx]
        tile.render(rl.Rectangle(row_x + c * (row_tile_w + self.padding), rect.y + r * (tile_h + self.padding), row_tile_w, tile_h))
        tile_idx += 1
