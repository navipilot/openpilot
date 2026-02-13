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

MAA Desire Helper

Turn execution flow:
1. APPROACHING (200m): Start dead reckoning, show turn suggestion
2. Driver turns on blinker: acknowledged
3. At 100m with blinker: COMMIT - block lane change, slow down, send desire
4. EXECUTING (30m): Track heading change
5. COMPLETE: When heading change ≈ expected turn angle

Turn desire is sent when:
- desireActive is true (from maa_controld)
- This requires: driver acknowledged (blinker) + committed (<100m) + EXECUTING state

Without blinker:
- System shows turn info but doesn't intervene
- No speed reduction, no desire sent
- Driver maintains full control
"""

from cereal import log, custom

TurnDirection = custom.MaaControl.TurnDirection
ManeuverType = custom.MaaControl.ManeuverType
TurnState = {
  'NONE': 0,
  'APPROACHING': 1,
  'EXECUTING': 2,
  'COMPLETE': 3,
  'MISSED': 4,
}

# Configuration
MAX_TURN_SPEED = 50.0 / 3.6  # m/s (50 km/h) - no turn assist above this
SHARP_TURN_ANGLE = 45.0  # degrees - above: turnLeft/Right, below: laneChangeLeft/Right


def should_block_lane_change(maa_control, v_ego: float) -> bool:
  """
  Check if lane change should be blocked due to approaching turn.

  Uses blockLaneChange from maa_controld which is set when:
  - Driver acknowledged (blinker on matching direction)
  - Within commit distance (100m)
  """
  if maa_control is None:
    return False
  if v_ego > MAX_TURN_SPEED:
    return False
  # Use the pre-computed blockLaneChange from maa_controld
  return maa_control.blockLaneChange


def get_turn_desire(maa_control, carstate, is_rhd: bool = False) -> log.Desire:
  """
  Get turn desire based on maaControl.

  desireActive from maa_controld is true when:
  - Driver acknowledged (blinker matches turn direction)
  - Committed (<100m) OR in EXECUTING state

  This function adds additional checks:
  - Speed < 50 km/h
  - maneuverType == turn

  Args:
    maa_control: MaaControl message
    carstate: CarState message
    is_rhd: True if right-hand drive (UK/Japan), False for left-hand drive (US/Taiwan)

  Returns:
  - turnLeft/Right if angle >= 45°
  - laneChangeLeft/Right if angle < 45°
  - none if conditions not met
  """
  if maa_control is None or not maa_control.turnValid:
    return log.Desire.none

  if maa_control.maneuverType != ManeuverType.turn:
    return log.Desire.none

  if carstate.vEgo > MAX_TURN_SPEED:
    return log.Desire.none

  # desireActive encapsulates: acknowledged + (committed OR executing)
  if not maa_control.desireActive:
    return log.Desire.none

  # Sharp turn (>= 45°) uses turnLeft/Right, gentle uses laneChange
  is_sharp = abs(maa_control.turnAngle) >= SHARP_TURN_ANGLE

  if maa_control.turnDirection == TurnDirection.left:
    return log.Desire.turnLeft if is_sharp else log.Desire.laneChangeLeft
  elif maa_control.turnDirection == TurnDirection.right:
    return log.Desire.turnRight if is_sharp else log.Desire.laneChangeRight

  return log.Desire.none
