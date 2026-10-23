from types import SimpleNamespace as NS

import pytest

from openpilot.cereal import log
from openpilot.selfdrive.controls.tests.test_desire_helper_stalk import TestDesireHelperStalkRequest as StalkFixture


@pytest.fixture
def setup():
  fixture = StalkFixture()
  fixture.setup_method()
  fixture.helper.tesla_auto_signals = True
  fixture.carrot_man.atcType = "fork left"
  return fixture


def step(f, lamp=False, owned=0, driver=0, active=True, prob=.5):
  cs = NS(canValid=True, leftBlinker=lamp, rightBlinker=False, vEgo=25., aEgo=0., trailerConnected=False,
          steeringTorque=0., steeringPressed=False, leftBlinkerStalkCount=0, rightBlinkerStalkCount=0,
          turnSignalDriverValid=True, turnSignalDriverDirection=driver, autoTurnSignalDirection=owned)
  f.helper.update(cs, NS(), active, prob, f.carrot_man, NS())
  return f.helper


def test_automatic_lamp_never_substitutes_driver_confirmation(setup):
  h = step(setup)
  h = step(setup)
  assert h.lane_change_state == log.LaneChangeState.preLaneChange
  assert h.auto_signal_direction == log.LaneChangeDirection.left
  request = h.auto_signal_request_id
  for _ in range(5):
    h = step(setup, lamp=True, owned=1)
    assert h.lane_change_state == log.LaneChangeState.preLaneChange
    assert h.auto_signal_request_id == request


def test_cancellation_tail_cannot_start_a_second_change(setup):
  setup.carrot_man.atcType = ""
  for _ in range(4):
    h = step(setup, lamp=True, owned=1)
    assert h.lane_change_state == log.LaneChangeState.off
    assert h.auto_signal_request_id == 0


def test_real_driver_still_confirms_despite_owned_feedback(setup):
  step(setup)
  step(setup)
  h = step(setup, lamp=True, owned=1, driver=1)
  assert h.lane_change_state == log.LaneChangeState.laneChangeStarting
  assert h.auto_signal_request_id == 0


def test_turn_desire_can_request_lamp_while_lane_change_is_off(setup):
  setup.carrot_man.atcType = "turn left"
  setup.helper.left.dist_to_edge_far = 5.
  step(setup)
  h = step(setup)
  assert h.desire == log.Desire.turnLeft
  assert h.lane_change_state == log.LaneChangeState.off
  assert h.auto_signal_direction == log.LaneChangeDirection.left
  h = step(setup, active=False)
  assert not h.auto_signal_request_id


def test_finishing_does_not_cancel_early_and_completed_pre_does_not_rearm(setup):
  step(setup)
  step(setup)
  h = setup.helper
  h.lane_change_state = log.LaneChangeState.laneChangeFinishing
  h.lane_change_direction = log.LaneChangeDirection.left
  h.lane_change_ll_prob = .5
  step(setup)
  assert h.auto_signal_request_id
  h.lane_change_ll_prob = .99
  step(setup)
  assert h.lane_change_state == log.LaneChangeState.preLaneChange
  assert h.auto_signal_request_id == 0
  step(setup, lamp=True, owned=1)
  assert h.auto_signal_request_id == 0


def test_pre_turn_signal_does_not_enable_a_maneuver(setup):
  setup.carrot_man.atcType = "turn left prepare"
  setup.carrot_man.xTurnInfo = 1
  setup.carrot_man.xDistToTurn = 100
  h = step(setup)
  assert h.desire == log.Desire.none
  assert h.lane_change_state == log.LaneChangeState.off
  assert h.auto_signal_direction == log.LaneChangeDirection.left


def test_driver_rejection_does_not_rearm_the_same_atc_pre_signal(setup):
  setup.carrot_man.atcType = "atc left"
  setup.carrot_man.xTurnInfo = 1
  setup.carrot_man.xDistToTurn = 100
  step(setup)
  step(setup)
  step(setup, driver=2)
  assert setup.helper.atc_active == 2
  assert not step(setup).auto_signal_request_id


def test_completion_latch_survives_off_until_a_new_navigation_request(setup):
  setup.carrot_man.atcType = "atc left"
  setup.carrot_man.xTurnInfo = 1
  setup.carrot_man.xDistToTurn = 100
  step(setup)
  h = step(setup)
  h.lane_change_state = log.LaneChangeState.laneChangeFinishing
  h.lane_change_ll_prob = .99
  step(setup)
  assert h.auto_signal_request_id == 0
  step(setup, active=False)
  assert not step(setup).auto_signal_request_id
  setup.carrot_man.atcType = ""
  step(setup)
  setup.carrot_man.atcType = "turn left prepare"
  assert step(setup).auto_signal_request_id
