import math

import numpy as np
import pytest

from openpilot.sunnypilot.selfdrive.traffic_control.stop_profile import StopProfileGenerator


TIMES = np.array([0.0, 0.05, 0.15, 0.30, 0.50, 0.80, 1.20, 1.70, 2.30, 3.00])


def test_profile_starts_gentle_deceleration_for_red_at_100m():
  profile = StopProfileGenerator(comfort_brake=2.4, jerk_limit=0.8)
  speeds, accels, jerks = profile.build_stop(v_ego=16.7, a_ego=0.0, remaining_distance=94.0, times=TIMES)
  assert speeds[-1] < speeds[0]
  assert accels[1] < 0.0
  assert np.min(accels) >= -2.4
  assert np.max(np.abs(jerks)) <= 0.8 + 1e-6


def test_profile_holds_zero_speed_without_negative_velocity():
  profile = StopProfileGenerator()
  speeds, accels, _ = profile.build_stop(v_ego=0.0, a_ego=0.0, remaining_distance=0.0, times=TIMES, hold=True)
  assert np.all(speeds == 0.0)
  assert np.all(accels == 0.0)


def test_release_profile_limits_initial_acceleration_and_ramps():
  profile = StopProfileGenerator(release_jerk_limit=0.5)
  profile.previous_accel = -0.2
  speeds, accels, jerks = profile.build_release(v_ego=0.0, base_accel=1.0, times=TIMES)
  assert accels[1] < 0.0
  assert accels[-1] > accels[1]
  assert np.max(jerks) <= 0.5 + 1e-6


def test_required_stop_distance_uses_delay_jerk_and_brake_consistently():
  gentle = StopProfileGenerator.required_stop_distance(
    v_ego=15.0, a_ego=0.0, actuator_delay=0.2, max_brake=2.2, jerk_limit=0.55,
  )
  standard = StopProfileGenerator.required_stop_distance(
    v_ego=15.0, a_ego=0.0, actuator_delay=0.2, max_brake=2.5, jerk_limit=0.8,
  )
  maximum = StopProfileGenerator.required_stop_distance(
    v_ego=15.0, a_ego=0.0, actuator_delay=0.2, max_brake=3.0, jerk_limit=1.1,
  )
  assert gentle > standard > maximum > 0.0


def test_positive_acceleration_and_longer_delay_increase_required_stop_distance():
  baseline = StopProfileGenerator.required_stop_distance(
    v_ego=10.0, a_ego=0.0, actuator_delay=0.2, max_brake=3.0, jerk_limit=1.0,
  )
  delayed = StopProfileGenerator.required_stop_distance(
    v_ego=10.0, a_ego=0.5, actuator_delay=0.5, max_brake=3.0, jerk_limit=1.0,
  )
  assert delayed > baseline


def test_required_stop_distance_matches_closed_form_route11_yellow_case():
  distance = StopProfileGenerator.required_stop_distance(
    v_ego=10.707, a_ego=-1.111, actuator_delay=0.25,
    max_brake=3.0, jerk_limit=1.1,
  )
  assert distance == pytest.approx(25.918873, abs=1e-5)


def test_required_stop_distance_stops_inside_the_delay_interval():
  distance = StopProfileGenerator.required_stop_distance(
    v_ego=1.0, a_ego=-2.0, actuator_delay=1.0,
    max_brake=3.0, jerk_limit=1.0,
  )
  assert distance == pytest.approx(0.25)


def test_required_stop_distance_stops_inside_the_jerk_ramp():
  distance = StopProfileGenerator.required_stop_distance(
    v_ego=1.0, a_ego=0.0, actuator_delay=0.0,
    max_brake=10.0, jerk_limit=1.0,
  )
  assert distance == pytest.approx(2.0 * math.sqrt(2.0) / 3.0)


def test_required_stop_distance_includes_the_constant_max_brake_segment():
  distance = StopProfileGenerator.required_stop_distance(
    v_ego=10.0, a_ego=0.0, actuator_delay=0.0,
    max_brake=2.0, jerk_limit=1.0,
  )
  assert distance == pytest.approx(104.0 / 3.0)


def test_positive_acceleration_is_not_clipped_to_the_braking_magnitude():
  bounded = StopProfileGenerator.required_stop_distance(
    v_ego=10.0, a_ego=2.0, actuator_delay=0.5,
    max_brake=2.0, jerk_limit=1.0,
  )
  accelerating = StopProfileGenerator.required_stop_distance(
    v_ego=10.0, a_ego=5.0, actuator_delay=0.5,
    max_brake=2.0, jerk_limit=1.0,
  )
  assert accelerating > bounded


@pytest.mark.parametrize(("field", "value"), [
  ("v_ego", float("nan")),
  ("a_ego", float("nan")),
  ("actuator_delay", float("nan")),
  ("max_brake", float("nan")),
  ("jerk_limit", float("nan")),
  ("v_ego", float("inf")),
])
def test_non_finite_stop_envelope_inputs_fail_closed(field, value):
  inputs = {
    "v_ego": 10.0,
    "a_ego": 0.0,
    "actuator_delay": 0.2,
    "max_brake": 2.5,
    "jerk_limit": 0.8,
  }
  inputs[field] = value
  assert math.isinf(StopProfileGenerator.required_stop_distance(**inputs))


def test_legacy_dt_argument_does_not_change_the_closed_form_result():
  inputs = {
    "v_ego": 12.0,
    "a_ego": 0.3,
    "actuator_delay": 0.25,
    "max_brake": 2.5,
    "jerk_limit": 0.8,
  }
  assert StopProfileGenerator.required_stop_distance(**inputs, dt=0.5) == pytest.approx(
    StopProfileGenerator.required_stop_distance(**inputs, dt=0.001),
  )
