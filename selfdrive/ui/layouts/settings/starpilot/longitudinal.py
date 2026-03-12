from __future__ import annotations

from openpilot.system.ui.lib.multilang import tr, tr_noop
from openpilot.system.ui.widgets.list_view import button_item, value_item
from openpilot.system.ui.widgets.scroller_tici import Scroller
from openpilot.selfdrive.ui.layouts.settings.starpilot.panel import StarPilotPanel

class StarPilotLongitudinalLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    # Main panel items
    items = [
      button_item(
        tr_noop("Advanced Longitudinal Tuning"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Advanced acceleration and braking control changes</b> to fine-tune how openpilot drives."),
      ),
      button_item(
        tr_noop("Conditional Experimental Mode"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Automatically switch to Experimental Mode</b> when set conditions are met."),
      ),
      button_item(
        tr_noop("Curve Speed Controller"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Automatically slow down for upcoming curves</b> using data learned from your driving style."),
      ),
      button_item(
        tr_noop("Driving Personalities"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Customize the Driving Personalities</b> to better match your driving style."),
      ),
      button_item(
        tr_noop("Longitudinal Tuning"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Acceleration and braking control changes</b> to fine-tune how openpilot drives."),
      ),
      button_item(
        tr_noop("Quality of Life"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Miscellaneous acceleration and braking control changes</b> to fine-tune how openpilot drives."),
      ),
      button_item(
        tr_noop("Weather"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Adjust driving behavior</b> based on weather conditions."),
        callback=lambda: self._navigate_to("weather"),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

    # Sub-panels
    self._sub_panels = {
      "weather": StarPilotWeatherLayout(),
      "low_visibility": StarPilotLowVisibilityLayout(),
      "rain": StarPilotRainLayout(),
      "rainstorm": StarPilotRainStormLayout(),
      "snow": StarPilotSnowLayout(),
    }

    # Wire up navigation callbacks for sub-panels
    for name, panel in self._sub_panels.items():
      if hasattr(panel, 'set_navigate_callback'):
        panel.set_navigate_callback(self._navigate_to)
      if hasattr(panel, 'set_back_callback'):
        panel.set_back_callback(self._go_back)

class StarPilotWeatherLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    items = [
      button_item(
        tr_noop("Low Visibility"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Driving adjustments for fog, haze, or other low-visibility conditions.</b>"),
        callback=lambda: self._navigate("low_visibility"),
      ),
      button_item(
        tr_noop("Rain"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Driving adjustments for rainy conditions.</b>"),
        callback=lambda: self._navigate("rain"),
      ),
      button_item(
        tr_noop("Rainstorms"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Driving adjustments for rainstorms.</b>"),
        callback=lambda: self._navigate("rainstorm"),
      ),
      button_item(
        tr_noop("Snow"),
        lambda: tr("MANAGE"),
        tr_noop("<b>Driving adjustments for snowy conditions.</b>"),
        callback=lambda: self._navigate("snow"),
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

  def _navigate(self, sub_panel: str):
    if self._navigate_callback:
      self._navigate_callback(sub_panel)

class StarPilotLowVisibilityLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    def save_following(value: float):
      self._params.put_int("IncreaseFollowingLowVisibility", int(value))

    def save_stopped_distance(value: float):
      self._params.put_int("IncreasedStoppedDistanceLowVisibility", int(value))

    def save_reduce_accel(value: float):
      self._params.put_int("ReduceAccelerationLowVisibility", int(value))

    def save_reduce_lateral(value: float):
      self._params.put_int("ReduceLateralAccelerationLowVisibility", int(value))

    items = [
      value_item(
        tr_noop("Increase Following Distance by:"),
        lambda: self._params.get_int("IncreaseFollowingLowVisibility", return_default=True, default="0"),
        min_val=0,
        max_val=3,
        step=0.5,
        unit=" seconds",
        description=tr_noop("<b>Add extra space behind lead vehicles in low visibility.</b> Increase for more space; decrease for tighter gaps."),
        callback=save_following,
      ),
      value_item(
        tr_noop("Increase Stopped Distance by:"),
        lambda: self._params.get_int("IncreasedStoppedDistanceLowVisibility", return_default=True, default="0"),
        min_val=0,
        max_val=10,
        step=1,
        unit=" feet",
        description=tr_noop("<b>Add extra buffer when stopped behind vehicles in low visibility.</b> Increase for more room; decrease for shorter gaps."),
        callback=save_stopped_distance,
      ),
      value_item(
        tr_noop("Reduce Acceleration by:"),
        lambda: self._params.get_int("ReduceAccelerationLowVisibility", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop(
          "<b>Lower the maximum acceleration in low visibility.</b> Increase for softer takeoffs; decrease for quicker but less stable takeoffs."
        ),
        callback=save_reduce_accel,
      ),
      value_item(
        tr_noop("Reduce Speed in Curves by:"),
        lambda: self._params.get_int("ReduceLateralAccelerationLowVisibility", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop(
          "<b>Lower the desired speed while driving through curves in low visibility.</b> Increase for safer, gentler turns; decrease for more aggressive driving in curves."
        ),
        callback=save_reduce_lateral,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

class StarPilotRainLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    def save_following(value: float):
      self._params.put_int("IncreaseFollowingRain", int(value))

    def save_stopped_distance(value: float):
      self._params.put_int("IncreasedStoppedDistanceRain", int(value))

    def save_reduce_accel(value: float):
      self._params.put_int("ReduceAccelerationRain", int(value))

    def save_reduce_lateral(value: float):
      self._params.put_int("ReduceLateralAccelerationRain", int(value))

    items = [
      value_item(
        tr_noop("Increase Following Distance by:"),
        lambda: self._params.get_int("IncreaseFollowingRain", return_default=True, default="0"),
        min_val=0,
        max_val=3,
        step=0.5,
        unit=" seconds",
        description=tr_noop("<b>Add extra space behind lead vehicles in rain.</b> Increase for more space; decrease for tighter gaps."),
        callback=save_following,
      ),
      value_item(
        tr_noop("Increase Stopped Distance by:"),
        lambda: self._params.get_int("IncreasedStoppedDistanceRain", return_default=True, default="0"),
        min_val=0,
        max_val=10,
        step=1,
        unit=" feet",
        description=tr_noop("<b>Add extra buffer when stopped behind vehicles in rain.</b> Increase for more room; decrease for shorter gaps."),
        callback=save_stopped_distance,
      ),
      value_item(
        tr_noop("Reduce Acceleration by:"),
        lambda: self._params.get_int("ReduceAccelerationRain", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop("<b>Lower the maximum acceleration in rain.</b> Increase for softer takeoffs; decrease for quicker but less stable takeoffs."),
        callback=save_reduce_accel,
      ),
      value_item(
        tr_noop("Reduce Speed in Curves by:"),
        lambda: self._params.get_int("ReduceLateralAccelerationRain", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop(
          "<b>Lower the desired speed while driving through curves in rain.</b> Increase for safer, gentler turns; decrease for more aggressive driving in curves."
        ),
        callback=save_reduce_lateral,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

class StarPilotRainStormLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    def save_following(value: float):
      self._params.put_int("IncreaseFollowingRainStorm", int(value))

    def save_stopped_distance(value: float):
      self._params.put_int("IncreasedStoppedDistanceRainStorm", int(value))

    def save_reduce_accel(value: float):
      self._params.put_int("ReduceAccelerationRainStorm", int(value))

    def save_reduce_lateral(value: float):
      self._params.put_int("ReduceLateralAccelerationRainStorm", int(value))

    items = [
      value_item(
        tr_noop("Increase Following Distance by:"),
        lambda: self._params.get_int("IncreaseFollowingRainStorm", return_default=True, default="0"),
        min_val=0,
        max_val=3,
        step=0.5,
        unit=" seconds",
        description=tr_noop("<b>Add extra space behind lead vehicles in a rainstorm.</b> Increase for more space; decrease for tighter gaps."),
        callback=save_following,
      ),
      value_item(
        tr_noop("Increase Stopped Distance by:"),
        lambda: self._params.get_int("IncreasedStoppedDistanceRainStorm", return_default=True, default="0"),
        min_val=0,
        max_val=10,
        step=1,
        unit=" feet",
        description=tr_noop("<b>Add extra buffer when stopped behind vehicles in a rainstorm.</b> Increase for more room; decrease for shorter gaps."),
        callback=save_stopped_distance,
      ),
      value_item(
        tr_noop("Reduce Acceleration by:"),
        lambda: self._params.get_int("ReduceAccelerationRainStorm", return_default=True, default="0"),
        min_val=0,
        max_val=10,
        step=1,
        unit=" feet",
        description=tr_noop("<b>Add extra buffer when stopped behind vehicles in rain.</b> Increase for more room; decrease for shorter gaps."),
        callback=save_reduce_accel,
      ),
      value_item(
        tr_noop("Reduce Acceleration by:"),
        lambda: self._params.get_int("ReduceAccelerationRain", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop("<b>Lower the maximum acceleration in rain.</b> Increase for softer takeoffs; decrease for quicker but less stable takeoffs."),
      ),
      value_item(
        tr_noop("Reduce Speed in Curves by:"),
        lambda: self._params.get_int("ReduceLateralAccelerationRain", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop(
          "<b>Lower the desired speed while driving through curves in rain.</b> Increase for safer, gentler turns; decrease for more aggressive driving in curves."
        ),
        callback=save_reduce_lateral,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)

class StarPilotSnowLayout(StarPilotPanel):
  def __init__(self):
    super().__init__()

    def save_following(value: float):
      self._params.put_int("IncreaseFollowingSnow", int(value))

    def save_stopped_distance(value: float):
      self._params.put_int("IncreasedStoppedDistanceSnow", int(value))

    def save_reduce_accel(value: float):
      self._params.put_int("ReduceAccelerationSnow", int(value))

    def save_reduce_lateral(value: float):
      self._params.put_int("ReduceLateralAccelerationSnow", int(value))

    items = [
      value_item(
        tr_noop("Increase Following Distance by:"),
        lambda: self._params.get_int("IncreaseFollowingSnow", return_default=True, default="0"),
        min_val=0,
        max_val=3,
        step=0.5,
        unit=" seconds",
        description=tr_noop("<b>Add extra space behind lead vehicles in snow.</b> Increase for more space; decrease for tighter gaps."),
        callback=save_following,
      ),
      value_item(
        tr_noop("Increase Stopped Distance by:"),
        lambda: self._params.get_int("IncreasedStoppedDistanceSnow", return_default=True, default="0"),
        min_val=0,
        max_val=10,
        step=1,
        unit=" feet",
        description=tr_noop("<b>Add extra buffer when stopped behind vehicles in snow.</b> Increase for more room; decrease for shorter gaps."),
        callback=save_stopped_distance,
      ),
      value_item(
        tr_noop("Reduce Acceleration by:"),
        lambda: self._params.get_int("ReduceAccelerationSnow", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop("<b>Lower the maximum acceleration in snow.</b> Increase for softer takeoffs; decrease for quicker but less stable takeoffs."),
        callback=save_reduce_accel,
      ),
      value_item(
        tr_noop("Reduce Speed in Curves by:"),
        lambda: self._params.get_int("ReduceLateralAccelerationSnow", return_default=True, default="0"),
        min_val=0,
        max_val=50,
        step=5,
        unit="%",
        description=tr_noop(
          "<b>Lower the desired speed while driving through curves in snow.</b> Increase for safer, gentler turns; decrease for more aggressive driving in curves."
        ),
        callback=save_reduce_lateral,
      ),
    ]

    self._scroller = Scroller(items, line_separator=True, spacing=0)
