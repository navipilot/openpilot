from __future__ import annotations
from openpilot.system.hardware import HARDWARE
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel, create_master_toggle_panel, create_tile_panel
from openpilot.selfdrive.ui.layouts.settings.starpilot.tabbed_panel import TabSectionSpec, TabbedSectionHost
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import AetherSliderDialog

class StarPilotAdvancedLateralLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Actuator Delay"), "desc": tr_noop("The time between openpilot's steering command and the vehicle's response."), "type": "value", "get_value": lambda: f"{self._params.get_float('SteerDelay'):.2f}s", "on_click": lambda: self._show_float_selector("SteerDelay", 0.01, 1.0, 0.01, "s"), "icon": "toggle_icons/icon_advanced_lateral_tune.png", "color": "#597497", "visible": self._show_steer_delay},
      {"title": tr_noop("Friction"), "desc": tr_noop("Compensates for steering friction around center."), "type": "value", "get_value": lambda: f"{self._params.get_float('SteerFriction'):.3f}", "on_click": self._show_steer_friction_selector, "icon": "toggle_icons/icon_advanced_lateral_tune.png", "color": "#597497", "visible": self._show_steer_friction},
      {"title": tr_noop("Kp Factor"), "desc": tr_noop("How strongly openpilot corrects lateral position."), "type": "value", "get_value": lambda: f"{self._params.get_float('SteerKP'):.2f}", "on_click": self._show_steer_kp_selector, "icon": "toggle_icons/icon_advanced_lateral_tune.png", "color": "#597497", "visible": self._show_steer_kp},
      {"title": tr_noop("Lateral Acceleration"), "desc": tr_noop("Maps steering torque to turning response."), "type": "value", "get_value": lambda: f"{self._params.get_float('SteerLatAccel'):.2f}", "on_click": self._show_steer_lat_accel_selector, "icon": "toggle_icons/icon_advanced_lateral_tune.png", "color": "#597497", "visible": self._show_steer_lat_accel},
      {"title": tr_noop("Steer Ratio"), "desc": tr_noop("Adjust the relationship between steering wheel input and road-wheel angle."), "type": "value", "get_value": lambda: f"{self._params.get_float('SteerRatio'):.2f}", "on_click": self._show_steer_ratio_selector, "icon": "toggle_icons/icon_advanced_lateral_tune.png", "color": "#597497", "visible": self._show_steer_ratio},
      {"title": tr_noop("Force Auto-Tune On"), "desc": tr_noop("Force-enable live auto-tuning for friction and lateral acceleration."), "type": "toggle", "get_state": lambda: self._params.get_bool("ForceAutoTune"), "set_state": self._set_force_auto_tune, "icon": "toggle_icons/icon_tuning.png", "color": "#597497", "visible": self._show_force_auto_tune},
      {"title": tr_noop("Force Auto-Tune Off"), "desc": tr_noop("Force-disable live auto-tuning and use your set values instead."), "type": "toggle", "get_state": lambda: self._params.get_bool("ForceAutoTuneOff"), "set_state": self._set_force_auto_tune_off, "icon": "toggle_icons/icon_tuning.png", "color": "#597497", "visible": self._show_force_auto_tune_off},
      {"title": tr_noop("Force Torque Controller"), "desc": tr_noop("Use torque-based steering control instead of the stock steering mode when supported."), "type": "toggle", "get_state": lambda: self._params.get_bool("ForceTorqueController"), "set_state": lambda x: self._on_reboot_toggle("ForceTorqueController", x), "icon": "toggle_icons/icon_advanced_lateral_tune.png", "color": "#597497", "visible": self._show_force_torque_controller},
    ]
    self._rebuild_grid()

  def _advanced_enabled(self):
    return self._params.get_bool("AdvancedLateralTune")

  def _using_nnff(self):
    return starpilot_state.car_state.hasNNFFLog and self._params.get_bool("LateralTune") and self._params.get_bool("NNFF")

  def _forcing_auto_tune(self):
    return not starpilot_state.car_state.hasAutoTune and self._params.get_bool("ForceAutoTune")

  def _forcing_auto_tune_off(self):
    return starpilot_state.car_state.hasAutoTune and self._params.get_bool("ForceAutoTuneOff")

  def _forcing_torque_controller(self):
    return not starpilot_state.car_state.isAngleCar and self._params.get_bool("ForceTorqueController")

  def _torque_tuning_active(self):
    return starpilot_state.car_state.isTorqueCar or self._forcing_torque_controller() or self._using_nnff()

  def _manual_tuning_values_enabled(self):
    if starpilot_state.car_state.hasAutoTune:
      return self._forcing_auto_tune_off()
    return not self._forcing_auto_tune()

  def _show_steer_delay(self):
    return self._advanced_enabled() and starpilot_state.car_state.steerActuatorDelay != 0

  def _show_steer_friction(self):
    return (self._advanced_enabled() and starpilot_state.car_state.friction != 0 and self._torque_tuning_active()
            and not self._using_nnff() and self._manual_tuning_values_enabled())

  def _show_steer_kp(self):
    return (self._advanced_enabled() and starpilot_state.car_state.steerKp != 0 and self._torque_tuning_active()
            and not starpilot_state.car_state.isAngleCar)

  def _show_steer_lat_accel(self):
    return (self._advanced_enabled() and starpilot_state.car_state.latAccelFactor != 0 and self._torque_tuning_active()
            and not self._using_nnff() and self._manual_tuning_values_enabled())

  def _show_steer_ratio(self):
    return self._advanced_enabled() and starpilot_state.car_state.steerRatio != 0 and self._manual_tuning_values_enabled()

  def _show_force_auto_tune(self):
    return (self._advanced_enabled() and not starpilot_state.car_state.hasAutoTune and not starpilot_state.car_state.isAngleCar
            and self._torque_tuning_active())

  def _show_force_auto_tune_off(self):
    return self._advanced_enabled() and starpilot_state.car_state.hasAutoTune and not starpilot_state.car_state.isAngleCar

  def _show_force_torque_controller(self):
    return self._advanced_enabled() and not starpilot_state.car_state.isAngleCar and not starpilot_state.car_state.isTorqueCar

  def _set_force_auto_tune(self, state):
    self._params.put_bool("ForceAutoTune", state)
    if state:
      self._params.put_bool("ForceAutoTuneOff", False)

  def _set_force_auto_tune_off(self, state):
    self._params.put_bool("ForceAutoTuneOff", state)
    if state:
      self._params.put_bool("ForceAutoTune", False)

  def _show_steer_friction_selector(self):
    self._show_float_selector("SteerFriction", 0.0, max(1.0, starpilot_state.car_state.friction * 1.5), 0.01)

  def _show_steer_kp_selector(self):
    base = max(0.01, starpilot_state.car_state.steerKp)
    self._show_float_selector("SteerKP", base * 0.5, base * 1.5, 0.01)

  def _show_steer_lat_accel_selector(self):
    base = max(0.01, starpilot_state.car_state.latAccelFactor)
    self._show_float_selector("SteerLatAccel", base * 0.5, base * 1.5, 0.01)

  def _show_steer_ratio_selector(self):
    base = max(0.01, starpilot_state.car_state.steerRatio)
    self._show_float_selector("SteerRatio", base * 0.5, base * 1.5, 0.01)

  def _show_float_selector(self, key, min_v, max_v, step, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_float(key, float(val))
        self._rebuild_grid()
    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, step, self._params.get_float(key), on_close, unit=unit, color="#597497"))

  def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state
    if ui_state.started:
      dialog = ConfirmDialog(tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), callback=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None)
      gui_app.push_widget(dialog)

class StarPilotAlwaysOnLateralLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Always On Lateral"), "type": "toggle", "get_state": lambda: self._params.get_bool("AlwaysOnLateral"), "set_state": lambda x: self._on_reboot_toggle("AlwaysOnLateral", x), "icon": "toggle_icons/icon_always_on_lateral.png", "color": "#597497"},
      {"title": tr_noop("Enable With LKAS"), "type": "toggle", "get_state": lambda: self._params.get_bool("AlwaysOnLateralLKAS"), "set_state": lambda x: self._params.put_bool("AlwaysOnLateralLKAS", x), "icon": "toggle_icons/icon_always_on_lateral.png", "color": "#597497", "visible": lambda: self._params.get_bool("AlwaysOnLateral") and starpilot_state.car_state.lkasAllowedForAOL},
      {"title": tr_noop("Pause Below"), "type": "value", "get_value": lambda: f"{self._params.get_int('PauseAOLOnBrake')} mph", "on_click": lambda: self._show_speed_selector("PauseAOLOnBrake"), "icon": "toggle_icons/icon_always_on_lateral.png", "color": "#597497", "visible": lambda: self._params.get_bool("AlwaysOnLateral")},
    ]
    self._rebuild_grid()

  def _show_speed_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()
    gui_app.push_widget(AetherSliderDialog(tr(key), 0, 100, 1, self._params.get_int(key), on_close, unit=" mph", color="#597497"))

  def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state
    if ui_state.started:
      gui_app.push_widget(ConfirmDialog(tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), callback=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None))

class StarPilotLaneChangesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Lane Changes"), "type": "toggle", "get_state": lambda: self._params.get_bool("LaneChanges"), "set_state": lambda s: self._params.put_bool("LaneChanges", s), "icon": "toggle_icons/icon_lane.png", "color": "#597497"},
      {"title": tr_noop("Automatic Lane Changes"), "type": "toggle", "get_state": lambda: self._params.get_bool("NudgelessLaneChange"), "set_state": lambda s: self._params.put_bool("NudgelessLaneChange", s), "icon": "toggle_icons/icon_lane.png", "color": "#597497", "visible": lambda: self._params.get_bool("LaneChanges")},
      {"title": tr_noop("Lane Change Delay"), "type": "value", "get_value": lambda: f"{self._params.get_float('LaneChangeTime'):.1f}s", "on_click": lambda: self._show_float_selector("LaneChangeTime", 0.0, 5.0, 0.1, "s"), "icon": "toggle_icons/icon_lane.png", "color": "#597497", "visible": lambda: self._params.get_bool("LaneChanges") and self._params.get_bool("NudgelessLaneChange")},
      {"title": tr_noop("Min Lane Change Speed"), "type": "value", "get_value": lambda: f"{self._params.get_int('MinimumLaneChangeSpeed')} mph", "on_click": lambda: self._show_speed_selector("MinimumLaneChangeSpeed"), "icon": "toggle_icons/icon_lane.png", "color": "#597497", "visible": lambda: self._params.get_bool("LaneChanges")},
      {"title": tr_noop("Minimum Lane Width"), "type": "value", "get_value": lambda: f"{self._params.get_float('LaneDetectionWidth'):.1f} ft", "on_click": lambda: self._show_float_selector("LaneDetectionWidth", 0.0, 15.0, 0.1, " ft"), "icon": "toggle_icons/icon_lane.png", "color": "#597497", "visible": lambda: self._params.get_bool("LaneChanges") and self._params.get_bool("NudgelessLaneChange")},
      {"title": tr_noop("One Lane Change Per Signal"), "type": "toggle", "get_state": lambda: self._params.get_bool("OneLaneChange"), "set_state": lambda s: self._params.put_bool("OneLaneChange", s), "icon": "toggle_icons/icon_lane.png", "color": "#597497", "visible": lambda: self._params.get_bool("LaneChanges")},
      {"title": tr_noop("Lane Change Smoothing"), "desc": tr_noop("How smoothly openpilot commits to a lane change. 10 is stock; lower values produce a gentler, more gradual maneuver."), "type": "value", "get_value": lambda: f"{self._params.get_int('LaneChangeSmoothing')}", "on_click": lambda: self._show_pace_selector("LaneChangeSmoothing"), "icon": "toggle_icons/icon_lane.png", "color": "#597497", "visible": lambda: self._params.get_bool("LaneChanges")},
    ]
    self._rebuild_grid()

  def _show_speed_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()
    gui_app.push_widget(AetherSliderDialog(tr(key), 0, 100, 1, self._params.get_int(key), on_close, unit=" mph", color="#597497"))

  def _show_float_selector(self, key, min_v, max_v, step, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_float(key, float(val))
        self._rebuild_grid()
    gui_app.push_widget(AetherSliderDialog(tr(key), min_v, max_v, step, self._params.get_float(key), on_close, unit=unit, color="#597497"))

  def _show_pace_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()
    current = self._params.get_int(key) if self._params.get_int(key) > 0 else 10
    gui_app.set_modal_overlay(AetherSliderDialog(tr(key), 1, 10, 1, current, on_close, color="#597497"))

class StarPilotLateralTuneLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Force Turn Desires Below Lane Change Speed"), "desc": tr_noop("Allow openpilot to follow turn intent below the minimum lane change speed when signaling."), "type": "toggle", "get_state": lambda: self._params.get_bool("TurnDesires"), "set_state": lambda x: self._params.put_bool("TurnDesires", x), "icon": "toggle_icons/icon_lateral_tune.png", "color": "#597497", "visible": self._lateral_enabled},
      {"title": tr_noop("Neural Network Feedforward (NNFF)"), "desc": tr_noop("Use the full neural-network feedforward steering controller when available."), "type": "toggle", "get_state": lambda: self._params.get_bool("NNFF"), "set_state": self._set_nnff, "icon": "toggle_icons/icon_lateral_tune.png", "color": "#597497", "visible": self._show_nnff},
      {"title": tr_noop("Neural Network Feedforward (NNFF) Lite"), "desc": tr_noop("Use the lightweight NNFF steering logic when the full model is off."), "type": "toggle", "get_state": lambda: self._params.get_bool("NNFFLite"), "set_state": lambda x: self._on_reboot_toggle("NNFFLite", x), "icon": "toggle_icons/icon_lateral_tune.png", "color": "#597497", "visible": self._show_nnff_lite},
    ]
    self._rebuild_grid()

  def _lateral_enabled(self):
    return self._params.get_bool("LateralTune")

  def _show_nnff(self):
    return self._lateral_enabled() and starpilot_state.car_state.hasNNFFLog and not starpilot_state.car_state.isAngleCar

  def _show_nnff_lite(self):
    return self._lateral_enabled() and not self._params.get_bool("NNFF") and not starpilot_state.car_state.isAngleCar

  def _set_nnff(self, state):
    self._params.put_bool("NNFF", state)
    if state:
      self._params.put_bool("NNFFLite", False)

  def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state
    if ui_state.started:
      gui_app.push_widget(ConfirmDialog(tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), callback=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None))

class StarPilotLateralQOLLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Quality of Life"), "type": "toggle", "get_state": lambda: self._params.get_bool("QOLLateral"), "set_state": lambda x: self._params.put_bool("QOLLateral", x), "icon": "toggle_icons/icon_quality_of_life.png", "color": "#597497"},
      {"title": tr_noop("Pause Steering Below"), "type": "value", "get_value": lambda: f"{self._params.get_int('PauseLateralSpeed')} mph", "on_click": lambda: self._show_speed_selector("PauseLateralSpeed"), "icon": "toggle_icons/icon_quality_of_life.png", "color": "#597497", "visible": lambda: self._params.get_bool("QOLLateral")}
    ]
    self._rebuild_grid()

  def _show_speed_selector(self, key):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()
    gui_app.push_widget(AetherSliderDialog(tr(key), 0, 100, 1, self._params.get_int(key), on_close, unit=" mph", color="#597497"))

class StarPilotLateralLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    steering_panel = create_tile_panel([
      {"title": tr_noop("Always On Lateral"), "panel": "always_on_lateral", "icon": "toggle_icons/icon_always_on_lateral.png", "color": "#597497"},
      {"title": tr_noop("Quality of Life"), "panel": "qol", "icon": "toggle_icons/icon_quality_of_life.png", "color": "#597497"},
    ], {
      "always_on_lateral": StarPilotAlwaysOnLateralLayout(),
      "qol": StarPilotLateralQOLLayout(),
    })
    tune_panel = create_master_toggle_panel([
      {
        "title": tr_noop("Advanced Lateral Tuning"),
        "desc": tr_noop("Advanced steering control changes to fine-tune how openpilot drives."),
        "manage_title": tr_noop("Advanced Settings"),
        "manage_desc": tr_noop("Adjust steering response, torque controller behavior, and auto-tuning controls."),
        "manage_label": tr_noop("Configure"),
        "disabled_label": tr_noop("Enable First"),
        "panel": "advanced_lateral",
        "get_state": lambda: self._params.get_bool("AdvancedLateralTune"),
        "set_state": lambda x: self._params.put_bool("AdvancedLateralTune", x),
        "icon": "toggle_icons/icon_advanced_lateral_tune.png",
        "color": "#597497",
      },
      {
        "title": tr_noop("Lateral Tuning"),
        "desc": tr_noop("Miscellaneous steering control changes such as turn desires and NNFF modes."),
        "manage_title": tr_noop("Lateral Settings"),
        "manage_desc": tr_noop("Open turn-intent and neural-network feedforward controls."),
        "manage_label": tr_noop("Configure"),
        "disabled_label": tr_noop("Enable First"),
        "panel": "lateral_tune",
        "get_state": lambda: self._params.get_bool("LateralTune"),
        "set_state": lambda x: self._params.put_bool("LateralTune", x),
        "icon": "toggle_icons/icon_lateral_tune.png",
        "color": "#597497",
      },
    ], {
      "advanced_lateral": StarPilotAdvancedLateralLayout(),
      "lateral_tune": StarPilotLateralTuneLayout(),
    })

    self._section_tabs = TabbedSectionHost([
      TabSectionSpec("steering", "Steering", steering_panel),
      TabSectionSpec("lane", "Lane", StarPilotLaneChangesLayout()),
      TabSectionSpec("tune", "Tune", tune_panel),
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
