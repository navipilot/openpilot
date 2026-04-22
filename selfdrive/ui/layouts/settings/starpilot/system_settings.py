from __future__ import annotations
import json
import os
import shutil
import subprocess
import threading
from datetime import datetime
from pathlib import Path

import pyray as rl

from openpilot.system.hardware import HARDWARE
from openpilot.system.ui.lib.application import gui_app, FontWeight, MouseEvent, MousePos
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.widgets import DialogResult, Widget
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.keyboard import Keyboard
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog
from openpilot.system.ui.widgets.label import gui_label

from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import (
  AETHER_LIST_METRICS,
  AetherListColors,
  AetherVerticalSlider,
  build_list_panel_frame,
  draw_list_panel_shell,
  draw_toggle_pill,
)

LEGACY_STARPILOT_PARAM_RENAMES = {
  "FrogPilotApiToken": "StarPilotApiToken",
  "FrogPilotCarParams": "StarPilotCarParams",
  "FrogPilotCarParamsPersistent": "StarPilotCarParamsPersistent",
  "FrogPilotDongleId": "StarPilotDongleId",
  "FrogPilotStats": "StarPilotStats",
}

EXCLUDED_KEYS = {
  "AvailableModels",
  "AvailableModelNames",
  "StarPilotStats",
  "GithubSshKeys",
  "GithubUsername",
  "MapBoxRequests",
  "ModelDrivesAndScores",
  "OverpassRequests",
  "SpeedLimits",
  "SpeedLimitsFiltered",
  "UpdaterAvailableBranches",
}

REPORT_CATEGORIES = [
  "Acceleration feels harsh or jerky",
  "An alert was unclear and I'm not sure what it meant",
  "Braking is too sudden or uncomfortable",
  "I'm not sure if this is normal or a bug:",
  "My steering wheel buttons aren't working",
  "openpilot disengages when I don't expect it",
  "openpilot feels sluggish or slow to respond",
  "Something else (please describe)",
]


class SystemSettingsManagerView(Widget):
  """Single-view, zero-scroll, fully-flat system settings dashboard.
  Left column: always-visible slider bands + toggles.
  Right column: action groups filling full height."""

  ROW_PAD = 4
  BAND_GAP = 8  # gap between slider bands and between band zone and toggles

  def __init__(self, controller: "StarPilotSystemLayout"):
    super().__init__()
    self._controller = controller
    self._pressed_target: str | None = None
    self._action_rects: dict[str, rl.Rectangle] = {}
    self._toggle_rects: dict[str, rl.Rectangle] = {}

    shutdown_labels = {0: tr("5 mins")}
    for i in range(1, 4): shutdown_labels[i] = f"{i * 15} mins"
    for i in range(4, 34): shutdown_labels[i] = f"{i - 3} " + (tr("hour") if i == 4 else tr("hours"))
    brightness_labels = {101: tr("Auto"), 0: tr("Off")}

    self._vsliders: dict[str, AetherVerticalSlider] = {
      "ScreenBrightness": AetherVerticalSlider(0, 101, 1, self._controller._params.get_int("ScreenBrightness"), lambda v: self._controller._set_brightness("ScreenBrightness", v), title="Offroad", unit="%", labels=brightness_labels, color=AetherListColors.PRIMARY),
      "ScreenBrightnessOnroad": AetherVerticalSlider(1, 101, 1, max(1, self._controller._params.get_int("ScreenBrightnessOnroad")), lambda v: self._controller._set_brightness("ScreenBrightnessOnroad", max(1, int(v))), title="Onroad", unit="%", labels=brightness_labels, color=AetherListColors.PRIMARY),
      "ScreenTimeout": AetherVerticalSlider(5, 60, 5, self._controller._params.get_int("ScreenTimeout"), lambda v: self._controller._params.put_int("ScreenTimeout", int(v)), title="Off Timer", unit="s", color=AetherListColors.PRIMARY),
      "ScreenTimeoutOnroad": AetherVerticalSlider(5, 60, 5, self._controller._params.get_int("ScreenTimeoutOnroad"), lambda v: self._controller._params.put_int("ScreenTimeoutOnroad", int(v)), title="On Timer", unit="s", color=AetherListColors.PRIMARY),
      "DeviceShutdown": AetherVerticalSlider(0, 33, 1, self._controller._params.get_int("DeviceShutdown"), lambda v: self._controller._params.put_int("DeviceShutdown", int(v)), title="Shutdown", labels=shutdown_labels, color=AetherListColors.PRIMARY),
      "LowVoltageShutdown": AetherVerticalSlider(11.8, 12.5, 0.1, self._controller._params.get_float("LowVoltageShutdown"), lambda v: self._controller._params.put_float("LowVoltageShutdown", float(v)), title="Low Volt", unit="V", color=AetherListColors.PRIMARY),
    }

    # Slider bands: (label, slider_keys)
    self._display_band = ["ScreenBrightness", "ScreenBrightnessOnroad", "ScreenTimeout", "ScreenTimeoutOnroad"]
    self._power_band = ["DeviceShutdown", "LowVoltageShutdown"]

  def _clear_ephemeral_state(self):
    self._pressed_target = None

  def show_event(self):
    super().show_event()
    self._clear_ephemeral_state()

  def hide_event(self):
    super().hide_event()
    self._clear_ephemeral_state()

  def _handle_mouse_press(self, mouse_pos: MousePos):
    self._pressed_target = None
    for action_id, rect in self._action_rects.items():
      if rl.check_collision_point_rec(mouse_pos, rect):
        self._pressed_target = f"action:{action_id}"
        return
    for toggle_id, rect in self._toggle_rects.items():
      if rl.check_collision_point_rec(mouse_pos, rect):
        self._pressed_target = f"toggle:{toggle_id}"
        return
    for slider in self._vsliders.values():
      slider._handle_mouse_press(mouse_pos)

  def _handle_mouse_event(self, mouse_event: MouseEvent):
    for slider in self._vsliders.values():
      slider._handle_mouse_event(mouse_event)

  def _handle_mouse_release(self, mouse_pos: MousePos):
    target = self._pressed_target
    self._pressed_target = None
    if target:
      prefix, action_id = target.split(":", 1)
      rect = self._action_rects.get(action_id) if prefix == "action" else self._toggle_rects.get(action_id)
      if rect and rl.check_collision_point_rec(mouse_pos, rect):
        self._controller.handle_action(action_id)
    for slider in self._vsliders.values():
      slider._handle_mouse_release(mouse_pos)

  # --- Main render ---

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    self._action_rects.clear()
    self._toggle_rects.clear()

    frame = build_list_panel_frame(rect)
    draw_list_panel_shell(frame)

    hdr = frame.header
    gui_label(rl.Rectangle(hdr.x, hdr.y + 4, hdr.width * 0.55, 40), tr("System Settings"), 40, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(hdr.x, hdr.y + 48, hdr.width * 0.7, 36), tr("Manage device behavior, power, and storage."), 24, AetherListColors.SUBTEXT, FontWeight.NORMAL)

    actual_header_h = 100
    content_y = hdr.y + actual_header_h
    content_h = (frame.shell.y + frame.shell.height) - content_y - AETHER_LIST_METRICS.panel_padding_bottom
    content_w = frame.scroll.width
    section_gap = AETHER_LIST_METRICS.section_gap

    col_w = (content_w - section_gap) / 2
    left_col = rl.Rectangle(frame.scroll.x, content_y, col_w, content_h)
    right_col = rl.Rectangle(frame.scroll.x + col_w + section_gap, content_y, col_w, content_h)

    self._sync_slider_values()
    self._draw_left_column(left_col)
    self._draw_actions_column(right_col)

  def _sync_slider_values(self):
    params = self._controller._params
    for key, slider in self._vsliders.items():
      if slider._is_dragging:
        continue
      pval = float(params.get_float(key)) if key == "LowVoltageShutdown" else float(params.get_int(key))
      if slider.current_val != pval:
        slider.current_val = pval

  # --- Left column: slider bands + toggles ---

  def _draw_left_column(self, rect: rl.Rectangle):
    params = self._controller._params
    no_uploads = params.get_bool("NoUploads")
    disable_onroad = params.get_bool("DisableOnroadUploads")

    # Slider bands take fixed proportion of height (40%), toggles take rest (60%)
    band_zone_h = rect.height * 0.40
    toggle_zone_h = rect.height - band_zone_h - self.BAND_GAP

    # Two slider bands split the band zone: display (4 sliders) gets 55%, power (2 sliders) gets 45%
    display_h = band_zone_h * 0.55
    power_h = band_zone_h * 0.45 - self.BAND_GAP

    self._draw_slider_band(rl.Rectangle(rect.x, rect.y, rect.width, display_h), tr("Display"), self._display_band)
    self._draw_slider_band(rl.Rectangle(rect.x, rect.y + display_h + self.BAND_GAP, rect.width, power_h), tr("Power"), self._power_band)

    # Toggles below
    toggle_y = rect.y + band_zone_h + self.BAND_GAP
    toggles = [
      ("StandbyMode", tr("Standby Mode"), params.get_bool("StandbyMode"), True),
      ("IncreaseThermalLimits", tr("Raise Thermal Limits"), params.get_bool("IncreaseThermalLimits"), True),
      ("NoUploads", tr("Disable Uploads"), no_uploads, True),
      ("UseKonikServer", tr("Use Konik Server"), self._controller._get_konik_state(), True),
      ("DisableOnroadUploads", tr("No Onroad Uploads"), disable_onroad, not no_uploads),
      ("NoLogging", tr("Disable Logging"), params.get_bool("NoLogging"), True),
      ("HigherBitrate", tr("HD Recording"), params.get_bool("HigherBitrate"), not disable_onroad and not no_uploads),
      ("DebugMode", tr("Debug Mode"), params.get_bool("DebugMode"), True),
    ]
    toggle_row_h = toggle_zone_h / len(toggles)
    for i, (tid, title, val, enabled) in enumerate(toggles):
      slot_y = toggle_y + i * toggle_row_h
      inner = rl.Rectangle(rect.x, slot_y + self.ROW_PAD, rect.width, toggle_row_h - 2 * self.ROW_PAD)
      self._render_toggle_pill(inner, tid, title, val, enabled)

  def _draw_slider_band(self, rect, label, slider_keys):
    """Draws a labeled card containing vertical sliders arranged horizontally."""
    # Card background
    rl.draw_rectangle_rounded(rect, 0.15, 16, rl.Color(28, 30, 36, 255))
    rl.draw_rectangle_rounded_lines_ex(rect, 0.15, 16, 1, rl.Color(255, 255, 255, 15))

    # Section label at top-left
    label_h = 18
    gui_label(rl.Rectangle(rect.x + 12, rect.y + 4, rect.width * 0.3, label_h), label, 16, AetherListColors.MUTED, FontWeight.MEDIUM)

    # Slider zone
    slider_top = rect.y + label_h + 6
    slider_h = rect.height - label_h - 10
    slider_area_w = rect.width - 16
    n = len(slider_keys)
    gap = 8
    col_w = (slider_area_w - (n - 1) * gap) / n
    start_x = rect.x + 8

    for i, key in enumerate(slider_keys):
      col_x = start_x + i * (col_w + gap)
      self._vsliders[key].render(rl.Rectangle(col_x, slider_top, col_w, slider_h))

  # --- Right column: action groups ---

  def _draw_actions_column(self, rect: rl.Rectangle):
    state = self._controller._get_force_drive_state()
    groups = [
      (tr("Storage & Logs"), [
        {"id": "Storage", "label": tr("Clear Data"), "danger": True},
        {"id": "ErrorLogs", "label": tr("Clear Logs"), "danger": True}]),
      (tr("System Backups"), [
        {"id": "CreateBackup", "label": tr("Create")},
        {"id": "RestoreBackup", "label": tr("Restore")},
        {"id": "DeleteBackup", "label": tr("Delete"), "danger": True}]),
      (tr("Toggle Backups"), [
        {"id": "CreateToggleBackup", "label": tr("Create")},
        {"id": "RestoreToggleBackup", "label": tr("Restore")},
        {"id": "DeleteToggleBackup", "label": tr("Delete"), "danger": True}]),
      (tr("Drive Mode"), [
        {"id": "DriveDefault", "label": tr("Auto"), "active": state == tr("Default")},
        {"id": "DriveOnroad", "label": tr("Onroad"), "active": state == tr("Onroad")},
        {"id": "DriveOffroad", "label": tr("Offroad"), "active": state == tr("Offroad")}]),
      (tr("Quick Actions"), [
        {"id": "FlashPanda", "label": tr("Flash Panda")},
        {"id": "ReportIssue", "label": tr("Report Issue")}]),
      (tr("Reset"), [
        {"id": "ResetDefaults", "label": tr("Reset Toggles"), "danger": True},
        {"id": "ResetStock", "label": tr("Stock OP"), "danger": True}]),
    ]
    row_h = rect.height / len(groups)
    for i, (title, actions) in enumerate(groups):
      slot_y = rect.y + i * row_h
      inner = rl.Rectangle(rect.x, slot_y + self.ROW_PAD, rect.width, row_h - 2 * self.ROW_PAD)
      self._render_action_group(inner, title, actions)

  # --- Primitives ---

  def _render_toggle_pill(self, rect, toggle_id, title, value, enabled=True):
    if enabled:
      self._toggle_rects[toggle_id] = rect
    mouse_pos = gui_app.last_mouse_event.pos
    hovered = rl.check_collision_point_rec(mouse_pos, rect)
    pressed = self._pressed_target == f"toggle:{toggle_id}"
    draw_toggle_pill(rect, value, enabled, title, tr("ON") if value else tr("OFF"), hovered, pressed)

  def _render_action_group(self, rect, title, actions):
    rl.draw_rectangle_rounded(rect, 0.3, 16, rl.Color(35, 35, 40, 255))
    btn_h = min(52, rect.height - 20)
    btn_y = rect.y + (rect.height - btn_h) / 2
    title_y = rect.y + (rect.height - 24) / 2
    title_w = rect.width * 0.30
    gui_label(rl.Rectangle(rect.x + 20, title_y, title_w, 24), title, 22, rl.WHITE, FontWeight.BOLD)
    btn_gap = 10
    available_w = rect.width - title_w - 48
    btn_w = (available_w - (len(actions) - 1) * btn_gap) / len(actions)
    start_x = rect.x + rect.width - available_w - 16
    mouse_pos = gui_app.last_mouse_event.pos
    for i, action in enumerate(actions):
      btn_rect = rl.Rectangle(start_x + i * (btn_w + btn_gap), btn_y, btn_w, btn_h)
      self._action_rects[action["id"]] = btn_rect
      hovered = rl.check_collision_point_rec(mouse_pos, btn_rect)
      pressed = self._pressed_target == f"action:{action['id']}"
      active = action.get("active", False)
      color = AetherListColors.PRIMARY if active else rl.Color(60, 60, 65, 255)
      if action.get("danger"):
        color = AetherListColors.DANGER
      if hovered: color = rl.Color(min(color.r + 20, 255), min(color.g + 20, 255), min(color.b + 20, 255), 255)
      if pressed: color = rl.Color(max(color.r - 20, 0), max(color.g - 20, 0), max(color.b - 20, 0), 255)
      rl.draw_rectangle_rounded(btn_rect, 0.4, 16, color)
      gui_label(btn_rect, action["label"], 22, rl.WHITE, FontWeight.BOLD, alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER)

class StarPilotSystemLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._keyboard = Keyboard(min_text_size=0)
    self._manager_view = SystemSettingsManagerView(self)

  def show_event(self):
    super().show_event()
    self._manager_view.show_event()

  def hide_event(self):
    super().hide_event()
    self._manager_view.hide_event()

  def _render(self, rect: rl.Rectangle):
    self._manager_view.render(rect)



  def handle_action(self, action_id: str):
    if action_id == "ScreenManagement":
      self._params.put_bool("ScreenManagement", not self._params.get_bool("ScreenManagement"))
    elif action_id == "StandbyMode":
      self._params.put_bool("StandbyMode", not self._params.get_bool("StandbyMode"))
    elif action_id == "DeviceManagement":
      self._params.put_bool("DeviceManagement", not self._params.get_bool("DeviceManagement"))
    elif action_id == "IncreaseThermalLimits":
      self._params.put_bool("IncreaseThermalLimits", not self._params.get_bool("IncreaseThermalLimits"))
    elif action_id == "UseKonikServer":
      self._on_konik_toggle(not self._get_konik_state())
    elif action_id == "NoLogging":
      self._params.put_bool("NoLogging", not self._params.get_bool("NoLogging"))
    elif action_id == "NoUploads":
      self._params.put_bool("NoUploads", not self._params.get_bool("NoUploads"))
    elif action_id == "DisableOnroadUploads":
      self._params.put_bool("DisableOnroadUploads", not self._params.get_bool("DisableOnroadUploads"))
    elif action_id == "HigherBitrate":
      self._on_higher_bitrate_toggle(not self._params.get_bool("HigherBitrate"))
    elif action_id == "Storage":
      self._on_delete_driving_data()
    elif action_id == "ErrorLogs":
      self._on_delete_error_logs()
    elif action_id == "CreateBackup":
      self._on_create_backup()
    elif action_id == "RestoreBackup":
      self._on_restore_backup()
    elif action_id == "DeleteBackup":
      self._on_delete_backup()
    elif action_id == "CreateToggleBackup":
      self._on_create_toggle_backup()
    elif action_id == "RestoreToggleBackup":
      self._on_restore_toggle_backup()
    elif action_id == "DeleteToggleBackup":
      self._on_delete_toggle_backup()
    elif action_id == "DebugMode":
      self._params.put_bool("DebugMode", not self._params.get_bool("DebugMode"))
    elif action_id == "DriveDefault":
      self._params.put_bool("ForceOffroad", False)
      self._params.put_bool("ForceOnroad", False)
    elif action_id == "DriveOnroad":
      self._params.put_bool("ForceOnroad", True)
      self._params.put_bool("ForceOffroad", False)
    elif action_id == "DriveOffroad":
      self._params.put_bool("ForceOffroad", True)
      self._params.put_bool("ForceOnroad", False)
    elif action_id == "FlashPanda":
      self._on_flash_panda()
    elif action_id == "ReportIssue":
      self._on_report_issue()
    elif action_id == "ResetDefaults":
      self._on_reset_defaults()
    elif action_id == "ResetStock":
      self._on_reset_stock()

  def _set_brightness(self, key, val):
    self._params.put_int(key, int(val))
    if key in ("ScreenBrightnessOnroad", "ScreenBrightness") and hasattr(HARDWARE, 'set_brightness'):
        HARDWARE.set_brightness(int(val))

  def _get_konik_state(self):
    if Path("/data/not_vetted").exists():
      return True
    return self._params.get_bool("UseKonikServer")

  def _on_konik_toggle(self, state):
    self._params.put_bool("UseKonikServer", state)
    cache_path = Path("/cache/use_konik")
    if state:
      cache_path.parent.mkdir(parents=True, exist_ok=True)
      cache_path.touch()
    else:
      if cache_path.exists():
        cache_path.unlink()
    if ui_state.started:
      gui_app.push_widget(
        ConfirmDialog(
          tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), on_close=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None
        )
      )

  def _on_higher_bitrate_toggle(self, state):
    self._params.put_bool("HigherBitrate", state)
    cache_path = Path("/cache/use_HD")
    if state:
      cache_path.parent.mkdir(parents=True, exist_ok=True)
      cache_path.touch()
    else:
      if cache_path.exists():
        cache_path.unlink()
    if ui_state.started:
      gui_app.push_widget(
        ConfirmDialog(
          tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), on_close=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None
        )
      )

  def _get_storage(self):
    paths = ["/data/media/0/osm/offline", "/data/media/0/realdata", "/data/backups"]
    total = 0
    for p in paths:
      pp = Path(p)
      if pp.exists():
        total += sum(f.stat().st_size for f in pp.rglob('*') if f.is_file())
    mb = total / (1024 * 1024)
    if mb > 1024:
      return f"{(mb / 1024):.2f} GB"
    return f"{mb:.2f} MB"

  def _on_delete_driving_data(self):
    def _do_delete(res):
      if res == DialogResult.CONFIRM:
        def _task():
          drive_paths = ["/data/media/0/realdata/", "/data/media/0/realdata_HD/", "/data/media/0/realdata_konik/"]
          for path in drive_paths:
            p = Path(path)
            if p.exists():
              for entry in p.iterdir():
                if entry.is_dir():
                  shutil.rmtree(entry, ignore_errors=True)
        threading.Thread(target=_task, daemon=True).start()
        gui_app.push_widget(alert_dialog(tr("Driving data deletion started.")))
    gui_app.push_widget(ConfirmDialog(tr("Delete all driving data and footage?"), tr("Delete"), on_close=_do_delete))

  def _on_delete_error_logs(self):
    def _do_delete(res):
      if res == DialogResult.CONFIRM:
        shutil.rmtree("/data/error_logs", ignore_errors=True)
        os.makedirs("/data/error_logs", exist_ok=True)
        gui_app.push_widget(alert_dialog(tr("Error logs deleted.")))
    gui_app.push_widget(ConfirmDialog(tr("Delete all error logs?"), tr("Delete"), on_close=_do_delete))

  def _get_backups(self, folder="backups"):
    b_dir = Path(f"/data/{folder}")
    if not b_dir.exists():
      return []
    if folder == "backups":
      return [f.name for f in b_dir.glob("*.tar.zst") if "in_progress" not in f.name]
    return [d.name for d in b_dir.iterdir() if d.is_dir() and "in_progress" not in d.name]

  def _on_create_backup(self):
    def on_name(res, name):
      if res == DialogResult.CONFIRM:
        safe_name = name.replace(" ", "_") if name else f"backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        backup_path = f"/data/backups/{safe_name}.tar.zst"
        if Path(backup_path).exists():
          gui_app.push_widget(alert_dialog(tr("A backup with this name already exists.")))
          return
        gui_app.push_widget(alert_dialog(tr("Backup creation started.")))
        def _task():
          os.makedirs("/data/backups", exist_ok=True)
          subprocess.run(["tar", "--use-compress-program=zstd", "-cf", backup_path, "/data/openpilot"])
        threading.Thread(target=_task, daemon=True).start()
    self._keyboard.reset(min_text_size=0)
    self._keyboard.set_title(tr("Name your backup"), "")
    self._keyboard.set_text("")
    self._keyboard.set_callback(lambda result: on_name(result, self._keyboard.text))
    gui_app.push_widget(self._keyboard)

  def _on_restore_backup(self):
    backups = self._get_backups("backups")
    if not backups:
      gui_app.push_widget(alert_dialog(tr("No backups found.")))
      return
    dialog = MultiOptionDialog(tr("Select Backup"), backups)
    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        gui_app.push_widget(alert_dialog(tr("Restoring... device will reboot.")))
        def _task():
          subprocess.run(["rm", "-rf", "/data/openpilot/*"])
          subprocess.run(["tar", "--use-compress-program=zstd", "-xf", f"/data/backups/{dialog.selection}", "-C", "/"])
          os.system("reboot")
        threading.Thread(target=_task, daemon=True).start()
    gui_app.push_widget(dialog, callback=_on_select)

  def _on_delete_backup(self):
    backups = self._get_backups("backups")
    if not backups:
      gui_app.push_widget(alert_dialog(tr("No backups found.")))
      return
    dialog = MultiOptionDialog(tr("Delete Backup"), backups)
    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        os.remove(f"/data/backups/{dialog.selection}")
    gui_app.push_widget(dialog, callback=_on_select)

  def _on_create_toggle_backup(self):
    def on_name(res, name):
      if res == DialogResult.CONFIRM:
        safe_name = name.replace(" ", "_") if name else f"toggle_backup_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
        backup_path = Path(f"/data/toggle_backups/{safe_name}")
        if backup_path.exists():
          gui_app.push_widget(alert_dialog(tr("A toggle backup with this name already exists.")))
          return
        os.makedirs(backup_path, exist_ok=True)
        shutil.copytree("/data/params/d", str(backup_path), dirs_exist_ok=True)
        gui_app.push_widget(alert_dialog(tr("Toggle backup created.")))
    self._keyboard.reset(min_text_size=0)
    self._keyboard.set_title(tr("Name your toggle backup"), "")
    self._keyboard.set_text("")
    self._keyboard.set_callback(lambda result: on_name(result, self._keyboard.text))
    gui_app.push_widget(self._keyboard)

  def _on_restore_toggle_backup(self):
    backups = self._get_backups("toggle_backups")
    if not backups:
      gui_app.push_widget(alert_dialog(tr("No toggle backups found.")))
      return
    dialog = MultiOptionDialog(tr("Select Toggle Backup"), backups)
    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        def on_confirm(r2):
          if r2 == DialogResult.CONFIRM:
            src = Path(f"/data/toggle_backups/{dialog.selection}")
            params_dir = Path("/data/params/d")
            for old_key, new_key in LEGACY_STARPILOT_PARAM_RENAMES.items():
              if (src / old_key).exists():
                (params_dir / new_key).unlink(missing_ok=True)
            shutil.copytree(str(src), "/data/params/d", dirs_exist_ok=True)
            for old_key, new_key in LEGACY_STARPILOT_PARAM_RENAMES.items():
              old_path = params_dir / old_key
              new_path = params_dir / new_key
              if old_path.exists():
                old_path.replace(new_path)
            gui_app.push_widget(alert_dialog(tr("Toggles restored.")))
        gui_app.push_widget(ConfirmDialog(tr("This will overwrite your current toggles."), tr("Restore"), on_close=on_confirm))
    gui_app.push_widget(dialog, callback=_on_select)

  def _on_delete_toggle_backup(self):
    backups = self._get_backups("toggle_backups")
    if not backups:
      gui_app.push_widget(alert_dialog(tr("No toggle backups found.")))
      return
    dialog = MultiOptionDialog(tr("Delete Toggle Backup"), backups)
    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        shutil.rmtree(f"/data/toggle_backups/{dialog.selection}", ignore_errors=True)
    gui_app.push_widget(dialog, callback=_on_select)

  def _get_force_drive_state(self):
    if self._params.get_bool("ForceOnroad"):
      return tr("Onroad")
    if self._params.get_bool("ForceOffroad"):
      return tr("Offroad")
    return tr("Default")

  def _on_flash_panda(self):
    def _do_flash(res):
      if res == DialogResult.CONFIRM:
        self._params_memory.put_bool("FlashPanda", True)
        gui_app.push_widget(alert_dialog(tr("Panda flashing started. Device will reboot when finished.")))
    gui_app.push_widget(ConfirmDialog(tr("Flash Panda firmware?"), tr("Flash"), callback=_do_flash))

  def _on_report_issue(self):
    def on_category(res):
      if res != DialogResult.CONFIRM or not dialog.selection:
        return
      discord_user = self._params.get("DiscordUsername", encoding='utf-8') or ""
      def on_discord(res2, username):
        if res2 == DialogResult.CONFIRM and username:
          self._params.put("DiscordUsername", username)
          report = json.dumps({"DiscordUser": username, "Issue": dialog.selection})
          self._params_memory.put("IssueReported", report)
          gui_app.push_widget(alert_dialog(tr("Issue reported. Thank you!")))
      self._keyboard.reset(min_text_size=1)
      self._keyboard.set_title(tr("Discord Username"), "")
      self._keyboard.set_text(discord_user or "")
      self._keyboard.set_callback(lambda result: on_discord(result, self._keyboard.text))
      gui_app.push_widget(self._keyboard)
    dialog = MultiOptionDialog(tr("Select Issue"), REPORT_CATEGORIES, callback=on_category)
    gui_app.push_widget(dialog)

  def _on_reset_defaults(self):
    def _do_reset(res):
      if res == DialogResult.CONFIRM:
        all_keys = self._params.all_keys()
        for k in all_keys:
          if k in EXCLUDED_KEYS:
            continue
          default = self._params.get_default_value(k)
          if default is not None:
            self._params.put(k, default)
        gui_app.push_widget(alert_dialog(tr("Toggles reset to defaults.")))
    gui_app.push_widget(ConfirmDialog(tr("Reset all toggles to defaults?"), tr("Reset"), callback=_do_reset))

  def _on_reset_stock(self):
    def _do_reset(res):
      if res == DialogResult.CONFIRM:
        all_keys = self._params.all_keys()
        for k in all_keys:
          if k in EXCLUDED_KEYS:
            continue
          stock = self._params.get_stock_value(k)
          if stock is not None:
            self._params.put(k, stock)
        gui_app.push_widget(alert_dialog(tr("Toggles reset to stock openpilot.")))
    gui_app.push_widget(ConfirmDialog(tr("Reset all toggles to stock openpilot?"), tr("Reset"), callback=_do_reset))
