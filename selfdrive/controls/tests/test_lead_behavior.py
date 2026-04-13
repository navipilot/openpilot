from openpilot.selfdrive.controls.lib.lead_behavior import (
  get_tracked_lead_catchup_bias,
  should_disable_far_lead_throttle,
)


def test_tracked_lead_catchup_bias_for_hanging_gap():
  bias = get_tracked_lead_catchup_bias(31.4, 78.7, 38.0, 0.1)
  assert bias > 10.0


def test_tracked_lead_catchup_bias_ignores_near_desired_gap():
  bias = get_tracked_lead_catchup_bias(31.4, 50.0, 38.0, 0.1)
  assert bias == 0.0


def test_tracked_lead_catchup_bias_ignores_very_far_gap():
  bias = get_tracked_lead_catchup_bias(31.4, 110.0, 38.0, 0.1)
  assert bias == 0.0


def test_tracked_lead_catchup_bias_applies_to_two_second_highway_gap():
  bias = get_tracked_lead_catchup_bias(30.4, 63.0, 40.0, 0.4)
  assert bias > 14.0


def test_tracked_lead_catchup_bias_stays_off_once_at_set_speed():
  bias = get_tracked_lead_catchup_bias(31.4, 78.7, 38.0, 0.1, v_cruise=31.4)
  assert bias == 0.0


def test_disable_far_lead_throttle_rejects_two_second_plus_gap():
  should_disable = should_disable_far_lead_throttle(31.4, 78.7, 38.0, 0.1, False)
  assert not should_disable


def test_disable_far_lead_throttle_keeps_mild_coast_near_target_gap():
  should_disable = should_disable_far_lead_throttle(31.4, 52.0, 38.0, 0.5, False)
  assert should_disable


def test_disable_far_lead_throttle_rejects_fast_closing():
  should_disable = should_disable_far_lead_throttle(31.4, 52.0, 38.0, 3.5, False)
  assert not should_disable
