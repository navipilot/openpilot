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

    # Different gear signal names and messages per model
    # Z6 uses changan.dbc with GW_331/TCU_GearForDisplay
    # Z6_IDD uses changan.dbc with GW_338/TCU_GearForDisplay
    # QIYUAN_A05/A07 use changan_can.dbc with GEAR/gearShifter
    if CP.carFingerprint == CAR.CHANGAN_Z6:
      # Some changan.dbc versions only define TCU_GearForDisplay enum under GW_338.
      self.shifter_values = can_define.dv.get("GW_331", {}).get("TCU_GearForDisplay")
      if self.shifter_values is None:
        self.shifter_values = can_define.dv.get("GW_338", {}).get("TCU_GearForDisplay", {})
    elif CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      self.shifter_values = can_define.dv.get("GW_338", {}).get("TCU_GearForDisplay", {})
    else:  # QIYUAN_A05, QIYUAN_A07
      self.shifter_values = can_define.dv.get("GEAR", {}).get("gearShifter", {})

    self.eps_torque_scale = EPS_SCALE[CP.carFingerprint] / 100.0
    self.cluster_speed_hyst_gap = CV.KPH_TO_MS / 2.0
    self.cluster_min_speed = CV.KPH_TO_MS / 2.0

    self.angle_offset = FirstOrderFilter(None, 60.0, DT_CTRL, initialized=False)

    self.cruiseEnable = False
    self.cruiseEnablePrev = False
    self.cruiseSpeed = 0
    self.buttonPlus = 0
    self.buttonReduce = 0

    self.steeringPressed = False
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      self.steeringPressedMin = 1
      self.steeringPressedMax = 3
    else:
      self.steeringPressedMin = 1
      self.steeringPressedMax = 6

    self.iacc_enable_switch_button_pressed = 0
    self.iacc_enable_switch_button_prev = 0
    self.iacc_enable_switch_button_rising_edge = False

    self.sigs244 = {} # 加速度 ACC状态等
    self.sigs1ba = {} # 方向盘控制
    self.sigs17e = {} # 扭矩
    self.sigs307 = {} # 巡航车速 跟车距离
    self.sigs31a = {} # ACC_IACCHWAEnable 状态
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

    def sig(parser, msg, name, default=0):
      return parser.vl.get(msg, {}).get(name, default)

    # Door / Seatbelt
    ret.doorOpen = sig(cp, "GW_28B", "BCM_DriverDoorStatus") == 1
    ret.seatbeltUnlatched = sig(cp, "GW_50", "SRS_DriverBuckleSwitchStatus") != 0
    ret.parkingBrake = False

    # Vehicle Speed
    carspd = sig(cp, "GW_187", "ESP_VehicleSpeed")
    speed = carspd if carspd <= 5 else ((carspd / 0.98) + 2)
    ret.vEgoRaw = speed * CV.KPH_TO_MS
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)
    ret.vEgoCluster = ret.vEgo
    ret.standstill = abs(ret.vEgoRaw) < 1e-3

    # Gas, Brake, Gear
    if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      ret.brakePressed = sig(cp, "GW_1A6", "EMS_BrakePedalStatus") != 0
      ret.gasPressed = sig(cp, "GW_1A6", "EMS_AccPedal") > 1e-3
    else:
      ret.brakePressed = sig(cp, "GW_196", "EMS_BrakePedalStatus") != 0
      ret.gasPressed = sig(cp, "GW_196", "EMS_AccPedal") > 1e-3

    # Gear - different message/signal per model
    if self.CP.carFingerprint == CAR.CHANGAN_Z6:
      can_gear = int(cp.vl["GW_331"]["TCU_GearForDisplay"])  # changan.dbc Z6
    elif self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      can_gear = int(cp.vl["GW_338"]["TCU_GearForDisplay"])  # changan.dbc Z6_IDD
    else:  # QIYUAN_A05, QIYUAN_A07
      can_gear = int(cp.vl["GEAR"]["gearShifter"])  # changan_can.dbc
    ret.gearShifter = self.parse_gear_shifter(self.shifter_values.get(can_gear, None))
    ret.leftBlindspot = False   # 盲区
    ret.rightBlindspot = False  # 盲区

    # Lights
    ret.leftBlinker, ret.rightBlinker = self.update_blinker_from_stalk(200,
                                      sig(cp, "GW_28B", "BCM_TurnIndicatorLeft") > 0,
                                      sig(cp, "GW_28B", "BCM_TurnIndicatorRight") > 0)

    # Steering
    ret.steeringAngleOffsetDeg = 0
    ret.steeringAngleDeg = sig(cp, "GW_180", "SAS_SteeringAngle")
    ret.steeringRateDeg = cp.vl["GW_180"]["SAS_SteeringAngleSpeed"] # 转向速率: 8位 (23|8), 范围: [0, 255] 度/秒 2车一致 OK
    ret.steeringTorque = cp.vl["GW_17E"]["EPS_MeasuredTorsionBarTorque"] # 驾驶员扭矩 (扭杆): 12位 (7|12), 范围: [0, 4.095] Nm 2车一致 OK
    ret.steeringTorqueEps = cp.vl["GW_170"]["EPS_ActualTorsionBarTorq"] * self.eps_torque_scale # EPS 实际转矩: 16位 (23|16), 范围: [-5533, 60002] 物理值  需要仔细确认

    if self.steeringPressed:
      if abs(ret.steeringTorque) < self.steeringPressedMin and abs(ret.steeringAngleDeg) < 90:
        self.steeringPressed = False
    else:
      if abs(ret.steeringTorque) > self.steeringPressedMax:
        self.steeringPressed = True
    ret.steeringPressed = self.steeringPressed

    # 巡航状态 (Cruise State) - 信任原车 PCM 状态
    # 长安通过 GW_31A 或 GW_244 广播当前 ACC 状态

    # 状态映射:
    # ACC_ACCMode (GW_244) / ACC_IACCHWAMode (GW_31A)
    # 2: Ready/Standby, 3: Active, 4: Override, 7: Fault

    acc_mode = sig(cp_cam, "GW_31A", "ACC_IACCHWAMode")
    ret.cruiseState.available = True # 只要没有故障，Openpilot 随时准备接管
    ret.cruiseState.enabled = (acc_mode == 3) # 仅当原车进入 Active 模式时认为 Enabled

    # 巡航速度 (Set Speed)
    # 尝试从 GW_307 读取仪表显示的设定速度
    ret.cruiseState.speed = sig(cp_cam, "GW_307", "ACC_SetSpeed") * CV.KPH_TO_MS
    ret.cruiseState.standstill = ret.standstill

    # 系统故障与安全预警 (Faults & Safety)
    # ACC 故障：当 ACC 或 IACC/HWA 模式为 7 时，判定为系统故障
    ret.accFaulted = sig(cp_cam, "GW_244", "ACC_ACCMode") == 7 or sig(cp_cam, "GW_31A", "ACC_IACCHWAMode") == 7
    # 转向临时故障：EPS 的横向控制可用性状态。正常应为 1，若为 0 或 2 则判定为故障
    ret.steerFaultTemporary = sig(cp, "GW_17E", "EPS_LatCtrlAvailabilityStatus") != 1
    ret.steerFaultPermanent = False

    # 原车 ADAS 状态透传
    ret.stockFcw = sig(cp_cam, "GW_244", "ACC_FCWPreWarning") == 1
    ret.stockAeb = sig(cp_cam, "GW_244", "ACC_AEBCtrlType") > 0
    ret.genericToggle = False

    # 供控制器使用的信号快照 (用于同步计数器及保留原车不相关的状态位)
    self.sigs244 = copy.copy(cp_cam.vl["GW_244"]) #加速度
    self.sigs1ba = copy.copy(cp_cam.vl["GW_1BA"]) #方向盘
    self.sigs17e = copy.copy(cp.vl["GW_17E"])
    self.sigs307 = copy.copy(cp_cam.vl["GW_307"])
    self.sigs31a = copy.copy(cp_cam.vl["GW_31A"])

    # 滚动计数器提取 (用于在控制器中步进及同步序列号)
    self.counter_244 = sig(cp_cam, "GW_244", "ACC_RollingCounter_24E")
    self.counter_1ba = sig(cp_cam, "GW_1BA", "ACC_RollingCounter_1BA")
    self.counter_17e = sig(cp, "GW_17E", "EPS_RollingCounter_17E")
    self.counter_307 = sig(cp_cam, "GW_307", "ACC_RollingCounter_35E")
    self.counter_31a = sig(cp_cam, "GW_31A", "ACC_RollingCounter_36D")

    # 跟车距离按钮状态采集与追踪
    self.prev_distance_button = self.distance_button
    self.distance_button = sig(cp_cam, "GW_307", "ACC_DistanceLevel")

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
    ]

    # Different gear message/signal per model:
    # - Z6 uses GW_331 with TCU_GearForDisplay (in changan.dbc)
    # - Z6_IDD uses GW_338 with TCU_GearForDisplay (in changan.dbc)
    # - A05/A07 use GEAR with gearShifter (in changan_can.dbc)
    if CP.carFingerprint == CAR.CHANGAN_Z6:
      pt_messages += [("GW_331", 10)]  # TCU_GearForDisplay in changan.dbc Z6
    elif CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      pt_messages += [("GW_338", 10)]  # TCU_GearForDisplay in changan.dbc Z6_IDD
    else:  # QIYUAN_A05, QIYUAN_A07
      pt_messages += [("GEAR", 10)]  # gearShifter in changan_can.dbc

    pt_messages += [("GW_187", 100)]

    if CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
      pt_messages += [("GW_1A6", 100)]
    else: # Z6, A05, A07
      pt_messages += [("GW_196", 100)]

    cam_messages = [
      ("GW_1BA", 100),
      ("GW_244", 50),
      ("GW_307", 10),
      ("GW_31A", 100),
    ]

    return {
      Bus.pt: CANParser(DBC[CP.carFingerprint][Bus.pt], pt_messages, 0),
      Bus.cam: CANParser(DBC[CP.carFingerprint][Bus.pt], cam_messages, 2),
    }
