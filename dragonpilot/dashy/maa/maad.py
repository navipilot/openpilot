#!/usr/bin/env python3
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

dragonpilot Map-Aware Assist Daemon (maad)

Handles route calculation and navigation instructions using OSRM routing.
Similar to openpilot's navd but uses free OSRM API instead of Mapbox.

Control signals (maaControl) are published by maa_controld.py separately
for low-latency, deterministic control.

Flow:
1. Reads dp_maa_destination from params (set by dashy)
2. Fetches route from OSRM (free routing API) - async to avoid blocking
3. Subscribes to liveGPS for position updates
4. Publishes navInstruction (for UI) and navRoute (for map display)
"""
import json
import time
import threading
import queue
from dataclasses import dataclass
from typing import Optional

import cereal.messaging as messaging
from cereal import custom, log
from openpilot.common.params import Params
from openpilot.common.realtime import Ratekeeper
from openpilot.common.swaglog import cloudlog

from dragonpilot.dashy.maa.helpers import (
  Coordinate,
  coordinate_from_param,
  find_closest_point_on_route,
  distance_along_geometry,
  compute_turn_angle_at_index,
  compute_turn_curvature_at_index,
)
from dragonpilot.dashy.maa.providers import MapService
from dragonpilot.dashy.maa.providers.models import Coordinate as ProviderCoordinate
from dragonpilot.dashy.maa.route_tracker import RouteTracker, REROUTE_DISTANCE_BASE, REROUTE_DEBOUNCE_TIME

MANEUVER_TRANSITION_THRESHOLD = 10  # meters past maneuver to transition
NAV_RATE = 1.0  # Hz - navigation update rate


@dataclass
class Step:
  """Represents a navigation step/segment."""
  distance: float  # total distance of step in meters
  duration: float  # duration in seconds
  duration_typical: Optional[float]
  name: str
  maneuver_type: str
  maneuver_modifier: str
  geometry: list[Coordinate]  # coordinates for this step
  speed_limit: Optional[float]  # m/s
  speed_limit_sign: str  # 'mutcd' or 'vienna'
  # Pre-computed turn geometry (computed once when route is fetched)
  turn_angle: float = 0.0  # degrees, positive=left, negative=right
  turn_curvature: float = 0.0  # 1/m, positive=left, negative=right
  # Explicit maneuver point from OSRM (for fork/turn detection)
  maneuver_point: Optional[Coordinate] = None


class RouteEngine:
  def __init__(self, sm: messaging.SubMaster, pm: messaging.PubMaster):
    self.sm = sm
    self.pm = pm
    self.params = Params()
    self.map_service = MapService(self.params)

    # Get last GPS position from params
    self.last_position = coordinate_from_param("LastGPSPosition", self.params)
    if self.last_position is None:
      # Default to Taipei 101 for bench testing
      self.last_position = Coordinate(25.033976, 121.564472)
    self.last_bearing: Optional[float] = None

    self.gps_ok = False
    self.gps_speed = 0.0
    self.gps_accuracy = 0.0  # horizontal accuracy in meters
    self.last_gps_time = 0.0

    # Route state
    self.nav_destination: Optional[Coordinate] = None
    self.route: Optional[list[Step]] = None
    self.tracker = RouteTracker()  # OsmAnd-style route tracking

    # Recompute state
    self.recompute_backoff = 0
    self.recompute_countdown = 0

    # Async route calculation
    self._route_queue: queue.Queue = queue.Queue()
    self._route_thread: Optional[threading.Thread] = None
    self._route_calculating = False

    # Timing diagnostics
    self._frame_count = 0
    self._last_timing_log = time.monotonic()
    self._gps_save_counter = 0
    self._route_send_counter = 0

    # Params cache - avoid reading params every frame
    self._cached_destination: Optional[Coordinate] = None
    self._last_destination_check = 0.0
    self._destination_check_interval = 3.0  # seconds

    # First valid GPS flag - triggers immediate route calculation
    self._had_first_valid_gps = False

  def update(self):
    t0 = time.monotonic()

    self.sm.update(0)
    t1 = time.monotonic()

    self._update_location()
    self._check_route_result()  # Non-blocking check for async route result
    t2 = time.monotonic()

    try:
      self._recompute_route()
      t3 = time.monotonic()
      self._send_instruction()
      # Resend route periodically (every 1s) so new UI clients can see it
      if self.route is not None:
        self.send_route()
      t4 = time.monotonic()
    except Exception:
      cloudlog.exception("maad.failed_to_compute")
      t3 = t4 = time.monotonic()

    # Log timing every 10 seconds
    self._frame_count += 1
    total_time = t4 - t0
    if total_time > 0.05:  # Log if frame took > 50ms
      cloudlog.warning(f"maad slow frame: total={total_time*1000:.0f}ms "
                       f"(sm={1000*(t1-t0):.0f}, loc={1000*(t2-t1):.0f}, "
                       f"route={1000*(t3-t2):.0f}, instr={1000*(t4-t3):.0f})")

    if t4 - self._last_timing_log > 10.0:
      actual_rate = self._frame_count / (t4 - self._last_timing_log)
      cloudlog.info(f"maad rate: {actual_rate:.1f} Hz (target {NAV_RATE} Hz), frames={self._frame_count}")
      self._frame_count = 0
      self._last_timing_log = t4

  def _check_route_result(self):
    """Check for async route calculation result (non-blocking)."""
    try:
      result = self._route_queue.get_nowait()
      self._route_calculating = False

      if result is None:
        # Route calculation failed
        self._clear_route(clear_destination=False)
        return

      # Apply the calculated route
      self.route, route_data = result
      self.nav_curvature_valid = True

      # Start at first step (simple, like navd.py)
      self.tracker.set_step(0)
      self._reset_recompute_limits()  # Reset backoff on successful route
      cloudlog.warning(f"maad: route calculated - {route_data['distance']/1000:.1f}km, "
                       f"{route_data['duration']/60:.0f}min, {len(self.route)} steps")

      # Debug: log each step's maneuver info
      for i, s in enumerate(self.route):
        mp_str = f"({s.maneuver_point.latitude:.6f},{s.maneuver_point.longitude:.6f})" if s.maneuver_point else "None"
        cloudlog.info(f"maad step {i}: {s.maneuver_type} {s.maneuver_modifier} -> '{s.name}' ({s.distance:.0f}m) mp={mp_str}")

      self.send_route()

    except queue.Empty:
      pass  # No result yet

  def _update_location(self):
    """Update position from GPS."""
    # Debug: log GPS reception status every 50 frames
    self._gps_debug_counter = getattr(self, '_gps_debug_counter', 0) + 1
    if self._gps_debug_counter >= 50:
      self._gps_debug_counter = 0
      gps = self.sm['liveGPS']
      cloudlog.info(f"maad GPS: updated={self.sm.updated['liveGPS']} valid={self.sm.valid['liveGPS']} "
                    f"gpsOK={gps.gpsOK} pos=({gps.latitude:.6f},{gps.longitude:.6f})")

    if self.sm.updated['liveGPS']:
      gps = self.sm['liveGPS']

      # Always update position and speed from GPS (needed for route calculation)
      if gps.gpsOK:
        self.last_position = Coordinate(gps.latitude, gps.longitude)
        self.gps_speed = gps.speed
        self.gps_accuracy = gps.horizontalAccuracy  # Track for OsmAnd-style tolerance
        self.last_gps_time = time.monotonic()

        # Save last position every ~60 frames (12 seconds at 5Hz) to reduce I/O
        self._gps_save_counter += 1
        if self._gps_save_counter >= 60:
          self._gps_save_counter = 0
          self.params.put("LastGPSPosition", json.dumps({
            'latitude': gps.latitude,
            'longitude': gps.longitude
          }))

      # GPS is valid when OK, good accuracy, AND fully calibrated
      was_gps_ok = self.gps_ok
      is_calibrated = gps.status == custom.LiveGPS.Status.valid
      self.gps_ok = gps.gpsOK and gps.horizontalAccuracy <= 20.0 and is_calibrated

      # Only use bearing when fusion is fully calibrated
      if is_calibrated:
        self.last_bearing = gps.bearingDeg
      else:
        self.last_bearing = None  # clear stale bearing

      # Detect first valid GPS - triggers immediate route check
      if self.gps_ok and not was_gps_ok and not self._had_first_valid_gps:
        self._had_first_valid_gps = True
        cloudlog.info("maad: first valid GPS fix (calibrated), checking for destination")
    
    # Staleness check
    if time.monotonic() - self.last_gps_time > 2.0:
      self.gps_ok = False

  def _recompute_route(self):
    """Check if we need to recompute route."""
    # Don't start route until GPS is valid (OK + calibrated)
    if not self.gps_ok:
      return

    # Skip if route calculation is already in progress
    if self._route_calculating:
      return

    # Check params - immediately on first valid GPS, otherwise every 3 seconds
    now = time.monotonic()
    first_gps_check = self._had_first_valid_gps and self.route is None and self._cached_destination is None
    if first_gps_check or now - self._last_destination_check >= self._destination_check_interval:
      self._last_destination_check = now
      self._cached_destination = coordinate_from_param("dp_maa_destination", self.params)

    new_destination = self._cached_destination
    if new_destination is None:
      if self.nav_destination is not None or self.route is not None:
        self._clear_route()
      self._reset_recompute_limits()
      return

    should_recompute = self._should_recompute()
    if should_recompute and self.route is not None:
      cloudlog.warning(f"maad: reroute triggered, countdown={self.recompute_countdown}, backoff={self.recompute_backoff}")

    # New destination
    if new_destination != self.nav_destination:
      cloudlog.warning(f"Got new destination from dp_maa_destination param {new_destination}")
      self.nav_destination = new_destination
      should_recompute = True

    # Don't recompute when GPS drifts in tunnels
    if not self.gps_ok and self.tracker.step_idx is not None:
      return

    # First route calculation (no existing route) - start immediately without backoff
    is_first_route = self.route is None and should_recompute
    if is_first_route or (self.recompute_countdown == 0 and should_recompute):
      if not is_first_route:
        self.recompute_countdown = 2 ** self.recompute_backoff
        self.recompute_backoff = min(3, self.recompute_backoff + 1)  # Max 8 second backoff
      self._start_route_calculation(new_destination)
    else:
      self.recompute_countdown = max(0, self.recompute_countdown - 1)

  def _start_route_calculation(self, destination: Coordinate):
    """Start async route calculation in a separate thread."""
    start_pos = self.last_position
    bearing = self.last_bearing

    self._route_calculating = True
    self.nav_destination = destination

    cloudlog.info(f"maad: starting async route calculation {start_pos} -> {destination}")

    def calculate():
      try:
        result = self._fetch_route(start_pos, destination, bearing)
        self._route_queue.put(result)
      except Exception as e:
        cloudlog.exception(f"maad: route calculation failed: {e}")
        self._route_queue.put(None)

    self._route_thread = threading.Thread(target=calculate, daemon=True)
    self._route_thread.start()

  def _fetch_route(self, start: Coordinate, destination: Coordinate,
                   bearing: Optional[float]) -> Optional[tuple]:
    """Fetch route using MapService (runs in thread). Returns (route, route_data) or None."""
    origin = ProviderCoordinate(start.latitude, start.longitude)
    dest = ProviderCoordinate(destination.latitude, destination.longitude)

    provider_route = self.map_service.route_provider.get_route_sync(
      origin=origin,
      destination=dest,
      bearing=bearing
    )

    if provider_route is None:
      cloudlog.warning("maad: route provider returned None")
      return None

    # Convert provider Route to local Step format
    # Filter out depart/arrive - merge their geometry with adjacent steps
    route = []
    all_coords = []  # Full route geometry for turn angle computation
    pending_geometry = []  # Geometry from depart to merge with first real step

    for provider_step in provider_route.steps:
      # Convert provider Coordinates to helper Coordinates
      geometry = [
        Coordinate(c.latitude, c.longitude)
        for c in provider_step.geometry
      ]
      all_coords.extend(geometry)

      # Skip depart/arrive steps but keep their geometry
      if provider_step.maneuver_type in ('depart', 'arrive'):
        if provider_step.maneuver_type == 'depart':
          pending_geometry = geometry  # Save for merging with first real step
        elif provider_step.maneuver_type == 'arrive' and route:
          # Merge arrive geometry with last step
          route[-1].geometry.extend(geometry)
          route[-1].distance += provider_step.distance
          route[-1].duration += provider_step.duration
        continue

      # Merge pending depart geometry with this step
      if pending_geometry:
        geometry = pending_geometry + geometry
        pending_geometry = []

      # Convert provider maneuver_point to helpers Coordinate
      maneuver_pt = None
      if provider_step.maneuver_point:
        maneuver_pt = Coordinate(
          provider_step.maneuver_point.latitude,
          provider_step.maneuver_point.longitude
        )

      route_step = Step(
        distance=provider_step.distance,
        duration=provider_step.duration,
        duration_typical=provider_step.duration_typical or provider_step.duration,
        name=provider_step.name,
        maneuver_type=provider_step.maneuver_type,
        maneuver_modifier=provider_step.maneuver_modifier,
        geometry=geometry,
        speed_limit=provider_step.speed_limit,
        speed_limit_sign=provider_step.speed_limit_sign,
        maneuver_point=maneuver_pt,
      )
      route.append(route_step)

    # Pre-compute turn geometry at each step's maneuver point (end of step)
    coord_idx = 0
    for step in route:
      coord_idx += len(step.geometry)
      # Turn point is at the end of this step (start of next)
      turn_idx = min(coord_idx - 1, len(all_coords) - 2)
      if turn_idx > 0:
        step.turn_angle = compute_turn_angle_at_index(all_coords, turn_idx)
        step.turn_curvature = compute_turn_curvature_at_index(all_coords, turn_idx)

    # Build route_data dict for compatibility
    route_data = {
      'distance': provider_route.distance,
      'duration': provider_route.duration,
    }

    cloudlog.info(f"maad: route from {provider_route.provider} - "
                  f"{provider_route.distance/1000:.1f}km, {len(route)} steps")

    return (route, route_data)

  def _send_instruction(self):
    """Send navInstruction message ."""
    msg = messaging.new_message('navInstruction', valid=True)

    if self.tracker.step_idx is None or self.route is None or self.last_position is None or not self.gps_ok:
      # Debug: log why we're sending invalid
      reasons = []
      if self.tracker.step_idx is None:
        reasons.append("step_idx=None")
      if self.route is None:
        reasons.append("route=None")
      if self.last_position is None:
        reasons.append("position=None")
      if not self.gps_ok:
        reasons.append(f"gps_ok=False")
      cloudlog.info(f"maad: sending invalid navInstruction: {', '.join(reasons)}")
      msg.valid = False
      self.pm.send('navInstruction', msg)
      return

    # Sanity check: ensure step_idx is valid
    if self.tracker.step_idx >= len(self.route):
      cloudlog.error(f"maad: step_idx {self.tracker.step_idx} >= route length {len(self.route)}, resetting to 0")
      self.tracker.set_step(0)

    step = self.route[self.tracker.step_idx]
    geometry = step.geometry

    # Calculate distance along current step geometry
    along_geometry = distance_along_geometry(geometry, self.last_position)
    distance_to_maneuver = step.distance - along_geometry

    # Current instruction (depart/arrive already filtered out during route build)
    msg.navInstruction.maneuverDistance = distance_to_maneuver
    msg.navInstruction.maneuverPrimaryText = step.name or step.maneuver_type

    # Override maneuver type/modifier based on geometry
    # Geometry-first: always use turn_angle for direction when significant
    TURN_MIN_ANGLE = 20.0
    if abs(step.turn_angle) >= TURN_MIN_ANGLE:
      # Significant turn - use geometry for both type and direction
      if step.maneuver_type in ('continue', 'new name'):
        msg.navInstruction.maneuverType = 'turn'
      else:
        msg.navInstruction.maneuverType = step.maneuver_type
      # Always use geometry-based direction for significant turns
      msg.navInstruction.maneuverModifier = 'left' if step.turn_angle > 0 else 'right'
    else:
      msg.navInstruction.maneuverType = step.maneuver_type
      msg.navInstruction.maneuverModifier = step.maneuver_modifier

    # Next step's road name (the road to turn onto)
    if self.tracker.step_idx + 1 < len(self.route):
      next_step = self.route[self.tracker.step_idx + 1]
      msg.navInstruction.maneuverSecondaryText = next_step.name or ""

    # Compute total remaining time and distance
    remaining_ratio = 1.0 - along_geometry / max(step.distance, 1)
    total_distance = step.distance * remaining_ratio
    total_time = step.duration * remaining_ratio
    total_time_typical = (step.duration_typical or step.duration) * remaining_ratio

    for i in range(self.tracker.step_idx + 1, len(self.route)):
      total_distance += self.route[i].distance
      total_time += self.route[i].duration
      total_time_typical += self.route[i].duration_typical or self.route[i].duration

    msg.navInstruction.distanceRemaining = total_distance
    msg.navInstruction.timeRemaining = total_time
    msg.navInstruction.timeRemainingTypical = total_time_typical

    # Speed limit from closest coordinate
    if geometry:
      closest_idx, _ = find_closest_point_on_route(self.last_position, geometry)
      if closest_idx < len(geometry):
        closest = geometry[closest_idx]
        if 'maxspeed' in closest.annotations and self.gps_ok:
          msg.navInstruction.speedLimit = closest.annotations['maxspeed']

    if step.speed_limit_sign == 'mutcd':
      msg.navInstruction.speedLimitSign = log.NavInstruction.SpeedLimitSign.mutcd
    else:
      msg.navInstruction.speedLimitSign = log.NavInstruction.SpeedLimitSign.vienna

    self.pm.send('navInstruction', msg)

    # Send extended nav instruction (turn geometry)
    msg_ext = messaging.new_message('navInstructionExt')
    msg_ext.navInstructionExt.turnAngle = step.turn_angle
    msg_ext.navInstructionExt.turnCurvature = step.turn_curvature
    self.pm.send('navInstructionExt', msg_ext)

    # Transition to next step
    if self.tracker.update_step(self.route, self.last_position, self.last_bearing, self.gps_speed):
      self._reset_recompute_limits()

    # Check if arrived at destination
    if self.nav_destination:
      dist = self.nav_destination.distance_to(self.last_position)
      if dist < 30:  # Within 30m of destination
        cloudlog.warning("maad: destination reached")
        self.params.remove("dp_maa_destination")
        self._clear_route()

  def send_route(self):
    """Send navRoute message for dashy to display route on map."""
    coords = []

    if self.route is not None:
      for step in self.route:
        coords.extend([[c.longitude, c.latitude] for c in step.geometry])

    msg = messaging.new_message('navRoute', valid=True)
    msg.navRoute.coordinates = [{"longitude": c[0], "latitude": c[1]} for c in coords]
    self.pm.send('navRoute', msg)

  def _clear_route(self, clear_destination=True):
    """Clear navigation state."""
    self.route = None
    self.tracker.reset()
    if clear_destination:
      self.nav_destination = None

    # Send empty navRoute to clear map display
    msg = messaging.new_message('navRoute', valid=False)
    msg.navRoute.coordinates = []
    self.pm.send('navRoute', msg)

  def _reset_recompute_limits(self):
    """Reset recompute backoff and deviation timer."""
    self.recompute_backoff = 0
    self.recompute_countdown = 0
    self.tracker.deviation_start_time = None  # Reset OsmAnd-style deviation timer

  def _should_recompute(self) -> bool:
    """Check if route should be recomputed (delegates to RouteTracker)."""
    if self.route is None:
      return True
    return self.tracker.should_reroute(self.route, self.last_position, self.gps_accuracy)


def main():
  cloudlog.info("maad starting")

  pm = messaging.PubMaster(['navInstruction', 'navInstructionExt', 'navRoute'])
  sm = messaging.SubMaster(['liveGPS'], ignore_alive=['liveGPS'])

  rk = Ratekeeper(NAV_RATE)
  route_engine = RouteEngine(sm, pm)

  while True:
    try:
      route_engine.update()
    except Exception:
      cloudlog.exception("maad: error in main loop")
    rk.keep_time()


if __name__ == "__main__":
  main()
