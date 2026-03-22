from __future__ import annotations
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.metro import SliderDialog

class StarPilotThemesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._sub_panels = {
      "personalize": StarPilotPersonalizeLayout(),
    }
    self.CATEGORIES = [
      {"title": tr_noop("Personalize openpilot"), "panel": "personalize", "icon": "toggle_icons/icon_frog.png", "color": "#A200FF"},
      {"title": tr_noop("Holiday Themes"), "type": "toggle", "get_state": lambda: self._params.get_bool("HolidayThemes"), "set_state": lambda s: self._params.put_bool("HolidayThemes", s), "icon": "toggle_icons/icon_calendar.png", "color": "#A200FF"},
      {"title": tr_noop("Rainbow Path"), "type": "toggle", "get_state": lambda: self._params.get_bool("RainbowPath"), "set_state": lambda s: self._params.put_bool("RainbowPath", s), "icon": "toggle_icons/icon_rainbow.png", "color": "#A200FF"},
      {"title": tr_noop("Random Events"), "type": "toggle", "get_state": lambda: self._params.get_bool("RandomEvents"), "set_state": lambda s: self._params.put_bool("RandomEvents", s), "icon": "toggle_icons/icon_random.png", "color": "#A200FF"},
      {"title": tr_noop("Random Themes"), "type": "toggle", "get_state": lambda: self._params.get_bool("RandomThemes"), "set_state": lambda s: self._params.put_bool("RandomThemes", s), "icon": "toggle_icons/icon_random_themes.png", "color": "#A200FF"},
    ]
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'): panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'): panel.set_back_callback(self._go_back)
    self._rebuild_grid()

class StarPilotPersonalizeLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Boot Logo"), "type": "hub", "on_click": lambda: self._show_theme_selector("BootLogo"), "color": "#A200FF"},
      {"title": tr_noop("Color Scheme"), "type": "hub", "on_click": lambda: self._show_theme_selector("ColorScheme"), "color": "#A200FF"},
      {"title": tr_noop("Distance Icons"), "type": "hub", "on_click": lambda: self._show_theme_selector("DistanceIconPack"), "color": "#A200FF"},
      {"title": tr_noop("Icon Pack"), "type": "hub", "on_click": lambda: self._show_theme_selector("IconPack"), "color": "#A200FF"},
      {"title": tr_noop("Turn Signals"), "type": "hub", "on_click": lambda: self._show_theme_selector("SignalAnimation"), "color": "#A200FF"},
      {"title": tr_noop("Sound Pack"), "type": "hub", "on_click": lambda: self._show_theme_selector("SoundPack"), "color": "#A200FF"},
      {"title": tr_noop("Steering Wheel"), "type": "hub", "on_click": lambda: self._show_theme_selector("WheelIcon"), "color": "#A200FF"},
    ]
    self._rebuild_grid()

  def _show_theme_selector(self, key):
    themes = ["Stock", "Frog", "Cyberpunk", "Minimal"]
    current = self._params.get(key, encoding='utf-8') or "Stock"
    def on_select(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put(key, val)
        self._rebuild_grid()
    gui_app.set_modal_overlay(SelectionDialog(tr(key), themes, current, on_close=on_select))

class StarPilotVisualsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._sub_panels = {
      "advanced": StarPilotAdvancedVisualsLayout(),
      "widgets": StarPilotVisualWidgetsLayout(),
      "model": StarPilotModelUILayout(),
      "navigation": StarPilotNavigationVisualsLayout(),
      "qol": StarPilotVisualQOLLayout(),
    }
    self.CATEGORIES = [
      {"title": tr_noop("Advanced UI Controls"), "panel": "advanced", "icon": "toggle_icons/icon_advanced_device.png", "color": "#A200FF"},
      {"title": tr_noop("Driving Screen Widgets"), "panel": "widgets", "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Model UI"), "panel": "model", "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Navigation Widgets"), "panel": "navigation", "icon": "toggle_icons/icon_map.png", "color": "#A200FF"},
      {"title": tr_noop("Quality of Life"), "panel": "qol", "icon": "toggle_icons/icon_quality_of_life.png", "color": "#A200FF"},
    ]
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'): panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'): panel.set_back_callback(self._go_back)
    self._rebuild_grid()

class StarPilotAdvancedVisualsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Hide Speed"), "type": "toggle", "get_state": lambda: self._params.get_bool("HideSpeed"), "set_state": lambda s: self._params.put_bool("HideSpeed", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Hide Lead Marker"), "type": "toggle", "get_state": lambda: self._params.get_bool("HideLeadMarker"), "set_state": lambda s: self._params.put_bool("HideLeadMarker", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Hide Max Speed"), "type": "toggle", "get_state": lambda: self._params.get_bool("HideMaxSpeed"), "set_state": lambda s: self._params.put_bool("HideMaxSpeed", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Hide Alerts"), "type": "toggle", "get_state": lambda: self._params.get_bool("HideAlerts"), "set_state": lambda s: self._params.put_bool("HideAlerts", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Hide Speed Limit"), "type": "toggle", "get_state": lambda: self._params.get_bool("HideSpeedLimit"), "set_state": lambda s: self._params.put_bool("HideSpeedLimit", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Wheel Speed"), "type": "toggle", "get_state": lambda: self._params.get_bool("WheelSpeed"), "set_state": lambda s: self._params.put_bool("WheelSpeed", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
    ]
    self._rebuild_grid()

class StarPilotVisualWidgetsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Acceleration Path"), "type": "toggle", "get_state": lambda: self._params.get_bool("AccelerationPath"), "set_state": lambda s: self._params.put_bool("AccelerationPath", s), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Adjacent Lanes"), "type": "toggle", "get_state": lambda: self._params.get_bool("AdjacentPath"), "set_state": lambda s: self._params.put_bool("AdjacentPath", s), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Blind Spot Path"), "type": "toggle", "get_state": lambda: self._params.get_bool("BlindSpotPath"), "set_state": lambda s: self._params.put_bool("BlindSpotPath", s), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Compass"), "type": "toggle", "get_state": lambda: self._params.get_bool("Compass"), "set_state": lambda s: self._params.put_bool("Compass", s), "icon": "toggle_icons/icon_navigate.png", "color": "#A200FF"},
      {"title": tr_noop("Personality Button"), "type": "toggle", "get_state": lambda: self._params.get_bool("OnroadDistanceButton"), "set_state": lambda s: self._params.put_bool("OnroadDistanceButton", s), "icon": "toggle_icons/icon_personality.png", "color": "#A200FF"},
      {"title": tr_noop("Pedal Indicators"), "type": "toggle", "get_state": lambda: self._params.get_bool("PedalsOnUI"), "set_state": lambda s: self._params.put_bool("PedalsOnUI", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Rotating Wheel"), "type": "toggle", "get_state": lambda: self._params.get_bool("RotatingWheel"), "set_state": lambda s: self._params.put_bool("RotatingWheel", s), "icon": "toggle_icons/icon_steering.png", "color": "#A200FF"},
    ]
    self._rebuild_grid()

class StarPilotModelUILayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Dynamic Path"), "type": "toggle", "get_state": lambda: self._params.get_bool("DynamicPathWidth"), "set_state": lambda s: self._params.put_bool("DynamicPathWidth", s), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Lane Line Width"), "type": "value", "get_value": lambda: f"{self._params.get_int('LaneLinesWidth')}in", "on_click": lambda: self._show_int_selector("LaneLinesWidth", 0, 24, "in"), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Path Edge Width"), "type": "value", "get_value": lambda: f"{self._params.get_int('PathEdgeWidth')}%", "on_click": lambda: self._show_int_selector("PathEdgeWidth", 0, 100, "%"), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Path Width"), "type": "value", "get_value": lambda: f"{self._params.get_float('PathWidth'):.1f}ft", "on_click": lambda: self._show_float_selector("PathWidth", 0, 10, 0.1, "ft"), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
      {"title": tr_noop("Road Edge Width"), "type": "value", "get_value": lambda: f"{self._params.get_int('RoadEdgesWidth')}in", "on_click": lambda: self._show_int_selector("RoadEdgesWidth", 0, 24, "in"), "icon": "toggle_icons/icon_road.png", "color": "#A200FF"},
    ]
    self._rebuild_grid()

  def _show_int_selector(self, key, min_v, max_v, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int(key, int(val))
        self._rebuild_grid()
    gui_app.set_modal_overlay(SliderDialog(tr(key), min_v, max_v, 1, self._params.get_int(key), on_close, unit=unit, color="#A200FF"))

  def _show_float_selector(self, key, min_v, max_v, step, unit=""):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_float(key, float(val))
        self._rebuild_grid()
    gui_app.set_modal_overlay(SliderDialog(tr(key), min_v, max_v, step, self._params.get_float(key), on_close, unit=unit, color="#A200FF"))

class StarPilotNavigationVisualsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Road Name"), "type": "toggle", "get_state": lambda: self._params.get_bool("RoadNameUI"), "set_state": lambda s: self._params.put_bool("RoadNameUI", s), "icon": "toggle_icons/icon_navigate.png", "color": "#A200FF"},
      {"title": tr_noop("Speed Limits"), "type": "toggle", "get_state": lambda: self._params.get_bool("ShowSpeedLimits"), "set_state": lambda s: self._params.put_bool("ShowSpeedLimits", s), "icon": "toggle_icons/icon_speed_limit.png", "color": "#A200FF"},
      {"title": tr_noop("Mapbox Limits"), "type": "toggle", "get_state": lambda: self._params.get_bool("SLCMapboxFiller"), "set_state": lambda s: self._params.put_bool("SLCMapboxFiller", s), "icon": "toggle_icons/icon_speed_limit.png", "color": "#A200FF"},
      {"title": tr_noop("Vienna Signs"), "type": "toggle", "get_state": lambda: self._params.get_bool("UseVienna"), "set_state": lambda s: self._params.put_bool("UseVienna", s), "icon": "toggle_icons/icon_speed_limit.png", "color": "#A200FF"},
    ]
    self._rebuild_grid()

class StarPilotVisualQOLLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {"title": tr_noop("Camera View"), "type": "toggle", "get_state": lambda: self._params.get_bool("CameraView"), "set_state": lambda s: self._params.put_bool("CameraView", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Driver Camera"), "type": "toggle", "get_state": lambda: self._params.get_bool("DriverCamera"), "set_state": lambda s: self._params.put_bool("DriverCamera", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
      {"title": tr_noop("Stopped Timer"), "type": "toggle", "get_state": lambda: self._params.get_bool("StoppedTimer"), "set_state": lambda s: self._params.put_bool("StoppedTimer", s), "icon": "toggle_icons/icon_display.png", "color": "#A200FF"},
    ]
    self._rebuild_grid()
