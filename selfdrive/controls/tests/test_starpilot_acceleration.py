from types import SimpleNamespace

import pytest

from openpilot.common.constants import CV
from openpilot.selfdrive.controls.lib.longitudinal_planner import A_CRUISE_MIN
from openpilot.starpilot.common.accel_profile import ACCELERATION_PROFILES, DECELERATION_PROFILES
from openpilot.starpilot.controls.lib.starpilot_acceleration import (
  A_CRUISE_MIN_ECO,
  StarPilotAcceleration,
  get_slc_shaped_min_accel,
)


class FakePlanner:
  def __init__(self):
    self.v_cruise = 0.0
    self.starpilot_weather = SimpleNamespace(weather_id=0, reduce_acceleration=0.0)


def make_toggles(**overrides):
  defaults = {
    "acceleration_profile": ACCELERATION_PROFILES["STANDARD"],
    "deceleration_profile": DECELERATION_PROFILES["ECO"],
    "custom_accel_profile": False,
    "custom_accel_profile_values": [],
    "ev_tuning": True,
    "truck_tuning": False,
    "human_acceleration": False,
    "map_acceleration": False,
    "map_deceleration": False,
    "set_speed_offset": 0,
    "speed_limit_controller": True,
  }
  defaults.update(overrides)
  return SimpleNamespace(**defaults)


def make_lead(status=False, d_rel=150.0, v_lead=0.0, a_lead_k=0.0):
  return SimpleNamespace(status=status, dRel=d_rel, vLead=v_lead, aLeadK=a_lead_k)


def make_sm(*, set_speed_kph=100.0, v_target=0.0, slc_target=0.0, lead_one=None, lead_two=None,
            standstill=False, force_decel=False, red_light=False, forcing_stop=False,
            disable_throttle=False, eco_gear=False, sport_gear=False, force_coast=False,
            traffic_mode=False):
  return {
    "carState": SimpleNamespace(vCruise=set_speed_kph, standstill=standstill),
    "controlsState": SimpleNamespace(forceDecel=force_decel),
    "radarState": SimpleNamespace(
      leadOne=lead_one or make_lead(),
      leadTwo=lead_two or make_lead(),
    ),
    "starpilotCarState": SimpleNamespace(
      ecoGear=eco_gear,
      sportGear=sport_gear,
      forceCoast=force_coast,
      trafficModeEnabled=traffic_mode,
    ),
    "starpilotPlan": SimpleNamespace(
      vCruise=v_target,
      slcSpeedLimit=slc_target,
      redLight=red_light,
      forcingStop=forcing_stop,
      disableThrottle=disable_throttle,
    ),
  }


def test_slc_coast_window_prefers_coast_for_small_overspeed():
  accel = StarPilotAcceleration(FakePlanner())
  target = 56.0 * CV.MPH_TO_MS
  sm = make_sm(set_speed_kph=100.0, v_target=target, slc_target=target)

  accel.update(57.0 * CV.MPH_TO_MS, sm, make_toggles(deceleration_profile=DECELERATION_PROFILES["ECO"]))

  assert accel.min_accel == pytest.approx(-0.02, abs=1e-3)


def test_slc_coast_window_scales_by_profile_strength():
  v_ego = 65.0 * CV.MPH_TO_MS
  v_target = 60.0 * CV.MPH_TO_MS

  eco = get_slc_shaped_min_accel(v_ego, v_target, DECELERATION_PROFILES["ECO"], A_CRUISE_MIN_ECO)
  standard = get_slc_shaped_min_accel(v_ego, v_target, DECELERATION_PROFILES["STANDARD"], A_CRUISE_MIN)
  sport = get_slc_shaped_min_accel(v_ego, v_target, DECELERATION_PROFILES["SPORT"], A_CRUISE_MIN * 2)

  assert eco > standard > sport


def test_slc_coast_window_disabled_for_relevant_lead():
  accel = StarPilotAcceleration(FakePlanner())
  target = 56.0 * CV.MPH_TO_MS
  sm = make_sm(
    set_speed_kph=100.0,
    v_target=target,
    slc_target=target,
    lead_one=make_lead(status=True, d_rel=20.0, v_lead=45.0 * CV.MPH_TO_MS, a_lead_k=0.0),
  )

  accel.update(57.0 * CV.MPH_TO_MS, sm, make_toggles(deceleration_profile=DECELERATION_PROFILES["ECO"]))

  assert accel.min_accel == pytest.approx(A_CRUISE_MIN_ECO)


def test_slc_coast_window_disabled_when_target_drop_is_not_slc():
  accel = StarPilotAcceleration(FakePlanner())
  slc_target = 60.0 * CV.MPH_TO_MS
  sm = make_sm(
    set_speed_kph=100.0,
    v_target=55.0 * CV.MPH_TO_MS,
    slc_target=slc_target,
  )

  accel.update(57.0 * CV.MPH_TO_MS, sm, make_toggles(deceleration_profile=DECELERATION_PROFILES["ECO"]))

  assert accel.min_accel == pytest.approx(A_CRUISE_MIN_ECO)
