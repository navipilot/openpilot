from __future__ import annotations
import subprocess
import time
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
from openpilot.selfdrive.ui.layouts.settings.starpilot.tabbed_panel import TabSectionSpec, TabbedSectionHost
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import TileGrid, ToggleTile, SPACING

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
    self._section_tabs = TabbedSectionHost([
      TabSectionSpec("volume_control", "Volumes", StarPilotVolumeControlLayout()),
      TabSectionSpec("custom_alerts", "Alerts", StarPilotCustomAlertsLayout()),
    ])

  def set_navigate_callback(self, callback):
    self._section_tabs.set_navigate_callback(callback)

  def set_back_callback(self, callback):
    self._section_tabs.set_back_callback(callback)

  def _render(self, rect):
    self._section_tabs.render(rect)

  def set_current_sub_panel(self, sub_panel: str):
    self._section_tabs.set_current_sub_panel(sub_panel)

  def show_event(self):
    self._section_tabs.show_event()

  def hide_event(self):
    self._section_tabs.hide_event()

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

    self.SECTIONS = [
      {
        "title": tr_noop("Volume Levels"),
        "categories": self._build_volume_categories(),
      },
      {
        "title": tr_noop("Safety & Cooldown"),
        "categories": self._build_safety_categories(),
      }
    ]
    self._rebuild_grid()

  def _build_volume_categories(self):
    cats = []
    for key in StarPilotSoundsLayout.VOLUME_KEYS:
      info = self.VOLUME_INFO[key]

      def get_val(k=key):
        return float(self._params.get_int(k, return_default=True, default=100))

      def set_val(val, k=key):
        new_v = int(val)
        if new_v != 101 and new_v < self.VOLUME_INFO[k]["min"]:
          new_v = self.VOLUME_INFO[k]["min"]
        self._params.put_int(k, new_v)

      def test_cb(k=key):
        self._test_sound(k)

      cats.append({
        "title": info["title"],
        "type": "slider",
        "get_value": get_val,
        "set_value": set_val,
        "on_test": test_cb,
        "min_val": 0,
        "max_val": 101,
        "step": 1,
        "unit": "%",
        "labels": {0: tr("Muted"), 101: tr("Auto")},
        "icon": info["icon"],
        "color": "#3B82F6",
      })
    return cats

  def _build_safety_categories(self):
    def get_cooldown_val():
      return float(self._params.get_int(StarPilotSoundsLayout.COOLDOWN_KEY, return_default=True, default=0))

    def set_cooldown_val(val):
      self._params.put_int(StarPilotSoundsLayout.COOLDOWN_KEY, int(val))

    return [{
      "title": self.COOLDOWN_INFO["title"],
      "type": "slider",
      "get_value": get_cooldown_val,
      "set_value": set_cooldown_val,
      "min_val": 0,
      "max_val": float(self.COOLDOWN_INFO["max"]),
      "step": 1,
      "unit": " " + tr("min"),
      "labels": {0: tr("Off"), 1: tr("1 minute")},
      "icon": self.COOLDOWN_INFO["icon"],
      "color": "#3B82F6",
    }]

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
    self._tile_grid = TileGrid(columns=2, padding=SPACING.tile_gap, uniform_width=True)
    self.CATEGORIES = []
    for key in StarPilotSoundsLayout.CUSTOM_ALERTS_KEYS:
      info = self.ALERT_INFO[key]
      self.CATEGORIES.append({
        "title": info["title"],
        "type": "toggle",
        "get_state": lambda k=key: self._params.get_bool(k),
        "set_state": lambda s, k=key: self._params.put_bool(k, s),
        "icon": info["icon"],
        "color": "#3B82F6",
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
