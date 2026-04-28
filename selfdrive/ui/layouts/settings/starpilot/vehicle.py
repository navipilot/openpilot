from __future__ import annotations

from openpilot.system.hardware import HARDWARE
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.confirm_dialog import ConfirmDialog
from openpilot.system.ui.widgets.option_dialog import MultiOptionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel
from openpilot.selfdrive.ui.layouts.settings.starpilot.aethergrid import AetherSliderDialog, TileGrid, HubTile, ToggleTile, ValueTile
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.selfdrive.ui.mici.layouts.settings.fingerprint_catalog import (
  FingerprintModelOption,
  get_fingerprint_catalog,
  shorten_model_label,
)


def _lock_doors_timer_labels():
  labels: dict[float, str] = {0.0: tr("Never")}
  for i in range(5, 305, 5):
    labels[float(i)] = f"{i}s"
  return labels


class StarPilotVehicleSettingsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self._make_options, self._models_by_make, self._models_by_value, self._make_by_model = get_fingerprint_catalog()
    self._sub_panels = {
      "gm": StarPilotGMVehicleLayout(),
      "hkg": StarPilotHKGVehicleLayout(),
      "subaru": StarPilotSubaruVehicleLayout(),
      "toyota": StarPilotToyotaVehicleLayout(),
      "info": StarPilotVehicleInfoLayout(),
    }

    self.CATEGORIES = [
      {
        "title": tr_noop("Car Make"),
        "type": "value",
        "get_value": self._get_display_make,
        "on_click": self._on_select_make,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Car Model"),
        "type": "value",
        "get_value": self._get_display_model,
        "on_click": self._on_select_model,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Disable Fingerprinting"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("ForceFingerprint"),
        "set_state": lambda s: self._params.put_bool("ForceFingerprint", s),
        "color": "#64748B",
      },
      {
        "title": tr_noop("Disable openpilot Long"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("DisableOpenpilotLongitudinal"),
        "set_state": self._on_disable_long,
        "color": "#64748B",
      },
      {"title": tr_noop("GM Settings"), "panel": "gm", "icon": "toggle_icons/icon_vehicle.png", "color": "#64748B", "key": "gm"},
      {"title": tr_noop("HKG Settings"), "panel": "hkg", "icon": "toggle_icons/icon_vehicle.png", "color": "#64748B", "key": "hkg"},
      {"title": tr_noop("Subaru Settings"), "panel": "subaru", "icon": "toggle_icons/icon_vehicle.png", "color": "#64748B", "key": "subaru"},
      {"title": tr_noop("Toyota Settings"), "panel": "toyota", "icon": "toggle_icons/icon_vehicle.png", "color": "#64748B", "key": "toyota"},
      {"title": tr_noop("Vehicle Info"), "panel": "info", "icon": "toggle_icons/icon_vehicle.png", "color": "#64748B"},
    ]

    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

    self._rebuild_grid()

  def _rebuild_grid(self):
    if not self.CATEGORIES:
      return
    if self._tile_grid is None:
      self._tile_grid = TileGrid(columns=None, padding=20)
    self._tile_grid.clear()

    cs = starpilot_state.car_state
    for cat in self.CATEGORIES:
      key = cat.get("key")
      visible = True

      if key == "gm":
        visible = cs.isGM
      elif key == "hkg":
        visible = cs.isHKG
      elif key == "subaru":
        visible = cs.isSubaru
      elif key == "toyota":
        visible = cs.isToyota

      if not visible:
        continue

      tile_type = cat.get("type", "hub")
      if tile_type == "hub":
        on_click = cat.get("on_click")
        if on_click is None:
          on_click = lambda c=cat: self._navigate_to(c["panel"])
        tile = HubTile(
          title=tr(cat["title"]),
          desc=tr(cat.get("desc", "")),
          icon_path=cat.get("icon"),
          on_click=on_click,
          starpilot_icon=cat.get("starpilot_icon", True),
          bg_color=cat.get("color"),
        )
      elif tile_type == "toggle":
        tile = ToggleTile(title=tr(cat["title"]), get_state=cat["get_state"], set_state=cat["set_state"], icon_path=cat.get("icon"), bg_color=cat.get("color"))
      elif tile_type == "value":
        tile = ValueTile(title=tr(cat["title"]), get_value=cat["get_value"], on_click=cat["on_click"], icon_path=cat.get("icon"), bg_color=cat.get("color"))
      else:
        continue

      self._tile_grid.add_tile(tile)

  def _get_display_make(self):
    make = self._params.get("CarMake", encoding='utf-8') or ""
    if make:
      return make

    model = self._params.get("CarModel", encoding='utf-8') or ""
    if model:
      return self._make_by_model.get(model, tr("None"))
    return tr("None")

  def _get_selected_model_option(self) -> FingerprintModelOption | None:
    model = self._params.get("CarModel", encoding='utf-8') or ""
    if not model:
      return None

    model_name = self._params.get("CarModelName", encoding='utf-8') or ""
    make = self._params.get("CarMake", encoding='utf-8') or self._make_by_model.get(model, "")
    if make and model_name:
      for option in self._models_by_make.get(make, ()): 
        if option.value == model and option.label == model_name:
          return option

    return self._models_by_value.get(model)

  def _get_display_model(self):
    selected_option = self._get_selected_model_option()
    if selected_option is not None:
      return selected_option.button_label

    model = self._params.get("CarModel", encoding='utf-8') or ""
    model_name = self._params.get("CarModelName", encoding='utf-8') or ""
    make = self._params.get("CarMake", encoding='utf-8') or self._make_by_model.get(model, "")

    if model_name:
      return shorten_model_label(make, model_name) if make else model_name
    if model and model in self._models_by_value:
      return self._models_by_value[model].button_label
    return tr("None")

  def _on_select_make(self):
    makes = list(self._make_options)
    if not makes:
      gui_app.push_widget(ConfirmDialog(tr("No fingerprint list available."), tr("OK")))
      return

    current_make = self._params.get("CarMake", encoding='utf-8') or ""
    default_make = current_make if current_make in makes else makes[0]

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        self._params.put("CarMake", dialog.selection)
        current_model = self._params.get("CarModel", encoding='utf-8') or ""
        available_models = {option.value for option in self._models_by_make.get(dialog.selection, ())}
        if current_model not in available_models:
          self._params.remove("CarModel")
          self._params.remove("CarModelName")
        self._rebuild_grid()

    dialog = MultiOptionDialog(tr("Select Make"), makes, default_make, callback=on_select)
    gui_app.push_widget(dialog)

  def _on_select_model(self):
    make = self._params.get("CarMake", encoding='utf-8') or ""
    if not make:
      gui_app.push_widget(ConfirmDialog(tr("Please select a Car Make first!"), tr("OK")))
      return

    model_options = self._models_by_make.get(make, ())
    if not model_options:
      gui_app.push_widget(ConfirmDialog(tr("No models available for this make."), tr("OK")))
      return

    option_labels = [option.option_label for option in model_options]
    selected_by_label = {option.option_label: option for option in model_options}
    current_model = self._params.get("CarModel", encoding='utf-8') or ""
    current_model_name = self._params.get("CarModelName", encoding='utf-8') or ""
    default_option = next((option.option_label for option in model_options if option.value == current_model and option.label == current_model_name), None)
    if default_option is None:
      default_option = next((option.option_label for option in model_options if option.value == current_model), option_labels[0])

    def on_select(res):
      if res == DialogResult.CONFIRM and dialog.selection:
        selected_option = selected_by_label[dialog.selection]
        self._params.put("CarModel", selected_option.value)
        self._params.put("CarModelName", selected_option.label)
        self._params.put("CarMake", make)
        self._rebuild_grid()

    dialog = MultiOptionDialog(tr("Select Model"), option_labels, default_option, callback=on_select)
    gui_app.push_widget(dialog)

  def _on_disable_long(self, state):
    if state:

      def on_confirm(res):
        if res == DialogResult.CONFIRM:
          self._params.put_bool("DisableOpenpilotLongitudinal", True)
          from openpilot.selfdrive.ui.ui_state import ui_state

          if ui_state.started:
            HARDWARE.reboot()
        self._rebuild_grid()

      gui_app.push_widget(ConfirmDialog(tr("Disable openpilot longitudinal control?"), tr("Disable"), callback=on_confirm))
    else:
      self._params.put_bool("DisableOpenpilotLongitudinal", False)
      self._rebuild_grid()


class StarPilotGMVehicleLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Pedal for Long"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("GMPedalLongitudinal"),
        "set_state": lambda s: self._params.put_bool("GMPedalLongitudinal", s),
        "color": "#64748B",
        "key": "GMPedalLongitudinal",
      },
      {
        "title": tr_noop("Offsets on Dash Spoof"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("GMDashSpoofOffsets"),
        "set_state": lambda s: self._params.put_bool("GMDashSpoofOffsets", s),
        "color": "#64748B",
        "key": "GMDashSpoofOffsets",
      },
      {
        "title": tr_noop("Smooth Pedal on Hills"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("LongPitch"),
        "set_state": lambda s: self._params.put_bool("LongPitch", s),
        "color": "#64748B",
        "key": "LongPitch",
      },
      {
        "title": tr_noop("Remote Start Panda"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("RemoteStartBootsComma"),
        "set_state": lambda s: self._params.put_bool("RemoteStartBootsComma", s),
        "color": "#64748B",
      },
      {
        "title": tr_noop("Volt SNG Hack"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("VoltSNG"),
        "set_state": lambda s: self._params.put_bool("VoltSNG", s),
        "color": "#64748B",
        "key": "VoltSNG",
      },
    ]
    self._rebuild_grid()

  def _rebuild_grid(self):
    if not self.CATEGORIES:
      return
    if self._tile_grid is None:
      self._tile_grid = TileGrid(columns=None, padding=20)
    self._tile_grid.clear()

    cs = starpilot_state.car_state
    for cat in self.CATEGORIES:
      key = cat.get("key")
      visible = True

      if key == "GMPedalLongitudinal":
        visible = cs.hasPedal or cs.canUsePedal
      elif key == "GMDashSpoofOffsets":
        visible = cs.hasPedal or cs.canUsePedal
      elif key == "VoltSNG":
        visible = cs.isVolt and not cs.hasSNG

      if not visible:
        continue

      tile = ToggleTile(title=tr(cat["title"]), get_state=cat["get_state"], set_state=cat["set_state"], icon_path=cat.get("icon"), bg_color=cat.get("color"))
      self._tile_grid.add_tile(tile)


class StarPilotHKGVehicleLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Taco Bell Torque Hack"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("TacoTuneHacks"),
        "set_state": lambda s: self._params.put_bool("TacoTuneHacks", s),
        "color": "#64748B",
        "key": "TacoTuneHacks",
      },
    ]
    self._rebuild_grid()

  def _rebuild_grid(self):
    if not self.CATEGORIES:
      return
    if self._tile_grid is None:
      self._tile_grid = TileGrid(columns=None, padding=20)
    self._tile_grid.clear()

    cs = starpilot_state.car_state
    for cat in self.CATEGORIES:
      key = cat.get("key")
      visible = True

      if key == "TacoTuneHacks":
        visible = cs.isHKGCanFd

      if not visible:
        continue

      tile = ToggleTile(title=tr(cat["title"]), get_state=cat["get_state"], set_state=cat["set_state"], icon_path=cat.get("icon"), bg_color=cat.get("color"))
      self._tile_grid.add_tile(tile)


class StarPilotSubaruVehicleLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Stop and Go"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SubaruSNG"),
        "set_state": lambda s: self._params.put_bool("SubaruSNG", s),
        "color": "#64748B",
      },
    ]
    self._rebuild_grid()


class StarPilotToyotaVehicleLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Auto Lock Doors"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("LockDoors"),
        "set_state": lambda s: self._params.put_bool("LockDoors", s),
        "color": "#64748B",
      },
      {
        "title": tr_noop("Auto Unlock Doors"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("UnlockDoors"),
        "set_state": lambda s: self._params.put_bool("UnlockDoors", s),
        "color": "#64748B",
      },
      {
        "title": tr_noop("Lock Doors Timer"),
        "type": "value",
        "get_value": lambda: _lock_doors_timer_labels().get(self._params.get_int('LockDoorsTimer'), f"{self._params.get_int('LockDoorsTimer')}s"),
        "on_click": self._show_lock_timer_selector,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Dashboard Speed Offset"),
        "type": "value",
        "get_value": lambda: f"{self._params.get_float('ClusterOffset'):.3f}x",
        "on_click": self._show_offset_selector,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Stop-and-Go Hack"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("SNGHack"),
        "set_state": lambda s: self._params.put_bool("SNGHack", s),
        "color": "#64748B",
        "key": "SNGHack",
      },
      {
        "title": tr_noop("FrogsGoMoo Tweak"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("FrogsGoMoosTweak"),
        "set_state": lambda s: self._params.put_bool("FrogsGoMoosTweak", s),
        "color": "#64748B",
        "key": "FrogsGoMoosTweak",
      },
    ]
    self._rebuild_grid()

  def _rebuild_grid(self):
    if not self.CATEGORIES:
      return
    if self._tile_grid is None:
      self._tile_grid = TileGrid(columns=None, padding=20)
    self._tile_grid.clear()

    cs = starpilot_state.car_state
    for cat in self.CATEGORIES:
      key = cat.get("key")
      visible = True

      if key == "SNGHack":
        visible = not cs.hasSNG
      elif key == "FrogsGoMoosTweak":
        visible = cs.hasOpenpilotLongitudinal

      if not visible:
        continue

      tile_type = cat.get("type", "hub")
      if tile_type == "toggle":
        tile = ToggleTile(title=tr(cat["title"]), get_state=cat["get_state"], set_state=cat["set_state"], icon_path=cat.get("icon"), bg_color=cat.get("color"))
      elif tile_type == "value":
        tile = ValueTile(title=tr(cat["title"]), get_value=cat["get_value"], on_click=cat["on_click"], icon_path=cat.get("icon"), bg_color=cat.get("color"))
      else:
        continue

      self._tile_grid.add_tile(tile)

  def _show_lock_timer_selector(self):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_int("LockDoorsTimer", int(val))
        self._rebuild_grid()

    gui_app.push_widget(
      AetherSliderDialog(tr("Lock Doors Timer"), 0, 300, 5, self._params.get_int("LockDoorsTimer"), on_close, labels=_lock_doors_timer_labels(), color="#64748B")
    )

  def _show_offset_selector(self):
    def on_close(res, val):
      if res == DialogResult.CONFIRM:
        self._params.put_float("ClusterOffset", float(val))
        self._rebuild_grid()

    gui_app.push_widget(
      AetherSliderDialog(tr("Dashboard Speed Offset"), 1.000, 1.050, 0.001, self._params.get_float("ClusterOffset"), on_close, unit="x", color="#64748B")
    )


class StarPilotVehicleInfoLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Radar Support"),
        "type": "value",
        "get_value": lambda: tr("Yes") if starpilot_state.car_state.hasRadar else tr("No"),
        "on_click": lambda: None,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Longitudinal Support"),
        "type": "value",
        "get_value": lambda: tr("Yes") if starpilot_state.car_state.hasOpenpilotLongitudinal else tr("No"),
        "on_click": lambda: None,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Blind Spot Support"),
        "type": "value",
        "get_value": lambda: tr("Yes") if starpilot_state.car_state.hasBSM else tr("No"),
        "on_click": lambda: None,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Hardware Detected"),
        "type": "value",
        "get_value": lambda: (
          ", ".join(
            filter(
              None,
              [
                tr("Pedal") if starpilot_state.car_state.canUsePedal else "",
                tr("SASCM") if starpilot_state.car_state.hasSASCM else "",
                tr("SDSU") if starpilot_state.car_state.canUseSDSU else "",
                tr("ZSS") if starpilot_state.car_state.hasZSS else "",
              ],
            )
          )
          or tr("None")
        ),
        "on_click": lambda: None,
        "color": "#64748B",
      },
      {
        "title": tr_noop("Pedal Support"),
        "type": "value",
        "get_value": lambda: tr("Yes") if starpilot_state.car_state.canUsePedal else tr("No"),
        "on_click": lambda: None,
        "color": "#64748B",
      },
      {
        "title": tr_noop("SDSU Support"),
        "type": "value",
        "get_value": lambda: tr("Yes") if starpilot_state.car_state.canUseSDSU else tr("No"),
        "on_click": lambda: None,
        "color": "#64748B",
      },
      {
        "title": tr_noop("SNG Support"),
        "type": "value",
        "get_value": lambda: tr("Yes") if starpilot_state.car_state.hasSNG else tr("No"),
        "on_click": lambda: None,
        "color": "#64748B",
      },
    ]
    self._rebuild_grid()
