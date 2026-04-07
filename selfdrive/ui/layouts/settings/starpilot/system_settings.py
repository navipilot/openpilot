from __future__ import annotations

import pyray as rl

from openpilot.system.ui.lib.multilang import tr

from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import RadioTileGroup
from openpilot.selfdrive.ui.layouts.settings.starpilot.data import StarPilotDataLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.device import StarPilotDeviceLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.utilities import StarPilotUtilitiesLayout


class StarPilotSystemLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    self._section_names = ["device", "data_and_backups", "utilities"]
    self._active_section = self._section_names[0]
    self._sub_panels = {
      "device": StarPilotDeviceLayout(),
      "data_and_backups": StarPilotDataLayout(),
      "utilities": StarPilotUtilitiesLayout(),
    }

    self._section_tabs = RadioTileGroup(
      "",
      [tr("Device"), tr("Data"), tr("Utilities")],
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
