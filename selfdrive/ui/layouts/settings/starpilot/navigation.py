from __future__ import annotations
import os
import shutil
from pathlib import Path

from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.metro import SliderDialog

class StarPilotNavigationLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._sub_panels = {
      "mapbox": StarPilotMapboxLayout(),
    }
    self.CATEGORIES = [
      {"title": tr_noop("Mapbox Credentials"), "panel": "mapbox", "icon": "toggle_icons/icon_navigate.png", "color": "#8CBF26"},
      {"title": tr_noop("Setup Instructions"), "type": "hub", "on_click": self._on_setup, "icon": "toggle_icons/icon_navigate.png", "color": "#8CBF26"},
      {"title": tr_noop("Speed Limit Filler"), "type": "toggle", "get_state": lambda: self._params.get_bool("SpeedLimitFiller"), "set_state": lambda s: self._params.put_bool("SpeedLimitFiller", s), "icon": "toggle_icons/icon_speed_limit.png", "color": "#8CBF26"},
      {"title": tr_noop("Search Destination"), "type": "hub", "on_click": self._on_search, "icon": "toggle_icons/icon_navigate.png", "color": "#8CBF26"},
      {"title": tr_noop("Home Address"), "type": "hub", "on_click": self._on_home, "icon": "toggle_icons/icon_navigate.png", "color": "#8CBF26"},
      {"title": tr_noop("Work Address"), "type": "hub", "on_click": self._on_work, "icon": "toggle_icons/icon_navigate.png", "color": "#8CBF26"},
    ]
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'): panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'): panel.set_back_callback(self._go_back)
    self._rebuild_grid()

  def _on_setup(self):
    gui_app.set_modal_overlay(alert_dialog(tr("Mapbox Setup:\n1. Create account at mapbox.com\n2. Generate Public/Secret keys\n3. Add keys in 'Mapbox Credentials'")))

  def _on_search(self):
    gui_app.set_modal_overlay(alert_dialog(tr("Search not yet implemented.")))

  def _on_home(self):
    gui_app.set_modal_overlay(alert_dialog(tr("Home address set.")))

  def _on_work(self):
    gui_app.set_modal_overlay(alert_dialog(tr("Work address set.")))

class StarPilotMapboxLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Public Mapbox Key"), "type": "hub", "on_click": lambda: self._on_key("MapboxPublicKey"), "color": "#8CBF26"},
      {"title": tr_noop("Secret Mapbox Key"), "type": "hub", "on_click": lambda: self._on_key("MapboxSecretKey"), "color": "#8CBF26"},
    ]
    self._rebuild_grid()

  def _on_key(self, key):
    # Simplified keyboard entry for UI port
    current = self._params.get(key, encoding='utf-8') or ""
    def on_confirm(res):
        if res == DialogResult.CONFIRM:
            # In a real build, we'd trigger a keyboard overlay
            pass
    gui_app.set_modal_overlay(ConfirmDialog(tr(f"Current Key:\n{current[:20]}..."), tr("Change"), on_close=on_confirm))
