"""Public behavior tests for the independent Traffic Radar state."""

import openpilot.cereal.messaging as messaging
from types import SimpleNamespace

from openpilot.sunnypilot.selfdrive.traffic_control.controller import TrafficControlConfig, TrafficControlMode
from openpilot.sunnypilot.selfdrive.traffic_control.radar_state import (
  TrafficRadarGoPolicy,
  TrafficRadarSource,
)


def test_traffic_radar_channel_does_not_mutate_radar_state():
  radar_msg = messaging.new_message("radarState")
  traffic_msg = messaging.new_message("trafficRadarState")

  target = traffic_msg.trafficRadarState
  target.targetPresent = True
  target.oemTargetDistance = 42.0
  target.distanceToStopPoint = 36.0

  assert target.targetPresent
  assert target.oemTargetDistance == 42.0
  assert target.distanceToStopPoint == 36.0
  assert not radar_msg.radarState.leadOne.present
  assert not radar_msg.radarState.leadTwo.present


def ns(**kwargs):
  return SimpleNamespace(**kwargs)


def red_light_sm():
  traffic = ns(
    available=True, validForControl=True, sourceBus=2, dlc=6, featureState=3,
    stateMachine=4, controlSource=3, controlType=3, distance=80.0,
    lightState=1, continuationReason=0, confirmationType=0,
    warningSuppressionReason=0, unavailableReason=0, visionLight=True,
    visionSign=False, visionRoadMarking=False, visionLine=False,
    frameMonoTime=0, quality=2,
  )
  messages = {
    "carStateSP": ns(teslaTrafficControl=traffic),
    "carState": ns(vEgo=10.0, aEgo=0.0, gasPressed=False, brakePressed=False),
    "carControl": ns(enabled=True, longActive=True, leftBlinker=False, rightBlinker=False),
    "radarState": ns(leadOne=ns(present=False, dRel=0.0), leadTwo=ns(present=False, dRel=0.0)),
    "modelV2": ns(position=ns(x=[74.0] * 33), velocity=ns(x=[0.0] * 33)),
    "modelDataV2SP": ns(laneTurnDirection=0),
  }

  class FakeSubMaster:
    def __init__(self):
      self.messages = messages
      self.seen = dict.fromkeys(messages, True)
      self.alive = dict.fromkeys(messages, True)
      self.valid = dict.fromkeys(messages, True)

    def __getitem__(self, key):
      return self.messages[key]

  return FakeSubMaster()


def test_confirmed_red_is_published_as_a_separate_radar_like_target():
  sm = red_light_sm()
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))

  message = None
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    message = source.update(sm, now_ns)

  target = message.trafficRadarState
  assert target.targetPresent
  assert target.controlAllowed
  assert target.oemTargetDistance == target.distanceToStopPoint + 5.0
  assert 0.0 < target.distanceToStopPoint <= 75.0
  assert target.eventId > 0
  assert not sm["radarState"].leadOne.present
  assert not sm["radarState"].leadTwo.present


def test_persistent_off_release_publishes_a_known_transition_reason():
  sm = red_light_sm()
  sm["carState"].vEgo = 5.0
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))

  for now_ns, distance in ((100_000_000, 50.0), (600_000_000, 47.5)):
    sm["carStateSP"].teslaTrafficControl.distance = distance
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  sm["carStateSP"].teslaTrafficControl.lightState = 0
  target = None
  for now_ns, distance in ((1_100_000_000, 45.0), (1_600_000_000, 42.5),
                           (2_100_000_000, 40.0), (2_600_000_000, 37.5),
                           (3_100_000_000, 35.0)):
    sm["carStateSP"].teslaTrafficControl.distance = distance
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState

  assert target.phase == 6
  assert target.eventTransitionReason == 18


def test_long_dropout_reconfirmation_publishes_a_new_session_reason():
  sm = red_light_sm()
  sm["carState"].vEgo = 5.0
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))

  for now_ns, distance in ((100_000_000, 50.0), (600_000_000, 47.5)):
    sm["carStateSP"].teslaTrafficControl.distance = distance
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState
  session_id = target.stopSessionId

  sm["carStateSP"].teslaTrafficControl.distance = 45.0
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 3_100_000_000
  first = source.update(sm, 3_100_000_000).trafficRadarState
  sm["carStateSP"].teslaTrafficControl.distance = 42.5
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 3_600_000_000
  second = source.update(sm, 3_600_000_000).trafficRadarState

  assert first.stopSessionId == session_id
  assert second.stopSessionId == session_id + 1
  assert second.eventTransitionReason == 19


def test_red_and_green_control_do_not_depend_on_radar_health():
  sm = red_light_sm()
  for health in (sm.seen, sm.alive, sm.valid):
    health.pop("radarState")
  sm.messages.pop("radarState")
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo),
    go_policy=TrafficRadarGoPolicy.active,
  )

  for now_ns in range(100_000_000, 1_100_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    red = source.update(sm, now_ns).trafficRadarState

  assert red.targetPresent
  assert source.update(sm, 1_100_000_000).valid
  assert red.stopControlAllowed
  assert red.controlAllowed
  assert red.stopSessionId > 0

  sm["carStateSP"].teslaTrafficControl.lightState = 2
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_200_000_000
  source.update(sm, 1_200_000_000)
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_700_000_000
  green = source.update(sm, 1_700_000_000).trafficRadarState

  assert green.releaseEligible
  assert green.plannerStartRequested


def test_message_valid_requires_vehicle_and_control_state_but_not_radar():
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))

  no_radar = red_light_sm()
  for health in (no_radar.seen, no_radar.alive, no_radar.valid):
    health.pop("radarState")
  no_radar.messages.pop("radarState")
  no_radar["carStateSP"].teslaTrafficControl.frameMonoTime = 100_000_000
  assert source.update(no_radar, 100_000_000).valid

  bad_car = red_light_sm()
  bad_car.valid["carState"] = False
  bad_car["carStateSP"].teslaTrafficControl.frameMonoTime = 200_000_000
  assert not source.update(bad_car, 200_000_000).valid

  bad_control = red_light_sm()
  bad_control.alive["carControl"] = False
  bad_control["carStateSP"].teslaTrafficControl.frameMonoTime = 300_000_000
  assert not source.update(bad_control, 300_000_000).valid


def test_a_published_lead_never_changes_traffic_permissions():
  clear = red_light_sm()
  lead = red_light_sm()
  lead["radarState"].leadOne.present = True
  lead["radarState"].leadOne.dRel = 8.0
  clear_source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))
  lead_source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))

  for now_ns in range(100_000_000, 1_100_000_001, 100_000_000):
    clear["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    lead["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    clear_target = clear_source.update(clear, now_ns).trafficRadarState
    lead_target = lead_source.update(lead, now_ns).trafficRadarState

  assert lead_target.targetPresent == clear_target.targetPresent
  assert lead_target.controlAllowed == clear_target.controlAllowed
  assert lead_target.stopControlAllowed == clear_target.stopControlAllowed
  assert not lead_target.suppressedByPhysicalLead


def test_model_turn_direction_does_not_veto_current_lane_can():
  sm = red_light_sm()
  sm["modelDataV2SP"].laneTurnDirection = 1
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState
  assert not target.directionUnknown
  assert target.targetPresent
  assert target.controlAllowed


def test_vehicle_blinker_does_not_veto_current_lane_stop_or_release():
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  sm["carState"].leftBlinker = True
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo),
    go_policy=TrafficRadarGoPolicy.active,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState
  assert not target.stopDirectionUnknown
  assert target.stopControlAllowed
  assert not target.directionUnknown
  assert target.controlAllowed

  sm["carStateSP"].teslaTrafficControl.lightState = 2
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_200_000_000
  source.update(sm, 1_200_000_000)
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_700_000_000
  green_allowed = source.update(sm, 1_700_000_000).trafficRadarState
  assert green_allowed.phase == 6
  assert not green_allowed.directionUnknown
  assert green_allowed.plannerStartRequested

  sm["carState"].leftBlinker = False
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 2_200_000_000
  still_allowed = source.update(sm, 2_200_000_000).trafficRadarState
  assert not still_allowed.directionUnknown
  assert still_allowed.plannerStartRequested


def test_stale_observation_keeps_raw_age_and_distance_only_for_diagnostics():
  sm = red_light_sm()
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 100_000_000

  target = source.update(sm, 400_000_000).trafficRadarState

  assert not target.targetPresent
  assert not target.controlAllowed
  assert target.observationAgeMs == 300.0
  assert target.rawDistance == 80.0


def test_missing_raw_frame_is_reported_as_expired_not_zero_age():
  sm = red_light_sm()
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 0
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))

  target = source.update(sm, 1_000_000_000).trafficRadarState

  assert not target.rawObservationFresh
  assert target.observationAgeMs > 2000.0


def test_a_published_lead_is_left_to_the_base_planner():
  sm = red_light_sm()
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  real_lead = sm["radarState"].leadOne
  real_lead.present = True
  real_lead.dRel = 18.0
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_100_000_000
  target = source.update(sm, 1_100_000_000).trafficRadarState

  assert target.controlAllowed
  assert target.stopControlAllowed
  assert not target.suppressedByPhysicalLead
  assert real_lead.present
  assert real_lead.dRel == 18.0


def test_lead_message_changes_do_not_change_the_red_event():
  sm = red_light_sm()
  source = TrafficRadarSource(TrafficControlConfig(mode=TrafficControlMode.stopGo))
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    confirmed = source.update(sm, now_ns).trafficRadarState
  event_id = confirmed.eventId

  sm["radarState"].leadOne.present = True
  sm["radarState"].leadOne.dRel = 18.0
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_100_000_000
  with_lead = source.update(sm, 1_100_000_000).trafficRadarState
  assert not with_lead.suppressedByPhysicalLead
  assert with_lead.controlAllowed
  assert with_lead.eventId == event_id

  sm["radarState"].leadOne.present = False
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_200_000_000
  reacquired = source.update(sm, 1_200_000_000).trafficRadarState
  assert reacquired.controlAllowed
  assert reacquired.eventId == event_id


def released_green(go_policy: TrafficRadarGoPolicy, *, feature_zero_green: bool = False):
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  sm["modelV2"].position.x = [6.0] * 33
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo), go_policy=go_policy,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  sm["carStateSP"].teslaTrafficControl.lightState = 2
  if feature_zero_green:
    sm["carStateSP"].teslaTrafficControl.validForControl = True
    sm["carStateSP"].teslaTrafficControl.featureState = 0
    sm["carStateSP"].teslaTrafficControl.stateMachine = 6
    sm["carStateSP"].teslaTrafficControl.unavailableReason = 1
  message = None
  for now_ns in range(1_100_000_000, 1_900_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    message = source.update(sm, now_ns)
  return message.trafficRadarState


def test_passive_green_removes_target_without_start_request():
  target = released_green(TrafficRadarGoPolicy.passive)
  assert not target.targetPresent
  assert not target.plannerStartRequested


def test_active_green_requests_longitudinal_start_without_creating_a_target():
  target = released_green(TrafficRadarGoPolicy.active)
  assert not target.targetPresent
  assert target.plannerStartRequested
  assert target.eventId > 0


def test_stale_green_keeps_release_session_without_requesting_start():
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo, stationary_release_s=10.0),
    go_policy=TrafficRadarGoPolicy.active,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  traffic = sm["carStateSP"].teslaTrafficControl
  traffic.lightState = 2
  traffic.frameMonoTime = 1_100_000_000
  source.update(sm, 1_100_000_000)
  traffic.frameMonoTime = 1_600_000_000
  released = source.update(sm, 1_600_000_000).trafficRadarState
  assert released.phase == 6
  assert released.plannerStartRequested

  traffic.available = False
  stale = source.update(sm, 4_000_000_000).trafficRadarState
  assert stale.phase == 6
  assert not stale.rawObservationFresh
  assert not stale.rawGreenSeen
  assert not stale.releaseEligible
  assert not stale.plannerStartRequested

  traffic.available = True
  traffic.frameMonoTime = 4_100_000_000
  recovered = source.update(sm, 4_100_000_000).trafficRadarState
  assert recovered.phase == 0
  assert recovered.stopSessionId == 0
  assert not recovered.plannerStartRequested


def test_stationary_fresh_green_keeps_start_request_past_three_seconds():
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo, stationary_release_s=10.0),
    go_policy=TrafficRadarGoPolicy.active,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  traffic = sm["carStateSP"].teslaTrafficControl
  traffic.lightState = 2
  traffic.frameMonoTime = 1_100_000_000
  source.update(sm, 1_100_000_000)
  traffic.frameMonoTime = 1_600_000_000
  released = source.update(sm, 1_600_000_000).trafficRadarState
  session_id = released.stopSessionId
  assert released.plannerStartRequested

  still_released = released
  for now_ns in range(1_700_000_000, 5_000_000_001, 100_000_000):
    traffic.frameMonoTime = now_ns
    still_released = source.update(sm, now_ns).trafficRadarState
  assert still_released.phase == 6
  assert still_released.stopSessionId == session_id
  assert still_released.plannerStartRequested


def test_two_real_can_green_frames_request_cp_style_start():
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  sm["modelV2"].position.x = [6.0] * 33
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo),
    go_policy=TrafficRadarGoPolicy.active,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  traffic = sm["carStateSP"].teslaTrafficControl
  traffic.validForControl = True
  traffic.featureState = 0
  traffic.stateMachine = 6
  traffic.unavailableReason = 1
  traffic.lightState = 2
  traffic.frameMonoTime = 1_100_000_000

  first = source.update(sm, 1_100_000_000).trafficRadarState
  assert first.phase != 6
  traffic.frameMonoTime = 1_600_000_000
  target = source.update(sm, 1_600_000_000).trafficRadarState

  assert target.phase == 6
  assert target.releaseEligible
  assert target.plannerStartRequested


def test_can_green_request_is_independent_of_a_published_lead():
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  sm["modelV2"].position.x = [6.0] * 33
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo),
    go_policy=TrafficRadarGoPolicy.active,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  sm["radarState"].leadOne.present = True
  sm["radarState"].leadOne.dRel = 8.0
  traffic = sm["carStateSP"].teslaTrafficControl
  traffic.validForControl = True
  traffic.featureState = 0
  traffic.stateMachine = 6
  traffic.unavailableReason = 1
  traffic.lightState = 2
  traffic.frameMonoTime = 1_100_000_000

  first = source.update(sm, 1_100_000_000).trafficRadarState
  assert first.phase != 6
  traffic.frameMonoTime = 1_600_000_000
  target = source.update(sm, 1_600_000_000).trafficRadarState

  assert target.phase == 6
  assert not target.suppressedByPhysicalLead
  assert target.plannerStartRequested


def test_transient_lead_message_does_not_affect_same_event_green_start():
  sm = red_light_sm()
  sm["carState"].vEgo = 0.0
  sm["carStateSP"].teslaTrafficControl.distance = 12.0
  sm["modelV2"].position.x = [6.0] * 33
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo),
    go_policy=TrafficRadarGoPolicy.active,
  )
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    source.update(sm, now_ns)

  sm["radarState"].leadOne.present = True
  sm["radarState"].leadOne.dRel = 25.0
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_050_000_000
  unaffected = source.update(sm, 1_050_000_000).trafficRadarState
  assert not unaffected.suppressedByPhysicalLead
  assert unaffected.controlAllowed

  sm["radarState"].leadOne.present = False
  traffic = sm["carStateSP"].teslaTrafficControl
  traffic.lightState = 2
  traffic.frameMonoTime = 1_100_000_000
  first = source.update(sm, 1_100_000_000).trafficRadarState
  assert first.phase != 6
  traffic.frameMonoTime = 1_600_000_000
  released = source.update(sm, 1_600_000_000).trafficRadarState

  assert released.phase == 6
  assert not released.suppressedByPhysicalLead
  assert released.releaseEligible
  assert released.plannerStartRequested


def test_feature_zero_green_requests_start_only_after_the_same_event_holds():
  target = released_green(TrafficRadarGoPolicy.active, feature_zero_green=True)

  assert not target.targetPresent
  assert target.plannerStartRequested
  assert target.eventId > 0
  assert target.rawGreenSeen
  assert target.releaseEligible
  assert target.eventTransitionReason > 0


def test_feature_zero_green_without_a_held_red_event_never_requests_start():
  sm = red_light_sm()
  traffic = sm["carStateSP"].teslaTrafficControl
  traffic.validForControl = False
  traffic.featureState = 0
  traffic.stateMachine = 6
  traffic.unavailableReason = 1
  traffic.lightState = 2
  traffic.distance = 12.0
  sm["carState"].vEgo = 0.0
  source = TrafficRadarSource(
    TrafficControlConfig(mode=TrafficControlMode.stopGo),
    go_policy=TrafficRadarGoPolicy.active,
  )

  target = None
  for now_ns in range(0, 1_000_000_001, 100_000_000):
    traffic.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState

  assert not target.targetPresent
  assert not target.plannerStartRequested
  assert target.eventId == 0
  assert target.rawGreenSeen
  assert not target.releaseEligible
