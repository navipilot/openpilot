#!/usr/bin/env python3
import unittest
import numpy as np
from opendbc.car.structs import CarParams
from opendbc.safety.tests.libsafety import libsafety_py
import opendbc.safety.tests.common as common
from opendbc.safety.tests.common import CANPackerPanda

# MEB message IDs
MSG_ESC_51 = 0xFC        # RX from ESC, for wheel speeds
MSG_LH_EPS_03 = 0x9F     # RX from EPS, for driver steering torque
MSG_QFK_01 = 0x13D       # RX from QFK, for lateral control status
MSG_VMM_02 = 0x139       # RX from VMM, for ESP hold status
MSG_MOTOR_51 = 0x10B     # RX from ECU, for ACC status
MSG_MOTOR_54 = 0x14C     # RX from ECU, for gas pedal input
MSG_MOTOR_14 = 0x3BE     # RX from ECU, for brake pedal switch
MSG_GRA_ACC_01 = 0x12B   # TX by OP, ACC control buttons for cancel/resume
MSG_HCA_03 = 0x303       # TX by OP, lateral curvature control
MSG_EA_01 = 0x1A4        # TX by OP, Emergency Assist control
MSG_EA_02 = 0x1F0        # TX by OP, Emergency Assist HUD
MSG_LDW_02 = 0x397       # TX by OP, Lane line recognition and text alerts


class TestVolkswagenMebSafety(common.PandaCarSafetyTest):
  RELAY_MALFUNCTION_ADDRS = {0: (MSG_HCA_03,)}

  @classmethod
  def setUpClass(cls):
    if cls.__name__ == "TestVolkswagenMebSafety":
      cls.packer = None
      cls.safety = None
      raise unittest.SkipTest

  def setUp(self):
    self.packer = CANPackerPanda("vw_meb")
    self.safety = libsafety_py.libsafety
    self.safety.set_safety_hooks(CarParams.SafetyModel.volkswagenMeb, 0)
    self.safety.init_tests()

  # Wheel speeds from ESC_51
  def _speed_msg(self, speed):
    values = {"%s_Radgeschw" % s: speed for s in ["VL", "VR", "HL", "HR"]}
    return self.packer.make_can_msg_panda("ESC_51", 0, values)

  # Brake pedal switch from Motor_14
  def _user_brake_msg(self, brake):
    values = {"MO_Fahrer_bremst": brake}
    return self.packer.make_can_msg_panda("Motor_14", 0, values)

  # Gas pedal from Motor_54
  def _user_gas_msg(self, gas):
    values = {"Accelerator_Pressure": gas}
    return self.packer.make_can_msg_panda("Motor_54", 0, values)

  # ACC engagement status from Motor_51
  def _pcm_status_msg(self, enable):
    tsk_status = 3 if enable else 2
    values = {"TSK_Status": tsk_status}
    return self.packer.make_can_msg_panda("Motor_51", 0, values)

  # Driver steering input torque from LH_EPS_03
  def _torque_driver_msg(self, torque):
    values = {"EPS_Lenkmoment": abs(torque), "EPS_VZ_Lenkmoment": 1 if torque < 0 else 0}
    return self.packer.make_can_msg_panda("LH_EPS_03", 0, values)

  # Curvature command from HCA_03
  def _torque_cmd_msg(self, curvature, steer_req=1):
    values = {
      "Curvature": abs(curvature),
      "Curvature_VZ": 1 if curvature < 0 else 0,
      "RequestStatus": 4 if steer_req else 2,
      "Power": 50 if steer_req else 0,
    }
    return self.packer.make_can_msg_panda("HCA_03", 0, values)

  # GRA_ACC_01 button controls
  def _gra_acc_01_msg(self, cancel=0, resume=0, set_btn=0):
    values = {"GRA_Abbrechen": cancel, "GRA_Tip_Wiederaufnahme": resume, "GRA_Tip_Setzen": set_btn}
    return self.packer.make_can_msg_panda("GRA_ACC_01", 0, values)

  def test_enable_control_allowed_from_cruise(self):
    # Control allowed when stock ACC is engaged
    self.safety.set_controls_allowed(0)
    self._rx(self._pcm_status_msg(True))
    self.assertTrue(self.safety.get_controls_allowed())

  def test_disable_control_allowed_from_cruise(self):
    # Control disallowed when stock ACC disengages
    self.safety.set_controls_allowed(1)
    self._rx(self._pcm_status_msg(False))
    self.assertFalse(self.safety.get_controls_allowed())

  def test_disable_control_allowed_on_cancel(self):
    # Control disallowed on cancel button
    self.safety.set_controls_allowed(1)
    self._rx(self._gra_acc_01_msg(cancel=1))
    self.assertFalse(self.safety.get_controls_allowed())

  def test_brake_disengage(self):
    # Test brake pedal disengages controls
    self.safety.set_controls_allowed(1)
    self._rx(self._user_brake_msg(1))
    self.assertFalse(self.safety.get_controls_allowed())

  def test_gas_safety_check(self):
    # Test gas pedal press is detected
    for gas in np.arange(0, 256, 1):
      self._rx(self._user_gas_msg(gas))
      self.assertEqual(gas > 0, self.safety.get_gas_pressed_prev())


class TestVolkswagenMebStockSafety(TestVolkswagenMebSafety):
  @classmethod
  def setUpClass(cls):
    if cls.__name__ == "TestVolkswagenMebStockSafety":
      cls.packer = None
      cls.safety = None
      raise unittest.SkipTest


if __name__ == "__main__":
  unittest.main()
