from openpilot.selfdrive.carrot.route_lane_intent import RouteLaneIntent
from openpilot.selfdrive.controls.lib.lane_localizer import LaneLocalization
from openpilot.selfdrive.controls.lib.noa_behavior_planner import NoaBehaviorPlanner


def intent(distance=1_500):
  return RouteLaneIntent(
    valid=True, controlled_access=True, maneuver_type="exit_right",
    distance_to_maneuver_m=distance, lane_count=3, preferred_lane_mask=4,
  )


def localization(valid=True, lane=1, confidence=0.7):
  return LaneLocalization(
    valid=valid, estimated_lane=lane, confidence=confidence,
    lane_count=3, reason="tracking",
  )


def test_shadow_planner_waits_for_driver_and_never_requests_control():
  plan = NoaBehaviorPlanner().update(
    intent(), localization(), ego_speed_mps=25.0, safety_blocked=False, control_active=True,
  )

  assert plan.state == "WAIT_DRIVER_CONFIRM"
  assert plan.direction == "right"
  assert plan.request_active is False


def test_shadow_planner_rejects_safety_low_confidence_and_late_maneuvers():
  planner = NoaBehaviorPlanner()

  assert planner.update(intent(), localization(), ego_speed_mps=25.0, safety_blocked=True, control_active=True).state == "BLOCKED"
  assert planner.update(intent(), localization(valid=False, confidence=0.3), ego_speed_mps=25.0, safety_blocked=False, control_active=True).state == "LOW_CONFIDENCE"
  late = planner.update(intent(distance=500), localization(), ego_speed_mps=25.0, safety_blocked=False, control_active=True)
  assert late.state == "MISSED_EXIT"
  assert late.request_active is False
