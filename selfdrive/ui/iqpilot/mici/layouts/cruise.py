"""
Copyright © IQ.Lvbs, apart of Project Teal Lvbs, All Rights Reserved, licensed under https://konn3kt.com/tos

Cruise control settings for MICI (comma 4).
"""
from collections.abc import Callable

import pyray as rl

from openpilot.common.params import Params
from openpilot.selfdrive.ui.mici.widgets.button import DrumMappedParamButton, DrumParamButton, NeonBigParamToggle
from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.system.ui.widgets import NavWidget
from openpilot.system.ui.widgets.scroller import Scroller

# LongitudinalPersonality: 0=aggressive, 1=standard, 2=relaxed, 3=stock
FOLLOW_DISTANCE_OPTIONS = ["aggressive", "standard", "relaxed", "stock"]
FOLLOW_DISTANCE_VALUES  = [0, 1, 2, 3]


class CruiseLayoutMici(NavWidget):
  def __init__(self, back_callback: Callable | None = None):
    super().__init__()
    self._params = Params()

    # Experimental mode — enables IQ.Pilot longitudinal (FSD-style)
    self._experimental = NeonBigParamToggle(
      "Experimental Mode", "ExperimentalMode",
      toggle_callback=self._on_experimental_toggled,
    )

    # IQ Dynamic Mode — IQ.Pilot's dynamic longitudinal profile
    self._dynamic = NeonBigParamToggle(
      "IQ Dynamic Mode", "IQDynamicMode",
    )

    # Follow distance selector — drum picker with personality labels
    self._follow_dist = DrumMappedParamButton(
      "Follow Distance", "LongitudinalPersonality",
      FOLLOW_DISTANCE_OPTIONS, FOLLOW_DISTANCE_VALUES,
    )

    # Speed limit control — drum picker
    self._speed_limit = DrumParamButton(
      "Speed Limit Mode", "SpeedLimitMode",
      ["off", "info", "warning", "control"],
    )

    self._scroller = Scroller([
      self._experimental,
      self._dynamic,
      self._follow_dist,
      self._speed_limit,
    ], snap_items=False)

    if back_callback:
      self.set_back_callback(back_callback)

  def _on_experimental_toggled(self, checked: bool):
    # Experimental mode and IQ Dynamic Mode are mutually exclusive
    if checked:
      self._params.put_bool("IQDynamicMode", False)
      self._dynamic.refresh()
    self._refresh_enabled()

  def _refresh_enabled(self):
    long_available = (
      ui_state.CP is not None and
      getattr(ui_state.CP, "openpilotLongitudinalControl", False)
    )
    exp_checked  = self._experimental._checked
    dyn_checked  = self._dynamic._checked

    self._experimental.set_enabled(long_available and ui_state.is_offroad())
    self._dynamic.set_enabled(long_available and ui_state.is_offroad() and not exp_checked)
    self._follow_dist.set_enabled(long_available)
    self._speed_limit.set_enabled(long_available)

  def show_event(self):
    super().show_event()
    self._experimental.refresh()
    self._dynamic.refresh()
    self._follow_dist.refresh()
    self._speed_limit.refresh()
    self._refresh_enabled()
    self._scroller.show_event()

  def _update_state(self):
    super()._update_state()
    self._refresh_enabled()

  def _render(self, rect: rl.Rectangle):
    self._scroller.render(rect)
