from __future__ import annotations

from dataclasses import dataclass
from math import isfinite
from typing import Sequence


MIN_LANE_CONFIDENCE = 0.60
MIN_HINT_QUALITY = 0.70
MIN_TRACKING_QUALITY = 0.55
MAX_LANES = 16


@dataclass(frozen=True)
class LaneLocalization:
  estimated_lane: int = -1
  lane_probabilities: tuple[float, ...] = ()
  confidence: float = 0.0
  lane_count: int = 0
  valid: bool = False
  reason: str = "no_navigation_intent"


def _quality(lane_line_probs: Sequence[float], road_edge_distances: Sequence[float]) -> float:
  if len(lane_line_probs) < 3:
    return 0.0
  try:
    left, right = float(lane_line_probs[1]), float(lane_line_probs[2])
  except (TypeError, ValueError):
    return 0.0
  if not (isfinite(left) and isfinite(right)):
    return 0.0
  lane_quality = min(1.0, max(0.0, (left + right) / 2.0))
  edge_quality = 1.0
  if len(road_edge_distances) >= 2:
    try:
      edges = [float(distance) for distance in road_edge_distances[:2]]
      edge_quality = 1.0 if all(isfinite(distance) and distance >= 0.0 for distance in edges) else 0.0
    except (TypeError, ValueError):
      edge_quality = 0.0
  return lane_quality * edge_quality


class LaneLocalizer:
  """Conservative shadow lane estimator; it never produces a control request."""

  def __init__(self) -> None:
    self._route_generation: int | None = None
    self._lane_count = 0
    self._estimated_lane = -1
    self._confidence = 0.0
    self._was_changing = False
    self._pending_direction = "none"

  def reset(self) -> None:
    self._estimated_lane = -1
    self._confidence = 0.0
    self._was_changing = False
    self._pending_direction = "none"

  def update(
    self,
    *,
    route_generation: int,
    lane_count: int,
    current_lane_hint: int,
    lane_line_probs: Sequence[float],
    road_edge_distances: Sequence[float],
    lane_change_active: bool,
    lane_change_direction: str,
  ) -> LaneLocalization:
    if lane_count <= 0 or lane_count > MAX_LANES:
      self.reset()
      return LaneLocalization(reason="invalid_lane_count")

    route_changed = self._route_generation is not None and self._route_generation != route_generation
    lane_count_changed = self._lane_count not in (0, lane_count)
    if route_changed or lane_count_changed:
      self.reset()
    self._route_generation = route_generation
    self._lane_count = lane_count

    quality = _quality(lane_line_probs, road_edge_distances)
    direction = lane_change_direction if lane_change_direction in ("left", "right") else "none"
    if lane_change_active and direction != "none":
      self._pending_direction = direction
    elif self._was_changing and self._pending_direction != "none":
      delta = -1 if self._pending_direction == "left" else 1
      candidate = self._estimated_lane + delta
      if quality >= MIN_TRACKING_QUALITY and 0 <= candidate < lane_count:
        self._estimated_lane = candidate
        self._confidence = min(0.80, self._confidence + 0.15)
      else:
        self.reset()
      self._pending_direction = "none"
    self._was_changing = lane_change_active

    if self._estimated_lane < 0 and 0 <= current_lane_hint < lane_count and quality >= MIN_HINT_QUALITY:
      self._estimated_lane = current_lane_hint
      self._confidence = 0.35
      reason = "map_hint_pending_verification"
    elif self._estimated_lane < 0:
      reason = "current_lane_unknown"
    elif quality < MIN_TRACKING_QUALITY:
      self._confidence = max(0.0, self._confidence - 0.10)
      reason = "low_lane_confidence"
    else:
      self._confidence = min(0.80, self._confidence + 0.02)
      reason = "tracking"

    probabilities = [0.0] * lane_count
    if self._estimated_lane >= 0:
      probabilities[self._estimated_lane] = self._confidence
      remaining = (1.0 - self._confidence) / max(1, lane_count - 1)
      for index in range(lane_count):
        if index != self._estimated_lane:
          probabilities[index] = remaining
    else:
      probabilities = [1.0 / lane_count] * lane_count
    return LaneLocalization(
      estimated_lane=self._estimated_lane,
      lane_probabilities=tuple(probabilities),
      confidence=self._confidence,
      lane_count=lane_count,
      valid=self._estimated_lane >= 0 and self._confidence >= MIN_LANE_CONFIDENCE,
      reason=reason,
    )
