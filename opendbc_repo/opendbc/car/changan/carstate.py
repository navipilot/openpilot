import copy
from opendbc.can import CANDefine, CANParser
from opendbc.car import Bus, DT_CTRL, structs
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.common.filter_simple import FirstOrderFilter
from opendbc.car.interfaces import CarStateBase
from opendbc.car.changan.values import DBC, EPS_SCALE, CAR, ChanganFlags


class CarState(CarStateBase):
  def __init__(self, CP):
    super().__init__(CP)
    can_define = CANDefine(DBC[CP.carFingerprint][Bus.pt])

    # GW_331 (Z6) / GW_338 (Z6_IDD) 均有 TCU_GearForDisplay : 39|4@0+
    if CP.carFingerprint == CAR.CHANGAN_Z6:
      self.shifter_values = can_define.dv.get("GW_331", {}).get("TCU_GearForDisplay", {})
      if not self.shifter_values:
        self.shifter_values = can_define.dv.get("GW_338", {}).get("TCU_GearForDisplay", {})
    else:
      self.shifter_values = can_define.dv.get("GW_338", {}).get("TCU_GearForDisplay", {})

    self.eps_torque_scale = EPS_SCALE[CP.carFingerprint] / 100.0
    self.cluster_speed_hyst_gap = CV.KPH_TO_MS / 2.0
    self.cluster_min_speed = CV.KPH_TO_MS / 2.0

    self.angle_offset = FirstOrderFilter(None, 60.0, DT_CTRL, initialized=False)

    self.steeringPressed = False
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      self.steeringPressedMin = 1
      self.steeringPressedMax = 3
    else:
      self.steeringPressedMin = 1
      self.steeringPressedMax = 6

    # 供控制器使用的信号快照
    self.sigs244 = {}
    self.sigs1ba = {}
    self.sigs17e = {}
    self.sigs307 = {}
    self.sigs31a = {}

    # 滚动计数器
    self.counter_244 = 0
    self.counter_1ba = 0
    self.counter_17e = 0
    self.counter_307 = 0
    self.counter_31a = 0

    self.prev_distance_button = 0
    self.distance_button = 0

  def update(self, can_parsers) -> structs.CarState:
    cp = can_parsers[Bus.pt]
    cp_cam = can_parsers[Bus.cam]
    ret = structs.CarState()

    def sig(vl, msg, names, default=0):
      try:
        msg_values = vl[msg]
      except (KeyError, AssertionError):
        return default
      for name in names:
        if name in msg_values:
          return msg_values[name]
      return default

    # 车门 / 安全带
    ret.doorOpen = sig(cp.vl, "GW_28B", ["BCM_DriverDoorStatus"], 0) == 1
    ret.seatbeltUnlatched = sig(cp.vl, "GW_50", ["SRS_DriverBuckleSwitchStatus"], 0) == 1
    ret.parkingBrake = False

    # 车速
    # GW_187 / GW_17A: ESP_VehicleSpeed : 36|13@0+ (0.05625,0) 单位 km/h
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      carspd = sig(cp.vl, "GW_17A", ["ESP_VehicleSpeed"], 0)
    else:
      carspd = sig(cp.vl, "GW_187", ["ESP_VehicleSpeed"], 0)
    speed = carspd if carspd <= 5 else ((carspd / 0.98) + 2)
    ret.vEgoRaw = speed * CV.KPH_TO_MS
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)
    ret.vEgoCluster = ret.vEgo
    ret.standstill = abs(ret.vEgoRaw) < 1e-3

    # 油门 / 制动
    # GW_196: EMS_BrakePedalStatus : 55|2@0+  EMS_RealAccPedal : 7|8@0+
    # GW_1A6: EMS_BrakePedalStatus : 35|2@0+  EMS_AccPedal     : 47|8@0+
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      ret.brakePressed = sig(cp.vl, "GW_1A6", ["EMS_BrakePedalStatus"], 0) != 0
      ret.gasPressed = sig(cp.vl, "GW_1A6", ["EMS_AccPedal"],
                           sig(cp.vl, "GW_1C6", ["EMS_RealAccPedal"], 0)) != 0
    else:
      ret.brakePressed = sig(cp.vl, "GW_196", ["EMS_BrakePedalStatus"], 0) != 0
      ret.gasPressed = sig(cp.vl, "GW_196", ["EMS_RealAccPedal"], 0) != 0

    # 挡位
    # GW_331 / GW_338: TCU_GearForDisplay : 39|4@0+
    if self.CP.carFingerprint == CAR.CHANGAN_Z6:
      can_gear = int(sig(cp.vl, "GW_331", ["TCU_GearForDisplay"],
                         sig(cp.vl, "GW_338", ["TCU_GearForDisplay"], 0)))
    else:
      can_gear = int(sig(cp.vl, "GW_338", ["TCU_GearForDisplay"], 0))
    ret.gearShifter = self.parse_gear_shifter(self.shifter_values.get(can_gear, None))
    ret.leftBlindspot = False
    ret.rightBlindspot = False

    # 转向灯
    # GW_28B: BCM_TurnIndicatorLeft / BCM_TurnIndicatorRight
    left_blinker = sig(cp.vl, "GW_28B", ["BCM_TurnIndicatorLeft"], 0) == 1
    right_raw = sig(cp.vl, "GW_28B", ["BCM_TurnIndicatorRight"], 0)
    ret.leftBlinker, ret.rightBlinker = self.update_blinker_from_stalk(200, left_blinker, right_raw in (1, 2))

    # 转向角 / 扭矩
    # GW_180: SAS_SteeringAngle : 7|16@0-  SAS_SteeringAngleSpeed : 23|8@0-
    # GW_17E: EPS_MeasuredTorsionBarTorque : 7|12@0+  EPS_ActualTorsionBarTorq (GW_170)
    ret.steeringAngleOffsetDeg = 0
    ret.steeringAngleDeg = sig(cp.vl, "GW_180", ["SAS_SteeringAngle"], 0)
    ret.steeringRateDeg = sig(cp.vl, "GW_180", ["SAS_SteeringAngleSpeed"], 0)
    ret.steeringTorque = sig(cp.vl, "GW_17E", ["EPS_MeasuredTorsionBarTorque"], 0)
    ret.steeringTorqueEps = sig(cp.vl, "GW_170", ["EPS_ActualTorsionBarTorq"], 0) * self.eps_torque_scale

    if self.steeringPressed:
      if abs(ret.steeringTorque) < self.steeringPressedMin and abs(ret.steeringAngleDeg) < 90:
        self.steeringPressed = False
    else:
      if abs(ret.steeringTorque) > self.steeringPressedMax:
        self.steeringPressed = True
    ret.steeringPressed = self.steeringPressed

    # 原车 ACC 状态透传
    # GW_244: ACC_ACCMode : 54|3@0+  (2=Standby, 3=Active, 7=Fault)
    # GW_31A: ACC_IACCHWAMode : 135|3@0+  (3=Active)
    acc_mode_244 = sig(cp_cam.vl, "GW_244", ["ACC_ACCMode"], 0)
    acc_mode_31a = sig(cp_cam.vl, "GW_31A", ["ACC_IACCHWAMode"], 0)
    ret.cruiseState.available = True
    ret.cruiseState.enabled = (acc_mode_244 == 3)

    # GW_307: ACC_SetSpeed : 6|8@0+ (1,0) 单位 km/h（注意：DBC信号名是 ACC_SetSpeed，不是 vCruise）
    ret.cruiseState.speed = sig(cp_cam.vl, "GW_307", ["ACC_SetSpeed"], 0) * CV.KPH_TO_MS
    ret.cruiseState.standstill = ret.standstill

    # 故障检测
    ret.accFaulted = (acc_mode_244 == 7) or (acc_mode_31a == 7)
    eps_failed = sig(cp.vl, "GW_24F", ["EPS_EPSFailed"], 0) != 0
    # GW_17E: EPS_LatCtrlAvailabilityStatus : 53|2@0+ (1=OK, 2=Unavailable)
    lat_ctrl_unavailable = sig(cp.vl, "GW_17E", ["EPS_LatCtrlAvailabilityStatus"], 1) == 2
    ret.steerFaultTemporary = eps_failed or lat_ctrl_unavailable
    ret.steerFaultPermanent = False

    # 原车 ADAS 状态透传
    # GW_244: ACC_FCWPreWarning : 91|1@0+  ACC_AEBCtrlType : 104|4@0+
    ret.stockFcw = sig(cp_cam.vl, "GW_244", ["ACC_FCWPreWarning"], 0) == 1
    ret.stockAeb = sig(cp_cam.vl, "GW_244", ["ACC_AEBCtrlType"], 0) > 0
    ret.genericToggle = False

    # 控制器用信号快照
    self.sigs244 = copy.copy(cp_cam.vl["GW_244"])
    self.sigs1ba = copy.copy(cp_cam.vl["GW_1BA"])
    self.sigs17e = copy.copy(cp.vl["GW_17E"])
    self.sigs307 = copy.copy(cp_cam.vl["GW_307"])
    self.sigs31a = copy.copy(cp_cam.vl["GW_31A"])

    # 滚动计数器提取
    self.counter_244 = sig(cp_cam.vl, "GW_244", ["ACC_RollingCounter_24E"], 0)
    self.counter_1ba = sig(cp_cam.vl, "GW_1BA", ["ACC_RollingCounter_1BA"], 0)
    self.counter_17e = sig(cp.vl, "GW_17E", ["EPS_RollingCounter_17E"], 0)
    self.counter_307 = sig(cp_cam.vl, "GW_307", ["ACC_RollingCounter_35E"], 0)
    self.counter_31a = sig(cp_cam.vl, "GW_31A", ["ACC_RollingCounter_36D"], 0)

    # 跟车距离按钮 GW_307: ACC_DistanceLevel : 22|3@0+
    self.prev_distance_button = self.distance_button
    self.distance_button = sig(cp_cam.vl, "GW_307", ["ACC_DistanceLevel"], 0)

    return ret

  @staticmethod
  def get_can_parsers(CP):
    pt_messages = [
      ("GW_50",  2),
      ("GW_170", 100),
      ("GW_17E", 100),
      ("GW_180", 100),
      ("GW_24F", 50),
      ("GW_28B", 25),
      ("GW_28C", 50),
    ]

    if CP.carFingerprint == CAR.CHANGAN_Z6:
      pt_messages += [
        # 频率设 0：部分 Z6 车辆 bus0 不广播 GW_331，设为可选避免 CAN_INVALID
        ("GW_331", 0),
        ("GW_338", 0),
      ]
    else:
      pt_messages += [
        ("GW_338", 10),
        ("GW_17A", 100),
      ]

    if CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      pt_messages += [
        ("GW_1A6", 100),
        ("GW_1C6", 100),
      ]
    else:
      pt_messages += [
        ("GW_187", 100),
        ("GW_196", 100),
      ]

    cam_messages = [
      ("GW_1BA", 100),
      ("GW_244", 50),
      ("GW_307", 10),
      ("GW_31A", 10),   # GW_31A ACC_IACCHWAMode 在 byte16，实际更新较慢
    ]

    return {
      Bus.pt:  CANParser(DBC[CP.carFingerprint][Bus.pt], pt_messages, 0),
      Bus.cam: CANParser(DBC[CP.carFingerprint][Bus.pt], cam_messages, 2),
    }
