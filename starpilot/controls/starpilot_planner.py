#!/usr/bin/env python3
import json

import cereal.messaging as messaging

from openpilot.common.constants import CV
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.gps import get_gps_location_service
from openpilot.common.params import Params
from openpilot.common.realtime import DT_MDL
from openpilot.selfdrive.car.cruise import V_CRUISE_MAX, V_CRUISE_UNSET
from openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.long_mpc import A_CHANGE_COST, DANGER_ZONE_COST, J_EGO_COST, STOP_DISTANCE

from openpilot.starpilot.common.starpilot_utilities import calculate_lane_width, calculate_road_curvature
from openpilot.starpilot.common.starpilot_variables import CRUISING_SPEED, MINIMUM_LATERAL_ACCELERATION, PLANNER_TIME, THRESHOLD
from openpilot.starpilot.controls.lib.conditional_experimental_mode import ConditionalExperimentalMode
from openpilot.starpilot.controls.lib.starpilot_acceleration import StarPilotAcceleration
from openpilot.starpilot.controls.lib.starpilot_events import StarPilotEvents
from openpilot.starpilot.controls.lib.starpilot_following import StarPilotFollowing
from openpilot.starpilot.controls.lib.starpilot_vcruise import StarPilotVCruise
from openpilot.starpilot.controls.lib.weather_checker import WeatherChecker


class StarPilotPlanner:
  def __init__(self, error_log, ThemeManager):
    self.params = Params(return_defaults=True)
    self.params_memory = Params(memory=True)

    self.starpilot_acceleration = StarPilotAcceleration(self)
    self.starpilot_cem = ConditionalExperimentalMode(self)
    self.starpilot_events = StarPilotEvents(self, error_log, ThemeManager)
    self.starpilot_following = StarPilotFollowing(self)
    self.starpilot_vcruise = StarPilotVCruise(self)
    self.starpilot_weather = WeatherChecker(self)

    self.driving_in_curve = False
    self.gps_valid = False
    self.lateral_check = False
    self.model_stopped = False
    self.road_curvature_detected = False
    self.tracking_lead = False

    self.lane_width_left = 0
    self.lane_width_right = 0
    self.lateral_acceleration = 0
    self.model_length = 0
    self.road_curvature = 0
    self.time_to_curve = 0
    self.v_cruise = 0

    self.gps_position = None

    self.gps_location_service = get_gps_location_service(self.params)

    self.tracking_lead_filter = FirstOrderFilter(0, 0.5, DT_MDL)

  def shutdown(self):
    self.starpilot_vcruise.slc.shutdown()
    self.starpilot_weather.executor.shutdown(wait=False, cancel_futures=True)

  def update(self, now, time_validated, sm, starpilot_toggles):
    self.lead_one = sm["radarState"].leadOne

    controls_enabled = sm["selfdriveState"].enabled

    v_cruise_kph = min(sm["carState"].vCruise, V_CRUISE_MAX)
    if 0 < v_cruise_kph < V_CRUISE_UNSET and starpilot_toggles.set_speed_offset > 0:
      v_cruise_kph += starpilot_toggles.set_speed_offset
    v_cruise = v_cruise_kph * CV.KPH_TO_MS
    v_ego = max(sm["carState"].vEgo, 0)

    if controls_enabled:
      self.starpilot_acceleration.update(v_ego, sm, starpilot_toggles)
    else:
      self.starpilot_acceleration.max_accel = 0
      self.starpilot_acceleration.min_accel = 0

    if controls_enabled and starpilot_toggles.conditional_experimental_mode:
      self.starpilot_cem.update(v_ego, sm, starpilot_toggles)
    else:
      self.starpilot_cem.curve_detected = False
      self.starpilot_cem.stop_sign_and_light(v_ego, sm, PLANNER_TIME - 2)

    self.driving_in_curve = abs(self.lateral_acceleration) >= MINIMUM_LATERAL_ACCELERATION

    self.starpilot_events.update(controls_enabled, v_cruise, sm, starpilot_toggles)

    self.starpilot_following.update(controls_enabled, v_ego, sm, starpilot_toggles)

    gps_location = sm[self.gps_location_service]
    self.gps_position = {
      "latitude": gps_location.latitude,
      "longitude": gps_location.longitude,
      "bearing": gps_location.bearingDeg,
    }
    self.gps_valid = self.gps_position["latitude"] != 0 or self.gps_position["longitude"] != 0
    self.params_memory.put("LastGPSPosition", json.dumps(self.gps_position))

    if v_ego >= starpilot_toggles.minimum_lane_change_speed:
      self.lane_width_left = calculate_lane_width(sm["modelV2"].laneLines[0], sm["modelV2"].laneLines[1], sm["modelV2"].roadEdges[0])
      self.lane_width_right = calculate_lane_width(sm["modelV2"].laneLines[3], sm["modelV2"].laneLines[2], sm["modelV2"].roadEdges[1])
    else:
      self.lane_width_left = 0
      self.lane_width_right = 0

    self.lateral_acceleration = v_ego**2 * sm["controlsState"].curvature

    self.lateral_check = v_ego >= starpilot_toggles.pause_lateral_below_speed
    self.lateral_check |= not (sm["carState"].leftBlinker or sm["carState"].rightBlinker) and starpilot_toggles.pause_lateral_below_signal
    self.lateral_check |= sm["carState"].standstill
    self.lateral_check &= not sm["starpilotCarState"].pauseLateral

    self.model_length = sm["modelV2"].position.x[-1]

    self.model_stopped = self.model_length < CRUISING_SPEED * PLANNER_TIME
    self.model_stopped |= self.starpilot_vcruise.forcing_stop

    self.road_curvature, self.time_to_curve = calculate_road_curvature(sm["modelV2"], v_ego)

    self.road_curvature_detected = (1 / abs(self.road_curvature))**0.5 < v_ego > CRUISING_SPEED and not (sm["carState"].leftBlinker or sm["carState"].rightBlinker)

    if not sm["carState"].standstill:
      self.tracking_lead = self.update_lead_status(starpilot_toggles.stop_distance)

    self.v_cruise = self.starpilot_vcruise.update(controls_enabled, now, time_validated, v_cruise, v_ego, sm, starpilot_toggles)

    if self.gps_valid and time_validated and starpilot_toggles.weather_presets:
      self.starpilot_weather.update_weather(now, starpilot_toggles)
    else:
      self.starpilot_weather.weather_id = 0

  def update_lead_status(self, stop_distance=STOP_DISTANCE):
    following_lead = self.lead_one.status
    following_lead &= self.lead_one.dRel < self.model_length + float(stop_distance)

    self.tracking_lead_filter.update(following_lead)
    return self.tracking_lead_filter.x >= THRESHOLD

  def publish(self, theme_updated, sm, pm, starpilot_toggles):
    starpilot_plan_send = messaging.new_message("starpilotPlan")
    starpilot_plan_send.valid = sm.all_checks(service_list=["carState", "controlsState", "selfdriveState", "radarState"])
    starpilotPlan = starpilot_plan_send.starpilotPlan

    starpilotPlan.accelerationJerk = float(A_CHANGE_COST * self.starpilot_following.acceleration_jerk)
    starpilotPlan.dangerFactor = float(self.starpilot_following.danger_factor)
    starpilotPlan.dangerJerk = float(DANGER_ZONE_COST * self.starpilot_following.danger_jerk)
    starpilotPlan.speedJerk = float(J_EGO_COST * self.starpilot_following.speed_jerk)
    starpilotPlan.tFollow = float(self.starpilot_following.t_follow)

    starpilotPlan.cscControllingSpeed = self.starpilot_vcruise.csc_controlling_speed
    starpilotPlan.cscSpeed = float(self.starpilot_vcruise.csc_target)
    starpilotPlan.cscTraining = self.starpilot_vcruise.csc.enable_training

    starpilotPlan.desiredFollowDistance = int(self.starpilot_following.desired_follow_distance)
    starpilotPlan.disableThrottle = self.starpilot_following.disable_throttle
    starpilotPlan.trackingLead = self.tracking_lead

    starpilotPlan.experimentalMode = self.starpilot_cem.experimental_mode or self.starpilot_vcruise.slc.experimental_mode

    starpilotPlan.forcingStop = self.starpilot_vcruise.forcing_stop
    starpilotPlan.forcingStopLength = self.starpilot_vcruise.tracked_model_length

    starpilotPlan.starpilotEvents = self.starpilot_events.events.to_msg()

    starpilotPlan.starpilotToggles = json.dumps(vars(starpilot_toggles))

    if sm["starpilotCarState"].trafficModeEnabled:
      starpilotPlan.increasedStoppedDistance = 0
    else:
      starpilotPlan.increasedStoppedDistance = starpilot_toggles.increase_stopped_distance
      if self.starpilot_weather.weather_id != 0:
        starpilotPlan.increasedStoppedDistance += self.starpilot_weather.increase_stopped_distance

    starpilotPlan.laneWidthLeft = self.lane_width_left
    starpilotPlan.laneWidthRight = self.lane_width_right

    starpilotPlan.lateralCheck = self.lateral_check

    starpilotPlan.maxAcceleration = float(self.starpilot_acceleration.max_accel)
    starpilotPlan.minAcceleration = float(self.starpilot_acceleration.min_accel)

    starpilotPlan.redLight = self.starpilot_cem.stop_light_detected

    starpilotPlan.roadCurvature = self.road_curvature

    starpilotPlan.slcMapSpeedLimit = self.starpilot_vcruise.slc.map_speed_limit
    starpilotPlan.slcMapboxSpeedLimit = self.starpilot_vcruise.slc.mapbox_limit
    starpilotPlan.slcNextSpeedLimit = self.starpilot_vcruise.slc.next_speed_limit
    starpilotPlan.slcOverriddenSpeed = self.starpilot_vcruise.slc.overridden_speed
    starpilotPlan.slcSpeedLimit = self.starpilot_vcruise.slc_target
    starpilotPlan.slcSpeedLimitOffset = self.starpilot_vcruise.slc_offset
    starpilotPlan.slcSpeedLimitSource = self.starpilot_vcruise.slc.source
    starpilotPlan.speedLimitChanged = self.starpilot_vcruise.slc.speed_limit_changed_timer > DT_MDL
    starpilotPlan.unconfirmedSlcSpeedLimit = self.starpilot_vcruise.slc.unconfirmed_speed_limit

    starpilotPlan.themeUpdated = theme_updated

    starpilotPlan.vCruise = float(self.v_cruise)

    starpilotPlan.weatherDaytime = self.starpilot_weather.is_daytime
    starpilotPlan.weatherId = self.starpilot_weather.weather_id

    pm.send("starpilotPlan", starpilot_plan_send)
