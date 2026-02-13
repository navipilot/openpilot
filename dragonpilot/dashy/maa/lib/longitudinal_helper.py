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

Map-Aware Assist Longitudinal Helper

Provides navigation-based speed and acceleration limiting for the longitudinal planner.
This module keeps all nav-related planner logic isolated from the core planner code.

Features:
- Turn speed limiting based on maaControl cereal message
- Physics-based acceleration limiting using friction circle
- Smooth slowdown/resume transitions
- Driver acknowledgment (blinker) required to activate speed limiting

Adapted from CarrotPilot (https://github.com/ajouatom/openpilot)
Credit: carrotpilot team for curvature-based speed and physics-based accel limiting.

Usage in longitudinal_planner.py:
  from dragonpilot.dashy.maa.lib.longitudinal_helper import LongitudinalHelper

  # Add 'maaControl' to SubMaster
  # In update:
  maa = sm['maaControl']
  if sm.valid['maaControl'] and maa.turnValid and maa.speedLimitActive:
    # Only apply speed limit when driver acknowledged (blinker on)
    turn_speed = maa.turnSpeedLimit
    turn_distance = maa.turnDistance
  else:
    turn_speed, turn_distance = None, None
  v_cruise = self.maa_helper.apply_nav_speed_limit(v_cruise, turn_speed, turn_distance)
"""

from dataclasses import dataclass
from typing import Optional, List

# Import nav modules
try:
  from dragonpilot.dashy.maa.lib.accel_limiter import AccelLimiter
  ACCEL_LIMITER_AVAILABLE = True
except ImportError:
  ACCEL_LIMITER_AVAILABLE = False


@dataclass
class NavPlannerState:
  """Current state of nav planner integration."""
  nav_limited_speed: Optional[float] = None
  is_limiting_speed: bool = False
  is_limiting_accel: bool = False


@dataclass
class VirtualLead:
  """Virtual lead car for turn deceleration."""
  status: bool = False
  dRel: float = 100.0  # distance to virtual lead
  vLead: float = 0.0   # speed of virtual lead
  aLeadK: float = 0.0  # acceleration
  aLeadTau: float = 0.3


class RadarStateWrapper:
  """
  Wrapper to inject virtual lead into radarState for MPC.

  This allows the MPC to "see" a fake slow car at the turn point,
  triggering natural deceleration using the well-tuned lead following logic.
  """
  def __init__(self, radar_state, virtual_lead=None):
    self._radar_state = radar_state
    self._virtual_lead = virtual_lead

  @property
  def leadOne(self):
    real_lead = self._radar_state.leadOne
    # If no virtual lead, use real lead
    if self._virtual_lead is None or not self._virtual_lead.status:
      return real_lead
    # If real lead exists and is closer, use real lead
    if real_lead.status and real_lead.dRel < self._virtual_lead.dRel:
      return real_lead
    # Use virtual lead (turn point)
    return self._virtual_lead

  @property
  def leadTwo(self):
    return self._radar_state.leadTwo

  def __getattr__(self, name):
    return getattr(self._radar_state, name)


class LongitudinalHelper:
  """
  Navigation helper for longitudinal planner.

  Provides turn speed limiting and physics-based acceleration limiting
  based on navigation data. Keeps all nav logic isolated from core planner.

  Design rationale (based on automotive research):
  - Stopping distance at 50 km/h is ~48m (25m braking + 15m reaction)
  - Intersection approach speeds typically 30-50 km/h
  - Comfortable deceleration: 1.5-2.0 m/s² for normal driving
  """

  # Configuration - Speed limiting
  # Physics-based: calculate braking distance from speed difference
  # More natural driving: maintain speed, brake closer to turn
  SLOWDOWN_END_DIST = 10.0  # meters - be at turn speed by this distance (buffer before turn)
  SLOWDOWN_BUFFER = 15.0    # meters - extra buffer added to braking distance

  # Physics-based accel limiting
  ACCEL_LIMIT_ENABLED = True
  LAT_ACCEL_MAX = 2.5  # m/s² - max comfortable lateral acceleration

  # Comfortable deceleration target (m/s²)
  # Research shows 1.5-2.0 m/s² is comfortable for passengers
  # Using 2.0 for more natural, later braking
  COMFORT_DECEL = 2.0

  def __init__(self):
    """Initialize nav planner helper."""
    self.state = NavPlannerState()

    # Physics-based acceleration limiter
    if ACCEL_LIMITER_AVAILABLE and self.ACCEL_LIMIT_ENABLED:
      self.accel_limiter = AccelLimiter(
        lat_accel_max=self.LAT_ACCEL_MAX,
        comfort_mode=True
      )
    else:
      self.accel_limiter = None

  def apply_nav_speed_limit(
    self,
    v_cruise: float,
    turn_speed: Optional[float] = None,
    distance: Optional[float] = None
  ) -> float:
    """
    Apply navigation-based speed limiting with physics-based braking.

    Uses kinematic equation: d = (v² - v_target²) / (2 * decel)
    Starts braking at calculated distance + buffer for natural driving feel.

    Args:
      v_cruise: Current cruise speed in m/s
      turn_speed: Turn speed limit from maaControl (None if invalid)
      distance: Distance to turn from maaControl (None if invalid)

    Returns:
      Limited cruise speed
    """
    if turn_speed is None or distance is None:
      self.state.nav_limited_speed = None
      self.state.is_limiting_speed = False
      return v_cruise

    # No need to slow if already at or below turn speed
    if v_cruise <= turn_speed:
      self.state.nav_limited_speed = None
      self.state.is_limiting_speed = False
      return v_cruise

    # Calculate required braking distance using kinematics
    # d = (v² - v_target²) / (2 * decel)
    speed_diff_sq = v_cruise ** 2 - turn_speed ** 2
    braking_distance = speed_diff_sq / (2 * self.COMFORT_DECEL)

    # Start braking at: braking_distance + buffer + end_distance
    slowdown_start = braking_distance + self.SLOWDOWN_BUFFER + self.SLOWDOWN_END_DIST

    # Outside slowdown zone - no limit
    if distance > slowdown_start:
      self.state.nav_limited_speed = None
      self.state.is_limiting_speed = False
      return v_cruise

    # In slowdown zone - calculate target speed at this distance
    # v² = v_target² + 2 * decel * (distance - end_dist)
    if distance > self.SLOWDOWN_END_DIST:
      remaining = distance - self.SLOWDOWN_END_DIST
      # Target speed that allows comfortable braking to turn_speed
      target_speed_sq = turn_speed ** 2 + 2 * self.COMFORT_DECEL * remaining
      limited_speed = min(v_cruise, target_speed_sq ** 0.5)
    else:
      # Very close to turn - calculate achievable speed with comfort decel
      # This handles late blinker: never brake harder than COMFORT_DECEL
      # If we can't reach turn_speed in time, accept entering turn faster
      achievable_speed_sq = turn_speed ** 2 + 2 * self.COMFORT_DECEL * max(0, distance)
      achievable_speed = achievable_speed_sq ** 0.5
      # Never target lower than achievable (prevents harsh braking)
      limited_speed = min(v_cruise, max(turn_speed, achievable_speed))

    self.state.nav_limited_speed = limited_speed
    self.state.is_limiting_speed = True
    return limited_speed

  # Minimum distance for virtual lead - below this, don't use virtual lead
  # to avoid MPC braking to stop
  VIRTUAL_LEAD_MIN_DIST = 15.0  # meters

  def get_virtual_lead(
    self,
    v_ego: float,
    turn_speed: Optional[float] = None,
    distance: Optional[float] = None
  ) -> Optional[VirtualLead]:
    """
    Create a virtual lead car at the turn point for natural deceleration.

    The virtual lead "drives" at turn_speed, positioned at the turn point.
    This makes the MPC decelerate naturally as if following a slow car.

    Args:
      v_ego: Current vehicle speed in m/s
      turn_speed: Turn speed limit from maaControl (None if invalid)
      distance: Distance to turn from maaControl (None if invalid)

    Returns:
      VirtualLead object if turn is approaching, None otherwise
    """
    if turn_speed is None or distance is None:
      return None

    # Only create virtual lead when we need to slow down
    if v_ego <= turn_speed:
      return None

    # Don't use virtual lead when very close to turn - avoids brake-to-stop
    # At this point, we should already be at turn speed from earlier braking
    if distance < self.VIRTUAL_LEAD_MIN_DIST:
      return None

    # Calculate when to start showing virtual lead
    # Use same physics: braking distance + buffer
    speed_diff_sq = v_ego ** 2 - turn_speed ** 2
    braking_distance = speed_diff_sq / (2 * self.COMFORT_DECEL)
    activation_distance = braking_distance + self.SLOWDOWN_BUFFER + self.SLOWDOWN_END_DIST

    if distance > activation_distance:
      return None

    # Create virtual lead at turn point, moving at turn speed
    # The MPC will naturally decelerate to follow it
    return VirtualLead(
      status=True,
      dRel=distance,      # distance to virtual lead
      vLead=turn_speed,   # virtual lead moves at turn speed
      aLeadK=0.0,         # no acceleration (constant speed)
      aLeadTau=0.3        # response time constant
    )

  def apply_nav_accel_limit(
    self,
    v_ego: float,
    curvature: float,
    accel_clip: List[float]
  ) -> List[float]:
    """
    Apply physics-based acceleration limiting for turns.

    Uses friction circle: a_x² + a_y² ≤ a_max²

    Args:
      v_ego: Current vehicle speed in m/s
      curvature: Current road curvature (from nav or model)
      accel_clip: Current [min_accel, max_accel] limits

    Returns:
      Updated [min_accel, max_accel] with turn limiting applied
    """
    if self.accel_limiter is None:
      return accel_clip

    if abs(curvature) < 0.001:  # Essentially straight
      self.state.is_limiting_accel = False
      return accel_clip

    limited = list(self.accel_limiter.limit_accel_tuple(
      v_ego, curvature, tuple(accel_clip)
    ))

    self.state.is_limiting_accel = self.accel_limiter.state.is_limiting
    return limited

  # Staleness threshold for maaControl message (nanoseconds)
  STALE_THRESHOLD_NS = 5e8  # 0.5 seconds

  def process(
    self,
    sm,
    v_ego: float,
    v_cruise: float,
    accel_clip: List[float]
  ) -> tuple:
    """
    Process maaControl and return updated planner values.

    Encapsulates all MAA logic:
    - Validity and staleness checking
    - Speed limiting (when driver acknowledged via blinker)
    - Virtual lead creation for natural deceleration
    - Curvature-based acceleration limiting

    Args:
      sm: SubMaster with 'maaControl' and 'carState'
      v_ego: Current vehicle speed in m/s
      v_cruise: Current cruise speed in m/s
      accel_clip: Current [min_accel, max_accel] limits

    Returns:
      tuple: (v_cruise, accel_clip, virtual_lead)
    """
    virtual_lead = None

    maa = sm['maaControl']

    # Check valid and not stale
    maa_valid = sm.valid['maaControl'] and maa.turnValid
    if maa_valid and (sm.logMonoTime['carState'] - sm.logMonoTime['maaControl']) > self.STALE_THRESHOLD_NS:
      maa_valid = False

    if not maa_valid:
      self.state.nav_limited_speed = None
      self.state.is_limiting_speed = False
      self.state.is_limiting_accel = False
      return v_cruise, accel_clip, virtual_lead

    # Speed limiting only when driver acknowledged (blinker on)
    # Without blinker: informational only, no speed reduction
    if maa.speedLimitActive:
      virtual_lead = self.get_virtual_lead(v_ego, maa.turnSpeedLimit, maa.turnDistance)
      v_cruise = self.apply_nav_speed_limit(v_cruise, maa.turnSpeedLimit, maa.turnDistance)

    # Curvature-based acceleration limiting (always active when valid)
    if maa.curvatureValid and abs(maa.curvature) > 0.001:
      accel_clip = self.apply_nav_accel_limit(v_ego, maa.curvature, accel_clip)

    return v_cruise, accel_clip, virtual_lead
