from __future__ import annotations
import subprocess
from pathlib import Path

from openpilot.common.basedir import BASEDIR
from openpilot.frogpilot.common.frogpilot_variables import ACTIVE_THEME_PATH
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item, toggle_item, value_button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotSoundsLayout(StarPilotPanel):
  VOLUME_KEYS = [
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
    "GreenLightAlert",
    "LeadDepartingAlert",
    "LoudBlindspotAlert",
    "SpeedLimitChangedAlert",
  ]

  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Alert Volume Controller"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Set how loud each type of openpilot alert is</b> to keep routine prompts from becoming distracting."),
        callback=lambda: self._navigate_to("volume_control"),
        icon="toggle_icons/icon_mute.png",
        starpilot_icon=True,
      ),
      button_item(
        tr_noop("StarPilot Alerts"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Optional StarPilot alerts</b> that highlight driving events in a more noticeable way."),
        callback=lambda: self._navigate_to("custom_alerts"),
        icon="toggle_icons/icon_green_light.png",
        starpilot_icon=True,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

    self._sub_panels = {
      "volume_control": StarPilotVolumeControlLayout(),
      "custom_alerts": StarPilotCustomAlertsLayout(),
    }

    # Wire up navigation callbacks for sub-panels
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

  def refresh_visibility(self):
    for panel in self._sub_panels.values():
      if hasattr(panel, 'refresh_visibility'):
        panel.refresh_visibility()

class StarPilotVolumeControlLayout(StarPilotPanel):
  VOLUME_INFO = {
    "DisengageVolume": {
      "title": tr_noop("Disengage Volume"),
      "desc": tr_noop(
        "<b>Set the volume for alerts when openpilot disengages.</b><br><br>Examples include: \"Cruise Fault: Restart the Car\", \"Parking Brake Engaged\", \"Pedal Pressed\"."
      ),
      "min": 0,
    },
    "EngageVolume": {
      "title": tr_noop("Engage Volume"),
      "desc": tr_noop("<b>Set the volume for the chime when openpilot engages</b>, such as after pressing the \"RESUME\" or \"SET\" steering wheel buttons."),
      "min": 0,
    },
    "PromptVolume": {
      "title": tr_noop("Prompt Volume"),
      "desc": tr_noop(
        "<b>Set the volume for prompts that need attention.</b><br><br>Examples include: \"Car Detected in Blindspot\", \"Steering Temporarily Unavailable\", \"Turn Exceeds Steering Limit\"."
      ),
      "min": 0,
    },
    "PromptDistractedVolume": {
      "title": tr_noop("Prompt Distracted Volume"),
      "desc": tr_noop(
        "<b>Set the volume for prompts when openpilot detects driver distraction or unresponsiveness.</b><br><br>Examples include: \"Pay Attention\", \"Touch Steering Wheel\"."
      ),
      "min": 0,
    },
    "RefuseVolume": {
      "title": tr_noop("Refuse Volume"),
      "desc": tr_noop(
        "<b>Set the volume for alerts when openpilot refuses to engage.</b><br><br>Examples include: \"Brake Hold Active\", \"Door Open\", \"Seatbelt Unlatched\"."
      ),
      "min": 0,
    },
    "WarningSoftVolume": {
      "title": tr_noop("Warning Soft Volume"),
      "desc": tr_noop(
        "<b>Set the volume for softer warnings about potential risks.</b><br><br>Examples include: \"BRAKE! Risk of Collision\", \"Steering Temporarily Unavailable\"."
      ),
      "min": 25,
    },
    "WarningImmediateVolume": {
      "title": tr_noop("Warning Immediate Volume"),
      "desc": tr_noop(
        "<b>Set the volume for the loudest warnings that require urgent attention.</b><br><br>Examples include: \"DISENGAGE IMMEDIATELY — Driver Distracted\", \"DISENGAGE IMMEDIATELY — Driver Unresponsive\"."
      ),
      "min": 25,
    },
  }

  _sound_player_process = None

  def __init__(self):
    super().__init__()
    self._init_sound_player()

    volume_labels = {0.0: tr("Muted"), 101.0: tr("Auto")}
    for i in range(1, 101):
      volume_labels[float(i)] = f"{i}%"

    items = []
    for key in StarPilotSoundsLayout.VOLUME_KEYS:
      info = self.VOLUME_INFO[key]
      items.append(
        value_button_item(
          info["title"],
          key,
          min_val=info["min"],
          max_val=101,
          step=1,
          button_text="Test",
          button_callback=lambda k=key: self._test_sound(k),
          description=info["desc"],
          labels=volume_labels,
        )
      )

    self._scroller = Scroller(items, line_separator=True, spacing=0)

  @classmethod
  def _init_sound_player(cls):
    if cls._sound_player_process is not None:
      return

    program = """
import numpy as np
import sounddevice as sd
import sys
import wave

while True:
  try:
    line = sys.stdin.readline()
    if not line:
      break
    path, volume = line.strip().split('|')

    sound_file = wave.open(path, 'rb')
    audio = np.frombuffer(sound_file.readframes(sound_file.getnframes()), dtype=np.int16).astype(np.float32) / 32768.0

    sd.play(audio * float(volume), sound_file.getframerate())
    sd.wait()
  except Exception:
    pass
"""

    cls._sound_player_process = subprocess.Popen(
      ["python3", "-u", "-c", program],
      stdin=subprocess.PIPE,
      stdout=subprocess.DEVNULL,
      stderr=subprocess.DEVNULL,
    )

  def _play_sound_offroad(self, key: str):
    base_name = key.replace("Volume", "")
    snake_case = "".join(["_" + c.lower() if c.isupper() else c for c in base_name]).lstrip("_")
    stock_path = Path(BASEDIR) / "selfdrive" / "assets" / "sounds" / f"{snake_case}.wav"
    theme_path = ACTIVE_THEME_PATH / "sounds" / f"{snake_case}.wav"

    sound_path = theme_path if theme_path.exists() else stock_path
    if not sound_path.exists():
      return

    volume = self._params.get_int(key, return_default=True, default=100) / 100.0

    try:
      self._sound_player_process.stdin.write(f"{sound_path}|{volume}\n".encode())
      self._sound_player_process.stdin.flush()
    except Exception:
      pass

  def _test_sound(self, key: str):
    base_name = key.replace("Volume", "")
    if ui_state.started:
      camel_case = base_name[0].lower() + base_name[1:]
      self._params_memory.put("TestAlert", camel_case)
    else:
      self._play_sound_offroad(key)

class StarPilotCustomAlertsLayout(StarPilotPanel):
  ALERT_INFO = {
    "GoatScream": {
      "title": tr_noop("Goat Scream"),
      "desc": tr_noop(
        "<b>Play the infamous \"Goat Scream\" when the steering controller reaches its limit.</b> Based on the \"Turn Exceeds Steering Limit\" event."
      ),
    },
    "GreenLightAlert": {
      "title": tr_noop("Green Light Alert"),
      "desc": tr_noop(
        "<b>Play an alert when the model predicts a red light has turned green.</b><br><br><i><b>Disclaimer</b>: openpilot does not explicitly detect traffic lights. This alert is based on end-to-end model predictions from camera input and may trigger even when the light has not changed.</i>"
      ),
    },
    "LeadDepartingAlert": {
      "title": tr_noop("Lead Departing Alert"),
      "desc": tr_noop("<b>Play an alert when the lead vehicle departs from a stop.</b>"),
    },
    "LoudBlindspotAlert": {
      "title": tr_noop("Loud \"Car Detected in Blindspot\" Alert"),
      "desc": tr_noop(
        "<b>Play a louder alert if a vehicle is in the blind spot when attempting to change lanes.</b> Based on the \"Car Detected in Blindspot\" event."
      ),
    },
    "SpeedLimitChangedAlert": {
      "title": tr_noop("Speed Limit Changed Alert"),
      "desc": tr_noop("<b>Play an alert when the posted speed limit changes.</b>"),
    },
  }

  def __init__(self):
    super().__init__()
    self._toggle_items = {}

    for key in StarPilotSoundsLayout.CUSTOM_ALERTS_KEYS:
      info = self.ALERT_INFO[key]
      self._toggle_items[key] = toggle_item(
        info["title"],
        info["desc"],
        self._params.get_bool(key),
        callback=lambda s, k=key: self._params.put_bool(k, s),
      )

    self._scroller = Scroller(list(self._toggle_items.values()), line_separator=True, spacing=0)

  def refresh_visibility(self):
    current_level = int(self._params.get("TuningLevel", return_default=True, default="1") or "1")
    for key, item in self._toggle_items.items():
      min_level = self._tuning_levels.get(key, 0)
      visible = current_level >= min_level

      if key == "LoudBlindspotAlert":
        visible &= starpilot_state.car_state.hasBSM
      elif key == "SpeedLimitChangedAlert":
        visible &= self._params.get_bool("ShowSpeedLimits") or (
          starpilot_state.car_state.hasOpenpilotLongitudinal and self._params.get_bool("SpeedLimitController")
        )

      item.set_visible(visible)
