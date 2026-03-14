import pyray as rl

from openpilot.system.ui.lib.application import FontWeight, gui_app
from openpilot.system.ui.lib.text_measure import measure_text_cached

BUTTON_HEIGHT = 125
BUTTON_WIDTH = 300
BUTTON_TEXT_SIZE = 50
BUTTON_TOP_MARGIN = 60
BUTTON_RIGHT_MARGIN = 100
BUTTON_ROUNDNESS = 0.4
BUTTON_ROUND_SEGMENTS = 20

BUTTON_COLOR = rl.Color(41, 41, 41, 255)
BUTTON_PRESSED_COLOR = rl.Color(173, 173, 173, 255)


class SettingsBackButton:
  def __init__(self):
    self._font_medium = gui_app.font(FontWeight.MEDIUM)

  def draw(self, sidebar_rect: rl.Rectangle) -> rl.Rectangle:
    button_rect = rl.Rectangle(
      sidebar_rect.x + sidebar_rect.width - BUTTON_WIDTH - BUTTON_RIGHT_MARGIN,
      sidebar_rect.y + BUTTON_TOP_MARGIN,
      BUTTON_WIDTH,
      BUTTON_HEIGHT,
    )

    pressed = (
      rl.is_mouse_button_down(rl.MouseButton.MOUSE_BUTTON_LEFT)
      and rl.check_collision_point_rec(rl.get_mouse_position(), button_rect)
    )
    button_color = BUTTON_PRESSED_COLOR if pressed else BUTTON_COLOR
    rl.draw_rectangle_rounded(button_rect, BUTTON_ROUNDNESS, BUTTON_ROUND_SEGMENTS, button_color)

    back_text = "Back"
    text_size = measure_text_cached(self._font_medium, back_text, BUTTON_TEXT_SIZE)

    arrow_head_width = 15
    arrow_head_height = 30
    arrow_shaft_length = 20
    arrow_shaft_height = 10
    arrow_spacing = 15

    total_width = arrow_head_width + arrow_shaft_length + arrow_spacing + text_size.x

    start_x = button_rect.x + (button_rect.width - total_width) / 2
    center_y = button_rect.y + button_rect.height / 2

    # Draw arrow shaft
    shaft_rect = rl.Rectangle(
      start_x + arrow_head_width - 2,
      center_y - arrow_shaft_height / 2,
      arrow_shaft_length + 2,
      arrow_shaft_height
    )
    rl.draw_rectangle_rec(shaft_rect, rl.WHITE)

    # Draw arrow head
    v1 = rl.Vector2(start_x, center_y)
    v2 = rl.Vector2(start_x + arrow_head_width, center_y - arrow_head_height / 2)
    v3 = rl.Vector2(start_x + arrow_head_width, center_y + arrow_head_height / 2)
    rl.draw_triangle(v1, v3, v2, rl.WHITE)

    text_pos = rl.Vector2(
      start_x + arrow_head_width + arrow_shaft_length + arrow_spacing,
      center_y - text_size.y / 2,
    )
    rl.draw_text_ex(self._font_medium, back_text, text_pos, BUTTON_TEXT_SIZE, 0, rl.WHITE)

    return button_rect
