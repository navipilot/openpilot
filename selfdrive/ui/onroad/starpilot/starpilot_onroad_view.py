import pyray as rl
import time
from msgq.visionipc import VisionStreamType
from openpilot.common.params import Params
from openpilot.selfdrive.ui import UI_BORDER_SIZE
from openpilot.selfdrive.ui.onroad.augmented_road_view import AugmentedRoadView
from openpilot.selfdrive.ui.onroad.starpilot.curve_speed_border import render_glow, render_filament
from openpilot.selfdrive.ui.onroad.starpilot.path import render_adjacent_paths, render_blind_spot_path, render_path_edges
from openpilot.selfdrive.ui.onroad.starpilot.personality_button import PersonalityButton, BTN_SIZE
from openpilot.selfdrive.ui.onroad.starpilot.slc_speed_limit import (
  render_speed_limit, handle_slc_click, SET_SPEED_X_OFFSET, SET_SPEED_Y_OFFSET,
  SET_SPEED_WIDTH_IMP, SET_SPEED_WIDTH_MET, SET_SPEED_HEIGHT, SIGN_MARGIN,
)
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.selfdrive.ui.lib.starpilot_status import get_screen_edge_color
from openpilot.system.ui.lib.application import MousePos, gui_app, FontWeight


class StarPilotOnroadView(AugmentedRoadView):
  def __init__(self, stream_type: VisionStreamType = VisionStreamType.VISION_STREAM_ROAD):
    super().__init__(stream_type)
    self._params = Params()

    self._personality_button = PersonalityButton()
    self._font_bold = gui_app.font(FontWeight.BOLD)
    self._font_medium = gui_app.font(FontWeight.MEDIUM)
    self._standstill_started_at = 0.0

  def _render(self, rect: rl.Rectangle):
    super()._render(rect)

    if not ui_state.started:
      return

    self._render_slc(rect)
    self._render_overlays()
    self._render_path_features(rect)

  def _render_slc(self, rect: rl.Rectangle):
    content_rect = rl.Rectangle(
      rect.x + UI_BORDER_SIZE, rect.y + UI_BORDER_SIZE,
      rect.width - 2 * UI_BORDER_SIZE, rect.height - 2 * UI_BORDER_SIZE,
    )
    rl.begin_scissor_mode(
      int(content_rect.x), int(content_rect.y),
      int(content_rect.width), int(content_rect.height),
    )
    render_speed_limit(content_rect)
    rl.end_scissor_mode()

  def _render_overlays(self):
    self._position_personality_button()
    self._personality_button.render()
    self._render_road_name()
    self._render_standstill_timer()

  def _render_path_features(self, rect: rl.Rectangle):
    """Render path-related features (adjacent paths, blind spot, path edges)."""
    mr = self.model_renderer

    # Only render if we have path data
    if not mr._path.projected_points.size:
      return

    # Path edges (always rendered if track_edge_vertices exist)
    if mr._track_edge_vertices.size >= 4:
      render_path_edges(mr)

    # Adjacent paths or blind spot path (mutually exclusive, matching Qt behavior)
    adjacent_enabled = self._params.get_bool("AdjacentPath")
    blind_spot_enabled = self._params.get_bool("BlindSpotPath")

    if adjacent_enabled and mr._adjacent_path_vertices[0].size >= 4:
      render_adjacent_paths(mr)
    elif blind_spot_enabled and mr._adjacent_path_vertices[0].size >= 4:
      render_blind_spot_path(mr)

  def _position_personality_button(self):
    dm = self.driver_state_renderer
    toggle_on = self._params.get_bool("OnroadDistanceButton")

    if not dm.is_visible or not toggle_on:
      self._personality_button.set_visible(False)
      return

    self._personality_button.set_visible(
      lambda: ui_state.started and ui_state.has_longitudinal_control
    )

    y = dm.position_y - BTN_SIZE / 2
    if dm.is_rhd:
      x = dm.position_x - BTN_SIZE * 2
    else:
      x = dm.position_x + BTN_SIZE

    self._personality_button.set_position(x, y)

  def _render_road_name(self):
    if not self._params.get_bool("RoadNameUI"):
      return
    if not ui_state.sm.valid.get("mapdOut", False):
      return

    road_name = getattr(ui_state.sm["mapdOut"], "roadName", "")
    if not road_name:
      return

    text_size = rl.measure_text_ex(self._font_bold, road_name, 52, 0)
    pad_x, pad_y = 28, 18
    box_w = int(text_size.x + pad_x * 2)
    box_h = int(text_size.y + pad_y * 2)
    x = int((gui_app.width - box_w) / 2)
    y = int(gui_app.height - box_h - 22)

    rect = rl.Rectangle(x, y, box_w, box_h)
    rl.draw_rectangle_rounded(rect, 0.35, 10, rl.Color(0, 0, 0, 166))
    rl.draw_rectangle_rounded_lines_ex(rect, 0.35, 10, 3, rl.Color(255, 255, 255, 50))
    rl.draw_text_ex(
      self._font_bold,
      road_name,
      rl.Vector2(x + (box_w - text_size.x) / 2, y + (box_h - text_size.y) / 2),
      52,
      0,
      rl.WHITE,
    )

  def _render_standstill_timer(self):
    if not self._params.get_bool("stopped_timer"):
      self._standstill_started_at = 0.0
      return
    if not ui_state.sm.valid.get("carState", False):
      return

    car_state = ui_state.sm["carState"]
    if getattr(car_state, "standstill", False):
      if self._standstill_started_at == 0.0:
        self._standstill_started_at = time.monotonic()
    else:
      self._standstill_started_at = 0.0
      return

    if self._standstill_started_at == 0.0:
      return

    duration = int(time.monotonic() - self._standstill_started_at)
    if duration < 60:
      return

    minutes = duration // 60
    seconds = duration % 60
    minute_text = f"{minutes} minute{'s' if minutes != 1 else ''}"
    second_text = f"{seconds} second{'s' if seconds != 1 else ''}"
    minute_size = rl.measure_text_ex(self._font_bold, minute_text, 176, 0)
    second_size = rl.measure_text_ex(self._font_medium, second_text, 66, 0)

    x = gui_app.width / 2
    rl.draw_text_ex(
      self._font_bold,
      minute_text,
      rl.Vector2(x - minute_size.x / 2, 210 - minute_size.y / 2),
      176,
      0,
      rl.Color(255, 255, 255, 255),
    )
    rl.draw_text_ex(
      self._font_medium,
      second_text,
      rl.Vector2(x - second_size.x / 2, 290 - second_size.y / 2),
      66,
      0,
      rl.Color(255, 255, 255, 255),
    )

  def _draw_border(self, rect: rl.Rectangle):
    rl.draw_rectangle_lines_ex(rect, UI_BORDER_SIZE, rl.BLACK)
    border_rect = rl.Rectangle(rect.x + UI_BORDER_SIZE, rect.y + UI_BORDER_SIZE,
                                rect.width - 2 * UI_BORDER_SIZE, rect.height - 2 * UI_BORDER_SIZE)

    # Layer 3: Amber glow (behind standard border)
    render_glow(border_rect)

    # Layer 4: Standard border
    border_color = get_screen_edge_color(ui_state)
    rl.draw_rectangle_rounded_lines_ex(border_rect, 0.12, 10, UI_BORDER_SIZE, border_color)

    # Layer 5: Amber filament (on top of standard border)
    render_filament(border_rect)

  def _handle_mouse_press(self, mouse_pos: MousePos):
    content_rect = rl.Rectangle(
      UI_BORDER_SIZE, UI_BORDER_SIZE,
      gui_app.width - 2 * UI_BORDER_SIZE, gui_app.height - 2 * UI_BORDER_SIZE,
    )
    ss_width = SET_SPEED_WIDTH_MET if ui_state.is_metric else SET_SPEED_WIDTH_IMP
    ss_x = content_rect.x + SET_SPEED_X_OFFSET + (SET_SPEED_WIDTH_IMP - ss_width) // 2
    sign_x = ss_x + SIGN_MARGIN
    sign_y = content_rect.y + SET_SPEED_Y_OFFSET + SET_SPEED_HEIGHT + SIGN_MARGIN
    sign_width = ss_width - 2 * SIGN_MARGIN
    handle_slc_click(mouse_pos, sign_x, sign_y, sign_width)

    if self._personality_button.is_interacting:
      return
    super()._handle_mouse_press(mouse_pos)
