from __future__ import annotations

import math
import shutil
import threading
from dataclasses import dataclass
from pathlib import Path

from cereal import log, messaging
import pyray as rl

from openpilot.common.params import Params
from openpilot.selfdrive.ui.ui_state import device, ui_state
from openpilot.system.ui.lib.application import FontWeight, MousePos, gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.lib.scroll_panel2 import GuiScrollPanel2
from openpilot.system.ui.widgets import DialogResult, Widget
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.label import gui_label, gui_text_box
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog

from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import (
  AetherButton,
  AetherChip,
  AetherListColors,
  AetherScrollbar,
  HubTile,
  SPACING,
  TileGrid,
  build_list_panel_frame,
  draw_action_pill,
  draw_busy_ring,
  draw_list_panel_shell,
  draw_list_row_shell,
  draw_list_scroll_fades,
  draw_soft_card,
)
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.starpilot.common.maps_catalog import (
  MAPS_CATALOG,
  MAP_SCHEDULE_LABELS,
  get_selected_map_entries,
  normalize_schedule_value,
  sanitize_selected_locations_csv,
  schedule_label,
)
from openpilot.starpilot.common.maps_selection import COUNTRY_PREFIX


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
SECTION_HEADER_HEIGHT = 54
SECTION_HEADER_GAP = 12
GROUP_HEADER_HEIGHT = 48
GROUP_ROW_GAP = 12
GROUP_TILE_HEIGHT = 112
SUMMARY_ROW_HEIGHT = 82
ACTION_BUTTON_HEIGHT = 66
ACTION_BUTTON_GAP = 12
BROWSER_GAP = SPACING.tile_gap
BROWSER_SOURCE_TILE_HEIGHT = 142
BROWSER_SCOPE_TILE_HEIGHT = 134
BROWSER_GROUP_TILE_HEIGHT = 136
BROWSER_REGION_TILE_HEIGHT = 118
BROWSER_INTRO_HEIGHT = 50
BROWSER_SECTION_HEADER_HEIGHT = 44

COUNTRIES_SECTION = MAPS_CATALOG[0]
STATES_SECTION = MAPS_CATALOG[1]
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


class ActionButtonStrip(Widget):
  def __init__(self, primary_button: AetherButton, secondary_button: AetherButton):
    super().__init__()
    self._primary_button = primary_button
    self._secondary_button = secondary_button

  def set_touch_valid_callback(self, touch_callback):
    super().set_touch_valid_callback(touch_callback)
    self._primary_button.set_touch_valid_callback(touch_callback)
    self._secondary_button.set_touch_valid_callback(touch_callback)

  def _render(self, rect: rl.Rectangle):
    gap = ACTION_BUTTON_GAP
    primary_w = max(260.0, rect.width * 0.58)
    secondary_w = max(210.0, rect.width - primary_w - gap)
    self._primary_button.render(rl.Rectangle(rect.x, rect.y, primary_w, rect.height))
    self._secondary_button.render(rl.Rectangle(rect.x + primary_w + gap, rect.y, secondary_w, rect.height))


class MapStatusCard(Widget):
  def __init__(self, controller: "StarPilotMapsLayout"):
    super().__init__()
    self._controller = controller
    self._remove_rect = rl.Rectangle(0, 0, 0, 0)
    self._pressed_remove = False

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

    left_w = rect.width * 0.52
    right_x = rect.x + left_w + 20
    right_w = rect.x + rect.width - right_x - 18
    top_y = rect.y + 16

    gui_label(rl.Rectangle(rect.x + 20, top_y, left_w - 20, 24), self._controller._progress_title(), 24, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_text_box(
      rl.Rectangle(rect.x + 20, top_y + 28, left_w - 26, 40),
      self._controller._progress_body(),
      17,
      AetherListColors.SUBTEXT,
      font_weight=FontWeight.NORMAL,
      line_scale=0.94,
    )
    gui_label(rl.Rectangle(rect.x + 20, rect.y + rect.height - 22, left_w - 20, 18), self._controller._status_footer_text(), 16, AetherListColors.MUTED, FontWeight.MEDIUM)

    metric_gap = 18
    metric_w = max(120.0, (right_w - metric_gap) / 2)
    gui_label(rl.Rectangle(right_x, top_y, metric_w, 18), tr("Storage"), 16, AetherListColors.MUTED, FontWeight.MEDIUM)
    gui_label(rl.Rectangle(right_x, top_y + 16, metric_w, 24), self._controller._storage_text, 22, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(right_x + metric_w + metric_gap, top_y, metric_w, 18), tr("Last Updated"), 16, AetherListColors.MUTED, FontWeight.MEDIUM)
    gui_label(rl.Rectangle(right_x + metric_w + metric_gap, top_y + 16, metric_w, 24), self._controller._last_updated_text(), 20, AetherListColors.HEADER, FontWeight.SEMI_BOLD)

    selection_y = rect.y + 54
    action_w = 150
    action_h = 36
    selection_w = max(180.0, right_w - action_w - 14)
    gui_label(rl.Rectangle(right_x, selection_y, selection_w, 18), tr("Selected Regions"), 16, AetherListColors.MUTED, FontWeight.MEDIUM)
    gui_label(rl.Rectangle(right_x, selection_y + 18, selection_w, 22), self._controller._selection_summary_title(), 20, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_text_box(
      rl.Rectangle(right_x, selection_y + 40, selection_w, 18),
      self._controller._selection_summary_meta(),
      16,
      AetherListColors.SUBTEXT,
      font_weight=FontWeight.NORMAL,
      line_scale=0.92,
    )

    self._remove_rect = rl.Rectangle(rect.x + rect.width - action_w - 20, rect.y + rect.height - action_h - 16, action_w, action_h)
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


class SelectedMapsCard(Widget):
  def __init__(self, controller: "StarPilotMapsLayout"):
    super().__init__()
    self._controller = controller

  def _measure_height(self) -> float:
    rows = max(1, len(self._controller._selected_entries()))
    return 64 + rows * SUMMARY_ROW_HEIGHT

  def _render(self, rect: rl.Rectangle):
    draw_soft_card(rect, rl.Color(255, 255, 255, 4), rl.Color(255, 255, 255, 16))

    gui_label(rl.Rectangle(rect.x + 18, rect.y + 14, rect.width - 36, 26), tr("Included Regions"), 25, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(rect.x + 18, rect.y + 40, rect.width - 36, 20), tr("Review what stays offline here. Use the source tiles above to make changes."), 18, AetherListColors.SUBTEXT, FontWeight.NORMAL)

    entries = self._controller._selected_entries()
    rows = entries if entries else [{"token": "", "label": tr("No regions added yet."), "kind": "empty"}]
    y = rect.y + 64
    for index, entry in enumerate(rows):
      row_rect = rl.Rectangle(rect.x + 12, y, rect.width - 24, SUMMARY_ROW_HEIGHT)
      is_last = index == len(rows) - 1
      is_empty = entry.get("kind") == "empty"
      draw_list_row_shell(
        row_rect,
        current=not is_empty,
        is_last=is_last,
        current_bg=rl.Color(89, 116, 151, 12),
        current_border=rl.Color(116, 136, 168, 28),
      )

      kind_text = ""
      if not is_empty:
        kind_text = tr("Country") if entry["token"].startswith(COUNTRY_PREFIX) else tr("U.S. State")
        kind_chip = AetherChip(kind_text, rl.Color(89, 116, 151, 16), rl.Color(116, 136, 168, 46), AetherListColors.SUBTEXT, pill=True, font_size=16)
        kind_chip.render(rl.Rectangle(row_rect.x + 18, row_rect.y + 30, 124 if kind_text == tr("Country") else 138, 30))

      text_x = row_rect.x + 18 + (136 if kind_text else 0)
      gui_label(rl.Rectangle(text_x, row_rect.y + 26, row_rect.width - 220, 36), entry["label"], 24, AetherListColors.HEADER if not is_empty else AetherListColors.SUBTEXT, FontWeight.MEDIUM)

      if not is_empty:
        gui_label(rl.Rectangle(row_rect.x + row_rect.width - 180, row_rect.y + 24, 162, 34), tr("Edit above"), 16, AetherListColors.MUTED, FontWeight.MEDIUM, alignment=rl.GuiTextAlignment.TEXT_ALIGN_RIGHT)

      y += SUMMARY_ROW_HEIGHT


class MapBrowserCard(Widget):
  def __init__(self, controller: "StarPilotMapsLayout"):
    super().__init__()
    self._controller = controller
    self._source_grid = self._child(TileGrid(columns=2, padding=SPACING.tile_gap, uniform_width=True))
    self._scope_grid = self._child(TileGrid(columns=2, padding=SPACING.tile_gap, uniform_width=True))
    self._group_grid = self._child(TileGrid(columns=None, padding=SPACING.tile_gap))
    self._region_grid = self._child(TileGrid(columns=None, padding=SPACING.tile_gap))
    self._source_tiles: dict[str, HubTile] = {}
    self._scope_tiles: dict[str, HubTile] = {}
    self._group_tiles: dict[str, HubTile] = {}
    self._region_tiles: dict[str, HubTile] = {}
    self._header_action_rect = rl.Rectangle(0, 0, 0, 0)
    self._header_pressed = False

    source_specs = [
      ("states", tr_noop("United States"), lambda: self._controller._browser_source_primary(self._controller.VIEW_STATES), lambda: self._controller._browser_source_desc(self._controller.VIEW_STATES), lambda: self._controller._set_view_index(self._controller.VIEW_STATES)),
      ("countries", tr_noop("Countries"), lambda: self._controller._browser_source_primary(self._controller.VIEW_COUNTRIES), lambda: self._controller._browser_source_desc(self._controller.VIEW_COUNTRIES), lambda: self._controller._set_view_index(self._controller.VIEW_COUNTRIES)),
    ]
    for key, title, primary, desc, on_click in source_specs:
      tile = self._child(HubTile(title=title, desc=desc, icon_path="", on_click=on_click, bg_color="#10B981", get_status=primary))
      self._source_tiles[key] = tile
      self._source_grid.add_tile(tile)

    scope_specs = [
      ("regional", tr_noop("Regional Set"), lambda: self._controller._browser_scope_primary(False), lambda: self._controller._browser_scope_desc(False), lambda: self._controller._set_full_us_mode(False)),
      ("whole_us", tr_noop("Whole U.S."), lambda: self._controller._browser_scope_primary(True), lambda: self._controller._browser_scope_desc(True), lambda: self._controller._set_full_us_mode(True)),
    ]
    for key, title, primary, desc, on_click in scope_specs:
      tile = self._child(HubTile(title=title, desc=desc, icon_path="", on_click=on_click, bg_color="#10B981", get_status=primary))
      self._scope_tiles[key] = tile
      self._scope_grid.add_tile(tile)

    for group in COUNTRIES_SECTION["groups"] + STATES_SECTION["groups"]:
      group_key = group["key"]
      tile = self._child(
        HubTile(
          title=tr_noop(group["title"]),
          desc=lambda group=group: self._controller._group_secondary_text(group),
          icon_path="",
          on_click=lambda group_key=group_key: self._controller._set_active_group(group_key),
          bg_color="#10B981",
          get_status=lambda group=group: self._controller._group_primary_text(group),
        )
      )
      self._group_tiles[group_key] = tile
      self._group_grid.add_tile(tile)
      for region in group["regions"]:
        token = region["token"]
        tile = self._child(
          HubTile(
            title=tr_noop(region["label"]),
            desc=lambda token=token: self._controller._region_secondary_text(token),
            icon_path="",
            on_click=lambda token=token: self._controller._toggle_region(token),
            bg_color="#10B981",
            get_status=lambda token=token: self._controller._region_primary_text(token),
          )
        )
        self._region_tiles[token] = tile
        self._region_grid.add_tile(tile)

  def set_touch_valid_callback(self, touch_callback):
    super().set_touch_valid_callback(touch_callback)
    for grid in (self._source_grid, self._scope_grid, self._group_grid, self._region_grid):
      grid.set_touch_valid_callback(touch_callback)
    for tile in self._source_tiles.values():
      tile.set_touch_valid_callback(touch_callback)
    for tile in self._scope_tiles.values():
      tile.set_touch_valid_callback(touch_callback)
    for tile in self._group_tiles.values():
      tile.set_touch_valid_callback(touch_callback)
    for tile in self._region_tiles.values():
      tile.set_touch_valid_callback(touch_callback)

  def _handle_mouse_press(self, mouse_pos: MousePos):
    if not self._touch_valid():
      return
    if rl.check_collision_point_rec(mouse_pos, self._header_action_rect):
      self._header_pressed = True

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._header_pressed:
      self._header_pressed = False
      if self._touch_valid() and rl.check_collision_point_rec(mouse_pos, self._header_action_rect):
        self._controller._toggle_active_group()

  def _grid_height(self, count: int, width: float, tile_height: float) -> float:
    if count <= 0:
      return 0.0
    max_cols_by_width = max(1, int((width + BROWSER_GAP) / (300 + BROWSER_GAP)))
    preferred_cols = 2 if count <= 2 else 3 if count <= 6 else 4
    cols = max(1, min(count, preferred_cols, max_cols_by_width))
    rows = math.ceil(count / max(cols, 1))
    return rows * tile_height + BROWSER_GAP * max(0, rows - 1)

  def _render_tile_grid(self, grid: TileGrid, rect: rl.Rectangle, tiles: list[Widget], tile_height: float):
    if not tiles or rect.height <= 0:
      return
    grid.tiles = list(tiles)
    grid.set_parent_rect(self._parent_rect or self._rect)
    grid.render(rect)

  def _set_tile_active(self, tile: HubTile, active: bool):
    tile.surface_color = rl.Color(16, 185, 129, 255) if active else rl.Color(93, 102, 116, 255)

  def _measure_height(self, width: float) -> float:
    source_count = 2
    total = BROWSER_INTRO_HEIGHT + self._grid_height(source_count, width, BROWSER_SOURCE_TILE_HEIGHT)
    total += SECTION_CARD_GAP
    if self._controller._is_states_view():
      total += self._grid_height(2, width, BROWSER_SCOPE_TILE_HEIGHT) + SECTION_CARD_GAP
      if self._controller._is_full_us_mode():
        total += BROWSER_SECTION_HEADER_HEIGHT + self._grid_height(1, width, BROWSER_GROUP_TILE_HEIGHT)
        return total
    total += BROWSER_SECTION_HEADER_HEIGHT
    total += self._grid_height(len(self._controller._current_groups()), width, BROWSER_GROUP_TILE_HEIGHT)
    total += SECTION_CARD_GAP + BROWSER_SECTION_HEADER_HEIGHT
    total += self._grid_height(len(self._controller._active_group_regions()), width, BROWSER_REGION_TILE_HEIGHT)
    return total

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    draw_soft_card(rect, rl.Color(255, 255, 255, 4), rl.Color(255, 255, 255, 14))

    content_x = rect.x
    content_w = rect.width
    y = rect.y

    gui_label(rl.Rectangle(content_x + 4, y, content_w * 0.62, 28), self._controller._browser_title(), 28, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(content_x + 4, y + 26, content_w * 0.72, 20), self._controller._browser_subtitle(), 17, AetherListColors.SUBTEXT, FontWeight.NORMAL)

    count_text = self._controller._active_group_count_text()
    count_chip_w = max(142, min(182, 62 + len(count_text) * 9))
    AetherChip(count_text, rl.Color(89, 116, 151, 20), rl.Color(116, 136, 168, 42), AetherListColors.HEADER, pill=True, font_size=16).render(
      rl.Rectangle(content_x + content_w - count_chip_w - 126, y + 4, count_chip_w, 34)
    )
    self._header_action_rect = rl.Rectangle(content_x + content_w - 112, y + 4, 112, 34)
    draw_action_pill(
      self._header_action_rect,
      self._controller._active_group_action_label(),
      rl.Color(255, 255, 255, 8),
      rl.Color(255, 255, 255, 22),
      AetherListColors.HEADER,
      font_size=16,
    )

    y += BROWSER_INTRO_HEIGHT

    source_tiles = [self._source_tiles["states"], self._source_tiles["countries"]]
    self._set_tile_active(self._source_tiles["states"], self._controller._view_index == self._controller.VIEW_STATES)
    self._set_tile_active(self._source_tiles["countries"], self._controller._view_index == self._controller.VIEW_COUNTRIES)
    source_h = self._grid_height(len(source_tiles), content_w, BROWSER_SOURCE_TILE_HEIGHT)
    self._render_tile_grid(self._source_grid, rl.Rectangle(content_x, y, content_w, source_h), source_tiles, BROWSER_SOURCE_TILE_HEIGHT)
    y += source_h + SECTION_CARD_GAP

    if self._controller._is_states_view():
      scope_tiles = [self._scope_tiles["regional"], self._scope_tiles["whole_us"]]
      self._set_tile_active(self._scope_tiles["regional"], not self._controller._is_full_us_mode())
      self._set_tile_active(self._scope_tiles["whole_us"], self._controller._is_full_us_mode() or self._controller._get_map_state(US_COUNTRY_TOKEN))
      scope_h = self._grid_height(len(scope_tiles), content_w, BROWSER_SCOPE_TILE_HEIGHT)
      self._render_tile_grid(self._scope_grid, rl.Rectangle(content_x, y, content_w, scope_h), scope_tiles, BROWSER_SCOPE_TILE_HEIGHT)
      y += scope_h + SECTION_CARD_GAP

      if self._controller._is_full_us_mode():
        gui_label(rl.Rectangle(content_x + 4, y, content_w, 24), tr("Whole-country download"), 24, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
        gui_label(rl.Rectangle(content_x + 4, y + 22, content_w, 18), tr("Use this when you want every U.S. map instead of choosing states by region."), 16, AetherListColors.SUBTEXT, FontWeight.NORMAL)
        y += BROWSER_SECTION_HEADER_HEIGHT
        self._set_tile_active(self._region_tiles[US_COUNTRY_TOKEN], self._controller._get_map_state(US_COUNTRY_TOKEN))
        self._render_tile_grid(self._group_grid, rl.Rectangle(content_x, y, content_w, self._grid_height(1, content_w, BROWSER_GROUP_TILE_HEIGHT)), [self._region_tiles[US_COUNTRY_TOKEN]], BROWSER_GROUP_TILE_HEIGHT)
        return

    gui_label(rl.Rectangle(content_x + 4, y, content_w, 24), self._controller._group_section_title(), 24, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(content_x + 4, y + 22, content_w, 18), self._controller._group_section_desc(), 16, AetherListColors.SUBTEXT, FontWeight.NORMAL)
    y += BROWSER_SECTION_HEADER_HEIGHT

    group_tiles = [self._group_tiles[group["key"]] for group in self._controller._current_groups()]
    for group in self._controller._current_groups():
      self._set_tile_active(self._group_tiles[group["key"]], self._controller._active_group_key() == group["key"])
    group_h = self._grid_height(len(group_tiles), content_w, BROWSER_GROUP_TILE_HEIGHT)
    self._render_tile_grid(self._group_grid, rl.Rectangle(content_x, y, content_w, group_h), group_tiles, BROWSER_GROUP_TILE_HEIGHT)
    y += group_h + SECTION_CARD_GAP

    gui_label(rl.Rectangle(content_x + 4, y, content_w, 24), self._controller._region_section_title(), 24, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(content_x + 4, y + 22, content_w, 18), self._controller._region_section_desc(), 16, AetherListColors.SUBTEXT, FontWeight.NORMAL)
    y += BROWSER_SECTION_HEADER_HEIGHT

    region_tiles = [self._region_tiles[region["token"]] for region in self._controller._active_group_regions() if region["token"] != US_COUNTRY_TOKEN or not self._controller._is_states_view()]
    for region in self._controller._active_group_regions():
      token = region["token"]
      if token in self._region_tiles:
        self._set_tile_active(self._region_tiles[token], self._controller._get_map_state(token))
    region_h = self._grid_height(len(region_tiles), content_w, BROWSER_REGION_TILE_HEIGHT)
    self._render_tile_grid(self._region_grid, rl.Rectangle(content_x, y, content_w, region_h), region_tiles, BROWSER_REGION_TILE_HEIGHT)


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

    self._status_card = self._child(MapStatusCard(self))
    self._selected_card = self._child(SelectedMapsCard(self))

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
    self._action_strip = self._child(ActionButtonStrip(self._download_button, self._schedule_button))
    self._browser_card = self._child(MapBrowserCard(self))

    self._browser_card.set_touch_valid_callback(lambda: self._scroll_panel.is_touch_valid())
    self._selected_card.set_touch_valid_callback(lambda: self._scroll_panel.is_touch_valid())
    self._action_strip.set_touch_valid_callback(lambda: self._scroll_panel.is_touch_valid())

    self._refresh_storage_cache(force=True)
    self._sync_download_state(force=True)

  def show_event(self):
    super().show_event()
    self._scroll_offset = 0.0
    if self._cancel_requested() and self._cancel_requested_at is None:
      self._cancel_requested_at = rl.get_time()
    if self._cancel_requested() and self._cancel_visual_until <= rl.get_time():
      self._cancel_visual_until = rl.get_time() + 2.5
    self._refresh_storage_cache(force=True)
    self._sync_download_state(force=True)

  def hide_event(self):
    super().hide_event()
    self._scroll_offset = 0.0
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
      if not path.is_file():
        continue
      has_files = True
      total_size += path.stat().st_size
    return _format_mb(total_size), has_files

  def _selected_tokens(self) -> set[str]:
    return _selected_token_set(self._params.get("MapsSelected", encoding="utf-8") or "")

  def _selected_entries(self) -> list[dict[str, str]]:
    entries = get_selected_map_entries(self._params.get("MapsSelected", encoding="utf-8") or "")
    for entry in entries:
      entry["kind"] = "country" if entry["token"].startswith(COUNTRY_PREFIX) else "state"
    return entries

  def _selected_count(self) -> int:
    return len(self._selected_tokens())

  def _selection_summary_title(self) -> str:
    count = self._selected_count()
    if count == 0:
      return tr("Choose regions to get started")
    if count == 1:
      return tr("1 region ready")
    return tr(f"{count} regions ready")

  def _selection_summary_meta(self) -> str:
    entries = self._selected_entries()
    if not entries:
      return tr("Pick only the places you actually drive.")

    countries = sum(1 for entry in entries if entry["token"].startswith(COUNTRY_PREFIX))
    states = len(entries) - countries
    parts = []
    if countries:
      parts.append(tr(f"{countries} countries"))
    if states:
      parts.append(tr(f"{states} U.S. states"))
    return " / ".join(parts)

  def _set_view_index(self, index: int):
    self._view_index = max(self.VIEW_COUNTRIES, min(self.VIEW_STATES, index))

  def _on_group_change(self, index: int):
    self._set_view_index(index)

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

  def _active_group_key(self) -> str:
    return self._active_state_group_key if self._is_states_view() else self._active_country_group_key

  def _set_active_group(self, group_key: str):
    group_keys = {group["key"] for group in self._current_groups()}
    if group_key not in group_keys:
      return
    if self._is_states_view():
      self._active_state_group_key = group_key
    else:
      self._active_country_group_key = group_key

  def _active_group(self) -> dict:
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
    return sum(1 for region in group["regions"] if self._get_map_state(region["token"]))

  def _group_primary_text(self, group: dict) -> str:
    count = self._group_selected_count(group)
    if self._is_states_view():
      return tr(f"{count}/{len(group['regions'])} states")
    if any(region["token"] == US_COUNTRY_TOKEN for region in group["regions"]):
      return tr("Whole country") if self._get_map_state(US_COUNTRY_TOKEN) else tr("Tap to add")
    return tr(f"{count}/{len(group['regions'])} selected")

  def _group_secondary_text(self, group: dict) -> str:
    if self._is_states_view():
      return tr("Open this region") if self._active_group_key() != group["key"] else tr("Active region")
    if any(region["token"] == US_COUNTRY_TOKEN for region in group["regions"]):
      return tr("Entire U.S. package")
    return tr("Open this continent") if self._active_group_key() != group["key"] else tr("Active continent")

  def _active_group_count_text(self) -> str:
    group = self._active_group()
    count = self._group_selected_count(group)
    total = len(group["regions"])
    if self._is_states_view():
      return tr(f"{count}/{total} states")
    return tr(f"{count}/{total} selected")

  def _active_group_action_label(self) -> str:
    if self._is_full_us_mode():
      return tr("Remove") if self._get_map_state(US_COUNTRY_TOKEN) else tr("Add U.S.")
    group = self._active_group()
    return tr("Clear") if self._group_selected_count(group) == len(group["regions"]) else tr("Select All")

  def _toggle_active_group(self):
    if self._is_full_us_mode():
      self._toggle_region(US_COUNTRY_TOKEN)
      return
    group = self._active_group()
    state = self._group_selected_count(group) != len(group["regions"])
    for region in group["regions"]:
      self._set_map_state(region["token"], state)

  def _toggle_region(self, token: str):
    self._set_map_state(token, not self._get_map_state(token))

  def _region_primary_text(self, token: str) -> str:
    if self._get_map_state(token):
      return tr("Included")
    if token == US_COUNTRY_TOKEN:
      return tr("Whole country")
    return tr("Tap to add")

  def _region_secondary_text(self, token: str) -> str:
    if self._is_states_view():
      return tr("U.S. state")
    return tr("Country") if token != US_COUNTRY_TOKEN else tr("Country-wide map set")

  def _browser_source_primary(self, view_index: int) -> str:
    active = self._view_index == view_index
    if view_index == self.VIEW_COUNTRIES:
      country_count = sum(1 for entry in self._selected_entries() if entry["token"].startswith(COUNTRY_PREFIX))
      return tr("Active") if active else tr(f"{country_count} selected")
    state_count = sum(1 for entry in self._selected_entries() if not entry["token"].startswith(COUNTRY_PREFIX))
    return tr("Active") if active else tr(f"{state_count} selected")

  def _browser_source_desc(self, view_index: int) -> str:
    if view_index == self.VIEW_COUNTRIES:
      return tr("Tap continents, then choose countries")
    return tr("Choose states by region or grab the whole U.S.")

  def _browser_scope_primary(self, full_us: bool) -> str:
    if full_us:
      return tr("Active") if self._is_full_us_mode() else (tr("Included") if self._get_map_state(US_COUNTRY_TOKEN) else tr("One-tap full set"))
    return tr("Active") if not self._is_full_us_mode() else tr("Choose by region")

  def _browser_scope_desc(self, full_us: bool) -> str:
    if full_us:
      return tr("Download the entire U.S. package")
    return tr("Choose a U.S. region, then pick states")

  def _browser_title(self) -> str:
    if self._is_full_us_mode():
      return tr("United States: Whole-country set")
    group = self._active_group()
    if self._is_states_view():
      return tr(f"United States: {group['title']}")
    return tr(f"Countries: {group['title']}")

  def _browser_subtitle(self) -> str:
    if self._is_full_us_mode():
      return tr("Use the same full-width StarPilot tiles to add or remove the complete U.S. offline package.")
    if self._is_states_view():
      return tr("Choose an American region first, then add only the states you want kept offline.")
    return tr("Choose a continent first, then tap the countries you want available for offline speed-limit data.")

  def _group_section_title(self) -> str:
    return tr("Choose a U.S. region") if self._is_states_view() else tr("Choose a continent")

  def _group_section_desc(self) -> str:
    return tr("These tiles match the main StarPilot menu and keep each selection surface large and deliberate.")

  def _region_section_title(self) -> str:
    if self._is_states_view():
      return tr(f"States in {self._active_group()['title']}")
    return tr(f"Countries in {self._active_group()['title']}")

  def _region_section_desc(self) -> str:
    if self._is_states_view():
      return tr("Tap individual states, or use the action above to grab the whole region.")
    return tr("Tap countries individually, or use the action above to grab the whole continent.")

  def _get_map_state(self, token: str) -> bool:
    return token in self._selected_tokens()

  def _set_map_state(self, token: str, state: bool):
    selected = self._selected_tokens()
    if state:
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
    return self._has_downloaded_data and not self._download_in_flight() and not self._is_visually_cancelling()

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
        self._params_memory.put_bool("DownloadMaps", True)
        self._params_memory.remove("CancelDownloadMaps")
        self._download_started_at = rl.get_time()

    gui_app.push_widget(ConfirmDialog(tr("Start downloading offline maps for the selected regions?"), tr("Download"), callback=on_confirm))

  def _on_cancel(self):
    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        self._params_memory.put_bool("CancelDownloadMaps", True)
        self._params_memory.remove("DownloadMaps")
        self._cancel_requested_at = rl.get_time()
        self._cancel_visual_until = rl.get_time() + 2.5

    gui_app.push_widget(ConfirmDialog(tr("Cancel the current map download?"), tr("Cancel Download"), callback=on_confirm))

  def _has_downloaded_maps(self) -> bool:
    return self._has_downloaded_data

  def _on_remove(self):
    if not self._remove_enabled():
      return

    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        def remove_worker():
          if OFFLINE_MAPS_PATH.exists():
            shutil.rmtree(OFFLINE_MAPS_PATH, ignore_errors=True)
          self._refresh_storage_cache(force=True)

        threading.Thread(target=remove_worker, daemon=True).start()
        gui_app.push_widget(alert_dialog(tr("Offline maps removed.")))

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
    total = self._browser_card._measure_height(width)
    total += SECTION_CARD_GAP + self._selected_card._measure_height()
    return total

  def _draw_scroll_content(self, rect: rl.Rectangle, width: float):
    self._browser_card.set_parent_rect(rect)
    self._selected_card.set_parent_rect(rect)

    y = rect.y + self._scroll_offset

    browser_height = self._browser_card._measure_height(width)
    self._browser_card.render(rl.Rectangle(rect.x, y, width, browser_height))
    y += browser_height + SECTION_CARD_GAP

    self._selected_card.render(rl.Rectangle(rect.x, y, width, self._selected_card._measure_height()))

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    frame = build_list_panel_frame(rect)
    draw_list_panel_shell(frame)

    hdr = frame.header
    title_y = hdr.y + HEADER_TOP_OFFSET
    selected_text = tr(f"{self._selected_count()} selected")
    selected_chip_w = max(152, min(208, 78 + len(selected_text) * 9))
    title_w = hdr.width - selected_chip_w - 18
    gui_label(rl.Rectangle(hdr.x, title_y, title_w, HEADER_TITLE_HEIGHT), tr("Map Data"), 40, AetherListColors.HEADER, FontWeight.SEMI_BOLD)
    gui_label(rl.Rectangle(hdr.x, title_y + 40, title_w, HEADER_SUBTITLE_HEIGHT), tr("Use offline maps for speed-limit control and keep only the regions you need."), 22, AetherListColors.SUBTEXT, FontWeight.NORMAL)
    AetherChip(selected_text, rl.Color(89, 116, 151, 24), rl.Color(116, 136, 168, 48), AetherListColors.HEADER, pill=True).render(
      rl.Rectangle(hdr.x + hdr.width - selected_chip_w, title_y + 8, selected_chip_w, 34)
    )

    header_status_y = title_y + HEADER_TITLE_HEIGHT + HEADER_SUBTITLE_HEIGHT + 8
    header_status_rect = rl.Rectangle(hdr.x, header_status_y, hdr.width, hdr.y + hdr.height - header_status_y - HEADER_BOTTOM_GAP)
    self._status_card.render(header_status_rect)

    scroll_rect = frame.scroll
    content_width = scroll_rect.width - 18
    action_rect = rl.Rectangle(scroll_rect.x, scroll_rect.y, content_width, ACTION_BUTTON_HEIGHT)
    self._action_strip.set_parent_rect(scroll_rect)
    self._action_strip.render(action_rect)

    scroll_content_y = action_rect.y + ACTION_BUTTON_HEIGHT + SECTION_CARD_GAP
    scroll_content_rect = rl.Rectangle(scroll_rect.x, scroll_content_y, scroll_rect.width, max(0.0, scroll_rect.height - ACTION_BUTTON_HEIGHT - SECTION_CARD_GAP))
    self._content_height = self._measure_content_height(content_width)
    self._scroll_panel.set_enabled(self.is_visible)
    self._scroll_offset = self._scroll_panel.update(scroll_content_rect, max(self._content_height, scroll_content_rect.height))

    rl.begin_scissor_mode(int(scroll_content_rect.x), int(scroll_content_rect.y), int(scroll_content_rect.width), int(scroll_content_rect.height))
    self._draw_scroll_content(scroll_content_rect, content_width)
    rl.end_scissor_mode()

    if self._content_height > scroll_content_rect.height:
      self._scrollbar.render(scroll_content_rect, self._content_height, self._scroll_offset)

    draw_list_scroll_fades(scroll_content_rect, self._content_height, self._scroll_offset, AetherListColors.PANEL_BG)
