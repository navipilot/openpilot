from __future__ import annotations
import subprocess
from pathlib import Path

import pyray as rl

from openpilot.common.basedir import BASEDIR
from openpilot.starpilot.common.starpilot_variables import ACTIVE_THEME_PATH
from openpilot.system.ui.lib.application import gui_app, FontWeight, MouseEvent, MousePos
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import Widget
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import (
  AETHER_LIST_METRICS,
  AetherContinuousSlider,
  AetherListColors,
  DEFAULT_PANEL_STYLE,
  _point_hits,
  draw_settings_panel_header,
  draw_toggle_pill,
  init_list_panel,
)

PANEL_STYLE = DEFAULT_PANEL_STYLE
SECTION_GAP = AETHER_LIST_METRICS.section_gap



class SoundsManagerView(Widget):
  def __init__(self, controller: "StarPilotSoundsLayout"):
    super().__init__()
    self._controller = controller
    self._pressed_target: str | None = None

    self._sliders: dict[str, AetherContinuousSlider] = {}
    self._slider_was_dragging: dict[str, bool] = {}
    self._toggle_rects: dict[str, rl.Rectangle] = {}

    self._init_sliders()

  def _init_sliders(self):
    for key in self._controller.VOLUME_KEYS:
      val = self._controller._params.get_int(key, return_default=True, default=100)
      
      def on_change(v, k=key):
        new_v = int(v)
        if new_v != 101 and new_v < self._controller.VOLUME_INFO[k]["min"]:
          new_v = self._controller.VOLUME_INFO[k]["min"]
        self._controller._params.put_int(k, new_v)

      slider = AetherContinuousSlider(
        min_val=0.0,
        max_val=101.0,
        step=1.0,
        current_val=float(val),
        on_change=on_change,
        title=tr(self._controller.VOLUME_INFO[key]["title"]),
        unit="%",
        labels={0.0: tr("Muted"), 101.0: tr("Auto")},
        color=AetherListColors.PRIMARY
      )
      self._sliders[key] = slider
      self._slider_was_dragging[key] = False

    cd_val = self._controller._params.get_int(self._controller.COOLDOWN_KEY, return_default=True, default=0)
    def on_cd_change(v):
      self._controller._params.put_int(self._controller.COOLDOWN_KEY, int(v))
    
    cd_slider = AetherContinuousSlider(
      min_val=0.0,
      max_val=float(self._controller.COOLDOWN_INFO["max"]),
      step=1.0,
      current_val=float(cd_val),
      on_change=on_cd_change,
      title=tr(self._controller.COOLDOWN_INFO["title"]),
      unit=" " + tr("min"),
      labels={0.0: tr("Off"), 1.0: tr("1 minute")},
      color=AetherListColors.PRIMARY
    )
    self._sliders[self._controller.COOLDOWN_KEY] = cd_slider
    self._slider_was_dragging[self._controller.COOLDOWN_KEY] = False

  def _handle_mouse_press(self, mouse_pos: MousePos):
    self._pressed_target = self._target_at(mouse_pos)
    for slider in self._sliders.values():
      slider._handle_mouse_press(mouse_pos)

  def _handle_mouse_release(self, mouse_pos: MousePos):
    for slider in self._sliders.values():
      slider._handle_mouse_release(mouse_pos)
      
    target = self._target_at(mouse_pos)
    if self._pressed_target is not None and self._pressed_target == target:
      self._activate_target(target)
    self._pressed_target = None

  def _handle_mouse_event(self, mouse_event: MouseEvent):
    for slider in self._sliders.values():
      slider._handle_mouse_event(mouse_event)

  def _target_at(self, mouse_pos: MousePos) -> str | None:
    for key, rect in self._toggle_rects.items():
      if _point_hits(mouse_pos, rect, pad_x=6, pad_y=6):
        return f"toggle:{key}"
    return None

  def _activate_target(self, target: str):
    if target.startswith("toggle:"):
      key = target.split(":", 1)[1]
      info = self._controller.ALERT_INFO.get(key)
      if info and info.get("is_enabled", lambda: True)():
        current = self._controller._params.get_bool(key)
        self._controller._params.put_bool(key, not current)

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    self._toggle_rects.clear()

    frame, _scroll_rect, _content_width = init_list_panel(rect, PANEL_STYLE)

    self._draw_header(frame.header)

    metrics = AETHER_LIST_METRICS
    actual_header_height = 100
    content_y = frame.header.y + actual_header_height
    content_h = (frame.shell.y + frame.shell.height) - content_y - metrics.panel_padding_bottom

    content_rect = rl.Rectangle(
        frame.scroll.x,
        content_y,
        frame.scroll.width,
        content_h
    )
    
    col_width = (content_rect.width - SECTION_GAP) / 2
    left_col = rl.Rectangle(content_rect.x, content_rect.y, col_width, content_rect.height)
    right_col = rl.Rectangle(content_rect.x + col_width + SECTION_GAP, content_rect.y, col_width, content_rect.height)

    self._draw_volume_section(left_col)
    self._draw_utility_section(right_col)

    for key, slider in self._sliders.items():
      is_dragging = slider._is_dragging
      if self._slider_was_dragging[key] and not is_dragging:
        if key in self._controller.VOLUME_KEYS:
          self._controller._test_sound(key)
      self._slider_was_dragging[key] = is_dragging

  def _draw_header(self, rect: rl.Rectangle):
    draw_settings_panel_header(rect, tr("Sounds & Alerts"), tr("Manage system volumes and custom alert toggles."), subtitle_size=24)

  def _draw_volume_section(self, rect: rl.Rectangle):
    num_volumes = len(self._controller.VOLUME_KEYS)
    vol_row_h = rect.height / num_volumes

    for index, key in enumerate(self._controller.VOLUME_KEYS):
      row_rect = rl.Rectangle(rect.x, rect.y + index * vol_row_h, rect.width, vol_row_h)
      self._draw_slider_row(row_rect, key, self._controller.VOLUME_INFO[key])

  def _draw_utility_section(self, rect: rl.Rectangle):
    total_elements = 7 # 1 cooldown + 6 alerts
    row_h = rect.height / total_elements

    # Cooldown Slider (Index 0)
    cd_row_rect = rl.Rectangle(rect.x, rect.y, rect.width, row_h)
    self._draw_slider_row(cd_row_rect, self._controller.COOLDOWN_KEY, self._controller.COOLDOWN_INFO)
    
    # Custom Alert Toggle Pills (Indices 1 to 6)
    for index, key in enumerate(self._controller.CUSTOM_ALERTS_KEYS):
      row_rect = rl.Rectangle(rect.x, rect.y + (index + 1) * row_h, rect.width, row_h)
      self._draw_toggle_row(row_rect, key, self._controller.ALERT_INFO[key])

  def _draw_slider_row(self, rect: rl.Rectangle, key: str, info: dict):
    slider = self._sliders[key]
    
    padded_rect = rl.Rectangle(rect.x, rect.y + 4, rect.width - 12, rect.height - 8)
    
    if not slider._is_dragging:
      current_param = self._controller._params.get_int(key, return_default=True, default=100 if key != self._controller.COOLDOWN_KEY else 0)
      if slider.current_val != current_param:
         slider.current_val = float(current_param)

    slider.render(padded_rect)

  def _draw_toggle_row(self, rect: rl.Rectangle, key: str, info: dict):
    padded_rect = rl.Rectangle(rect.x, rect.y + 4, rect.width - 12, rect.height - 8)
    
    current_val = self._controller._params.get_bool(key)
    is_enabled = info.get("is_enabled", lambda: True)()
    
    mouse_pos = gui_app.last_mouse_event.pos
    hovered = _point_hits(mouse_pos, padded_rect, pad_x=6, pad_y=6)
    pressed = self._pressed_target == f"toggle:{key}"
    
    status_str = tr("ON") if current_val else tr("OFF")
    if not is_enabled: status_str = tr(info.get("disabled_label", "UNAVAILABLE"))
    
    draw_toggle_pill(padded_rect, current_val, is_enabled, tr(info["title"]), status_str, hovered, pressed)
    
    self._toggle_rects[key] = padded_rect


class StarPilotSoundsLayout(StarPilotPanel):
  COOLDOWN_KEY = "SwitchbackModeCooldown"
  VOLUME_KEYS = [
    "BelowSteerSpeedVolume",
    "DisengageVolume",
    "EngageVolume",
    "PromptVolume",
    "PromptDistractedVolume",
    "RefuseVolume",
    "WarningSoftVolume",
    "WarningImmediateVolume",
  ]
  CUSTOM_ALERTS_KEYS = [
    "GoatScream",
    "GoatScreamCriticalAlerts",
    "GreenLightAlert",
    "LeadDepartingAlert",
    "LoudBlindspotAlert",
    "SpeedLimitChangedAlert",
  ]

  COOLDOWN_INFO = {"title": tr_noop("Switchback Mode Cooldown"), "min": 0, "max": 30}
  VOLUME_INFO = {
    "BelowSteerSpeedVolume": {"title": tr_noop("Min Steer Speed Alert"), "min": 0},
    "DisengageVolume": {"title": tr_noop("Disengage Volume"), "min": 0},
    "EngageVolume": {"title": tr_noop("Engage Volume"), "min": 0},
    "PromptVolume": {"title": tr_noop("Prompt Volume"), "min": 0},
    "PromptDistractedVolume": {"title": tr_noop("Distracted Volume"), "min": 0},
    "RefuseVolume": {"title": tr_noop("Refuse Volume"), "min": 0},
    "WarningSoftVolume": {"title": tr_noop("Warning Soft"), "min": 25},
    "WarningImmediateVolume": {"title": tr_noop("Warning Immediate"), "min": 25},
  }

  _sound_player_process = None

  def __init__(self):
    super().__init__()
    self._init_sound_player()

    self.ALERT_INFO = {
      "GoatScream": {"title": tr_noop("Goat Scream")},
      "GoatScreamCriticalAlerts": {"title": tr_noop("Goat Critical")},
      "GreenLightAlert": {"title": tr_noop("Green Light")},
      "LeadDepartingAlert": {"title": tr_noop("Lead Departure")},
      "LoudBlindspotAlert": {
        "title": tr_noop("Loud Blindspot"), 
        "is_enabled": lambda: starpilot_state.car_state.hasBSM,
        "disabled_label": tr_noop("Needs BSM")
      },
      "SpeedLimitChangedAlert": {
        "title": tr_noop("Speed Limit"),
        "is_enabled": lambda: self._params.get_bool("ShowSpeedLimits") or (
          starpilot_state.car_state.hasOpenpilotLongitudinal and self._params.get_bool("SpeedLimitController")
        ),
        "disabled_label": tr_noop("Needs Speed Limits")
      },
    }

    self._manager_view = SoundsManagerView(self)

  def _render(self, rect: rl.Rectangle):
    self._manager_view.render(rect)

  def show_event(self):
    super().show_event()

  def hide_event(self):
    super().hide_event()

  @classmethod
  def _init_sound_player(cls):
    if cls._sound_player_process is not None and cls._sound_player_process.poll() is None: return
    program = """
import numpy as np
import sounddevice as sd
import sys
import wave
while True:
  try:
    line = sys.stdin.readline()
    if not line: break
    path, volume = line.strip().split('|')
    with wave.open(path, 'rb') as sound_file:
      audio = np.frombuffer(sound_file.readframes(sound_file.getnframes()), dtype=np.int16).astype(np.float32) / 32768.0
      sd.play(audio * float(volume), sound_file.getframerate())
    sd.wait()
  except Exception:
    sd._terminate()
    sd._initialize()
"""
    cls._sound_player_process = subprocess.Popen(["python3", "-u", "-c", program], stdin=subprocess.PIPE, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

  def _test_sound(self, key: str):
    base_name = key.replace("Volume", "")
    if ui_state.started:
      alert_name = "belowSteerSpeed" if base_name == "BelowSteerSpeed" else base_name[0].lower() + base_name[1:]
      self._params_memory.put("TestAlert", alert_name)
    else:
      self._play_sound_offroad(key)

  def _play_sound_offroad(self, key: str):
    base_name = key.replace("Volume", "")
    preview_base_name = "Prompt" if base_name == "BelowSteerSpeed" else base_name
    snake_case = "".join(["_" + c.lower() if c.isupper() else c for c in preview_base_name]).lstrip("_")
    stock_path = Path(BASEDIR) / "selfdrive" / "assets" / "sounds" / f"{snake_case}.wav"
    theme_path = ACTIVE_THEME_PATH / "sounds" / f"{snake_case}.wav"
    sound_path = theme_path if theme_path.exists() else stock_path
    if not sound_path.exists(): return
    volume = self._params.get_int(key, return_default=True, default=100) / 100.0
    if self._sound_player_process.poll() is not None:
      self._sound_player_process = None
      self._init_sound_player()
    try:
      self._sound_player_process.stdin.write(f"{sound_path}|{volume}\n".encode())
      self._sound_player_process.stdin.flush()
    except: pass
