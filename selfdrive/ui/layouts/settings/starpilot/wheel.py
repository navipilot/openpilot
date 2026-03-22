from __future__ import annotations
from openpilot.selfdrive.ui.lib.starpilot_state import starpilot_state
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets import DialogResult
from openpilot.system.ui.widgets.selection_dialog import SelectionDialog
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotWheelLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()
    self.CATEGORIES = [
      {
        "title": tr_noop("Remap Cancel Button"),
        "type": "toggle",
        "get_state": lambda: self._params.get_bool("RemapCancelToDistance"),
        "set_state": lambda s: self._params.put_bool("RemapCancelToDistance", s),
        "icon": "toggle_icons/icon_steering.png",
        "color": "#FFC40D"
      },
    ]
    self._rebuild_grid()
