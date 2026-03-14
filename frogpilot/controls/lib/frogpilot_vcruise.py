#!/usr/bin/env python3
from openpilot.common.constants import CV

from openpilot.frogpilot.common.frogpilot_variables import CRUISING_SPEED
from openpilot.frogpilot.controls.lib.curve_speed_controller import CurveSpeedController

class FrogPilotVCruise:
  def __init__(self, FrogPilotPlanner):
    self.frogpilot_planner = FrogPilotPlanner

    self.csc = CurveSpeedController(self)

  def update(self, long_control_active, now, time_validated, v_cruise, v_ego, sm, frogpilot_toggles):
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

    targets = [self.csc_target, v_cruise]
    v_cruise = min([target if target >= CRUISING_SPEED else v_cruise for target in targets])

    return v_cruise
