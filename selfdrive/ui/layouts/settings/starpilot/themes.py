from __future__ import annotations
from pathlib import Path
import re

from openpilot.system.hardware import HARDWARE
from openpilot.system.hardware.hw import Paths
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

if HARDWARE.get_device_type() == "pc":
  THEME_SAVE_PATH = Path(Paths.comma_home()) / "starpilot" / "data" / "themes"
else:
  THEME_SAVE_PATH = Path("/data/themes")

HOLIDAY_THEME_NAMES = {
  "new_years": "New Year's",
  "valentines_day": "Valentine's Day",
  "st_patricks_day": "St. Patrick's Day",
  "world_frog_day": "World Frog Day",
  "april_fools": "April Fools",
  "easter_week": "Easter",
  "may_the_fourth": "May the Fourth",
  "cinco_de_mayo": "Cinco de Mayo",
  "stitch_day": "Stitch Day",
  "fourth_of_july": "Fourth of July",
  "halloween_week": "Halloween",
  "thanksgiving_week": "Thanksgiving",
  "christmas_week": "Christmas",
}

THEME_KEY_CONFIG = {
  "BootLogo": {
    "default": "starpilot",
    "kind": "files",
    "path": THEME_SAVE_PATH / "bootlogos",
    "extra": [],
  },
  "ColorScheme": {
    "default": "stock",
    "kind": "themes",
    "path": THEME_SAVE_PATH / "theme_packs",
    "subfolder": "colors",
    "extra": [("stock", "Stock"), *HOLIDAY_THEME_NAMES.items()],
  },
  "DistanceIconPack": {
    "default": "stock",
    "kind": "themes",
    "path": THEME_SAVE_PATH / "theme_packs",
    "subfolder": "distance_icons",
    "extra": [("stock", "Stock"), *HOLIDAY_THEME_NAMES.items()],
  },
  "IconPack": {
    "default": "stock",
    "kind": "themes",
    "path": THEME_SAVE_PATH / "theme_packs",
    "subfolder": "icons",
    "extra": [("stock", "Stock"), *HOLIDAY_THEME_NAMES.items()],
  },
  "SignalAnimation": {
    "default": "stock",
    "kind": "themes",
    "path": THEME_SAVE_PATH / "theme_packs",
    "subfolder": "signals",
    "extra": [("none", "None"), *HOLIDAY_THEME_NAMES.items()],
  },
  "SoundPack": {
    "default": "stock",
    "kind": "themes",
    "path": THEME_SAVE_PATH / "theme_packs",
    "subfolder": "sounds",
    "extra": [("stock", "Stock"), *HOLIDAY_THEME_NAMES.items()],
  },
  "WheelIcon": {
    "default": "stock",
    "kind": "files",
    "path": THEME_SAVE_PATH / "steering_wheels",
    "extra": [("none", "None"), ("stock", "Stock"), *HOLIDAY_THEME_NAMES.items()],
  },
}


class StarPilotThemesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    self._sub_panels = {
      "personalize": StarPilotPersonalizeLayout(),
    }

    self.CATEGORIES = [
      {
        "title": tr_noop("Custom Themes"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CustomThemes"),
        "set_state": lambda s: self._params.put_bool("CustomThemes", s),
        "icon": "toggle_icons/icon_frog.png",
        "color": "#542A71",
      },
      {
        "title": tr_noop("Personalize openpilot"),
        "panel": "personalize",
        "icon": "toggle_icons/icon_frog.png",
        "color": "#542A71",
        "desc": tr_noop("Customize the overall look and feel."),
      },
      {
        "title": tr_noop("Holiday Themes"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("HolidayThemes"),
        "set_state": lambda s: self._params.put_bool("HolidayThemes", s),
        "icon": "toggle_icons/icon_calendar.png",
        "color": "#542A71",
      },
      {
        "title": tr_noop("Rainbow Path"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("RainbowPath"),
        "set_state": lambda s: self._params.put_bool("RainbowPath", s),
        "icon": "toggle_icons/icon_rainbow.png",
        "color": "#542A71",
      },
      {
        "title": tr_noop("Random Events"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("RandomEvents"),
        "set_state": lambda s: self._params.put_bool("RandomEvents", s),
        "icon": "toggle_icons/icon_random.png",
        "color": "#542A71",
      },
      {
        "title": tr_noop("Random Themes"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("RandomThemes"),
        "set_state": lambda s: self._params.put_bool("RandomThemes", s),
        "icon": "toggle_icons/icon_random_themes.png",
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {"title": tr_noop("Startup Alert"), "type": "hub", "on_click": self._on_startup_alert, "color": "#542A71"},
    ]

    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

    self._rebuild_grid()

  def _on_startup_alert(self):
    options = ["Stock", "StarPilot", "Clear"]
    current_top = self._params.get("StartupMessageTop", encoding='utf-8') or ""
    if current_top == "Be ready to take over at any time":
      current = "Stock"
    elif current_top == "Hop in and buckle up!":
      current = "StarPilot"
    else:
      current = "Clear"

    def on_select(res, val):
      if res == DialogResult.CONFIRM:
        if val == "Stock":
          self._params.put("StartupMessageTop", "Be ready to take over at any time")
          self._params.put("StartupMessageBottom", "Always keep hands on wheel and eyes on road")
        elif val == "StarPilot":
          self._params.put("StartupMessageTop", "Hop in and buckle up!")
          self._params.put("StartupMessageBottom", "Human-tested, frog-approved")
        else:
          self._params.remove("StartupMessageTop")
          self._params.remove("StartupMessageBottom")

    gui_app.set_modal_overlay(SelectionDialog(tr("Startup Alert"), options, current, on_close=on_select))


class StarPilotPersonalizeLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Custom Themes"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("CustomThemes"),
        "set_state": lambda s: self._params.put_bool("CustomThemes", s),
        "color": "#542A71",
      },
      {
        "title": tr_noop("Boot Logo"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("BootLogo"),
        "on_click": lambda: self._show_theme_selector("BootLogo"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {
        "title": tr_noop("Color Scheme"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("ColorScheme"),
        "on_click": lambda: self._show_theme_selector("ColorScheme"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {
        "title": tr_noop("Distance Icons"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("DistanceIconPack"),
        "on_click": lambda: self._show_theme_selector("DistanceIconPack"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {
        "title": tr_noop("Icon Pack"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("IconPack"),
        "on_click": lambda: self._show_theme_selector("IconPack"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {
        "title": tr_noop("Turn Signals"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("SignalAnimation"),
        "on_click": lambda: self._show_theme_selector("SignalAnimation"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {
        "title": tr_noop("Sound Pack"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("SoundPack"),
        "on_click": lambda: self._show_theme_selector("SoundPack"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
      {
        "title": tr_noop("Steering Wheel"),
        "type": "value",
        "get_value": lambda: self._get_theme_value("WheelIcon"),
        "on_click": lambda: self._show_theme_selector("WheelIcon"),
        "color": "#542A71",
        "visible": lambda: self._params.get_bool("CustomThemes"),
      },
    ]
    self._rebuild_grid()

  @staticmethod
  def _display_name(value: str) -> str:
    if not value:
      return "Stock"

    lowered = value.lower()
    if lowered in HOLIDAY_THEME_NAMES:
      return HOLIDAY_THEME_NAMES[lowered]
    if lowered == "stock":
      return "Stock"
    if lowered == "none":
      return "None"

    base, creator = (value.split("~", 1) + [""])[:2] if "~" in value else (value, "")
    user_created_suffixes = ("-user_created", "_user_created", "-user-created", "_user-created")
    user_created = False
    for suffix in user_created_suffixes:
      if base.endswith(suffix):
        base = base[:-len(suffix)]
        user_created = True
        break

    parts = [part for part in re.split(r"[-_]+", base) if part]
    display = " ".join(part[:1].upper() + part[1:] for part in parts) if parts else value
    if user_created:
      display += " (User Created)"
    if creator:
      display += f" - by: {creator}"
    return display

  def _get_downloaded_slugs(self, key: str) -> list[str]:
    config = THEME_KEY_CONFIG[key]
    path = config["path"]
    if not path.exists():
      return []

    slugs = set()
    if config["kind"] == "files":
      for entry in path.iterdir():
        if entry.is_file():
          slugs.add(entry.stem)
    else:
      subfolder = config["subfolder"]
      for entry in path.iterdir():
        if entry.is_dir() and (entry / subfolder).exists():
          slugs.add(entry.name)

    return sorted(slugs, key=str.casefold)

  def _build_theme_options(self, key: str) -> tuple[list[str], dict[str, str], str]:
    config = THEME_KEY_CONFIG[key]
    current_slug = self._params.get(key, encoding='utf-8') or config["default"]

    options_map = {}
    for slug in self._get_downloaded_slugs(key):
      display = self._display_name(slug)
      if display not in options_map:
        options_map[display] = slug

    for slug, display in config["extra"]:
      options_map[display] = slug

    current_display = self._display_name(current_slug)
    if current_display not in options_map:
      options_map[current_display] = current_slug

    options = sorted(options_map.keys(), key=str.casefold)
    return options, options_map, current_display

  def _get_theme_value(self, key: str) -> str:
    default = THEME_KEY_CONFIG[key]["default"]
    return self._display_name(self._params.get(key, encoding='utf-8') or default)

  def _show_theme_selector(self, key):
    themes, option_map, current = self._build_theme_options(key)
    if not themes:
      return

    def on_select(res, val):
      if res == DialogResult.CONFIRM:
        selected_slug = option_map.get(val)
        if selected_slug is None:
          return
        self._params.put(key, selected_slug)
        self._rebuild_grid()

    gui_app.set_modal_overlay(SelectionDialog(tr(key), themes, current, on_close=on_select))
