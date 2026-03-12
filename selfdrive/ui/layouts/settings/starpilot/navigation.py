from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import toggle_item, button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotNavigationLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      toggle_item(
        tr_noop("Use Live Map Data"),
        tr_noop("<b>Use live map data</b> for real-time navigation updates."),
        False,
      ),
      button_item(
        tr_noop("Navigation Settings"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Configure navigation-specific options</b> like route preferences."),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
