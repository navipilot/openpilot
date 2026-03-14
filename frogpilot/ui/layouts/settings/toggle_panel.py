from collections.abc import Callable

import pyray as rl

from cereal import car
import cereal.messaging as messaging

from openpilot.common.params import Params
from openpilot.system.hardware import HARDWARE
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr
from openpilot.system.ui.widgets import DialogResult, Widget
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog
from openpilot.system.ui.widgets.scroller_tici import Scroller

from openpilot.frogpilot.system.ui.widgets.list_view import (
  frogpilot_button_item, frogpilot_button_param_item, frogpilot_button_toggle_item,
  frogpilot_label_item, frogpilot_manage_control_item, frogpilot_multiple_button_item,
  frogpilot_numeric_control_item, frogpilot_numeric_with_button_item, frogpilot_toggle_item,
)
from openpilot.frogpilot.ui.layouts.settings.toggle_metadata import ToggleDefinition, ToggleType

CAR_PARAM_RESOLVERS: dict[str, Callable] = {
  "blind_spot_monitoring": lambda cp: cp.enableBsm,
  "openpilot_longitudinal": lambda cp: cp.openpilotLongitudinalControl,
  "pcm_cruise": lambda cp: cp.pcmCruise,
  "radar_support": lambda cp: not cp.radarUnavailable,
  "subaru_brand": lambda cp: cp.brand == "subaru",
  "toyota_brand": lambda cp: cp.brand == "toyota",
}


class TogglePanel(Widget):
  def __init__(self, toggle_definitions: tuple[ToggleDefinition, ...], back_callback: Callable):
    super().__init__()

    self._params = Params(return_defaults=True)

    self._back_callback = back_callback

    self._active_panel: Widget | None = None

    self._tuning_level: int = self._params.get("TuningLevel")
    self._car_flags: dict[str, bool] = self._load_car_flags()

    self._tuning_levels: dict[str, int] = {
      toggle_definition.param: self._params.get_tuning_level(toggle_definition.param)
      for toggle_definition in toggle_definitions
    }

    self._toggle_defs: dict[str, ToggleDefinition] = {
      toggle_definition.param: toggle_definition
      for toggle_definition in toggle_definitions
    }

    parent_params = {
      toggle_definition.parent_param
      for toggle_definition in toggle_definitions
      if toggle_definition.parent_param
    }

    child_params: dict[str, list[str]] = {}
    for parent_param in parent_params:
      child_params[parent_param] = [
        toggle_definition.param
        for toggle_definition in toggle_definitions
        if toggle_definition.parent_param == parent_param
      ]

    self._sub_panels: dict[str, Scroller] = {}
    for parent_param in parent_params:
      child_items = []
      for toggle_definition in toggle_definitions:
        if toggle_definition.parent_param != parent_param:
          continue
        if toggle_definition.param in parent_params:
          item = self._create_parent_item(toggle_definition)
        else:
          item = self._create_child_item(toggle_definition)
        item.set_visible(lambda param=toggle_definition.param: self._is_param_visible(param))
        child_items.append(item)
      self._sub_panels[parent_param] = Scroller(child_items, line_separator=True, spacing=0)

    main_items = []
    for toggle_definition in toggle_definitions:
      if toggle_definition.parent_param is not None:
        continue

      if toggle_definition.param in parent_params:
        item = self._create_parent_item(toggle_definition)
        item.set_visible(lambda param=toggle_definition.param: any(
          self._is_param_visible(child_param) for child_param in child_params[param]
        ))
      else:
        item = self._create_child_item(toggle_definition)
        item.set_visible(lambda param=toggle_definition.param: self._is_param_visible(param))
      main_items.append(item)

    self._main_scroller = Scroller(main_items, line_separator=True, spacing=0)

  def show_event(self):
    self._tuning_level = self._params.get("TuningLevel")
    self._car_flags = self._load_car_flags()
    self._main_scroller.show_event()

  def hide_event(self):
    if self._active_panel is not None:
      self._active_panel = None

  def can_navigate_back(self) -> bool:
    return self._active_panel is not None

  def navigate_back(self) -> bool:
    if not self.can_navigate_back():
      return False

    self._show_main_panel()
    return True

  def _is_param_visible(self, param: str) -> bool:
    if self._tuning_level < self._tuning_levels.get(param, 0):
      return False

    toggle_definition = self._toggle_defs.get(param)
    if toggle_definition is None:
      return True

    if toggle_definition.depends_on:
      for dependency in toggle_definition.depends_on:
        negated = dependency.startswith("!")
        key = dependency[1:] if negated else dependency
        value = self._is_param_set(key)
        if negated and value:
          return False
        if not negated and not value:
          return False

    if toggle_definition.car_params:
      for car_param in toggle_definition.car_params:
        negated = car_param.startswith("!")
        key = car_param[1:] if negated else car_param
        flag = self._car_flags.get(key, False)
        if negated and flag:
          return False
        if not negated and not flag:
          return False

    return True

  def _is_param_set(self, key: str) -> bool:
    try:
      return self._params.get_bool(key)
    except Exception:
      val = self._params.get(key)
      return bool(val)

  def _load_car_flags(self) -> dict[str, bool]:
    flags: dict[str, bool] = {}
    try:
      cp_bytes = self._params.get("CarParamsPersistent")
      if cp_bytes:
        car_params = messaging.log_from_bytes(cp_bytes, car.CarParams)
        for key, resolver in CAR_PARAM_RESOLVERS.items():
          flags[key] = resolver(car_params)
    except Exception:
      pass
    return flags

  def _make_bool_callback(self, param: str, reboot_required: bool) -> Callable[[bool], None]:
    def callback(state: bool):
      self._params.put_bool(param, state)
      if reboot_required:
        self._prompt_reboot()
    return callback

  def _prompt_reboot(self):
    def handle_result(result: int):
      if result == DialogResult.CONFIRM:
        HARDWARE.reboot()

    dialog = ConfirmDialog(tr("Reboot required to take effect."), tr("Reboot Now"), tr("Reboot Later"))
    gui_app.set_modal_overlay(dialog, callback=handle_result)

  def _create_child_item(self, toggle_definition: ToggleDefinition):
    title = lambda t=toggle_definition.title: tr(t)
    description = lambda d=toggle_definition.description: tr(d)
    icon = toggle_definition.icon

    if toggle_definition.toggle_type == ToggleType.NUMERIC:
      return frogpilot_numeric_control_item(
        title=title, description=description, icon=icon,
        value_getter=lambda param=toggle_definition.param: self._params.get(param),
        value_setter=lambda value, param=toggle_definition.param: self._params.put(param, value),
        value_formatter=lambda value, unit=toggle_definition.unit, value_map=toggle_definition.value_map: value_map[value] if value_map and value in value_map else f"{value}{unit}",
        min_value=int(toggle_definition.min_value),
        max_value=int(toggle_definition.max_value),
        step=int(toggle_definition.step),
      )

    if toggle_definition.toggle_type == ToggleType.NUMERIC_WITH_BUTTON:
      return frogpilot_numeric_with_button_item(
        title=title, description=description, icon=icon,
        value_getter=lambda param=toggle_definition.param: self._params.get(param),
        value_setter=lambda value, param=toggle_definition.param: self._params.put(param, value),
        value_formatter=lambda value, unit=toggle_definition.unit, value_map=toggle_definition.value_map: value_map[value] if value_map and value in value_map else f"{value}{unit}",
        min_value=int(toggle_definition.min_value),
        max_value=int(toggle_definition.max_value),
        step=int(toggle_definition.step),
        button_text=lambda bt=toggle_definition.button_labels[0]: tr(bt),
      )

    if toggle_definition.toggle_type == ToggleType.BUTTON:
      return frogpilot_button_item(
        title=title, description=description, icon=icon,
        button_text=lambda bt=toggle_definition.button_labels[0]: tr(bt),
      )

    if toggle_definition.toggle_type == ToggleType.BUTTON_PARAM:
      return frogpilot_button_param_item(
        title=title, description=description, icon=icon,
        button_text=lambda bt=toggle_definition.button_labels[0]: tr(bt),
        value_getter=lambda param=toggle_definition.param, value_map=toggle_definition.value_map: value_map.get(self._params.get(param), "") if value_map else "",
      )

    if toggle_definition.toggle_type == ToggleType.BUTTON_TOGGLE:
      return frogpilot_button_toggle_item(
        title=title, description=description, icon=icon,
        buttons=[lambda label=label: tr(label) for label in (toggle_definition.button_labels or [])],
        state_getters=[lambda param=param: self._params.get_bool(param) for param in (toggle_definition.button_options or [])],
        state_setters=[lambda state, param=param: self._params.put_bool(param, state) for param in (toggle_definition.button_options or [])],
      )

    if toggle_definition.toggle_type == ToggleType.LABEL:
      return frogpilot_label_item(
        title=title, description=description, icon=icon,
        value_getter=lambda param=toggle_definition.param: str(self._params.get(param)),
      )

    if toggle_definition.toggle_type == ToggleType.MULTI_BUTTON:
      return frogpilot_multiple_button_item(
        title=title, description=description, icon=icon,
        buttons=[lambda label=label: tr(label) for label in (toggle_definition.button_labels or [])],
        selected_index=-1,
      )

    if toggle_definition.toggle_type == ToggleType.MANAGE:
      return frogpilot_manage_control_item(
        title=title, description=description, icon=icon,
        initial_state=self._params.get_bool(toggle_definition.param),
        toggle_callback=self._make_bool_callback(toggle_definition.param, toggle_definition.reboot_required),
        button_callback=lambda param=toggle_definition.param: self._open_panel(param) if param in self._sub_panels else None,
      )

    # BOOLEAN (default)
    return frogpilot_toggle_item(
      title=title, description=description, icon=icon,
      initial_state=self._params.get_bool(toggle_definition.param),
      callback=self._make_bool_callback(toggle_definition.param, toggle_definition.reboot_required),
    )

  def _create_parent_item(self, toggle_definition: ToggleDefinition):
    return frogpilot_manage_control_item(
      title=lambda t=toggle_definition.title: tr(t),
      description=lambda d=toggle_definition.description: tr(d),
      initial_state=self._params.get_bool(toggle_definition.param),
      toggle_callback=self._make_bool_callback(toggle_definition.param, toggle_definition.reboot_required),
      button_callback=lambda param=toggle_definition.param: self._open_panel(param),
      icon=toggle_definition.icon,
    )

  def _open_panel(self, parent_param: str):
    self._active_panel = self._sub_panels[parent_param]
    self._active_panel.show_event()

  def _render(self, rect: rl.Rectangle):
    if self._active_panel is not None:
      self._active_panel.render(rect)
    else:
      self._main_scroller.render(rect)

  def _show_main_panel(self):
    self._active_panel = None
    self._main_scroller.show_event()
