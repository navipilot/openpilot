from __future__ import annotations
from collections.abc import Callable
from dataclasses import dataclass
from enum import IntEnum

import pyray as rl

from openpilot.common.params import Params
from openpilot.system.ui.widgets import Widget

class StarPilotPanelType(IntEnum):
  MAIN = 0
  SOUNDS = 1
  DRIVING_MODEL = 2
  LONGITUDINAL = 3
  LATERAL = 4
  MAPS = 5
  NAVIGATION = 6
  DATA = 7
  DEVICE = 8
  UTILITIES = 9
  VISUALS = 10
  THEMES = 11
  VEHICLE = 12
  WHEEL = 13

@dataclass
class StarPilotPanelInfo:
  name: str
  instance: Widget

class StarPilotPanel(Widget):
  def __init__(self):
    super().__init__()
    self._params = Params()
    self._params_memory = Params(memory=True)
    self._tuning_levels: dict[str, int] = {}
    self._navigate_callback: Callable | None = None
    self._back_callback: Callable | None = None
    self._current_sub_panel = ""
    self._sub_panels: dict[str, Widget] = {}
    self._scroller = None

  def set_navigate_callback(self, callback: Callable):
    self._navigate_callback = callback

  def set_back_callback(self, callback: Callable):
    self._back_callback = callback

  def set_tuning_levels(self, levels: dict[str, int]):
    self._tuning_levels = levels

  def _navigate_to(self, sub_panel: str):
    self._current_sub_panel = sub_panel
    if self._navigate_callback:
      self._navigate_callback(sub_panel)

  def _go_back(self):
    self._current_sub_panel = ""
    if self._back_callback:
      self._back_callback()

  def _render(self, rect: rl.Rectangle):
    if self._current_sub_panel and self._current_sub_panel in self._sub_panels:
      self._sub_panels[self._current_sub_panel].render(rect)
    elif self._scroller:
      self._scroller.render(rect)

  def show_event(self):
    super().show_event()
    if self._current_sub_panel and self._current_sub_panel in self._sub_panels:
      self._sub_panels[self._current_sub_panel].show_event()
    elif self._scroller:
      self._scroller.show_event()
