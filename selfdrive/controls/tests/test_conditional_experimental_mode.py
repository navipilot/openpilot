from pathlib import Path
from types import SimpleNamespace

from openpilot.common.constants import CV
from openpilot.starpilot.controls.starpilot_planner import StarPilotPlanner
import openpilot.starpilot.controls.starpilot_planner as starpilot_planner_module
import openpilot.starpilot.controls.lib.conditional_experimental_mode as conditional_experimental_mode_module
from openpilot.starpilot.controls.lib.conditional_experimental_mode import ConditionalExperimentalMode


def make_cem(*, model_length: float, model_stopped: bool = False, tracking_lead: bool = False,
             lead_status: bool = False, lead_d_rel: float = float("inf"),
             lead_v_lead: float = 0.0, lead_model_prob: float = 0.0, lead_radar: bool = False):
  planner = SimpleNamespace(
    params=None,
    params_memory=None,
    model_length=model_length,
    model_stopped=model_stopped,
    tracking_lead=tracking_lead,
    starpilot_vcruise=SimpleNamespace(stop_sign_confirmed=False),
    starpilot_following=SimpleNamespace(slower_lead=False, following_lead=False),
    lead_one=SimpleNamespace(status=lead_status, dRel=lead_d_rel, vLead=lead_v_lead,
                             modelProb=lead_model_prob, radar=lead_radar),
  )
  return ConditionalExperimentalMode(planner)


def make_sm(traffic_mode_enabled: bool = False):
  return {
    "starpilotCarState": SimpleNamespace(trafficModeEnabled=traffic_mode_enabled),
  }


def run_stop_light_detector(cem, v_ego, *, steps: int, tracking_lead: bool = False):
  for _ in range(steps):
    cem.starpilot_planner.tracking_lead = tracking_lead
    cem.stop_sign_and_light(v_ego, make_sm(), model_time=7.0)


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
  run_stop_light_detector(cem, v_ego, steps=20)

  assert cem.stop_light_detected


def test_chattering_lead_does_not_trigger_stop_light():
  v_ego = 22 * CV.MPH_TO_MS
  model_length = v_ego * 4.0

  cem = make_cem(model_length=model_length)
  for i in range(30):
    cem.starpilot_planner.tracking_lead = (i % 2 == 0)
    cem.starpilot_planner.lead_one.status = (i % 2 == 0)
    cem.starpilot_planner.lead_one.dRel = model_length + 5.0
    cem.stop_sign_and_light(v_ego, make_sm(), model_time=7.0)

  assert not cem.stop_light_detected


def test_close_visible_but_untracked_lead_blocks_stop_light():
  v_ego = 22 * CV.MPH_TO_MS
  model_length = v_ego * 4.0

  cem = make_cem(model_length=model_length, lead_status=True, lead_d_rel=model_length + 5.0)
  run_stop_light_detector(cem, v_ego, steps=30)

  assert not cem.stop_light_detected


def test_far_visible_lead_does_not_block_stop_light():
  v_ego = 22 * CV.MPH_TO_MS
  model_length = v_ego * 4.0

  cem = make_cem(model_length=model_length, lead_status=True, lead_d_rel=v_ego * 7.0 + 30.0)
  run_stop_light_detector(cem, v_ego, steps=30)

  assert cem.stop_light_detected


def test_stop_light_stays_latched_until_untracked_stopped_lead_handoff():
  v_ego = 45 * CV.MPH_TO_MS
  model_length = v_ego * 4.0

  cem = make_cem(model_length=model_length)
  run_stop_light_detector(cem, v_ego, steps=30)
  assert cem.stop_light_detected

  cem.starpilot_planner.lead_one.status = True
  cem.starpilot_planner.lead_one.dRel = model_length - 5.0
  cem.starpilot_planner.lead_one.vLead = 0.5
  run_stop_light_detector(cem, v_ego, steps=10, tracking_lead=False)

  assert cem.stop_light_detected


def test_stop_light_latch_holds_slow_high_confidence_vision_lead_during_model_flicker(monkeypatch):
  v_ego = 40 * CV.MPH_TO_MS
  model_length = v_ego * 3.8
  cem = make_cem(
    model_length=model_length,
    lead_status=True,
    lead_d_rel=model_length - 5.0,
    lead_v_lead=3.0,
    lead_model_prob=0.98,
  )

  run_stop_light_detector(cem, v_ego, steps=20)
  assert cem.stop_light_detected

  monotonic_values = iter([10.0, 10.2])
  monkeypatch.setattr(conditional_experimental_mode_module.time, "monotonic", lambda: next(monotonic_values))

  cem.stop_sign_and_light(v_ego, make_sm(), model_time=7.0)
  cem.stop_light_detected = False
  cem.stop_light_model_detected = False
  cem.stop_light_filter.x = 0.0
  cem.starpilot_planner.model_length = v_ego * 9.0
  cem.stop_sign_and_light(v_ego, make_sm(), model_time=7.0)

  assert cem.stop_light_detected


def test_slow_lead_holds_through_tracking_flap_for_high_confidence_vision_lead():
  v_ego = 35 * CV.MPH_TO_MS
  cem = make_cem(
    model_length=v_ego * 5.0,
    tracking_lead=True,
    lead_status=True,
    lead_d_rel=v_ego * 5.0,
    lead_v_lead=8.0 * CV.MPH_TO_MS,
    lead_model_prob=0.95,
  )
  toggles = SimpleNamespace(conditional_slower_lead=True, conditional_stopped_lead=False)

  cem.slow_lead_filter.x = 1.0
  cem.slow_lead_detected = True

  cem.starpilot_planner.tracking_lead = False
  cem.starpilot_planner.starpilot_following.slower_lead = False
  cem.slow_lead(toggles, v_ego)
  assert cem.slow_lead_detected


def test_slow_lead_does_not_linger_at_crawl_when_stopped_lead_disabled():
  v_ego = 1.5
  cem = make_cem(
    model_length=20.0,
    tracking_lead=True,
    lead_status=True,
    lead_d_rel=8.0,
    lead_v_lead=1.2,
    lead_model_prob=0.99,
  )
  toggles = SimpleNamespace(conditional_slower_lead=True, conditional_stopped_lead=False)

  cem.slow_lead_filter.x = 1.0
  cem.slow_lead_detected = True
  cem.starpilot_planner.starpilot_following.slower_lead = False
  cem.slow_lead(toggles, v_ego)

  assert not cem.slow_lead_detected


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

    starpilot_toggles = SimpleNamespace(
      set_speed_offset=0,
      conditional_experimental_mode=True,
      minimum_lane_change_speed=100.0,
      pause_lateral_below_speed=0.0,
      pause_lateral_below_signal=False,
      stop_distance=6.0,
      weather_presets=False,
    )

    for controls_enabled, aol_enabled in ((True, False), (False, True)):
      seen.clear()

      sm = {
        "radarState": SimpleNamespace(leadOne=SimpleNamespace(status=True, dRel=25.0, vLead=0.5)),
        "selfdriveState": SimpleNamespace(enabled=controls_enabled),
        "carState": SimpleNamespace(vCruise=50.0, vEgo=20.0, standstill=False, leftBlinker=False, rightBlinker=False),
        "controlsState": SimpleNamespace(curvature=0.02),
        "modelV2": SimpleNamespace(position=SimpleNamespace(x=[0.0, 30.0]), laneLines=[None] * 4, roadEdges=[None] * 2),
        "starpilotCarState": SimpleNamespace(pauseLateral=False, alwaysOnLateralEnabled=aol_enabled),
        planner.gps_location_service: SimpleNamespace(latitude=1.0, longitude=1.0, bearingDeg=90.0),
      }

      planner.tracking_lead_filter.x = 1.0
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
