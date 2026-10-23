from types import SimpleNamespace as NS

import pytest
from opendbc.can import CANPacker
from openpilot.cereal import car, log
from openpilot.selfdrive.car.tesla_turn_signals import (
  BODY, CANCEL_NS, CONTEXT_NS, FRONT, SESSION_NS, UI, TeslaTurnSignals, body_checksum, body_frame,
)

N = 10_000_000_000


def template(counter=4):
  data = bytearray.fromhex("a58c61b45ac34700")
  data[6] = (counter << 4) | 7
  data[7] = body_checksum(data)
  return bytes(data)


def state(**kwargs):
  values = {"leftBlinkerStalkCount": 0, "rightBlinkerStalkCount": 0, "turnSignalDriverValid": True,
            "turnSignalDriverDirection": 0, "canValid": True, "gearShifter": "drive"}
  return NS(**(values | kwargs))


def intent(direction=1, request=1):
  meta = log.ModelDataV2.new_message().meta
  meta.autoTurnSignalDirection = direction
  meta.autoTurnSignalRequestId = request
  return meta.as_reader()


def feedback(c, now, ui=(0, 0), front=(0, 0), body=True):
  p = CANPacker("tesla_model3_party")
  v = CANPacker("tesla_model3_vehicle")
  frames = [p.make_can_msg("UI_warning", 0, {"leftBlinkerBlinking": ui[0], "rightBlinkerBlinking": ui[1]}),
            v.make_can_msg("ID3F5VCFRONT_lighting", 1,
                          {"VCFRONT_turnSignalLeftStatus": front[0], "VCFRONT_turnSignalRightStatus": front[1]})]
  if body:
    frames.append((BODY, template(), 1))
  c.observe([(now, frames)], now)


def update(c, now=N, direction=1, request=1, fresh=True, cs=None, active=True):
  return c.update(cs or state(), NS(latActive=active), intent(direction, request), fresh, now)


def echo(c, frame, now, source=0x81):
  c.observe([(now, [(frame.address, frame.dat, source)])], now)


def opened(direction=1):
  c = TeslaTurnSignals(True)
  feedback(c, N)
  frame = update(c, direction=direction)[0]
  echo(c, frame, N + 1)
  feedback(c, N + 2, ui=(2, 0) if direction == 1 else (0, 2),
           front=(1, 0) if direction == 1 else (0, 1))
  assert c.session.on_seen
  return c


def cancelling(direction=1):
  c = opened(direction)
  sent = update(c, N + 3, direction=0)[0]
  assert sent.dat[1] & 3 == 3
  echo(c, sent, N + 4)
  return c


@pytest.mark.parametrize("direction", [1, 2])
def test_full_lamp_cycle_requires_stable_ui_and_physical_off(direction):
  c = cancelling(direction)
  for dt in (10, 200_000_010, 400_000_010, 600_000_010):
    feedback(c, N + dt)
  assert c.session is None
  assert c.status == "cancelled"
  assert not update(c, N + 600_000_011, direction=direction)  # consumed request
  assert update(c, N + 600_000_012, direction=direction, request=2)


@pytest.mark.parametrize("front", [0, 2, 3])
def test_flashing_dark_phase_fault_and_sna_never_confirm_cancellation(front):
  c = cancelling()
  for dt in (10, 200_000_010, 400_000_010, 600_000_010):
    feedback(c, N + dt, ui=(1, 0), front=(front, 0))
  assert c.session is not None
  assert c.status != "cancelled"
  c.advance(N + CANCEL_NS + 100)
  assert c.status == "fault:cancel_not_confirmed"
  assert c.owned_direction == 1
  assert not c.take_sends(N + CANCEL_NS + 101)


def test_independent_receive_clock_cancels_when_controlsd_stops():
  c = opened()
  c.advance(N + CONTEXT_NS + 1)
  assert c.session.cancelling
  assert c.take_sends(N + CONTEXT_NS + 2, cancel_only=True)[0].dat[1] & 3 == 3


@pytest.mark.parametrize("gate", ["disabled", "model", "lateral", "can", "gear", "driver", "stalk", "no_template"])
def test_start_gates(gate):
  c = TeslaTurnSignals(gate != "disabled")
  feedback(c, N, body=gate != "no_template")
  cs = state(canValid=gate != "can", gearShifter="park" if gate == "gear" else "drive",
             turnSignalDriverValid=gate != "stalk", turnSignalDriverDirection=1 if gate == "driver" else 0)
  assert not update(c, fresh=gate != "model", active=gate != "lateral", cs=cs)


@pytest.mark.parametrize("direction", [1, 2, 3])
def test_body_only_changes_owned_fields(direction):
  original = template(15)
  sent = body_frame(original, direction)
  assert sent[0] == original[0] and sent[3:6] == original[3:6]
  assert sent[1] & 0xFC == original[1] & 0xFC
  assert sent[2] & 0xE1 == original[2] & 0xE1
  assert sent[6] == 7
  assert sent[7] == body_checksum(sent)


def test_bad_template_and_reused_template_are_rejected():
  with pytest.raises(ValueError):
    body_frame(bytes(8), 1)
  c = opened()
  assert c.take_sends(N + 3)
  assert not c.take_sends(N + 4)


def test_driver_takes_ownership_without_waiting_for_manual_lamp_to_go_dark():
  c = opened()
  sends = update(c, N + 3, cs=state(leftBlinkerStalkCount=1, turnSignalDriverDirection=1))
  assert c.owned_direction == 0
  echo(c, sends[0], N + 4)
  assert c.session is None and c.status == "driver_takeover"


def test_reverse_request_cancels_before_other_direction():
  c = opened()
  sends = update(c, N + 3, direction=2, request=2)
  assert len(sends) == 1 and sends[0].dat[1] & 3 == 3
  assert c.session.direction == 1


def test_session_limit_requests_cancel_even_with_fresh_controls():
  c = opened()
  now = N + SESSION_NS
  feedback(c, now, ui=(2, 0), front=(1, 0))
  assert c.session.cancelling  # no control update can revive an expired lease
  assert update(c, now)[0].dat[1] & 3 == 3


def test_schema_defaults_disable_automatic_provenance():
  assert car.CarState.new_message().autoTurnSignalDirection == 0
  assert not car.CarState.new_message().turnSignalDriverValid
  assert log.ModelDataV2.new_message().meta.autoTurnSignalRequestId == 0


def test_ui_decoder_uses_real_dbc_and_does_not_accept_corrupt_checksum():
  c = TeslaTurnSignals(True)
  feedback(c, N, ui=(2, 1))
  assert c.ui == (2, 1)
  c.observe([(N + 1, [(UI, bytes(7), 0)])], N + 1)
  assert c.ui_time == N
  assert FRONT != BODY


def test_real_capnp_inputs_and_can_serialization():
  from openpilot.selfdrive.pandad import can_capnp_to_list, can_list_to_can_capnp

  c = TeslaTurnSignals(True)
  feedback(c, N)
  cs = car.CarState.new_message(canValid=True, gearShifter="drive", turnSignalDriverValid=True)
  cc = car.CarControl.new_message(latActive=True)
  meta = log.ModelDataV2.new_message().meta
  meta.autoTurnSignalDirection = "left"
  meta.autoTurnSignalRequestId = 42
  sends = c.update(cs.as_reader(), cc.as_reader(), meta.as_reader(), True, N)
  packet = can_list_to_can_capnp(sends, msgtype="sendcan", valid=True)
  assert can_capnp_to_list([packet], msgtype="sendcan")[0][1] == [tuple(sends[0])]


@pytest.mark.parametrize("gate", ["model", "lateral", "can", "gear", "stalk"])
def test_loss_of_any_start_gate_requests_cancel_instead_of_relighting(gate):
  c = opened()
  sends = update(c, N + 3, fresh=gate != "model", active=gate != "lateral",
                 cs=state(canValid=gate != "can", gearShifter="park" if gate == "gear" else "drive",
                          turnSignalDriverValid=gate != "stalk"))
  assert sends[0].dat[1] & 3 == 3
  assert c.owned_direction == 1


def test_lost_echo_allows_bounded_cancel_retries_then_latches_fault():
  c = opened()
  sends = update(c, N + 3, direction=0)
  assert sends[0].dat[1] & 3 == 3
  for attempt in (1, 2):
    now = N + 3 + attempt * 1_200_000_001
    feedback(c, now)
    assert c.take_sends(now)[0].dat[1] & 3 == 3
  feedback(c, N + 3_600_000_010)
  assert not c.take_sends(N + 3_600_000_010)
  c.advance(N + CANCEL_NS + 4)
  assert c.failed
  assert c.owned_direction == 1
  for dt in (5_100_000_010, 5_300_000_010, 5_500_000_010, 5_700_000_010):
    feedback(c, N + dt)
  assert c.owned_direction == 0
  assert c.failed and c.status.startswith("fault:")
  assert not update(c, N + 5_700_000_011, request=99)


def test_rejected_cancel_is_never_reported_as_success():
  c = opened()
  sends = update(c, N + 3, direction=0)
  echo(c, sends[0], N + 4, source=0xC1)
  assert c.failed and c.status == "fault:panda_rejected_cancel"
  assert c.owned_direction == 1


def test_hazards_and_fast_driver_tap_release_automatic_ownership():
  for hazards in (False, True):
    c = opened()
    if hazards:
      feedback(c, N + 3, ui=(2, 2), front=(1, 1))
    sends = update(c, N + 4, cs=state(leftBlinkerStalkCount=1 if not hazards else 0))
    assert sends[0].dat[1] & 3 == 3
    assert not c.owned_direction
    echo(c, sends[0], N + 5)
    assert c.status == "driver_takeover"


@pytest.mark.parametrize("ui,front", [((3, 0), (0, 0)), ((2, 3), (1, 0)), ((2, 0), (2, 0)), ((2, 0), (1, 3))])
def test_fault_or_unavailable_feedback_cancels_an_active_session(ui, front):
  c = opened()
  feedback(c, N + 3, ui=ui, front=front)
  assert c.session.cancelling
  assert c.take_sends(N + 4)[0].dat[1] & 3 == 3
