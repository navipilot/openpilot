"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
from enum import StrEnum

from opendbc.car import Bus, create_button_events, structs
from opendbc.can.parser import CANParser
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.tesla.values import DBC, CANBUS
from opendbc.sunnypilot.car.tesla.values import TeslaFlagsSP

ButtonType = structs.CarState.ButtonEvent.Type
SPEED_AUTO_RESUME_GESTURE_NS = 1_000_000_000


class CarStateExt:
  def __init__(self, CP: structs.CarParams, CP_SP: structs.CarParamsSP):
    self.CP = CP
    self.CP_SP = CP_SP

    self.active_touch_points = 0
    self.tesla_speed_button_template = None
    self.tesla_speed_button_template_nanos = 0
    self.tesla_speed_limit_target = 0.0
    self.tesla_speed_limit_target_valid = False
    self.tesla_speed_units = "KPH"
    self.tesla_manual_speed_adjustment_counter = 0
    self.tesla_speed_auto_resume_gesture_counter = 0
    self._tesla_speed_resume_up_nanos = 0
    self._tesla_speed_resume_down_nanos = 0
    self._tesla_speed_resume_wait_idle = False

  def update_speed_button_template(self, data: bytes, monotonic_nanos: int) -> None:
    # 只接受右滚轮复用值为 1 的帧；空闲帧用于克隆，非空闲帧用于识别人工接管。
    if len(data) != 8 or (data[0] & 0x03) != 1:
      return

    raw_tick = data[3] & 0x3F
    if raw_tick == 0:
      self.tesla_speed_button_template = bytes(data)
      self.tesla_speed_button_template_nanos = int(monotonic_nanos)
      self._tesla_speed_resume_wait_idle = False
      return

    # 双向恢复手势完成后忽略尾随重复帧，直到滚轮重新回到空闲状态。
    if self._tesla_speed_resume_wait_idle:
      return

    signed_tick = raw_tick - 0x40 if raw_tick & 0x20 else raw_tick
    direction = 1 if signed_tick > 0 else -1
    self.tesla_manual_speed_adjustment_counter += 1
    now_nanos = int(monotonic_nanos)
    opposite_nanos = self._tesla_speed_resume_down_nanos if direction > 0 else self._tesla_speed_resume_up_nanos
    # 一秒内相反方向各滚一次，视为恢复自动设置速度的明确手势。
    if opposite_nanos and now_nanos - opposite_nanos <= SPEED_AUTO_RESUME_GESTURE_NS:
      self.tesla_speed_auto_resume_gesture_counter += 1
      self._tesla_speed_resume_up_nanos = 0
      self._tesla_speed_resume_down_nanos = 0
      self._tesla_speed_resume_wait_idle = True
    elif direction > 0:
      self._tesla_speed_resume_up_nanos = now_nanos
      self._tesla_speed_resume_down_nanos = 0
    else:
      self._tesla_speed_resume_down_nanos = now_nanos
      self._tesla_speed_resume_up_nanos = 0

  def update_speed_limit_target(self, target: float, valid: bool) -> None:
    self.tesla_speed_limit_target = float(target) if valid else 0.0
    self.tesla_speed_limit_target_valid = bool(valid)

  def update(self, ret: structs.CarState, ret_sp: structs.CarStateSP, can_parsers: dict[StrEnum, CANParser]) -> None:
    if self.CP_SP.flags & TeslaFlagsSP.HAS_VEHICLE_BUS:
      cp_adas = can_parsers[Bus.adas]

      prev_active_touch_points = self.active_touch_points
      self.active_touch_points = int(cp_adas.vl["UI_status2"]["UI_activeTouchPoints"])

      finger_count = None
      if self.CP_SP.flags & TeslaFlagsSP.MADS_SCREEN_BUTTON_3_FINGER:
        finger_count = 3
      elif self.CP_SP.flags & TeslaFlagsSP.MADS_SCREEN_BUTTON_4_FINGER:
        finger_count = 4
      elif self.CP_SP.flags & TeslaFlagsSP.MADS_SCREEN_BUTTON_5_FINGER:
        finger_count = 5

      if finger_count is not None:
        ret.buttonEvents = [*create_button_events(self.active_touch_points, prev_active_touch_points,
                                                  {finger_count: ButtonType.lkas})]
    cp_party = can_parsers[Bus.party]

    cp_ap_party = can_parsers[Bus.ap_party]

    speed_units = self.can_define.dv["DI_state"]["DI_speedUnits"].get(int(cp_party.vl["DI_state"]["DI_speedUnits"]), None)
    if speed_units in ("KPH", "MPH"):
      self.tesla_speed_units = speed_units
    speed_limit = cp_ap_party.vl["DAS_status"]["DAS_fusedSpeedLimit"]
    if self.can_define.dv["DAS_status"]["DAS_fusedSpeedLimit"].get(int(speed_limit), None) in ["NONE", "UNKNOWN_SNA"]:
      ret_sp.speedLimit = 0
    else:
      if speed_units == "KPH":
        ret_sp.speedLimit = speed_limit * CV.KPH_TO_MS
      elif speed_units == "MPH":
        ret_sp.speedLimit = speed_limit * CV.MPH_TO_MS

    ret.genericToggle = cp_party.vl["UI_warning"]["scrollWheelPressed"] != 0

  @staticmethod
  def get_parser(CP: structs.CarParams, CP_SP: structs.CarParamsSP) -> dict[StrEnum, CANParser]:
    messages = {}

    if CP_SP.flags & TeslaFlagsSP.HAS_VEHICLE_BUS:
      messages[Bus.adas] = CANParser(DBC[CP.carFingerprint][Bus.adas], [], CANBUS.vehicle)

    return messages
