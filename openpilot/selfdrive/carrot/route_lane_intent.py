from __future__ import annotations

from dataclasses import dataclass
from typing import Any


NOA_INTENT_SCHEMA_VERSION = 1
NOA_INTENT_MAX_AGE_MS = 5_000
NOA_INTENT_MAX_FUTURE_SKEW_MS = 1_000
MAX_LANES = 16
SUPPORTED_MANEUVER_TYPES = frozenset({
  "none", "guidance", "exit", "exit_left", "exit_right", "fork", "ramp",
})


@dataclass(frozen=True)
class RouteLaneIntent:
  valid: bool = False
  route_generation: int = 0
  route_change_reason: str = ""
  controlled_access: bool = False
  maneuver_type: str = "none"
  distance_to_maneuver_m: int = 0
  lane_count: int = 0
  preferred_lane_mask: int = 0
  current_lane_hint: int = -1
  required_lane_changes_hint: int = 0
  next_lane_change_direction: str = "none"
  confidence: float = 0.0
  reject_reason: str = "missing"


def _dict(value: Any) -> dict[str, Any]:
  return value if isinstance(value, dict) else {}


def _integer(value: Any, default: int = 0) -> int:
  if isinstance(value, bool):
    return default
  try:
    return int(value)
  except (TypeError, ValueError):
    return default


def _finite_float(value: Any, default: float = 0.0) -> float:
  try:
    parsed = float(value)
  except (TypeError, ValueError):
    return default
  return parsed if parsed == parsed and abs(parsed) != float("inf") else default


def _reject(reason: str, **values: Any) -> RouteLaneIntent:
  return RouteLaneIntent(reject_reason=reason, **values)


def parse_route_lane_intent(value: Any, now_ms: int) -> RouteLaneIntent:
  """Validates an app NOA intent for diagnostics only; it has no control authority."""
  intent = _dict(value)
  if _integer(intent.get("schema_version"), -1) != NOA_INTENT_SCHEMA_VERSION:
    return _reject("unsupported_schema")

  freshness = _dict(intent.get("freshness"))
  source_timestamp_ms = _integer(freshness.get("source_timestamp_ms"), -1)
  if source_timestamp_ms <= 0:
    return _reject("missing_timestamp")
  if source_timestamp_ms > now_ms + NOA_INTENT_MAX_FUTURE_SKEW_MS:
    return _reject("future_timestamp")
  if now_ms - source_timestamp_ms > NOA_INTENT_MAX_AGE_MS:
    return _reject("stale")

  route_generation = _integer(intent.get("route_generation"), -1)
  if route_generation < 0:
    return _reject("invalid_route_generation")
  route_change_reason = str(_dict(intent.get("route")).get("generation_reason") or "")[:64]

  road = _dict(intent.get("road"))
  if not bool(road.get("controlled_access", False)):
    return _reject("unsupported_road", route_generation=route_generation)

  maneuver = _dict(intent.get("maneuver"))
  maneuver_type = str(maneuver.get("type") or "none")
  if maneuver_type not in SUPPORTED_MANEUVER_TYPES:
    return _reject("unsupported_maneuver", route_generation=route_generation, controlled_access=True)

  lanes = _dict(intent.get("lanes"))
  lane_count = _integer(lanes.get("lane_count"), -1)
  lane_values = lanes.get("lanes")
  if lane_count <= 0 or lane_count > MAX_LANES or not isinstance(lane_values, list) or len(lane_values) != lane_count:
    return _reject("invalid_lanes", route_generation=route_generation, controlled_access=True, maneuver_type=maneuver_type)
  if lanes.get("index_order") != "left_to_right":
    return _reject("unsupported_lane_order", route_generation=route_generation, controlled_access=True, maneuver_type=maneuver_type)

  preferred_lane_mask = _integer(lanes.get("preferred_lane_mask"), -1)
  valid_mask = (1 << lane_count) - 1
  if preferred_lane_mask < 0 or preferred_lane_mask & ~valid_mask:
    return _reject("invalid_preferred_lanes", route_generation=route_generation, controlled_access=True, maneuver_type=maneuver_type, lane_count=lane_count)

  preferred_lanes = [index for index in range(lane_count) if preferred_lane_mask & (1 << index)]
  if maneuver_type not in ("none", "guidance") and not preferred_lanes:
    return _reject("missing_preferred_lanes", route_generation=route_generation, controlled_access=True, maneuver_type=maneuver_type, lane_count=lane_count)

  current_lane_hint = _integer(lanes.get("current_lane_hint"), -1)
  if current_lane_hint < -1 or current_lane_hint >= lane_count:
    return _reject("invalid_current_lane", route_generation=route_generation, controlled_access=True, maneuver_type=maneuver_type, lane_count=lane_count)

  distance_m = max(0, _integer(maneuver.get("distance_m"), 0))
  confidence = min(1.0, max(0.0, _finite_float(road.get("confidence"), 0.0)))
  target_lane = min(preferred_lanes, key=lambda index: abs(index - current_lane_hint)) if current_lane_hint >= 0 and preferred_lanes else -1
  required_lane_changes = abs(target_lane - current_lane_hint) if target_lane >= 0 else 0
  direction = "left" if target_lane < current_lane_hint else "right" if target_lane > current_lane_hint else "none"

  return RouteLaneIntent(
    valid=True,
    route_generation=route_generation,
    route_change_reason=route_change_reason,
    controlled_access=True,
    maneuver_type=maneuver_type,
    distance_to_maneuver_m=distance_m,
    lane_count=lane_count,
    preferred_lane_mask=preferred_lane_mask,
    current_lane_hint=current_lane_hint,
    required_lane_changes_hint=required_lane_changes,
    next_lane_change_direction=direction,
    confidence=confidence,
    reject_reason="",
  )
