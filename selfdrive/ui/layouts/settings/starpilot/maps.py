from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotMapsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Download Map Data"),
        lambda: tr("DOWNLOAD"),
        tr_noop("<b>Download map data</b> for the Speed Limit Controller."),
      ),
      button_item(
        tr_noop("Manage Map Data"),
        lambda: tr("MANAGE"),
        tr_noop("<b>View or delete downloaded map data.</b>"),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
