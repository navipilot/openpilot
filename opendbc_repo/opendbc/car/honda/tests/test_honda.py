import re

from opendbc.car.structs import CarParams
from opendbc.car.honda.carcontroller import (
  CIVIC_BOSCH_MODIFIED_STEER_CAN_MAX,
  get_civic_bosch_modified_steer_can_max,
  get_civic_bosch_modified_steering_pressed,
  get_civic_bosch_modified_torque_lpf_tau,
)
from opendbc.car.honda.fingerprints import FW_VERSIONS
from opendbc.car.honda.values import CAR, HONDA_BOSCH, HONDA_BOSCH_TJA_CONTROL, HondaFlags

HONDA_FW_VERSION_RE = br"[A-Z0-9]{5}-[A-Z0-9]{3}(-|,)[A-Z0-9]{4}(\x00){2}$"


class TestHondaFingerprint:
  def test_fw_version_format(self):
    # Asserts all FW versions follow an expected format
    for fw_by_ecu in FW_VERSIONS.values():
      for fws in fw_by_ecu.values():
        for fw in fws:
          assert re.match(HONDA_FW_VERSION_RE, fw) is not None, fw

  def test_tja_bosch_only(self):
    assert set(HONDA_BOSCH_TJA_CONTROL).issubset(set(HONDA_BOSCH)), "Nidec car found in TJA control list"

  def test_modified_civic_steer_can_max_is_scoped(self, monkeypatch):
    CP = CarParams.new_message()
    CP.carFingerprint = CAR.HONDA_CIVIC_BOSCH
    CP.flags = int(HondaFlags.BOSCH)
    CP.dashcamOnly = True

    monkeypatch.setattr("opendbc.car.honda.carcontroller.civic_bosch_modified_lateral_testing_ground_active", lambda: True)
    assert get_civic_bosch_modified_steer_can_max(4096, CP) == CIVIC_BOSCH_MODIFIED_STEER_CAN_MAX

    CP.dashcamOnly = False
    assert get_civic_bosch_modified_steer_can_max(4096, CP) == 4096

  def test_modified_civic_torque_lpf_tau_reacts_to_sign_change(self):
    assert get_civic_bosch_modified_torque_lpf_tau(0.7, -0.1, 25.0) == 0.09
    assert get_civic_bosch_modified_torque_lpf_tau(0.02, 0.01, 12.0) == 0.15
    assert get_civic_bosch_modified_torque_lpf_tau(0.30, 0.0, 12.0) == 0.11

  def test_modified_civic_steering_pressed_filter_rejects_short_same_direction_spikes(self):
    filter_s, pressed = get_civic_bosch_modified_steering_pressed(True, 1500.0, 0.8, 0.01, False)
    assert not pressed
    assert filter_s > 0.01

    filter_s = 0.31
    filter_s, pressed = get_civic_bosch_modified_steering_pressed(True, 1500.0, 0.8, filter_s, False)
    assert pressed

  def test_modified_civic_steering_pressed_filter_allows_opposing_driver_torque_quickly(self):
    filter_s, pressed = get_civic_bosch_modified_steering_pressed(True, -1500.0, 0.8, 0.11, False)
    assert pressed
