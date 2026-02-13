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

Model Helper for modeld integration.

Provides turn desire logic for modeld.py.
"""

from cereal import log
from dragonpilot.dashy.maa.lib.maa_desire import (
  should_block_lane_change,
  get_turn_desire,
)


class ModelHelper:
  """
  Helper class for MAA integration with modeld.

  Usage in modeld.py:
    from dragonpilot.dashy.maa.lib import ModelHelper
    model_helper = ModelHelper()

    # In the loop:
    is_rhd = sm["driverMonitoringState"].isRHD
    desire = model_helper.get_desire(sm, lane_change_desire, is_rhd)
  """

  def __init__(self):
    self.active = False
    self.last_desire = log.Desire.none

  def get_desire(self, sm, lane_change_desire: int, is_rhd: bool = False) -> int:
    """
    Get combined desire from MAA turn logic and lane change.

    Priority:
    1. Active lane change (driver initiated) - don't interrupt
    2. MAA turn desire (navigation turn)
    3. Lane change desire (pre-lane-change, keep, etc.)

    Args:
      sm: SubMaster with maaControl and carState
      lane_change_desire: Desire from DesireHelper (lane change logic)
      is_rhd: True if right-hand drive

    Returns:
      Final desire value
    """
    # Check if MAA data is available
    if not sm.valid.get('maaControl', False) or not sm.valid.get('carState', False):
      return lane_change_desire

    # Check for stale maaControl (wait for 0.5s)
    if (sm.logMonoTime['carState'] - sm.logMonoTime['maaControl']) > 5e8:
      return lane_change_desire

    maa_control = sm['maaControl']
    carstate = sm['carState']

    # Don't interrupt active lane change
    if lane_change_desire in (log.Desire.laneChangeLeft, log.Desire.laneChangeRight):
      self.active = False
      return lane_change_desire

    # Check if lane change should be blocked due to approaching turn
    if should_block_lane_change(maa_control, carstate.vEgo):
      # Block lane change desires, but allow none/keep
      if lane_change_desire in (log.Desire.laneChangeLeft, log.Desire.laneChangeRight):
        lane_change_desire = log.Desire.none

    # Get MAA turn desire
    maa_desire = get_turn_desire(maa_control, carstate, is_rhd)

    # MAA turn desire takes priority over none/keep
    if maa_desire != log.Desire.none:
      self.active = True
      self.last_desire = maa_desire
      return maa_desire

    # If MAA was active but now returns none, we completed the turn
    if self.active and maa_desire == log.Desire.none:
      self.active = False

    return lane_change_desire

  def update(self, modelv2, desire_state):
    """
    Update helper with model output (for future use).

    Can be used to detect turn completion via desireState probabilities.

    Args:
      modelv2: ModelV2 message
      desire_state: Model's desire state output
    """
    # Reserved for future use - detecting turn completion via model output
    pass
