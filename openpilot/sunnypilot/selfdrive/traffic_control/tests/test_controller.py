import json
import math
from pathlib import Path

from openpilot.sunnypilot.selfdrive.traffic_control.controller import (
  TeslaTrafficControlController,
  TrafficControlConfig,
  TrafficControlMode,
  TrafficControlPhase,
)
from openpilot.sunnypilot.selfdrive.traffic_control.tesla_observer import TeslaTrafficControlObservation


def observation(distance=80.0, light=1, now_s=1.0, *, available=True, bus=2,
                control_type=3, dlc=6, feature_state=0, state_machine=0):
  return TeslaTrafficControlObservation(
    available=available,
    valid_for_control=available and 0.0 <= distance <= 200.0,
    source_bus=bus,
    dlc=dlc,
    feature_state=feature_state,
    state_machine=state_machine,
    control_source=3,
    control_type=control_type,
    distance=distance,
    light_state=light,
    frame_mono_time=int(now_s * 1e9),
    quality=2,
  )


def update(controller, now_s, obs, *, v_ego=10.0,
           brake=False, gas=False, blinker=False, enabled=True, long_active=True,
           model_distance=None, model_candidate=False):
  del model_distance, model_candidate
  return controller.update(
    obs, int(now_s * 1e9), v_ego=v_ego, a_ego=0.0,
    enabled=enabled, long_active=long_active,
    gas_pressed=gas, brake_pressed=brake, turn_signal_active=blinker,
  )


def controller(mode=TrafficControlMode.stopGo, **kwargs):
  return TeslaTrafficControlController(TrafficControlConfig(mode=mode, **kwargs))


def establish_red(c, *, distance=80.0, speed=10.0, start=1.0):
  first = update(c, start, observation(distance, 1, start), v_ego=speed)
  second = update(c, start + 0.5, observation(distance - speed * 0.5, 1, start + 0.5), v_ego=speed)
  return first, second


def test_red_requires_two_distinct_real_frames():
  c = controller()
  first = update(c, 1.0, observation(80.0, 1, 1.0))
  duplicate = update(c, 1.2, observation(80.0, 1, 1.0))
  confirmed = update(c, 1.5, observation(75.0, 1, 1.5))
  assert first.phase == TrafficControlPhase.redCandidate
  assert duplicate.phase == TrafficControlPhase.redCandidate
  assert confirmed.phase in (TrafficControlPhase.approachRed, TrafficControlPhase.braking)
  assert confirmed.apply_constraint


def test_two_hz_high_speed_red_candidate_compensates_ego_motion():
  c = controller()
  first = update(c, 1.0, observation(80.0, 1, 1.0), v_ego=20.0)
  second = update(c, 1.5, observation(70.0, 1, 1.5), v_ego=20.0)
  assert first.phase == TrafficControlPhase.redCandidate
  assert second.phase in c.ACTIVE_PHASES
  assert c.event_id == 1


def test_far_low_urgency_red_requires_half_second_continuous_evidence():
  c = controller()
  first = update(c, 1.00, observation(199.0, 1, 1.00), v_ego=13.5)
  second = update(c, 1.05, observation(198.3, 1, 1.05), v_ego=13.5)
  third = update(c, 1.17, observation(196.7, 1, 1.17), v_ego=13.5)
  expired = update(c, 1.28, observation(196.7, 0, 1.28), v_ego=13.5)

  assert first.phase == TrafficControlPhase.redCandidate
  assert second.phase == TrafficControlPhase.redCandidate
  assert third.phase == TrafficControlPhase.redCandidate
  assert expired.phase == TrafficControlPhase.off
  assert c.event_id == 0
  assert c.stop_session_id == 0


def test_far_low_urgency_red_confirms_after_half_second():
  c = controller()
  update(c, 1.0, observation(190.0, 1, 1.0), v_ego=10.0)
  update(c, 1.1, observation(189.0, 1, 1.1), v_ego=10.0)
  decision = update(c, 1.5, observation(185.0, 1, 1.5), v_ego=10.0)

  assert decision.phase in c.ACTIVE_PHASES
  assert c.event_id == 1


def test_near_urgent_red_still_confirms_in_two_frames():
  c = controller()
  first = update(c, 1.0, observation(45.0, 1, 1.0), v_ego=15.0)
  second = update(c, 1.1, observation(43.5, 1, 1.1), v_ego=15.0)

  assert first.phase == TrafficControlPhase.redCandidate
  assert second.phase in c.ACTIVE_PHASES
  assert c.event_id == 1


def test_nearer_low_urgency_replacement_requires_half_second_continuous_evidence():
  c = controller()
  establish_red(c, distance=190.0, speed=5.0)
  original_event = c.event_id
  original_session = c.stop_session_id

  update(c, 2.0, observation(100.0, 1, 2.0), v_ego=5.0)
  update(c, 2.1, observation(99.5, 1, 2.1), v_ego=5.0)
  assert c.event_id == original_event
  assert c.stop_session_id == original_session

  confirmed = update(c, 2.5, observation(97.5, 1, 2.5), v_ego=5.0)
  assert c.event_id == original_event + 1
  assert c.stop_session_id == original_session + 1
  assert confirmed.phase in c.ACTIVE_PHASES


def test_farther_active_replacement_requires_one_second_of_continuous_evidence():
  c = controller()
  establish_red(c, distance=35.0, speed=5.0)
  original_session = c.stop_session_id

  first = update(c, 2.0, observation(70.0, 1, 2.0), v_ego=5.0)
  second = update(c, 2.5, observation(67.5, 1, 2.5), v_ego=5.0)

  assert first.stop_session_id == original_session
  assert second.stop_session_id == original_session

  confirmed = update(c, 3.0, observation(65.0, 1, 3.0), v_ego=5.0)
  assert confirmed.stop_session_id == original_session + 1
  assert math.isclose(confirmed.remaining_distance, 60.0, abs_tol=0.1)


def test_nearer_active_replacement_still_confirms_in_two_frames():
  c = controller()
  establish_red(c, distance=100.0, speed=10.0)
  original_session = c.stop_session_id

  first = update(c, 2.0, observation(35.0, 1, 2.0), v_ego=10.0)
  confirmed = update(c, 2.5, observation(30.0, 1, 2.5), v_ego=10.0)

  assert first.stop_session_id == original_session
  assert confirmed.stop_session_id == original_session + 1
  assert math.isclose(confirmed.remaining_distance, 25.0, abs_tol=0.1)


def test_transient_27_to_63_to_25_tracks_do_not_churn_stop_sessions():
  c = controller()
  establish_red(c, distance=27.0, speed=5.0)
  original_session = c.stop_session_id

  for now_s, distance in ((2.0, 63.0), (2.5, 60.5), (3.0, 25.0), (3.5, 22.5)):
    decision = update(c, now_s, observation(distance, 1, now_s), v_ego=5.0)
    assert decision.stop_session_id == original_session

  assert c.transition_reason == "stop_confirmed"


def test_new_stop_session_discards_stale_green_tracking_geometry():
  c = controller()
  update(c, 1.0, observation(100.0, 2, 1.0), v_ego=0.0)
  update(c, 1.5, observation(100.0, 2, 1.5), v_ego=0.0)
  assert c.stop_station is not None
  assert c.remaining_distance == 95.0

  first_red = update(c, 2.0, observation(42.0, 1, 2.0), v_ego=0.0)
  confirmed = update(c, 2.5, observation(42.0, 1, 2.5), v_ego=0.0)

  assert first_red.phase == TrafficControlPhase.redCandidate
  assert confirmed.phase in c.ACTIVE_PHASES
  assert c.stop_session_id == 1
  assert math.isclose(confirmed.remaining_distance, 37.0, abs_tol=0.1)
  assert math.isclose(confirmed.can_remaining, 37.0, abs_tol=0.1)


def test_internal_tesla_state_fields_do_not_change_color_decision():
  c = controller()
  update(c, 1.0, observation(50, 1, 1.0, feature_state=0, state_machine=6))
  decision = update(c, 1.5, observation(45, 1, 1.5, feature_state=0, state_machine=0))
  assert decision.active


def test_controller_independently_rejects_wrong_bus_type_and_short_dlc():
  for kwargs in ({"bus": 1}, {"control_type": 2}, {"dlc": 5}):
    c = controller()
    update(c, 1.0, observation(40, 1, 1.0, **kwargs))
    decision = update(c, 1.5, observation(35, 1, 1.5, **kwargs))
    assert decision.phase == TrafficControlPhase.off


def test_distance_200_is_trusted_but_201_cannot_create_a_target():
  accepted = controller()
  establish_red(accepted, distance=200.0, speed=10.0)
  assert accepted.phase in accepted.ACTIVE_PHASES

  rejected = controller()
  update(rejected, 1.0, observation(201, 1, 1.0))
  decision = update(rejected, 1.5, observation(201, 1, 1.5))
  assert decision.phase == TrafficControlPhase.off
  assert rejected.event_id == 0


def test_default_reference_is_five_meters_and_model_cannot_replace_can_geometry():
  c = controller()
  update(c, 1.0, observation(50, 1, 1.0), model_distance=8.0, model_candidate=True)
  decision = update(c, 1.5, observation(45, 1, 1.5), model_distance=4.0, model_candidate=True)
  assert decision.stop_reference == 5.0
  assert math.isclose(decision.remaining_distance, 40.0, abs_tol=0.1)


def test_stop_station_keeps_converging_to_fresh_can_inside_ten_meters():
  c = controller()
  establish_red(c, distance=20.0, speed=5.0)
  update(c, 2.2, observation(11.5, 1, 2.2), v_ego=5.0)
  previous_station = c.stop_station
  assert c.remaining_distance <= 10.0
  # Fresh, continuous CAN remains authoritative in the final approach.
  update(c, 2.4, observation(8.0, 1, 2.4), v_ego=5.0)
  assert c.stop_station != previous_station
  assert abs(c.remaining_distance - 3.0) <= 1.5
  assert c.remaining_distance < 10.0


def test_committed_stop_survives_transport_dropout():
  c = controller()
  establish_red(c)
  stale = observation(70, 1, 1.5, available=False)
  decision = update(c, 2.5, stale)
  assert decision.phase in c.ACTIVE_PHASES
  assert decision.apply_constraint


def test_short_off_evidence_loss_freezes_active_stop_geometry():
  c = controller()
  establish_red(c, distance=50.0, speed=5.0)
  session_id = c.stop_session_id
  stop_station = c.stop_station
  can_remaining = c.can_remaining

  off = update(c, 2.0, observation(50.0, 0, 2.0), v_ego=5.0)

  assert off.phase in c.ACTIVE_PHASES
  assert off.stop_session_id == session_id
  assert c.stop_station == stop_station
  assert off.can_remaining == can_remaining
  assert off.remaining_distance < can_remaining


def test_persistent_off_evidence_loss_smoothly_releases_a_moving_stop():
  c = controller()
  establish_red(c, distance=50.0, speed=5.0)
  session_id = c.stop_session_id

  for now_s, distance in ((2.0, 45.0), (2.5, 42.5), (3.0, 40.0), (3.5, 37.5)):
    still_stopping = update(c, now_s, observation(distance, 0, now_s), v_ego=5.0)
    assert still_stopping.phase in c.ACTIVE_PHASES

  released = update(c, 4.0, observation(35.0, 0, 4.0), v_ego=5.0)
  assert released.phase == TrafficControlPhase.release
  assert released.stop_session_id == session_id
  assert c.transition_reason == "signal_lost_release"


def test_persistent_out_of_range_evidence_smoothly_releases_a_moving_stop():
  c = controller()
  establish_red(c, distance=50.0, speed=5.0)

  for now_s, distance in ((2.0, 230.0), (2.5, 227.5), (3.0, 225.0), (3.5, 222.5)):
    still_stopping = update(c, now_s, observation(distance, 1, now_s), v_ego=5.0)
    assert still_stopping.phase in c.ACTIVE_PHASES

  released = update(c, 4.0, observation(220.0, 1, 4.0), v_ego=5.0)
  assert released.phase == TrafficControlPhase.release
  assert c.transition_reason == "signal_lost_release"


def test_persistent_unsupported_color_evidence_smoothly_releases_a_moving_stop():
  c = controller()
  establish_red(c, distance=50.0, speed=5.0)
  stop_station = c.stop_station

  for now_s in (2.0, 2.5, 3.0, 3.5):
    still_stopping = update(c, now_s, observation(45.0, 5, now_s), v_ego=5.0)
    assert still_stopping.phase in c.ACTIVE_PHASES
    assert c.stop_station == stop_station

  released = update(c, 4.0, observation(45.0, 5, 4.0), v_ego=5.0)
  assert released.phase == TrafficControlPhase.release
  assert c.transition_reason == "signal_lost_release"


def test_signal_loss_release_expires_while_off_frames_continue():
  c = controller(release_s=3.0)
  establish_red(c, distance=50.0, speed=5.0)

  for now_s, distance in ((2.0, 45.0), (2.5, 42.5), (3.0, 40.0),
                          (3.5, 37.5), (4.0, 35.0)):
    released = update(c, now_s, observation(distance, 0, now_s), v_ego=5.0)
  assert released.phase == TrafficControlPhase.release

  for now_s, distance in ((4.5, 32.5), (5.0, 30.0), (5.5, 27.5),
                          (6.0, 25.0), (6.5, 22.5)):
    still_releasing = update(c, now_s, observation(distance, 0, now_s), v_ego=5.0)
  still_releasing = update(c, 6.9, observation(20.5, 0, 6.9), v_ego=5.0)
  expired = update(c, 7.0, observation(20.0, 0, 7.0), v_ego=5.0)

  assert still_releasing.phase == TrafficControlPhase.release
  assert expired.phase == TrafficControlPhase.off
  assert expired.stop_session_id == 0


def test_off_evidence_loss_timer_restarts_after_red_recovers():
  c = controller()
  establish_red(c, distance=50.0, speed=5.0)
  session_id = c.stop_session_id

  update(c, 2.0, observation(45.0, 0, 2.0), v_ego=5.0)
  recovered = update(c, 2.5, observation(42.5, 1, 2.5), v_ego=5.0)
  assert recovered.stop_session_id == session_id

  update(c, 3.0, observation(40.0, 0, 3.0), v_ego=5.0)
  still_stopping = update(c, 4.5, observation(32.5, 0, 4.5), v_ego=5.0)
  assert still_stopping.phase in c.ACTIVE_PHASES

  released = update(c, 5.0, observation(30.0, 0, 5.0), v_ego=5.0)
  assert released.phase == TrafficControlPhase.release


def test_red_after_signal_loss_release_requires_two_fresh_frames_to_rearm():
  c = controller()
  establish_red(c, distance=50.0, speed=5.0)
  session_id = c.stop_session_id

  for now_s, distance in ((2.0, 45.0), (2.5, 42.5), (3.0, 40.0),
                          (3.5, 37.5), (4.0, 35.0)):
    released = update(c, now_s, observation(distance, 0, now_s), v_ego=5.0)
  assert released.phase == TrafficControlPhase.release

  first_red = update(c, 4.5, observation(32.5, 1, 4.5), v_ego=5.0)
  assert first_red.phase == TrafficControlPhase.release
  assert first_red.stop_session_id == session_id

  second_red = update(c, 5.0, observation(30.0, 1, 5.0), v_ego=5.0)
  assert second_red.phase in c.ACTIVE_PHASES
  assert second_red.stop_session_id == session_id + 1


def test_persistent_off_does_not_release_a_stationary_hold():
  c = controller()
  establish_red(c, distance=5.0, speed=0.0)
  held = update(c, 2.0, observation(5.0, 1, 2.0), v_ego=0.0)
  assert held.phase == TrafficControlPhase.hold
  session_id = c.stop_session_id

  for now_s in (2.5, 3.0, 3.5, 4.0, 4.5, 5.0):
    held = update(c, now_s, observation(5.0, 0, now_s), v_ego=0.0)

  assert held.phase == TrafficControlPhase.hold
  assert held.should_stop
  assert held.stop_session_id == session_id


def test_vehicle_reaching_frozen_stop_station_enters_hold_during_off_gap():
  c = controller()
  establish_red(c, distance=18.0, speed=6.0)
  session_id = c.stop_session_id
  assert c.can_remaining == 10.0

  decision = None
  for now_s, speed in ((2.0, 6.0), (2.5, 6.0), (3.0, 6.0),
                       (3.5, 2.0), (4.0, 0.0)):
    decision = update(c, now_s, observation(15.0, 0, now_s), v_ego=speed)

  assert decision is not None
  assert decision.phase == TrafficControlPhase.hold
  assert decision.should_stop
  assert decision.stop_session_id == session_id
  assert decision.remaining_distance == 0.0
  assert decision.can_remaining == 10.0


def test_vehicle_reaching_frozen_stop_station_enters_hold_during_transport_gap():
  c = controller()
  establish_red(c, distance=18.0, speed=6.0)
  session_id = c.stop_session_id
  assert c.can_remaining == 10.0

  decision = None
  for now_s, speed in ((2.0, 6.0), (2.5, 6.0), (3.0, 6.0),
                       (3.5, 2.0), (4.0, 0.0)):
    decision = update(
      c, now_s, observation(15.0, 1, now_s, available=False), v_ego=speed,
    )

  assert decision is not None
  assert decision.phase == TrafficControlPhase.hold
  assert decision.should_stop
  assert decision.stop_session_id == session_id
  assert decision.remaining_distance == 0.0
  assert decision.can_remaining == 10.0


def test_persistent_off_does_not_release_a_confirmed_flashing_stop():
  c = controller()
  for now_s, distance, light in ((1.0, 80.0, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
                                 (1.7, 74.4, 2), (2.2, 70.4, 4),
                                 (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4)):
    decision = update(c, now_s, observation(distance, light, now_s), v_ego=8.0)
  assert decision.phase == TrafficControlPhase.flashingGreenStop
  session_id = c.stop_session_id

  for now_s, distance in ((3.7, 58.4), (4.2, 54.4), (4.7, 50.4),
                          (5.2, 46.4), (5.7, 42.4)):
    decision = update(c, now_s, observation(distance, 4, now_s), v_ego=8.0)

  assert decision.phase == TrafficControlPhase.flashingGreenStop
  assert decision.stop_session_id == session_id


def test_stop_safety_permission_is_independent_from_raw_freshness():
  c = controller()
  establish_red(c)

  stale = update(c, 2.0, observation(70, 1, 1.5, available=False))
  assert stale.stop_safety_allowed
  assert not stale.stop_control_allowed

  current_lane = update(c, 2.1, observation(69, 1, 2.1), blinker=True)
  assert current_lane.stop_safety_allowed
  assert current_lane.stop_control_allowed


def test_long_raw_dropout_requires_two_fresh_stop_frames_before_rearming():
  c = controller()
  establish_red(c, distance=60.0, speed=5.0)
  session_id = c.stop_session_id

  update(c, 2.0, observation(55.0, 1, 1.5, available=False), v_ego=5.0)
  first = update(c, 4.1, observation(44.5, 1, 4.1), v_ego=5.0)
  assert first.stop_session_id == session_id
  assert not first.stop_safety_allowed
  assert not first.stop_control_allowed

  duplicate = update(c, 4.2, observation(44.5, 1, 4.1), v_ego=5.0)
  assert not duplicate.stop_safety_allowed
  assert not duplicate.stop_control_allowed

  second = update(c, 4.6, observation(42.0, 1, 4.6), v_ego=5.0)
  assert second.stop_session_id == session_id + 1
  assert second.stop_safety_allowed
  assert second.stop_control_allowed


def test_long_dropout_clears_stationary_green_confirmation_count():
  c = controller()
  establish_red(c, distance=5.0, speed=0.0)

  first_green = update(c, 2.0, observation(5.0, 2, 2.0), v_ego=0.0)
  assert first_green.phase in c.ACTIVE_PHASES
  update(c, 4.3, observation(5.0, 2, 2.0, available=False), v_ego=0.0)

  recovered_first = update(c, 4.5, observation(5.0, 2, 4.5), v_ego=0.0)
  recovered_second = update(c, 5.0, observation(5.0, 2, 5.0), v_ego=0.0)

  assert recovered_first.phase in c.ACTIVE_PHASES
  assert recovered_second.phase == TrafficControlPhase.release


def test_farther_replacement_confirmation_cannot_span_a_long_transport_gap():
  c = controller()
  establish_red(c, distance=35.0, speed=5.0)
  session_id = c.stop_session_id

  update(c, 2.0, observation(70.0, 1, 2.0), v_ego=5.0)
  update(c, 2.2, observation(70.0, 1, 2.0, available=False), v_ego=5.0)
  recovered_first = update(c, 4.5, observation(57.5, 1, 4.5), v_ego=5.0)
  recovered_second = update(c, 5.0, observation(55.0, 1, 5.0), v_ego=5.0)
  recovered_third = update(c, 5.5, observation(52.5, 1, 5.5), v_ego=5.0)

  assert recovered_first.stop_session_id == session_id
  assert recovered_second.stop_session_id == session_id
  assert recovered_third.stop_session_id == session_id + 1


def test_critical_dropout_age_starts_at_the_last_real_can_frame():
  c = controller()
  establish_red(c, distance=60.0, speed=5.0)

  update(c, 2.3, observation(57.5, 1, 1.5, available=False), v_ego=5.0)
  first_recovery = update(c, 3.6, observation(52.5, 1, 3.6), v_ego=5.0)

  assert not first_recovery.stop_safety_allowed
  assert not first_recovery.stop_control_allowed


def test_direct_multi_second_real_frame_gap_requires_reconfirmation():
  c = controller()
  establish_red(c, distance=60.0, speed=5.0)

  first_after_gap = update(c, 4.1, observation(50.0, 1, 4.1), v_ego=5.0)

  assert not first_after_gap.stop_safety_allowed
  assert not first_after_gap.stop_control_allowed


def test_long_dropout_stop_reconfirmation_does_not_delay_green_release():
  c = controller()
  establish_red(c, distance=30.0, speed=5.0)

  first_green = update(c, 4.1, observation(25.0, 2, 4.1), v_ego=5.0)
  second_green = update(c, 4.6, observation(22.5, 2, 4.6), v_ego=5.0)

  assert first_green.phase == TrafficControlPhase.release
  assert second_green.phase == TrafficControlPhase.release
  assert second_green.apply_constraint


def test_green_requires_two_real_frames_and_releases_same_event():
  c = controller()
  establish_red(c, distance=30.0, speed=5.0)
  one = update(c, 1.8, observation(26.0, 2, 1.8), v_ego=5.0)
  assert one.phase == TrafficControlPhase.release


def test_moving_green_releases_even_when_distance_was_recalculated():
  c = controller()
  establish_red(c, distance=100.0, speed=10.0)

  released = update(c, 2.0, observation(35.0, 2, 2.0), v_ego=5.0)

  assert released.phase == TrafficControlPhase.release
  assert released.light_state == 2


def test_stationary_recalculated_green_requires_two_distinct_frames_to_release():
  c = controller()
  establish_red(c, distance=100.0, speed=10.0)

  first = update(c, 2.0, observation(11.0, 2, 2.0), v_ego=0.0)
  duplicate = update(c, 2.1, observation(11.0, 2, 2.0), v_ego=0.0)
  second = update(c, 2.5, observation(11.0, 2, 2.5), v_ego=0.0)

  assert first.phase in c.ACTIVE_PHASES
  assert duplicate.phase in c.ACTIVE_PHASES
  assert second.phase == TrafficControlPhase.release
  assert second.light_state == 2


def test_stationary_green_still_requires_two_distinct_real_frames():
  c = controller()
  establish_red(c, distance=5.0, speed=0.0)
  one = update(c, 1.8, observation(5.0, 2, 1.8), v_ego=0.0)
  assert one.phase in c.ACTIVE_PHASES
  duplicate = update(c, 1.9, observation(5.0, 2, 1.8), v_ego=0.0)
  assert duplicate.phase in c.ACTIVE_PHASES
  two = update(c, 2.3, observation(5.0, 2, 2.3), v_ego=0.0)
  assert two.phase == TrafficControlPhase.release


def test_stationary_green_release_survives_the_normal_three_second_timeout():
  c = controller(stationary_release_s=10.0)
  establish_red(c, distance=5.0, speed=0.0)
  update(c, 2.0, observation(5.0, 2, 2.0), v_ego=0.0)
  released = update(c, 2.5, observation(5.0, 2, 2.5), v_ego=0.0)
  assert released.phase == TrafficControlPhase.release

  still_waiting = released
  for step in range(30, 121, 5):
    now_s = step / 10.0
    still_waiting = update(c, now_s, observation(5.0, 2, now_s), v_ego=0.0)
  assert still_waiting.phase == TrafficControlPhase.release
  assert still_waiting.stop_session_id == released.stop_session_id

  expired = update(c, 12.5, observation(5.0, 2, 12.5), v_ego=0.0)
  assert expired.phase == TrafficControlPhase.off
  assert expired.stop_session_id == 0


def test_moving_green_release_keeps_the_existing_three_second_timeout():
  c = controller(stationary_release_s=10.0)
  establish_red(c, distance=5.0, speed=0.0)
  update(c, 2.0, observation(5.0, 2, 2.0), v_ego=0.0)
  update(c, 2.5, observation(5.0, 2, 2.5), v_ego=0.0)

  expired = None
  for step in range(30, 61, 5):
    now_s = step / 10.0
    expired = update(c, now_s, observation(max(0.0, 5.0 - (now_s - 2.5)), 2, now_s), v_ego=1.0)
  assert expired.phase == TrafficControlPhase.off
  assert expired.stop_session_id == 0


def test_long_green_can_dropout_clears_stationary_release_before_recovery():
  c = controller(stationary_release_s=10.0, critical_observation_dropout_s=2.0)
  establish_red(c, distance=5.0, speed=0.0)
  update(c, 2.0, observation(5.0, 2, 2.0), v_ego=0.0)
  released = update(c, 2.5, observation(5.0, 2, 2.5), v_ego=0.0)
  assert released.phase == TrafficControlPhase.release

  stale = update(c, 4.8, observation(5.0, 2, 2.5, available=False), v_ego=0.0)
  assert stale.phase == TrafficControlPhase.release
  recovered = update(c, 5.0, observation(5.0, 2, 5.0), v_ego=0.0)
  assert recovered.phase == TrafficControlPhase.off
  assert recovered.stop_session_id == 0


def test_far_red_after_release_gets_a_fresh_stop_station():
  c = controller()
  establish_red(c, distance=5.0, speed=0.0)
  update(c, 2.0, observation(5.0, 2, 2.0), v_ego=0.0)
  released = update(c, 2.5, observation(5.0, 2, 2.5), v_ego=0.0)
  assert released.phase == TrafficControlPhase.release

  update(c, 3.0, observation(150.0, 1, 3.0), v_ego=15.0)
  new_red = update(c, 3.5, observation(142.5, 1, 3.5), v_ego=15.0)

  assert new_red.phase in c.ACTIVE_PHASES
  assert math.isclose(new_red.remaining_distance, 137.5, abs_tol=0.1)


def test_stop_only_mode_releases_the_stop_without_requesting_go():
  c = controller(TrafficControlMode.stopOnly)
  establish_red(c, distance=30.0, speed=5.0)
  update(c, 1.8, observation(26.0, 2, 1.8), v_ego=5.0)
  decision = update(c, 2.3, observation(23.5, 2, 2.3), v_ego=5.0)
  assert decision.phase == TrafficControlPhase.release


def test_second_in_range_off_edge_latches_flashing_green_stop():
  c = controller()
  update(c, 1.0, observation(80.0, 2, 1.0), v_ego=8.0)
  update(c, 1.1, observation(79.2, 2, 1.1), v_ego=8.0)
  first_off = update(c, 1.2, observation(78.4, 4, 1.2), v_ego=8.0)
  assert first_off.phase == TrafficControlPhase.off
  update(c, 1.7, observation(74.4, 2, 1.7), v_ego=8.0)
  second_off = update(c, 2.2, observation(70.4, 4, 2.2), v_ego=8.0)
  assert second_off.phase == TrafficControlPhase.flashingGreenStop
  assert second_off.apply_constraint
  assert c.flash_latched


def test_irregular_second_off_edge_resets_flash_candidate():
  c = controller()
  for now_s, distance, light in (
    (1.0, 80.0, 2), (1.2, 78.4, 4),
    (1.7, 74.4, 2), (3.3, 61.6, 4),
  ):
    decision = update(c, now_s, observation(distance, light, now_s), v_ego=8.0)
  assert decision.phase == TrafficControlPhase.off
  assert not decision.apply_constraint
  assert not c.flash_latched


def test_discontinuous_green_cannot_bypass_confirmed_flashing_green_stop():
  c = controller()
  armed = None
  for now_s, distance, light in (
    (1.0, 80.0, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
    (1.7, 74.4, 2), (2.2, 70.4, 4),
    (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4),
  ):
    armed = update(c, now_s, observation(distance, light, now_s), v_ego=8.0)

  assert armed is not None
  assert armed.phase == TrafficControlPhase.flashingGreenStop
  assert c.flash_latched
  session_id = c.stop_session_id

  discontinuous_green = update(c, 3.7, observation(20.0, 2, 3.7), v_ego=8.0)

  assert discontinuous_green.phase == TrafficControlPhase.flashingGreenStop
  assert discontinuous_green.stop_session_id == session_id
  assert c.flash_latched
  assert c.release_since_ns is None


def test_route10_single_off_then_stable_green_never_enters_a_flash_phase():
  c = controller()
  update(c, 1.0, observation(200.0, 2, 1.0), v_ego=10.0)
  first_off = update(c, 1.1, observation(199.0, 0, 1.1), v_ego=10.0)
  assert first_off.phase == TrafficControlPhase.off

  now_s = 1.6
  distance = 194.0
  while distance >= 5.0:
    decision = update(c, now_s, observation(distance, 2, now_s), v_ego=10.0)
    assert decision.phase == TrafficControlPhase.off
    assert not decision.apply_constraint
    distance -= 5.0
    now_s += 0.5


def test_single_off_cannot_overwrite_an_existing_release_session():
  c = controller()
  establish_red(c, distance=5.0, speed=0.0)
  update(c, 2.0, observation(5.0, 2, 2.0), v_ego=0.0)
  released = update(c, 2.5, observation(5.0, 2, 2.5), v_ego=0.0)
  assert released.phase == TrafficControlPhase.release
  session_id = c.stop_session_id

  one_off = update(c, 3.0, observation(5.0, 0, 3.0), v_ego=0.0)
  assert one_off.phase == TrafficControlPhase.release
  assert c.stop_session_id == session_id
  green = update(c, 3.5, observation(5.0, 2, 3.5), v_ego=0.0)
  assert green.phase == TrafficControlPhase.release
  assert c.stop_session_id == session_id


def test_out_of_range_green_off_cadence_cannot_arm_inside_control_range():
  c = controller()
  # Even a regular OFF cadence at the 254 m sentinel is untrusted.
  # Colors above 200 m are diagnostic-only and cannot establish future control evidence.
  for now_s, distance, light in (
    (1.0, 210.0, 2), (1.9, 254.0, 4),
    (2.5, 238.0, 2), (3.0, 254.0, 4),
    (3.5, 223.0, 2), (4.0, 254.0, 4),
  ):
    decision = update(c, now_s, observation(distance, light, now_s), v_ego=13.0)
    assert decision.phase == TrafficControlPhase.off
    assert c.event_id == 0

  trusted = update(c, 4.5, observation(200.0, 2, 4.5), v_ego=13.0)
  assert trusted.phase == TrafficControlPhase.off
  assert not trusted.apply_constraint
  assert not c.flash_latched


def test_single_far_green_off_dropout_cannot_create_a_flashing_green_stop():
  c = controller()
  update(c, 1.0, observation(210.0, 2, 1.0), v_ego=13.0)
  update(c, 1.9, observation(254.0, 0, 1.9), v_ego=13.0)
  trusted = update(c, 2.5, observation(200.0, 2, 2.5), v_ego=13.0)

  assert trusted.phase == TrafficControlPhase.off
  assert not c.flash_latched
  assert c.event_id == 0


def test_turn_signal_does_not_shadow_a_confirmed_current_lane_flash_stop():
  c = controller()
  for now_s, distance, light in ((1.0, 80.0, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
                                 (1.7, 74.4, 2), (2.2, 70.4, 4),
                                 (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4)):
    decision = update(c, now_s, observation(distance, light, now_s), v_ego=8.0, blinker=True)
  assert decision.phase == TrafficControlPhase.flashingGreenStop
  assert decision.stop_safety_allowed
  assert decision.stop_control_allowed


def test_route4f_discontinuous_off_cannot_form_flash_but_yellow_can_stop():
  c = controller()
  update(c, 1.0, observation(62.5, 2, 1.0), v_ego=5.0)
  update(c, 1.5, observation(60.0, 2, 1.5), v_ego=5.0)
  discontinuous_off = update(c, 2.0, observation(94.0, 0, 2.0), v_ego=5.0)
  assert discontinuous_off.phase != TrafficControlPhase.greenFlashCandidate
  assert not c.flash_latched
  update(c, 2.5, observation(49.0, 3, 2.5), v_ego=8.0)
  yellow = update(c, 3.0, observation(45.0, 3, 3.0), v_ego=8.0)
  assert yellow.phase == TrafficControlPhase.yellowStop
  assert math.isclose(yellow.remaining_distance, 40.0, abs_tol=1.0)


def test_flashing_green_stop_rejects_a_short_green_pulse_but_releases_on_stable_green():
  c = controller()
  for now_s, distance, light in ((1.0, 80, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
                                 (1.7, 74.4, 2), (2.2, 70.4, 4),
                                 (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4)):
    update(c, now_s, observation(distance, light, now_s), v_ego=8.0)
  short_pulse = update(c, 3.7, observation(58.4, 2, 3.7), v_ego=8.0)
  assert short_pulse.phase == TrafficControlPhase.flashingGreenStop
  update(c, 4.2, observation(54.4, 2, 4.2), v_ego=8.0)
  update(c, 4.7, observation(50.4, 2, 4.7), v_ego=8.0)
  stable_green = update(c, 5.2, observation(46.4, 2, 5.2), v_ego=8.0)
  assert stable_green.phase == TrafficControlPhase.release
  assert not c.flash_latched


def test_red_after_flashing_green_returns_to_ordinary_stop_and_can_release():
  c = controller()
  for now_s, distance, light in ((1.0, 80, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
                                 (1.7, 74.4, 2), (2.2, 70.4, 4),
                                 (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4)):
    update(c, now_s, observation(distance, light, now_s), v_ego=8.0)
  assert c.flash_latched

  first_red = update(c, 3.7, observation(58.4, 1, 3.7), v_ego=8.0)
  assert c.flash_latched
  assert first_red.phase == TrafficControlPhase.flashingGreenStop
  confirmed_red = update(c, 4.2, observation(54.4, 1, 4.2), v_ego=8.0)
  assert not c.flash_latched
  assert confirmed_red.phase in c.ACTIVE_PHASES
  update(c, 4.7, observation(50.4, 2, 4.7), v_ego=8.0)
  released = update(c, 5.2, observation(46.4, 2, 5.2), v_ego=8.0)

  assert released.phase == TrafficControlPhase.release


def test_yellow_after_flashing_green_returns_to_ordinary_stop_and_can_release():
  c = controller()
  for now_s, distance, light in ((1.0, 80, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
                                 (1.7, 74.4, 2), (2.2, 70.4, 4),
                                 (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4)):
    update(c, now_s, observation(distance, light, now_s), v_ego=8.0)

  first_yellow = update(c, 3.7, observation(58.4, 3, 3.7), v_ego=8.0)
  assert c.flash_latched
  assert first_yellow.phase == TrafficControlPhase.flashingGreenStop
  confirmed_yellow = update(c, 4.2, observation(54.4, 3, 4.2), v_ego=8.0)
  assert not c.flash_latched
  assert confirmed_yellow.phase == TrafficControlPhase.yellowStop
  update(c, 4.7, observation(50.4, 2, 4.7), v_ego=8.0)
  released = update(c, 5.2, observation(46.4, 2, 5.2), v_ego=8.0)

  assert released.phase == TrafficControlPhase.release


def test_early_yellow_stops_and_late_yellow_passes_once():
  early = controller()
  update(early, 1.0, observation(80, 3, 1.0), v_ego=10.0)
  early_decision = update(early, 1.5, observation(75, 3, 1.5), v_ego=10.0)
  assert early_decision.phase == TrafficControlPhase.yellowStop

  late = controller()
  update(late, 1.0, observation(20, 3, 1.0), v_ego=15.0)
  late_decision = update(late, 1.2, observation(17, 3, 1.2), v_ego=15.0)
  assert late_decision.phase == TrafficControlPhase.yellowPass
  assert not late_decision.apply_constraint

  later_red = update(late, 1.7, observation(9.5, 1, 1.7), v_ego=12.0)
  assert later_red.phase == TrafficControlPhase.yellowPass
  assert not later_red.apply_constraint


def test_active_yellow_promotes_a_confirmed_recalculated_yellow_track():
  c = controller()
  update(c, 1.0, observation(80.0, 3, 1.0), v_ego=10.0)
  update(c, 1.5, observation(75.0, 3, 1.5), v_ego=10.0)
  assert c.phase == TrafficControlPhase.yellowStop
  old_session = c.stop_session_id

  first = update(c, 2.0, observation(40.0, 3, 2.0), v_ego=10.0)
  confirmed = update(c, 2.5, observation(35.0, 3, 2.5), v_ego=10.0)

  assert first.stop_session_id == old_session
  assert confirmed.phase == TrafficControlPhase.yellowStop
  assert confirmed.stop_session_id == old_session + 1
  assert math.isclose(confirmed.remaining_distance, 30.0, abs_tol=0.1)


def test_green_tracking_geometry_is_preserved_when_same_target_turns_red():
  c = controller()
  update(c, 1.0, observation(80.0, 2, 1.0), v_ego=10.0)
  update(c, 1.5, observation(75.0, 2, 1.5), v_ego=10.0)
  green_station = c.stop_station
  update(c, 2.0, observation(70.0, 1, 2.0), v_ego=10.0)
  confirmed = update(c, 2.5, observation(65.0, 1, 2.5), v_ego=10.0)
  assert confirmed.phase in c.ACTIVE_PHASES
  assert green_station is not None
  assert abs((c.stop_station or 0.0) - green_station) <= 2.0


def test_yellow_pass_latch_does_not_leak_into_the_next_intersection():
  c = controller()
  update(c, 1.0, observation(20, 3, 1.0), v_ego=15.0)
  first = update(c, 1.2, observation(17, 3, 1.2), v_ego=15.0)
  assert first.phase == TrafficControlPhase.yellowPass
  update(c, 2.0, observation(254, 2, 2.0), v_ego=15.0)
  update(c, 3.0, observation(80, 3, 3.0), v_ego=5.0)
  second = update(c, 3.5, observation(77.5, 3, 3.5), v_ego=5.0)
  assert second.phase == TrafficControlPhase.yellowStop
  assert second.stop_control_allowed


def test_mid_dilemma_yellow_makes_and_latches_one_decision():
  c = controller(comfort_brake=2.4)
  # The second frame requires about 2.6 m/s^2: inside the dilemma band but
  # above the configured comfortable brake, so the one-shot result is PASS.
  update(c, 1.0, observation(35, 3, 1.0), v_ego=12.0)
  decision = update(c, 1.5, observation(32.7, 3, 1.5), v_ego=12.0)
  assert decision.phase == TrafficControlPhase.yellowPass
  assert c.yellow_latched is False


def test_existing_red_stop_treats_first_yellow_as_stop_without_reconfirmation():
  c = controller()
  establish_red(c)
  decision = update(c, 1.8, observation(72.0, 3, 1.8))
  assert decision.phase == TrafficControlPhase.yellowStop
  assert decision.apply_constraint


def test_zero_to_254_distance_wrap_is_passed_not_a_new_forward_target():
  c = controller()
  establish_red(c, distance=10.0, speed=5.0)
  update(c, 2.0, observation(1.0, 1, 2.0), v_ego=5.0)
  decision = update(c, 2.1, observation(254.0, 1, 2.1), v_ego=5.0)
  assert decision.phase == TrafficControlPhase.passed
  assert not decision.apply_constraint
  assert decision.remaining_distance == 0.0


def test_signal_event_keeps_its_identity_and_stop_geometry_across_updates():
  c = controller()
  establish_red(c)
  event_id = c.event_id
  before = c.stop_station
  decision = update(c, 2.0, observation(70, 1, 2.0))
  assert decision.active
  assert c.event_id == event_id
  assert c.stop_station == before or c.remaining_distance > 10.0


def test_active_braking_event_promotes_a_confirmed_recalculated_red_track():
  c = controller()
  establish_red(c, distance=100.0, speed=10.0)
  original_event = c.event_id
  original_session = c.stop_session_id
  first = update(c, 2.0, observation(35.0, 1, 2.0), v_ego=10.0)
  assert c.event_id == original_event
  assert first.remaining_distance > 80.0
  second = update(c, 2.5, observation(30.0, 1, 2.5), v_ego=10.0)
  assert c.event_id == original_event + 1
  assert c.stop_session_id == original_session + 1
  assert second.phase in c.ACTIVE_PHASES
  assert math.isclose(second.remaining_distance, 25.0, abs_tol=0.1)
  assert math.isclose(second.can_remaining, 25.0, abs_tol=0.1)


def test_route1d_recalculated_red_track_cannot_lock_out_later_green_release():
  c = controller()
  update(c, 1.0, observation(95.0, 1, 1.0), v_ego=10.0)
  update(c, 1.5, observation(90.0, 1, 1.5), v_ego=10.0)
  old_session = c.stop_session_id

  first_recalculated = update(c, 2.0, observation(53.0, 1, 2.0), v_ego=10.0)
  confirmed_recalculated = update(c, 2.1, observation(52.0, 1, 2.1), v_ego=10.0)

  assert first_recalculated.stop_session_id == old_session
  assert confirmed_recalculated.stop_session_id == old_session + 1
  assert math.isclose(confirmed_recalculated.remaining_distance, 47.0, abs_tol=0.1)
  assert math.isclose(confirmed_recalculated.can_remaining, 47.0, abs_tol=0.1)

  first_green = update(c, 2.5, observation(11.0, 2, 2.5), v_ego=0.0)
  second_green = update(c, 3.0, observation(11.0, 2, 3.0), v_ego=0.0)

  assert first_green.phase in c.ACTIVE_PHASES
  assert second_green.phase == TrafficControlPhase.release
  assert not second_green.should_stop


def test_single_active_target_jump_cannot_replace_the_stop_session():
  c = controller()
  establish_red(c, distance=100.0, speed=10.0)
  session_id = c.stop_session_id
  event_id = c.event_id
  first = update(c, 2.0, observation(35.0, 1, 2.0), v_ego=10.0)
  assert c.event_id == event_id
  assert c.stop_session_id == session_id
  assert first.remaining_distance > 80.0

  recovered = update(c, 2.5, observation(85.0, 1, 2.5), v_ego=10.0)
  assert c.event_id == event_id
  assert c.stop_session_id == session_id
  assert recovered.phase in c.ACTIVE_PHASES


def test_hold_cannot_move_to_a_discontinuous_red_distance():
  c = controller()
  establish_red(c, distance=5.0, speed=0.0)
  update(c, 2.0, observation(5.0, 1, 2.0), v_ego=0.0)
  assert c.phase == TrafficControlPhase.hold
  session_id = c.stop_session_id

  update(c, 2.5, observation(50.0, 1, 2.5), v_ego=0.0)
  decision = update(c, 3.0, observation(50.0, 1, 3.0), v_ego=0.0)

  assert decision.phase == TrafficControlPhase.hold
  assert decision.should_stop
  assert c.stop_session_id == session_id


def test_route63_style_can_fusion_does_not_hold_ten_meters_early():
  c = controller()
  fixture = json.loads((Path(__file__).parent / "fixtures/route63_regressions.json").read_text())
  sequence = fixture["event6Fusion"]
  t0 = sequence[0]["t"] - 1.0
  last_now = 0.5
  last_speed = 11.5
  last_observation = observation(66.0, 1, 0.5)
  update(c, last_now, last_observation, v_ego=last_speed)
  for sample in sequence:
    now_s = sample["t"] - t0
    distance, speed = sample["rawDistance"], sample["vEgo"]
    # Production calls the controller at 20 Hz even though 0x25D is roughly
    # 2 Hz. Replay duplicate CAN timestamps so ego-station integration uses
    # the real control cadence instead of clamping a multi-second test gap.
    while last_now + 0.05 < now_s - 1e-6:
      last_now += 0.05
      update(c, last_now, last_observation, v_ego=last_speed)
    decision = update(c, now_s, observation(distance, 1, now_s), v_ego=speed)
    last_now = now_s
    last_speed = speed
    last_observation = observation(distance, 1, now_s)
  assert decision.phase != TrafficControlPhase.hold
  assert decision.remaining_distance >= 8.0
  assert abs(decision.remaining_distance - (16.0 - decision.stop_reference)) <= 2.0


def test_driver_gas_bypasses_the_current_event_until_it_is_passed():
  c = controller()
  update(c, 1.0, observation(40.0, 1, 1.0), gas=True)
  update(c, 1.5, observation(35.0, 1, 1.5), gas=False)
  decision = update(c, 2.0, observation(30.0, 1, 2.0), gas=False)
  assert decision.phase == TrafficControlPhase.bypass
  assert not decision.driver_override_active
  assert not decision.apply_constraint
  still_bypassed = update(c, 2.5, observation(25.0, 1, 2.5), gas=False)
  assert still_bypassed.phase == TrafficControlPhase.bypass
  assert not still_bypassed.apply_constraint


def test_pre_event_gas_does_not_poison_a_later_red_event():
  c = controller()

  # Route log regression: gas was pressed while 0x25D reported no in-range
  # signal. The old code latched the whole future event into driver_bypass, so
  # a red appearing later at 106 m was ignored all the way to standstill.
  no_signal = update(c, 1.0, observation(254.0, 0, 1.0), gas=True, v_ego=11.0)
  assert no_signal.phase == TrafficControlPhase.off
  assert no_signal.driver_override_active

  update(c, 2.0, observation(220.0, 0, 2.0), gas=False, v_ego=11.0)
  first_red = update(c, 3.0, observation(106.0, 1, 3.0), gas=False, v_ego=11.0)
  confirmed_red = update(c, 3.5, observation(100.5, 1, 3.5), gas=False, v_ego=11.0)

  assert first_red.phase == TrafficControlPhase.redCandidate
  assert confirmed_red.phase in c.ACTIVE_PHASES
  assert confirmed_red.apply_constraint
  assert confirmed_red.stop_control_allowed


def test_speed_limit_bypasses_the_whole_event_instead_of_rearming_late():
  c = controller(max_control_speed=10.0)
  first = update(c, 1.0, observation(80.0, 1, 1.0), v_ego=12.0)
  assert first.phase == TrafficControlPhase.bypass
  slower = update(c, 1.5, observation(74.0, 1, 1.5), v_ego=8.0)
  assert slower.phase == TrafficControlPhase.bypass
  assert not slower.apply_constraint


def test_turn_signal_does_not_override_current_lane_can_stop():
  c = controller()
  update(c, 1.0, observation(40.0, 1, 1.0), blinker=True)
  decision = update(c, 1.5, observation(35.0, 1, 1.5), blinker=True)
  assert not decision.direction_unknown
  assert decision.active
  assert decision.apply_constraint
  assert decision.stop_control_allowed


def test_turn_signal_never_requires_current_lane_stop_reconfirmation():
  c = controller()
  establish_red(c, distance=40.0, speed=5.0)
  signalled = update(c, 1.6, observation(34.5, 1, 1.6), v_ego=5.0, blinker=True)
  assert signalled.stop_control_allowed
  cached = update(c, 1.7, observation(34.5, 1, 1.6), v_ego=5.0, blinker=False)
  assert cached.stop_control_allowed
  first = update(c, 2.1, observation(32.0, 1, 2.1), v_ego=5.0)
  assert first.stop_control_allowed
  second = update(c, 2.6, observation(29.5, 1, 2.6), v_ego=5.0)
  assert second.stop_control_allowed


def test_discontinuous_red_does_not_gain_special_authority_from_turn_signal_changes():
  c = controller()
  establish_red(c, distance=40.0, speed=5.0)
  update(c, 1.6, observation(34.5, 1, 1.6), v_ego=5.0, blinker=True)
  first = update(c, 2.0, observation(32.5, 1, 2.0), v_ego=5.0, blinker=False)
  assert first.stop_control_allowed
  discontinuous = update(c, 2.5, observation(100.0, 1, 2.5), v_ego=5.0)
  assert discontinuous.stop_control_allowed
  update(c, 3.0, observation(97.5, 1, 3.0), v_ego=5.0)
  continuous = update(c, 3.5, observation(95.0, 1, 3.5), v_ego=5.0)
  assert continuous.stop_control_allowed


def test_far_active_stop_expires_after_raw_can_dropout():
  c = controller()
  establish_red(c, distance=80.0, speed=10.0)
  stale = observation(70.0, 1, 1.5, available=False)
  update(c, 2.0, stale, v_ego=10.0)
  expired = update(c, 3.0, stale, v_ego=10.0)
  assert not expired.stop_control_allowed
  assert expired.phase in c.ACTIVE_PHASES


def test_near_active_stop_has_bounded_dropout_but_hold_remains_latched():
  c = controller()
  establish_red(c, distance=20.0, speed=2.0)
  stale = observation(15.0, 1, 1.5, available=False)
  update(c, 2.0, stale, v_ego=1.0)
  expired = update(c, 4.1, stale, v_ego=1.0)
  assert expired.phase in c.ACTIVE_PHASES
  assert not expired.stop_control_allowed

  held = controller()
  establish_red(held, distance=6.0, speed=0.0)
  update(held, 2.0, observation(6.0, 1, 2.0), v_ego=0.0)
  assert held.phase == TrafficControlPhase.hold
  still_held = update(held, 5.0, observation(6.0, 1, 2.0, available=False), v_ego=0.0)
  assert still_held.phase == TrafficControlPhase.hold


def test_stop_reconfirmation_never_adds_extra_green_go_delay():
  c = controller()
  establish_red(c, distance=30.0, speed=5.0)
  update(c, 1.7, observation(26.5, 1, 1.7), v_ego=5.0, blinker=True)
  first_green = update(c, 2.0, observation(25.0, 2, 2.0), v_ego=5.0, blinker=False)
  second_green = update(c, 2.5, observation(22.5, 2, 2.5), v_ego=5.0, blinker=False)
  assert first_green.phase == TrafficControlPhase.release
  assert second_green.phase == TrafficControlPhase.release


def test_far_red_after_release_starts_a_new_session():
  c = controller()
  establish_red(c, distance=20.0, speed=0.0)
  old_session = c.stop_session_id
  update(c, 2.0, observation(20.0, 2, 2.0), v_ego=0.0)
  update(c, 2.5, observation(20.0, 2, 2.5), v_ego=0.0)
  update(c, 3.0, observation(150.0, 1, 3.0), v_ego=10.0)
  update(c, 3.5, observation(145.0, 1, 3.5), v_ego=10.0)
  assert c.stop_session_id != old_session


def test_same_target_red_flicker_after_release_preserves_session():
  c = controller()
  establish_red(c, distance=20.0, speed=0.0)
  old_session = c.stop_session_id
  update(c, 2.0, observation(20.0, 2, 2.0), v_ego=0.0)
  update(c, 2.5, observation(20.0, 2, 2.5), v_ego=0.0)
  first_red = update(c, 3.0, observation(19.0, 1, 3.0), v_ego=0.0)
  second_red = update(c, 3.5, observation(18.0, 1, 3.5), v_ego=0.0)

  assert first_red.phase == TrafficControlPhase.release
  assert second_red.phase in c.ACTIVE_PHASES
  assert c.stop_session_id == old_session


def test_flashing_green_latch_clears_after_passed_before_new_red():
  c = controller()
  for now_s, distance, light in ((1.0, 80, 2), (1.1, 79.2, 2), (1.2, 78.4, 4),
                                 (1.7, 74.4, 2), (2.2, 70.4, 4),
                                 (2.7, 66.4, 2), (3.2, 62.4, 4), (3.4, 60.8, 4)):
    update(c, now_s, observation(distance, light, now_s), v_ego=8.0)
  assert c.flash_latched
  now_s = 3.2
  for distance in (65.4, 60.4, 55.4, 50.4, 45.4, 40.4, 35.4, 30.4, 25.4, 20.4, 15.4, 10.4, 5.4, 1.0):
    now_s += 0.5
    update(c, now_s, observation(distance, 1, now_s), v_ego=10.0)
  update(c, now_s + 0.1, observation(254.0, 1, now_s + 0.1), v_ego=10.0)
  assert not c.flash_latched
  update(c, now_s + 1.0, observation(50.0, 1, now_s + 1.0), v_ego=5.0)
  decision = update(c, now_s + 1.5, observation(47.5, 1, now_s + 1.5), v_ego=5.0)
  assert decision.phase in (TrafficControlPhase.approachRed, TrafficControlPhase.braking)


def test_driver_gas_temporarily_suppresses_control_and_disabled_longitudinal_resets():
  c = controller()
  establish_red(c)
  override = update(c, 2.0, observation(70, 1, 2.0), gas=True)
  assert override.driver_override_active
  assert not override.apply_constraint
  reset = update(c, 20.0, observation(60, 1, 20.0), enabled=False)
  assert reset.phase == TrafficControlPhase.off
