from __future__ import annotations
import subprocess
from pathlib import Path

import pyray as rl

from openpilot.common.basedir import BASEDIR
from openpilot.starpilot.common.starpilot_variables import ACTIVE_THEME_PATH
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import TileGrid, ToggleTile, AetherSliderDialog, RadioTileGroup

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

  def __init__(self):
    super().__init__()
    self._section_names = ["volume_control", "custom_alerts"]
    self._active_section = self._section_names[0]
    self._sub_panels = {
      "volume_control": StarPilotVolumeControlLayout(),
      "custom_alerts": StarPilotCustomAlertsLayout(),
    }
    self._section_tabs = RadioTileGroup(
      "",
      [tr("Volumes"), tr("Alerts")],
      0,
      self._on_section_change,
    )

    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(lambda sub_panel, section_name=name: self._navigate_to_child(section_name, sub_panel))
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

  def _on_section_change(self, index: int):
    if 0 <= index < len(self._section_names):
      self._set_active_section(self._section_names[index])

  def _set_active_section(self, section_name: str, child_panel: str = ""):
    if section_name not in self._sub_panels:
      return

    if section_name != self._active_section:
      self._sub_panels[self._active_section].hide_event()
      self._active_section = section_name
      self._sub_panels[self._active_section].show_event()

    self._section_tabs.set_index(self._section_names.index(section_name))
    panel = self._sub_panels[section_name]
    if hasattr(panel, 'set_current_sub_panel'):
      panel.set_current_sub_panel(child_panel)

  def _navigate_to_child(self, section_name: str, child_panel: str):
    self._set_active_section(section_name, child_panel)
    if self._navigate_callback:
      self._navigate_callback(f"{section_name}:{child_panel}")

  def set_current_sub_panel(self, sub_panel: str):
    super().set_current_sub_panel(sub_panel)
    if not sub_panel:
      self._set_active_section(self._active_section, "")
      return

    if ":" in sub_panel:
      section_name, child_panel = sub_panel.split(":", 1)
      self._set_active_section(section_name, child_panel)
    elif sub_panel in self._section_names:
      self._set_active_section(sub_panel)

  def _render(self, rect):
    tab_rect = rl.Rectangle(rect.x, rect.y, rect.width, 110)
    panel_rect = rl.Rectangle(rect.x, rect.y + 140, rect.width, rect.height - 140)
    self._section_tabs.render(tab_rect)
    self._sub_panels[self._active_section].render(panel_rect)

  def show_event(self):
    super().show_event()
    self._sub_panels[self._active_section].show_event()

  def hide_event(self):
    super().hide_event()
    self._sub_panels[self._active_section].hide_event()

class StarPilotVolumeControlLayout(StarPilotPanel):
  COOLDOWN_INFO = {"title": tr_noop("Switchback Mode Cooldown"), "icon": "toggle_icons/icon_mute.png", "min": 0, "max": 30}
  VOLUME_INFO = {
    "BelowSteerSpeedVolume": {"title": tr_noop("Min Steer Speed Alert"), "icon": "toggle_icons/icon_mute.png", "min": 0},
    "DisengageVolume": {"title": tr_noop("Disengage Volume"), "icon": "toggle_icons/icon_mute.png", "min": 0},
    "EngageVolume": {"title": tr_noop("Engage Volume"), "icon": "toggle_icons/icon_green_light.png", "min": 0},
    "PromptVolume": {"title": tr_noop("Prompt Volume"), "icon": "toggle_icons/icon_message.png", "min": 0},
    "PromptDistractedVolume": {"title": tr_noop("Distracted Volume"), "icon": "toggle_icons/icon_display.png", "min": 0},
    "RefuseVolume": {"title": tr_noop("Refuse Volume"), "icon": "toggle_icons/icon_mute.png", "min": 0},
    "WarningSoftVolume": {"title": tr_noop("Warning Soft"), "icon": "toggle_icons/icon_conditional.png", "min": 25},
    "WarningImmediateVolume": {"title": tr_noop("Warning Immediate"), "icon": "toggle_icons/icon_conditional.png", "min": 25},
  }

  _sound_player_process = None

  def __init__(self):
    super().__init__()
    self._init_sound_player()
    self._tile_grid = TileGrid(columns=2, padding=20, uniform_width=True)
    
    self.CATEGORIES = []
    for key in StarPilotSoundsLayout.VOLUME_KEYS:
      info = self.VOLUME_INFO[key]
      
      def get_val(k=key):
        v = self._params.get_int(k, return_default=True, default=100)
        if v == 0: return tr("Muted")
        if v == 101: return tr("Auto")
        return f"{v}%"

      def on_click(k=key, i=info):
        self._show_volume_selector(k, i)

      self.CATEGORIES.append({
        "title": info["title"],
        "type": "value",
        "get_value": get_val,
        "on_click": on_click,
        "icon": info["icon"],
        "color": "#E63956"
      })

    def get_cooldown_val():
      v = self._params.get_int(StarPilotSoundsLayout.COOLDOWN_KEY, return_default=True, default=0)
      if v == 0:
        return tr("Off")
      if v == 1:
        return tr("1 minute")
      return f"{v} {tr('minutes')}"

    self.CATEGORIES.append({
      "title": self.COOLDOWN_INFO["title"],
      "type": "value",
      "get_value": get_cooldown_val,
      "on_click": self._show_cooldown_selector,
      "icon": self.COOLDOWN_INFO["icon"],
      "color": "#E63956"
    })

    self._rebuild_grid()

  def _show_volume_selector(self, key: str, info: dict):
    current_v = self._params.get_int(key, return_default=True, default=100)
    
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        new_v = int(val)
        if new_v != 101 and new_v < info["min"]:
          new_v = info["min"]
        self._params.put_int(key, new_v)
        self._test_sound(key)
        self._rebuild_grid()

    gui_app.set_modal_overlay(AetherSliderDialog(
      tr(info["title"]), 0, 101, 1, current_v, on_close,
      unit="%", labels={0: tr("Muted"), 101: tr("Auto")}, color="#E63956"
    ))

  def _show_cooldown_selector(self):
    current_v = self._params.get_int(StarPilotSoundsLayout.COOLDOWN_KEY, return_default=True, default=0)

    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(StarPilotSoundsLayout.COOLDOWN_KEY, int(val))
        self._rebuild_grid()

    gui_app.set_modal_overlay(AetherSliderDialog(
      tr(self.COOLDOWN_INFO["title"]), 0, self.COOLDOWN_INFO["max"], 1, current_v, on_close,
      unit=" min", labels={0: tr("Off")}, color="#E63956"
    ))

  @classmethod
  def _init_sound_player(cls):
    if cls._sound_player_process is not None: return
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
    sound_file = wave.open(path, 'rb')
    audio = np.frombuffer(sound_file.readframes(sound_file.getnframes()), dtype=np.int16).astype(np.float32) / 32768.0
    sd.play(audio * float(volume), sound_file.getframerate())
    sd.wait()
  except: pass
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
    try:
      self._sound_player_process.stdin.write(f"{sound_path}|{volume}\n".encode())
      self._sound_player_process.stdin.flush()
    except: pass

class StarPilotCustomAlertsLayout(StarPilotPanel):
  ALERT_INFO = {
    "GoatScream": {"title": tr_noop("Goat Scream"), "icon": "toggle_icons/icon_sound.png"},
    "GoatScreamCriticalAlerts": {"title": tr_noop("Goat Critical"), "icon": "toggle_icons/icon_sound.png"},
    "GreenLightAlert": {"title": tr_noop("Green Light"), "icon": "toggle_icons/icon_green_light.png"},
    "LeadDepartingAlert": {"title": tr_noop("Lead Departure"), "icon": "toggle_icons/icon_steering.png"},
    "LoudBlindspotAlert": {"title": tr_noop("Loud Blindspot"), "icon": "toggle_icons/icon_display.png"},
    "SpeedLimitChangedAlert": {"title": tr_noop("Speed Limit"), "icon": "toggle_icons/icon_speed_limit.png"},
  }

  def __init__(self):
    super().__init__()
    self._tile_grid = TileGrid(columns=2, padding=20, uniform_width=True)
    self.CATEGORIES = []
    for key in StarPilotSoundsLayout.CUSTOM_ALERTS_KEYS:
      info = self.ALERT_INFO[key]
      self.CATEGORIES.append({
        "title": info["title"],
        "type": "toggle",
        "get_state": lambda k=key: self._params.get_bool(k),
        "set_state": lambda s, k=key: self._params.put_bool(k, s),
        "icon": info["icon"],
        "color": "#E63956",
        "key": key # Store for visibility check
      })
    self._rebuild_grid()

  def _rebuild_grid(self):
    if not self.CATEGORIES:
      return
    self._tile_grid.clear()

    for cat in self.CATEGORIES:
      key = cat.get("key")
      is_enabled = lambda: True
      disabled_label = ""

      if key == "LoudBlindspotAlert":
        is_enabled = lambda: starpilot_state.car_state.hasBSM
        disabled_label = tr_noop("Needs BSM")
      elif key == "SpeedLimitChangedAlert":
        is_enabled = lambda: self._params.get_bool("ShowSpeedLimits") or (
          starpilot_state.car_state.hasOpenpilotLongitudinal and self._params.get_bool("SpeedLimitController")
        )
        disabled_label = tr_noop("Needs Speed Limits")

      tile = ToggleTile(
        title=tr(cat["title"]),
        get_state=cat["get_state"],
        set_state=cat["set_state"],
        icon_path=cat.get("icon"),
        bg_color=cat.get("color"),
        is_enabled=is_enabled,
        disabled_label=tr(disabled_label) if disabled_label else "",
      )
      self._tile_grid.add_tile(tile)
