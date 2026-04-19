from datetime import datetime, timezone
from types import SimpleNamespace

import pytest

from openpilot.common.constants import CV
from openpilot.starpilot.controls.lib.speed_limit_controller import SpeedLimitController


class FakeParams:
  def __init__(self, initial=None):
    self.values = dict(initial or {})

  def get(self, key, encoding=None):
    return self.values.get(key)

  def get_bool(self, key):
    return bool(self.values.get(key, False))

  def get_float(self, key):
    return float(self.values.get(key, 0) or 0)

  def put_nonblocking(self, key, value):
    self.values[key] = value

  def remove(self, key):
    self.values.pop(key, None)


def make_toggles(**overrides):
  defaults = {
    "is_metric": False,
    "map_speed_lookahead_higher": 0.0,
    "map_speed_lookahead_lower": 0.0,
    "slc_fallback_previous_speed_limit": False,
    "slc_fallback_set_speed": False,
    "slc_mapbox_filler": False,
    "speed_limit_confirmation_higher": False,
    "speed_limit_confirmation_lower": False,
    "speed_limit_controller_override_manual": True,
    "speed_limit_controller_override_set_speed": False,
    "speed_limit_filler": False,
    "speed_limit_offset1": 0.0,
    "speed_limit_offset2": 0.0,
    "speed_limit_offset3": 0.0,
    "speed_limit_offset4": 0.0,
    "speed_limit_offset5": 0.0,
    "speed_limit_offset6": 0.0,
    "speed_limit_offset7": 0.0,
    "speed_limit_priority1": "Dashboard",
    "speed_limit_priority2": "Map Data",
    "speed_limit_priority_highest": False,
    "speed_limit_priority_lowest": False,
    "vision_speed_limit_detection": False,
  }
  defaults.update(overrides)
  return SimpleNamespace(**defaults)


def make_sm(*, gas_pressed, enabled=True, accel_pressed=False, decel_pressed=False):
  return {
    "carControl": SimpleNamespace(longActive=True),
    "carState": SimpleNamespace(gasPressed=gas_pressed, steeringAngleDeg=0.0),
    "liveParameters": SimpleNamespace(angleOffsetDeg=0.0),
    "mapdOut": SimpleNamespace(nextSpeedLimitDistance=0.0, nextSpeedLimit=0.0, speedLimit=0.0),
    "selfdriveState": SimpleNamespace(enabled=enabled),
    "starpilotCarState": SimpleNamespace(accelPressed=accel_pressed, decelPressed=decel_pressed),
  }


def make_controller(**toggle_overrides):
  params = FakeParams()
  planner = SimpleNamespace(
    gps_position={},
    gps_valid=False,
    params=params,
    params_memory=FakeParams(),
  )
  controller = SpeedLimitController(SimpleNamespace(starpilot_planner=planner))
  controller.starpilot_toggles = make_toggles(**toggle_overrides)
  return controller


def mph(value):
  return value * CV.MPH_TO_MS


def test_new_source_limit_clears_override_until_gas_release():
  controller = make_controller()
  try:
    controller.source = "Dashboard"
    controller.target = mph(55)
    controller.previous_source = "Dashboard"
    controller.previous_target = mph(55)
    controller.overridden_speed = mph(65)

    sm = make_sm(gas_pressed=True)
    controller.update_limits(mph(45), datetime.now(timezone.utc), False, mph(75), mph(65), sm)
    controller.update_override(mph(75), 0.0, mph(65), 0.0, sm)

    assert controller.target == pytest.approx(mph(45))
    assert controller.source == "Dashboard"
    assert controller.overridden_speed == 0
    assert not controller.override_slc

    controller.update_limits(mph(45), datetime.now(timezone.utc), False, mph(75), mph(65), sm)
    controller.update_override(mph(75), 0.0, mph(65), 0.0, sm)

    assert controller.overridden_speed == 0
    assert not controller.override_slc

    controller.update_override(mph(75), 0.0, mph(65), 0.0, make_sm(gas_pressed=False))
    controller.update_override(mph(75), 0.0, mph(65), 0.0, sm)

    assert controller.overridden_speed == pytest.approx(mph(65))
    assert controller.override_slc
  finally:
    controller.shutdown()


def test_unconfirmed_lower_limit_keeps_existing_override():
  controller = make_controller(speed_limit_confirmation_lower=True)
  try:
    controller.source = "Dashboard"
    controller.target = mph(55)
    controller.previous_source = "Dashboard"
    controller.previous_target = mph(55)
    controller.overridden_speed = mph(65)

    sm = make_sm(gas_pressed=True)
    controller.update_limits(mph(45), datetime.now(timezone.utc), False, mph(75), mph(65), sm)
    controller.update_override(mph(75), 0.0, mph(65), 0.0, sm)

    assert controller.target == pytest.approx(mph(55))
    assert controller.unconfirmed_speed_limit == pytest.approx(mph(45))
    assert controller.overridden_speed == pytest.approx(mph(65))
    assert controller.override_slc
  finally:
    controller.shutdown()
