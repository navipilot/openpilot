from pathlib import Path
from types import SimpleNamespace

from openpilot.common.constants import CV
from openpilot.starpilot.controls.starpilot_planner import StarPilotPlanner
import openpilot.starpilot.controls.starpilot_planner as starpilot_planner_module
from openpilot.starpilot.controls.lib.conditional_experimental_mode import ConditionalExperimentalMode


def make_cem(*, model_length: float, model_stopped: bool = False, tracking_lead: bool = False):
  planner = SimpleNamespace(
    params=None,
    params_memory=None,
    model_length=model_length,
    model_stopped=model_stopped,
    tracking_lead=tracking_lead,
  )
  return ConditionalExperimentalMode(planner)


def make_sm(traffic_mode_enabled: bool = False):
  return {
    "starpilotCarState": SimpleNamespace(trafficModeEnabled=traffic_mode_enabled),
  }


def test_low_speed_cruise_does_not_trigger_stop_light_from_model_stopped():
  v_ego = 10 * CV.MPH_TO_MS
  model_length = v_ego * 10.0

  cem = make_cem(model_length=model_length, model_stopped=True)
  cem.stop_sign_and_light(v_ego, make_sm(), model_time=7.0)

  assert not cem.stop_light_detected


def test_predicted_stop_within_threshold_triggers_stop_light():
  v_ego = 30 * CV.MPH_TO_MS
  model_length = v_ego * 4.0

  cem = make_cem(model_length=model_length)
  cem.stop_sign_and_light(v_ego, make_sm(), model_time=7.0)

  assert cem.stop_light_detected


class DummyThemeManager:
  def update_wheel_image(self, *args, **kwargs):
    pass


def test_starpilot_planner_updates_cem_with_current_frame_state(monkeypatch):
  planner = StarPilotPlanner(Path("/tmp/nonexistent"), DummyThemeManager())

  try:
    monkeypatch.setattr(starpilot_planner_module, "calculate_road_curvature", lambda model, v_ego: (0.01, 1.0))
    monkeypatch.setattr(planner.starpilot_acceleration, "update", lambda *args, **kwargs: None)
    monkeypatch.setattr(planner.starpilot_events, "update", lambda *args, **kwargs: None)
    monkeypatch.setattr(planner.starpilot_vcruise, "update", lambda *args, **kwargs: 0.0)
    monkeypatch.setattr(planner.starpilot_weather, "update_weather", lambda *args, **kwargs: None)

    def following_update(*args, **kwargs):
      planner.starpilot_following.following_lead = planner.tracking_lead
      planner.starpilot_following.slower_lead = planner.tracking_lead

    monkeypatch.setattr(planner.starpilot_following, "update", following_update)

    seen = {}

    def cem_update(v_ego, sm, starpilot_toggles):
      seen.update({
        "tracking_lead": planner.tracking_lead,
        "following_lead": planner.starpilot_following.following_lead,
        "slower_lead": planner.starpilot_following.slower_lead,
        "model_length": planner.model_length,
        "driving_in_curve": planner.driving_in_curve,
        "road_curvature_detected": planner.road_curvature_detected,
      })

    monkeypatch.setattr(planner.starpilot_cem, "update", cem_update)

    planner.model_length = 0.0
    planner.tracking_lead = False
    planner.driving_in_curve = False
    planner.lateral_acceleration = 0.0
    planner.road_curvature_detected = False
    planner.starpilot_following.following_lead = False
    planner.starpilot_following.slower_lead = False

    sm = {
      "radarState": SimpleNamespace(leadOne=SimpleNamespace(status=True, dRel=25.0, vLead=0.5)),
      "selfdriveState": SimpleNamespace(enabled=True),
      "carState": SimpleNamespace(vCruise=50.0, vEgo=20.0, standstill=False, leftBlinker=False, rightBlinker=False),
      "controlsState": SimpleNamespace(curvature=0.02),
      "modelV2": SimpleNamespace(position=SimpleNamespace(x=[0.0, 30.0]), laneLines=[None] * 4, roadEdges=[None] * 2),
      "starpilotCarState": SimpleNamespace(pauseLateral=False),
      planner.gps_location_service: SimpleNamespace(latitude=1.0, longitude=1.0, bearingDeg=90.0),
    }
    starpilot_toggles = SimpleNamespace(
      set_speed_offset=0,
      conditional_experimental_mode=True,
      minimum_lane_change_speed=100.0,
      pause_lateral_below_speed=0.0,
      pause_lateral_below_signal=False,
      stop_distance=6.0,
      weather_presets=False,
    )

    planner.update(0.0, False, sm, starpilot_toggles)

    assert seen == {
      "tracking_lead": True,
      "following_lead": True,
      "slower_lead": True,
      "model_length": 30.0,
      "driving_in_curve": True,
      "road_curvature_detected": True,
    }
  finally:
    planner.shutdown()
