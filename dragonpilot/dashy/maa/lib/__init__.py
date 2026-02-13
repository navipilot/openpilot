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

dragonpilot Map-Aware Assist Library

Core modules:
- maa_desire: Simple turn desire logic (blinker confirmation, lane change blocking, RHD/LHD)
- model_helper: Helper class for modeld integration
- longitudinal_helper: Planner integration for speed/accel limiting
"""

from dragonpilot.dashy.maa.lib.maa_desire import (
  should_block_lane_change,
  get_turn_desire,
  is_crossing_turn,
  get_turn_trigger_distance,
)
from dragonpilot.dashy.maa.lib.model_helper import ModelHelper
from dragonpilot.dashy.maa.lib.longitudinal_helper import LongitudinalHelper

__all__ = [
  'should_block_lane_change',
  'get_turn_desire',
  'is_crossing_turn',
  'get_turn_trigger_distance',
  'ModelHelper',
  'LongitudinalHelper',
]
