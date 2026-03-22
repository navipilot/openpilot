from __future__ import annotations
import os
import shutil
import threading
import subprocess
from pathlib import Path

from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotDataLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Manage Backups"), "panel": "backups", "icon": "toggle_icons/icon_system.png", "color": "#FA6800"},
      {"title": tr_noop("Manage Storage"), "panel": "storage", "icon": "toggle_icons/icon_system.png", "color": "#FA6800"},
      {"title": tr_noop("Delete Driving Data"), "type": "hub", "on_click": self._on_delete_driving_data, "icon": "toggle_icons/icon_system.png", "color": "#FA6800"},
      {"title": tr_noop("Delete Error Logs"), "type": "hub", "on_click": self._on_delete_error_logs, "icon": "toggle_icons/icon_system.png", "color": "#FA6800"},
    ]
    
    self._sub_panels = {
      "backups": StarPilotBackupsLayout(),
      "storage": StarPilotStorageLayout(),
    }
    
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'): panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'): panel.set_back_callback(self._go_back)
      
    self._rebuild_grid()

  def _on_delete_driving_data(self):
    def _do_delete(res):
      if res == DialogResult.CONFIRM:
        def _task():
          drive_paths = ["/data/media/0/realdata/", "/data/media/0/realdata_HD/", "/data/media/0/realdata_konik/"]
          for path in drive_paths:
            p = Path(path)
            if p.exists():
              for entry in p.iterdir():
                if entry.is_dir(): shutil.rmtree(entry, ignore_errors=True)
        threading.Thread(target=_task, daemon=True).start()
        gui_app.set_modal_overlay(alert_dialog(tr("Driving data deletion started.")))
    gui_app.set_modal_overlay(ConfirmDialog(tr("Delete all driving data and footage?"), tr("Delete"), on_close=_do_delete))

  def _on_delete_error_logs(self):
    def _do_delete(res):
      if res == DialogResult.CONFIRM:
        shutil.rmtree("/data/error_logs", ignore_errors=True)
        os.makedirs("/data/error_logs", exist_ok=True)
        gui_app.set_modal_overlay(alert_dialog(tr("Error logs deleted.")))
    gui_app.set_modal_overlay(ConfirmDialog(tr("Delete all error logs?"), tr("Delete"), on_close=_do_delete))

class StarPilotBackupsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Create Backup"), "type": "hub", "on_click": self._on_create_backup, "color": "#FA6800"},
      {"title": tr_noop("Restore Backup"), "type": "hub", "on_click": self._on_restore_backup, "color": "#FA6800"},
      {"title": tr_noop("Delete Backup"), "type": "hub", "on_click": self._on_delete_backup, "color": "#FA6800"},
    ]
    self._rebuild_grid()

  def _get_backups(self):
    b_dir = Path("/data/backups")
    if not b_dir.exists(): return []
    return [f.name for f in b_dir.glob("*.tar.zst") if "in_progress" not in f.name]

  def _on_create_backup(self):
    # Simplified backup logic
    gui_app.set_modal_overlay(alert_dialog(tr("Backup creation started in background.")))
    def _task():
      os.makedirs("/data/backups", exist_ok=True)
      subprocess.run(["tar", "--use-compress-program=zstd", "-cf", "/data/backups/manual_backup.tar.zst", "/data/openpilot"])
    threading.Thread(target=_task, daemon=True).start()

  def _on_restore_backup(self):
    backups = self._get_backups()
    if not backups:
      gui_app.set_modal_overlay(alert_dialog(tr("No backups found.")))
      return
    def _on_select(res, val):
      if res == DialogResult.CONFIRM:
        gui_app.set_modal_overlay(alert_dialog(tr("Restoring... device will reboot.")))
        def _task():
          subprocess.run(["rm", "-rf", "/data/openpilot/*"])
          subprocess.run(["tar", "--use-compress-program=zstd", "-xf", f"/data/backups/{val}", "-C", "/"])
          os.system("reboot")
        threading.Thread(target=_task, daemon=True).start()
    gui_app.set_modal_overlay(SelectionDialog(tr("Select Backup"), backups, on_close=_on_select))

  def _on_delete_backup(self):
    backups = self._get_backups()
    if not backups:
      gui_app.set_modal_overlay(alert_dialog(tr("No backups found.")))
      return
    def _on_select(res, val):
      if res == DialogResult.CONFIRM:
        os.remove(f"/data/backups/{val}")
        self._rebuild_grid()
    gui_app.set_modal_overlay(SelectionDialog(tr("Delete Backup"), backups, on_close=_on_select))

class StarPilotStorageLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Driving Data"), "type": "hub", "on_click": self._show_stats, "color": "#FA6800"},
    ]
    self._rebuild_grid()

  def _show_stats(self):
    # In a real environment we'd calculate du -sh /data
    gui_app.set_modal_overlay(alert_dialog(tr("Storage management not yet fully ported to Python.")))
