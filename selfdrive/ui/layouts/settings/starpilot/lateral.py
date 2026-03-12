from __future__ import annotations

from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog
from openpilot.system.ui.widgets.list_view import button_item, value_button_item, button_toggle_item, toggle_item, value_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotAdvancedLateralLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      value_button_item(
        lambda: (
          tr("Actuator Delay")
          + (f" (Default: {starpilot_state.car_state.steerActuatorDelay:.2f})" if starpilot_state.car_state.steerActuatorDelay != 0 else "")
        ),
        "SteerDelay",
        min_val=0.01,
        max_val=1.0,
        step=0.01,
        button_text="Reset",
        button_callback=lambda: self._params.put_float("SteerDelay", starpilot_state.car_state.steerActuatorDelay),
        description=tr_noop(
          "<b>The time between openpilot's steering command and the vehicle's response.</b> Increase if the vehicle reacts late; decrease if it feels jumpy. Auto-learned by default."
        ),
        enabled=lambda: starpilot_state.car_state.steerActuatorDelay != 0,
      ),
      value_button_item(
        lambda: tr("Friction") + (f" (Default: {starpilot_state.car_state.friction:.2f})" if starpilot_state.car_state.friction != 0 else ""),
        "SteerFriction",
        min_val=0.0,
        max_val=1.0,
        step=0.01,
        button_text="Reset",
        button_callback=lambda: self._params.put_float("SteerFriction", starpilot_state.car_state.friction),
        description=tr_noop(
          "<b>Compensates for steering friction.</b> Increase if the wheel sticks near center; decrease if it jitters. Auto-learned by default."
        ),
        enabled=lambda: (
          starpilot_state.car_state.friction != 0
          and (not starpilot_state.car_state.hasAutoTune or (starpilot_state.car_state.hasAutoTune and self._params.get_bool("ForceAutoTuneOff")))
        ),
      ),
      value_button_item(
        lambda: tr("Kp Factor") + (f" (Default: {starpilot_state.car_state.steerKp:.2f})" if starpilot_state.car_state.steerKp != 0 else ""),
        "SteerKP",
        min_val=lambda: starpilot_state.car_state.steerKp * 0.5,
        max_val=lambda: starpilot_state.car_state.steerKp * 1.5,
        step=0.01,
        button_text="Reset",
        button_callback=lambda: self._params.put_float("SteerKP", starpilot_state.car_state.steerKp),
        description=tr_noop(
          "<b>How strongly openpilot corrects lane position.</b> Higher is tighter but twitchier; lower is smoother but slower. Auto-learned by default."
        ),
        enabled=lambda: starpilot_state.car_state.steerKp != 0 and not starpilot_state.car_state.isAngleCar,
      ),
      value_button_item(
        lambda: (
          tr("Lateral Acceleration") + (f" (Default: {starpilot_state.car_state.latAccelFactor:.2f})" if starpilot_state.car_state.latAccelFactor != 0 else "")
        ),
        "SteerLatAccel",
        min_val=lambda: starpilot_state.car_state.latAccelFactor * 0.5,
        max_val=lambda: starpilot_state.car_state.latAccelFactor * 1.5,
        step=0.01,
        button_text="Reset",
        button_callback=lambda: self._params.put_float("SteerLatAccel", starpilot_state.car_state.latAccelFactor),
        description=tr_noop(
          "<b>Maps steering torque to turning response.</b> Increase for sharper turns; decrease for gentler steering. Auto-learned by default."
        ),
        enabled=lambda: (
          starpilot_state.car_state.latAccelFactor != 0
          and (not starpilot_state.car_state.hasAutoTune or (starpilot_state.car_state.hasAutoTune and self._params.get_bool("ForceAutoTuneOff")))
        ),
      ),
      value_button_item(
        lambda: tr("Steer Ratio") + (f" (Default: {starpilot_state.car_state.steerRatio:.2f})" if starpilot_state.car_state.steerRatio != 0 else ""),
        "SteerRatio",
        min_val=lambda: starpilot_state.car_state.steerRatio * 0.5,
        max_val=lambda: starpilot_state.car_state.steerRatio * 1.5,
        step=0.01,
        button_text="Reset",
        button_callback=lambda: self._params.put_float("SteerRatio", starpilot_state.car_state.steerRatio),
        description=tr_noop(
          "<b>The relationship between steering wheel rotation and road wheel angle.</b> Increase if steering feels too quick or twitchy; decrease if it feels too slow or weak. Auto-learned by default."
        ),
        enabled=lambda: (
          starpilot_state.car_state.steerRatio != 0
          and (not starpilot_state.car_state.hasAutoTune or (starpilot_state.car_state.hasAutoTune and self._params.get_bool("ForceAutoTuneOff")))
        ),
      ),
      toggle_item(
        tr_noop("Force Auto-Tune On"),
        tr_noop("<b>Force-enable openpilot's live auto-tuning for \"Friction\" and \"Lateral Acceleration\".</b>"),
        self._params.get_bool("ForceAutoTune"),
        callback=lambda x: self._params.put_bool("ForceAutoTune", x),
        enabled=lambda: not starpilot_state.car_state.hasAutoTune and not starpilot_state.car_state.isAngleCar,
      ),
      toggle_item(
        tr_noop("Force Auto-Tune Off"),
        tr_noop("<b>Force-disable openpilot's live auto-tuning for \"Friction\" and \"Lateral Acceleration\" and use the set value instead.</b>"),
        self._params.get_bool("ForceAutoTuneOff"),
        callback=lambda x: self._params.put_bool("ForceAutoTuneOff", x),
        enabled=lambda: starpilot_state.car_state.hasAutoTune,
      ),
      toggle_item(
        tr_noop("Force Torque Controller"),
        tr_noop("<b>Use torque-based steering control instead of angle-based control for smoother lane keeping, especially in curves.</b>"),
        self._params.get_bool("ForceTorqueController"),
        callback=lambda x: self._on_reboot_toggle("ForceTorqueController", x),
        enabled=lambda: not starpilot_state.car_state.isAngleCar and not starpilot_state.car_state.isTorqueCar,
      ),
    ]
    self._scroller = Scroller(items, line_separator=True, spacing=0)

  def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state

    if ui_state.started:
      from openpilot.system.ui.lib.application import gui_app

      def _confirm_reboot(res):
        gui_app.set_modal_overlay(None)
        if res == DialogResult.CONFIRM:
          from openpilot.system.hardware import HARDWARE
          HARDWARE.reboot()

      dialog = ConfirmDialog("Reboot required to take effect. Reboot now?", "Reboot", "Cancel", on_close=_confirm_reboot)
      gui_app.set_modal_overlay(dialog)

class StarPilotAlwaysOnLateralLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      toggle_item(
        tr_noop("Always On Lateral"),
        tr_noop("<b>openpilot's steering remains active even when the accelerator or brake pedals are pressed.</b>"),
        self._params.get_bool("AlwaysOnLateral"),
        callback=lambda x: self._on_reboot_toggle("AlwaysOnLateral", x),
        icon="toggle_icons/icon_always_on_lateral.png",
        starpilot_icon=True,
      ),
      toggle_item(
        tr_noop("Enable With LKAS"),
        tr_noop("<b>Enable \"Always On Lateral\" whenever \"LKAS\" is on, even when openpilot is not engaged.</b>"),
        self._params.get_bool("AlwaysOnLateralLKAS"),
        callback=lambda x: self._params.put_bool("AlwaysOnLateralLKAS", x),
        enabled=lambda: starpilot_state.car_state.lkasAllowedForAOL,
      ),
      value_item(
        tr_noop("Pause on Brake Press Below"),
        "PauseAOLOnBrake",
        min_val=0,
        max_val=99,
        step=1,
        unit="mph",
        description=tr_noop("<b>Pause \"Always On Lateral\" below the set speed while the brake pedal is pressed.</b>"),
        is_metric=True,
      ),
    ]
    self._scroller = Scroller(items, line_separator=True, spacing=0)

  def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state

    if ui_state.started:
      from openpilot.system.ui.lib.application import gui_app

      def _confirm_reboot(res):
        gui_app.set_modal_overlay(None)
        if res == DialogResult.CONFIRM:
          from openpilot.system.hardware import HARDWARE
          HARDWARE.reboot()

      dialog = ConfirmDialog("Reboot required to take effect. Reboot now?", "Reboot", "Cancel", on_close=_confirm_reboot)
      gui_app.set_modal_overlay(dialog)

class StarPilotLaneChangesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    def _get_lane_change_labels():
      labels = {0.0: tr("Instant")}
      for i in range(1, 51):
        val = i / 10.0
        labels[val] = f"{val:.1f} seconds" if val != 1.0 else "1.0 second"
      return labels

    items = [
      toggle_item(
        tr_noop("Lane Changes"),
        tr_noop("<b>Allow openpilot to change lanes.</b>"),
        self._params.get_bool("LaneChanges"),
        callback=lambda x: self._params.put_bool("LaneChanges", x),
        icon="toggle_icons/icon_lane.png",
        starpilot_icon=True,
      ),
      toggle_item(
        tr_noop("Automatic Lane Changes"),
        tr_noop("<b>When the turn signal is on, openpilot will automatically change lanes.</b> No steering-wheel nudge required!"),
        self._params.get_bool("NudgelessLaneChange"),
        callback=lambda x: self._params.put_bool("NudgelessLaneChange", x),
      ),
      value_item(
        tr_noop("Lane Change Delay"),
        "LaneChangeTime",
        min_val=0.0,
        max_val=5.0,
        step=0.1,
        description=tr_noop("<b>Delay between turn signal activation and the start of an automatic lane change.</b>"),
        labels=_get_lane_change_labels(),
        enabled=lambda: self._params.get_bool("LaneChanges") and self._params.get_bool("NudgelessLaneChange"),
      ),
      value_item(
        tr_noop("Minimum Lane Change Speed"),
        "MinimumLaneChangeSpeed",
        min_val=0,
        max_val=99,
        step=1,
        unit="mph",
        description=tr_noop("<b>Lowest speed at which openpilot will change lanes.</b>"),
        is_metric=True,
      ),
      value_item(
        tr_noop("Minimum Lane Width"),
        "LaneDetectionWidth",
        min_val=0.0,
        max_val=15.0,
        step=0.1,
        unit="feet",
        description=tr_noop("<b>Prevent automatic lane changes into lanes narrower than the set width.</b>"),
        enabled=lambda: self._params.get_bool("LaneChanges") and self._params.get_bool("NudgelessLaneChange"),
        is_metric=True,
      ),
      toggle_item(
        tr_noop("One Lane Change Per Signal"),
        tr_noop("<b>Limit automatic lane changes to one per turn-signal activation.</b>"),
        self._params.get_bool("OneLaneChange"),
        callback=lambda x: self._params.put_bool("OneLaneChange", x),
      ),
    ]
    self._scroller = Scroller(items, line_separator=True, spacing=0)

class StarPilotLateralTuneLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      toggle_item(
        tr_noop("Force Turn Desires Below Lane Change Speed"),
        tr_noop("<b>While driving below the minimum lane change speed with an active turn signal, instruct openpilot to turn left/right.</b>"),
        self._params.get_bool("TurnDesires"),
        callback=lambda x: self._params.put_bool("TurnDesires", x),
      ),
      toggle_item(
        tr_noop("Neural Network Feedforward (NNFF)"),
        tr_noop(
          "<b>Twilsonco's \"Neural Network FeedForward\" controller.</b> Uses a trained neural network model to predict steering torque based on vehicle speed, roll, and past/future planned path data for smoother, model-based steering."
        ),
        self._params.get_bool("NNFF"),
        callback=lambda x: self._on_reboot_toggle("NNFF", x),
        enabled=lambda: starpilot_state.car_state.hasNNFFLog and not starpilot_state.car_state.isAngleCar,
      ),
      toggle_item(
        tr_noop("Neural Network Feedforward (NNFF) Lite"),
        tr_noop(
          "<b>A lightweight version of Twilsonco's \"Neural Network FeedForward\" controller.</b> Uses the \"look-ahead\" planned lateral jerk logic from the full model to help smoothen steering adjustments in curves, but does not use the full neural network for torque calculation."
        ),
        self._params.get_bool("NNFFLite"),
        callback=lambda x: self._on_reboot_toggle("NNFFLite", x),
        enabled=lambda: not self._params.get_bool("NNFF") and not starpilot_state.car_state.isAngleCar,
      ),
    ]
    self._scroller = Scroller(items, line_separator=True, spacing=0)

  def _on_reboot_toggle(self, key, state):
    self._params.put_bool(key, state)
    from openpilot.selfdrive.ui.ui_state import ui_state

    if ui_state.started:
      from openpilot.system.ui.lib.application import gui_app

      def _confirm_reboot(res):
        gui_app.set_modal_overlay(None)
        if res == DialogResult.CONFIRM:
          from openpilot.system.hardware import HARDWARE
          HARDWARE.reboot()

      dialog = ConfirmDialog("Reboot required to take effect. Reboot now?", "Reboot", "Cancel", on_close=_confirm_reboot)
      gui_app.set_modal_overlay(dialog)

class StarPilotLateralQOLLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      value_button_item(
        tr_noop("Pause Steering Below"),
        "PauseLateralSpeed",
        min_val=0,
        max_val=99,
        step=1,
        unit="mph",
        description=tr_noop("<b>Pause steering below the set speed.</b>"),
        sub_toggles=[("PauseLateralOnSignal", True)],
        labels={0: tr_noop("Off")},
        is_metric=True,
      )
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

class StarPilotLateralLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    self._sub_panels = {
      "advanced_lateral": StarPilotAdvancedLateralLayout(),
      "always_on_lateral": StarPilotAlwaysOnLateralLayout(),
      "lane_changes": StarPilotLaneChangesLayout(),
      "lateral_tune": StarPilotLateralTuneLayout(),
      "qol": StarPilotLateralQOLLayout(),
    }

    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

    items = [
      button_item(
        tr_noop("Advanced Lateral Tuning"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Advanced steering control changes to fine-tune how openpilot drives.</b>"),
        callback=lambda: self._navigate_to("advanced_lateral"),
        icon="toggle_icons/icon_advanced_lateral_tune.png",
        starpilot_icon=True,
      ),
      button_item(
        tr_noop("Always On Lateral"),
        lambda: tr("MANAGE"),
        tr_noop("<b>openpilot's steering remains active even when the accelerator or brake pedals are pressed.</b>"),
        callback=lambda: self._navigate_to("always_on_lateral"),
        icon="toggle_icons/icon_always_on_lateral.png",
        starpilot_icon=True,
      ),
      button_item(
        tr_noop("Lane Changes"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Allow openpilot to change lanes.</b>"),
        callback=lambda: self._navigate_to("lane_changes"),
        icon="toggle_icons/icon_lane.png",
        starpilot_icon=True,
      ),
      button_item(
        tr_noop("Lateral Tuning"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Miscellaneous steering control changes</b> to fine-tune how openpilot drives."),
        callback=lambda: self._navigate_to("lateral_tune"),
        icon="toggle_icons/icon_lateral_tune.png",
        starpilot_icon=True,
      ),
      button_item(
        tr_noop("Quality of Life"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Steering control changes to fine-tune how openpilot drives.</b>"),
        callback=lambda: self._navigate_to("qol"),
        icon="toggle_icons/icon_quality_of_life.png",
        starpilot_icon=True,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
