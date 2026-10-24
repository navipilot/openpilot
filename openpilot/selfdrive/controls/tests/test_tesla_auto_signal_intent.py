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


@pytest.mark.parametrize("size,position,direction", [
  (3, 1, 2), (3, 2, 2), (3, 3, 1), (3, 4, 1),
  (4, 1, 2), (4, 2, 2), (4, 3, 2), (4, 4, 2),
  (4, 5, 1), (4, 6, 1), (4, 7, 1), (4, 8, 1),
])
@pytest.mark.parametrize("bus", [0, 1])
@pytest.mark.parametrize("release_in_batch", [False, True])
def test_received_stalk_frame_selects_matching_lane_change(setup, monkeypatch, size, position, direction, bus, release_in_batch):
  from opendbc.can import CANPacker
  from opendbc.car.tesla.interface import CarInterface
  from opendbc.car.tesla.tests.test_auto_turn_signals import N, params, stalk_frame

  ci = CarInterface(params())
  setup.carrot_man.atcType = ""
  h = setup.helper
  packer = CANPacker("tesla_model3_party")
  monkeypatch.setattr("opendbc.car.tesla.carstate.time.clock_gettime_ns", lambda clock: N)

  def receive(positions, lamp_direction=0):
    frames = [stalk_frame(p, bus, size) for p in positions]
    frames.append(packer.make_can_msg("UI_warning", 0, {"leftBlinkerBlinking": 2 if lamp_direction == 1 else 0,
                                                      "rightBlinkerBlinking": 2 if lamp_direction == 2 else 0}))
    cs = ci.update([(N, frames)])
    cs.canValid, cs.vEgo = True, 25.
    h.update(cs, NS(), True, .5, setup.carrot_man, NS())

  receive([0])
  receive([position, 0] if release_in_batch else [position], direction)
  assert h.lane_change_state == log.LaneChangeState.preLaneChange
  assert h.driver_blinker_state == direction
  receive([0] if release_in_batch else [position], direction)
  assert h.lane_change_state == log.LaneChangeState.laneChangeStarting
  assert h.lane_change_direction == direction


@pytest.mark.parametrize("invalid", ["sna", "crc", "dlc"])
def test_untrusted_stalk_cannot_start_a_manual_lane_change(setup, monkeypatch, invalid):
  from opendbc.car.tesla.interface import CarInterface
  from opendbc.car.tesla.tests.test_auto_turn_signals import N, params, stalk_frame

  ci = CarInterface(params())
  setup.carrot_man.atcType = ""
  frame = stalk_frame(9 if invalid == "sna" else 4)
  if invalid == "crc":
    frame = (frame[0], bytes((frame[1][0] ^ 1,)) + frame[1][1:], frame[2])
  elif invalid == "dlc":
    frame = (frame[0], frame[1][:2], frame[2])
  monkeypatch.setattr("opendbc.car.tesla.carstate.time.clock_gettime_ns", lambda clock: N)
  for packet in (stalk_frame(0), frame, frame):
    cs = ci.update([(N, [packet])])
    cs.canValid, cs.vEgo = True, 25.
    setup.helper.update(cs, NS(), True, .5, setup.carrot_man, NS())
  assert setup.helper.lane_change_state == log.LaneChangeState.off
  assert not setup.helper.auto_signal_request_id
  assert cs.leftBlinkerStalkCount == cs.rightBlinkerStalkCount == 0


@pytest.mark.parametrize("valid,counts", [(True, (1, 1)), (False, (1, 0)), (False, (0, 1))])
def test_ambiguous_or_invalid_counter_changes_never_default_to_left(setup, valid, counts):
  cs = NS(leftBlinker=False, rightBlinker=False, autoTurnSignalDirection=0, turnSignalDriverDirection=0,
          turnSignalDriverValid=True, leftBlinkerStalkCount=0, rightBlinkerStalkCount=0)
  h = setup.helper
  assert h._update_driver_blinker(cs)[0] == h._update_stalk(cs) == 0
  cs.turnSignalDriverValid = valid
  cs.leftBlinkerStalkCount, cs.rightBlinkerStalkCount = counts
  assert h._update_driver_blinker(cs)[0] == h._update_stalk(cs) == 0


def step(f, lamp=False, owned=0, driver=0, active=True, prob=.5, valid=True, trailer=False, model=None, speed=25., angle=0.):
  cs = NS(canValid=valid, leftBlinker=lamp and owned != 2, rightBlinker=lamp and owned == 2, vEgo=speed, aEgo=0., trailerConnected=trailer,
          steeringAngleDeg=angle,
          steeringTorque=0., steeringPressed=False, leftBlinkerStalkCount=0, rightBlinkerStalkCount=0,
          turnSignalDriverValid=True, turnSignalDriverDirection=driver, autoTurnSignalDirection=owned)
  f.helper.update(cs, model or NS(), active, prob, f.carrot_man, NS())
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
  for _ in range(120):
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
  assert step(setup).auto_signal_request_id == request


def test_unknown_lamp_is_not_driver_even_without_an_automatic_owner(setup):
  setup.carrot_man.atcType = ""
  for _ in range(5):
    h = step(setup, lamp=True)
    assert h.driver_blinker_state == 0
    assert h.lane_change_state == log.LaneChangeState.off


@pytest.mark.parametrize("driver", [1, 2])
@pytest.mark.parametrize("opposite_tail", [False, True])
def test_manual_provenance_survives_release_and_fast_tap(setup, driver, opposite_tail):
  h = setup.helper
  cs = NS(leftBlinker=False, rightBlinker=False, turnSignalDriverValid=True, turnSignalDriverDirection=0,
          leftBlinkerStalkCount=0, rightBlinkerStalkCount=0)
  assert h._update_driver_blinker(cs)[0] == 0
  setattr(cs, "leftBlinkerStalkCount" if driver == 1 else "rightBlinkerStalkCount", 1)
  assert h._update_driver_blinker(cs)[0] == driver
  cs.leftBlinker, cs.rightBlinker = opposite_tail and driver == 2, opposite_tail and driver == 1
  for _ in range(5):
    assert h._update_driver_blinker(cs)[0] == 0  # The physical lamp can arrive after a quick release.
  cs.leftBlinker, cs.rightBlinker = driver == 1, driver == 2
  for _ in range(30):
    assert h._update_driver_blinker(cs)[0] == driver
  cs.leftBlinker = cs.rightBlinker = False
  for _ in range(12):
    assert h._update_driver_blinker(cs)[0] == 0
  cs.leftBlinker, cs.rightBlinker = driver == 1, driver == 2
  assert h._update_driver_blinker(cs)[0] == 0


@pytest.mark.parametrize("direction", [1, 2])
def test_real_turn_disable_does_not_complete_lamps_until_started_and_stably_straight(setup, direction):
  from openpilot.selfdrive.controls.lib.desire_helper import DesireHelper

  h = setup.helper
  sign = 1 if direction == 1 else -1
  setup.carrot_man.atcType = "turn left" if direction == 1 else "turn right"
  getattr(h, "left" if direction == 1 else "right").dist_to_edge_far = 5.
  step(setup)
  request = step(setup).auto_signal_request_id
  model = NS(meta=NS(desireState=[0.] * 5), orientationRate=NS(z=[sign * .4] * 16), orientation=NS(z=[sign * .2] * 16))
  h._check_desire_state = DesireHelper._check_desire_state.__get__(h)
  model.orientationRate.z[15] = sign * .2
  for _ in range(10):
    step(setup, model=model, speed=5., angle=sign * 100.)
    assert h.turn_disable_count == 200  # Original steering behavior is unchanged.
    assert h.auto_signal_request_id == request
  for speed, angle, yaw in ((0., 0., 0.), (.5, 100., .02), (2., 70., .08)):
    model.orientationRate.z = [sign * yaw] * 16
    for _ in range(30):
      assert step(setup, model=model, speed=speed, angle=sign * angle).auto_signal_request_id == request
  model.orientationRate.z = [sign * .01] * 16
  for _ in range(19):
    assert step(setup, model=model, speed=5., angle=sign * 10.).auto_signal_request_id == request
  assert not step(setup, model=model, speed=5., angle=sign * 10.).auto_signal_request_id
  for _ in range(10):
    assert not step(setup, model=model, speed=5., angle=sign * 10.).auto_signal_request_id


@pytest.mark.parametrize("direction", [1, 2])
@pytest.mark.parametrize("reader", [False, True], ids=["builder", "reader"])
def test_turn_lifecycle_with_real_capnp_model_lists(setup, direction, reader):
  sign = 1 if direction == 1 else -1
  setup.carrot_man.atcType = "turn left" if direction == 1 else "turn right"
  getattr(setup.helper, "left" if direction == 1 else "right").dist_to_edge_far = 5.
  model = log.ModelDataV2.new_message()
  model.meta.desireState = [0.] * 8
  model.orientationRate.z = [sign * .2] * 33
  model.orientation.z = [sign * .2] * 33
  step(setup, model=model.as_reader() if reader else model, speed=5., angle=sign * 90.)
  request = step(setup, model=model.as_reader() if reader else model, speed=5., angle=sign * 90.).auto_signal_request_id
  assert request

  for _ in range(10):
    h = step(setup, model=model.as_reader() if reader else model, speed=5., angle=sign * 90.)
    assert h.auto_signal_direction == direction
    assert h.auto_signal_request_id == request
  assert h._auto_turn_started >= .3

  model.orientationRate.z = [sign * .01] * 33
  for _ in range(19):
    assert step(setup, model=model.as_reader() if reader else model, speed=5., angle=sign * 10.).auto_signal_request_id == request
  assert not step(setup, model=model.as_reader() if reader else model, speed=5., angle=sign * 10.).auto_signal_request_id
  for _ in range(10):
    assert not step(setup, model=model.as_reader() if reader else model, speed=5., angle=sign * 10.).auto_signal_request_id


@pytest.mark.parametrize("reader", [False, True], ids=["builder", "reader"])
@pytest.mark.parametrize("invalid", ["short_rates", "short_headings", "nan_rate", "nan_heading", "angle", "speed"])
def test_real_capnp_invalid_geometry_cannot_complete_turn(setup, reader, invalid):
  h = setup.helper
  h._auto_turn_direction = log.LaneChangeDirection.left
  h._auto_turn_started = .3
  h._auto_turn_peak_angle = 90.
  h._auto_turn_settled = .95
  model = log.ModelDataV2.new_message()
  model.orientationRate.z = [.01] * (15 if invalid == "short_rates" else 33)
  model.orientation.z = [0.] * (15 if invalid == "short_headings" else 33)
  if invalid == "nan_rate":
    model.orientationRate.z[0] = float("nan")
  elif invalid == "nan_heading":
    model.orientation.z[15] = float("nan")
  cs = NS(vEgo=float("inf") if invalid == "speed" else 5., steeringAngleDeg=float("nan") if invalid == "angle" else 5.)
  for _ in range(40):
    h._update_auto_turn_geometry(cs, model.as_reader() if reader else model)
    assert h._auto_turn_direction == log.LaneChangeDirection.left
    assert not h._auto_turn_consumed
    assert h._auto_turn_settled == 0.


@pytest.mark.parametrize("direction", [1, 2])
def test_real_turn_model_roundtrip_reaches_lamp_controller(setup, direction):
  from openpilot.selfdrive.car.tests.test_tesla_turn_signals import N, echo, feedback, state
  from openpilot.selfdrive.car.tesla_turn_signals import TeslaTurnSignals

  sign = 1 if direction == 1 else -1
  setup.carrot_man.atcType = "turn left" if direction == 1 else "turn right"
  getattr(setup.helper, "left" if direction == 1 else "right").dist_to_edge_far = 5.
  signals = TeslaTurnSignals(True)
  lamp_on = cancelled = False
  active_sends = 0
  for frame in range(90):
    now = N + frame * 50_000_000
    turning = frame < 45
    model = log.ModelDataV2.new_message()
    model.orientationRate.z = [sign * (.2 if turning else .01)] * 33
    model.orientation.z = [sign * .2] * 33
    h = step(setup, model=model, speed=5., angle=sign * (90. if turning else 10.))
    model.meta.autoTurnSignalDirection = h.auto_signal_direction
    model.meta.autoTurnSignalRequestId = h.auto_signal_request_id
    ui = (2, 0) if direction == 1 else (0, 2)
    front = (1, 0) if direction == 1 else (0, 1)
    feedback(signals, now, ui=ui if lamp_on else (0, 0), front=front if lamp_on else (0, 0))
    with log.ModelDataV2.from_bytes(model.to_bytes()) as received:
      sends = signals.update(state(), NS(latActive=True), received.meta, True, now)
    for send in sends:
      if send.dat[1] & 3 == direction:
        assert not cancelled
        active_sends += 1
        lamp_on = True
      else:
        assert send.dat[1] & 3 == 3 and not h.auto_signal_request_id
        cancelled = True
        lamp_on = False
      echo(signals, send, now + 1)
  assert active_sends >= 40
  assert cancelled and not lamp_on
  assert signals.status == "cancelled" and signals.session is None
  assert not h.auto_signal_request_id


@pytest.mark.parametrize("navigation_type", ["", "none", "none prepare", "straight", "straight prepare"])
@pytest.mark.parametrize("direction", [1, 2])
def test_turn_navigation_can_end_before_geometry_settles_and_abort_does_not_relight(setup, navigation_type, direction):
  sign = 1 if direction == 1 else -1
  setup.carrot_man.atcType = "turn left" if direction == 1 else "turn right"
  getattr(setup.helper, "left" if direction == 1 else "right").dist_to_edge_far = 5.
  step(setup)
  request = step(setup).auto_signal_request_id
  model = NS(orientationRate=NS(z=[sign * .2] * 16), orientation=NS(z=[sign * .2] * 16))
  for _ in range(10):
    assert step(setup, model=model, speed=4., angle=sign * 90.).auto_signal_request_id == request
  setup.carrot_man.atcType = navigation_type
  for _ in range(100):
    assert step(setup, model=model, speed=4., angle=sign * 90.).auto_signal_request_id == request
  assert not step(setup, model=model, active=False).auto_signal_request_id
  assert not step(setup, model=model).auto_signal_request_id


@pytest.mark.parametrize("navigation_type", ["", "none", "none prepare", "straight", "straight prepare"])
def test_navigation_exit_lamp_hold_finishes_after_stable_geometry(setup, navigation_type):
  setup.carrot_man.atcType = "turn left"
  setup.helper.left.dist_to_edge_far = 5.
  step(setup)
  request = step(setup).auto_signal_request_id
  model = NS(orientationRate=NS(z=[.2] * 16), orientation=NS(z=[.2] * 16))
  for _ in range(10):
    step(setup, model=model, speed=4., angle=90.)
  setup.carrot_man.atcType = navigation_type
  assert step(setup, model=model, speed=4., angle=90.).auto_signal_request_id == request
  model.orientationRate.z = [.01] * 16
  for _ in range(19):
    assert step(setup, model=model, speed=4., angle=10.).auto_signal_request_id == request
  assert not step(setup, model=model, speed=4., angle=10.).auto_signal_request_id
  for _ in range(10):
    assert not step(setup, model=model, speed=4., angle=10.).auto_signal_request_id


@pytest.mark.parametrize("direction", [1, 2])
def test_opposite_navigation_turn_starts_a_new_request_without_reusing_geometry(setup, direction):
  h = setup.helper
  sign = 1 if direction == 1 else -1
  setup.carrot_man.atcType = "turn left" if direction == 1 else "turn right"
  h.left.dist_to_edge_far = h.right.dist_to_edge_far = 5.
  step(setup)
  request = step(setup).auto_signal_request_id
  model = NS(orientationRate=NS(z=[sign * .2] * 16), orientation=NS(z=[sign * .2] * 16))
  for _ in range(10):
    step(setup, model=model, speed=4., angle=sign * 90.)
  setup.carrot_man.atcType = "turn right" if direction == 1 else "turn left"
  setup.carrot_man.xTurnInfo = 3 - direction
  step(setup, model=model, speed=4., angle=sign * 90.)
  for _ in range(10):
    step(setup, model=model, speed=4., angle=sign * 90.)
    assert h.auto_signal_direction == 3 - direction
    assert h.auto_signal_request_id > request
    assert h._auto_turn_started == 0.
  model.orientationRate.z = [-sign * .2] * 16
  for _ in range(10):
    step(setup, model=model, speed=4., angle=-sign * 90.)
  assert h._auto_turn_started >= .3


@pytest.mark.parametrize("direction", [1, 2])
@pytest.mark.parametrize("opposite_input", ["angle", "yaw"])
def test_turn_start_geometry_must_match_requested_direction(setup, direction, opposite_input):
  sign = 1 if direction == 1 else -1
  setup.carrot_man.atcType = "turn left" if direction == 1 else "turn right"
  getattr(setup.helper, "left" if direction == 1 else "right").dist_to_edge_far = 5.
  step(setup)
  request = step(setup).auto_signal_request_id
  model = NS(orientationRate=NS(z=[sign * (-.2 if opposite_input == "yaw" else .2)] * 16), orientation=NS(z=[0.] * 16))
  for _ in range(10):
    step(setup, model=model, speed=4., angle=sign * (-90. if opposite_input == "angle" else 90.))
  assert setup.helper._auto_turn_started == 0.
  model.orientationRate.z = [.01] * 16
  for _ in range(30):
    assert step(setup, model=model, speed=4., angle=5.).auto_signal_request_id == request


@pytest.mark.parametrize("geometry", ["missing", "nan", "heading", "not_started"])
def test_turn_completion_requires_valid_started_and_road_geometry(setup, geometry):
  setup.carrot_man.atcType = "turn left"
  setup.helper.left.dist_to_edge_far = 5.
  step(setup)
  request = step(setup).auto_signal_request_id
  model = NS(orientationRate=NS(z=[.2] * 16), orientation=NS(z=[.2] * 16))
  if geometry != "not_started":
    for _ in range(10):
      step(setup, model=model, speed=4., angle=90.)
  model.orientationRate.z = [.01] * 16
  if geometry == "missing":
    model.orientation.z = []
  elif geometry == "nan":
    model.orientationRate.z[0] = float("nan")
  elif geometry == "heading":
    model.orientation.z[15] = .5
  for _ in range(40):
    assert step(setup, model=model, speed=4., angle=5.).auto_signal_request_id == request
