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
  TileGrid,
  ToggleTile,
  RadioTileGroup,
  build_list_panel_frame,
  draw_list_panel_shell,
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
  Left column: massive equalizer sliders and smart home style dashboard toggles.
  Right column: categorized thick action cards."""

  def __init__(self, controller: "StarPilotSystemLayout"):
    super().__init__()
    self._controller = controller
    self._pressed_target: str | None = None
    self._action_rects: dict[str, rl.Rectangle] = {}

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

    self._display_band = ["ScreenBrightness", "ScreenBrightnessOnroad", "ScreenTimeout", "ScreenTimeoutOnroad"]
    self._power_band = ["DeviceShutdown", "LowVoltageShutdown"]

    self._toggles = [
      ToggleTile("Standby Mode", lambda: self._controller._params.get_bool("StandbyMode"), lambda v: self._controller._params.put_bool("StandbyMode", v)),
      ToggleTile("Raise Temp", lambda: self._controller._params.get_bool("IncreaseThermalLimits"), lambda v: self._controller._params.put_bool("IncreaseThermalLimits", v)),
      ToggleTile("No Uploads", lambda: self._controller._params.get_bool("NoUploads"), lambda v: self._controller._params.put_bool("NoUploads", v)),
      ToggleTile("Konik Server", lambda: self._controller._get_konik_state(), lambda v: self._controller._on_konik_toggle(v)),
      ToggleTile("No Onroad", lambda: self._controller._params.get_bool("DisableOnroadUploads"), lambda v: self._controller._params.put_bool("DisableOnroadUploads", v), is_enabled=lambda: not self._controller._params.get_bool("NoUploads")),
      ToggleTile("No Logging", lambda: self._controller._params.get_bool("NoLogging"), lambda v: self._controller._params.put_bool("NoLogging", v)),
      ToggleTile("HD Record", lambda: self._controller._params.get_bool("HigherBitrate"), lambda v: self._controller._on_higher_bitrate_toggle(v), is_enabled=lambda: not self._controller._params.get_bool("DisableOnroadUploads") and not self._controller._params.get_bool("NoUploads")),
      ToggleTile("Debug Mode", lambda: self._controller._params.get_bool("DebugMode"), lambda v: self._controller._params.put_bool("DebugMode", v)),
    ]
    self._toggle_grid = TileGrid(columns=4, padding=16, uniform_width=True)
    for t in self._toggles:
      self._toggle_grid.add_tile(t)

    self._drive_mode_radio = RadioTileGroup("", [tr("Auto"), tr("Onroad"), tr("Offroad")], self._get_drive_mode_index(), self._on_drive_mode_change)

  def _get_drive_mode_index(self):
    state = self._controller._get_force_drive_state()
    if state == tr("Default"): return 0
    if state == tr("Onroad"): return 1
    if state == tr("Offroad"): return 2
    return 0

  def _on_drive_mode_change(self, idx):
    if idx == 0:
        self._controller.handle_action("DriveDefault")
    elif idx == 1:
        self._controller.handle_action("DriveOnroad")
    elif idx == 2:
        self._controller.handle_action("DriveOffroad")

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
        break
    
    for t in self._toggles:
      t._handle_mouse_press(mouse_pos)
    self._drive_mode_radio._handle_mouse_press(mouse_pos)
    for slider in self._vsliders.values():
      slider._handle_mouse_press(mouse_pos)

  def _handle_mouse_event(self, mouse_event: MouseEvent):
    for slider in self._vsliders.values():
      slider._handle_mouse_event(mouse_event)
    for t in self._toggles:
      t._handle_mouse_event(mouse_event)

  def _handle_mouse_release(self, mouse_pos: MousePos):
    target = self._pressed_target
    self._pressed_target = None
    if target and target.startswith("action:"):
      action_id = target.split(":", 1)[1]
      rect = self._action_rects.get(action_id)
      if rect and rl.check_collision_point_rec(mouse_pos, rect):
        self._controller.handle_action(action_id)

    for t in self._toggles:
      t._handle_mouse_release(mouse_pos)
    self._drive_mode_radio._handle_mouse_release(mouse_pos)
    for slider in self._vsliders.values():
      slider._handle_mouse_release(mouse_pos)

  # --- Main render ---

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    self._action_rects.clear()

    frame = build_list_panel_frame(rect)
    draw_list_panel_shell(frame)

    hdr = frame.header
    gui_label(rl.Rectangle(hdr.x, hdr.y + 4, hdr.width * 0.55, 40), tr("System Settings"), 40, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(hdr.x, hdr.y + 48, hdr.width * 0.7, 36), tr("Manage device behavior, power, and storage."), 24, AetherListColors.SUBTEXT, FontWeight.NORMAL)

    actual_header_h = 100
    content_y = hdr.y + actual_header_h
    content_h = (frame.shell.y + frame.shell.height) - content_y - AETHER_LIST_METRICS.panel_padding_bottom
    content_w = frame.scroll.width
    section_gap = 24

    self._sync_slider_values()
    if content_w < 1260:
      left_ratio = 0.56 if content_w < 1080 else 0.52
      col_w = (content_w - section_gap) / 2
      left_col = rl.Rectangle(frame.scroll.x, content_y, col_w * left_ratio + col_w * 0.44, content_h)
      right_col_x = left_col.x + left_col.width + section_gap
      right_col = rl.Rectangle(right_col_x, content_y, frame.scroll.x + content_w - right_col_x, content_h)
      self._draw_left_column(left_col)
      self._draw_right_column(right_col, compact=True)
    else:
      col_w = (content_w - section_gap) / 2
      left_col = rl.Rectangle(frame.scroll.x, content_y, col_w, content_h)
      right_col = rl.Rectangle(frame.scroll.x + col_w + section_gap, content_y, col_w, content_h)
      self._draw_left_column(left_col)
      self._draw_right_column(right_col, compact=False)

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
    band_zone_h = rect.height * (0.52 if rect.width < 760 else 0.45)
    toggle_zone_h = rect.height - band_zone_h - 16

    display_h = band_zone_h * 0.58
    power_h = band_zone_h - display_h - 16

    self._draw_slider_band(rl.Rectangle(rect.x, rect.y, rect.width, display_h), tr("Display"), self._display_band)
    self._draw_slider_band(rl.Rectangle(rect.x, rect.y + display_h + 16, rect.width, power_h), tr("Power"), self._power_band)

    toggle_y = rect.y + band_zone_h + 16
    toggle_rect = rl.Rectangle(rect.x, toggle_y, rect.width, toggle_zone_h)
    self._toggle_grid.render(toggle_rect)

  def _draw_slider_band(self, rect, label, slider_keys):
    rl.draw_rectangle_rounded(rect, 0.15, 16, rl.Color(28, 30, 36, 255))
    rl.draw_rectangle_rounded_lines_ex(rect, 0.15, 16, 1, rl.Color(255, 255, 255, 15))

    label_h = 24
    gui_label(rl.Rectangle(rect.x + 16, rect.y + 12, rect.width * 0.3, label_h), label, 20, AetherListColors.SUBTEXT, FontWeight.BOLD)

    slider_top = rect.y + label_h + 20
    slider_h = rect.height - label_h - 32
    slider_area_w = rect.width - 32
    gap = 12

    n = len(slider_keys)
    cols = max(1, min(n, int((slider_area_w + gap) / (120 + gap))))
    rows = (n + cols - 1) // cols
    col_w = (slider_area_w - (cols - 1) * gap) / cols
    row_gap = 12
    total_slider_h = max(0, slider_h - row_gap * max(0, rows - 1))
    cell_h = total_slider_h / max(1, rows)
    content_w = min(n, cols) * col_w + max(0, min(n, cols) - 1) * gap
    start_x = rect.x + 16 + (slider_area_w - content_w) / 2

    for i, key in enumerate(slider_keys):
      row = i // cols
      col = i % cols
      col_x = start_x + col * (col_w + gap)
      col_y = slider_top + row * (cell_h + row_gap)
      self._vsliders[key].render(rl.Rectangle(col_x, col_y, col_w, cell_h))

  # --- Right column: action cards ---

  def _draw_right_column(self, rect: rl.Rectangle, compact: bool = False):
    gap = 16
    total_h = rect.height - 2 * gap
    if compact:
      h1 = total_h * 0.26
      h2 = total_h * 0.42
      h3 = total_h * 0.32
    else:
      h1 = total_h * 0.30
      h2 = total_h * 0.40
      h3 = total_h * 0.30

    card1_rect = rl.Rectangle(rect.x, rect.y, rect.width, h1)
    card2_rect = rl.Rectangle(rect.x, rect.y + h1 + gap, rect.width, h2)
    card3_rect = rl.Rectangle(rect.x, rect.y + h1 + h2 + 2*gap, rect.width, h3)

    self._draw_card_background(card1_rect, tr("Drive & Actions"))
    self._draw_card1_content(card1_rect, compact=compact)

    self._draw_card_background(card2_rect, tr("Backups Management"))
    self._draw_card2_content(card2_rect, compact=compact)

    self._draw_card_background(card3_rect, tr("Maintenance (Caution)"))
    self._draw_card3_content(card3_rect, compact=compact)

  def _draw_card_background(self, rect, title):
    rl.draw_rectangle_rounded(rect, 0.15, 16, rl.Color(28, 30, 36, 255))
    rl.draw_rectangle_rounded_lines_ex(rect, 0.15, 16, 1, rl.Color(255, 255, 255, 15))
    gui_label(rl.Rectangle(rect.x + 16, rect.y + 12, rect.width, 24), title, 20, AetherListColors.SUBTEXT, FontWeight.BOLD)

  def _draw_action_button(self, rect: rl.Rectangle, action_id: str, label: str, danger: bool = False, active: bool = False):
    self._action_rects[action_id] = rect
    mouse_pos = gui_app.last_mouse_event.pos
    hovered = rl.check_collision_point_rec(mouse_pos, rect)
    pressed = self._pressed_target == f"action:{action_id}"
    
    color = AetherListColors.PRIMARY if active else rl.Color(45, 48, 55, 255)
    border_color = rl.Color(255, 255, 255, 20)
    
    if danger:
        color = rl.Color(90, 35, 40, 255)
        border_color = rl.Color(173, 78, 90, 180)
        
    if hovered: color = rl.Color(min(color.r + 20, 255), min(color.g + 20, 255), min(color.b + 20, 255), 255)
    if pressed: color = rl.Color(max(color.r - 20, 0), max(color.g - 20, 0), max(color.b - 20, 0), 255)
    
    rl.draw_rectangle_rounded(rect, 0.25, 16, color)
    rl.draw_rectangle_rounded_lines_ex(rect, 0.25, 16, 1, border_color)
    text_color = rl.Color(255, 200, 200, 255) if danger else rl.WHITE
    font_size = max(14, min(22, int(min(rect.width, rect.height) * 0.24)))
    gui_label(rect, label, font_size, text_color, FontWeight.BOLD, alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER)

  def _draw_button_row(self, rect: rl.Rectangle, buttons: list[tuple[str, str, bool]], columns: int, *, gap: int = 16, button_h: int | None = None):
    cols = max(1, min(columns, len(buttons)))
    rows = (len(buttons) + cols - 1) // cols
    resolved_button_h = button_h if button_h is not None else max(34, min(64, int((rect.height - gap * max(0, rows - 1)) / max(1, rows))))
    button_w = (rect.width - gap * max(0, cols - 1)) / cols
    for idx, (action_id, label, danger) in enumerate(buttons):
      row = idx // cols
      col = idx % cols
      bx = rect.x + col * (button_w + gap)
      by = rect.y + row * (resolved_button_h + gap)
      self._draw_action_button(rl.Rectangle(bx, by, button_w, resolved_button_h), action_id, label, danger=danger)
    return rows * resolved_button_h + gap * max(0, rows - 1)

  def _draw_card1_content(self, rect, compact: bool = False):
    content_y = rect.y + 44
    content_h = rect.height - 44 - 16
    if compact:
      stacked = rect.width < 760
      units = 3 if stacked else 2
      gap = 10
      control_h = max(30, min(64, int((content_h - gap * (units + 1)) / units)))
    else:
      ACTION_BTN_H = 64
      gap = max(12, (content_h - (ACTION_BTN_H * 2)) / 3)
      control_h = ACTION_BTN_H

    ry = content_y + gap
    radio_rect = rl.Rectangle(rect.x + 16, ry, rect.width - 32, control_h)
    self._drive_mode_radio.current_index = self._get_drive_mode_index()
    self._drive_mode_radio.render(radio_rect)
    
    by = ry + control_h + gap
    btn_w = (rect.width - 32 - 16) / 2
    if compact and rect.width < 760:
      self._draw_button_row(rl.Rectangle(rect.x + 16, by, rect.width - 32, control_h * 2 + gap), [("FlashPanda", tr("Flash Panda"), False), ("ReportIssue", tr("Report Issue"), False)], 1, gap=gap, button_h=control_h)
    else:
      self._draw_action_button(rl.Rectangle(rect.x + 16, by, btn_w, control_h), "FlashPanda", tr("Flash Panda"))
      self._draw_action_button(rl.Rectangle(rect.x + 16 + btn_w + 16, by, btn_w, control_h), "ReportIssue", tr("Report Issue"))

  def _draw_card2_content(self, rect, compact: bool = False):
    content_y = rect.y + 44
    if compact:
      gap = 8
      label_h = 18
      cols = 2 if rect.width >= 560 else 1
      rows_per_group = (3 + cols - 1) // cols
      total_rows = rows_per_group * 2
      available_h = rect.height - 44 - 16 - label_h * 2 - gap * 5
      button_h = max(28, min(56, int(available_h / max(1, total_rows))))
    else:
      ACTION_BTN_H = 64
      gap = max(12, (rect.height - 44 - 16 - (ACTION_BTN_H * 2)) / 3)
      label_h = ACTION_BTN_H
      cols = 3
      rows_per_group = 1
      button_h = ACTION_BTN_H
    
    sys_y = content_y + gap
    label_w = 110 if rect.width >= 760 and not compact else 92
    label_size = 22 if not compact else 18
    gui_label(rl.Rectangle(rect.x + 16, sys_y, label_w, label_h), tr("System:"), label_size, rl.WHITE, FontWeight.SEMI_BOLD)
    start_x = rect.x + label_w + 20
    row_w = rect.width - (start_x - rect.x) - 16
    system_h = self._draw_button_row(rl.Rectangle(start_x, sys_y, row_w, rows_per_group * button_h + gap * max(0, rows_per_group - 1)), [("CreateBackup", tr("Create"), False), ("RestoreBackup", tr("Restore"), False), ("DeleteBackup", tr("Delete"), True)], cols, gap=gap, button_h=button_h)

    tog_y = sys_y + max(label_h, system_h) + gap
    gui_label(rl.Rectangle(rect.x + 16, tog_y, label_w, label_h), tr("Toggles:"), label_size, rl.WHITE, FontWeight.SEMI_BOLD)
    self._draw_button_row(rl.Rectangle(start_x, tog_y, row_w, rows_per_group * button_h + gap * max(0, rows_per_group - 1)), [("CreateToggleBackup", tr("Create"), False), ("RestoreToggleBackup", tr("Restore"), False), ("DeleteToggleBackup", tr("Delete"), True)], cols, gap=gap, button_h=button_h)

  def _draw_card3_content(self, rect, compact: bool = False):
    content_y = rect.y + 44
    if compact:
      gap = 10
      columns = 2 if rect.width >= 520 else 1
      rows_per_group = (2 + columns - 1) // columns
      available_h = rect.height - 44 - 16 - gap * 3
      button_h = max(30, min(56, int(available_h / max(2, rows_per_group * 2))))
    else:
      ACTION_BTN_H = 64
      gap = max(12, (rect.height - 44 - 16 - (ACTION_BTN_H * 2)) / 3)
      columns = 2
      rows_per_group = 1
      button_h = ACTION_BTN_H
    
    sys_y = content_y + gap
    group_h = rows_per_group * button_h + gap * max(0, rows_per_group - 1)
    system_h = self._draw_button_row(rl.Rectangle(rect.x + 16, sys_y, rect.width - 32, group_h), [("Storage", tr("Clear Data"), True), ("ErrorLogs", tr("Clear Logs"), True)], columns, gap=gap, button_h=button_h)
    
    tog_y = sys_y + system_h + gap
    self._draw_button_row(rl.Rectangle(rect.x + 16, tog_y, rect.width - 32, group_h), [("ResetDefaults", tr("Reset Toggles"), True), ("ResetStock", tr("Stock OP"), True)], columns, gap=gap, button_h=button_h)

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
    elif action_id == "DeviceManagement":
      self._params.put_bool("DeviceManagement", not self._params.get_bool("DeviceManagement"))
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
          tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), callback=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None
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
          tr("Reboot required. Reboot now?"), tr("Reboot"), tr("Cancel"), callback=lambda res: HARDWARE.reboot() if res == DialogResult.CONFIRM else None
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
    gui_app.push_widget(ConfirmDialog(tr("Delete all driving data and footage?"), tr("Delete"), callback=_do_delete))

  def _on_delete_error_logs(self):
    def _do_delete(res):
      if res == DialogResult.CONFIRM:
        shutil.rmtree("/data/error_logs", ignore_errors=True)
        os.makedirs("/data/error_logs", exist_ok=True)
        gui_app.push_widget(alert_dialog(tr("Error logs deleted.")))
    gui_app.push_widget(ConfirmDialog(tr("Delete all error logs?"), tr("Delete"), callback=_do_delete))

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

    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        gui_app.push_widget(alert_dialog(tr("Restoring... device will reboot.")))
        def _task():
          subprocess.run(["rm", "-rf", "/data/openpilot/*"])
          subprocess.run(["tar", "--use-compress-program=zstd", "-xf", f"/data/backups/{dialog.selection}", "-C", "/"])
          os.system("reboot")
        threading.Thread(target=_task, daemon=True).start()

    dialog = MultiOptionDialog(tr("Select Backup"), backups, callback=_on_select)
    gui_app.push_widget(dialog)

  def _on_delete_backup(self):
    backups = self._get_backups("backups")
    if not backups:
      gui_app.push_widget(alert_dialog(tr("No backups found.")))
      return

    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        os.remove(f"/data/backups/{dialog.selection}")

    dialog = MultiOptionDialog(tr("Delete Backup"), backups, callback=_on_select)
    gui_app.push_widget(dialog)

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
        gui_app.push_widget(ConfirmDialog(tr("This will overwrite your current toggles."), tr("Restore"), callback=on_confirm))

    dialog = MultiOptionDialog(tr("Select Toggle Backup"), backups, callback=_on_select)
    gui_app.push_widget(dialog)

  def _on_delete_toggle_backup(self):
    backups = self._get_backups("toggle_backups")
    if not backups:
      gui_app.push_widget(alert_dialog(tr("No toggle backups found.")))
      return

    def _on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        shutil.rmtree(f"/data/toggle_backups/{dialog.selection}", ignore_errors=True)

    dialog = MultiOptionDialog(tr("Delete Toggle Backup"), backups, callback=_on_select)
    gui_app.push_widget(dialog)

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
