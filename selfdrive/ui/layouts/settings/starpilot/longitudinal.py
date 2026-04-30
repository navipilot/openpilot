from __future__ import annotations
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog
from openpilot.system.ui.widgets.keyboard import Keyboard
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel, create_master_toggle_panel, create_tile_panel
from openpilot.selfdrive.ui.layouts.settings.starpilot.tabbed_panel import TabSectionSpec, TabbedSectionHost
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import AetherSliderDialog
from openpilot.starpilot.common.accel_profile import (
  ACCELERATION_PROFILES,
  DECELERATION_PROFILES,
  normalize_acceleration_profile,
  normalize_deceleration_profile,
)
from openpilot.starpilot.common.experimental_state import sync_persist_experimental_state


ACCELERATION_PROFILE_OPTIONS = [
  (ACCELERATION_PROFILES["STANDARD"], "Standard"),
  (ACCELERATION_PROFILES["ECO"], "Eco"),
  (ACCELERATION_PROFILES["SPORT"], "Sport"),
  (ACCELERATION_PROFILES["SPORT_PLUS"], "Sport+"),
]

DECELERATION_PROFILE_OPTIONS = [
  (DECELERATION_PROFILES["STANDARD"], "Standard"),
  (DECELERATION_PROFILES["ECO"], "Eco"),
  (DECELERATION_PROFILES["SPORT"], "Sport"),
]


class StarPilotLongitudinalLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    longitudinal_tune_panel = StarPilotLongitudinalTuneLayout()
    advanced_longitudinal_panel = StarPilotAdvancedLongitudinalLayout()
    tune_panel = create_master_toggle_panel([
      {
        "title": tr_noop("Longitudinal Tuning"),
        "desc": tr_noop("Acceleration and braking control changes to fine-tune how openpilot drives."),
        "manage_title": tr_noop("Longitudinal Settings"),
        "manage_desc": tr_noop("Open acceleration profiles, human-like behavior, lead detection, and turn-speed controls."),
        "manage_label": tr_noop("Configure"),
        "disabled_label": tr_noop("Enable First"),
        "panel": "tuning",
        "get_state": lambda: self._params.get_bool("LongitudinalTune"),
        "set_state": lambda s: self._params.put_bool("LongitudinalTune", s),
        "icon": "toggle_icons/icon_longitudinal_tune.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Advanced Longitudinal Tuning"),
        "desc": tr_noop("Advanced acceleration and braking changes for refining launch, stopping, and actuator response."),
        "manage_title": tr_noop("Advanced Settings"),
        "manage_desc": tr_noop("Adjust actuator delay, launch and stop behavior, and powertrain-specific tuning options."),
        "manage_label": tr_noop("Configure"),
        "disabled_label": tr_noop("Enable First"),
        "panel": "advanced",
        "get_state": lambda: self._params.get_bool("AdvancedLongitudinalTune"),
        "set_state": lambda s: self._params.put_bool("AdvancedLongitudinalTune", s),
        "icon": "toggle_icons/icon_advanced_longitudinal_tune.png",
        "color": "#597497",
      },
    ], {
      "tuning": longitudinal_tune_panel,
      "advanced": advanced_longitudinal_panel,
      "personalities": StarPilotPersonalitiesLayout(),
      "traffic_personality": StarPilotPersonalityProfileLayout("Traffic"),
      "aggressive_personality": StarPilotPersonalityProfileLayout("Aggressive"),
      "standard_personality": StarPilotPersonalityProfileLayout("Standard"),
      "relaxed_personality": StarPilotPersonalityProfileLayout("Relaxed"),
    }, extra_categories=[
      {
        "title": tr_noop("Driving Personalities"),
        "desc": tr_noop("Customize the Traffic, Aggressive, Standard, and Relaxed profiles to match your driving style."),
        "panel": "personalities",
        "icon": "toggle_icons/icon_personality.png",
        "color": "#597497",
      },
    ])

    adaptive_panel = create_tile_panel([
      {"title": tr_noop("Conditional Experimental"), "panel": "conditional", "icon": "toggle_icons/icon_conditional.png", "color": "#597497"},
      {"title": tr_noop("Curve Speed"), "panel": "curve", "icon": "toggle_icons/icon_speed_map.png", "color": "#597497"},
      {"title": tr_noop("Weather"), "panel": "weather", "icon": "toggle_icons/icon_rainbow.png", "color": "#597497"},
    ], {
      "conditional": StarPilotConditionalExperimentalLayout(),
      "curve": StarPilotCurveSpeedLayout(),
      "weather": StarPilotWeatherLayout(),
      "low_visibility": StarPilotWeatherBase("LowVisibility"),
      "rain": StarPilotWeatherBase("Rain"),
      "rainstorm": StarPilotWeatherBase("RainStorm"),
      "snow": StarPilotWeatherBase("Snow"),
    })

    self._section_tabs = TabbedSectionHost([
      TabSectionSpec("tune", "Tune", tune_panel),
      TabSectionSpec("adaptive", "Adaptive", adaptive_panel),
      TabSectionSpec("limits", "Limits", StarPilotSpeedLimitControllerLayout()),
      TabSectionSpec("daily", "Daily", StarPilotLongitudinalQOLLayout()),
    ])
    self._rebuild_grid()

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


class StarPilotAdvancedLongitudinalLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("EV Tuning"),
        "desc": tr_noop("Use acceleration tuning intended for EV and direct-drive vehicles."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("EVTuning"),
        "set_state": self._set_ev_tuning,
        "color": "#597497",
        "is_enabled": lambda: not self._params.get_bool("TruckTuning"),
        "disabled_label": tr_noop("Truck Active"),
        "visible": self._advanced_enabled,
      },
      {
        "title": tr_noop("Truck Tuning"),
        "desc": tr_noop("Use stronger launch and acceleration behavior intended for heavier vehicles."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("TruckTuning"),
        "set_state": self._set_truck_tuning,
        "color": "#597497",
        "is_enabled": lambda: not self._params.get_bool("EVTuning"),
        "disabled_label": tr_noop("EV Active"),
        "visible": self._advanced_enabled,
      },
      {
        "title": tr_noop("Actuator Delay"),
        "desc": tr_noop("The time between an acceleration or brake command and the vehicle's response."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('LongitudinalActuatorDelay'):.2f}s",
        "on_click": lambda: self._show_float_selector("LongitudinalActuatorDelay", 0.0, 1.0, 0.01, "s"),
        "color": "#597497",
        "visible": self._advanced_enabled,
      },
      {
        "title": tr_noop("Maximum Acceleration"),
        "desc": tr_noop("Limit the strongest acceleration openpilot is allowed to command."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('MaxDesiredAcceleration'):.1f}m/s²",
        "on_click": lambda: self._show_float_selector("MaxDesiredAcceleration", 0.1, 4.0, 0.1, "m/s²"),
        "color": "#597497",
        "visible": self._advanced_enabled,
      },
      {
        "title": tr_noop("Start Acceleration"),
        "desc": tr_noop("Extra acceleration applied when moving away from a stop."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('StartAccel'):.2f}m/s²",
        "on_click": lambda: self._show_float_selector("StartAccel", 0.0, 4.0, 0.01, "m/s²"),
        "color": "#597497",
        "visible": lambda: self._advanced_enabled() and not self._using_human_acceleration(),
      },
      {
        "title": tr_noop("Stop Acceleration"),
        "desc": tr_noop("Brake force used to hold the vehicle at a complete stop."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('StopAccel'):.2f}m/s²",
        "on_click": lambda: self._show_float_selector("StopAccel", -4.0, 0.0, 0.01, "m/s²"),
        "color": "#597497",
        "visible": self._advanced_enabled,
      },
      {
        "title": tr_noop("Stopping Rate"),
        "desc": tr_noop("How quickly braking ramps up when openpilot is bringing the car to a stop."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('StoppingDecelRate'):.3f}m/s²",
        "on_click": lambda: self._show_float_selector("StoppingDecelRate", 0.001, 1.0, 0.001, "m/s²"),
        "color": "#597497",
        "visible": self._show_stop_tuning_values,
      },
      {
        "title": tr_noop("Start Speed"),
        "desc": tr_noop("The speed where openpilot transitions out of the stopped state."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('VEgoStarting'):.2f}m/s",
        "on_click": lambda: self._show_float_selector("VEgoStarting", 0.01, 1.0, 0.01, "m/s"),
        "color": "#597497",
        "visible": self._show_stop_tuning_values,
      },
      {
        "title": tr_noop("Stop Speed"),
        "desc": tr_noop("The speed where openpilot considers the vehicle fully stopped."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('VEgoStopping'):.2f}m/s",
        "on_click": lambda: self._show_float_selector("VEgoStopping", 0.01, 1.0, 0.01, "m/s"),
        "color": "#597497",
        "visible": self._show_stop_tuning_values,
      },
    ]
    self._rebuild_grid()

  def _advanced_enabled(self):
    return self._params.get_bool("AdvancedLongitudinalTune")

  def _using_human_acceleration(self):
    return self._params.get_bool("LongitudinalTune") and self._params.get_bool("HumanAcceleration")

  def _show_stop_tuning_values(self):
    return self._advanced_enabled() and not (starpilot_state.car_state.isToyota and self._params.get_bool("FrogsGoMoosTweak"))

  def _set_ev_tuning(self, state: bool):
    self._params.put_bool("EVTuning", state)
    if state:
      self._params.put_bool("TruckTuning", False)

  def _set_truck_tuning(self, state: bool):
    self._params.put_bool("TruckTuning", state)
    if state:
      self._params.put_bool("EVTuning", False)

  def _show_float_selector(self, key, min_v, max_v, step, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_float(key, float(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, step, self._params.get_float(key), on_close, unit=unit, color="#597497"))


class StarPilotConditionalExperimentalLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Conditional Experimental Mode"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ConditionalExperimental"),
        "set_state": lambda s: self._params.put_bool("ConditionalExperimental", s),
        "icon": "toggle_icons/icon_conditional.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Persist Experimental State"),
        "desc": tr_noop("Keep your manual Conditional Experimental override through reboots until you manually clear it."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("PersistExperimentalState"),
        "set_state": self._set_persist_experimental_state,
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Below Speed"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('CESpeed')} mph",
        "on_click": lambda: self._show_speed_selector("CESpeed"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Curves"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CECurves"),
        "set_state": lambda s: self._params.put_bool("CECurves", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Curves Lead"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CECurvesLead"),
        "set_state": lambda s: self._params.put_bool("CECurvesLead", s),
        "visible": lambda: self._params.get_bool("CECurves"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Stop Lights"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CEStopLights"),
        "set_state": lambda s: self._params.put_bool("CEStopLights", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Lead Detected"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CELead"),
        "set_state": lambda s: self._params.put_bool("CELead", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Slower Lead"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CESlowerLead"),
        "set_state": lambda s: self._params.put_bool("CESlowerLead", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental") and self._params.get_bool("CELead"),
      },
      {
        "title": tr_noop("Stopped Lead"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CEStoppedLead"),
        "set_state": lambda s: self._params.put_bool("CEStoppedLead", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental") and self._params.get_bool("CELead"),
      },
      {
        "title": tr_noop("Predicted Stop"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('CEModelStopTime')}s",
        "on_click": lambda: self._show_int_selector("CEModelStopTime", 0, 10, "s"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Signal Below"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('CESignalSpeed')} mph",
        "on_click": lambda: self._show_speed_selector("CESignalSpeed"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Speed Lead"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('CESpeedLead')} mph",
        "on_click": lambda: self._show_speed_selector("CESpeedLead"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
      {
        "title": tr_noop("Signal Lane Detection"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CESignalLaneDetection"),
        "set_state": lambda s: self._params.put_bool("CESignalLaneDetection", s),
        "visible": lambda: self._params.get_int("CESignalSpeed") > 0,
        "color": "#597497",
      },
      {
        "title": tr_noop("Status Widget"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ShowCEMStatus"),
        "set_state": lambda s: self._params.put_bool("ShowCEMStatus", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("ConditionalExperimental"),
      },
    ]
    self._rebuild_grid()

  def _set_persist_experimental_state(self, state: bool):
    sync_persist_experimental_state(self._params, self._params_memory, state)

  def _show_speed_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), 0, 100, 1, self._params.get_int(key), on_close, unit=" mph", color="#597497"))

  def _show_int_selector(self, key, min_v, max_v, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, 1, self._params.get_int(key), on_close, unit=unit, color="#597497"))


class StarPilotCurveSpeedLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Curve Speed Controller"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CurveSpeedController"),
        "set_state": lambda s: self._params.put_bool("CurveSpeedController", s),
        "icon": "toggle_icons/icon_speed_map.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Status Widget"),
        "desc": tr_noop("Show the Curve Speed Controller ambient effect on the driving screen."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ShowCSCStatus"),
        "set_state": lambda s: self._params.put_bool("ShowCSCStatus", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("CurveSpeedController"),
      },
      {
        "title": tr_noop("Calibrated Lateral Accel"),
        "desc": tr_noop("The learned lateral acceleration from collected driving data. Higher values allow faster cornering."),
        "type": "value",
        "get_value": lambda: f"{self._params_memory.get_float('CalibratedLateralAcceleration'):.2f} m/s²",
        "on_click": lambda: None,
        "color": "#597497",
        "visible": lambda: self._params.get_bool("CurveSpeedController"),
      },
      {
        "title": tr_noop("Calibration Progress"),
        "desc": tr_noop("How much curve data has been collected. Normal for the value to stay low."),
        "type": "value",
        "get_value": lambda: f"{self._params_memory.get_float('CalibrationProgress'):.2f}%",
        "on_click": lambda: None,
        "color": "#597497",
        "visible": lambda: self._params.get_bool("CurveSpeedController"),
      },
      {
        "title": tr_noop("Reset Curve Data"),
        "desc": tr_noop("Reset collected user data for Curve Speed Controller."),
        "type": "hub",
        "on_click": lambda: self._reset_curve_data(),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("CurveSpeedController"),
      },
    ]
    self._rebuild_grid()

  def _reset_curve_data(self):
    def on_close(res):
      if res == DialogResult.CONFIRM:
        self._params.put_float("CalibratedLateralAcceleration", 2.00)
        self._params.remove("CalibrationProgress")
        self._params.remove("CurvatureData")
        self._rebuild_grid()

    gui_app.push_widget(ConfirmDialog(tr("Reset Curve Data?"), tr("Confirm"), callback=on_close))


class StarPilotPersonalitiesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Driving Personalities"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CustomPersonalities"),
        "set_state": lambda s: self._params.put_bool("CustomPersonalities", s),
        "icon": "toggle_icons/icon_personality.png",
        "color": "#597497",
      },
      {"title": tr_noop("Traffic"), "panel": "traffic_personality", "icon": "toggle_icons/icon_personality.png", "color": "#597497"},
      {"title": tr_noop("Aggressive"), "panel": "aggressive_personality", "icon": "toggle_icons/icon_personality.png", "color": "#597497"},
      {"title": tr_noop("Standard"), "panel": "standard_personality", "icon": "toggle_icons/icon_personality.png", "color": "#597497"},
      {"title": tr_noop("Relaxed"), "panel": "relaxed_personality", "icon": "toggle_icons/icon_personality.png", "color": "#597497"},
    ]
    self._rebuild_grid()


class StarPilotPersonalityProfileLayout(StarPilotPanel):
  def __init__(self, profile: str):
    super().__init__()
    self._profile = profile
    follow_min = 1.0 if profile == "Traffic" else 0.5
    follow_max = 2.5 if profile == "Traffic" else 3.0
    self.CATEGORIES = [
      {
        "title": tr_noop("Follow Distance"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float(self._profile + 'Follow'):.2f}s",
        "on_click": lambda: self._show_float_selector(self._profile + "Follow", follow_min, follow_max, 0.05, "s"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Follow High"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float(self._profile + 'FollowHigh'):.2f}s",
        "on_click": lambda: self._show_float_selector(self._profile + "FollowHigh", 1.0, 3.0, 0.05, "s"),
        "visible": lambda: self._profile != "Traffic",
        "color": "#597497",
      },
      {
        "title": tr_noop("Accel Smoothness"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int(self._profile + 'JerkAcceleration')}%",
        "on_click": lambda: self._show_int_selector(self._profile + "JerkAcceleration", 25, 200, "%"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Brake Smoothness"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int(self._profile + 'JerkDeceleration')}%",
        "on_click": lambda: self._show_int_selector(self._profile + "JerkDeceleration", 25, 200, "%"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Safety Gap Bias"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int(self._profile + 'JerkDanger')}%",
        "on_click": lambda: self._show_int_selector(self._profile + "JerkDanger", 25, 200, "%"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Slowdown Response"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int(self._profile + 'JerkSpeedDecrease')}%",
        "on_click": lambda: self._show_int_selector(self._profile + "JerkSpeedDecrease", 25, 200, "%"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Speed-Up Response"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int(self._profile + 'JerkSpeed')}%",
        "on_click": lambda: self._show_int_selector(self._profile + "JerkSpeed", 25, 200, "%"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Reset to Defaults"),
        "type": "hub",
        "on_click": lambda: self._reset_profile(),
        "color": "#597497",
      },
    ]
    self._rebuild_grid()

  def _reset_profile(self):
    def on_close(res):
      if res == DialogResult.CONFIRM:
        for key in ["Follow", "FollowHigh", "JerkAcceleration", "JerkDeceleration", "JerkDanger", "JerkSpeedDecrease", "JerkSpeed"]:
          self._params.remove(self._profile + key)
        self._rebuild_grid()

    gui_app.push_widget(ConfirmDialog(tr("Reset to Defaults?"), tr("Confirm"), callback=on_close))

  def _show_float_selector(self, key, min_v, max_v, step, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_float(key, float(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, step, self._params.get_float(key), on_close, unit=unit, color="#597497"))

  def _show_int_selector(self, key, min_v, max_v, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, 5, self._params.get_int(key), on_close, unit=unit, color="#597497"))


class StarPilotLongitudinalTuneLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Acceleration Profile"),
        "desc": tr_noop("Choose how quickly openpilot speeds up."),
        "type": "value",
        "get_value": self._get_acceleration_profile_label,
        "on_click": self._show_acceleration_profile_selector,
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
      {
        "title": tr_noop("Deceleration Profile"),
        "desc": tr_noop("Choose how firmly openpilot slows the car down."),
        "type": "value",
        "get_value": self._get_deceleration_profile_label,
        "on_click": self._show_deceleration_profile_selector,
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
      {
        "title": tr_noop("Human-Like Acceleration"),
        "desc": tr_noop("Smooth throttle behavior at low speed with stronger takeoff from a stop."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("HumanAcceleration"),
        "set_state": lambda s: self._params.put_bool("HumanAcceleration", s),
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
      {
        "title": tr_noop("Human-Like Following"),
        "desc": tr_noop("Adjust following behavior to feel more natural behind other vehicles."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("HumanFollowing"),
        "set_state": lambda s: self._params.put_bool("HumanFollowing", s),
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
      {
        "title": tr_noop("Coast Up To Leads"),
        "desc": tr_noop("Allow openpilot to briefly coast toward far leads before applying normal throttle again."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CoastUpToLeads"),
        "set_state": lambda s: self._params.put_bool("CoastUpToLeads", s),
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
      {
        "title": tr_noop("Human-Like Lane Changes"),
        "desc": tr_noop("Use radar-informed behavior during lane changes when radar support is available."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("HumanLaneChanges"),
        "set_state": lambda s: self._params.put_bool("HumanLaneChanges", s),
        "color": "#597497",
        "visible": lambda: self._longitudinal_enabled() and starpilot_state.car_state.hasRadar,
      },
      {
        "title": tr_noop("Lead Detection Sensitivity"),
        "desc": tr_noop("Control how aggressively openpilot detects and reacts to vehicles ahead."),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('LeadDetectionThreshold')}%",
        "on_click": lambda: self._show_int_selector("LeadDetectionThreshold", 25, 50, "%"),
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
      {
        "title": tr_noop("\"Taco Bell Run\" Turn Speed Hack"),
        "desc": tr_noop("Slow down more assertively for turns."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("TacoTune"),
        "set_state": lambda s: self._params.put_bool("TacoTune", s),
        "color": "#597497",
        "visible": self._longitudinal_enabled,
      },
    ]
    self._rebuild_grid()

  def _longitudinal_enabled(self):
    return self._params.get_bool("LongitudinalTune")

  def _get_acceleration_profile_label(self):
    value = normalize_acceleration_profile(self._params.get("AccelerationProfile", encoding='utf-8'))
    return self._profile_label_for_value(value, ACCELERATION_PROFILE_OPTIONS)

  def _get_deceleration_profile_label(self):
    value = normalize_deceleration_profile(self._params.get("DecelerationProfile", encoding='utf-8'))
    return self._profile_label_for_value(value, DECELERATION_PROFILE_OPTIONS)

  def _show_acceleration_profile_selector(self):
    self._show_selection("Acceleration Profile", "AccelerationProfile", ACCELERATION_PROFILE_OPTIONS, normalize_acceleration_profile(self._params.get("AccelerationProfile", encoding='utf-8')))

  def _show_deceleration_profile_selector(self):
    self._show_selection("Deceleration Profile", "DecelerationProfile", DECELERATION_PROFILE_OPTIONS, normalize_deceleration_profile(self._params.get("DecelerationProfile", encoding='utf-8')))

  def _profile_label_for_value(self, value, options):
    for option_value, option_label in options:
      if option_value == value:
        return tr(option_label)
    return tr(options[0][1])

  def _show_selection(self, title, key, options, current_value):
    option_labels = [tr(option_label) for _, option_label in options]
    label_to_value = {tr(option_label): option_value for option_value, option_label in options}
    default_option = next((tr(option_label) for option_value, option_label in options if option_value == current_value), option_labels[0])

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put_int(key, label_to_value[dialog.selection])
        self._rebuild_grid()

    dialog = MultiOptionDialog(tr(title), option_labels, default_option, callback=on_select)
    gui_app.push_widget(dialog)

  def _show_int_selector(self, key, min_v, max_v, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, 1, self._params.get_int(key), on_close, unit=unit, color="#597497"))


class StarPilotLongitudinalQOLLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Quality of Life"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("QOLLongitudinal"),
        "set_state": lambda s: self._params.put_bool("QOLLongitudinal", s),
        "icon": "toggle_icons/icon_quality_of_life.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Cruise Interval"),
        "type": "value",
        "get_value": lambda: f"{max(1, self._params.get_int('CustomCruise'))} mph",
        "on_click": lambda: self._show_speed_selector("CustomCruise"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Cruise Long"),
        "type": "value",
        "get_value": lambda: f"{max(1, self._params.get_int('CustomCruiseLong'))} mph",
        "on_click": lambda: self._show_speed_selector("CustomCruiseLong"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Reverse Cruise"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ReverseCruise"),
        "set_state": lambda s: self._params.put_bool("ReverseCruise", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Force Stops"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ForceStops"),
        "set_state": lambda s: self._params.put_bool("ForceStops", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Force Stop Distance Offset"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('ForceStopDistanceOffset'):+d} ft",
        "on_click": lambda: self._show_int_selector("ForceStopDistanceOffset", -20, 20, " ft"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal") and self._params.get_bool("ForceStops"),
      },
      {
        "title": tr_noop("Force Standstill State"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ForceStandstill"),
        "set_state": lambda s: self._params.put_bool("ForceStandstill", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Stopped Distance"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('IncreasedStoppedDistance')} ft",
        "on_click": lambda: self._show_int_selector("IncreasedStoppedDistance", 0, 10, " ft"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Set Speed Offset"),
        "type": "value",
        "get_value": lambda: f"+{self._params.get_int('SetSpeedOffset')} mph",
        "on_click": lambda: self._show_int_selector("SetSpeedOffset", 0, 99, " mph"),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Map Gears"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("MapGears"),
        "set_state": lambda s: self._params.put_bool("MapGears", s),
        "color": "#597497",
        "visible": lambda: self._params.get_bool("QOLLongitudinal"),
      },
      {
        "title": tr_noop("Map Acceleration"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("MapAcceleration"),
        "set_state": lambda s: self._params.put_bool("MapAcceleration", s),
        "visible": lambda: self._params.get_bool("MapGears"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Map Deceleration"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("MapDeceleration"),
        "set_state": lambda s: self._params.put_bool("MapDeceleration", s),
        "visible": lambda: self._params.get_bool("MapGears"),
        "color": "#597497",
      },
    ]
    self._rebuild_grid()

  def _show_speed_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    current = max(1, self._params.get_int(key))
    gui_app.push_widget(AetherSliderDialog(tr(key), 1, 100, 1, current, on_close, unit=" mph", color="#597497"))

  def _show_int_selector(self, key, min_v, max_v, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, 1, self._params.get_int(key), on_close, unit=unit, color="#597497"))


class StarPilotSpeedLimitControllerLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._sub_panels = {
      "slc_offsets": StarPilotSLCOffsetsLayout(),
      "slc_qol": StarPilotSLCQOLLayout(),
      "slc_visuals": StarPilotSLCVisualsLayout(),
    }
    self.CATEGORIES = [
      {
        "title": tr_noop("Speed Limit Controller"),
        "desc": tr_noop("Limit the car's maximum speed to the current speed limit."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SpeedLimitController"),
        "set_state": lambda s: self._params.put_bool("SpeedLimitController", s),
        "icon": "toggle_icons/icon_speed_limit.png",
        "color": "#597497",
      },
      {"title": tr_noop("SLC Offsets"), "panel": "slc_offsets", "icon": "toggle_icons/icon_speed_limit.png", "color": "#597497"},
      {"title": tr_noop("SLC Quality of Life"), "panel": "slc_qol", "icon": "toggle_icons/icon_speed_limit.png", "color": "#597497"},
      {"title": tr_noop("SLC Visuals"), "panel": "slc_visuals", "icon": "toggle_icons/icon_speed_limit.png", "color": "#597497"},
      {
        "title": tr_noop("Fallback Speed"),
        "type": "value",
        "get_value": lambda: self._params.get("SLCFallback", encoding='utf-8') or "Set Speed",
        "on_click": lambda: self._show_selection("SLCFallback", ["Set Speed", "Experimental Mode", "Previous Limit"]),
        "color": "#597497",
      },
      {
        "title": tr_noop("Override Speed"),
        "type": "value",
        "get_value": lambda: self._params.get("SLCOverride", encoding='utf-8') or "None",
        "on_click": lambda: self._show_selection("SLCOverride", ["None", "Set With Gas Pedal", "Max Set Speed"]),
        "color": "#597497",
      },
      {
        "title": tr_noop("Source Priority"),
        "type": "value",
        "get_value": self._get_priority_value,
        "on_click": self._on_priority_clicked,
        "color": "#597497",
      },
    ]

    for panel in self._sub_panels.values():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

    self._rebuild_grid()

  def _get_priority_value(self):
    primary = self._params.get("SLCPriority1", encoding='utf-8') or "Map Data"
    secondary = self._params.get("SLCPriority2", encoding='utf-8') or "None"
    if primary in ("Highest", "Lowest") or secondary in ("", "None", primary):
      return primary
    return f"{primary}, {secondary}"

  def _on_priority_clicked(self):
    primary_options = ["Dashboard", "Map Data", "Vision", "Highest", "Lowest"]
    current_primary = self._params.get("SLCPriority1", encoding='utf-8') or "Map Data"
    current_secondary = self._params.get("SLCPriority2", encoding='utf-8') or "None"

    def on_secondary_select(primary, dialog, res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put("SLCPriority1", primary)
        self._params.put("SLCPriority2", dialog.selection)
        self._rebuild_grid()

    def show_secondary_dialog(primary):
      secondary_options = ["None"] + [option for option in ("Dashboard", "Map Data", "Vision") if option != primary]
      selected_secondary = current_secondary if current_secondary in secondary_options else "None"
      secondary_dialog = MultiOptionDialog(tr("SLC Secondary Priority"), secondary_options, selected_secondary, callback=lambda res: on_secondary_select(primary, secondary_dialog, res))
      gui_app.push_widget(secondary_dialog)

    def on_primary_select(res):
      if res != DialogResult.CONFIRM or not primary_dialog.selection:
        return
      if primary_dialog.selection in ("Highest", "Lowest"):
        self._params.put("SLCPriority1", primary_dialog.selection)
        self._params.put("SLCPriority2", "None")
        self._rebuild_grid()
        return
      show_secondary_dialog(primary_dialog.selection)

    primary_dialog = MultiOptionDialog(tr("SLC Primary Priority"), primary_options, current_primary, callback=on_primary_select)
    gui_app.push_widget(primary_dialog)

  def _show_selection(self, key, options):
    current = self._params.get(key, encoding='utf-8') or "None"

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put(key, dialog.selection)
        self._rebuild_grid()

    dialog = MultiOptionDialog(tr(key), options, current, callback=on_select)
    gui_app.push_widget(dialog)


class StarPilotSLCOffsetsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = []
    for i in range(1, 8):
      key = f"Offset{i}"
      self.CATEGORIES.append(
        {
          "title": tr_noop(f"Offset {i}"),
          "type": "value",
          "get_value": lambda k=key: f"{self._params.get_int(k)}{self._speed_unit()}",
          "on_click": lambda k=key: self._show_speed_selector(k),
          "color": "#597497",
        }
      )
    self._rebuild_grid()

  def _is_metric(self):
    return self._params.get_bool("IsMetric")

  def _speed_unit(self):
    return " km/h" if self._is_metric() else " mph"

  def _speed_range(self):
    return (-150, 150) if self._is_metric() else (-99, 99)

  def _show_speed_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    min_value, max_value = self._speed_range()
    gui_app.push_widget(
      AetherSliderDialog(tr(key), min_value, max_value, 1, self._params.get_int(key), on_close, unit=self._speed_unit(), color="#597497")
    )


class StarPilotSLCQOLLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Auto Match Speed Limits"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SetSpeedLimit"),
        "set_state": lambda s: self._params.put_bool("SetSpeedLimit", s),
        "color": "#597497",
      },
      {
        "title": tr_noop("Confirm New Limits"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SLCConfirmation"),
        "set_state": lambda s: self._params.put_bool("SLCConfirmation", s),
        "color": "#597497",
      },
      {
        "title": tr_noop("Confirm Lower"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SLCConfirmationLower"),
        "set_state": lambda s: self._params.put_bool("SLCConfirmationLower", s),
        "visible": lambda: self._params.get_bool("SLCConfirmation"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Confirm Higher"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SLCConfirmationHigher"),
        "set_state": lambda s: self._params.put_bool("SLCConfirmationHigher", s),
        "visible": lambda: self._params.get_bool("SLCConfirmation"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Higher Lookahead"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('SLCLookaheadHigher')}s",
        "on_click": lambda: self._show_int_selector("SLCLookaheadHigher", 0, 30, "s"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Lower Lookahead"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('SLCLookaheadLower')}s",
        "on_click": lambda: self._show_int_selector("SLCLookaheadLower", 0, 30, "s"),
        "color": "#597497",
      },
      {
        "title": tr_noop("Mapbox Fallback"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SLCMapboxFiller"),
        "set_state": lambda s: self._params.put_bool("SLCMapboxFiller", s),
        "color": "#597497",
      },
    ]
    self._rebuild_grid()

  def _show_int_selector(self, key, min_v, max_v, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()

    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, 1, self._params.get_int(key), on_close, unit=unit, color="#597497"))


class StarPilotSLCVisualsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Show SLC Offset"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ShowSLCOffset"),
        "set_state": lambda s: self._params.put_bool("ShowSLCOffset", s),
        "color": "#597497",
      },
      {
        "title": tr_noop("Show Sources"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SpeedLimitSources"),
        "set_state": lambda s: self._params.put_bool("SpeedLimitSources", s),
        "color": "#597497",
      },
    ]
    self._rebuild_grid()


class StarPilotWeatherLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._keyboard = Keyboard(min_text_size=1)
    self.CATEGORIES = [
      {"title": tr_noop("Low Visibility"), "panel": "low_visibility", "icon": "toggle_icons/icon_rainbow.png", "color": "#597497"},
      {"title": tr_noop("Rain"), "panel": "rain", "icon": "toggle_icons/icon_rainbow.png", "color": "#597497"},
      {"title": tr_noop("Rainstorms"), "panel": "rainstorm", "icon": "toggle_icons/icon_rainbow.png", "color": "#597497"},
      {"title": tr_noop("Snow"), "panel": "snow", "icon": "toggle_icons/icon_rainbow.png", "color": "#597497"},
      {
        "title": tr_noop("Set Weather Key"),
        "type": "hub",
        "on_click": lambda: self._set_weather_key(),
        "color": "#597497",
      },
    ]
    self._rebuild_grid()

  def _set_weather_key(self):
    options = ["ADD", "REMOVE"]

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        if dialog.selection == "ADD":

          def on_key(res, text):
            if res == DialogResult.CONFIRM:
              self._params.put("WeatherAPIKey", text)
              self._rebuild_grid()

          self._keyboard.reset(min_text_size=1)
          self._keyboard.set_title(tr("Weather API Key"), "")
          self._keyboard.set_text("")
          self._keyboard.set_callback(lambda result: on_key(result, self._keyboard.text))
          gui_app.push_widget(self._keyboard)
        elif dialog.selection == "REMOVE":

          def on_confirm(res):
            if res == DialogResult.CONFIRM:
              self._params.remove("WeatherAPIKey")
              self._rebuild_grid()

          gui_app.push_widget(ConfirmDialog(tr("Remove API Key?"), tr("Confirm"), callback=on_confirm))

    dialog = MultiOptionDialog(tr("Weather API Key"), options, "ADD", callback=on_select)
    gui_app.push_widget(dialog)


class StarPilotWeatherBase(StarPilotPanel):
  def __init__(self, suffix: str):
    super().__init__()
    self._suffix = suffix
    self.CATEGORIES = [
      {
        "title": tr_noop("Following Distance"),
        "type": "value",
        "get_value": lambda: f"+{self._params.get_int('IncreaseFollowing' + self._suffix)}s",
        "on_click": lambda: self._show_value_selector("IncreaseFollowing" + self._suffix, 0, 3, 0.5, "s"),
        "icon": "toggle_icons/icon_longitudinal_tune.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Stopped Distance"),
        "type": "value",
        "get_value": lambda: f"+{self._params.get_int('IncreasedStoppedDistance' + self._suffix)} ft",
        "on_click": lambda: self._show_value_selector("IncreasedStoppedDistance" + self._suffix, 0, 10, 1, " ft"),
        "icon": "toggle_icons/icon_longitudinal_tune.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Reduce Accel"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('ReduceAcceleration' + self._suffix)}%",
        "on_click": lambda: self._show_value_selector("ReduceAcceleration" + self._suffix, 0, 99, 1, "%"),
        "icon": "toggle_icons/icon_longitudinal_tune.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Reduce Curve Speed"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_int('ReduceLateralAcceleration' + self._suffix)}%",
        "on_click": lambda: self._show_value_selector("ReduceLateralAcceleration" + self._suffix, 0, 99, 1, "%"),
        "icon": "toggle_icons/icon_longitudinal_tune.png",
        "color": "#597497",
      },
    ]
    self._rebuild_grid()

  def _show_value_selector(self, key, min_v, max_v, step, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(float(val)))
        self._rebuild_grid()

    curr = self._params.get_int(key)
    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, step, curr, on_close, unit=unit, color="#597497"))
