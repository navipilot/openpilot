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


def step(f, lamp=False, owned=0, driver=0, active=True, prob=.5, valid=True, trailer=False):
  cs = NS(canValid=valid, leftBlinker=lamp and owned != 2, rightBlinker=lamp and owned == 2, vEgo=25., aEgo=0., trailerConnected=trailer,
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


@pytest.mark.parametrize("argument,direction", [("LEFT", 1), ("RIGHT", 2)])
def test_app_short_command_lamp_lasts_until_fsm_completion(setup, argument, direction):
  setup.carrot_man.atcType = ""
  setup.carrot_man.carrotCmd = "LANECHANGE"
  setup.carrot_man.carrotCmdIndex = 1
  setup.carrot_man.carrotArg = argument
  h = step(setup)
  request = h.auto_signal_request_id
  assert request
  for _ in range(12):
    h = step(setup)
    assert h.lane_change_state == log.LaneChangeState.laneChangeStarting
    assert h.auto_signal_direction == direction
    assert h.auto_signal_request_id == request
  assert h.carrot_lane_change_count == 0
  for _ in range(40):
    h = step(setup, prob=0.)
    if h.lane_change_state == log.LaneChangeState.off:
      break
    assert h.auto_signal_direction == direction
    assert h.auto_signal_request_id == request
  assert h.lane_change_state == log.LaneChangeState.off
  assert not h.auto_signal_request_id
  for _ in range(5):
    assert not step(setup, lamp=True, owned=direction).auto_signal_request_id
  setup.carrot_man.carrotCmdIndex = 2
  assert step(setup).auto_signal_request_id > request


@pytest.mark.parametrize("gate", ["lateral", "driver", "can", "trailer"])
def test_app_latch_does_not_survive_cancellation_or_resume_same_session(setup, gate):
  setup.carrot_man.atcType = ""
  setup.carrot_man.carrotCmd = "LANECHANGE"
  setup.carrot_man.carrotCmdIndex = 1
  setup.carrot_man.carrotArg = "LEFT"
  for _ in range(8):
    step(setup)
  h = step(setup, active=gate != "lateral", driver=2 if gate == "driver" else 0,
           valid=gate != "can", trailer=gate == "trailer")
  assert not h.auto_signal_request_id
  assert not step(setup).auto_signal_request_id


@pytest.mark.parametrize("argument,direction", [("LEFT", 1), ("RIGHT", 2)])
def test_app_can_send_when_vehicle_feedback_arrives_after_command_pulse(setup, argument, direction):
  from openpilot.selfdrive.car.tests.test_tesla_turn_signals import N, feedback, state
  from openpilot.selfdrive.car.tesla_turn_signals import TeslaTurnSignals

  setup.carrot_man.atcType = ""
  setup.carrot_man.carrotCmd = "LANECHANGE"
  setup.carrot_man.carrotCmdIndex = 1
  setup.carrot_man.carrotArg = argument
  core = TeslaTurnSignals(True)
  for frame in range(12):
    h = step(setup)
    meta = log.ModelDataV2.new_message().meta
    meta.autoTurnSignalDirection = h.auto_signal_direction
    meta.autoTurnSignalRequestId = h.auto_signal_request_id
    now = N + frame * 50_000_000
    if frame == 7:
      assert h.carrot_lane_change_count == 0
      feedback(core, now)
    sends = core.update(state(), NS(latActive=True), meta.as_reader(), True, now)
    if frame < 7:
      assert not sends and core.session is None
    elif frame == 7:
      assert len(sends) == 1 and sends[0].dat[1] & 3 == direction
    else:
      assert core.session.direction == direction and not core.session.cancelling


@pytest.mark.parametrize("side,direction", [("left", 1), ("right", 2)])
def test_turn_preparation_to_turn_keeps_one_lamp_session(setup, side, direction):
  setup.carrot_man.atcType = f"turn {side} prepare"
  setup.carrot_man.xTurnInfo = direction
  setup.carrot_man.xDistToTurn = 100
  getattr(setup.helper, side).dist_to_edge_far = 5.
  h = step(setup)
  request = h.auto_signal_request_id
  assert request and h.auto_signal_direction == direction
  setup.carrot_man.atcType = f"turn {side}"
  h = step(setup)
  assert h.desire == log.Desire.none  # Keep the original one-frame maneuver suppression.
  assert h.auto_signal_request_id == request
  h = step(setup)
  assert h.desire == (log.Desire.turnLeft if direction == 1 else log.Desire.turnRight)
  assert h.auto_signal_direction == direction and h.auto_signal_request_id == request
  h.turn_disable_count = 10
  assert not step(setup).auto_signal_request_id
