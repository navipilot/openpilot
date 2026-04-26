#!/usr/bin/env python3
from openpilot.common.constants import CV


HIGHWAY_LEAD_BEHAVIOR_MIN_SPEED = 45. * CV.MPH_TO_MS
VISION_LEAD_TRACK_MIN_DISTANCE = 25.0
VISION_LEAD_TRACK_BASE_TIME_GAP = 1.75
VISION_LEAD_TRACK_CLOSING_GAIN = 0.20
VISION_LEAD_TRACK_CLOSING_CAP = 2.50


def should_track_lead(lead_status: bool, lead_distance: float, model_length: float, stop_distance: float,
                      v_ego: float, *, v_lead: float | None = None, radar: bool = False) -> bool:
  if not lead_status:
    return False

  tracking_buffer = max(float(stop_distance), 4.0)
  model_limit = float(model_length) + tracking_buffer
  if radar:
    return float(lead_distance) < model_limit

  closing_speed = max(0.0, float(v_ego) - float(v_lead if v_lead is not None else v_ego))
  vision_time_gap = VISION_LEAD_TRACK_BASE_TIME_GAP + min(closing_speed * VISION_LEAD_TRACK_CLOSING_GAIN,
                                                          VISION_LEAD_TRACK_CLOSING_CAP)
  vision_limit = max(VISION_LEAD_TRACK_MIN_DISTANCE, float(v_ego) * vision_time_gap + tracking_buffer)
  return float(lead_distance) < min(model_limit, vision_limit)


def get_tracked_lead_catchup_bias(v_ego: float, lead_distance: float, desired_gap: float, closing_speed: float,
                                  v_cruise: float | None = None) -> float:
  gap_error = lead_distance - desired_gap
  actual_hw = lead_distance / max(v_ego, 1e-3)
  desired_hw = desired_gap / max(v_ego, 1e-3)

  if v_ego <= HIGHWAY_LEAD_BEHAVIOR_MIN_SPEED:
    return 0.0
  if v_cruise is not None and v_ego >= v_cruise:
    return 0.0
  if gap_error <= 0.0:
    return 0.0

  # Encourage ACC to treat a tracked lead as the active constraint when we're
  # hanging far above the requested time gap, but don't override cruise for a
  # truly distant lead or one we're already closing on decisively.
  if actual_hw <= max(desired_hw + 0.3, 1.72):
    return 0.0
  if actual_hw >= max(desired_hw + 1.6, 3.0):
    return 0.0
  if closing_speed > max(2.5, 0.12 * v_ego):
    return 0.0

  return min(gap_error * 0.65, max(14.0, 0.75 * v_ego))


def should_disable_far_lead_throttle(v_ego: float, lead_distance: float, desired_gap: float,
                                     closing_speed: float, following_lead: bool) -> bool:
  actual_hw = lead_distance / max(v_ego, 1e-3)
  desired_hw = desired_gap / max(v_ego, 1e-3)

  if following_lead or v_ego <= HIGHWAY_LEAD_BEHAVIOR_MIN_SPEED:
    return False

  # Don't coast if we're already materially above the requested headway.
  if actual_hw > max(desired_hw + 0.45, 1.85):
    return False

  coast_window_open = lead_distance > desired_gap + max(4.0, 0.15 * v_ego)
  coast_window_far = lead_distance < desired_gap + max(15.0, 0.75 * v_ego)
  gentle_closing = closing_speed < max(1.5, 0.08 * v_ego)
  ttc = lead_distance / max(closing_speed, 1e-3) if closing_speed > 0.1 else 1e6

  return coast_window_open and coast_window_far and gentle_closing and ttc > 6.0 and lead_distance > desired_gap + 6.0
