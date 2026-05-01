import re

from opendbc.car.structs import CarParams
from opendbc.car.honda.carcontroller import CIVIC_BOSCH_MODIFIED_STEER_CAN_MAX, get_civic_bosch_modified_steer_can_max
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
