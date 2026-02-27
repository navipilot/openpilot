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

Acceleration Limiter

Physics-based acceleration limiting for turns.

Algorithm adapted from CarrotPilot (https://github.com/ajouatom/openpilot)
Credit: carrotpilot team for the physics-based lateral acceleration approach.

The key insight is that total acceleration is limited by tire grip:
  a_total² = a_x² + a_y² ≤ a_max²

Where:
  a_x = longitudinal acceleration (throttle/brake)
  a_y = lateral acceleration (from turning) = v² × curvature

This means during turns, we must reduce longitudinal acceleration
to stay within the grip circle.
"""

import math
from dataclasses import dataclass
from typing import Tuple, Optional


@dataclass
class AccelLimits:
  """Acceleration limits."""
  min_accel: float  # m/s² (negative = braking)
  max_accel: float  # m/s² (positive = acceleration)


@dataclass
class AccelLimiterState:
  """Current state of acceleration limiter."""
  v_ego: float = 0.0
  curvature: float = 0.0
  lateral_accel: float = 0.0
  available_long_accel: float = 0.0
  is_limiting: bool = False


class AccelLimiter:
  """
  Physics-based acceleration limiter for turns.

  Uses the friction circle concept: total acceleration magnitude
  is limited by tire grip. During turns, lateral acceleration
  consumes part of this budget, leaving less for longitudinal
  acceleration.

  Adapted from CarrotPilot's limit_accel_in_turns function.
  """

  # Default lateral acceleration limit (m/s²)
  # Comfortable limit is ~2-3 m/s², sporty is ~4 m/s²
  DEFAULT_LAT_ACCEL_MAX = 2.5

  # Lookup table for max total acceleration vs speed
  # Higher speeds = lower max lateral accel for comfort
  A_TOTAL_MAX_BP = [0., 10., 20., 30., 40.]  # m/s
  A_TOTAL_MAX_V = [3.0, 2.8, 2.5, 2.2, 2.0]  # m/s²

  def __init__(
    self,
    lat_accel_max: float = DEFAULT_LAT_ACCEL_MAX,
    comfort_mode: bool = True
  ):
    """
    Initialize acceleration limiter.

    Args:
      lat_accel_max: Maximum allowed lateral acceleration (m/s²)
      comfort_mode: If True, use speed-dependent limits
    """
    self.lat_accel_max = lat_accel_max
    self.comfort_mode = comfort_mode
    self.state = AccelLimiterState()

  def get_max_total_accel(self, v_ego: float) -> float:
    """
    Get maximum total acceleration for current speed.

    In comfort mode, reduces limit at higher speeds.
    """
    if not self.comfort_mode:
      return self.lat_accel_max

    # Interpolate from lookup table
    import numpy as np
    return np.interp(v_ego, self.A_TOTAL_MAX_BP, self.A_TOTAL_MAX_V)

  def compute_lateral_accel(self, v_ego: float, curvature: float) -> float:
    """
    Compute lateral acceleration from speed and curvature.

    a_y = v² × κ

    Args:
      v_ego: Vehicle speed in m/s
      curvature: Road curvature in 1/m

    Returns:
      Lateral acceleration in m/s²
    """
    return v_ego * v_ego * abs(curvature)

  def compute_available_long_accel(
    self,
    v_ego: float,
    curvature: float,
    a_max: Optional[float] = None
  ) -> float:
    """
    Compute available longitudinal acceleration given current turn.

    Uses friction circle: a_x² + a_y² ≤ a_max²
    Solving for a_x: a_x = sqrt(a_max² - a_y²)

    Adapted from CarrotPilot's limit_accel_in_turns.

    Args:
      v_ego: Vehicle speed in m/s
      curvature: Road curvature in 1/m
      a_max: Override for max total acceleration

    Returns:
      Maximum available longitudinal acceleration in m/s²
    """
    if a_max is None:
      a_max = self.get_max_total_accel(v_ego)

    # Compute lateral acceleration
    a_y = self.compute_lateral_accel(v_ego, curvature)
    a_y_abs = abs(a_y)

    # Update state
    self.state.v_ego = v_ego
    self.state.curvature = curvature
    self.state.lateral_accel = a_y

    # Check if lateral accel exceeds limit
    if a_y_abs >= a_max:
      # Already at or over limit - no longitudinal accel available
      self.state.available_long_accel = 0.0
      self.state.is_limiting = True
      return 0.0

    # Compute remaining budget for longitudinal acceleration
    a_x_available = math.sqrt(a_max * a_max - a_y_abs * a_y_abs)

    self.state.available_long_accel = a_x_available
    self.state.is_limiting = a_x_available < a_max * 0.9  # Limiting if < 90% available

    return a_x_available

  def limit_accel(
    self,
    v_ego: float,
    curvature: float,
    accel_limits: AccelLimits,
    a_max: Optional[float] = None
  ) -> AccelLimits:
    """
    Apply turn-based acceleration limiting.

    Args:
      v_ego: Vehicle speed in m/s
      curvature: Road curvature in 1/m
      accel_limits: Current acceleration limits
      a_max: Override for max total acceleration

    Returns:
      New AccelLimits with turn limiting applied
    """
    a_x_available = self.compute_available_long_accel(v_ego, curvature, a_max)

    # Clamp max acceleration to available budget
    new_max = min(accel_limits.max_accel, a_x_available)

    # Don't limit braking as much - we may need to slow down
    # But still apply some limit for comfort
    new_min = max(accel_limits.min_accel, -a_x_available * 1.5)

    return AccelLimits(
      min_accel=new_min,
      max_accel=new_max
    )

  def limit_accel_tuple(
    self,
    v_ego: float,
    curvature: float,
    accel_limits: Tuple[float, float],
    a_max: Optional[float] = None
  ) -> Tuple[float, float]:
    """
    Apply turn-based acceleration limiting (tuple interface).

    For compatibility with existing planner code.

    Args:
      v_ego: Vehicle speed in m/s
      curvature: Road curvature in 1/m
      accel_limits: (min_accel, max_accel) tuple
      a_max: Override for max total acceleration

    Returns:
      (min_accel, max_accel) tuple with turn limiting applied
    """
    limits = self.limit_accel(
      v_ego,
      curvature,
      AccelLimits(accel_limits[0], accel_limits[1]),
      a_max
    )
    return (limits.min_accel, limits.max_accel)
