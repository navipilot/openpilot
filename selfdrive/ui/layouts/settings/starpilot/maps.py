from __future__ import annotations

import shutil
import threading
from dataclasses import dataclass
from pathlib import Path

from cereal import log, messaging
import pyray as rl

from openpilot.common.params import Params
from openpilot.selfdrive.ui.ui_state import device, ui_state
from openpilot.system.ui.lib.application import FontWeight, MouseEvent, MousePos, gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.lib.scroll_panel2 import GuiScrollPanel2
from openpilot.system.ui.lib.text_measure import measure_text_cached
from openpilot.system.ui.widgets import DialogResult, Widget
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.label import gui_label, gui_text_box
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog

from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import (
  AetherButton,
  AetherChip,
  AetherListColors,
  AetherSegmentedControl,
  AetherScrollbar,
  build_list_panel_frame,
  draw_action_pill,
  draw_busy_ring,
  draw_list_panel_shell,
  draw_selection_list_row,
  draw_list_scroll_fades,
  draw_soft_card,
)
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.starpilot.common.maps_catalog import (
  MAPS_CATALOG,
  MAP_TOKEN_LABELS,
  MAP_SCHEDULE_LABELS,
  get_selected_map_entries,
  normalize_schedule_value,
  sanitize_selected_locations_csv,
  schedule_label,
)
from openpilot.starpilot.common.maps_selection import COUNTRY_PREFIX, STATE_PREFIX


NetworkType = log.DeviceState.NetworkType

NETWORK_TYPE_LABELS = {
  NetworkType.none: tr_noop("Offline"),
  NetworkType.wifi: tr_noop("Wi-Fi"),
  NetworkType.ethernet: tr_noop("Ethernet"),
  NetworkType.cell2G: tr_noop("2G"),
  NetworkType.cell3G: tr_noop("3G"),
  NetworkType.cell4G: tr_noop("LTE"),
  NetworkType.cell5G: tr_noop("5G"),
}

OFFLINE_MAPS_PATH = Path("/data/media/0/osm/offline")
CANCEL_REQUEST_TIMEOUT = 3.0
HEADER_TOP_OFFSET = 10
HEADER_TITLE_HEIGHT = 40
HEADER_SUBTITLE_HEIGHT = 28
HEADER_BOTTOM_GAP = 12
SECTION_CARD_GAP = 18
BROWSER_TOOLBAR_HEIGHT = 78
BROWSER_SECTION_HEADER_HEIGHT = 44
BROWSER_INSET = 16
BROWSER_TAB_GAP = 12
BROWSER_CONTEXT_TAB_GAP = 10
BROWSER_CONTEXT_TAB_HEIGHT = 54
BROWSER_CONTEXT_MIN_TAB_WIDTH = 132
BROWSER_CONTEXT_MAX_TAB_WIDTH = 228
BROWSER_REGION_ROW_HEIGHT = 94
BROWSER_EMPTY_STATE_HEIGHT = 140

COUNTRIES_SECTION = next(section for section in MAPS_CATALOG if section["key"] == "countries")
STATES_SECTION = next(section for section in MAPS_CATALOG if section["key"] == "states")
US_COUNTRY_TOKEN = f"{COUNTRY_PREFIX}US"


def _format_mb(size_bytes: int) -> str:
  mb = size_bytes / (1024 * 1024)
  if mb >= 1024:
    return f"{mb / 1024:.2f} GB"
  return f"{mb:.2f} MB"


def _format_elapsed_ms(elapsed_ms: int) -> str:
  if elapsed_ms <= 0:
    return tr("Calculating...")
  total_seconds = elapsed_ms // 1000
  hours = total_seconds // 3600
  minutes = (total_seconds % 3600) // 60
  seconds = total_seconds % 60
  if hours > 0:
    return f"{hours:d}:{minutes:02d}:{seconds:02d}"
  return f"{minutes:d}:{seconds:02d}"


def _format_eta_ms(elapsed_ms: int, downloaded_files: int, total_files: int) -> str:
  if elapsed_ms <= 0 or downloaded_files <= 0 or total_files <= 0 or downloaded_files >= total_files:
    return tr("Calculating...")
  remaining_files = total_files - downloaded_files
  if remaining_files <= 0:
    return tr("Almost done")
  files_per_ms = downloaded_files / max(elapsed_ms, 1)
  if files_per_ms <= 0:
    return tr("Calculating...")
  remaining_ms = int(remaining_files / files_per_ms)
  return _format_elapsed_ms(remaining_ms)


def _selected_token_set(selected_raw: str | bytes | None) -> set[str]:
  normalized = sanitize_selected_locations_csv(selected_raw or "")
  tokens = {token for token in normalized.split(",") if token}
  return tokens


@dataclass(slots=True)
class MapsDownloadState:
  active: bool = False
  cancelled: bool = False
  total_files: int = 0
  downloaded_files: int = 0
  primary_location: str = ""
  location_count: int = 0
  percent: int = 0
  progress_text: str = ""


class MapStatusCard(Widget):
  def __init__(self, controller: "StarPilotMapsLayout"):
    super().__init__()
    self._controller = controller
    self._remove_rect = rl.Rectangle(0, 0, 0, 0)
    self._pressed_remove = False
    self._primary_button = self._child(controller._download_button)
    self._secondary_button = self._child(controller._schedule_button)

  def set_touch_valid_callback(self, touch_callback):
    super().set_touch_valid_callback(touch_callback)
    self._primary_button.set_touch_valid_callback(touch_callback)
    self._secondary_button.set_touch_valid_callback(touch_callback)

  def _handle_mouse_press(self, mouse_pos: MousePos):
    if not self._touch_valid():
      return
    if rl.check_collision_point_rec(mouse_pos, self._remove_rect) and self._controller._remove_enabled():
      self._pressed_remove = True

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._pressed_remove:
      self._pressed_remove = False
      if self._touch_valid() and rl.check_collision_point_rec(mouse_pos, self._remove_rect) and self._controller._remove_enabled():
        self._controller._on_remove()

  def _render(self, rect: rl.Rectangle):
    draw_soft_card(rect, rl.Color(255, 255, 255, 4), rl.Color(255, 255, 255, 16))

    inset = 20
    content_w = rect.width - inset * 2
    actions_w = max(248.0, min(304.0, rect.width * 0.22))
    left_w = max(360.0, min(520.0, content_w * 0.42))
    metrics_x = rect.x + inset + left_w + 24
    metrics_w = max(280.0, rect.x + rect.width - inset - actions_w - 24 - metrics_x)
    actions_x = rect.x + rect.width - inset - actions_w
    top_y = rect.y + 18

    gui_label(rl.Rectangle(rect.x + inset, top_y, left_w, 24), self._controller._progress_title(), 24, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_text_box(
      rl.Rectangle(rect.x + inset, top_y + 30, left_w, 38),
      self._controller._progress_body(),
      17,
      AetherListColors.SUBTEXT,
      font_weight=FontWeight.NORMAL,
      line_scale=0.94,
    )
    gui_label(rl.Rectangle(rect.x + inset, rect.y + rect.height - 26, left_w, 18), self._controller._status_footer_text(), 16, AetherListColors.MUTED, FontWeight.MEDIUM)

    metric_gap = 20
    metric_w = max(108.0, (metrics_w - metric_gap) / 2)
    metric_y = top_y + 2
    gui_label(rl.Rectangle(metrics_x, metric_y, metric_w, 18), tr("Storage"), 16, AetherListColors.MUTED, FontWeight.MEDIUM)
    gui_label(rl.Rectangle(metrics_x, metric_y + 16, metric_w, 24), self._controller._storage_text, 22, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(metrics_x + metric_w + metric_gap, metric_y, metric_w, 18), tr("Last Updated"), 16, AetherListColors.MUTED, FontWeight.MEDIUM)
    gui_label(rl.Rectangle(metrics_x + metric_w + metric_gap, metric_y + 16, metric_w, 24), self._controller._last_updated_text(), 20, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(metrics_x, metric_y + 38, metrics_w, 18), tr("Selected Regions"), 16, AetherListColors.MUTED, FontWeight.MEDIUM)
    gui_label(rl.Rectangle(metrics_x, metric_y + 56, metrics_w, 22), self._controller._selection_summary_title(), 20, AetherListColors.HEADER, FontWeight.SEMI_BOLD)

    button_h = 46
    button_gap = 8
    button_y = rect.y + 14
    self._primary_button.render(rl.Rectangle(actions_x, button_y, actions_w, button_h))
    self._secondary_button.render(rl.Rectangle(actions_x, button_y + button_h + button_gap, actions_w, button_h))

    action_w = 150
    action_h = 36
    self._remove_rect = rl.Rectangle(metrics_x + metrics_w - action_w, rect.y + rect.height - action_h - 14, action_w, action_h)
    enabled = self._controller._remove_enabled()
    draw_action_pill(
      self._remove_rect,
      tr("Remove Maps"),
      rl.Color(173, 78, 90, 26 if enabled else 12),
      rl.Color(173, 78, 90, 58 if enabled else 24),
      AetherListColors.HEADER if enabled else AetherListColors.MUTED,
      font_size=17,
    )

    if self._controller._download_state.active:
      center = rl.Vector2(rect.x + rect.width - 34, rect.y + 34)
      draw_busy_ring(center, rl.get_time() * 160, AetherListColors.PRIMARY, inner_radius=10, outer_radius=14, sweep=210, thickness=20)


class MapBrowserCard(Widget):
  def __init__(self, controller: "StarPilotMapsLayout"):
    super().__init__()
    self._controller = controller
    self._pressed_target: str | None = None
    self._selected_action_rects: dict[str, rl.Rectangle] = {}
    self._context_tab_rects: dict[str, rl.Rectangle] = {}
    self._region_row_rects: dict[str, rl.Rectangle] = {}

    self._source_selector = self._child(
      AetherSegmentedControl(
        [tr_noop("United States"), tr_noop("Countries")],
        lambda: 0 if self._controller._is_states_view() else 1,
        lambda index: self._controller._set_view_index(self._controller.VIEW_STATES if index == 0 else self._controller.VIEW_COUNTRIES),
        statuses=[
          lambda: self._controller._browser_source_primary(self._controller.VIEW_STATES),
          lambda: self._controller._browser_source_primary(self._controller.VIEW_COUNTRIES),
        ],
      )
    )
  def set_touch_valid_callback(self, touch_callback):
    super().set_touch_valid_callback(touch_callback)
    self._source_selector.set_touch_valid_callback(touch_callback)

  def _handle_mouse_press(self, mouse_pos: MousePos):
    if not self._touch_valid():
      return
    self._pressed_target = self._target_at(mouse_pos)

  def _handle_mouse_release(self, mouse_pos: MousePos):
    target = self._target_at(mouse_pos) if self._touch_valid() else None
    pressed_target = self._pressed_target
    self._pressed_target = None
    if pressed_target is not None and pressed_target == target:
      self._activate_target(pressed_target)

  def _handle_mouse_event(self, mouse_event: MouseEvent):
    if self._pressed_target is not None and self._target_at(mouse_event.pos) != self._pressed_target:
      self._pressed_target = None

  def _target_at(self, mouse_pos: MousePos) -> str | None:
    parent_rect = getattr(self, "_parent_rect", None)
    if parent_rect is not None and not rl.check_collision_point_rec(mouse_pos, parent_rect):
      return None
    for token, rect in self._selected_action_rects.items():
      if rl.check_collision_point_rec(mouse_pos, rect):
        return f"selected-remove:{token}"
    for group_key, rect in self._context_tab_rects.items():
      if rl.check_collision_point_rec(mouse_pos, rect):
        return f"group:{group_key}"
    for token, rect in self._region_row_rects.items():
      if rl.check_collision_point_rec(mouse_pos, rect):
        return f"region:{token}"
    return None

  def _activate_target(self, target: str):
    if target.startswith("selected-remove:"):
      self._controller._set_map_state(target.split(":", 1)[1], False)
      return
    if target.startswith("group:"):
      self._controller._set_active_group(target.split(":", 1)[1])
      return
    if target.startswith("region:"):
      self._controller._toggle_region(target.split(":", 1)[1])

  def _row_height(self, count: int, row_height: float) -> float:
    return 0.0 if count <= 0 else count * row_height

  def _context_tab_width(self, label: str) -> float:
    font = gui_app.font(FontWeight.MEDIUM)
    size = measure_text_cached(font, label, 22, spacing=1)
    return max(BROWSER_CONTEXT_MIN_TAB_WIDTH, min(BROWSER_CONTEXT_MAX_TAB_WIDTH, size.x + 54))

  def _measure_context_tabs_height(self, width: float) -> float:
    groups = self._controller._current_groups_with_full_us()
    if not groups:
      return 0.0

    rows = 1
    row_w = 0.0
    available_w = max(1.0, width)
    for group in groups:
      tab_w = self._context_tab_width(group["title"])
      next_w = tab_w if row_w <= 0 else row_w + BROWSER_CONTEXT_TAB_GAP + tab_w
      if next_w > available_w and row_w > 0:
        rows += 1
        row_w = tab_w
      else:
        row_w = next_w
    return rows * BROWSER_CONTEXT_TAB_HEIGHT + max(0, rows - 1) * BROWSER_CONTEXT_TAB_GAP

  def _render_context_tabs(self, rect: rl.Rectangle):
    groups = self._controller._current_groups_with_full_us()
    self._context_tab_rects.clear()
    if not groups:
      return

    mouse_pos = gui_app.last_mouse_event.pos
    x = rect.x
    y = rect.y
    available_w = max(1.0, rect.width)

    for group in groups:
      tab_w = self._context_tab_width(group["title"])
      if x > rect.x and (x - rect.x + tab_w) > available_w:
        x = rect.x
        y += BROWSER_CONTEXT_TAB_HEIGHT + BROWSER_CONTEXT_TAB_GAP

      tab_rect = rl.Rectangle(x, y, tab_w, BROWSER_CONTEXT_TAB_HEIGHT)
      self._context_tab_rects[group["key"]] = tab_rect
      current = self._controller._active_group_key() == group["key"]
      hovered = rl.check_collision_point_rec(mouse_pos, tab_rect)
      pressed = self._pressed_target == f"group:{group['key']}"

      fill = rl.Color(255, 255, 255, 12 if current else (8 if hovered else 4))
      border = rl.Color(255, 255, 255, 28 if current else 14)
      text_color = AetherListColors.HEADER if current else AetherListColors.SUBTEXT
      meta_color = AetherListColors.HEADER if current else AetherListColors.MUTED
      if pressed:
        fill = rl.Color(255, 255, 255, min(fill.a + 6, 24))

      draw_soft_card(tab_rect, fill, border)
      gui_label(
        rl.Rectangle(tab_rect.x + 16, tab_rect.y + 9, tab_rect.width - 32, 22),
        group["title"],
        22,
        text_color,
        FontWeight.MEDIUM,
      )
      gui_label(
        rl.Rectangle(tab_rect.x + 16, tab_rect.y + 30, tab_rect.width - 32, 16),
        self._controller._group_primary_text(group),
        15,
        meta_color,
        FontWeight.NORMAL,
      )
      if current:
        rl.draw_rectangle_rec(rl.Rectangle(tab_rect.x + 16, tab_rect.y + tab_rect.height - 4, tab_rect.width - 32, 2), rl.Color(116, 136, 168, 170))

      x += tab_w + BROWSER_CONTEXT_TAB_GAP

  def _render_empty_state(self, rect: rl.Rectangle, title: str, body: str):
    state_rect = rl.Rectangle(rect.x, rect.y, rect.width, rect.height)
    draw_soft_card(state_rect, rl.Color(255, 255, 255, 4), rl.Color(255, 255, 255, 10))
    gui_label(
      rl.Rectangle(state_rect.x, state_rect.y + 28, state_rect.width, 34),
      title,
      28,
      AetherListColors.HEADER,
      FontWeight.MEDIUM,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER,
    )
    gui_label(
      rl.Rectangle(state_rect.x + 44, state_rect.y + 68, state_rect.width - 88, 48),
      body,
      20,
      AetherListColors.SUBTEXT,
      FontWeight.NORMAL,
      alignment=rl.GuiTextAlignment.TEXT_ALIGN_CENTER,
    )

  def _render_selected_rows(self, rect: rl.Rectangle, entries: list[dict[str, str]]):
    self._selected_action_rects.clear()
    if not entries:
      self._render_empty_state(
        rect,
        tr("No regions selected"),
        tr("Choose the areas you actually drive in, then start the download when you are parked."),
      )
      return

    mouse_pos = gui_app.last_mouse_event.pos
    for index, entry in enumerate(entries):
      token = entry["token"]
      row_rect = rl.Rectangle(rect.x, rect.y + index * BROWSER_REGION_ROW_HEIGHT, rect.width, BROWSER_REGION_ROW_HEIGHT)
      row_hovered = rl.check_collision_point_rec(mouse_pos, row_rect)
      target_key = f"selected-remove:{token}"
      action_width = 132
      draw_selection_list_row(
        row_rect,
        title=entry["label"],
        subtitle=self._controller._selected_entry_subtitle(entry),
        action_text=tr("Remove"),
        current=True,
        hovered=row_hovered,
        pressed=self._pressed_target == target_key,
        is_last=index == len(entries) - 1,
        action_width=action_width,
        action_chip=False,
        action_text_color=AetherListColors.MUTED,
      )
      action_rect = rl.Rectangle(row_rect.x + row_rect.width - action_width, row_rect.y, action_width, row_rect.height)
      self._selected_action_rects[token] = action_rect

  def _render_region_rows(self, rect: rl.Rectangle, regions: list[dict]):
    if not regions:
      self._render_empty_state(
        rect,
        tr("Everything in this view is selected"),
        tr("Switch source or choose another tab below to add more regions."),
      )
      return

    mouse_pos = gui_app.last_mouse_event.pos
    self._region_row_rects.clear()
    for index, region in enumerate(regions):
      token = region["token"]
      selected = self._controller._get_map_state(token)
      row_rect = rl.Rectangle(rect.x, rect.y + index * BROWSER_REGION_ROW_HEIGHT, rect.width, BROWSER_REGION_ROW_HEIGHT)
      self._region_row_rects[token] = row_rect
      hovered = rl.check_collision_point_rec(mouse_pos, row_rect)
      target_key = f"region:{token}"
      action_text = self._controller._region_action_text(token)
      draw_selection_list_row(
        row_rect,
        title=region["label"],
        subtitle=self._controller._region_primary_text(token),
        action_text=action_text,
        current=selected,
        hovered=hovered,
        pressed=self._pressed_target == target_key,
        is_last=index == len(regions) - 1,
        action_width=142,
        action_chip=selected,
        action_fill=rl.Color(94, 168, 130, 34),
        action_border=rl.Color(94, 168, 130, 62),
      )

  def _active_browse_regions(self) -> list[dict]:
    return self._controller._browse_regions_for_active_group()

  def _measure_toolbar_height(self) -> float:
    return BROWSER_TOOLBAR_HEIGHT

  def _render_section_header(self, rect: rl.Rectangle, title: str, *, count_text: str | None = None, action_text: str | None = None):
    self._header_action_rect = rl.Rectangle(0, 0, 0, 0)
    title_right = rect.x + rect.width
    gap = 10
    control_y = rect.y + (rect.height - 38) / 2

    if action_text:
      action_w = max(112.0, min(154.0, 54 + len(action_text) * 8))
      self._header_action_rect = rl.Rectangle(rect.x + rect.width - action_w, control_y, action_w, 38)
      draw_action_pill(
        self._header_action_rect,
        action_text,
        rl.Color(255, 255, 255, 8),
        rl.Color(255, 255, 255, 22),
        AetherListColors.HEADER,
        font_size=17,
      )
      title_right = self._header_action_rect.x - gap

    if count_text:
      chip_w = max(122.0, min(184.0, 62 + len(count_text) * 9))
      chip_rect = rl.Rectangle(title_right - chip_w, control_y, chip_w, 38)
      AetherChip(count_text, rl.Color(89, 116, 151, 18), rl.Color(116, 136, 168, 38), AetherListColors.HEADER, pill=True, font_size=16).render(chip_rect)
      title_right = chip_rect.x - gap

    gui_label(
      rl.Rectangle(rect.x + 4, rect.y + (rect.height - 28) / 2, max(0.0, title_right - rect.x - 8), 28),
      title,
      24,
      AetherListColors.HEADER,
      FontWeight.SEMI_BOLD,
    )

  def _render_toolbar(self, rect: rl.Rectangle):
    draw_soft_card(rect, rl.Color(255, 255, 255, 3), rl.Color(255, 255, 255, 10))
    self._header_action_rect = rl.Rectangle(0, 0, 0, 0)
    source_rect = rl.Rectangle(rect.x, rect.y, rect.width, BROWSER_TOOLBAR_HEIGHT)
    self._source_selector.set_parent_rect(self._parent_rect or rect)
    self._source_selector.render(source_rect)

  def _measure_height(self, width: float) -> float:
    total = 0.0
    selected_count = len(self._controller._selected_entries_for_display())
    total += BROWSER_SECTION_HEADER_HEIGHT
    total += self._row_height(selected_count, BROWSER_REGION_ROW_HEIGHT) if selected_count else BROWSER_EMPTY_STATE_HEIGHT
    total += SECTION_CARD_GAP
    total += self._measure_toolbar_height() + 10
    total += self._measure_context_tabs_height(width)
    total += 10 + BROWSER_SECTION_HEADER_HEIGHT
    browse_count = len(self._active_browse_regions())
    total += self._row_height(browse_count, BROWSER_REGION_ROW_HEIGHT) if browse_count else BROWSER_EMPTY_STATE_HEIGHT
    return total

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    if not self._touch_valid():
      self._pressed_target = None
    draw_soft_card(rect, rl.Color(255, 255, 255, 4), rl.Color(255, 255, 255, 14))
    self._selected_action_rects.clear()
    self._context_tab_rects.clear()
    self._region_row_rects.clear()

    content_x = rect.x + BROWSER_INSET
    content_w = rect.width - BROWSER_INSET * 2
    y = rect.y

    selected_entries = self._controller._selected_entries_for_display()
    self._render_section_header(
      rl.Rectangle(content_x, y, content_w, BROWSER_SECTION_HEADER_HEIGHT),
      tr("Selected Regions"),
      count_text=self._controller._selected_section_count_text(),
    )
    y += BROWSER_SECTION_HEADER_HEIGHT

    selected_h = self._row_height(len(selected_entries), BROWSER_REGION_ROW_HEIGHT) if selected_entries else BROWSER_EMPTY_STATE_HEIGHT
    self._render_selected_rows(rl.Rectangle(content_x, y, content_w, selected_h), selected_entries)
    y += selected_h + SECTION_CARD_GAP

    toolbar_h = self._measure_toolbar_height()
    self._render_toolbar(rl.Rectangle(content_x, y, content_w, toolbar_h))
    y += toolbar_h + 10

    context_tabs_h = self._measure_context_tabs_height(content_w)
    self._render_context_tabs(rl.Rectangle(content_x, y, content_w, context_tabs_h))
    y += context_tabs_h + 10

    self._render_section_header(
      rl.Rectangle(content_x, y, content_w, BROWSER_SECTION_HEADER_HEIGHT),
      tr("Browse More"),
      count_text=self._controller._active_group_count_text(),
    )
    y += BROWSER_SECTION_HEADER_HEIGHT

    regions = self._active_browse_regions()
    region_h = self._row_height(len(regions), BROWSER_REGION_ROW_HEIGHT) if regions else BROWSER_EMPTY_STATE_HEIGHT
    self._render_region_rows(rl.Rectangle(content_x, y, content_w, region_h), regions)


class StarPilotMapsLayout(StarPilotPanel):
  VIEW_COUNTRIES = 0
  VIEW_STATES = 1

  def __init__(self):
    super().__init__()
    self._params_memory = Params(memory=True)
    self._worker_params = Params()
    self._map_sm = messaging.SubMaster(["mapdExtendedOut", "starpilotCarState"])
    self._scroll_panel = GuiScrollPanel2(horizontal=False)
    self._scrollbar = AetherScrollbar()
    self._scroll_offset = 0.0
    self._content_height = 0.0

    self._storage_text = "0 MB"
    self._has_downloaded_data = False
    self._storage_updated_at = 0.0
    self._storage_refresh_thread: threading.Thread | None = None
    self._storage_refresh_pending = False
    self._download_started_at: float | None = None
    self._cancel_requested_at: float | None = None
    self._cancel_visual_until = 0.0
    self._download_state = MapsDownloadState()
    self._view_index = self.VIEW_STATES
    self._active_country_group_key = COUNTRIES_SECTION["groups"][0]["key"]
    self._active_state_group_key = STATES_SECTION["groups"][0]["key"]
    self._full_us_mode = False
    self._whole_us_context_initialized = False

    self._download_button = self._child(
      AetherButton(
        self._primary_action_label,
        self._on_primary_action,
        enabled=self._primary_action_enabled,
        emphasized=True,
        font_size=24,
      )
    )
    self._schedule_button = self._child(
      AetherButton(
        lambda: tr(f"Auto Update: {schedule_label(self._params.get('PreferredSchedule'))}"),
        self._on_schedule,
        emphasized=False,
        font_size=22,
      )
    )

    self._status_card = self._child(MapStatusCard(self))
    self._browser_card = self._child(MapBrowserCard(self))

    self._browser_card.set_touch_valid_callback(lambda: self._scroll_panel.is_touch_valid())
    self._status_card.set_touch_valid_callback(lambda: True)

    self._sync_whole_us_view_state()
    self._refresh_storage_cache(force=True)
    self._sync_download_state(force=True)

  def show_event(self):
    super().show_event()
    self._scroll_offset = 0.0
    self._whole_us_context_initialized = False
    self._sync_whole_us_view_state()
    if self._cancel_requested() and self._cancel_requested_at is None:
      self._cancel_requested_at = rl.get_time()
    if self._cancel_requested() and self._cancel_visual_until <= rl.get_time():
      self._cancel_visual_until = rl.get_time() + 2.5
    self._refresh_storage_cache(force=True)
    self._sync_download_state(force=True)

  def hide_event(self):
    super().hide_event()
    self._scroll_offset = 0.0
    self._whole_us_context_initialized = False
    device.set_override_interactive_timeout(None)

  def _update_state(self):
    super()._update_state()
    self._sync_download_state()
    self._refresh_storage_cache()

    if self._download_state.active:
      device.set_override_interactive_timeout(300)
    else:
      device.set_override_interactive_timeout(None)

    if self._cancel_requested_at is not None and not self._download_state.active:
      if (rl.get_time() - self._cancel_requested_at) >= CANCEL_REQUEST_TIMEOUT:
        self._params_memory.remove("CancelDownloadMaps")
        self._cancel_requested_at = None

    if self._cancel_visual_until and rl.get_time() >= self._cancel_visual_until and not self._download_state.active:
      self._cancel_visual_until = 0.0

  def _sync_download_state(self, force: bool = False):
    del force
    self._map_sm.update(0)
    progress = self._map_sm["mapdExtendedOut"].downloadProgress if self._map_sm.valid.get("mapdExtendedOut", False) else None
    active = bool(progress.active) if progress is not None else False

    if active and self._download_started_at is None:
      self._download_started_at = rl.get_time()
    if active:
      self._cancel_requested_at = None
      self._cancel_visual_until = 0.0
    if not self._download_in_flight() and self._download_started_at is not None:
      self._download_started_at = None

    total_files = int(progress.totalFiles) if progress is not None else 0
    downloaded_files = int(progress.downloadedFiles) if progress is not None else 0
    percent = int((downloaded_files * 100) / max(total_files, 1)) if total_files > 0 else 0
    location_count = (len(progress.locationDetails) or len(progress.locations)) if progress is not None else 0
    primary_location = ""
    if progress is not None and len(progress.locationDetails) > 0:
      primary_location = str(progress.locationDetails[0].location)
    elif progress is not None and len(progress.locations) > 0:
      primary_location = str(progress.locations[0])

    progress_text = ""
    if active:
      progress_text = tr(f"{downloaded_files} / {total_files} ({percent}%)")
      if primary_location:
        progress_text = f"{progress_text}  {primary_location}"

    self._download_state = MapsDownloadState(
      active=active,
      cancelled=bool(progress.cancelled) if progress is not None else False,
      total_files=total_files,
      downloaded_files=downloaded_files,
      primary_location=primary_location,
      location_count=location_count,
      percent=percent,
      progress_text=progress_text,
    )

  def _refresh_storage_cache(self, force: bool = False):
    now = rl.get_time()
    if self._storage_refresh_pending:
      return
    if not force and (now - self._storage_updated_at) < 4.0:
      return

    def refresh_worker():
      try:
        storage_text, has_downloaded_data = self._calculate_storage_state()
        self._storage_text = storage_text
        self._has_downloaded_data = has_downloaded_data
        self._storage_updated_at = rl.get_time()
      finally:
        self._storage_refresh_pending = False

    self._storage_refresh_pending = True
    self._storage_updated_at = now
    self._storage_refresh_thread = threading.Thread(target=refresh_worker, daemon=True)
    self._storage_refresh_thread.start()

  def _calculate_storage_state(self) -> tuple[str, bool]:
    if not OFFLINE_MAPS_PATH.exists():
      return "0 MB", False

    total_size = 0
    has_files = False
    for path in OFFLINE_MAPS_PATH.rglob("*"):
      try:
        if not path.is_file():
          continue
        has_files = True
        total_size += path.stat().st_size
      except OSError:
        continue
    return _format_mb(total_size), has_files

  def _selected_tokens(self) -> set[str]:
    return _selected_token_set(self._params.get("MapsSelected", encoding="utf-8") or "")

  def _selected_entries(self) -> list[dict[str, str]]:
    entries = get_selected_map_entries(self._params.get("MapsSelected", encoding="utf-8") or "")
    for entry in entries:
      entry["kind"] = "country" if entry["token"].startswith(COUNTRY_PREFIX) else "state"
    return entries

  def _selected_entries_for_display(self) -> list[dict[str, str]]:
    entries = self._selected_entries()

    def sort_key(entry: dict[str, str]) -> tuple[int, str]:
      token = entry["token"]
      if token == US_COUNTRY_TOKEN:
        return (0, entry["label"])
      if token.startswith(STATE_PREFIX):
        return (1, entry["label"])
      return (2, entry["label"])

    return sorted(entries, key=sort_key)

  def _selected_count(self) -> int:
    return len(self._selected_tokens())

  def _selected_section_count_text(self) -> str:
    count = self._selected_count()
    if count == 0:
      return tr("Nothing selected")
    if count == 1:
      return tr("1 region")
    return tr(f"{count} regions")

  def _selection_summary_title(self) -> str:
    count = self._selected_count()
    if count == 0:
      return tr("No regions selected")
    if count == 1:
      return tr("1 region selected")
    return tr(f"{count} regions selected")

  def _selected_entry_subtitle(self, entry: dict[str, str]) -> str:
    token = entry["token"]
    if token == US_COUNTRY_TOKEN:
      return tr("Whole U.S. package")
    if token.startswith(STATE_PREFIX):
      return tr("Included in next download")
    return tr("Country map selected")

  def _set_view_index(self, index: int):
    self._view_index = max(self.VIEW_COUNTRIES, min(self.VIEW_STATES, index))
    if self._is_states_view():
      self._full_us_mode = self._active_state_group_key == "whole_us"
    else:
      self._full_us_mode = False

  def _has_full_us_selected(self) -> bool:
    return US_COUNTRY_TOKEN in self._selected_tokens()

  def _sync_whole_us_view_state(self):
    if self._whole_us_context_initialized:
      return
    if self._has_full_us_selected():
      self._active_state_group_key = "whole_us"
      self._full_us_mode = self._is_states_view()
    self._whole_us_context_initialized = True

  def _is_states_view(self) -> bool:
    return self._view_index == self.VIEW_STATES

  def _is_full_us_mode(self) -> bool:
    return self._is_states_view() and self._full_us_mode

  def _set_full_us_mode(self, enabled: bool):
    self._full_us_mode = bool(enabled) and self._is_states_view()

  def _current_section(self) -> dict:
    return STATES_SECTION if self._is_states_view() else COUNTRIES_SECTION

  def _current_groups(self) -> list[dict]:
    return self._current_section()["groups"]

  def _full_us_regions(self) -> list[dict]:
    return [{"token": US_COUNTRY_TOKEN, "label": MAP_TOKEN_LABELS[US_COUNTRY_TOKEN]}]

  def _whole_us_group(self) -> dict:
    return {"key": "whole_us", "title": tr("Whole U.S."), "regions": self._full_us_regions()}

  def _current_groups_with_full_us(self) -> list[dict]:
    groups = list(self._current_groups())
    if self._is_states_view():
      return [self._whole_us_group(), *groups]
    return groups

  def _active_group_key(self) -> str:
    if self._is_full_us_mode():
      return "whole_us"
    return self._active_state_group_key if self._is_states_view() else self._active_country_group_key

  def _set_active_group(self, group_key: str):
    if self._is_states_view() and group_key == "whole_us":
      self._active_state_group_key = "whole_us"
      self._set_full_us_mode(True)
      return

    group_keys = {group["key"] for group in self._current_groups()}
    if group_key not in group_keys:
      return
    if self._is_states_view():
      self._full_us_mode = False
      self._active_state_group_key = group_key
    else:
      self._active_country_group_key = group_key

  def _active_group(self) -> dict:
    if self._is_full_us_mode():
      return self._whole_us_group()

    group_key = self._active_group_key()
    for group in self._current_groups():
      if group["key"] == group_key:
        return group
    fallback = self._current_groups()[0]
    self._set_active_group(fallback["key"])
    return fallback

  def _active_group_regions(self) -> list[dict]:
    return self._active_group()["regions"]

  def _group_selected_count(self, group: dict) -> int:
    if self._is_states_view() and self._has_full_us_selected() and group["key"] != "whole_us":
      return len(group["regions"])
    return sum(1 for region in group["regions"] if self._get_map_state(region["token"]))

  def _group_primary_text(self, group: dict) -> str:
    if group["key"] == "whole_us":
      return tr("Full country package") if self._get_map_state(US_COUNTRY_TOKEN) else tr("One-tap full set")
    count = self._group_selected_count(group)
    if self._is_states_view():
      return tr(f"{count}/{len(group['regions'])} states")
    return tr(f"{count}/{len(group['regions'])} selected")

  def _active_group_count_text(self) -> str:
    if self._is_full_us_mode():
      return tr("Already selected") if self._get_map_state(US_COUNTRY_TOKEN) else tr("Whole U.S. package")
    group = self._active_group()
    browse_count = len(self._browse_regions_for_active_group())
    total = len(group["regions"])
    if self._is_states_view():
      return tr(f"{browse_count} of {total} available")
    return tr(f"{browse_count} of {total} available")

  def _toggle_region(self, token: str):
    self._set_map_state(token, not self._get_map_state(token))

  def _region_primary_text(self, token: str) -> str:
    if self._get_map_state(token):
      return tr("Included")
    if token == US_COUNTRY_TOKEN and self._is_full_us_mode():
      return tr("Whole country")
    return tr("Tap to add")

  def _region_action_text(self, token: str) -> str:
    if self._get_map_state(token):
      return tr("Included")
    return tr("Add U.S.") if token == US_COUNTRY_TOKEN and self._is_full_us_mode() else tr("Add")

  def _browser_source_primary(self, view_index: int) -> str:
    active = self._view_index == view_index
    if view_index == self.VIEW_COUNTRIES:
      country_count = sum(1 for entry in self._selected_entries() if entry["token"].startswith(COUNTRY_PREFIX) and entry["token"] != US_COUNTRY_TOKEN)
      return tr("Current view") if active else tr(f"{country_count} selected")
    state_count = sum(1 for entry in self._selected_entries() if entry["token"].startswith(STATE_PREFIX))
    if self._has_full_us_selected():
      state_count += 1
    return tr("Current view") if active else tr(f"{state_count} selected")

  def _browse_regions_for_active_group(self) -> list[dict]:
    selected_tokens = self._selected_tokens()
    if self._is_states_view() and self._has_full_us_selected():
      if self._is_full_us_mode():
        return []
      return []
    regions = self._full_us_regions() if self._is_full_us_mode() else self._active_group_regions()
    return [region for region in regions if region["token"] not in selected_tokens]

  def _get_map_state(self, token: str) -> bool:
    return token in self._selected_tokens()

  def _set_map_state(self, token: str, state: bool):
    selected = self._selected_tokens()
    if state:
      if token == US_COUNTRY_TOKEN:
        selected = {item for item in selected if not item.startswith(STATE_PREFIX)}
      elif token.startswith(STATE_PREFIX):
        selected.discard(US_COUNTRY_TOKEN)
        self._full_us_mode = False
      selected.add(token)
    else:
      selected.discard(token)
    self._params.put("MapsSelected", sanitize_selected_locations_csv(sorted(selected)))

  def _network_type(self):
    return ui_state.sm["deviceState"].networkType if ui_state.sm.valid.get("deviceState", False) else NetworkType.none

  def _network_label(self) -> str:
    return tr(NETWORK_TYPE_LABELS.get(self._network_type(), tr_noop("Offline")))

  def _is_online(self) -> bool:
    return self._network_type() != NetworkType.none

  def _is_parked(self) -> bool:
    if ui_state.is_offroad():
      return True
    if self._map_sm.valid.get("starpilotCarState", False):
      return bool(self._map_sm["starpilotCarState"].isParked)
    return False

  def _download_requested(self) -> bool:
    return self._params_memory.get_bool("DownloadMaps")

  def _cancel_requested(self) -> bool:
    return self._params_memory.get_bool("CancelDownloadMaps")

  def _is_visually_cancelling(self) -> bool:
    return self._cancel_requested() or (self._cancel_visual_until > rl.get_time())

  def _download_in_flight(self) -> bool:
    return self._download_state.active or self._download_requested()

  def _remove_enabled(self) -> bool:
    return self._has_downloaded_data and self._is_parked() and not self._download_in_flight() and not self._is_visually_cancelling()

  def _download_gate_reason(self) -> str:
    if self._download_in_flight():
      return tr("Download in progress")
    if self._selected_count() == 0:
      return tr("Select regions first")
    if not self._is_online():
      return tr("Connect to the internet")
    if not self._is_parked():
      return tr("Park to download")
    return ""

  def _primary_action_enabled(self) -> bool:
    if self._is_visually_cancelling():
      return False
    if self._download_in_flight():
      return True
    return self._download_gate_reason() == ""

  def _primary_action_label(self) -> str:
    if self._is_visually_cancelling():
      return tr("Cancelling...")
    if self._download_in_flight():
      return tr("Cancel Download")
    return tr("Download Offline Maps")

  def _on_primary_action(self):
    if self._download_in_flight():
      self._on_cancel()
    else:
      self._on_download()

  def _on_schedule(self):
    options = list(MAP_SCHEDULE_LABELS.values())
    current = schedule_label(self._params.get("PreferredSchedule"))

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put_int("PreferredSchedule", normalize_schedule_value(dialog.selection))

    dialog = MultiOptionDialog(tr("Auto Update Schedule"), options, current, callback=on_select)
    gui_app.push_widget(dialog)

  def _on_download(self):
    gate_reason = self._download_gate_reason()
    if gate_reason:
      gui_app.push_widget(alert_dialog(gate_reason))
      return

    current_selected = self._params.get("MapsSelected", encoding="utf-8") or ""
    selected_raw = sanitize_selected_locations_csv(current_selected)
    if selected_raw != current_selected:
      self._params.put("MapsSelected", selected_raw)
    if not selected_raw:
      gui_app.push_widget(alert_dialog(tr("Select at least one region before downloading maps.")))
      return

    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        gate_reason = self._download_gate_reason()
        if gate_reason:
          gui_app.push_widget(alert_dialog(gate_reason))
          return
        self._params_memory.put_bool("DownloadMaps", True)
        self._params_memory.remove("CancelDownloadMaps")
        self._download_started_at = rl.get_time()

    gui_app.push_widget(ConfirmDialog(tr("Start downloading offline maps for the selected regions?"), tr("Download"), callback=on_confirm))

  def _on_cancel(self):
    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        if not self._download_in_flight():
          self._cancel_requested_at = None
          self._cancel_visual_until = 0.0
          return
        self._params_memory.put_bool("CancelDownloadMaps", True)
        self._params_memory.remove("DownloadMaps")
        self._cancel_requested_at = rl.get_time()
        self._cancel_visual_until = rl.get_time() + 2.5

    gui_app.push_widget(ConfirmDialog(tr("Cancel the current map download?"), tr("Cancel Download"), callback=on_confirm))

  def _has_downloaded_maps(self) -> bool:
    return self._has_downloaded_data

  def _on_remove(self):
    if not self._remove_enabled():
      if not self._is_parked():
        gui_app.push_widget(alert_dialog(tr("Park to remove downloaded maps.")))
      return

    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        if not self._remove_enabled():
          if not self._is_parked():
            gui_app.push_widget(alert_dialog(tr("Park to remove downloaded maps.")))
          return

        def remove_worker():
          if OFFLINE_MAPS_PATH.exists():
            shutil.rmtree(OFFLINE_MAPS_PATH, ignore_errors=True)
          self._refresh_storage_cache(force=True)

        threading.Thread(target=remove_worker, daemon=True).start()
        gui_app.push_widget(alert_dialog(tr("Removing offline maps...")))

    gui_app.push_widget(ConfirmDialog(tr("Delete all downloaded offline map data?"), tr("Remove Maps"), callback=on_confirm))

  def _last_updated_text(self) -> str:
    last_update = self._worker_params.get("LastMapsUpdate", encoding="utf-8")
    return last_update or tr("Never")

  def _progress_title(self) -> str:
    if self._is_visually_cancelling():
      return tr("Cancelling Download")
    if self._download_state.active:
      return tr("Downloading Now")
    if self._download_requested():
      return tr("Preparing Download")
    if self._has_downloaded_maps():
      return tr("Offline Maps Ready")
    return tr("Download Readiness")

  def _progress_body(self) -> str:
    if self._download_state.active:
      elapsed_ms = int((rl.get_time() - (self._download_started_at or rl.get_time())) * 1000)
      elapsed_text = _format_elapsed_ms(elapsed_ms)
      eta_text = _format_eta_ms(elapsed_ms, self._download_state.downloaded_files, self._download_state.total_files)
      if self._download_state.primary_location:
        return tr(f"{self._download_state.progress_text}\nElapsed {elapsed_text} | ETA {eta_text}")
      return tr(f"{self._download_state.downloaded_files} / {self._download_state.total_files} ({self._download_state.percent}%)\nElapsed {elapsed_text} | ETA {eta_text}")

    if self._is_visually_cancelling():
      return tr("Stop request sent. The current transfer will wind down safely.")

    if self._download_requested():
      return tr("Contacting the background downloader and preparing the selected regions.")

    gate_reason = self._download_gate_reason()
    if gate_reason:
      if self._selected_count() == 0:
        return tr("Pick regions below, then start the first offline download.")
      return gate_reason
    return tr("Ready to refresh speed-limit map data.")

  def _status_footer_text(self) -> str:
    parts = [self._network_label(), tr("Parked") if self._is_parked() else tr("Not parked")]
    if self._download_state.active:
      parts.append(tr(f"{self._download_state.percent}% complete"))
    elif self._is_visually_cancelling():
      parts.append(tr("Cancelling"))
    elif self._download_requested():
      parts.append(tr("Starting"))
    else:
      parts.append(schedule_label(self._params.get("PreferredSchedule")))
    return "  •  ".join(parts)

  def _measure_content_height(self, width: float) -> float:
    return self._browser_card._measure_height(width)

  def _draw_scroll_content(self, rect: rl.Rectangle, width: float):
    self._browser_card.set_parent_rect(rect)

    y = rect.y + self._scroll_offset

    browser_height = self._browser_card._measure_height(width)
    self._browser_card.render(rl.Rectangle(rect.x, y, width, browser_height))

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    frame = build_list_panel_frame(rect)
    draw_list_panel_shell(frame)

    hdr = frame.header
    title_y = hdr.y + HEADER_TOP_OFFSET
    gui_label(rl.Rectangle(hdr.x, title_y, hdr.width, HEADER_TITLE_HEIGHT), tr("Map Data"), 40, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(hdr.x, title_y + 40, hdr.width * 0.72, HEADER_SUBTITLE_HEIGHT), tr("Use offline maps for speed-limit control and keep only the regions you need."), 22, AetherListColors.SUBTEXT, FontWeight.NORMAL)

    header_status_y = title_y + HEADER_TITLE_HEIGHT + HEADER_SUBTITLE_HEIGHT + 8
    header_status_rect = rl.Rectangle(hdr.x, header_status_y, hdr.width, hdr.y + hdr.height - header_status_y - HEADER_BOTTOM_GAP)
    self._status_card.render(header_status_rect)

    scroll_rect = frame.scroll
    content_width = scroll_rect.width - 18
    scroll_content_rect = rl.Rectangle(scroll_rect.x, scroll_rect.y, scroll_rect.width, scroll_rect.height)
    self._content_height = self._measure_content_height(content_width)
    self._scroll_panel.set_enabled(self.is_visible)
    self._scroll_offset = self._scroll_panel.update(scroll_content_rect, max(self._content_height, scroll_content_rect.height))

    rl.begin_scissor_mode(int(scroll_content_rect.x), int(scroll_content_rect.y), int(scroll_content_rect.width), int(scroll_content_rect.height))
    self._draw_scroll_content(scroll_content_rect, content_width)
    rl.end_scissor_mode()

    if self._content_height > scroll_content_rect.height:
      self._scrollbar.render(scroll_content_rect, self._content_height, self._scroll_offset)

    draw_list_scroll_fades(scroll_content_rect, self._content_height, self._scroll_offset, AetherListColors.PANEL_BG)
