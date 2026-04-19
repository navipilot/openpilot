import copy
from opendbc.can import CANDefine, CANParser
from opendbc.car import Bus, structs
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.interfaces import CarStateBase
from opendbc.car.changan.values import DBC, EPS_SCALE, CAR


class CarState(CarStateBase):
  def __init__(self, CP):
    super().__init__(CP)
    can_define = CANDefine(DBC[CP.carFingerprint][Bus.pt])

    # 挡位：两款车型均使用 GW_338 TCU_GearForDisplay : 39|4@0+
    self.shifter_values = can_define.dv.get("GW_338", {}).get("TCU_GearForDisplay", {})

    self.eps_torque_scale = EPS_SCALE[CP.carFingerprint] / 100.0

    # 驾驶员转向干预阈值
    self.steeringPressed = False
    if CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      self.steeringPressedMin = 0.5
      self.steeringPressedMax = 3
    else:
      self.steeringPressedMin = 0.5
      self.steeringPressedMax = 1.5

    # ── 巡航状态（内部维护，与原版逻辑一致）────────────────────────
    self.cruiseEnable = False
    self.cruiseEnablePrev = False
    self.cruiseSpeed = 0
    self.buttonPlus = 0
    self.buttonReduce = 0

    # IACC 使能按钮上升沿检测
    self.iacc_enable_switch_button_pressed = 0
    self.iacc_enable_switch_button_prev = 0
    self.iacc_enable_switch_button_rising_edge = False

    # 控制器用信号快照
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
      """Safe signal extraction with proper error handling"""
      try:
        if not isinstance(vl, dict):
          return default
        msg_values = vl.get(msg, {})
        if not isinstance(msg_values, dict):
          return default
        for name in names:
          if name in msg_values:
            return msg_values[name]
        return default
      except (KeyError, AssertionError, AttributeError, TypeError):
        return default

    # ── 车门 / 安全带 ─────────────────────────────────────────────
    ret.doorOpen = sig(cp.vl, "GW_28B", ["BCM_DriverDoorStatus"], 0) == 1
    ret.seatbeltUnlatched = sig(cp.vl, "GW_50", ["SRS_DriverBuckleSwitchStatus"], 0) == 1
    ret.parkingBrake = False

    # ── 车速 ──────────────────────────────────────────────────────
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      carspd = sig(cp.vl, "GW_17A", ["ESP_VehicleSpeed"], 0)
    else:
      carspd = sig(cp.vl, "GW_187", ["ESP_VehicleSpeed"], 0)

    # Safe speed calculation
    if isinstance(carspd, (int, float)) and carspd > 0:
      speed = carspd if carspd <= 5 else ((carspd / 0.98) + 2)
    else:
      speed = 0

    ret.vEgoRaw = speed * CV.KPH_TO_MS
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)
    ret.vEgoCluster = ret.vEgo
    ret.standstill = abs(ret.vEgoRaw) < 1e-3

    # ── 油门 / 制动 ───────────────────────────────────────────────
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      ret.brakePressed = sig(cp.vl, "GW_1A6", ["EMS_BrakePedalStatus"], 0) != 0
      ret.gasPressed = sig(cp.vl, "GW_1A6", ["EMS_AccPedal"], 0) != 0
    else:
      ret.brakePressed = sig(cp.vl, "GW_196", ["EMS_BrakePedalStatus"], 0) != 0
      ret.gasPressed = sig(cp.vl, "GW_196", ["EMS_RealAccPedal"], 0) != 0

    # ── 转向角 / 转向扭矩 ───────────────────────────────────────────
    angle = sig(cp.vl, "GW_180", ["SAS_SteeringAngle"], 0)
    if isinstance(angle, (int, float)):
      ret.steeringAngleDeg = float(angle) * 0.1
    else:
      ret.steeringAngleDeg = 0.0

    # 转向扭矩
    torque = sig(cp.vl, "GW_17E", ["EPS_MeasuredTorsionBarTorque"], 0)
    if isinstance(torque, (int, float)):
      ret.steeringTorque = float(torque) * self.eps_torque_scale
    else:
      ret.steeringTorque = 0.0

    ret.steeringTorqueEps = sig(cp.vl, "GW_170", ["EPS_ActualTorsionBarTorq"], 0)

    # 转向按压检测
    if abs(ret.steeringTorque) > self.steeringPressedMax:
      self.steeringPressed = True
    elif abs(ret.steeringTorque) < self.steeringPressedMin:
      self.steeringPressed = False
    ret.steeringPressed = self.steeringPressed

    # 转向系统故障检测
    ret.steerFaultTemporary = sig(cp.vl, "GW_24F", ["EPS_EPSFailed"], 0) != 0
    if not ret.steerFaultTemporary:
      ret.steerFaultTemporary = sig(cp.vl, "GW_17E", ["EPS_LatCtrlAvailabilityStatus"], 0) == 2



    # ── 巡航按钮处理 ───────────────────────────────────────────────
    btn = sig(cp.vl, "GW_28C", ["GW_MFS_IACCenable_switch_signal"], 0)
    cancel = sig(cp.vl, "GW_28C", ["GW_MFS_Cancle_switch_signal"], 0)
    res_plus = sig(cp.vl, "GW_28C", ["GW_MFS_RESPlus_switch_signal"], 0)
    set_red = sig(cp.vl, "GW_28C", ["GW_MFS_SETReduce_switch_signal"], 0)

    # IACC 使能按钮上升沿检测
    self.iacc_enable_switch_button_pressed = btn
    if btn == 1 and self.iacc_enable_switch_button_prev == 0:
      self.iacc_enable_switch_button_rising_edge = True
    else:
      self.iacc_enable_switch_button_rising_edge = False
    self.iacc_enable_switch_button_prev = btn

    # 巡航按钮状态机
    if btn == 1:
      self.cruiseEnable = True
      self.cruiseSpeed = ret.vEgoRaw * CV.MS_TO_KPH

    if cancel == 1 or ret.brakePressed:
      self.cruiseEnable = False

    if res_plus == 1 and self.buttonPlus == 0 and self.cruiseEnable:
      self.cruiseSpeed = (self.cruiseSpeed // 5 + 1) * 5

    if set_red == 1 and self.buttonReduce == 0 and self.cruiseEnable:
      self.cruiseSpeed = max((self.cruiseSpeed // 5 - 1) * 5, 0)

    self.cruiseEnablePrev = self.cruiseEnable
    self.buttonPlus = res_plus
    self.buttonReduce = set_red

    # ── 原车 ACC 状态 ─────────────────────────────────────────────
    acc_mode_244 = sig(cp_cam.vl, "GW_244", ["ACC_ACCMode"], 0)
    acc_mode_31a = sig(cp_cam.vl, "GW_31A", ["ACC_IACCHWAMode"], 0)
    iacc_enable = sig(cp_cam.vl, "GW_31A", ["ACC_IACCHWAEnable"], 0)

    ret.cruiseState.available = (iacc_enable == 1)
    ret.cruiseState.speed = self.cruiseSpeed * CV.KPH_TO_MS
    if self.cruiseSpeed != 0:
      ret.cruiseState.speedCluster = self.cruiseSpeed * CV.KPH_TO_MS
    ret.cruiseState.standstill = ret.standstill
    if self.cruiseEnable:
      ret.cruiseState.standstill = True
    ret.cruiseState.enabled = self.cruiseEnable

    # ── ACC 故障 ──────────────────────────────────────────────────
    ret.accFaulted = (acc_mode_244 == 7) or (acc_mode_31a == 7)

    # ── 原车 ADAS 状态 ────────────────────────────────────────────
    ret.stockFcw = sig(cp_cam.vl, "GW_244", ["ACC_FCWPreWarning"], 0) == 1
    ret.stockAeb = sig(cp_cam.vl, "GW_244", ["ACC_AEBCtrlType"], 0) > 0
    ret.genericToggle = False

    # ── 控制器用信号快照（安全访问）────────────────────────────────
    if isinstance(cp_cam.vl, dict) and "GW_244" in cp_cam.vl:
      self.sigs244 = copy.copy(cp_cam.vl["GW_244"])
    if isinstance(cp_cam.vl, dict) and "GW_1BA" in cp_cam.vl:
      self.sigs1ba = copy.copy(cp_cam.vl["GW_1BA"])
    if isinstance(cp.vl, dict) and "GW_17E" in cp.vl:
      self.sigs17e = copy.copy(cp.vl["GW_17E"])
    if isinstance(cp_cam.vl, dict) and "GW_307" in cp_cam.vl:
      self.sigs307 = copy.copy(cp_cam.vl["GW_307"])
    if isinstance(cp_cam.vl, dict) and "GW_31A" in cp_cam.vl:
      self.sigs31a = copy.copy(cp_cam.vl["GW_31A"])

    # ── 滚动计数器提取（安全访问）──────────────────────────────────
    self.counter_244 = sig(cp_cam.vl, "GW_244", ["ACC_RollingCounter_24E"], 0)
    self.counter_1ba = sig(cp_cam.vl, "GW_1BA", ["ACC_RollingCounter_1BA"], 0)
    self.counter_17e = sig(cp.vl, "GW_17E", ["EPS_RollingCounter_17E"], 0)
    self.counter_307 = sig(cp_cam.vl, "GW_307", ["ACC_RollingCounter_35E"], 0)
    self.counter_31a = sig(cp_cam.vl, "GW_31A", ["ACC_RollingCounter_36D"], 0)

    # 跟车距离按钮
    self.prev_distance_button = self.distance_button
    self.distance_button = sig(cp_cam.vl, "GW_307", ["ACC_DistanceLevel"], 0)

    return ret

  @staticmethod
  def get_can_parsers(CP):
    pt_messages = [
      ("GW_50", 2),
      ("GW_170", 100),
      ("GW_17E", 100),
      ("GW_180", 100),
      ("GW_24F", 50),
      ("GW_28B", 25),
      ("GW_28C", 50),
      ("GW_338", 0),  # 挡位，频率0=可选
    ]

    if CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      pt_messages += [
        ("GW_17A", 100),
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
      ("GW_31A", 10),
    ]

    return {
      Bus.pt: CANParser(DBC[CP.carFingerprint][Bus.pt], pt_messages, 0),
      Bus.cam: CANParser(DBC[CP.carFingerprint][Bus.pt], cam_messages, 2),
    }