from __future__ import annotations
from collections.abc import Callable
import pyray as rl

from openpilot.common.params import Params
from openpilot.system.ui.widgets import Widget
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.system.ui.widgets.list_view import multiple_button_item, category_buttons_item

from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanelType, StarPilotPanelInfo
from openpilot.selfdrive.ui.layouts.settings.starpilot.sounds import StarPilotSoundsLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.driving_model import StarPilotDrivingModelLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.longitudinal import StarPilotLongitudinalLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.lateral import StarPilotLateralLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.maps import StarPilotMapsLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.navigation import StarPilotNavigationLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.data import StarPilotDataLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.device import StarPilotDeviceLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.utilities import StarPilotUtilitiesLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.visuals import StarPilotVisualsLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.themes import StarPilotThemesLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.vehicle import StarPilotVehicleSettingsLayout
from openpilot.selfdrive.ui.layouts.settings.starpilot.wheel import StarPilotWheelLayout

STARPILOT_ICONS_DIR = "toggle_icons"

class StarPilotLayout(Widget):
  CATEGORIES = [
    {
      "title": "Alerts and Sounds",
      "icon": "icon_sound.png",
      "desc": "<b>Adjust alert volumes and enable custom notifications.</b>",
      "buttons": [("MANAGE", "SOUNDS", 0)],
    },
    {
      "title": "Driving Controls",
      "icon": "icon_steering.png",
      "desc": "<b>Fine-tune custom StarPilot acceleration, braking, and steering controls.</b>",
      "buttons": [("DRIVING MODEL", "DRIVING_MODEL", 0), ("GAS / BRAKE", "LONGITUDINAL", 0), ("STEERING", "LATERAL", 0)],
    },
    {
      "title": "Navigation",
      "icon": "icon_navigate.png",
      "desc": "<b>Download map data for the Speed Limit Controller.</b>",
      "buttons": [("MAP DATA", "MAPS", 0), ("NAVIGATION", "NAVIGATION", 1)],
    },
    {
      "title": "System Settings",
      "icon": "icon_system.png",
      "desc": "<b>Manage backups, device settings, screen options, storage, and tools to keep StarPilot running smoothly.</b>",
      "buttons": [("DATA", "DATA", 0), ("DEVICE CONTROLS", "DEVICE", 2), ("UTILITIES", "UTILITIES", 0)],
    },
    {
      "title": "Theme and Appearance",
      "icon": "icon_display.png",
      "desc": "<b>Customize the look of the driving screen and interface, including themes!</b>",
      "buttons": [("APPEARANCE", "VISUALS", 0), ("THEME", "THEMES", 0)],
    },
    {
      "title": "Vehicle Settings",
      "icon": "icon_vehicle.png",
      "desc": "<b>Configure car-specific options and steering wheel button mappings.</b>",
      "buttons": [("VEHICLE SETTINGS", "VEHICLE", 0), ("WHEEL CONTROLS", "WHEEL", 1)],
    },
  ]

  def __init__(self):
    super().__init__()
    self._params = Params()

    self._current_panel = StarPilotPanelType.MAIN
    self._depth_callback: Callable | None = None
    self._settings_layout = None

    self._panel_stack: list[tuple[StarPilotPanelType, str]] = []  
    self._sub_panel_callbacks: dict[str, Callable] = {}  

    self._toggle_tuning_levels: dict[str, int] = {}
    all_keys = self._params.all_keys()
    for key in all_keys:
      level = self._params.get_tuning_level(key)
      if level is not None:
        self._toggle_tuning_levels[key] = level

    self._panels = {
      StarPilotPanelType.MAIN: StarPilotPanelInfo("", None),
      StarPilotPanelType.SOUNDS: StarPilotPanelInfo(tr_noop("Sounds"), StarPilotSoundsLayout()),
      StarPilotPanelType.DRIVING_MODEL: StarPilotPanelInfo(tr_noop("Driving Model"), StarPilotDrivingModelLayout()),
      StarPilotPanelType.LONGITUDINAL: StarPilotPanelInfo(tr_noop("Gas / Brake"), StarPilotLongitudinalLayout()),
      StarPilotPanelType.LATERAL: StarPilotPanelInfo(tr_noop("Steering"), StarPilotLateralLayout()),
      StarPilotPanelType.MAPS: StarPilotPanelInfo(tr_noop("Map Data"), StarPilotMapsLayout()),
      StarPilotPanelType.NAVIGATION: StarPilotPanelInfo(tr_noop("Navigation"), StarPilotNavigationLayout()),
      StarPilotPanelType.DATA: StarPilotPanelInfo(tr_noop("Data Management"), StarPilotDataLayout()),
      StarPilotPanelType.DEVICE: StarPilotPanelInfo(tr_noop("Device Controls"), StarPilotDeviceLayout()),
      StarPilotPanelType.UTILITIES: StarPilotPanelInfo(tr_noop("Utilities"), StarPilotUtilitiesLayout()),
      StarPilotPanelType.VISUALS: StarPilotPanelInfo(tr_noop("Appearance"), StarPilotVisualsLayout()),
      StarPilotPanelType.THEMES: StarPilotPanelInfo(tr_noop("Themes"), StarPilotThemesLayout()),
      StarPilotPanelType.VEHICLE: StarPilotPanelInfo(tr_noop("Vehicle Settings"), StarPilotVehicleSettingsLayout()),
      StarPilotPanelType.WHEEL: StarPilotPanelInfo(tr_noop("Wheel Controls"), StarPilotWheelLayout()),
    }

    self._setup_longitudinal_sub_panels()
    self._setup_sounds_sub_panels()
    self._setup_lateral_sub_panels()

    for panel_type in [
      StarPilotPanelType.SOUNDS,
      StarPilotPanelType.DRIVING_MODEL,
    ]:
      panel = self._panels[panel_type].instance
      if panel and hasattr(panel, 'set_tuning_levels'):
        panel.set_tuning_levels(self._toggle_tuning_levels)

    tuning_levels = [tr("Minimal"), tr("Standard"), tr("Advanced"), tr("Developer")]
    tuning_level_str = self._params.get("TuningLevel", return_default=True, default="1")
    current_tuning_level = int(tuning_level_str) if tuning_level_str else 1

    items = [
      multiple_button_item(
        tr_noop("Tuning Level"),
        tr_noop(
          "Choose your tuning level. Lower levels keep it simple; higher levels unlock more toggles for finer control.\n\n"
          "Minimal - Ideal for those who prefer simplicity or ease of use\n"
          "Standard - Recommended for most users for a balanced experience\n"
          "Advanced - Fine-tuning for experienced users\n"
          "Developer - Highly customizable settings for seasoned enthusiasts"
        ),
        tuning_levels,
        current_tuning_level,
        callback=self._on_tuning_level_changed,
        icon=f"{STARPILOT_ICONS_DIR}/icon_tuning.png",
        starpilot_icon=True,
      ),
    ]

    panel_type_map = {
      "SOUNDS": StarPilotPanelType.SOUNDS,
      "DRIVING_MODEL": StarPilotPanelType.DRIVING_MODEL,
      "LONGITUDINAL": StarPilotPanelType.LONGITUDINAL,
      "LATERAL": StarPilotPanelType.LATERAL,
      "MAPS": StarPilotPanelType.MAPS,
      "NAVIGATION": StarPilotPanelType.NAVIGATION,
      "DATA": StarPilotPanelType.DATA,
      "DEVICE": StarPilotPanelType.DEVICE,
      "UTILITIES": StarPilotPanelType.UTILITIES,
      "VISUALS": StarPilotPanelType.VISUALS,
      "THEMES": StarPilotPanelType.THEMES,
      "VEHICLE": StarPilotPanelType.VEHICLE,
      "WHEEL": StarPilotPanelType.WHEEL,
    }

    for cat in self.CATEGORIES:
      filtered_buttons = []
      for btn_label, panel_key, min_level in cat["buttons"]:
        if current_tuning_level >= min_level:
          panel_type = panel_type_map[panel_key]
          callback = lambda p=panel_type: self._set_current_panel(p)
          filtered_buttons.append((tr(btn_label), callback))

      if filtered_buttons:
        full_icon_path = f"{STARPILOT_ICONS_DIR}/{cat['icon']}"
        item = category_buttons_item(
          title=tr(cat["title"]),
          buttons=filtered_buttons,
          description=tr(cat["desc"]),
          icon=full_icon_path,
          starpilot_icon=True,
        )
        items.append(item)

    self._main_scroller = Scroller(items, line_separator=True, spacing=0)

  def set_depth_callback(self, callback: Callable):
    self._depth_callback = callback

  def set_settings_layout(self, settings_layout):
    self._settings_layout = settings_layout

  def navigate_back(self):
    if self._panel_stack:
      self._panel_stack.pop()
    if self._panel_stack:
      self._update_sub_panel_visibility()
    else:
      self._set_current_panel(StarPilotPanelType.MAIN)

  def _push_sub_panel(self, sub_panel_name: str):
    self._panel_stack.append((self._current_panel, sub_panel_name))
    self._update_sub_panel_visibility()

  def _update_sub_panel_visibility(self):
    if self._current_panel == StarPilotPanelType.LONGITUDINAL:
      longitudinal = self._panels[StarPilotPanelType.LONGITUDINAL].instance
      if longitudinal:
        current_sub = self._get_current_sub_panel()
        if hasattr(longitudinal, 'set_current_sub_panel'):
          longitudinal.set_current_sub_panel(current_sub)
    elif self._current_panel == StarPilotPanelType.SOUNDS:
      sounds = self._panels[StarPilotPanelType.SOUNDS].instance
      if sounds:
        current_sub = self._get_current_sub_panel()
        if hasattr(sounds, '_navigate_to'):
          sounds._current_sub_panel = current_sub

  def _get_current_sub_panel(self) -> str:
    if self._panel_stack and self._panel_stack[-1][0] == self._current_panel:
      return self._panel_stack[-1][1]
    return ""

  def _setup_longitudinal_sub_panels(self):
    longitudinal = self._panels[StarPilotPanelType.LONGITUDINAL].instance
    if longitudinal and hasattr(longitudinal, 'set_navigate_callback'):
      longitudinal.set_navigate_callback(self._push_sub_panel)

  def _setup_sounds_sub_panels(self):
    sounds = self._panels[StarPilotPanelType.SOUNDS].instance
    if sounds and hasattr(sounds, 'set_navigate_callback'):
      sounds.set_navigate_callback(self._push_sub_panel)

  def _setup_lateral_sub_panels(self):
    lateral = self._panels[StarPilotPanelType.LATERAL].instance
    if lateral and hasattr(lateral, 'set_navigate_callback'):
      lateral.set_navigate_callback(self._push_sub_panel)

  def _on_tuning_level_changed(self, index: int):
    self._params.put_nonblocking("TuningLevel", index)
    if self._settings_layout:
      self._settings_layout.refresh_developer_visibility()
    for panel_info in self._panels.values():
      panel = panel_info.instance
      if panel and hasattr(panel, 'refresh_visibility'):
        panel.refresh_visibility()
    self._rebuild_main_scroller(index)

  def _rebuild_main_scroller(self, tuning_level: int):
    tuning_levels = [tr("Minimal"), tr("Standard"), tr("Advanced"), tr("Developer")]

    items = [
      multiple_button_item(
        tr_noop("Tuning Level"),
        tr_noop(
          "Choose your tuning level. Lower levels keep it simple; higher levels unlock more toggles for finer control.\n\n"
          "Minimal - Ideal for those who prefer simplicity or ease of use\n"
          "Standard - Recommended for most users for a balanced experience\n"
          "Advanced - Fine-tuning for experienced users\n"
          "Developer - Highly customizable settings for seasoned enthusiasts"
        ),
        tuning_levels,
        tuning_level,
        callback=self._on_tuning_level_changed,
        icon=f"{STARPILOT_ICONS_DIR}/icon_tuning.png",
        starpilot_icon=True,
      ),
    ]

    panel_type_map = {
      "SOUNDS": StarPilotPanelType.SOUNDS,
      "DRIVING_MODEL": StarPilotPanelType.DRIVING_MODEL,
      "LONGITUDINAL": StarPilotPanelType.LONGITUDINAL,
      "LATERAL": StarPilotPanelType.LATERAL,
      "MAPS": StarPilotPanelType.MAPS,
      "NAVIGATION": StarPilotPanelType.NAVIGATION,
      "DATA": StarPilotPanelType.DATA,
      "DEVICE": StarPilotPanelType.DEVICE,
      "UTILITIES": StarPilotPanelType.UTILITIES,
      "VISUALS": StarPilotPanelType.VISUALS,
      "THEMES": StarPilotPanelType.THEMES,
      "VEHICLE": StarPilotPanelType.VEHICLE,
      "WHEEL": StarPilotPanelType.WHEEL,
    }

    for cat in self.CATEGORIES:
      filtered_buttons = []
      for btn_label, panel_key, min_level in cat["buttons"]:
        if tuning_level >= min_level:
          panel_type = panel_type_map[panel_key]
          callback = lambda p=panel_type: self._set_current_panel(p)
          filtered_buttons.append((tr(btn_label), callback))

      if filtered_buttons:
        full_icon_path = f"{STARPILOT_ICONS_DIR}/{cat['icon']}"
        item = category_buttons_item(
          title=tr(cat["title"]),
          buttons=filtered_buttons,
          description=tr(cat["desc"]),
          icon=full_icon_path,
          starpilot_icon=True,
        )
        items.append(item)

    self._main_scroller = Scroller(items, line_separator=True, spacing=0)

  def _set_current_panel(self, panel_type: StarPilotPanelType):
    if panel_type != self._current_panel:
      if self._current_panel != StarPilotPanelType.MAIN:
        self._panels[self._current_panel].instance.hide_event()
      self._current_panel = panel_type
      if panel_type != StarPilotPanelType.MAIN:
        self._panels[panel_type].instance.show_event()

    depth = 1 if panel_type != StarPilotPanelType.MAIN else 0
    if self._depth_callback:
      self._depth_callback(depth)

  def _render(self, rect: rl.Rectangle):
    if self._current_panel == StarPilotPanelType.MAIN:
      self._main_scroller.render(rect)
    else:
      panel = self._panels[self._current_panel]
      if panel.instance:
        panel.instance.render(rect)

  def show_event(self):
    super().show_event()
    if self._current_panel == StarPilotPanelType.MAIN:
      self._main_scroller.show_event()
    else:
      self._panels[self._current_panel].instance.show_event()

  def hide_event(self):
    super().hide_event()
    if self._current_panel != StarPilotPanelType.MAIN:
      self._panels[self._current_panel].instance.hide_event()
