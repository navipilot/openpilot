"""
Copyright (c) 2026, Rick Lan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sublicense,
for non-commercial purposes only, subject to the following conditions:

- The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
- Commercial use (e.g. use in a product, service, or activity intended to
  generate revenue) is prohibited without explicit written permission from
  the copyright holder.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Route Tracker - OsmAnd-inspired route tracking logic

Handles:
- Step transition detection (finding closest segment among lookahead)
- Reroute detection with time-based debounce
- GPS accuracy-aware tolerances

References:
- OsmAnd RoutingHelper.java: lookAheadFindMinOrthogonalDistance (line 562)
- OsmAnd RoutingHelperUtils.java: bearing thresholds (lines 175, 200)
- OsmAnd AnnounceTimeDistances.java: positioning tolerance (line 25)
"""
import time
from typing import Optional, TYPE_CHECKING

from openpilot.common.swaglog import cloudlog

from dragonpilot.dashy.maa.helpers import (
  Coordinate,
  minimum_distance,
  normalize_angle,
  project_onto_segment,
)

if TYPE_CHECKING:
  from dragonpilot.dashy.maa.maad import Step


# Reroute parameters (OsmAnd-inspired)
REROUTE_DISTANCE_BASE = 25  # meters off route before considering reroute
REROUTE_DEBOUNCE_TIME = 3.0  # seconds of sustained deviation before reroute
POSITIONING_TOLERANCE = 12  # meters GPS error buffer

# Bearing thresholds (OsmAnd: RoutingHelperUtils.java)
WRONG_DIRECTION_THRESHOLD = 90.0  # degrees - wrong movement direction
UTURN_THRESHOLD = 135.0  # degrees - U-turn needed

# Speed-based lookahead (OsmAnd: RoutingHelper.java:562)
LOOKAHEAD_SLOW = 8  # segments for slow speed
LOOKAHEAD_FAST = 15  # segments for fast speed (highway)
FAST_SPEED_THRESHOLD = 20.0  # m/s (~72 km/h) to switch to fast lookahead

# Step transition - snap zone around maneuver point
MANEUVER_SNAP_ZONE = 100.0  # meters before/after maneuver to check

# Maneuver point detection (bearing-based)
MANEUVER_PROXIMITY_THRESHOLD = 50.0  # meters - must be this close to check bearing
MANEUVER_BEHIND_ANGLE = 90.0  # degrees - point is "behind" if angle > this


class RouteTracker:
  """Tracks position along a route with snap-to-road logic."""

  def __init__(self):
    self.step_idx: Optional[int] = None
    self.segment_idx: int = 0  # Current segment within step
    self.deviation_start_time: Optional[float] = None

  def reset(self):
    """Reset tracking state."""
    self.step_idx = None
    self.segment_idx = 0
    self.deviation_start_time = None

  def set_step(self, idx: int):
    """Set current step index."""
    self.step_idx = idx
    self.segment_idx = 0
    self.deviation_start_time = None

  def get_lookahead(self, speed: float) -> int:
    """Get lookahead distance based on speed (OsmAnd: RoutingHelper.java:562)."""
    return LOOKAHEAD_FAST if speed > FAST_SPEED_THRESHOLD else LOOKAHEAD_SLOW

  def snap_to_route(
    self,
    route: list['Step'],
    position: Coordinate,
    bearing: Optional[float] = None,
    speed: float = 0.0,
  ) -> tuple[int, int, float, float, float]:
    """Snap position to route geometry.

    Returns:
      (step_idx, segment_idx, t, distance_to_route, distance_along_step) where:
      - step_idx: which step we're on
      - segment_idx: which segment within the step
      - t: position along segment (0-1)
      - distance_to_route: perpendicular distance to route
      - distance_along_step: how far along the current step (meters)
    """
    if not route or self.step_idx is None:
      return self.step_idx or 0, 0, 0.0, float('inf'), 0.0

    lookahead = self.get_lookahead(speed)
    best_dist = float('inf')
    best_step = self.step_idx
    best_seg = 0
    best_t = 0.0

    # Search current step and lookahead steps
    end_idx = min(self.step_idx + lookahead, len(route))
    for step_idx in range(self.step_idx, end_idx):
      step = route[step_idx]

      for seg_idx in range(len(step.geometry) - 1):
        a = step.geometry[seg_idx]
        b = step.geometry[seg_idx + 1]
        seg_len = a.distance_to(b)
        if seg_len < 1.0:
          continue

        _, t, dist = project_onto_segment(a, b, position)

        # If we have bearing, validate direction for non-current steps
        if bearing is not None and step_idx > self.step_idx:
          route_bearing = a.bearing_to(b)
          bearing_diff = abs(normalize_angle(bearing - route_bearing))
          if bearing_diff > WRONG_DIRECTION_THRESHOLD:
            continue  # Skip segments going wrong direction

        if dist < best_dist:
          best_dist = dist
          best_step = step_idx
          best_seg = seg_idx
          best_t = t

    # Calculate distance along the best step
    dist_along = 0.0
    if best_step < len(route):
      step = route[best_step]
      for i in range(best_seg):
        if i < len(step.geometry) - 1:
          dist_along += step.geometry[i].distance_to(step.geometry[i + 1])
      # Add partial distance in current segment
      if best_seg < len(step.geometry) - 1:
        seg_len = step.geometry[best_seg].distance_to(step.geometry[best_seg + 1])
        dist_along += seg_len * best_t

    return best_step, best_seg, best_t, best_dist, dist_along

  def find_closest_step(
    self,
    route: list['Step'],
    position: Coordinate,
    bearing: Optional[float],
    speed: float = 0.0,
  ) -> int:
    """Find closest step among current and lookahead steps (OsmAnd-style).

    Uses orthogonal distance to segments with bearing validation.
    Returns the step index with minimum distance that matches travel direction.
    """
    if not route or position is None or self.step_idx is None:
      return self.step_idx or 0

    lookahead = self.get_lookahead(speed)

    if bearing is None:
      return self._find_closest_by_distance(route, position, lookahead)

    min_dist = float('inf')
    closest_idx = self.step_idx

    end_idx = min(self.step_idx + lookahead, len(route))
    for step_idx in range(self.step_idx, end_idx):
      step = route[step_idx]

      for i in range(len(step.geometry) - 1):
        a = step.geometry[i]
        b = step.geometry[i + 1]
        seg_len = a.distance_to(b)
        if seg_len < 1.0:
          continue

        d = minimum_distance(a, b, position)
        if d < min_dist:
          # Check bearing match before accepting
          route_bearing = a.bearing_to(b)
          bearing_diff = abs(normalize_angle(bearing - route_bearing))

          # Only accept if bearing within 90 degrees (not going backwards)
          if bearing_diff < WRONG_DIRECTION_THRESHOLD or step_idx == self.step_idx:
            min_dist = d
            closest_idx = step_idx

    return closest_idx

  def _find_closest_by_distance(
    self,
    route: list['Step'],
    position: Coordinate,
    lookahead: int,
  ) -> int:
    """Fallback: find closest step by distance only (no bearing check)."""
    min_dist = float('inf')
    closest_idx = self.step_idx

    end_idx = min(self.step_idx + lookahead, len(route))
    for step_idx in range(self.step_idx, end_idx):
      step = route[step_idx]
      for i in range(len(step.geometry) - 1):
        a = step.geometry[i]
        b = step.geometry[i + 1]
        if a.distance_to(b) < 1.0:
          continue
        d = minimum_distance(a, b, position)
        if d < min_dist:
          min_dist = d
          closest_idx = step_idx

    return closest_idx

  def should_reroute(
    self,
    route: list['Step'],
    position: Coordinate,
    gps_accuracy: float = 0.0,
  ) -> bool:
    """Check if route should be recomputed (OsmAnd-style time-based debounce).

    Returns True if vehicle has been off-route for REROUTE_DEBOUNCE_TIME seconds.
    Uses GPS accuracy-aware tolerance for distance threshold.
    """
    if self.step_idx is None or not route:
      return True

    # Don't reroute in last segment
    if self.step_idx == len(route) - 1:
      return False

    # GPS accuracy-aware tolerance (OsmAnd: AnnounceTimeDistances.java:25)
    tolerance = POSITIONING_TOLERANCE / 2 + gps_accuracy
    reroute_threshold = max(REROUTE_DISTANCE_BASE, tolerance)

    # Find minimum distance to current step geometry
    min_d = reroute_threshold + 1
    step = route[self.step_idx]

    for i in range(len(step.geometry) - 1):
      a = step.geometry[i]
      b = step.geometry[i + 1]
      if a.distance_to(b) < 1.0:
        continue
      min_d = min(min_d, minimum_distance(a, b, position))

    now = time.monotonic()

    # Time-based debounce (OsmAnd: 10 seconds of sustained deviation)
    if min_d > reroute_threshold:
      if self.deviation_start_time is None:
        self.deviation_start_time = now
        cloudlog.info(f"maad: deviation detected, dist={min_d:.0f}m > threshold={reroute_threshold:.0f}m")
      elif now - self.deviation_start_time > REROUTE_DEBOUNCE_TIME:
        cloudlog.warning(f"maad: rerouting after {REROUTE_DEBOUNCE_TIME}s deviation")
        return True
    else:
      # Back on route - reset timer
      if self.deviation_start_time is not None:
        cloudlog.info("maad: back on route, resetting deviation timer")
      self.deviation_start_time = None

    return False

  def _get_geometry_tail(self, geometry: list[Coordinate], max_dist: float) -> list[Coordinate]:
    """Get the last max_dist meters of geometry (before maneuver point)."""
    if len(geometry) < 2:
      return geometry

    # Walk backwards from end
    result = [geometry[-1]]
    dist = 0.0
    for i in range(len(geometry) - 2, -1, -1):
      seg_dist = geometry[i].distance_to(geometry[i + 1])
      if dist + seg_dist > max_dist:
        break
      result.insert(0, geometry[i])
      dist += seg_dist

    return result

  def _get_geometry_head(self, geometry: list[Coordinate], max_dist: float) -> list[Coordinate]:
    """Get the first max_dist meters of geometry (after maneuver point)."""
    if len(geometry) < 2:
      return geometry

    # Walk forwards from start
    result = [geometry[0]]
    dist = 0.0
    for i in range(1, len(geometry)):
      seg_dist = geometry[i - 1].distance_to(geometry[i])
      if dist + seg_dist > max_dist:
        break
      result.append(geometry[i])
      dist += seg_dist

    return result

  def _snap_to_geometry(self, geometry: list[Coordinate], position: Coordinate) -> float:
    """Find minimum distance from position to geometry segments."""
    min_dist = float('inf')
    for i in range(len(geometry) - 1):
      a, b = geometry[i], geometry[i + 1]
      if a.distance_to(b) < 1.0:
        continue
      _, _, dist = project_onto_segment(a, b, position)
      min_dist = min(min_dist, dist)
    return min_dist

  def _check_passed_maneuver_point(
    self,
    maneuver_pt: Coordinate,
    position: Coordinate,
    bearing: Optional[float],
  ) -> bool:
    """Check if vehicle has passed the maneuver point using bearing.

    A vehicle has "passed" a point when that point is behind it (>90° off heading).

    Before fork:     Vehicle → → → [Fork Point]
                     bearing_to_fork ≈ 0° (ahead)

    After fork:      [Fork Point]     Vehicle → → →
                     bearing_to_fork ≈ 180° (behind)
    """
    dist = position.distance_to(maneuver_pt)

    if bearing is not None:
      # Calculate bearing FROM vehicle TO maneuver point
      bearing_to_pt = position.bearing_to(maneuver_pt)
      angle_diff = abs(normalize_angle(bearing - bearing_to_pt))

      # Debug logging
      if dist < 500:
        cloudlog.info(f"maad: maneuver check step={self.step_idx} dist={dist:.1f}m "
                      f"bearing={bearing:.1f} to_pt={bearing_to_pt:.1f} angle_diff={angle_diff:.1f}")

      # If maneuver point is behind us (>90° off heading), we've passed it
      # This works regardless of distance - if it's behind us, we passed it
      if angle_diff > MANEUVER_BEHIND_ANGLE:
        cloudlog.info(f"maad: passed maneuver point, advancing step {self.step_idx} -> {self.step_idx + 1} "
                      f"(dist={dist:.1f}m, angle_diff={angle_diff:.1f}°)")
        self.step_idx += 1
        self.segment_idx = 0
        self.deviation_start_time = None
        return True
    else:
      # No bearing - use distance only (very close = passed)
      if dist < 15.0:
        cloudlog.info(f"maad: passed maneuver point (no bearing), advancing step {self.step_idx} -> {self.step_idx + 1} "
                      f"(dist={dist:.1f}m)")
        self.step_idx += 1
        self.segment_idx = 0
        self.deviation_start_time = None
        return True

    return False

  def update_step(
    self,
    route: list['Step'],
    position: Coordinate,
    bearing: Optional[float],
    speed: float = 0.0,
  ) -> bool:
    """Update current step by detecting when maneuver point is passed.

    Uses explicit maneuver point with bearing-based detection if available,
    otherwise falls back to geometry comparison.
    """
    if not route or self.step_idx is None:
      return False

    # Need a next step to transition to
    if self.step_idx + 1 >= len(route):
      return False

    current_step = route[self.step_idx]
    next_step = route[self.step_idx + 1]

    # TODO: Re-enable maneuver point detection after fixing display issues
    # # Use explicit maneuver point if available (preferred - works at forks)
    # # Note: OSRM's maneuver.location is at the START of each step, so we check
    # # the NEXT step's maneuver_point (where the turn/fork happens)
    # if next_step.maneuver_point is not None:
    #   return self._check_passed_maneuver_point(next_step.maneuver_point, position, bearing)
    # else:
    #   cloudlog.warning(f"maad: step {self.step_idx + 1} has no maneuver_point, using geometry fallback")

    # Geometry-based comparison (standard OSRM/OsmAnd approach)
    if not current_step.geometry or not next_step.geometry:
      return False

    # Get geometry around the maneuver point
    # Before: last 100m of current step
    before_geom = self._get_geometry_tail(current_step.geometry, MANEUVER_SNAP_ZONE)
    # After: first 100m of next step
    after_geom = self._get_geometry_head(next_step.geometry, MANEUVER_SNAP_ZONE)

    # Snap to both geometries
    dist_before = self._snap_to_geometry(before_geom, position)
    dist_after = self._snap_to_geometry(after_geom, position)

    # Debug logging
    cloudlog.debug(f"maad: step transition check step={self.step_idx} "
                   f"dist_before={dist_before:.1f}m dist_after={dist_after:.1f}m")

    # If closer to "after" geometry, we've passed the maneuver
    if dist_after < dist_before:
      # Additional bearing check if available
      if bearing is not None and len(after_geom) >= 2:
        route_bearing = after_geom[0].bearing_to(after_geom[1])
        bearing_diff = abs(normalize_angle(bearing - route_bearing))
        if bearing_diff > WRONG_DIRECTION_THRESHOLD:
          # Going wrong direction on next step - don't transition
          return False

      cloudlog.info(f"maad: maneuver passed {self.step_idx} -> {self.step_idx + 1} "
                    f"(dist_before={dist_before:.1f}m, dist_after={dist_after:.1f}m)")
      self.step_idx += 1
      self.segment_idx = 0
      self.deviation_start_time = None
      return True

    return False
