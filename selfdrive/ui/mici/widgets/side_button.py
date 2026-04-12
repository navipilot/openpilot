import pyray as rl

from openpilot.system.ui.widgets import Widget
from openpilot.system.ui.lib.application import gui_app

WIDTH, HEIGHT = 112, 240


class SideButton(Widget):
  def __init__(self, btn_type: str):
    super().__init__()
    self.type = btn_type
    self.set_rect(rl.Rectangle(0, 0, WIDTH, HEIGHT))

    if btn_type not in ("check", "back"):
      btn_type = "back"
    btn_img_path = f"icons_mici/buttons/button_side_{btn_type}.png"
    btn_img_pressed_path = f"icons_mici/buttons/button_side_{btn_type}_pressed.png"
    self._txt_btn = gui_app.texture(btn_img_path, 100, 224)
    self._txt_btn_back = gui_app.texture(btn_img_pressed_path, 100, 224)

  def _render(self, _):
    x = int(self._rect.x + 12)
    y = int(self._rect.y + (self._rect.height - self._txt_btn.height) / 2)
    rl.draw_texture(self._txt_btn if not self.is_pressed else self._txt_btn_back, x, y, rl.WHITE)
    return False
