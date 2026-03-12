#!/usr/bin/env python3
import time

import numpy as np

from openpilot.common.constants import CV
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.realtime import DT_MDL

from openpilot.frogpilot.common.frogpilot_variables import CRUISING_SPEED, THRESHOLD

CEStatus = {
  "OFF": 0,              # Off
  "USER_DISABLED": 1,    # "Experimental Mode" disabled by user
  "USER_OVERRIDDEN": 2,  # "Experimental Mode" enabled by user
  "CURVATURE": 3,        # Road curvature condition
  "LEAD": 4,             # Slower lead vehicle condition
  "SIGNAL": 5,           # Turn signal condition
  "SPEED": 6,            # Speed condition
  "SPEED_LIMIT": 7,      # Speed limit controller condition
  "STOP_LIGHT": 8        # Stop light or sign condition
}

class ConditionalExperimentalMode:
  # Speed ranges: [0-35, 35-55, 55-70, 70+ mph]
  FILTER_TIME_CURVES = [0.9, 0.8, 0.6, 0.5]
  FILTER_TIME_LEADS = [0.9, 0.8, 0.7, 0.5]
  FILTER_TIME_LIGHTS = [0.9, 0.8, 0.75, 0.55]
  LIGHT_BOOSTS = [1.0, 1.2, 1.1, 1.0]
  LIGHT_MAX_TIME = 9.0
  CEM_TRANSITION_GUARD_TIME = 0.50
  CEM_TRANSITION_BUFFER_TIME = 0.25

  def __init__(self, FrogPilotPlanner):
    self.frogpilot_planner = FrogPilotPlanner

    self.curvature_filter = FirstOrderFilter(0.0, self.FILTER_TIME_CURVES[1], DT_MDL)
    self.slow_lead_filter = FirstOrderFilter(0.0, self.FILTER_TIME_LEADS[1], DT_MDL)
    self.stop_light_filter = FirstOrderFilter(0.0, self.FILTER_TIME_LIGHTS[1], DT_MDL)

    self.curve_detected = False
    self.slow_lead_detected = False
    self.experimental_mode = False
    self.stop_light_detected = False
    self.mode_hold_until = 0.0
    self.mode_false_since = 0.0

  def _update_filter_time_constants(self, v_ego):
    speed_mph = v_ego * CV.MS_TO_MPH
    curve_time = float(np.interp(speed_mph, [0, 35, 55, 70], self.FILTER_TIME_CURVES))
    lead_time = float(np.interp(speed_mph, [0, 35, 55, 70], self.FILTER_TIME_LEADS))
    light_time = float(np.interp(speed_mph, [0, 35, 55, 70], self.FILTER_TIME_LIGHTS))

    self.curvature_filter = FirstOrderFilter(self.curvature_filter.x, curve_time, DT_MDL)
    self.slow_lead_filter = FirstOrderFilter(self.slow_lead_filter.x, lead_time, DT_MDL)
    self.stop_light_filter = FirstOrderFilter(self.stop_light_filter.x, light_time, DT_MDL)

  def update(self, v_ego, sm, frogpilot_toggles):
    now = time.monotonic()
    if frogpilot_toggles.experimental_mode_via_press:
      self.status_value = self.frogpilot_planner.params_memory.get_int("CEStatus")
    else:
      self.status_value = CEStatus["OFF"]

    if self.status_value not in (CEStatus["USER_DISABLED"], CEStatus["USER_OVERRIDDEN"]) and not sm["carState"].standstill:
      self.update_conditions(v_ego, sm, frogpilot_toggles)
      triggered = self.check_conditions(v_ego, sm, frogpilot_toggles)

      if triggered:
        self.mode_hold_until = now + self.CEM_TRANSITION_GUARD_TIME
        self.mode_false_since = 0.0
      elif self.mode_false_since == 0.0:
        self.mode_false_since = now

      hold_active = now < self.mode_hold_until
      transition_buffer_active = self.mode_false_since != 0.0 and (now - self.mode_false_since) < self.CEM_TRANSITION_BUFFER_TIME

      self.experimental_mode = triggered or hold_active or transition_buffer_active
      self.frogpilot_planner.params_memory.put_int("CEStatus", self.status_value if self.experimental_mode else CEStatus["OFF"])
    else:
      self.mode_hold_until = 0.0
      self.mode_false_since = 0.0
      self.experimental_mode &= sm["carState"].standstill and self.frogpilot_planner.model_stopped
      self.experimental_mode &= self.status_value != CEStatus["USER_DISABLED"]
      self.experimental_mode |= self.status_value == CEStatus["USER_OVERRIDDEN"]

      self.stop_light_detected &= self.status_value not in (CEStatus["USER_DISABLED"], CEStatus["USER_OVERRIDDEN"])
      self.stop_light_filter.x = 0

  def check_conditions(self, v_ego, sm, frogpilot_toggles):
    if self.curve_detected and frogpilot_toggles.conditional_curves and (not self.frogpilot_planner.frogpilot_following.following_lead or frogpilot_toggles.conditional_curves_lead):
      self.status_value = CEStatus["CURVATURE"]
      return True

    if self.slow_lead_detected and frogpilot_toggles.conditional_lead:
      self.status_value = CEStatus["LEAD"]
      return True

    if (sm["carState"].leftBlinker or sm["carState"].rightBlinker) and v_ego < frogpilot_toggles.conditional_signal:
      desired_lane = self.frogpilot_planner.lane_width_left if sm["carState"].leftBlinker else self.frogpilot_planner.lane_width_right
      if desired_lane < frogpilot_toggles.lane_detection_width or not frogpilot_toggles.conditional_signal_lane_detection:
        self.status_value = CEStatus["SIGNAL"]
        return True

    below_speed = 1 <= v_ego < frogpilot_toggles.conditional_limit and not self.frogpilot_planner.frogpilot_following.following_lead
    below_speed_with_lead = 1 <= v_ego < frogpilot_toggles.conditional_limit_lead and self.frogpilot_planner.frogpilot_following.following_lead
    if below_speed or below_speed_with_lead:
      self.status_value = CEStatus["SPEED"]
      return True

    if self.frogpilot_planner.frogpilot_vcruise.slc.experimental_mode:
      self.status_value = CEStatus["SPEED_LIMIT"]
      return True

    if self.stop_light_detected and frogpilot_toggles.conditional_model_stop_time != 0:
      self.status_value = CEStatus["STOP_LIGHT"]
      return True

    return False

  def update_conditions(self, v_ego, sm, frogpilot_toggles):
    self._update_filter_time_constants(v_ego)
    self.curve_detection(v_ego, frogpilot_toggles)
    self.slow_lead(v_ego, frogpilot_toggles)
    self.stop_sign_and_light(v_ego, sm, frogpilot_toggles.conditional_model_stop_time)

  def curve_detection(self, v_ego, frogpilot_toggles):
    self.curvature_filter.update(self.frogpilot_planner.driving_in_curve or self.frogpilot_planner.road_curvature_detected)
    self.curve_detected = self.curvature_filter.x >= THRESHOLD and v_ego > CRUISING_SPEED

  def slow_lead(self, v_ego, frogpilot_toggles):
    if self.frogpilot_planner.tracking_lead:
      slower_lead = (v_ego - self.frogpilot_planner.lead_one.vLead) > CRUISING_SPEED and frogpilot_toggles.conditional_slower_lead
      slower_lead |= getattr(self.frogpilot_planner.frogpilot_following, "slower_lead", False) and frogpilot_toggles.conditional_slower_lead
      stopped_lead = self.frogpilot_planner.lead_one.vLead < 1 and frogpilot_toggles.conditional_stopped_lead

      self.slow_lead_filter.update(slower_lead or stopped_lead)
      lead_prob = getattr(self.frogpilot_planner.lead_one, 'modelProb', 1.0)
      adjusted_threshold = THRESHOLD * (1.0 + 0.2 * (1.0 - lead_prob))
      self.slow_lead_detected = self.slow_lead_filter.x >= adjusted_threshold
    else:
      self.slow_lead_filter.x = 0
      self.slow_lead_detected = False

  def stop_sign_and_light(self, v_ego, sm, model_time):
    if sm["frogpilotCarState"].trafficModeEnabled:
      self.stop_light_filter.x = 0
      self.stop_light_detected = False
      return

    speed_mph = v_ego * CV.MS_TO_MPH
    if speed_mph > 75:
      self.stop_light_filter.x = 0
      self.stop_light_detected = False
      return

    light_boost = float(np.interp(speed_mph, [0, 35, 55, 70], self.LIGHT_BOOSTS))
    cap_factor = float(np.interp(speed_mph, [0, 35, 45], [0.0, 0.0, 1.0]))
    adjusted_model_time = model_time * light_boost
    if cap_factor > 0:
      adjusted_model_time = min(adjusted_model_time, self.LIGHT_MAX_TIME * cap_factor + model_time * (1.0 - cap_factor))

    model_stopping = self.frogpilot_planner.model_length < v_ego * adjusted_model_time
    self.stop_light_filter.update(self.frogpilot_planner.model_stopped or model_stopping)
    self.stop_light_detected = self.stop_light_filter.x >= (THRESHOLD ** 2) and not self.frogpilot_planner.tracking_lead
