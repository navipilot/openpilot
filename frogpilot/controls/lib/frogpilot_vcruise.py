#!/usr/bin/env python3
from openpilot.common.constants import CV
from openpilot.common.realtime import DT_MDL

from openpilot.frogpilot.common.frogpilot_variables import CRUISING_SPEED, PLANNER_TIME
from openpilot.frogpilot.controls.lib.curve_speed_controller import CurveSpeedController
from openpilot.frogpilot.controls.lib.speed_limit_controller import SpeedLimitController

OVERRIDE_FORCE_STOP_TIMER = 10
PCM_STALK_STEP = 5 * CV.KPH_TO_MS  # Tesla stalk always moves in exactly 5 kph increments

class FrogPilotVCruise:
  def __init__(self, FrogPilotPlanner):
    self.frogpilot_planner = FrogPilotPlanner

    self.csc = CurveSpeedController(self)
    self.slc = SpeedLimitController(self)

    self.forcing_stop = False
    self.override_force_stop = False

    self.force_stop_timer = 0
    self.override_force_stop_timer = 0
    self.tracked_model_length = 0

    self.pcm_autonomous_drop = False  # True when Tesla firmware dropped cruise via sign reading
    self.prev_v_cruise = 0.0

  def update(self, long_control_active, now, time_validated, v_cruise, v_ego, sm, frogpilot_toggles):
    force_stop = self.frogpilot_planner.frogpilot_cem.stop_light_detected and long_control_active and frogpilot_toggles.force_stops
    force_stop &= self.frogpilot_planner.model_stopped
    force_stop &= self.override_force_stop_timer <= 0

    self.force_stop_timer = self.force_stop_timer + DT_MDL if force_stop else 0

    force_stop_enabled = self.force_stop_timer >= 1

    self.override_force_stop |= sm["carState"].gasPressed
    self.override_force_stop |= sm["frogpilotCarState"].accelPressed
    self.override_force_stop &= force_stop_enabled

    if self.override_force_stop:
      self.override_force_stop_timer = OVERRIDE_FORCE_STOP_TIMER
    elif self.override_force_stop_timer > 0:
      self.override_force_stop_timer -= DT_MDL

    v_cruise_cluster = max(sm["carState"].vCruiseCluster * CV.KPH_TO_MS, v_cruise)
    v_cruise_diff = v_cruise_cluster - v_cruise

    v_ego_cluster = max(sm["carState"].vEgoCluster, v_ego)
    v_ego_diff = v_ego_cluster - v_ego

    # FrogsGoMoo's Curve Speed Controller
    if long_control_active and v_ego > CRUISING_SPEED and self.frogpilot_planner.road_curvature_detected and frogpilot_toggles.curve_speed_controller:
      self.csc.update_target(v_ego)

      self.csc_controlling_speed = True

      self.csc_target = self.csc.target
    else:
      self.csc.log_data(long_control_active, v_ego, sm)

      self.csc_controlling_speed = False
      self.csc.target_set = False

      self.csc_target = v_cruise

    # Pfeiferj's Speed Limit Controller
    self.slc.frogpilot_toggles = frogpilot_toggles

    if frogpilot_toggles.speed_limit_controller:
      self.slc.update_limits(sm["frogpilotCarState"].dashboardSpeedLimit, now, time_validated, v_cruise, v_ego, sm)
      self.slc.update_override(v_cruise, v_cruise_diff, v_ego, v_ego_diff, sm)

      self.slc_offset = self.slc.offset
      self.slc_target = self.slc.target
    elif frogpilot_toggles.show_speed_limits:
      self.slc.update_limits(sm["frogpilotCarState"].dashboardSpeedLimit, now, time_validated, v_cruise, v_ego, sm)

      self.slc_offset = 0
      self.slc_target = self.slc.target
    else:
      self.slc_offset = 0
      self.slc_target = 0

    # Detect Tesla PCM autonomously dropping cruise via speed sign reading.
    # Stalk presses always arrive as exactly ±5 kph steps; larger single-frame drops
    # are the Tesla firmware reacting to a sign (e.g. ramp advisory 50/40 kph).
    # Only check when long active to avoid false positives on cruise engagement
    # (V_CRUISE_UNSET → V_CRUISE_INITIAL is a large drop that would otherwise trigger).
    v_cruise_dropped = self.prev_v_cruise - v_cruise
    if long_control_active and v_cruise_dropped > PCM_STALK_STEP and not sm["carState"].gasPressed and self.slc_target > v_cruise:
      self.pcm_autonomous_drop = True
    elif sm["carState"].gasPressed or v_cruise > self.prev_v_cruise:
      self.pcm_autonomous_drop = False
    self.prev_v_cruise = v_cruise

    if force_stop_enabled and not self.override_force_stop:
      self.forcing_stop |= not sm["carState"].standstill

      self.tracked_model_length = max(self.tracked_model_length - (v_ego * DT_MDL), 0)
      v_cruise = min((self.tracked_model_length // PLANNER_TIME), v_cruise)

    else:
      self.forcing_stop = False

      self.tracked_model_length = self.frogpilot_planner.model_length

      targets = [self.csc_target]
      if frogpilot_toggles.speed_limit_controller and self.slc_target > 0:
        slc_speed = max(self.slc.overridden_speed, self.slc_target + self.slc_offset) - v_ego_diff
        targets.append(slc_speed)
      else:
        targets.append(v_cruise)
      v_cruise = min([target if target >= CRUISING_SPEED else v_cruise for target in targets])

      # If Tesla firmware autonomously dropped cruise (sign reading) and CSC is not
      # actively managing speed for a curve, restore to the SLC floor so FrogPilot
      # maintains the map-validated speed rather than following the sign misread.
      if self.pcm_autonomous_drop and self.slc_target > 0 and not self.csc_controlling_speed:
        slc_floor = max(self.slc.overridden_speed, self.slc_target + self.slc_offset) - v_ego_diff
        v_cruise = max(v_cruise, slc_floor)

    return v_cruise
