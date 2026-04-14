from __future__ import annotations
from collections.abc import Callable
from dataclasses import dataclass
from enum import IntEnum

import pyray as rl

from openpilot.common.params import Params
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.widgets import Widget
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import TileGrid, HubTile, ToggleTile, ValueTile, SliderTile, SPACING
from openpilot.selfdrive.ui.layouts.settings.starpilot.sectioned_panel import SectionedTileLayout, TileSection


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
    SYSTEM = 14


@dataclass
class StarPilotPanelInfo:
    name: str
    instance: Widget


class StarPilotPanel(Widget):
    def __init__(self):
        super().__init__()
        self._params = Params()
        self._params_memory = Params(memory=True)
        self._navigate_callback: Callable | None = None
        self._back_callback: Callable | None = None
        self._current_sub_panel = ""
        self._sub_panels: dict[str, Widget] = {}
        self._scroller = None
        self._tile_grid = None
        self._sectioned_grid = None
        self.CATEGORIES = []
        self.SECTIONS = []

    def set_navigate_callback(self, callback: Callable):
        self._navigate_callback = callback

    def set_back_callback(self, callback: Callable):
        self._back_callback = callback

    def set_current_sub_panel(self, sub_panel: str):
        self._current_sub_panel = sub_panel

    def _is_category_visible(self, cat: dict) -> bool:
        visible_fn = cat.get("visible")
        return visible_fn is None or visible_fn()

    def _build_tile(self, cat: dict) -> Widget | None:
        tile_type = cat.get("type", "hub")
        if tile_type == "hub":
            on_click = cat.get("on_click")
            if on_click is None:
                on_click = lambda c=cat: self._navigate_to(c["panel"])

            return HubTile(
                title=tr(cat["title"]),
                desc=tr(cat.get("desc", "")),
                icon_path=cat.get("icon"),
                on_click=on_click,
                starpilot_icon=cat.get("starpilot_icon", True),
                bg_color=cat.get("color"),
                get_status=cat.get("get_status"),
            )

        if tile_type == "toggle":
            raw_set_state = cat["set_state"]

            def on_toggle(state: bool, setter=raw_set_state):
                setter(state)
                self._rebuild_grid()

            return ToggleTile(title=tr(cat["title"]), get_state=cat["get_state"], set_state=on_toggle, icon_path=cat.get("icon"), bg_color=cat.get("color"), desc=tr(cat.get("desc", "")), is_enabled=cat.get("is_enabled"), disabled_label=cat.get("disabled_label", ""))

        if tile_type == "value":
            return ValueTile(title=tr(cat["title"]), get_value=cat["get_value"], on_click=cat["on_click"], icon_path=cat.get("icon"), bg_color=cat.get("color"), is_enabled=cat.get("is_enabled"), desc=tr(cat.get("desc", "")))

        if tile_type == "slider":
            return SliderTile(
                title=tr(cat["title"]),
                get_value=cat["get_value"],
                set_value=cat["set_value"],
                min_val=cat["min_val"],
                max_val=cat["max_val"],
                step=cat["step"],
                unit=cat.get("unit", ""),
                labels=cat.get("labels", {}),
                icon_path=cat.get("icon"),
                bg_color=cat.get("color"),
                is_enabled=cat.get("is_enabled"),
                desc=tr(cat.get("desc", "")),
                on_test=cat.get("on_test"),
            )

        return None

    def _build_tile_grid(self, categories: list[dict], columns: int | None = None, padding: int | None = None, uniform_width: bool = False) -> TileGrid:
        grid = TileGrid(columns=columns, padding=padding, uniform_width=uniform_width)
        for cat in categories:
            if not self._is_category_visible(cat):
                continue
            tile = self._build_tile(cat)
            if tile is not None:
                grid.add_tile(tile)
        return grid

    def _rebuild_grid(self):
        if self.SECTIONS:
            if self._sectioned_grid is None:
                self._sectioned_grid = SectionedTileLayout()

            sections: list[TileSection] = []
            for section in self.SECTIONS:
                visible_fn = section.get("visible")
                if visible_fn is not None and not visible_fn():
                    continue

                grid = self._build_tile_grid(
                    section.get("categories", []),
                    columns=section.get("columns", 2),
                    padding=section.get("padding", SPACING.tile_gap),
                    uniform_width=section.get("uniform_width", True),
                )
                if grid.tiles:
                    sections.append(TileSection(tr(section["title"]), grid))

            self._sectioned_grid.set_sections(sections)
            return

        if not self.CATEGORIES:
            return

        if self._tile_grid is None:
            self._tile_grid = TileGrid(columns=None, padding=SPACING.tile_gap)

        self._tile_grid.clear()

        for cat in self.CATEGORIES:
            if not self._is_category_visible(cat):
                continue
            tile = self._build_tile(cat)
            if tile is not None:
                self._tile_grid.add_tile(tile)

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
        elif self.SECTIONS and self._sectioned_grid:
            self._sectioned_grid.render(rect)
        elif self.CATEGORIES and self._tile_grid:
            self._tile_grid.render(rect)
        elif self._scroller:
            self._scroller.render(rect)

    def show_event(self):
        super().show_event()
        self._rebuild_grid()
        if self._current_sub_panel and self._current_sub_panel in self._sub_panels:
            self._sub_panels[self._current_sub_panel].show_event()
        elif self.SECTIONS and self._sectioned_grid:
            self._sectioned_grid.show_event()
        elif self._scroller:
            self._scroller.show_event()

    def hide_event(self):
        super().hide_event()
        if self._current_sub_panel and self._current_sub_panel in self._sub_panels:
            self._sub_panels[self._current_sub_panel].hide_event()
        elif self.SECTIONS and self._sectioned_grid:
            self._sectioned_grid.hide_event()
        elif self._scroller:
            self._scroller.hide_event()


def create_tile_panel(categories: list[dict], sub_panels: dict[str, Widget] | None = None) -> StarPilotPanel:
    panel = StarPilotPanel()
    panel.CATEGORIES = categories
    panel._sub_panels = sub_panels or {}
    panel._tile_grid = TileGrid(columns=2, padding=SPACING.tile_gap, uniform_width=True)

    for name, child in panel._sub_panels.items():
        if hasattr(child, 'set_navigate_callback'):
            child.set_navigate_callback(panel._navigate_to)
        if hasattr(child, 'set_back_callback'):
            child.set_back_callback(panel._go_back)

    panel._rebuild_grid()
    return panel


def create_master_toggle_panel(toggle_specs: list[dict], sub_panels: dict[str, Widget] | None = None,
                                extra_categories: list[dict] | None = None) -> StarPilotPanel:
    panel = create_tile_panel([], sub_panels)
    categories: list[dict] = []

    for spec in toggle_specs:
        get_state = spec["get_state"]
        visible = spec.get("visible")
        manage_enabled = spec.get("manage_enabled", get_state)

        categories.append({
            "title": spec["title"],
            "desc": spec.get("desc", ""),
            "type": "toggle",
            "get_state": get_state,
            "set_state": spec["set_state"],
            "icon": spec.get("icon"),
            "color": spec.get("color"),
            "visible": visible,
        })

        categories.append({
            "title": spec.get("manage_title", "Settings"),
            "desc": spec.get("manage_desc", ""),
            "type": "value",
            "get_value": lambda enabled=get_state, active_label=spec.get("manage_label", "Manage"), inactive_label=spec.get("disabled_label", "Enable First"): tr(active_label) if enabled() else tr(inactive_label),
            "on_click": lambda sub_panel=spec["panel"]: panel._navigate_to(sub_panel),
            "is_enabled": manage_enabled,
            "icon": spec.get("manage_icon", spec.get("icon")),
            "color": spec.get("color"),
            "visible": visible,
        })

    panel.CATEGORIES = categories + list(extra_categories or [])
    panel._rebuild_grid()
    return panel
