from __future__ import annotations
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

ACTION_OPTIONS = [
  {"id": 0, "name": "No Action"},
  {"id": 1, "name": "Change Personality", "requires_longitudinal": True},
  {"id": 2, "name": "Force Coast", "requires_longitudinal": True},
  {"id": 3, "name": "Pause Steering"},
  {"id": 4, "name": "Pause Accel/Brake", "requires_longitudinal": True},
  {"id": 5, "name": "Toggle Experimental", "requires_longitudinal": True},
  {"id": 6, "name": "Toggle Traffic", "requires_longitudinal": True},
  {"id": 7, "name": "Toggle Switchback"},
  {"id": 8, "name": "Create Bookmark"},
]
ACTION_NAMES = [option["name"] for option in ACTION_OPTIONS]
ACTION_IDS = {option["name"]: option["id"] for option in ACTION_OPTIONS}
ACTION_NAME_BY_ID = {option["id"]: option["name"] for option in ACTION_OPTIONS}


class StarPilotWheelLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Remap Cancel Button"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("RemapCancelToDistance"),
        "set_state": self._set_cancel_remap_state,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Nostalgia Mode"),
        "desc": tr_noop("Use the left paddle to pause openpilot acceleration and braking while Always On Lateral stays active on supported Hyundai CAN-FD cars."),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("NostalgiaMode"),
        "set_state": lambda s: self._params.put_bool("NostalgiaMode", s),
        "key": "NostalgiaMode",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Distance Button"),
        "type": "value",
        "get_value": lambda: self._get_action_name("DistanceButtonControl"),
        "on_click": lambda: self._show_action_picker("DistanceButtonControl"),
        "color": "#64748B",
      },
      {
        "title": tr_noop("Distance (Long Press)"),
        "type": "value",
        "get_value": lambda: self._get_action_name("LongDistanceButtonControl"),
        "on_click": lambda: self._show_action_picker("LongDistanceButtonControl"),
        "color": "#64748B",
      },
      {
        "title": tr_noop("Distance (Very Long)"),
        "type": "value",
        "get_value": lambda: self._get_action_name("VeryLongDistanceButtonControl"),
        "on_click": lambda: self._show_action_picker("VeryLongDistanceButtonControl"),
        "color": "#64748B",
      },
      {
        "title": tr_noop("LKAS Button"),
        "type": "value",
        "get_value": lambda: self._get_action_name("LKASButtonControl"),
        "on_click": lambda: self._show_action_picker("LKASButtonControl"),
        "is_enabled": lambda: not self._lkas_locked(),
        "key": "LKASButtonControl",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Mode Button"),
        "type": "value",
        "get_value": lambda: self._get_action_name("ModeButtonControl"),
        "on_click": lambda: self._show_action_picker("ModeButtonControl"),
        "key": "ModeButtonControl",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Mode (Long Press)"),
        "type": "value",
        "get_value": lambda: self._get_action_name("LongModeButtonControl"),
        "on_click": lambda: self._show_action_picker("LongModeButtonControl"),
        "key": "LongModeButtonControl",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Mode (Very Long)"),
        "type": "value",
        "get_value": lambda: self._get_action_name("VeryLongModeButtonControl"),
        "on_click": lambda: self._show_action_picker("VeryLongModeButtonControl"),
        "key": "VeryLongModeButtonControl",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Star Button"),
        "type": "value",
        "get_value": lambda: self._get_action_name("StarButtonControl"),
        "on_click": lambda: self._show_action_picker("StarButtonControl"),
        "key": "StarButtonControl",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Star (Long Press)"),
        "type": "value",
        "get_value": lambda: self._get_action_name("LongStarButtonControl"),
        "on_click": lambda: self._show_action_picker("LongStarButtonControl"),
        "key": "LongStarButtonControl",
        "color": "#64748B",
      },
      {
        "title": tr_noop("Star (Very Long)"),
        "type": "value",
        "get_value": lambda: self._get_action_name("VeryLongStarButtonControl"),
        "on_click": lambda: self._show_action_picker("VeryLongStarButtonControl"),
        "key": "VeryLongStarButtonControl",
        "color": "#64748B",
      },
    ]
    self._rebuild_grid()

  def _lkas_locked(self):
    return self._params.get_bool("RemapCancelToDistance")

  def _force_lkas_no_action(self):
    if self._params.get_int("LKASButtonControl") != 0:
      self._params.put_int("LKASButtonControl", 0)
      self._params_memory.put_bool("StarPilotTogglesUpdated", True)

  def _set_cancel_remap_state(self, state):
    self._params.put_bool("RemapCancelToDistance", state)
    if state:
      self._force_lkas_no_action()
    self._rebuild_grid()

  def _get_action_name(self, key):
    if key == "LKASButtonControl" and self._lkas_locked():
      self._force_lkas_no_action()
      return ACTION_NAME_BY_ID[0]

    idx = self._params.get_int(key)
    return ACTION_NAME_BY_ID.get(idx, ACTION_NAMES[0])

  def _get_available_actions(self, key=None):
    if key == "LKASButtonControl" and self._lkas_locked():
      return [ACTION_NAME_BY_ID[0]]

    cs = starpilot_state.car_state
    return [
      option["name"] for option in ACTION_OPTIONS
      if cs.hasOpenpilotLongitudinal or not option.get("requires_longitudinal", False)
    ]

  def _show_action_picker(self, key):
    if key == "LKASButtonControl" and self._lkas_locked():
      self._force_lkas_no_action()
      self._rebuild_grid()
      return

    actions = self._get_available_actions(key)
    current = self._get_action_name(key)
    if current not in actions:
      current = actions[0]

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put_int(key, ACTION_IDS.get(dialog.selection, 0))
        self._params_memory.put_bool("StarPilotTogglesUpdated", True)
        self._rebuild_grid()

    dialog = MultiOptionDialog(tr(key), actions, current, callback=on_select)
    gui_app.push_widget(dialog)

  def _rebuild_grid(self):
    if not self.CATEGORIES:
      return
    if self._lkas_locked():
      self._force_lkas_no_action()
    if self._tile_grid is None:
      self._tile_grid = __import__('openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid', fromlist=['TileGrid']).TileGrid(columns=None, padding=20)
    self._tile_grid.clear()
    cs = starpilot_state.car_state
    for cat in self.CATEGORIES:
      key = cat.get("key")
      visible = True
      if key == "LKASButtonControl":
        visible &= not cs.isSubaru
        visible &= not (cs.lkasAllowedForAOL and self._params.get_bool("AlwaysOnLateral") and self._params.get_bool("AlwaysOnLateralLKAS"))
      if key == "NostalgiaMode":
        visible &= cs.isHKGCanFd and cs.hasOpenpilotLongitudinal
      if key in ("ModeButtonControl", "LongModeButtonControl", "VeryLongModeButtonControl",
                 "StarButtonControl", "LongStarButtonControl", "VeryLongStarButtonControl"):
        visible &= cs.hasModeStarButtons
      if not visible:
        continue
      tile_type = cat.get("type", "hub")
      if tile_type == "toggle":
        from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import ToggleTile

        tile = ToggleTile(title=tr(cat["title"]), get_state=cat["get_state"], set_state=cat["set_state"], bg_color=cat.get("color"))
      elif tile_type == "value":
        from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import ValueTile

        tile = ValueTile(title=tr(cat["title"]), get_value=cat["get_value"], on_click=cat["on_click"], bg_color=cat.get("color"), is_enabled=cat.get("is_enabled"))
      else:
        continue
      self._tile_grid.add_tile(tile)
