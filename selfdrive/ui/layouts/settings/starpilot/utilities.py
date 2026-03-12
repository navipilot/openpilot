from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotUtilitiesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Update StarPilot"),
        lambda: tr("CHECK"),
        tr_noop("<b>Check for updates</b> and update StarPilot to the latest version."),
      ),
      button_item(
        tr_noop("Reset Settings"),
        lambda: tr("RESET"),
        tr_noop("<b>Reset all StarPilot settings</b> to their default values."),
      ),
      button_item(
        tr_noop("View Logs"),
        lambda: tr("VIEW"),
        tr_noop("<b>View StarPilot logs</b> for debugging."),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
