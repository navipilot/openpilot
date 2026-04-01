import pyray as rl
from msgq.visionipc import VisionStreamType
from openpilot.common.params import Params
from openpilot.selfdrive.ui import UI_BORDER_SIZE
from openpilot.selfdrive.ui.onroad.augmented_road_view import AugmentedRoadView, BORDER_COLORS
from openpilot.selfdrive.ui.onroad.starpilot.curve_speed_border import render_glow, render_filament
from openpilot.selfdrive.ui.onroad.starpilot.personality_button import PersonalityButton, BTN_SIZE
from openpilot.selfdrive.ui.onroad.starpilot.slc_speed_limit import (
  render_speed_limit, handle_slc_click, SET_SPEED_X_OFFSET, SET_SPEED_Y_OFFSET,
  SET_SPEED_WIDTH_IMP, SET_SPEED_WIDTH_MET, SET_SPEED_HEIGHT, SIGN_MARGIN,
)
from openpilot.selfdrive.ui.ui_state import ui_state, UIStatus
from openpilot.system.ui.lib.application import MousePos, gui_app

AOL_COLOR = rl.Color(10, 186, 181, 255)


class StarPilotOnroadView(AugmentedRoadView):
  def __init__(self, stream_type: VisionStreamType = VisionStreamType.VISION_STREAM_ROAD):
    super().__init__(stream_type)
    self._params = Params()

    self._personality_button = PersonalityButton()

  def _render(self, rect: rl.Rectangle):
    super()._render(rect)

    if not ui_state.started:
      return

    self._render_slc(rect)
    self._render_overlays()

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

  def _draw_border(self, rect: rl.Rectangle):
    rl.draw_rectangle_lines_ex(rect, UI_BORDER_SIZE, rl.BLACK)
    border_rect = rl.Rectangle(rect.x + UI_BORDER_SIZE, rect.y + UI_BORDER_SIZE,
                                rect.width - 2 * UI_BORDER_SIZE, rect.height - 2 * UI_BORDER_SIZE)

    # Layer 3: Amber glow (behind standard border)
    render_glow(border_rect)

    # Layer 4: Standard border
    border_color = AOL_COLOR if ui_state.always_on_lateral_active else BORDER_COLORS.get(ui_state.status, BORDER_COLORS[UIStatus.DISENGAGED])
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
