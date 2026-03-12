from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotVehicleSettingsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Vehicle Settings"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Configure car-specific options</b> like model name and features."),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
