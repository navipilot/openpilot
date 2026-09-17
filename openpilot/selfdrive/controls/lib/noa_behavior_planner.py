from __future__ import annotations

from dataclasses import dataclass

from openpilot.selfdrive.carrot.route_lane_intent import RouteLaneIntent
from openpilot.selfdrive.controls.lib.lane_localizer import LaneLocalization


LANE_CHANGE_TIME_S = 7.0
SETTLE_DISTANCE_M = 60.0
JUNCTION_FREEZE_DISTANCE_M = 200.0
SAFETY_MARGIN_M = 150.0


@dataclass(frozen=True)
class NoaBehaviorPlan:
  state: str
  request_active: bool = False
  direction: str = "none"
  required_distance_m: float = 0.0
  remaining_lane_changes: int = 0
  confidence: float = 0.0
  reject_reason: str = ""


def _required_distance_m(lane_changes: int, ego_speed_mps: float) -> float:
  speed = max(0.0, ego_speed_mps)
  return lane_changes * (speed * LANE_CHANGE_TIME_S + SETTLE_DISTANCE_M) + JUNCTION_FREEZE_DISTANCE_M + SAFETY_MARGIN_M


class NoaBehaviorPlanner:
  """Shadow-only NOA planner. request_active is intentionally always false."""

  def update(
    self,
    intent: RouteLaneIntent,
    localization: LaneLocalization,
    *,
    ego_speed_mps: float,
    safety_blocked: bool,
    control_active: bool,
  ) -> NoaBehaviorPlan:
    if not intent.valid:
      state = "NAV_STALE" if intent.reject_reason in ("stale", "future_timestamp") else "UNSUPPORTED_ROAD"
      return NoaBehaviorPlan(state=state, reject_reason=intent.reject_reason)
    if not control_active:
      return NoaBehaviorPlan(state="OFF", reject_reason="control_inactive")
    if safety_blocked:
      return NoaBehaviorPlan(state="BLOCKED", reject_reason="safety_gate")
    if not localization.valid:
      return NoaBehaviorPlan(
        state="LOW_CONFIDENCE",
        confidence=localization.confidence,
        reject_reason=localization.reason,
      )

    preferred_lanes = [
      index for index in range(intent.lane_count)
      if intent.preferred_lane_mask & (1 << index)
    ]
    target_lane = min(preferred_lanes, key=lambda index: abs(index - localization.estimated_lane)) if preferred_lanes else -1
    lane_changes = abs(target_lane - localization.estimated_lane) if target_lane >= 0 else 0
    direction = "left" if target_lane < localization.estimated_lane else "right" if target_lane > localization.estimated_lane else "none"
    required_distance_m = _required_distance_m(lane_changes, ego_speed_mps)
    if lane_changes == 0:
      return NoaBehaviorPlan(
        state="ACTIVE_CRUISE",
        required_distance_m=required_distance_m,
        confidence=localization.confidence,
      )
    if intent.distance_to_maneuver_m <= required_distance_m:
      return NoaBehaviorPlan(
        state="MISSED_EXIT",
        direction=direction,
        required_distance_m=required_distance_m,
        remaining_lane_changes=lane_changes,
        confidence=localization.confidence,
        reject_reason="insufficient_distance",
      )
    return NoaBehaviorPlan(
      state="WAIT_DRIVER_CONFIRM",
      direction=direction,
      required_distance_m=required_distance_m,
      remaining_lane_changes=lane_changes,
      confidence=localization.confidence,
      reject_reason="shadow_only",
    )
