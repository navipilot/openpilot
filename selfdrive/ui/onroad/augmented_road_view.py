import time
import numpy as np
import pyray as rl
from cereal import log, messaging
from msgq.visionipc import VisionStreamType
from openpilot.common.constants import CV
from openpilot.selfdrive.ui import UI_BORDER_SIZE
from openpilot.selfdrive.ui.ui_state import ui_state, UIStatus
from openpilot.selfdrive.ui.onroad.alert_renderer import AlertRenderer, ALERT_COLORS, AlertStatus
from openpilot.selfdrive.ui.onroad.driver_state import DriverStateRenderer
from openpilot.selfdrive.ui.onroad.hud_renderer import HudRenderer
from openpilot.selfdrive.ui.onroad.model_renderer import ModelRenderer
from openpilot.selfdrive.ui.onroad.cameraview import CameraView
from openpilot.system.ui.lib.application import gui_app, FontWeight
from openpilot.common.transformations.camera import DEVICE_CAMERAS, DeviceCameraConfig, view_frame_from_device_frame
from openpilot.common.transformations.orientation import rot_from_euler

OpState = log.SelfdriveState.OpenpilotState
CALIBRATED = log.LiveCalibrationData.Status.calibrated
ROAD_CAM = VisionStreamType.VISION_STREAM_ROAD
WIDE_CAM = VisionStreamType.VISION_STREAM_WIDE_ROAD
DEFAULT_DEVICE_CAMERA = DEVICE_CAMERAS["tici", "ar0231"]

BORDER_COLORS = {
  UIStatus.DISENGAGED: rl.Color(0x12, 0x28, 0x39, 0xFF),  # Blue for disengaged state
  UIStatus.OVERRIDE: rl.Color(0x89, 0x92, 0x8D, 0xFF),  # Gray for override state
  UIStatus.ENGAGED: rl.Color(0x16, 0x7F, 0x40, 0xFF),  # Green for engaged state
}

WIDE_CAM_MAX_SPEED = 10.0  # m/s (22 mph)
ROAD_CAM_MIN_SPEED = 15.0  # m/s (34 mph)
INF_POINT = np.array([1000.0, 0.0, 0.0])


class MinSteerSpeedBanner:
  """One-shot-per-drive banner that stays visible for the first below-min-steer interval."""

  def __init__(self):
    self._shown_this_drive = False
    self._showing_interval = False
    self._has_been_above_min = False
    self._was_under_min = False
    self._last_started_frame = -1
    self._font = gui_app.font(FontWeight.BOLD)

  def _reset(self):
    self._shown_this_drive = False
    self._showing_interval = False
    self._has_been_above_min = False
    self._was_under_min = False

  def _get_message(self, min_steer_speed: float) -> str:
    speed_units = CV.MS_TO_KPH if ui_state.is_metric else CV.MS_TO_MPH
    speed = int(round(min_steer_speed * speed_units))
    unit = "km/h" if ui_state.is_metric else "mph"
    return f"Steer Unavailable Under {speed} {unit}"

  def _update_state(self):
    if not ui_state.started:
      self._last_started_frame = -1
      self._reset()
      return

    if ui_state.started_frame != self._last_started_frame:
      self._last_started_frame = ui_state.started_frame
      self._reset()

    sm = ui_state.sm
    if sm.recv_frame["carParams"] < ui_state.started_frame or sm.recv_frame["carState"] < ui_state.started_frame:
      return

    min_steer_speed = float(sm["carParams"].minSteerSpeed)
    if min_steer_speed <= 0:
      self._showing_interval = False
      self._was_under_min = False
      return

    under_min = float(sm["carState"].vEgo) < min_steer_speed
    if not under_min:
      self._has_been_above_min = True

    crossed_below = under_min and not self._was_under_min
    if (not self._shown_this_drive) and crossed_below and self._has_been_above_min:
      self._showing_interval = True
      self._shown_this_drive = True

    if self._showing_interval and not under_min:
      self._showing_interval = False

    self._was_under_min = under_min

  def render(self, rect: rl.Rectangle):
    self._update_state()

    if not self._showing_interval:
      return

    min_steer_speed = float(ui_state.sm["carParams"].minSteerSpeed)
    if min_steer_speed <= 0:
      return

    color = ALERT_COLORS[AlertStatus.userPrompt]
    color = rl.Color(color.r, color.g, color.b, int(255 * 0.93))
    translucent = rl.Color(color.r, color.g, color.b, 0)
    dropdown_height = min(200, int(rect.height * 0.38))
    solid_height = max(34, int(dropdown_height * 0.22))

    rl.draw_rectangle(int(rect.x), int(rect.y), int(rect.width), solid_height, color)
    rl.draw_rectangle_gradient_v(
      int(rect.x),
      int(rect.y + solid_height),
      int(rect.width),
      int(dropdown_height - solid_height),
      color,
      translucent,
    )

    text = self._get_message(min_steer_speed)
    font_size = 52
    max_text_width = rect.width - 100
    text_size = rl.measure_text_ex(self._font, text, font_size, 0)
    while font_size > 36 and text_size.x > max_text_width:
      font_size -= 2
      text_size = rl.measure_text_ex(self._font, text, font_size, 0)

    text_pos = rl.Vector2(
      rect.x + (rect.width - text_size.x) / 2,
      rect.y + max(12, (dropdown_height * 0.34) - (text_size.y / 2)),
    )
    rl.draw_text_ex(self._font, text, text_pos, font_size, 0, rl.Color(255, 255, 255, 242))


class AugmentedRoadView(CameraView):
  def __init__(self, stream_type: VisionStreamType = VisionStreamType.VISION_STREAM_ROAD):
    super().__init__("camerad", stream_type)
    self._set_placeholder_color(BORDER_COLORS[UIStatus.DISENGAGED])

    self.device_camera: DeviceCameraConfig | None = None
    self.view_from_calib = view_frame_from_device_frame.copy()
    self.view_from_wide_calib = view_frame_from_device_frame.copy()

    self._matrix_cache_key = (0, 0.0, 0.0, stream_type)
    self._cached_matrix: np.ndarray | None = None
    self._content_rect = rl.Rectangle()

    self.model_renderer = ModelRenderer()
    self._hud_renderer = HudRenderer()
    self.alert_renderer = AlertRenderer()
    self.driver_state_renderer = DriverStateRenderer()
    self._min_steer_speed_banner = MinSteerSpeedBanner()

    # debug
    self._pm = messaging.PubMaster(['uiDebug'])

  def _render(self, rect):
    # Only render when system is started to avoid invalid data access
    start_draw = time.monotonic()
    if not ui_state.started:
      return

    self._switch_stream_if_needed(ui_state.sm)

    # Update calibration before rendering
    self._update_calibration()

    # Create inner content area with border padding
    self._content_rect = rl.Rectangle(
      rect.x + UI_BORDER_SIZE,
      rect.y + UI_BORDER_SIZE,
      rect.width - 2 * UI_BORDER_SIZE,
      rect.height - 2 * UI_BORDER_SIZE,
    )

    # Enable scissor mode to clip all rendering within content rectangle boundaries
    # This creates a rendering viewport that prevents graphics from drawing outside the border
    rl.begin_scissor_mode(
      int(self._content_rect.x),
      int(self._content_rect.y),
      int(self._content_rect.width),
      int(self._content_rect.height)
    )

    # Render the base camera view
    super()._render(self._content_rect)

    # Draw all UI overlays
    self.model_renderer.render(self._content_rect)
    self._hud_renderer.render(self._content_rect)
    self.alert_renderer.render(self._content_rect)
    self.driver_state_renderer.render(self._content_rect)
    self._min_steer_speed_banner.render(self._content_rect)

    # Custom UI extension point - add custom overlays here
    # Use self._content_rect for positioning within camera bounds

    # End clipping region
    rl.end_scissor_mode()

    # Draw colored border based on driving state
    self._draw_border(rect)

    # publish uiDebug
    msg = messaging.new_message('uiDebug')
    msg.uiDebug.drawTimeMillis = (time.monotonic() - start_draw) * 1000
    self._pm.send('uiDebug', msg)

  def _handle_mouse_press(self, _):
    if not self._hud_renderer.user_interacting() and self._click_callback is not None:
      self._click_callback()

  def _handle_mouse_release(self, _):
    # We only call click callback on press if not interacting with HUD
    pass

  def _draw_border(self, rect: rl.Rectangle):
    rl.draw_rectangle_lines_ex(rect, UI_BORDER_SIZE, rl.BLACK)
    border_roundness = 0.12
    border_color = BORDER_COLORS.get(ui_state.status, BORDER_COLORS[UIStatus.DISENGAGED])
    border_rect = rl.Rectangle(rect.x + UI_BORDER_SIZE, rect.y + UI_BORDER_SIZE,
                               rect.width - 2 * UI_BORDER_SIZE, rect.height - 2 * UI_BORDER_SIZE)
    rl.draw_rectangle_rounded_lines_ex(border_rect, border_roundness, 10, UI_BORDER_SIZE, border_color)

  def _switch_stream_if_needed(self, sm):
    if sm['selfdriveState'].experimentalMode and WIDE_CAM in self.available_streams:
      v_ego = sm['carState'].vEgo
      if v_ego < WIDE_CAM_MAX_SPEED:
        target = WIDE_CAM
      elif v_ego > ROAD_CAM_MIN_SPEED:
        target = ROAD_CAM
      else:
        # Hysteresis zone - keep current stream
        target = self.stream_type
    else:
      target = ROAD_CAM

    if self.stream_type != target:
      self.switch_stream(target)

  def _update_calibration(self):
    # Update device camera if not already set
    sm = ui_state.sm
    if not self.device_camera and sm.seen['roadCameraState'] and sm.seen['deviceState']:
      self.device_camera = DEVICE_CAMERAS[(str(sm['deviceState'].deviceType), str(sm['roadCameraState'].sensor))]

    # Check if live calibration data is available and valid
    if not (sm.updated["liveCalibration"] and sm.valid['liveCalibration']):
      return

    calib = sm['liveCalibration']
    if len(calib.rpyCalib) != 3 or calib.calStatus != CALIBRATED:
      return

    # Update view_from_calib matrix
    device_from_calib = rot_from_euler(calib.rpyCalib)
    self.view_from_calib = view_frame_from_device_frame @ device_from_calib

    # Update wide calibration if available
    if hasattr(calib, 'wideFromDeviceEuler') and len(calib.wideFromDeviceEuler) == 3:
      wide_from_device = rot_from_euler(calib.wideFromDeviceEuler)
      self.view_from_wide_calib = view_frame_from_device_frame @ wide_from_device @ device_from_calib

  def _calc_frame_matrix(self, rect: rl.Rectangle) -> np.ndarray:
    # Check if we can use cached matrix
    cache_key = (
      ui_state.sm.recv_frame['liveCalibration'],
      self._content_rect.width,
      self._content_rect.height,
      self.stream_type
    )
    if cache_key == self._matrix_cache_key and self._cached_matrix is not None:
      return self._cached_matrix

    # Get camera configuration
    device_camera = self.device_camera or DEFAULT_DEVICE_CAMERA
    is_wide_camera = self.stream_type == WIDE_CAM
    intrinsic = device_camera.ecam.intrinsics if is_wide_camera else device_camera.fcam.intrinsics
    calibration = self.view_from_wide_calib if is_wide_camera else self.view_from_calib
    zoom = 2.0 if is_wide_camera else 1.1

    # Calculate transforms for vanishing point
    calib_transform = intrinsic @ calibration
    kep = calib_transform @ INF_POINT

    # Calculate center points and dimensions
    x, y = self._content_rect.x, self._content_rect.y
    w, h = self._content_rect.width, self._content_rect.height
    cx, cy = intrinsic[0, 2], intrinsic[1, 2]

    # Ensure zoom views the whole area
    zoom = max(zoom, w / (2 * cx), h / (2 * cy))

    # Calculate max allowed offsets with margins
    margin = 5
    max_x_offset = max(0.0, cx * zoom - w / 2 - margin)
    max_y_offset = max(0.0, cy * zoom - h / 2 - margin)

    # Calculate and clamp offsets to prevent out-of-bounds issues
    try:
      if abs(kep[2]) > 1e-6:
        x_offset = np.clip((kep[0] / kep[2] - cx) * zoom, -max_x_offset, max_x_offset)
        y_offset = np.clip((kep[1] / kep[2] - cy) * zoom, -max_y_offset, max_y_offset)
      else:
        x_offset, y_offset = 0, 0
    except (ZeroDivisionError, OverflowError):
      x_offset, y_offset = 0, 0

    # Cache the computed transformation matrix to avoid recalculations
    self._matrix_cache_key = cache_key
    self._cached_matrix = np.array([
      [zoom * 2 * cx / w, 0, -x_offset / w * 2],
      [0, zoom * 2 * cy / h, -y_offset / h * 2],
      [0, 0, 1.0]
    ])

    video_transform = np.array([
      [zoom, 0.0, (w / 2 + x - x_offset) - (cx * zoom)],
      [0.0, zoom, (h / 2 + y - y_offset) - (cy * zoom)],
      [0.0, 0.0, 1.0]
    ])
    self.model_renderer.set_transform(video_transform @ calib_transform)

    return self._cached_matrix


if __name__ == "__main__":
  gui_app.init_window("OnRoad Camera View")
  road_camera_view = AugmentedRoadView(ROAD_CAM)
  gui_app.push_widget(road_camera_view)
  print("***press space to switch camera view***")
  try:
    for _ in gui_app.render():
      ui_state.update()
      if rl.is_key_released(rl.KeyboardKey.KEY_SPACE):
        if WIDE_CAM in road_camera_view.available_streams:
          stream = ROAD_CAM if road_camera_view.stream_type == WIDE_CAM else WIDE_CAM
          road_camera_view.switch_stream(stream)
  finally:
    road_camera_view.close()
