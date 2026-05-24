import copy
from opendbc.can import CANDefine, CANParser
from opendbc.car import Bus, structs
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.interfaces import CarStateBase
from opendbc.car.tesla.values import DBC, CANBUS, GEAR_MAP, STEER_THRESHOLD

ButtonType = structs.CarState.ButtonEvent.Type

# DTR_Dist_Rq: 0/33/66/100/133/166/200 = ACC_DIST_1~7, 255=SNA
DTR_DIST_MAP = {0: 1, 33: 2, 66: 3, 100: 4, 133: 5, 166: 6, 200: 7}


class CarState(CarStateBase):
  def __init__(self, CP):
    super().__init__(CP)
    self.can_define = CANDefine(DBC[CP.carFingerprint][Bus.party])
    self.shifter_values = self.can_define.dv["DI_systemStatus"]["DI_gear"]

    self.hands_on_level = 0
    self.das_control = None

  def update(self, can_parsers) -> structs.CarState:
    cp_party = can_parsers[Bus.party]
    cp_ap_party = can_parsers[Bus.ap_party]
    ret = structs.CarState()
    length = 0.11

    # Vehicle speed
    ret.vEgoRaw = cp_party.vl["DI_speed"]["DI_vehicleSpeed"] * CV.KPH_TO_MS
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)

    # Wheel speeds (km/h -> m/s)
    ws = cp_party.vl["ESP_wheelSpeeds"]
    ret.wheelSpeeds.fl = ws["ESP_wheelSpeedFrL"] * CV.KPH_TO_MS
    ret.wheelSpeeds.fr = ws["ESP_wheelSpeedFrR"] * CV.KPH_TO_MS
    ret.wheelSpeeds.rl = ws["ESP_wheelSpeedReL"] * CV.KPH_TO_MS
    ret.wheelSpeeds.rr = ws["ESP_wheelSpeedReR"] * CV.KPH_TO_MS

    # Displayed speed
    ui_speed_units_raw = int(cp_party.vl["DI_speed"]["DI_uiSpeedUnits"])
    ui_speed_units = self.can_define.dv.get("DI_speed", {}).get("DI_uiSpeedUnits", {}).get(ui_speed_units_raw, ui_speed_units_raw)
    if ui_speed_units in ("DI_SPEED_KPH", "KPH", 0):
      ret.vEgoCluster = cp_party.vl["DI_speed"]["DI_uiSpeed"] * CV.KPH_TO_MS
    elif ui_speed_units in ("DI_SPEED_MPH", "MPH", 1):
      ret.vEgoCluster = cp_party.vl["DI_speed"]["DI_uiSpeed"] * CV.MPH_TO_MS

    # Gas pedal
    pedal_status = cp_party.vl["DI_systemStatus"]["DI_accelPedalPos"]
    ret.gas = pedal_status / 100.0
    ret.gasPressed = pedal_status > 0

    # Motor speed (EV: motor RPM from inverter)
    ret.engineRpm = cp_party.vl["DI_torque"]["DI_axleSpeed"]

    # Brake pedal
    # Brake pedal position (0.0-1.0) from iBooster push-rod displacement [0,47] mm
    brake_rod = cp_party.vl["IBST_status"]["IBST_sInputRodDriver"]
    ret.brake = max(0.0, brake_rod / 47.0) if brake_rod > 0 else 0.0
    ret.brakePressed = cp_party.vl["IBST_status"]["IBST_driverBrakeApply"] == 2
    ret.brakeLights = cp_party.vl["ESP_status"]["ESP_brakeLamp"] == 1
    ret.regenBraking = cp_party.vl["DI_systemStatus"]["DI_regenLight"] != 0
    ret.espDisabled = cp_party.vl["ESP_status"]["ESP_espFaultLamp"] != 0
    ret.espActive = cp_party.vl["ESP_status"]["ESP_espModeActive"] != 0

    # Steering wheel
    epas_status = cp_party.vl["EPAS3S_sysStatus"]
    self.hands_on_level = epas_status["EPAS3S_handsOnLevel"]
    ret.steeringAngleDeg = -epas_status["EPAS3S_internalSAS"]
    ret.steeringRateDeg = -cp_ap_party.vl["SCCM_steeringAngleSensor"]["SCCM_steeringAngleSpeed"]
    ret.steeringTorque = -epas_status["EPAS3S_torsionBarTorque"]
    ret.steeringTorqueEps = -epas_status["EPAS3S_steeringRackForce"] * length / self.CP.steerRatio

    # Stock handsOnLevel uses >0.5 for 0.25s, but this threshold reacts faster.
    ret.steeringPressed = self.update_steering_pressed(abs(ret.steeringTorque) > STEER_THRESHOLD, 5)

    eac_status = self.can_define.dv["EPAS3S_sysStatus"]["EPAS3S_eacStatus"].get(int(epas_status["EPAS3S_eacStatus"]), None)
    ret.steerFaultPermanent = eac_status == "EAC_FAULT"
    ret.steerFaultTemporary = eac_status == "EAC_INHIBITED"
    ret.vehicleSensorsInvalid = cp_ap_party.vl["SCCM_steeringAngleSensor"]["SCCM_steeringAngleValidity"] != 1

    # Cruise state
    cruise_state = self.can_define.dv["DI_state"]["DI_cruiseState"].get(int(cp_party.vl["DI_state"]["DI_cruiseState"]), None)
    speed_units = self.can_define.dv["DI_state"]["DI_speedUnits"].get(int(cp_party.vl["DI_state"]["DI_speedUnits"]), None)

    scale_speed = 1.01
    ret.cruiseState.enabled = cruise_state in ("ENABLED", "STANDSTILL", "OVERRIDE", "PRE_FAULT", "PRE_CANCEL")
    if speed_units in ("KPH", "DI_SPEED_KPH", 0):
      ret.cruiseState.speedCluster = cp_party.vl["DI_state"]["DI_digitalSpeed"] * CV.KPH_TO_MS
    elif speed_units in ("MPH", "DI_SPEED_MPH", 1):
      ret.cruiseState.speedCluster = cp_party.vl["DI_state"]["DI_digitalSpeed"] * CV.MPH_TO_MS
    ret.cruiseState.speed = max(ret.cruiseState.speedCluster / scale_speed, 1e-3)
    ret.cruiseState.available = cruise_state == "STANDBY" or ret.cruiseState.enabled
    ret.cruiseState.standstill = False  # This needs to be false, since we can resume from stop without sending anything special
    ret.standstill = cruise_state == "STANDSTILL"
    ret.accFaulted = cruise_state == "FAULT"

    # DAS_fusedSpeedLimit is DBC-scaled to kph/mph (0=unknown, 31=none).
    speed_limit = cp_ap_party.vl["DAS_status"]["DAS_fusedSpeedLimit"]
    if 0 < speed_limit <= 150:
      # Use same units as displayed speed
      if ui_speed_units in ("DI_SPEED_KPH", "KPH", 0):
        ret.speedLimit = speed_limit
      else:
        ret.speedLimit = speed_limit * CV.MPH_TO_KPH

    park_brake_state = self.can_define.dv["DI_state"]["DI_parkBrakeState"].get(int(cp_party.vl["DI_state"]["DI_parkBrakeState"]), None)
    vehicle_hold_state = self.can_define.dv["DI_state"]["DI_vehicleHoldState"].get(int(cp_party.vl["DI_state"]["DI_vehicleHoldState"]), None)
    ret.parkingBrake = park_brake_state == "APPLIED"
    ret.brakeHoldActive = vehicle_hold_state == "STANDSTILL"

    # Gear
    ret.gearShifter = GEAR_MAP[self.can_define.dv["DI_systemStatus"]["DI_gear"].get(int(cp_party.vl["DI_systemStatus"]["DI_gear"]), "DI_GEAR_INVALID")]

    # Doors
    ret.doorOpen = cp_party.vl["UI_warning"]["anyDoorOpen"] == 1

    # Blinkers
    ret.leftBlinker = cp_party.vl["UI_warning"]["leftBlinkerBlinking"] in (1, 2)
    ret.rightBlinker = cp_party.vl["UI_warning"]["rightBlinkerBlinking"] in (1, 2)

    # High beam stalk used as generic toggle (openpilot convention)
    ret.genericToggle = cp_party.vl["UI_warning"]["highBeam"] == 1

    # Seatbelt
    ret.seatbeltUnlatched = cp_party.vl["UI_warning"]["buckleStatus"] != 1

    # Blindspot
    ret.leftBlindspot = cp_ap_party.vl["DAS_status"]["DAS_blindSpotRearLeft"] != 0
    ret.rightBlindspot = cp_ap_party.vl["DAS_status"]["DAS_blindSpotRearRight"] != 0

    # AEB
    ret.stockAeb = cp_ap_party.vl["DAS_control"]["DAS_aebEvent"] == 1
    ret.stockFcw = cp_ap_party.vl["DAS_status"]["DAS_forwardCollisionWarning"] != 0

    # Stock Autosteer should be off (includes FSD)
    ret.invalidLkasSetting = cp_ap_party.vl["DAS_settings"]["DAS_autosteerEnabled"] != 0

    # Battery SOC (State of Charge) from BMS
    ret.fuelGauge = cp_party.vl["ID292BMS_SOC"]["SOCUI292"] / 100.0  # Convert from % to 0.0-1.0

    # Yaw rate from RCM inertial sensor
    inertial1 = cp_ap_party.vl["ID101RCM_inertial1"]
    if inertial1["RCM_yawRateQF"] == 1:  # Quality flag: 1 = valid
      ret.yawRate = inertial1["RCM_yawRate"]

    # Following distance from scroll wheel (vehicle bus)
    cp_vehicle = can_parsers[Bus.vehicle]
    dtr_dist_rq = int(cp_vehicle.vl["STW_ACTN_RQ"]["DTR_Dist_Rq"])
    ret.pcmCruiseGap = DTR_DIST_MAP.get(dtr_dist_rq, 0)

    # Buttons # ToDo: add Gap adjust button

    # Messages needed by carcontroller
    self.das_control = copy.copy(cp_ap_party.vl["DAS_control"])

    return ret

  @staticmethod
  def get_can_parsers(CP):
    party_messages = [
      ("DI_speed", 10),
      ("ESP_wheelSpeeds", 50),
      ("DI_systemStatus", 10),
      ("DI_torque", 10),
      ("IBST_status", 50),
      ("ESP_status", 50),
      ("EPAS3S_sysStatus", 100),
      ("DI_state", 10),
      ("UI_warning", 10),
      ("ID292BMS_SOC", 10),
    ]

    ap_party_messages = [
      ("SCCM_steeringAngleSensor", 100),
      ("DAS_status", 10),
      ("DAS_control", 25),
      ("DAS_settings", 1),
      ("ID101RCM_inertial1", 100),
    ]

    vehicle_messages = [
      ("STW_ACTN_RQ", 0),
    ]

    return {
      Bus.party: CANParser(DBC[CP.carFingerprint][Bus.party], party_messages, CANBUS.party),
      Bus.ap_party: CANParser(DBC[CP.carFingerprint][Bus.party], ap_party_messages, CANBUS.autopilot_party),
      Bus.vehicle: CANParser(DBC[CP.carFingerprint][Bus.vehicle], vehicle_messages, CANBUS.vehicle),
    }
