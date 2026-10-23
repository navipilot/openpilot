from unittest.mock import patch

import pytest

from opendbc.can import CANPacker
from opendbc.car import Bus, gen_empty_fingerprint, structs
from opendbc.car.tesla.carstate import CarState
from opendbc.car.tesla.interface import CarInterface
from opendbc.car.tesla.values import CAR, TeslaFlags, TeslaSafetyFlags

N = 10_000_000_000


def params(candidate=CAR.TESLA_MODEL_Y, enabled=True, vehicle_bus=True, longitudinal=False):
  fingerprint = gen_empty_fingerprint()
  if vehicle_bus:
    fingerprint[1][0x3DF] = 8
  with patch("opendbc.car.tesla.interface.Params") as store:
    store.return_value.get_bool.return_value = enabled
    if candidate == CAR.TESLA_MODEL_X:
      # Model X has no generic torque entry; test its Tesla capability gate directly.
      ret = structs.CarParams()
      ret.carFingerprint = candidate
      return CarInterface._get_params(ret, candidate, fingerprint, [], longitudinal, False, False)
    return CarInterface.get_params(candidate, fingerprint, [], longitudinal, False, False)


@pytest.mark.parametrize("candidate", [CAR.TESLA_MODEL_3, CAR.TESLA_MODEL_Y, CAR.TESLA_MODEL_X])
@pytest.mark.parametrize("enabled", [False, True])
@pytest.mark.parametrize("vehicle_bus", [False, True])
@pytest.mark.parametrize("longitudinal", [False, True])
def test_configuration_is_independent_and_only_enables_supported_platforms(candidate, enabled, vehicle_bus, longitudinal):
  cp = params(candidate, enabled, vehicle_bus, longitudinal)
  expected = candidate != CAR.TESLA_MODEL_X and enabled and vehicle_bus
  assert bool(cp.flags & TeslaFlags.AUTO_TURN_SIGNALS) == expected
  assert bool(cp.safetyConfigs[0].safetyParam & TeslaSafetyFlags.AUTO_TURN_SIGNALS) == expected
  assert bool(cp.safetyConfigs[0].safetyParam & TeslaSafetyFlags.LONG_CONTROL) == longitudinal
  assert cp.dashcamOnly == (candidate == CAR.TESLA_MODEL_X)


def test_real_stalk_frames_count_presses_without_counting_half_to_full_twice():
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  packer = CANPacker("tesla_model3_party")
  with patch("opendbc.car.tesla.carstate.time.monotonic_ns", return_value=N):
    for raw, direction, counts in ((3, 1, (1, 0)), (4, 1, (1, 0)), (0, 0, (1, 0)),
                                   (3, 1, (2, 0)), (1, 2, (2, 1)), (2, 2, (2, 1))):
      frame = packer.make_can_msg("SCCM_leftStalk", 0, {"SCCM_turnIndicatorStalkStatus": raw})
      parsers[Bus.party].update([[N, [frame]]])
      ret = cs.update(parsers)
      assert ret.turnSignalDriverValid
      assert ret.turnSignalDriverDirection == direction
      assert (ret.leftBlinkerStalkCount, ret.rightBlinkerStalkCount) == counts
    parsers[Bus.party].update([[N, [packer.make_can_msg("SCCM_leftStalk", 0, {"SCCM_turnIndicatorStalkStatus": 5})]]])
    assert not cs.update(parsers).turnSignalDriverValid
  with patch("opendbc.car.tesla.carstate.time.monotonic_ns", return_value=N + 1_000_000_001):
    assert not cs.update(parsers).turnSignalDriverValid


def test_optional_stalk_does_not_change_disabled_parser_or_fake_lamp_feedback():
  cp = params(enabled=False)
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  assert 0x249 not in parsers[Bus.party].addresses
  ret = cs.update(parsers)
  assert not ret.turnSignalDriverValid
  assert ret.autoTurnSignalDirection == 0
  parsers[Bus.party].vl["UI_warning"]["leftBlinkerBlinking"] = 2
  assert cs.update(parsers).leftBlinker


def test_press_and_release_in_one_batch_is_not_lost():
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  packer = CANPacker("tesla_model3_party")
  frames = [packer.make_can_msg("SCCM_leftStalk", 0, {"SCCM_turnIndicatorStalkStatus": position}) for position in (3, 4, 0)]
  with patch("opendbc.car.tesla.carstate.time.monotonic_ns", return_value=N):
    parsers[Bus.party].update([[N, frames]])
    ret = cs.update(parsers)
  assert ret.turnSignalDriverValid and ret.turnSignalDriverDirection == 0
  assert ret.leftBlinkerStalkCount == 1 and ret.rightBlinkerStalkCount == 0
