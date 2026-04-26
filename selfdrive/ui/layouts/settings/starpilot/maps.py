from __future__ import annotations

import shutil
from pathlib import Path

import pyray as rl

from openpilot.system.ui.lib.application import FontWeight, MousePos, gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.lib.scroll_panel2 import GuiScrollPanel2
from openpilot.system.ui.widgets import DialogResult, Widget
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog, alert_dialog
from openpilot.system.ui.widgets.label import gui_label
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog

from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import (
  AetherChip,
  AetherScrollbar,
  HubTile,
  TileGrid,
  ToggleTile,
  ValueTile,
  build_list_panel_frame,
  draw_action_pill,
  draw_list_panel_shell,
  draw_list_scroll_fades,
  draw_soft_card,
)
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.starpilot.common.maps_catalog import MAPS_CATALOG, MAP_SCHEDULE_LABELS, sanitize_selected_locations_csv, schedule_label, schedule_param_value


TOP_ACTION_ROW_HEIGHT = 132
GROUP_HEADER_HEIGHT = 52
GROUP_BODY_GAP = 14
GROUP_CARD_GAP = 18
GROUP_TILE_HEIGHT = 124


class MapGroupCard(Widget):
  def __init__(self, controller: "StarPilotMapsLayout", section_title: str, group: dict):
    super().__init__()
    self._controller = controller
    self._title = group["title"]
    self._regions = group["regions"]
    self._tile_grid = self._child(TileGrid(columns=None, padding=14, uniform_width=True))
    self._action_rect = rl.Rectangle(0, 0, 0, 0)
    self._action_pressed = False

    for region in self._regions:
      token = region["token"]
      self._tile_grid.add_tile(
        ToggleTile(
          title=tr_noop(region["label"]),
          get_state=lambda token=token: self._controller._get_map_state(token),
          set_state=lambda state, token=token: self._controller._set_map_state(token, state),
          bg_color="#68ACA3",
        )
      )

  def _selected_count(self) -> int:
    return sum(1 for region in self._regions if self._controller._get_map_state(region["token"]))

  def _all_selected(self) -> bool:
    return self._selected_count() == len(self._regions)

  def _toggle_all(self):
    state = not self._all_selected()
    for region in self._regions:
      self._controller._set_map_state(region["token"], state)

  def _measure_height(self, width: float) -> float:
    content_w = max(0.0, width - 32)
    rows = self._tile_grid.get_row_count(available_width=content_w)
    body_h = rows * GROUP_TILE_HEIGHT + self._tile_grid.gap * max(0, rows - 1)
    return GROUP_HEADER_HEIGHT + GROUP_BODY_GAP + body_h + 28

  def _handle_mouse_press(self, mouse_pos: MousePos):
    if rl.check_collision_point_rec(mouse_pos, self._action_rect):
      self._action_pressed = True

  def _handle_mouse_release(self, mouse_pos: MousePos):
    if self._action_pressed:
      self._action_pressed = False
      if rl.check_collision_point_rec(mouse_pos, self._action_rect):
        self._toggle_all()

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    draw_soft_card(rect, rl.Color(255, 255, 255, 4), rl.Color(255, 255, 255, 15), radius=0.08, segments=18)

    header_rect = rl.Rectangle(rect.x + 16, rect.y + 12, rect.width - 32, GROUP_HEADER_HEIGHT)
    title_w = max(0.0, header_rect.width - 160)
    title_rect = rl.Rectangle(header_rect.x, header_rect.y, title_w, 28)
    gui_label(title_rect, self._title, 28, rl.Color(236, 242, 250, 255), FontWeight.SEMI_BOLD)

    count = self._selected_count()
    count_label = tr(f"{count}/{len(self._regions)} selected")
    chip = AetherChip(count_label, rl.Color(89, 116, 151, 26), rl.Color(116, 136, 168, 52), rl.Color(236, 242, 250, 255), pill=True)
    chip_w = min(136, max(96, header_rect.width * 0.26))
    chip_rect = rl.Rectangle(header_rect.x + max(0.0, header_rect.width - chip_w - 68), header_rect.y + 2, chip_w, 34)
    chip.render(chip_rect)

    action_text = tr("Clear All") if self._all_selected() else tr("Select All")
    self._action_rect = rl.Rectangle(header_rect.x + header_rect.width - 60, header_rect.y + 2, 60, 34)
    draw_action_pill(self._action_rect, action_text, rl.Color(89, 116, 151, 20), rl.Color(255, 255, 255, 26), rl.Color(236, 242, 250, 255), font_size=16)

    content_x = rect.x + 16
    content_y = header_rect.y + GROUP_HEADER_HEIGHT + GROUP_BODY_GAP
    content_w = rect.width - 32
    rows = self._tile_grid.get_row_count(available_width=content_w)
    body_h = rows * GROUP_TILE_HEIGHT + self._tile_grid.gap * max(0, rows - 1)
    grid_rect = rl.Rectangle(content_x, content_y, content_w, body_h)
    self._tile_grid.set_parent_rect(rect)
    self._tile_grid.render(grid_rect)


class StarPilotMapsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._scroll_panel = GuiScrollPanel2(horizontal=False)
    self._scrollbar = AetherScrollbar()
    self._content_height = 0.0
    self._scroll_offset = 0.0
    self._selected_count = 0
    self._storage_text = "0 MB"
    self._storage_updated_at = 0.0

    self._action_grid = TileGrid(columns=None, padding=16, uniform_width=True)
    self._action_tiles = [
      HubTile(
        title=tr_noop("Download Maps"),
        desc=tr_noop("Download the selected regions."),
        icon_path="toggle_icons/icon_map.png",
        on_click=self._on_download,
        starpilot_icon=True,
        bg_color="#68ACA3",
      ),
      ValueTile(
        title=tr_noop("Auto Update Schedule"),
        get_value=lambda: schedule_label(self._params.get("PreferredSchedule")),
        on_click=self._on_schedule,
        icon_path="toggle_icons/icon_calendar.png",
        bg_color="#68ACA3",
      ),
      ValueTile(
        title=tr_noop("Storage Used"),
        get_value=self._get_storage,
        on_click=lambda: None,
        icon_path="toggle_icons/icon_system.png",
        bg_color="#68ACA3",
      ),
      HubTile(
        title=tr_noop("Remove Maps"),
        desc=tr_noop("Delete downloaded map files."),
        icon_path="toggle_icons/icon_map.png",
        on_click=self._on_remove,
        starpilot_icon=True,
        bg_color="#A64D5A",
      ),
    ]
    for tile in self._action_tiles:
      self._action_grid.add_tile(self._child(tile))

    self._sections: list[tuple[str, list[MapGroupCard]]] = []
    for section in MAPS_CATALOG:
      cards = [self._child(MapGroupCard(self, section["title"], group)) for group in section["groups"]]
      self._sections.append((section["title"], cards))

    self._refresh_storage_cache(force=True)
    self._sync_selected_count()

  def show_event(self):
    super().show_event()
    self._scroll_offset = 0.0
    self._refresh_storage_cache(force=True)
    self._sync_selected_count()

  def hide_event(self):
    super().hide_event()
    self._scroll_offset = 0.0

  def _refresh_storage_cache(self, force: bool = False):
    now = rl.get_time()
    if not force and (now - self._storage_updated_at) < 5.0:
      return
    self._storage_text = self._calculate_storage_used()
    self._storage_updated_at = now

  def _sync_selected_count(self):
    selected = set(sanitize_selected_locations_csv(self._params.get("MapsSelected", encoding="utf-8") or "").split(","))
    selected.discard("")
    self._selected_count = len(selected)

  def _get_map_state(self, token: str) -> bool:
    selected = set(sanitize_selected_locations_csv(self._params.get("MapsSelected", encoding="utf-8") or "").split(","))
    selected.discard("")
    return token in selected

  def _set_map_state(self, token: str, state: bool):
    selected = set(sanitize_selected_locations_csv(self._params.get("MapsSelected", encoding="utf-8") or "").split(","))
    selected.discard("")
    if state:
      selected.add(token)
    else:
      selected.discard(token)
    self._params.put("MapsSelected", sanitize_selected_locations_csv(sorted(selected)))
    self._sync_selected_count()

  def _calculate_storage_used(self) -> str:
    maps_path = Path("/data/media/0/osm/offline")
    if not maps_path.exists():
      return "0 MB"
    total_size = sum(f.stat().st_size for f in maps_path.rglob("*") if f.is_file())
    mb = total_size / (1024 * 1024)
    if mb > 1024:
      return f"{(mb / 1024):.2f} GB"
    return f"{mb:.2f} MB"

  def _get_storage(self) -> str:
    self._refresh_storage_cache()
    return self._storage_text

  def _on_schedule(self):
    options = list(MAP_SCHEDULE_LABELS.values())
    current = schedule_label(self._params.get("PreferredSchedule"))
    dialog = MultiOptionDialog(tr("Auto Update Schedule"), options, current)

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put("PreferredSchedule", schedule_param_value(dialog.selection))

    gui_app.push_widget(dialog, callback=on_select)

  def _on_download(self):
    current_selected = self._params.get("MapsSelected", encoding="utf-8") or ""
    selected_raw = sanitize_selected_locations_csv(current_selected)
    if selected_raw != current_selected:
      self._params.put("MapsSelected", selected_raw)
    if not selected_raw:
      gui_app.push_widget(alert_dialog(tr("Please select at least one region or state first!")))
      return

    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        self._params_memory.put_bool("DownloadMaps", True)
        gui_app.push_widget(alert_dialog(tr("Map download started in background.")))

    gui_app.push_widget(ConfirmDialog(tr("Start downloading maps for selected regions?"), tr("Download"), on_close=on_confirm))

  def _on_remove(self):
    def on_confirm(res):
      if res == DialogResult.CONFIRM:
        maps_path = Path("/data/media/0/osm/offline")
        if maps_path.exists():
          shutil.rmtree(maps_path, ignore_errors=True)
        gui_app.push_widget(alert_dialog(tr("Maps removed.")))
        self._refresh_storage_cache(force=True)

    gui_app.push_widget(ConfirmDialog(tr("Delete all downloaded map data?"), tr("Remove"), on_close=on_confirm))

  def _measure_content_height(self, width: float) -> float:
    action_rows = self._action_grid.get_row_count(available_width=width)
    total = action_rows * TOP_ACTION_ROW_HEIGHT + self._action_grid.gap * max(0, action_rows - 1)
    total += 30

    for _section_title, cards in self._sections:
      total += 34 + GROUP_CARD_GAP
      for card in cards:
        total += card._measure_height(width) + GROUP_CARD_GAP
      total += 12

    return total

  def _draw_section_title(self, rect: rl.Rectangle, title: str):
    gui_label(rect, title, 26, rl.Color(236, 242, 250, 230), FontWeight.SEMI_BOLD)

  def _draw_scroll_content(self, rect: rl.Rectangle, width: float):
    y = rect.y + self._scroll_offset

    action_rows = self._action_grid.get_row_count(available_width=width)
    action_h = action_rows * TOP_ACTION_ROW_HEIGHT + self._action_grid.gap * max(0, action_rows - 1)
    self._action_grid.render(rl.Rectangle(rect.x, y, width, action_h))
    y += action_h + 30

    for section_title, cards in self._sections:
      title_rect = rl.Rectangle(rect.x, y, width, 34)
      self._draw_section_title(title_rect, section_title)
      y += 34 + GROUP_CARD_GAP
      for card in cards:
        card_h = card._measure_height(width)
        card.render(rl.Rectangle(rect.x, y, width, card_h))
        y += card_h + GROUP_CARD_GAP
      y += 12

  def _render(self, rect: rl.Rectangle):
    self.set_rect(rect)
    frame = build_list_panel_frame(rect)
    draw_list_panel_shell(frame)

    hdr = frame.header
    chip_w = min(140, max(104, int(hdr.width * 0.18)))
    title_w = max(0, hdr.width - chip_w - 20)
    gui_label(rl.Rectangle(hdr.x, hdr.y + 4, title_w, 40), tr("Map Data"), 40, rl.Color(236, 242, 250, 255), FontWeight.SEMI_BOLD)
    gui_label(
      rl.Rectangle(hdr.x, hdr.y + 48, title_w, 36),
      tr("Select regions, schedule updates, and manage offline map storage."),
      24,
      rl.Color(164, 177, 196, 255),
      FontWeight.NORMAL,
    )
    chip = AetherChip(tr(f"{self._selected_count} selected"), rl.Color(89, 116, 151, 26), rl.Color(116, 136, 168, 52), rl.Color(236, 242, 250, 255), pill=True)
    chip.render(rl.Rectangle(hdr.x + max(0.0, hdr.width - chip_w), hdr.y + 12, chip_w, 34))

    scroll_rect = frame.scroll
    content_width = scroll_rect.width - 18
    self._content_height = self._measure_content_height(content_width)
    self._scroll_panel.set_enabled(self.is_visible)
    self._scroll_offset = self._scroll_panel.update(scroll_rect, max(self._content_height, scroll_rect.height))

    rl.begin_scissor_mode(int(scroll_rect.x), int(scroll_rect.y), int(scroll_rect.width), int(scroll_rect.height))
    self._draw_scroll_content(scroll_rect, content_width)
    rl.end_scissor_mode()

    if self._content_height > scroll_rect.height:
      self._scrollbar.render(scroll_rect, self._content_height, self._scroll_offset)

    draw_list_scroll_fades(scroll_rect, self._content_height, self._scroll_offset, rl.Color(8, 8, 10, 255))
