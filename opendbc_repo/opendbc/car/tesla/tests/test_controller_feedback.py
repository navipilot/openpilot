from types import SimpleNamespace

import pytest

from opendbc.can import CANParser
from opendbc.car import Bus, structs
from opendbc.car.tesla.carcontroller import CarController
from opendbc.car.tesla.interface import CarInterface
from opendbc.car.tesla.values import CAR


@pytest.mark.parametrize("cooperative", [False, True])
@pytest.mark.parametrize("active,override", [(False, False), (True, False), (True, True)])
def test_feedback_matches_final_steering_request_and_holds_between_transmissions(cooperative, active, override):
  cp = CarInterface.get_non_essential_params(CAR.TESLA_MODEL_3)
  controller = CarController({Bus.party: "tesla_model3_party"}, cp)
  controller.coop_steering = cooperative
  state = SimpleNamespace(
    out=structs.CarState(vEgo=10.0, vEgoRaw=10.0, steeringAngleDeg=12.3, steeringRateDeg=80.0, steeringTorque=1.0),
    steering_disengage=override, tesla_manual_speed_adjustment_counter=0,
    tesla_speed_auto_resume_gesture_counter=0, tesla_speed_limit_target_nanos=0,
    das_accCancel=False, cruise_override=False, das_control={"DAS_controlCounter": 0},
  )
  command = structs.CarControl(latActive=active)
  command.actuators.steeringAngleDeg = 30.0
  parser = CANParser("tesla_model3_party", [("DAS_steeringControl", 50)], 0)

  output, messages = controller.update(command.as_reader(), state, 0)
  parser.update([(0, messages)])
  request = parser.vl["DAS_steeringControl"]
  assert output.steeringAngleDeg == pytest.approx(-request["DAS_steeringAngleRequest"], abs=0.051)
  assert bool(request["DAS_steeringControlType"]) == (active and not override)
  if cooperative and (not active or override):
    assert output.steeringAngleDeg == pytest.approx(state.out.steeringAngleDeg)

  command.actuators.steeringAngleDeg = -30.0
  next_output, next_messages = controller.update(command.as_reader(), state, 10_000_000)
  assert all(message[0] != 0x488 for message in next_messages)
  assert next_output.steeringAngleDeg == output.steeringAngleDeg
