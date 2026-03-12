from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotVisualsLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Advanced UI Controls"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Advanced visual changes</b> to fine-tune how the driving screen looks."),
      ),
      button_item(
        tr_noop("Driving Screen Widgets"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Custom StarPilot widgets</b> for the driving screen."),
      ),
      button_item(
        tr_noop("Model UI"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Model visualizations</b> for the driving path, lane lines, path edges, and road edges."),
      ),
      button_item(
        tr_noop("Navigation Widgets"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Speed limits, and other navigation widgets.</b>"),
      ),
      button_item(
        tr_noop("Quality of Life"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Miscellaneous visual changes</b> to fine-tune how the driving screen looks."),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
