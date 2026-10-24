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
  assert bool(cp.flags & TeslaFlags.HAS_VEHICLE_BUS) == vehicle_bus
  assert bool(cp.flags & TeslaFlags.AUTO_SPEED_LIMIT) == (vehicle_bus and longitudinal)
  assert bool(cp.safetyConfigs[0].safetyParam & TeslaSafetyFlags.LONG_CONTROL) == longitudinal
  assert cp.dashcamOnly == (candidate == CAR.TESLA_MODEL_X)


@pytest.mark.parametrize("bus", [0, 2])
def test_extension_marker_on_another_bus_does_not_enable_vehicle_outputs(bus):
  fingerprint = gen_empty_fingerprint()
  fingerprint[bus][0x3DF] = 8
  with patch("opendbc.car.tesla.interface.Params") as store:
    store.return_value.get_bool.return_value = True
    cp = CarInterface.get_params(CAR.TESLA_MODEL_Y, fingerprint, [], True, False, False)
  assert not cp.flags & (TeslaFlags.HAS_VEHICLE_BUS | TeslaFlags.AUTO_TURN_SIGNALS | TeslaFlags.AUTO_SPEED_LIMIT)
  assert not cp.safetyConfigs[0].safetyParam & TeslaSafetyFlags.AUTO_TURN_SIGNALS


@pytest.mark.parametrize("bus", [0, 1])
def test_real_stalk_frames_count_presses_without_counting_half_to_full_twice(bus):
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  parser = parsers[Bus.party if bus == 0 else Bus.adas]
  packer = CANPacker("tesla_model3_party" if bus == 0 else "tesla_model3_vehicle")
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=N):
    for raw, direction, counts in ((3, 1, (1, 0)), (4, 1, (1, 0)), (0, 0, (1, 0)),
                                   (3, 1, (2, 0)), (1, 2, (2, 1)), (2, 2, (2, 1))):
      frame = packer.make_can_msg("SCCM_leftStalk", bus, {"SCCM_turnIndicatorStalkStatus": raw})
      parser.update([[N, [frame]]])
      ret = cs.update(parsers)
      assert ret.turnSignalDriverValid
      assert ret.turnSignalDriverDirection == direction
      assert (ret.leftBlinkerStalkCount, ret.rightBlinkerStalkCount) == counts
    parser.update([[N, [packer.make_can_msg("SCCM_leftStalk", bus, {"SCCM_turnIndicatorStalkStatus": 5})]]])
    assert not cs.update(parsers).turnSignalDriverValid
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=N + 1_000_000_001):
    assert not cs.update(parsers).turnSignalDriverValid


def test_optional_stalk_does_not_change_disabled_parser_or_fake_lamp_feedback():
  cp = params(enabled=False)
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  assert 0x249 not in parsers[Bus.party].addresses
  assert 0x249 not in parsers[Bus.adas].addresses
  ret = cs.update(parsers)
  assert not ret.turnSignalDriverValid
  assert ret.autoTurnSignalDirection == 0
  parsers[Bus.party].vl["UI_warning"]["leftBlinkerBlinking"] = 2
  assert cs.update(parsers).leftBlinker


@pytest.mark.parametrize("bus", [0, 1])
def test_press_and_release_in_one_batch_is_not_lost(bus):
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  packer = CANPacker("tesla_model3_party" if bus == 0 else "tesla_model3_vehicle")
  frames = [packer.make_can_msg("SCCM_leftStalk", bus, {"SCCM_turnIndicatorStalkStatus": position}) for position in (3, 4, 0)]
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=N):
    parsers[Bus.party if bus == 0 else Bus.adas].update([[N, frames]])
    ret = cs.update(parsers)
  assert ret.turnSignalDriverValid and ret.turnSignalDriverDirection == 0
  assert ret.leftBlinkerStalkCount == 1 and ret.rightBlinkerStalkCount == 0


@pytest.mark.parametrize("party_position", [0, 1, 5])
def test_conflicting_fresh_sources_do_not_authorize_automatic_output(party_position):
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  for bus, position in ((0, party_position), (1, 3)):
    packer = CANPacker("tesla_model3_party" if bus == 0 else "tesla_model3_vehicle")
    frame = packer.make_can_msg("SCCM_leftStalk", bus, {"SCCM_turnIndicatorStalkStatus": position})
    parsers[Bus.party if bus == 0 else Bus.adas].update([[N, [frame]]])
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=N):
    assert not cs.update(parsers).turnSignalDriverValid


def test_vehicle_stalk_does_not_fall_back_to_idle_party_when_stale():
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  vehicle = CANPacker("tesla_model3_vehicle")
  party = CANPacker("tesla_model3_party")
  parsers[Bus.adas].update([[N, [vehicle.make_can_msg("SCCM_leftStalk", 1, {"SCCM_turnIndicatorStalkStatus": 0})]]])
  later = N + 1_000_000_001
  parsers[Bus.party].update([[later, [party.make_can_msg("SCCM_leftStalk", 0, {"SCCM_turnIndicatorStalkStatus": 0})]]])
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=later):
    assert not cs.update(parsers).turnSignalDriverValid


def test_mirrored_stalk_and_invalid_gap_do_not_double_count_a_press():
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  packers = (CANPacker("tesla_model3_party"), CANPacker("tesla_model3_vehicle"))
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=N):
    for party, vehicle, valid in ((3, 3, True), (4, 3, True), (0, 3, False), (3, 3, True)):
      for bus, position in enumerate((party, vehicle)):
        frame = packers[bus].make_can_msg("SCCM_leftStalk", bus, {"SCCM_turnIndicatorStalkStatus": position})
        parsers[Bus.party if bus == 0 else Bus.adas].update([[N, [frame]]])
      ret = cs.update(parsers)
      assert ret.turnSignalDriverValid == valid
      assert ret.leftBlinkerStalkCount == 1


def test_party_fast_press_is_not_ignored_when_vehicle_bus_is_idle():
  cp = params()
  cs = CarState(cp)
  parsers = CarState.get_can_parsers(cp)
  vehicle = CANPacker("tesla_model3_vehicle")
  party = CANPacker("tesla_model3_party")
  parsers[Bus.adas].update([[N, [vehicle.make_can_msg("SCCM_leftStalk", 1, {"SCCM_turnIndicatorStalkStatus": 0})]]])
  frames = [party.make_can_msg("SCCM_leftStalk", 0, {"SCCM_turnIndicatorStalkStatus": p}) for p in (3, 0)]
  parsers[Bus.party].update([[N, frames]])
  with patch("opendbc.car.tesla.carstate.time.clock_gettime_ns", return_value=N):
    assert not cs.update(parsers).turnSignalDriverValid
