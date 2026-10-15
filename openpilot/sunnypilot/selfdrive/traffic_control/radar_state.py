"""Independent Traffic Radar producer state."""

from __future__ import annotations

from enum import IntEnum

import openpilot.cereal.messaging as messaging
from openpilot.sunnypilot.selfdrive.traffic_control.controller import (
  TeslaTrafficControlController,
  TrafficControlConfig,
  TrafficControlMode,
  TrafficControlPhase,
)
from openpilot.sunnypilot.selfdrive.traffic_control.tesla_observer import (
  TRAFFIC_CONTROL_STALE_NS,
  TeslaTrafficControlObservation,
)


class TrafficRadarGoPolicy(IntEnum):
  passive = 0
  active = 1


TRANSITION_REASON_CODES = {
  "": 0,
  "stop_confirmed": 1,
  "driver_bypass": 2,
  # Values 3 and 4 were used by the former radar/lead gate and remain
  # reserved so historical eventTransitionReason logs keep their meaning.
  "radar_invalid": 3,
  "lead_present": 4,
  "observation_dropout": 5,
  "stationary_hold": 6,
  "green_release": 7,
  "candidate_started": 8,
  "candidate_replaced": 9,
  "candidate_cancelled": 10,
  "speed_above_limit": 11,
  "flashing_green_stop": 12,
  "green_flash_candidate": 13,
  "yellow_stop": 14,
  "yellow_pass": 15,
  "distance_wrap_passed": 16,
  "red_after_release": 17,
  "signal_lost_release": 18,
  "dropout_reconfirmed": 19,
}


class TrafficRadarSource:
  """Publish a traffic-control target independently from physical radarState."""

  def __init__(self, config: TrafficControlConfig,
               go_policy: TrafficRadarGoPolicy = TrafficRadarGoPolicy.passive) -> None:
    self.controller = TeslaTrafficControlController(config)
    self.go_policy = go_policy

  @staticmethod
  def _observation(msg, now_ns: int) -> TeslaTrafficControlObservation:
    observation = TeslaTrafficControlObservation.from_message(msg)
    age_ns = now_ns - observation.frame_mono_time
    return observation if 0 <= age_ns <= TRAFFIC_CONTROL_STALE_NS else TeslaTrafficControlObservation()

  def update(self, sm, now_ns: int):
    car_state_sp_valid = bool(sm.seen['carStateSP'] and sm.alive['carStateSP'] and sm.valid['carStateSP'])
    car_state_valid = bool(sm.seen['carState'] and sm.alive['carState'] and sm.valid['carState'])
    car_control_valid = bool(sm.seen['carControl'] and sm.alive['carControl'] and sm.valid['carControl'])
    vehicle_inputs_valid = car_state_valid and car_control_valid
    raw_traffic = sm['carStateSP'].teslaTrafficControl if car_state_sp_valid else None
    raw_frame_mono_time = int(raw_traffic.frameMonoTime) if raw_traffic is not None else 0
    raw_distance = float(raw_traffic.distance) if raw_traffic is not None else 255.0
    observation = (self._observation(raw_traffic, now_ns)
                   if raw_traffic is not None else TeslaTrafficControlObservation())
    car_state = sm['carState']
    car_control = sm['carControl']
    # Tesla selects the signal for the vehicle's current lane before publishing
    # 0x25D. Turn indicators and model manoeuvre intent therefore cannot veto
    # the authoritative lane signal.
    direction_unknown = False
    decision = self.controller.update(
      observation, now_ns, v_ego=float(car_state.vEgo), a_ego=float(car_state.aEgo),
      enabled=bool(vehicle_inputs_valid and car_control.enabled),
      long_active=bool(vehicle_inputs_valid and car_control.longActive), gas_pressed=bool(car_state.gasPressed),
      brake_pressed=bool(car_state.brakePressed),
      turn_signal_active=direction_unknown,
      stop_direction_unknown=direction_unknown,
    )

    active_stop = decision.phase in (
      TrafficControlPhase.approachRed, TrafficControlPhase.braking, TrafficControlPhase.hold,
      TrafficControlPhase.flashingGreenStop, TrafficControlPhase.yellowStop,
    )
    # Traffic-light control is independent of physical or vision lead sensing.
    # Only the selected base longitudinal planner interprets lead inputs.
    valid_for_control = bool(decision.apply_constraint)
    stop_valid_for_control = bool(decision.stop_control_allowed)
    msg = messaging.new_message('trafficRadarState')
    target = msg.trafficRadarState
    target.targetPresent = bool(active_stop)
    # The confirmed STOP owns geometry even when the current raw frame is
    # explicit OFF (quality zero). Preserve its reference without promoting
    # that raw frame to an independently eligible traffic instruction.
    target.oemTargetDistance = float(
      decision.remaining_distance + decision.stop_reference
      if (active_stop or decision.quality > 0) and 0.0 <= raw_distance <= self.controller.config.max_control_distance
      else 0.0
    )
    target.targetRelativeVelocity = -float(car_state.vEgo) if active_stop else 0.0
    target.targetRelativeAcceleration = -float(car_state.aEgo) if active_stop else 0.0
    target.distanceToStopPoint = float(decision.remaining_distance)
    target.phase = int(decision.phase)
    target.lightState = int(decision.light_state)
    target.sourceBus = int(decision.source_bus)
    target.quality = int(decision.quality)
    target.confidence = 1.0 if active_stop else 0.0
    target.eventId = int(self.controller.event_id)
    target.stopSessionId = int(decision.stop_session_id)
    target.directionUnknown = decision.direction_unknown
    target.driverOverrideActive = decision.driver_override_active
    target.canRemaining = float(decision.can_remaining)
    target.stationInnovation = float(decision.station_innovation)
    target.publishMonoTime = int(now_ns)
    target.controlAllowed = valid_for_control
    target.stopControlAllowed = stop_valid_for_control
    target.stopSafetyAllowed = decision.stop_safety_allowed
    target.rawObservationFresh = decision.raw_observation_fresh
    target.stopDirectionUnknown = decision.stop_direction_unknown
    target.suppressedByPhysicalLead = False
    target.shouldStop = bool(decision.should_stop)
    raw_green_seen = bool(raw_traffic is not None and raw_traffic.available and raw_traffic.lightState == 2)
    target.plannerStartRequested = bool(
      self.go_policy == TrafficRadarGoPolicy.active and
      decision.mode == TrafficControlMode.stopGo and
      decision.phase == TrafficControlPhase.release and
      decision.raw_observation_fresh and raw_green_seen and
      valid_for_control
    )
    target.mode = int(decision.mode)
    target.rawGreenSeen = raw_green_seen
    target.releaseEligible = bool(
      target.rawGreenSeen and decision.phase == TrafficControlPhase.release and valid_for_control
    )
    target.eventContinuous = self.controller.event_continuous
    target.eventTransitionReason = TRANSITION_REASON_CODES.get(self.controller.transition_reason, 255)
    target.eventTransitionSeq = self.controller.transition_seq
    target.rawDistance = raw_distance
    target.observationAgeMs = float(
      max(0, now_ns - raw_frame_mono_time) / 1e6 if raw_frame_mono_time > 0 else 1.0e9
    )
    msg.valid = bool(car_state_sp_valid and vehicle_inputs_valid)
    return msg
