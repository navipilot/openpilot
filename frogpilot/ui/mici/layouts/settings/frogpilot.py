import numpy as np
import pyray as rl
import qrcode
import threading
import time

from collections.abc import Callable

from cereal import log
from openpilot.common.params import Params
from openpilot.selfdrive.ui.mici.widgets.button import BigButton
from openpilot.selfdrive.ui.mici.widgets.dialog import BigConfirmationDialog, BigDialog
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.system.ui.lib.application import FontWeight, MousePos, gui_app
from openpilot.system.ui.widgets.nav_widget import NavWidget
from openpilot.system.ui.widgets.label import UnifiedLabel
from openpilot.system.ui.widgets.scroller import Scroller

from openpilot.frogpilot.common.frogpilot_variables import THE_POND_URL
from openpilot.frogpilot.system.device_syncd import pair_to_the_pond, poll_status, unpair_from_the_pond

LABEL_HEIGHT = 80
MAX_POLLS = 100
POLL_INTERVAL = 3.0
STATUS_DISPLAY_TIME = 2.5


def generate_qr_code() -> rl.Texture:
  qr = qrcode.QRCode(error_correction=qrcode.constants.ERROR_CORRECT_L, box_size=1, border=0)
  qr.add_data(THE_POND_URL)
  qr.make(fit=True)

  pil_img = qr.make_image(fill_color="white", back_color="black").convert("RGBA")
  img_array = np.array(pil_img, dtype=np.uint8)

  rl_image = rl.Image()
  rl_image.data = rl.ffi.cast("void *", img_array.ctypes.data)
  rl_image.format = rl.PixelFormat.PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
  rl_image.height = pil_img.height
  rl_image.mipmaps = 1
  rl_image.width = pil_img.width

  return rl.load_texture_from_image(rl_image)


class PondPairButton(BigButton):
  def __init__(self):
    super().__init__("Pair Device", "", "icons_mici/settings/device/pair.png", icon_size=(66, 120))

    self._params = Params()

    self._pending_unpaired = False
    self._polling = False

    self._pending_code: str | None = None
    self._pending_error: str | None = None
    self._poll_result: str | None = None
    self._status_clear_time: float | None = None

    self._paired = self._params.get_bool("PondPaired")

  def _handle_mouse_release(self, mouse_pos: MousePos):
    super()._handle_mouse_release(mouse_pos)

    if ui_state.sm["deviceState"].networkType == log.DeviceState.NetworkType.none:
      gui_app.set_modal_overlay(BigDialog("No internet", "Please connect to the internet first!"))
      return
    if self._polling:
      return

    if self._paired:
      def unpair_confirmed():
        self.set_enabled(False)
        self.set_value("Unpairing...")

        threading.Thread(target=self._start_unpair_request, daemon=True).start()

      gui_app.push_widget(BigConfirmationDialog(
        "slide to unpair",
        gui_app.texture("../../frogpilot/assets/icons_mici/settings/icon_frog.png", 65, 65),
        unpair_confirmed,
        red=True,
      ))
    else:
      self.set_enabled(False)
      self.set_value("Requesting code...")

      threading.Thread(target=self._start_pair_request, daemon=True).start()

  def _start_pair_request(self):
    code, error = pair_to_the_pond()
    if error:
      self._pending_error = error
      return

    self._pending_code = code

  def _start_unpair_request(self):
    error = unpair_from_the_pond()
    if error:
      self._pending_error = error
      return

    self._params.put_bool("PondPaired", False)

    self._paired = False
    self._pending_unpaired = True

  def _start_status_poll(self, code: str):
    try:
      status = poll_status(code, poll_active=lambda: self._polling, max_polls=MAX_POLLS, poll_interval=POLL_INTERVAL)
      if status in ("paired", "expired"):
        if status == "paired":
          self._params.put_bool("PondPaired", True)
          self._paired = True
        self._poll_result = status
    finally:
      self._polling = False

  def _update_state(self):
    super()._update_state()

    if self._pending_code:
      self.set_enabled(True)
      self.set_value(f"Code: {self._pending_code}")

      gui_app.set_modal_overlay(BigDialog(f"Code: {self._pending_code}", f'Enter this code at "{THE_POND_URL}"'))

      self._polling = True
      threading.Thread(target=self._start_status_poll, args=(self._pending_code,), daemon=True).start()
      self._pending_code = None
      return

    if self._pending_error:
      self.set_enabled(True)

      if self._paired:
        self.set_value("Failed to unpair...")

        self._status_clear_time = time.monotonic()
      else:
        self.set_value("")

        gui_app.set_modal_overlay(BigDialog("Pairing failed...", self._pending_error))

      self._pending_error = None
      return

    if self._pending_unpaired:
      self.set_enabled(True)
      self.set_value("Unpaired!")

      self._pending_unpaired = False
      self._status_clear_time = time.monotonic()
      return

    if self._poll_result:
      if self._poll_result == "paired":
        self._params.put_bool("PondPaired", True)

        self.set_value("Paired!")

        gui_app.set_modal_overlay(BigDialog("Paired!", "Device successfully paired to The Pond!"))

        self._paired = True
      elif self._poll_result == "expired":
        self.set_value("Code expired...")

      self._poll_result = None
      self._status_clear_time = time.monotonic()
      return

    if self._status_clear_time and time.monotonic() - self._status_clear_time > STATUS_DISPLAY_TIME:
      self._status_clear_time = None
      self.set_value("")


class QRCodeDialog(NavWidget):
  def __init__(self):
    super().__init__()

    self.set_back_callback(lambda: gui_app.set_modal_overlay(None))

    self._qr_texture: rl.Texture | None = generate_qr_code()
    self._label = UnifiedLabel(THE_POND_URL, 48, font_weight=FontWeight.BOLD,
                               text_color=rl.Color(255, 255, 255, int(255 * 0.9)),
                               alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER)

  def _render(self, rect: rl.Rectangle):
    if self._qr_texture is None:
      return -1

    qr_area_height = rect.height - LABEL_HEIGHT

    scale = min(rect.width / self._qr_texture.width, qr_area_height / self._qr_texture.height)
    qr_size = rl.Vector2(self._qr_texture.width * scale, self._qr_texture.height * scale)
    qr_pos = rl.Vector2(rect.x + (rect.width - qr_size.x) / 2, rect.y + (qr_area_height - qr_size.y) / 2)

    rl.draw_texture_ex(self._qr_texture, qr_pos, 0.0, scale, rl.WHITE)

    self._label.set_position(rect.x, qr_pos.y + qr_size.y + 20)
    self._label.set_width(int(rect.width))
    self._label.render()
    return -1

  def hide_event(self):
    super().hide_event()

    if self._qr_texture and self._qr_texture.id != 0:
      rl.unload_texture(self._qr_texture)
      self._qr_texture = None


class FrogPilotLayout(NavWidget):
  def __init__(self, back_callback: Callable):
    super().__init__()

    self.set_back_callback(back_callback)

    pond_btn = BigButton("The Pond", "", "icons_mici/settings/comma_icon.png", icon_size=(33, 60))
    pond_btn.set_click_callback(lambda: gui_app.set_modal_overlay(QRCodeDialog()))

    self._scroller = Scroller([PondPairButton(), pond_btn], snap_items=False)

  def _render(self, rect: rl.Rectangle):
    self._scroller.render(rect)
