from collections.abc import Callable

from openpilot.frogpilot.ui.layouts.settings.toggle_metadata import ALERTS_AND_SOUNDS_TOGGLES
from openpilot.frogpilot.ui.layouts.settings.toggle_panel import TogglePanel


class AlertsAndSoundsLayout(TogglePanel):
  def __init__(self, back_callback: Callable):
    super().__init__(ALERTS_AND_SOUNDS_TOGGLES, back_callback)
