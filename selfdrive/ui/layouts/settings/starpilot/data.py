from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotDataLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Manage Backups"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Create, restore, or delete backups</b> of your StarPilot settings."),
      ),
      button_item(
        tr_noop("Manage Storage"),
        lambda: tr("MANAGE"),
        tr_noop("<b>View and manage storage usage</b> for models, maps, and other data."),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
