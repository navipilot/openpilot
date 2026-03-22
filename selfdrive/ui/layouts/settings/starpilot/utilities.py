from __future__ import annotations
from openpilot.system.hardware import HARDWARE
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotUtilitiesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Debug Mode"), "type": "toggle", "get_state": lambda: self._params.get_bool("DebugMode"), "set_state": lambda s: self._params.put_bool("DebugMode", s), "color": "#FA6800"},
      {"title": tr_noop("Flash Panda"), "type": "hub", "on_click": self._on_flash_panda, "color": "#FA6800"},
      {"title": tr_noop("Force Drive State"), "type": "value", "get_value": self._get_force_drive_state, "on_click": self._on_force_drive_state, "color": "#FA6800"},
      {"title": tr_noop("The Pond"), "type": "hub", "on_click": self._on_pond_clicked, "color": "#FA6800"},
      {"title": tr_noop("Report Issue"), "type": "hub", "on_click": self._on_report_issue, "color": "#FA6800"},
      {"title": tr_noop("Reset Toggles"), "type": "hub", "on_click": self._on_reset_toggles, "color": "#FA6800"},
    ]
    self._rebuild_grid()

  def _get_force_drive_state(self):
    if self._params.get_bool("ForceOnroad"): return tr("Onroad")
    if self._params.get_bool("ForceOffroad"): return tr("Offroad")
    return tr("Default")

  def _on_flash_panda(self):
    def _do_flash(res):
      if res == DialogResult.CONFIRM:
        self._params_memory.put_bool("FlashPanda", True)
        gui_app.set_modal_overlay(alert_dialog(tr("Panda flashing started. Device will reboot when finished.")))
    gui_app.set_modal_overlay(ConfirmDialog(tr("Flash Panda firmware?"), tr("Flash"), on_close=_do_flash))

  def _on_force_drive_state(self):
    options = [tr("Offroad"), tr("Onroad"), tr("Default")]
    def on_select(res, val):
      if res == DialogResult.CONFIRM:
        if val == tr("Offroad"):
          self._params.put_bool("ForceOffroad", True)
          self._params.put_bool("ForceOnroad", False)
        elif val == tr("Onroad"):
          self._params.put_bool("ForceOnroad", True)
          self._params.put_bool("ForceOffroad", False)
        else:
          self._params.put_bool("ForceOffroad", False)
          self._params.put_bool("ForceOnroad", False)
        self._rebuild_grid()
    current = self._get_force_drive_state()
    gui_app.set_modal_overlay(SelectionDialog(tr("Force Drive State"), options, current, on_close=on_select))

  def _on_pond_clicked(self):
    gui_app.set_modal_overlay(alert_dialog(tr("The Pond pairing not yet implemented in Python.")))

  def _on_report_issue(self):
    gui_app.set_modal_overlay(alert_dialog(tr("Issue reporting not yet implemented in Python.")))

  def _on_reset_toggles(self):
    def _do_reset(res):
      if res == DialogResult.CONFIRM:
        # Simplified reset logic
        all_keys = self._params.all_keys()
        for k in all_keys:
          default = self._params.get_default_value(k)
          if default: self._params.put(k, default)
        gui_app.set_modal_overlay(alert_dialog(tr("Toggles reset to default.")))
        self._rebuild_grid()
    gui_app.set_modal_overlay(ConfirmDialog(tr("Reset all toggles to default?"), tr("Reset"), on_close=_do_reset))
