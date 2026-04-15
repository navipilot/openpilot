import pytest
from types import SimpleNamespace
from parameterized import parameterized

from opendbc.car.car_helpers import interfaces
from opendbc.car.gm.carcontroller import should_spoof_dash_speed
import opendbc.car.gm.interface as gm_interface
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.gm.fingerprints import FINGERPRINTS
from opendbc.car.gm.values import CAMERA_ACC_CAR, CAR, CC_ONLY_CAR, GM_RX_OFFSET

CAMERA_DIAGNOSTIC_ADDRESS = 0x24b
VOLT_CARS = (
  CAR.CHEVROLET_VOLT,
  CAR.CHEVROLET_VOLT_2019,
  CAR.CHEVROLET_VOLT_ASCM,
  CAR.CHEVROLET_VOLT_CAMERA,
  CAR.CHEVROLET_VOLT_CC,
)


def _empty_fingerprint():
  return {bus: {} for bus in range(8)}


def _test_starpilot_toggles():
  return SimpleNamespace(
    car_model="",
    cluster_offset=1.0,
    disable_openpilot_long=False,
    force_fingerprint=False,
    vEgoStopping=0.5,
    volt_sng=False,
  )


class TestGMFingerprint:
  @parameterized.expand(FINGERPRINTS.items())
  def test_can_fingerprints(self, car_model, fingerprints):
    assert len(fingerprints) > 0

    assert all(len(finger) for finger in fingerprints)

    # The camera can sometimes be communicating on startup
    if car_model in CAMERA_ACC_CAR - CC_ONLY_CAR:
      for finger in fingerprints:
        for required_addr in (CAMERA_DIAGNOSTIC_ADDRESS, CAMERA_DIAGNOSTIC_ADDRESS + GM_RX_OFFSET):
          assert finger.get(required_addr) == 8, required_addr


class TestGMInterface:
  @parameterized.expand(VOLT_CARS)
  def test_volt_min_steer_speed_is_7_mph(self, car_model):
    CarInterface = interfaces[car_model]
    car_params = CarInterface.get_params(car_model, _empty_fingerprint(), [], alpha_long=False, is_release=False, docs=False,
                                         starpilot_toggles=_test_starpilot_toggles())

    assert car_params.minSteerSpeed == pytest.approx(7 * CV.MPH_TO_MS)

  @parameterized.expand([
    ("interceptor", True),
    ("ascm_int", False),
  ])
  def test_volt_testing_ground_tune_sets_nonzero_p_and_starting_state(self, _name, pedal_present, monkeypatch):
    CarInterface = interfaces[CAR.CHEVROLET_VOLT_ASCM]
    fingerprint = _empty_fingerprint()
    if pedal_present:
      fingerprint[0][0x201] = 8  # pedal detected
    fingerprint[0][0x2FF] = 8  # SASCM detected

    monkeypatch.setattr(gm_interface.testing_ground, "use_2", True, raising=False)

    car_params = CarInterface.get_params(CAR.CHEVROLET_VOLT_ASCM, fingerprint, [], alpha_long=False, is_release=False, docs=False,
                                         starpilot_toggles=_test_starpilot_toggles())

    assert list(car_params.longitudinalTuning.kpV) == [0.10, 0.072, 0.05, 0.04]
    assert list(car_params.longitudinalTuning.kiV) == [0.025, 0.03, 0.04, 0.055]
    assert car_params.startingState
    assert car_params.startAccel == pytest.approx(1.15)


class TestGMCarController:
  def test_dash_speed_spoof_respects_live_stock_acc_toggles(self):
    cp = SimpleNamespace(openpilotLongitudinalControl=True, enableGasInterceptorDEPRECATED=True)

    assert should_spoof_dash_speed(cp, SimpleNamespace(disable_openpilot_long=False, gm_pedal_longitudinal=True))
    assert not should_spoof_dash_speed(cp, SimpleNamespace(disable_openpilot_long=True, gm_pedal_longitudinal=True))
    assert not should_spoof_dash_speed(cp, SimpleNamespace(disable_openpilot_long=False, gm_pedal_longitudinal=False))

  def test_dash_speed_spoof_allows_non_pedal_long_when_op_long_enabled(self):
    cp = SimpleNamespace(openpilotLongitudinalControl=True, enableGasInterceptorDEPRECATED=False)

    assert should_spoof_dash_speed(cp, SimpleNamespace(disable_openpilot_long=False))
