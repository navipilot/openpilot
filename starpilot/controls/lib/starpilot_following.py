#!/usr/bin/env python3
import numpy as np

from openpilot.common.constants import CV
from openpilot.selfdrive.controls.lib.lead_behavior import should_disable_far_lead_throttle
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import COMFORT_BRAKE, LEAD_DANGER_FACTOR, desired_follow_distance, get_jerk_factor, get_T_FOLLOW

from openpilot.starpilot.common.starpilot_variables import CITY_SPEED_LIMIT, MAX_T_FOLLOW

TRAFFIC_MODE_BP = [0., CITY_SPEED_LIMIT]
PERSONALITY_BP = [45. * CV.MPH_TO_MS, 70. * CV.MPH_TO_MS]


def get_longitudinal_personality(sm):
  return sm["selfdriveState"].personality

class StarPilotFollowing:
  def __init__(self, StarPilotPlanner):
    self.starpilot_planner = StarPilotPlanner

    self.disable_throttle = False
    self.following_lead = False
    self.slower_lead = False

    self.acceleration_jerk = 0
    self.danger_jerk = 0
    self.desired_follow_distance = 0
    self.speed_jerk = 0
    self.t_follow = 0

  def update(self, long_control_active, v_ego, sm, starpilot_toggles):
    personality = get_longitudinal_personality(sm)

    if long_control_active and sm["starpilotCarState"].trafficModeEnabled:
      if sm["carState"].aEgo >= 0:
        self.base_acceleration_jerk = np.interp(v_ego, TRAFFIC_MODE_BP, starpilot_toggles.traffic_mode_jerk_acceleration)
        self.base_speed_jerk = np.interp(v_ego, TRAFFIC_MODE_BP, starpilot_toggles.traffic_mode_jerk_speed)
      else:
        self.base_acceleration_jerk = np.interp(v_ego, TRAFFIC_MODE_BP, starpilot_toggles.traffic_mode_jerk_deceleration)
        self.base_speed_jerk = np.interp(v_ego, TRAFFIC_MODE_BP, starpilot_toggles.traffic_mode_jerk_speed_decrease)

      self.base_danger_jerk = np.interp(v_ego, TRAFFIC_MODE_BP, starpilot_toggles.traffic_mode_jerk_danger)
      self.t_follow = np.interp(v_ego, TRAFFIC_MODE_BP, starpilot_toggles.traffic_mode_follow)
    elif long_control_active:
      if sm["carState"].aEgo >= 0:
        self.base_acceleration_jerk, self.base_danger_jerk, self.base_speed_jerk = get_jerk_factor(
          starpilot_toggles.aggressive_jerk_acceleration, starpilot_toggles.aggressive_jerk_danger, starpilot_toggles.aggressive_jerk_speed,
          starpilot_toggles.standard_jerk_acceleration, starpilot_toggles.standard_jerk_danger, starpilot_toggles.standard_jerk_speed,
          starpilot_toggles.relaxed_jerk_acceleration, starpilot_toggles.relaxed_jerk_danger, starpilot_toggles.relaxed_jerk_speed,
          starpilot_toggles.custom_personalities, personality
        )
      else:
        self.base_acceleration_jerk, self.base_danger_jerk, self.base_speed_jerk = get_jerk_factor(
          starpilot_toggles.aggressive_jerk_deceleration, starpilot_toggles.aggressive_jerk_danger, starpilot_toggles.aggressive_jerk_speed_decrease,
          starpilot_toggles.standard_jerk_deceleration, starpilot_toggles.standard_jerk_danger, starpilot_toggles.standard_jerk_speed_decrease,
          starpilot_toggles.relaxed_jerk_deceleration, starpilot_toggles.relaxed_jerk_danger, starpilot_toggles.relaxed_jerk_speed_decrease,
          starpilot_toggles.custom_personalities, personality
        )

      self.t_follow = get_T_FOLLOW(
        starpilot_toggles.aggressive_follow,
        starpilot_toggles.standard_follow,
        starpilot_toggles.relaxed_follow,
        starpilot_toggles.custom_personalities, personality
      )
      if isinstance(self.t_follow, (list, tuple)):
        self.t_follow = float(np.interp(v_ego, PERSONALITY_BP, self.t_follow))
      else:
        self.t_follow = float(self.t_follow)
    else:
      self.base_acceleration_jerk = 0
      self.base_danger_jerk = 0
      self.base_speed_jerk = 0
      self.t_follow = 0

    self.acceleration_jerk = self.base_acceleration_jerk
    self.danger_factor = LEAD_DANGER_FACTOR
    self.danger_jerk = self.base_danger_jerk
    self.speed_jerk = self.base_speed_jerk

    self.following_lead = self.starpilot_planner.tracking_lead and self.starpilot_planner.lead_one.dRel < (self.t_follow * 2) * v_ego
    self.slower_lead = False

    if self.starpilot_planner.starpilot_weather.weather_id != 0:
      self.t_follow = min(self.t_follow + self.starpilot_planner.starpilot_weather.increase_following_distance, MAX_T_FOLLOW)

    self.disable_throttle = False
    if self.starpilot_planner.tracking_lead and self.starpilot_planner.lead_one.status:
      lead_distance = self.starpilot_planner.lead_one.dRel
      v_lead = self.starpilot_planner.lead_one.vLead
      closing_speed = max(0.0, v_ego - v_lead)
      desired_gap = float(desired_follow_distance(v_ego, v_lead, self.t_follow))
      self.disable_throttle = should_disable_far_lead_throttle(v_ego, lead_distance, desired_gap, closing_speed, self.following_lead)

    if long_control_active and self.starpilot_planner.tracking_lead:
      self.update_follow_values(self.starpilot_planner.lead_one.dRel, v_ego, self.starpilot_planner.lead_one.vLead, starpilot_toggles)
      self.desired_follow_distance = int(desired_follow_distance(v_ego, self.starpilot_planner.lead_one.vLead, self.t_follow))
    else:
      self.desired_follow_distance = 0

  def update_follow_values(self, lead_distance, v_ego, v_lead, starpilot_toggles):
    if starpilot_toggles.human_following and v_lead > v_ego:
      distance_factor = max(lead_distance - (v_ego * self.t_follow), 1)
      from starpilot.common.starpilot_variables import get_starpilot_toggles
      fp_toggles = get_starpilot_toggles()
      accelerating_offset = float(np.clip(fp_toggles.stop_distance - v_ego, 1, distance_factor))

      self.acceleration_jerk /= accelerating_offset
      self.speed_jerk /= accelerating_offset
      self.t_follow /= accelerating_offset

    if (starpilot_toggles.conditional_slower_lead or starpilot_toggles.human_following) and v_lead < v_ego:
      distance_factor = max(lead_distance - (v_lead * self.t_follow), 1)
      braking_offset = float(np.clip(min(v_ego - v_lead, v_lead) - COMFORT_BRAKE, 1, distance_factor))

      if starpilot_toggles.human_following:
        from starpilot.common.starpilot_variables import get_starpilot_toggles
        fp_toggles = get_starpilot_toggles()
        far_lead_offset = max(lead_distance - (v_ego * self.t_follow) - fp_toggles.stop_distance, 0)
        self.t_follow /= braking_offset + far_lead_offset
      self.slower_lead = braking_offset > 1
