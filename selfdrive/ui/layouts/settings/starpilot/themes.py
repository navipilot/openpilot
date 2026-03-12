from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotThemesLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Select Theme"),
        lambda: tr("SELECT"),
        tr_noop("<b>Select a theme</b> for the StarPilot interface."),
      ),
      button_item(
        tr_noop("Holiday Themes"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Enable or disable holiday-themed visuals.</b>"),
      ),
      button_item(
        tr_noop("Custom Theme"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Create or import a custom theme.</b>"),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
