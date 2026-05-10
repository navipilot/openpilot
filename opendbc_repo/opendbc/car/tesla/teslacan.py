from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.tesla.values import CANBUS, CarControllerParams
from opendbc.car import DT_CTRL


class TeslaCAN:
  def __init__(self, CP, packer):
    self.CP = CP
    self.packer = packer
    self.l_jerk = 0.0

  def create_steering_control(self, angle, enabled, control_type):
    values = {
      "DAS_steeringAngleRequest": -angle,
      "DAS_steeringHapticRequest": 0,
      "DAS_steeringControlType": control_type if enabled else 0,
    }

    return self.packer.make_can_msg("DAS_steeringControl", CANBUS.party, values)

  def create_longitudinal_command(self, acc_state, accel, counter, v_ego, active, cruise_override, set_speed_kph=None):
    from opendbc.car.interfaces import V_CRUISE_MAX

    set_speed = max(v_ego * CV.MS_TO_KPH, 0)
    self.l_jerk = 0.0
    if active:
      self.l_jerk = 0 if cruise_override else (self.l_jerk + CarControllerParams.JERK_UP * DT_CTRL * 4)
      set_speed = 0 if accel < 0 else V_CRUISE_MAX
      if set_speed_kph is not None and accel >= 0:
        set_speed = max(0.0, min(V_CRUISE_MAX, float(set_speed_kph)))

    values = {
      "DAS_setSpeed": set_speed,
      "DAS_accState": acc_state,
      "DAS_aebEvent": 0,
      "DAS_jerkMin": CarControllerParams.JERK_LIMIT_MIN,
      "DAS_jerkMax": min(self.l_jerk, CarControllerParams.JERK_LIMIT_MAX),
      "DAS_accelMin": accel,
      "DAS_accelMax": max(accel, 0),
      "DAS_controlCounter": counter,
    }
    return self.packer.make_can_msg("DAS_control", CANBUS.party, values)

  def create_steering_allowed(self):
    values = {
      "APS_eacAllow": 1,
    }

    return self.packer.make_can_msg("APS_eacMonitor", CANBUS.party, values)


def tesla_checksum(address: int, sig, d: bytearray) -> int:
  checksum = (address & 0xFF) + ((address >> 8) & 0xFF)
  checksum_byte = sig.start_bit // 8
  for i in range(len(d)):
    if i != checksum_byte:
      checksum += d[i]
  return checksum & 0xFF
