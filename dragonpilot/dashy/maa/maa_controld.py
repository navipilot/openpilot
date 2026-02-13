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

MAA Control Daemon - Turn Assist with Dead Reckoning

================================================================================
OVERVIEW
================================================================================

This daemon provides high-frequency (20Hz) turn assist signals for navigation.
The key insight is that navInstruction updates at only 1Hz (too slow for smooth
turn tracking), so we use "snapshot and coast" - capture turn info once, then
dead reckon using the car's own sensors.

Subscribes to:
- navInstruction: turn info from maad (1Hz) - used as TRIGGER only
- liveGPS: position, bearing (1Hz)
- navRoute: route geometry - reset on route change
- carState: vEgo, blinker, yawRate (100Hz) - for dead reckoning

Publishes:
- maaControl: turnDistance, turnDirection, desireActive, turnState, etc.

================================================================================
DEAD RECKONING APPROACH
================================================================================

Problem:
  Navigation updates come in slow (1Hz). At 60 km/h, that's 17m between updates.
  This causes jerky distance countdown and imprecise turn detection.

Solution: "Snapshot and Coast"
  1. TRIGGER at ~200m: capture turn params (angle, distance, direction)
  2. IGNORE subsequent navInstruction updates for this turn
  3. DEAD RECKON distance: integrate vEgo from carState (100Hz)
  4. TRACK HEADING: integrate yawRate from carState during turn execution

  Distance remaining = initial_distance - ∫(vEgo × dt)
  Heading change     = ∫(yawRate × dt)

================================================================================
STATE MACHINE
================================================================================

  NONE ──────────────────────────────────────────────────────────────┐
    │                                                                │
    │ navInstruction shows turn within 200m                          │
    │ (significant angle ≥20°, has direction)                        │
    ▼                                                                │
  APPROACHING ───────────────────────────────────────────────────────┤
    │  • Show turn suggestion at <150m                               │
    │  • Wait for driver blinker (acknowledgment)                    │
    │  • Dead reckon distance using vEgo                             │
    │  • NOT tracking yaw yet (allows overtaking)                    │
    │                                                                │
    │  At <100m WITH blinker: COMMIT                                 │
    │    • blockLaneChange = true                                    │
    │    • speedLimitActive = true                                   │
    │    • desireActive = true                                       │
    │                                                                │
    │ Dead reckoned distance < 30m                                   │
    ▼                                                                │
  EXECUTING ─────────────────────────────────────────────────────────┤
    │  • Now tracking yaw (accumulated heading change)               │
    │  • Compare accumulated vs expected turn angle                  │
    │  • desireActive = true (only if acknowledged)                  │
    │                                                                │
    │ |accumulated_yaw| >= |expected_angle| - tolerance              │
    ▼                                                                │
  COMPLETE ──────► (2s cooldown) ──────► NONE                        │
                                                                     │
  MISSED ◄─── (any abort condition) ◄────────────────────────────────┘
    │
    │ Wait for route change (navRoute coords change)
    ▼
  NONE

================================================================================
DRIVER ACKNOWLEDGMENT (Two-Step, Like Lane Change)
================================================================================

Two confirmation steps:
1. Blinker matching turn direction = approach confirmed (speed limit, block lane change)
2. Steering torque in turn direction = turn execution confirmed (desire sent)

Blinker is the main confirmation for slowing down. Steering is required to
actually send the turn desire to the model (like lane change).

Distance   | Without Blinker          | With Blinker               | + Steering
-----------|--------------------------|----------------------------|------------------
200m-150m  | Informational only       | Informational only         | (same)
150m-100m  | Show suggestion          | speedLimitActive = true    | (same)
<100m      | Show suggestion          | slow down, block LC        | + desireActive
<30m       | Enter EXECUTING          | slow down, block LC        | + desireActive

Key behaviors:
- speedLimitActive: When blinker on, enforce turn speed limit
- blockLaneChange: At <100m with blinker, block lane change desire
- desireActive: Only sent when blinker AND steering confirmed (at turn)

If driver has blinker but doesn't steer:
- System slows down for the turn
- But doesn't send turn desire (driver steers manually)
- Once driver steers in turn direction, desire activates

================================================================================
ABORT/MISS DETECTION
================================================================================

Driver can override at any time. We detect "missed turn" via:

1. DROVE TOO FAR (no turn)
   - Condition: distance_traveled > 2× initial_distance
   - Meaning: drove way past expected turn point without turning
   - Example: triggered at 200m, drove 400m, barely any yaw change

2. WRONG DIRECTION
   - Condition: accumulated_yaw > 20° in opposite direction
   - Meaning: driver turned the wrong way
   - Example: expected right turn, driver turned 25° left

3. INSUFFICIENT TURN
   - Condition: drove 2× total distance but <30% of expected yaw
   - Meaning: drove through intersection without completing turn
   - Example: expected 90° turn, only turned 20° after driving 500m

4. TIMEOUT
   - Condition: 30 seconds of MOVING time (v_ego > 1 m/s)
   - Meaning: something went wrong, taking too long
   - Note: stopped time (traffic light) doesn't count

5. PASSED TURN (nav jumped to next)
   - Condition: dead_reckon < 50m but nav says > 150m
   - Meaning: nav is now showing NEXT turn, we passed this one
   - Example: we think 30m to turn, nav says 400m (next turn)

6. DIRECTION CHANGED
   - Condition: turnAngle sign flipped (left ↔ right)
   - Meaning: route recalculated or nav corrected itself
   - Example: was +90° (left), now -45° (right)

7. TURN DISAPPEARED
   - Condition: turnAngle dropped below 20° threshold
   - Meaning: no longer a significant turn (route changed or passed)

================================================================================
DRIVER OVERRIDE SCENARIOS
================================================================================

Overtaking another car:
  - During APPROACHING: No problem! We only track distance, not yaw.
    Driver can swerve left/right to overtake, doesn't affect tracking.
  - During EXECUTING: Brief swerves (<20°) won't trigger wrong direction.
    Only sustained opposite turn triggers abort.

Stopping at traffic light:
  - moving_time only increments when v_ego > 1 m/s
  - Can wait indefinitely at red light without timeout
  - Distance tracking pauses when stopped (vEgo ≈ 0)

Taking a different route intentionally:
  - System correctly detects as MISSED
  - Waits for navRoute to change (reroute)
  - Then ready to track new turn

================================================================================
DATA FLOW
================================================================================

  navInstruction (1Hz)                    carState (100Hz)
        │                                      │
        │ turnAngle, maneuverDistance          │ vEgo, yawRate, blinker
        │ maneuverType, modifier               │
        ▼                                      ▼
  ┌─────────────────────────────────────────────────────────┐
  │                    TurnTracker                          │
  │                                                         │
  │  trigger() ◄── at 200m, capture params                  │
  │      │                                                  │
  │      ▼                                                  │
  │  update(vEgo, yawRate) ◄── every 50ms (20Hz)            │
  │      │                                                  │
  │      ├── distance_traveled += vEgo × dt                 │
  │      ├── accumulated_yaw += yawRate × dt  (if EXECUTING)│
  │      └── check abort conditions                         │
  │                                                         │
  └─────────────────────────────────────────────────────────┘
        │
        ▼
  maaControl (20Hz)
    • turnDistance (dead reckoned)
    • turnState (NONE/APPROACHING/EXECUTING/COMPLETE/MISSED)
    • desireActive (for blinker/lane change desire)
    • turnProgress (accumulated yaw in degrees)

================================================================================
CONFIGURATION
================================================================================

TURN_TRIGGER_DISTANCE = 200m   # Start dead reckoning
TURN_DESIRE_DISTANCE  = 150m   # Show turn suggestion, wait for blinker
TURN_COMMIT_DISTANCE  = 100m   # With blinker: commit (block lane change, slow)
TURN_EXECUTE_DISTANCE = 30m    # Start tracking yaw
TURN_ANGLE_TOLERANCE  = 15°    # Turn complete within this
TURN_MIN_ANGLE        = 20°    # Minimum to be "significant"
TURN_TIMEOUT          = 30s    # Max moving time

================================================================================
OUTPUT FIELDS (maaControl)
================================================================================

turnDistance        - Dead reckoned distance to turn (m)
turnDirection       - left/right/none
turnAngle           - Expected turn angle (deg, + = left)
turnState           - 0=none, 1=approaching, 2=executing, 3=complete, 4=missed
turnProgress        - Accumulated yaw during turn (deg)
desireActive        - Send turn desire to model (blinker + steering confirmed)
driverAcknowledged  - Driver turned on matching blinker
speedLimitActive    - Enforce turn speed limit (blinker on)
blockLaneChange     - Block lane change desire (blinker + committed)
turnSpeedLimit      - Target speed for turn (m/s)

================================================================================
"""

import json
import math
import time
from enum import IntEnum

import numpy as np
from cereal import messaging, log, custom
from openpilot.common.params import Params
from openpilot.common.realtime import Ratekeeper
from openpilot.common.swaglog import cloudlog

from dragonpilot.dashy.maa.helpers import (
  Coordinate,
  compute_path_curvature,
  find_closest_point_on_route,
)


# CarrotPilot curvature-to-speed lookup table
# Adapted from https://github.com/ajouatom/openpilot
# Maps curvature (1/m) to recommended speed (km/h)
# Based on physics: v = sqrt(a_lat / κ) where a_lat ≈ 2.5 m/s² for comfort
V_CURVE_LOOKUP_BP = [0., 1./800., 1./670., 1./560., 1./440., 1./360., 1./265., 1./190., 1./135., 1./85., 1./55., 1./30., 1./25.]
V_CURVE_LOOKUP_VALS = [300., 150., 120., 110., 100., 90., 80., 70., 60., 50., 40., 15., 5.]  # km/h


# Configuration
CURVATURE_ASSIST_ENABLED = False  # Disable continuous curvature steering assist
CURVATURE_LOOKAHEAD = 2.5  # seconds ahead for curvature calculation
TURN_VALID_DISTANCE = 500.0  # meters - turn is valid if within this distance
MIN_SPEED_FOR_CURVATURE = 1.0  # m/s - minimum speed to use curvature

# Turn execution - dead reckoning based
TURN_TRIGGER_DISTANCE = 200.0   # meters - capture turn info and start dead reckoning
TURN_DESIRE_DISTANCE = 150.0    # meters - show turn suggestion, wait for blinker
TURN_COMMIT_DISTANCE = 100.0    # meters - if blinker on, commit (block lane change, slow down)
TURN_EXECUTE_DISTANCE = 30.0    # meters - start tracking heading (entering intersection)
TURN_ANGLE_TOLERANCE = 15.0     # degrees - turn complete when within this of target
TURN_MIN_ANGLE = 20.0           # degrees - minimum angle to consider a "turn"

# Abort detection thresholds
TURN_MISS_DISTANCE_FACTOR = 2.0   # drove 2x expected distance without turning = missed
TURN_MISS_YAW_THRESHOLD = 0.3     # must achieve at least 30% of turn angle
TURN_WRONG_DIRECTION_ANGLE = 20.0 # degrees in wrong direction = missed
TURN_TIMEOUT = 30.0               # seconds - max time in APPROACHING/EXECUTING


class TurnState(IntEnum):
  NONE = 0          # no turn pending
  APPROACHING = 1   # turn ahead, dead reckoning distance
  EXECUTING = 2     # in intersection, tracking heading change
  COMPLETE = 3      # turn done, cooldown before next
  MISSED = 4        # turn missed/aborted, wait for reroute


class TurnTracker:
  """
  Tracks turn execution using dead reckoning.

  Once triggered at ~200m, ignores navInstruction updates and purely
  uses vEgo (distance) and yawRate (heading) from carState.

  Driver acknowledgment flow (two-step, like lane change):
  - System shows turn suggestion at 150m
  - Driver turns on blinker = acknowledged (speed limit, block lane change)
  - Driver steers in turn direction = steering confirmed (desire sent)
  - Blinker gates the approach, steering gates the turn execution
  """

  def __init__(self):
    self.state = TurnState.NONE
    # Captured at trigger
    self.expected_angle = 0.0       # degrees (positive=left, negative=right)
    self.initial_distance = 0.0     # distance to turn when triggered
    self.direction = 'none'         # 'left' or 'right'
    self.maneuver_type = 0          # MaaControl.ManeuverType
    self.turn_speed_limit = 0.0     # m/s
    # Dead reckoning state
    self.distance_traveled = 0.0    # meters since trigger
    self.accumulated_yaw = 0.0      # degrees turned since execute start
    self.moving_time = 0.0          # accumulated time while moving (for timeout)
    self.execute_start_time = 0.0   # when EXECUTING started
    self.complete_time = 0.0        # when turn completed
    self.last_update_time = 0.0     # for dt calculation
    # Driver acknowledgment (two-step like lane change)
    self.driver_acknowledged = False  # blinker matches turn direction
    self.steering_confirmed = False   # steering torque matches turn direction

  def reset(self):
    self.state = TurnState.NONE
    self.expected_angle = 0.0
    self.initial_distance = 0.0
    self.direction = 'none'
    self.maneuver_type = 0
    self.turn_speed_limit = 0.0
    self.distance_traveled = 0.0
    self.accumulated_yaw = 0.0
    self.moving_time = 0.0
    self.execute_start_time = 0.0
    self.complete_time = 0.0
    self.last_update_time = 0.0
    self.driver_acknowledged = False
    self.steering_confirmed = False

  def get_estimated_distance(self) -> float:
    """Get estimated distance to turn based on dead reckoning."""
    if self.state in (TurnState.APPROACHING, TurnState.EXECUTING):
      return max(0.0, self.initial_distance - self.distance_traveled)
    return 9999.0

  def trigger(self, t: float, turn_distance: float, turn_angle: float,
              maneuver_type: int, turn_direction: int, turn_speed_limit: float):
    """Capture turn parameters and start dead reckoning."""
    self.state = TurnState.APPROACHING
    self.expected_angle = turn_angle
    self.initial_distance = turn_distance
    self.maneuver_type = maneuver_type
    self.turn_speed_limit = turn_speed_limit
    self.direction = 'left' if turn_direction == custom.MaaControl.TurnDirection.left else 'right'
    self.distance_traveled = 0.0
    self.accumulated_yaw = 0.0
    self.moving_time = 0.0
    self.last_update_time = t
    cloudlog.info(f"maa: turn triggered, angle={turn_angle:.1f}°, dist={turn_distance:.0f}m, dir={self.direction}")

  def check_blinker(self, left_blinker: bool, right_blinker: bool):
    """Check if driver turned on blinker matching turn direction."""
    if self.direction == 'left' and left_blinker:
      self.driver_acknowledged = True
    elif self.direction == 'right' and right_blinker:
      self.driver_acknowledged = True
    # Note: we don't reset acknowledged if blinker turns off
    # (driver may have tapped blinker briefly to acknowledge)

  def check_steering(self, steering_pressed: bool, steering_torque: float):
    """Check if driver is steering in the turn direction (like lane change confirmation)."""
    if not steering_pressed:
      return
    # Same logic as lane change: positive torque = left, negative = right
    if self.direction == 'left' and steering_torque > 0:
      self.steering_confirmed = True
    elif self.direction == 'right' and steering_torque < 0:
      self.steering_confirmed = True
    # Once confirmed, stays confirmed (one-time check)

  def update(self, t: float, v_ego: float, yaw_rate: float) -> bool:
    """
    Update turn state with dead reckoning.

    Args:
      t: current time (monotonic)
      v_ego: vehicle speed (m/s) from carState
      yaw_rate: yaw rate (rad/s) from carState

    Returns True if desire should be sent (acknowledged + within commit distance).
    """
    if self.state == TurnState.NONE:
      return False

    # Calculate dt
    dt = t - self.last_update_time if self.last_update_time > 0 else 0.05
    dt = min(dt, 0.2)  # Cap dt to handle timing glitches
    self.last_update_time = t

    # Integrate distance traveled (and moving time for timeout)
    self.distance_traveled += v_ego * dt
    if v_ego > 1.0:  # Only count time when actually moving
      self.moving_time += dt
    estimated_dist = self.get_estimated_distance()

    # Timeout check - based on moving time (allows waiting at traffic lights)
    if self.moving_time > TURN_TIMEOUT:
      cloudlog.warning(f"maa: turn timeout after {self.moving_time:.0f}s moving")
      self.state = TurnState.MISSED
      return False

    if self.state == TurnState.APPROACHING:
      # Check if we've reached the intersection
      if estimated_dist <= TURN_EXECUTE_DISTANCE:
        self.state = TurnState.EXECUTING
        self.execute_start_time = t
        self.accumulated_yaw = 0.0
        cloudlog.info(f"maa: turn executing, traveled={self.distance_traveled:.0f}m")

      # Miss detection: drove way past without entering execute
      if self.distance_traveled > self.initial_distance * TURN_MISS_DISTANCE_FACTOR:
        cloudlog.warning(f"maa: turn missed (drove past), traveled={self.distance_traveled:.0f}m")
        self.state = TurnState.MISSED
        return False

    elif self.state == TurnState.EXECUTING:
      # Integrate yaw rate (convert rad/s to deg)
      yaw_change_deg = math.degrees(yaw_rate) * dt
      self.accumulated_yaw += yaw_change_deg

      # Fix 3: Early abort if going straight through the "turn" (map error)
      # If we've traveled 20m past the turn point with no significant yaw change,
      # the turn doesn't exist - abort quickly instead of extended false braking
      distance_past_turn = self.distance_traveled - self.initial_distance
      if distance_past_turn > 20.0 and abs(self.accumulated_yaw) < 5.0:
        cloudlog.warning(f"maa: no turn detected (past {distance_past_turn:.0f}m, yaw={self.accumulated_yaw:.1f}°), aborting")
        self.state = TurnState.MISSED
        return False

      # Check completion: achieved target heading
      # For left turn: expected_angle > 0, accumulated should go positive
      # For right turn: expected_angle < 0, accumulated should go negative
      progress = abs(self.accumulated_yaw) / abs(self.expected_angle) if self.expected_angle != 0 else 0
      angle_remaining = abs(self.expected_angle) - abs(self.accumulated_yaw)

      if angle_remaining <= TURN_ANGLE_TOLERANCE:
        self.state = TurnState.COMPLETE
        self.complete_time = t
        cloudlog.info(f"maa: turn complete, yaw={self.accumulated_yaw:.1f}°, expected={self.expected_angle:.1f}°")
        return False

      # Wrong direction detection: significant yaw in opposite direction
      if self.expected_angle > 0 and self.accumulated_yaw < -TURN_WRONG_DIRECTION_ANGLE:
        cloudlog.warning(f"maa: turn wrong direction (expected left, went right)")
        self.state = TurnState.MISSED
        return False
      if self.expected_angle < 0 and self.accumulated_yaw > TURN_WRONG_DIRECTION_ANGLE:
        cloudlog.warning(f"maa: turn wrong direction (expected right, went left)")
        self.state = TurnState.MISSED
        return False

      # Miss detection: drove 2x expected total distance without sufficient turn
      total_expected = self.initial_distance + 50.0  # Add some buffer for turn itself
      if self.distance_traveled > total_expected * TURN_MISS_DISTANCE_FACTOR:
        if progress < TURN_MISS_YAW_THRESHOLD:
          cloudlog.warning(f"maa: turn missed (insufficient yaw), progress={progress:.1%}")
          self.state = TurnState.MISSED
          return False

    elif self.state == TurnState.COMPLETE:
      # Short cooldown then reset
      if t - self.complete_time > 2.0:
        self.reset()
      return False

    elif self.state == TurnState.MISSED:
      # Stay in missed state until route changes (reset called externally)
      return False

    # Desire active when:
    # Driver acknowledged (blinker on) AND steering confirmed AND either:
    # 1. APPROACHING and within commit distance (<100m), OR
    # 2. EXECUTING (in the turn)
    # Without both confirmations, NO desire is sent (driver maintains control)
    if not self.driver_acknowledged or not self.steering_confirmed:
      return False
    if self.state == TurnState.APPROACHING:
      return estimated_dist <= TURN_COMMIT_DISTANCE
    if self.state == TurnState.EXECUTING:
      return True
    return False


def get_turn_speed_from_curvature(curvature: float) -> float:
  """
  Compute recommended speed for turn using CarrotPilot's curvature lookup table.
  This is physics-based: v = sqrt(a_lat / κ) where a_lat ≈ 2.5 m/s² for comfort.

  Args:
    curvature: Road curvature in 1/m (positive or negative)

  Returns:
    Recommended speed in m/s
  """
  abs_curv = abs(curvature)
  speed_kph = np.interp(abs_curv, V_CURVE_LOOKUP_BP, V_CURVE_LOOKUP_VALS)
  return speed_kph / 3.6  # Convert to m/s


def get_turn_speed_limit(modifier: str, turn_angle: float, nav_type: str = '', curvature: float = 0.0) -> float:
  """
  Compute recommended speed for turn based on curvature (preferred) or heuristics.

  Uses CarrotPilot's curvature lookup table when curvature is available.
  Falls back to heuristic-based calculation when curvature is not available.

  Args:
    modifier: OSRM maneuver modifier (e.g., 'left', 'right', 'sharp left')
    turn_angle: Turn angle in degrees (positive=left, negative=right)
    nav_type: OSRM maneuver type (e.g., 'turn', 'off ramp')
    curvature: Road curvature in 1/m (from geometry calculation)

  Returns:
    Recommended speed in m/s
  """
  # Use curvature-based calculation when curvature is available
  # This is more accurate than heuristics as it's physics-based
  if abs(curvature) > 0.001:  # Meaningful curvature
    speed_from_curv = get_turn_speed_from_curvature(curvature)
    # Clamp to reasonable range: 5 km/h to 100 km/h for turns
    return max(5.0 / 3.6, min(100.0 / 3.6, speed_from_curv))

  # Fallback: Heuristic-based calculation when curvature not available

  # Highway maneuvers (exits, ramps, merges) - much higher speeds
  if nav_type in ('depart', 'off ramp', 'on ramp', 'merge', 'fork'):
    if 'sharp' in modifier:
      base_speed = 50.0  # km/h - sharp highway exit
    elif 'slight' in modifier:
      base_speed = 80.0  # km/h - gentle curve
    else:
      base_speed = 60.0  # km/h - normal exit ramp
    return base_speed / 3.6  # Don't apply angle adjustments to highway maneuvers

  # Intersection turns - lower speeds
  if 'sharp' in modifier:
    base_speed = 15.0  # km/h
  elif 'slight' in modifier:
    base_speed = 45.0  # km/h
  elif modifier in ('left', 'right'):
    base_speed = 25.0  # km/h
  elif 'uturn' in modifier:
    base_speed = 10.0  # km/h
  else:
    base_speed = 50.0  # km/h default

  # Adjust based on actual angle if available (only for intersection turns)
  abs_angle = abs(turn_angle)
  if abs_angle > 90:
    base_speed = min(base_speed, 15.0)
  elif abs_angle > 60:
    base_speed = min(base_speed, 25.0)
  elif abs_angle > 30:
    base_speed = min(base_speed, 35.0)

  return base_speed / 3.6  # Convert to m/s


def get_maneuver_type(nav_type: str, turn_angle: float = 0.0) -> int:
  """Determine maneuver type primarily from geometry, with OSRM hints.

  Geometry is the source of truth - OSRM labels can be wrong.
  """
  # Geometry-first: significant turn angle = it's a turn
  if abs(turn_angle) >= TURN_MIN_ANGLE:
    return custom.MaaControl.ManeuverType.turn

  # Highway maneuvers (even with small angle) - use laneChange desire
  if nav_type in ('off ramp', 'on ramp', 'merge', 'fork'):
    return custom.MaaControl.ManeuverType.laneChange

  # OSRM says turn but geometry is minor - still treat as turn
  if nav_type in ('turn', 'end of road'):
    return custom.MaaControl.ManeuverType.turn

  # Everything else: no action needed
  return custom.MaaControl.ManeuverType.none


def get_last_gps_position(params: Params) -> tuple:
  """Get last known GPS position from params. Returns (lat, lon, bearing) or None."""
  try:
    data = params.get("LastGPSPosition")
    if data:
      pos = json.loads(data)
      return pos.get('latitude'), pos.get('longitude'), pos.get('bearing', 0.0)
  except Exception:
    pass
  return None


def main():
  cloudlog.info("maa_controld: starting")

  params = Params()
  sm = messaging.SubMaster(
    ['navRoute', 'navInstruction', 'liveGPS', 'carState'],
    ignore_alive=['navRoute', 'navInstruction', 'liveGPS', 'carState']
  )
  pm = messaging.PubMaster(['maaControl'])

  rk = Ratekeeper(20)

  # State
  route_coords: list[Coordinate] = []
  last_route_len = 0
  last_gps_time = 0.0
  closest_idx = 0

  # Turn tracker (dead reckoning based)
  turn_tracker = TurnTracker()

  # Fallback position from params (used before liveGPS is ready)
  fallback_pos = get_last_gps_position(params)
  if fallback_pos:
    cloudlog.info(f"maa_controld: fallback position {fallback_pos[0]:.6f}, {fallback_pos[1]:.6f}")

  while True:
    sm.update(0)
    t = time.monotonic()

    # Get current position - prefer liveGPS, fall back to LastGPSPosition
    if sm.updated['liveGPS']:
      last_gps_time = time.monotonic()

    gps = sm['liveGPS']
    gps_stale = (time.monotonic() - last_gps_time) > 2.0
    gps_valid = gps.status == custom.LiveGPS.Status.valid and not gps_stale

    # Use fallback if liveGPS not ready
    use_fallback = not gps_valid and fallback_pos is not None
    if gps_valid:
      current_lat = gps.latitude
      current_lon = gps.longitude
      current_bearing = gps.bearingDeg
    elif use_fallback:
      current_lat, current_lon, current_bearing = fallback_pos
    else:
      current_lat = current_lon = current_bearing = None

    # Get turn info from navInstruction
    nav = sm['navInstruction']
    nav_valid = sm.valid['navInstruction']

    # Always update route coordinates (needed for turn angle calculation)
    nav_route = sm['navRoute']
    if sm.valid['navRoute'] and nav_route.coordinates:
      new_len = len(nav_route.coordinates)
      if new_len != last_route_len:
        route_coords = [
          Coordinate(c.latitude, c.longitude)
          for c in nav_route.coordinates
        ]
        last_route_len = new_len
        closest_idx = 0
        turn_tracker.reset()  # Reset turn state when route changes
        cloudlog.debug(f"maa_controld: route updated, {new_len} points")

    # Find current position on route
    has_position = current_lat is not None
    if route_coords and has_position:
      current_pos = Coordinate(current_lat, current_lon)
      try:
        # Optimization: Search locally around last known position
        search_start = max(0, closest_idx - 10)
        search_end = min(len(route_coords), closest_idx + 50)

        if closest_idx == 0:
          search_start = 0
          search_end = len(route_coords)

        subset = route_coords[search_start:search_end]
        local_idx, _ = find_closest_point_on_route(current_pos, subset)
        closest_idx = search_start + local_idx
      except Exception as e:
        cloudlog.warning(f"maa_controld: position error: {e}")

    # Get speed, blinker, and yawRate from carState
    cs = sm['carState']
    cs_valid = sm.valid['carState']
    v_ego = cs.vEgo if cs_valid else 0.0
    left_blinker = cs.leftBlinker if cs_valid else False
    right_blinker = cs.rightBlinker if cs_valid else False
    yaw_rate = cs.yawRate if cs_valid else 0.0

    # Continuous curvature assist (optional - for steering)
    curvature = 0.0
    curvature_valid = False
    if CURVATURE_ASSIST_ENABLED and route_coords and has_position and v_ego > MIN_SPEED_FOR_CURVATURE:
      try:
        curvature = compute_path_curvature(
          current_pos, current_bearing, route_coords, closest_idx, v_ego, CURVATURE_LOOKAHEAD
        )
        curvature_valid = True
      except Exception as e:
        cloudlog.warning(f"maa_controld: curvature error: {e}")

    # Build maaControl message
    msg = messaging.new_message('maaControl', valid=True)
    maa = msg.maaControl

    maa.curvature = float(curvature)
    maa.curvatureValid = curvature_valid

    # Get maneuver info from navInstruction (1Hz, used only for trigger)
    maneuver_dist = getattr(nav, 'maneuverDistance', None) if nav_valid else None

    # If turn tracker is active (including MISSED/COMPLETE), handle accordingly
    if turn_tracker.state in (TurnState.APPROACHING, TurnState.EXECUTING):
      # Safety checks: detect if turn info changed significantly
      if nav_valid and maneuver_dist is not None:
        # Use turnAngle from navInstructionExt - this is geometry-based (reliable)
        turn_angle = getattr(nav, 'turnAngle', 0.0) or 0.0
        estimated_dist = turn_tracker.get_estimated_distance()

        # Check 1: Did we pass the turn? (nav distance jumped up = now showing NEXT turn)
        # If we think we're close (<50m) but nav says far (>150m), we probably passed it
        if estimated_dist < 50.0 and maneuver_dist > 150.0:
          cloudlog.warning(f"maa: likely passed turn (est={estimated_dist:.0f}m, nav={maneuver_dist:.0f}m), resetting")
          turn_tracker.reset()

        # Check 2: Did direction flip? (route recalculated or now showing different turn)
        else:
          current_nav_dir = None
          if turn_angle > TURN_MIN_ANGLE:
            current_nav_dir = 'left'
          elif turn_angle < -TURN_MIN_ANGLE:
            current_nav_dir = 'right'

          if current_nav_dir and current_nav_dir != turn_tracker.direction:
            cloudlog.warning(f"maa: turn direction changed ({turn_tracker.direction} → {current_nav_dir}), angle={turn_angle:.1f}°, resetting")
            turn_tracker.reset()

          # Check 3: Turn disappeared (angle now below threshold)
          elif abs(turn_angle) < TURN_MIN_ANGLE:
            nav_type = getattr(nav, 'maneuverType', '') or ''
            if get_maneuver_type(nav_type, turn_angle) == custom.MaaControl.ManeuverType.none:
              cloudlog.warning(f"maa: turn no longer valid (angle={turn_angle:.1f}°), resetting")
              turn_tracker.reset()

    if turn_tracker.state in (TurnState.APPROACHING, TurnState.EXECUTING):
      # Check blinker and steering for driver acknowledgment (like lane change)
      turn_tracker.check_blinker(left_blinker, right_blinker)
      turn_tracker.check_steering(cs.steeringPressed, cs.steeringTorque)

      # Dead reckon distance - ignore navInstruction updates
      estimated_dist = turn_tracker.get_estimated_distance()

      # Fix 2: Allow abort if blinker turns off before commitment (>100m)
      # User can change their mind if not yet committed to the turn
      if turn_tracker.state == TurnState.APPROACHING and turn_tracker.driver_acknowledged:
        blinker_matches = (turn_tracker.direction == 'left' and left_blinker) or \
                          (turn_tracker.direction == 'right' and right_blinker)
        if not blinker_matches and estimated_dist > TURN_COMMIT_DISTANCE:
          turn_tracker.driver_acknowledged = False
          turn_tracker.steering_confirmed = False
          cloudlog.info("maa: blinker canceled before commit, aborting turn assist")
      maa.turnDistance = float(estimated_dist)
      maa.turnValid = True
      maa.turnAngle = float(turn_tracker.expected_angle)
      maa.maneuverType = turn_tracker.maneuver_type
      maa.turnSpeedLimit = float(turn_tracker.turn_speed_limit)

      # Direction from captured state
      if turn_tracker.direction == 'left':
        maa.turnDirection = custom.MaaControl.TurnDirection.left
      elif turn_tracker.direction == 'right':
        maa.turnDirection = custom.MaaControl.TurnDirection.right
      else:
        maa.turnDirection = custom.MaaControl.TurnDirection.none

      # Update tracker with dead reckoning
      desire_active = turn_tracker.update(t, v_ego, yaw_rate)
      maa.turnState = int(turn_tracker.state)
      maa.turnProgress = float(turn_tracker.accumulated_yaw)

      # Driver acknowledgment status
      maa.driverAcknowledged = turn_tracker.driver_acknowledged

      # Speed limit active when blinker on (driver acknowledged)
      maa.speedLimitActive = turn_tracker.driver_acknowledged

      # Block lane change when committed (blinker + within commit distance)
      maa.blockLaneChange = turn_tracker.driver_acknowledged and estimated_dist <= TURN_COMMIT_DISTANCE

      # desireActive: send turn desire to model (requires blinker)
      maa.desireActive = desire_active

      # Curvature from captured state (not recalculated)
      maa.curvature = 0.0
      maa.curvatureValid = False
      maa.turnCurvature = 0.0

    elif nav_valid and maneuver_dist is not None:
      # No active turn tracking - check if we should trigger
      maa.turnDistance = float(maneuver_dist)
      maa.turnValid = maneuver_dist < TURN_VALID_DISTANCE

      nav_type = getattr(nav, 'maneuverType', '') or ''
      modifier = getattr(nav, 'maneuverModifier', '') or ''

      # Get pre-computed turn geometry from navInstruction
      turn_angle = getattr(nav, 'turnAngle', 0.0) or 0.0
      turn_curvature = getattr(nav, 'turnCurvature', 0.0) or 0.0

      maa.turnAngle = float(turn_angle)
      maa.turnCurvature = float(turn_curvature)

      # Compute maneuver type
      maa.maneuverType = get_maneuver_type(nav_type, turn_angle)

      # Set turn direction
      if maa.maneuverType != custom.MaaControl.ManeuverType.none:
        if 'left' in modifier.lower():
          maa.turnDirection = custom.MaaControl.TurnDirection.left
        elif 'right' in modifier.lower():
          maa.turnDirection = custom.MaaControl.TurnDirection.right
        elif turn_angle > TURN_MIN_ANGLE:
          maa.turnDirection = custom.MaaControl.TurnDirection.left
        elif turn_angle < -TURN_MIN_ANGLE:
          maa.turnDirection = custom.MaaControl.TurnDirection.right
        else:
          maa.turnDirection = custom.MaaControl.TurnDirection.none
      else:
        maa.turnDirection = custom.MaaControl.TurnDirection.none

      # Compute turn speed limit using CarrotPilot curvature lookup table
      # Curvature-based is more accurate than heuristics when available
      turn_speed_limit = get_turn_speed_limit(modifier, turn_angle, nav_type, turn_curvature)
      maa.turnSpeedLimit = float(turn_speed_limit)

      # Check if we should trigger dead reckoning
      # Don't trigger if already in MISSED/COMPLETE state (wait for route change)
      can_trigger = turn_tracker.state == TurnState.NONE
      is_actionable = maa.maneuverType != custom.MaaControl.ManeuverType.none
      has_direction = maa.turnDirection != custom.MaaControl.TurnDirection.none
      is_significant = abs(turn_angle) >= TURN_MIN_ANGLE
      in_trigger_range = maneuver_dist <= TURN_TRIGGER_DISTANCE

      if can_trigger and is_actionable and has_direction and is_significant and in_trigger_range:
        # Trigger! Capture turn params and start dead reckoning
        turn_tracker.trigger(
          t, maneuver_dist, turn_angle,
          maa.maneuverType, maa.turnDirection, turn_speed_limit
        )
        # Check blinker and steering immediately after trigger
        turn_tracker.check_blinker(left_blinker, right_blinker)
        turn_tracker.check_steering(cs.steeringPressed, cs.steeringTorque)
        # Blinker = approach confirmation, steering = turn execution confirmation
        maa.desireActive = turn_tracker.driver_acknowledged and turn_tracker.steering_confirmed and maneuver_dist <= TURN_COMMIT_DISTANCE
        maa.turnState = int(turn_tracker.state)
        maa.turnProgress = 0.0
        maa.driverAcknowledged = turn_tracker.driver_acknowledged
        maa.speedLimitActive = turn_tracker.driver_acknowledged
        maa.blockLaneChange = turn_tracker.driver_acknowledged and maneuver_dist <= TURN_COMMIT_DISTANCE
      elif turn_tracker.state in (TurnState.COMPLETE, TurnState.MISSED):
        # In cooldown - call update to handle state transitions
        turn_tracker.update(t, v_ego, yaw_rate)
        maa.desireActive = False
        maa.turnState = int(turn_tracker.state)
        maa.turnProgress = float(turn_tracker.accumulated_yaw)
        maa.driverAcknowledged = False
        maa.speedLimitActive = False
        maa.blockLaneChange = False
      else:
        # Not triggered yet (turn too far away)
        maa.desireActive = False
        maa.turnState = int(TurnState.NONE)
        maa.turnProgress = 0.0
        maa.driverAcknowledged = False
        maa.speedLimitActive = False
        maa.blockLaneChange = False

    else:
      # No valid nav instruction
      maa.turnValid = False
      maa.turnDirection = custom.MaaControl.TurnDirection.none
      maa.turnSpeedLimit = 50.0 / 3.6
      maa.maneuverType = custom.MaaControl.ManeuverType.none
      maa.turnAngle = 0.0
      maa.turnCurvature = 0.0
      maa.turnDistance = 9999.0
      maa.desireActive = False
      maa.turnState = int(TurnState.NONE)
      maa.turnProgress = 0.0
      maa.driverAcknowledged = False
      maa.speedLimitActive = False
      maa.blockLaneChange = False

      # Only reset if not in MISSED state (wait for route change)
      if turn_tracker.state != TurnState.MISSED:
        turn_tracker.reset()

    pm.send('maaControl', msg)
    rk.keep_time()


if __name__ == "__main__":
  main()
