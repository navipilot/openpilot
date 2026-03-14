import pyray as rl

from collections.abc import Callable

from openpilot.common.params import Params
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult, Widget
from openpilot.system.ui.widgets.confirm_dialog import alert_dialog
from openpilot.system.ui.widgets.scroller_tici import Scroller

from openpilot.frogpilot.system.ui.widgets.list_view import frogpilot_multiple_button_item
from openpilot.frogpilot.ui.layouts.settings.alerts_and_sounds import AlertsAndSoundsLayout
from openpilot.frogpilot.ui.layouts.settings.toggle_metadata import TUNING_LEVEL_TOGGLE

PANEL_LAYOUT = (
  {
    "title": tr_noop("Alerts and Sounds"),
    "description": tr_noop("<b>Adjust alert volumes and enable custom notifications.</b>"),
    "buttons": (tr_noop("MANAGE"),),
    "icon": "../../../frogpilot/assets/toggle_icons/icon_sound.png",
    "panel_factories": (lambda back_callback: AlertsAndSoundsLayout(back_callback=back_callback),),
    "panel_targets": ("alerts_and_sounds",),
  },
  {
    "title": tr_noop("Driving Controls"),
    "description": tr_noop("<b>Fine-tune custom FrogPilot acceleration, braking, and steering controls.</b>"),
    "buttons": (tr_noop("DRIVING MODEL"), tr_noop("GAS / BRAKE"), tr_noop("STEERING")),
    "icon": "../../../frogpilot/assets/toggle_icons/icon_steering.png",
  },
  {
    "title": tr_noop("Navigation"),
    "description": tr_noop("<b>Download map data for the \"Speed Limit Controller\".</b>"),
    "buttons": (tr_noop("MAP DATA"), tr_noop("NAVIGATION")),
    "icon": "../../../frogpilot/assets/toggle_icons/icon_map.png",
  },
  {
    "title": tr_noop("System Settings"),
    "description": tr_noop("<b>Manage backups, device settings, screen options, storage, and tools to keep FrogPilot running smoothly.</b>"),
    "buttons": (tr_noop("DATA"), tr_noop("DEVICE CONTROLS"), tr_noop("UTILITIES")),
    "icon": "../../../frogpilot/assets/toggle_icons/icon_system.png",
  },
  {
    "title": tr_noop("Theme and Appearance"),
    "description": tr_noop("<b>Customize the look of the driving screen and interface, including themes!</b>"),
    "buttons": (tr_noop("APPEARANCE"), tr_noop("THEME")),
    "icon": "../../../frogpilot/assets/toggle_icons/icon_display.png",
  },
  {
    "title": tr_noop("Vehicle Settings"),
    "description": tr_noop("<b>Configure car-specific options and steering wheel button mappings.</b>"),
    "buttons": (tr_noop("VEHICLE SETTINGS"), tr_noop("WHEEL CONTROLS")),
    "icon": "../../../frogpilot/assets/toggle_icons/icon_vehicle.png",
  },
)

TUNING_LEVELS = {
  "MINIMAL": {"index": 0, "label": tr_noop("Minimal")},
  "STANDARD": {"index": 1, "label": tr_noop("Standard")},
  "ADVANCED": {"index": 2, "label": tr_noop("Advanced")},
  "DEVELOPER": {"index": 3, "label": tr_noop("Developer")},
}

WELCOME_TO_OPENPILOT_MESSAGE = tr_noop("Welcome to FrogPilot! Since you're new to openpilot, the \"Minimal\" toggle preset has been applied, but you can change this at any time via the \"Tuning Level\" button!")
WELCOME_TO_FROGPILOT_MESSAGE = tr_noop("Welcome to FrogPilot! Since you're new to FrogPilot, the \"Minimal\" toggle preset has been applied, but you can change this at any time via the \"Tuning Level\" button!")
FAIRLY_NEW_TO_FROGPILOT_MESSAGE = tr_noop("Since you're fairly new to FrogPilot, the \"Minimal\" toggle preset has been applied, but you can change this at any time via the \"Tuning Level\" button!")
EXPERIENCED_WITH_OPENPILOT_MESSAGE = tr_noop("Since you're experienced with openpilot, the \"Standard\" toggle preset has been applied, but you can change this at any time via the \"Tuning Level\" button!")
EXPERIENCED_WITH_FROGPILOT_MESSAGE = tr_noop("Since you're experienced with FrogPilot, the \"Standard\" toggle preset has been applied, but you can change this at any time via the \"Tuning Level\" button!")
VERY_EXPERIENCED_WITH_FROGPILOT_MESSAGE = tr_noop("Since you're very experienced with FrogPilot, the \"Advanced\" toggle preset has been applied, but you can change this at any time via the \"Tuning Level\" button!")


class FrogPilotLayout(Widget):
  def __init__(self):
    super().__init__()

    self.params = Params(return_defaults=True)

    self._show_welcome_message = not self.params.get_bool("TuningLevelConfirmed")

    self.tuning_level = self.params.get("TuningLevel")

    self._tuning_level_item = frogpilot_multiple_button_item(
      lambda: tr(TUNING_LEVEL_TOGGLE.title),
      lambda: tr(TUNING_LEVEL_TOGGLE.description),
      buttons=[lambda label=level["label"]: tr(label) for level in TUNING_LEVELS.values()],
      button_width=255,
      callback=lambda tuning_level: self.set_tuning_level(tuning_level),
      selected_index=self.tuning_level,
      icon=TUNING_LEVEL_TOGGLE.icon,
    )
    items = [self._tuning_level_item]

    self._active_panel: Widget | None = None

    self._panel_layouts: dict[str, Widget] = {
      panel_id: panel_factory(self._show_main_panel)
      for group in PANEL_LAYOUT
      for panel_factory, panel_id in zip(group.get("panel_factories", ()), group.get("panel_targets", ()))
    }

    for group in PANEL_LAYOUT:
      items.append(
        frogpilot_multiple_button_item(
          lambda title=group["title"]: tr(title),
          description=lambda description=group["description"]: tr(description),
          buttons=[lambda label=label: tr(label) for label in group["buttons"]],
          selected_index=-1,
          callback=(lambda button_index, panel_targets=group.get("panel_targets", ()): self._open_panel(panel_targets[button_index])),
          icon=group.get("icon", ""),
        )
      )

    self._main_scroller = Scroller(items, line_separator=True, spacing=0)

  def show_event(self):
    if self._show_welcome_message:
      self._welcome_message()

    self._main_scroller.show_event()

  def hide_event(self):
    if self._active_panel is not None:
      self._active_panel.hide_event()
    self._active_panel = None

  def can_navigate_back(self) -> bool:
    if self._active_panel is not None:
      panel_can_navigate_back = getattr(self._active_panel, "can_navigate_back", None)
      if callable(panel_can_navigate_back) and panel_can_navigate_back():
        return True

    return self._active_panel is not None

  def navigate_back(self) -> bool:
    if not self.can_navigate_back():
      return False

    if self._active_panel is not None:
      panel_navigate_back = getattr(self._active_panel, "navigate_back", None)
      if callable(panel_navigate_back) and panel_navigate_back():
        return True

    self._show_main_panel()
    return True

  def set_tuning_level(self, tuning_level: int, confirm: bool = False):
    self.tuning_level = tuning_level

    self._tuning_level_item.action_item.set_selected_button(tuning_level)

    self.params.put("TuningLevel", tuning_level)
    if confirm:
      self.params.put_bool("TuningLevelConfirmed", True)

  def _get_tuning_level_recommendation(self) -> tuple[int, str]:
    frogpilot_hours = (self.params.get("FrogPilotStats").get("FrogPilotSeconds", 0)) // (60 * 60)
    openpilot_hours = (self.params.get("KonikMinutes") + self.params.get("openpilotMinutes")) // 60

    if frogpilot_hours < 1 and openpilot_hours < 100:
      if openpilot_hours < 10:
        return TUNING_LEVELS["MINIMAL"]["index"], WELCOME_TO_OPENPILOT_MESSAGE
      return TUNING_LEVELS["MINIMAL"]["index"], WELCOME_TO_FROGPILOT_MESSAGE

    if frogpilot_hours < 50 and openpilot_hours < 100:
      return TUNING_LEVELS["MINIMAL"]["index"], FAIRLY_NEW_TO_FROGPILOT_MESSAGE

    if frogpilot_hours < 100:
      if openpilot_hours >= 100:
        return TUNING_LEVELS["STANDARD"]["index"], EXPERIENCED_WITH_OPENPILOT_MESSAGE
      return TUNING_LEVELS["STANDARD"]["index"], EXPERIENCED_WITH_FROGPILOT_MESSAGE

    return TUNING_LEVELS["ADVANCED"]["index"], VERY_EXPERIENCED_WITH_FROGPILOT_MESSAGE

  def _open_panel(self, panel_id: str):
    self._active_panel = self._panel_layouts[panel_id]
    self._active_panel.show_event()

  def _render(self, rect: rl.Rectangle):
    if self._active_panel is not None:
      self._active_panel.render(rect)
    else:
      self._main_scroller.render(rect)

  def _show_main_panel(self):
    self._active_panel = None
    self._main_scroller.show_event()

  def _welcome_message(self):
    tuning_level, message = self._get_tuning_level_recommendation()

    def handle_confirmation(result: int):
      if result != DialogResult.CONFIRM:
        return
      self.set_tuning_level(tuning_level, confirm=True)

    gui_app.set_modal_overlay(alert_dialog(tr(message)), callback=handle_confirmation)

    self._show_welcome_message = False
