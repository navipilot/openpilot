import copy
from opendbc.can import CANDefine, CANParser
from opendbc.car import Bus, create_button_events, structs, DT_CTRL
from opendbc.car.carlog import carlog
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.interfaces import CarStateBase
from opendbc.car.tesla.values import DBC, CANBUS, GEAR_MAP, STEER_THRESHOLD, TeslaFlags

ButtonType = structs.CarState.ButtonEvent.Type


class CarState(CarStateBase):
  def __init__(self, CP):
    super().__init__(CP)
    self.can_define = CANDefine(DBC[CP.carFingerprint][Bus.party])
    self.shifter_values = self.can_define.dv["DI_systemStatus"]["DI_gear"]

    self.autopark = False
    self.autopark_prev = False
    self.cruise_enabled_prev = False
    self.fsd14_error_logged = False
    self.suspected_fsd14 = False

    self.hands_on_level = 0
    self.das_control = None
    self.steering_disengage = False
    self.infotainment_touch_points = 0

    # Speed limit distance tracking
    self.totalDistance = 0.0
    self.speedLimitDistance = 0

  def update_autopark_state(self, autopark_state: str, cruise_enabled: bool):
    autopark_now = autopark_state in ("ACTIVE", "COMPLETE", "SELFPARK_STARTED")
    if autopark_now and not self.autopark_prev and not self.cruise_enabled_prev:
      self.autopark = True
    if not autopark_now:
      self.autopark = False
    self.autopark_prev = autopark_now
    self.cruise_enabled_prev = cruise_enabled

  def update_speed_limit_distance(self, ret):
    """Calculate distance to speed limit sign - similar to Hyundai implementation"""
    self.totalDistance += ret.vEgo * DT_CTRL
    if ret.speedLimit > 0 and not ret.gasPressed:
      if self.speedLimitDistance <= self.totalDistance:
        # Estimate distance ahead based on current speed limit (6 seconds worth)
        self.speedLimitDistance = self.totalDistance + ret.speedLimit * 6
      self.speedLimitDistance = max(self.totalDistance + 1, self.speedLimitDistance)
    else:
      self.speedLimitDistance = self.totalDistance
    ret.speedLimitDistance = self.speedLimitDistance - self.totalDistance

  def update(self, can_parsers) -> structs.CarState:
    cp_party = can_parsers[Bus.party]
    cp_ap_party = can_parsers[Bus.ap_party]
    cp_adas = can_parsers.get(Bus.adas)
    ret = structs.CarState()

    # Vehicle speed
    ret.wheelSpeeds = self.get_wheel_speeds(
      cp_party.vl["ESP_wheelSpeeds"]["ESP_wheelSpeedFrL"],
      cp_party.vl["ESP_wheelSpeeds"]["ESP_wheelSpeedFrR"],
      cp_party.vl["ESP_wheelSpeeds"]["ESP_wheelSpeedReL"],
      cp_party.vl["ESP_wheelSpeeds"]["ESP_wheelSpeedReR"],
    )
    wheel_speeds_valid = cp_party.vl["ESP_B"]["ESP_wheelSpeedsQF"] == 1
    if wheel_speeds_valid:
      ret.vEgoRaw = (ret.wheelSpeeds.fl + ret.wheelSpeeds.fr + ret.wheelSpeeds.rl + ret.wheelSpeeds.rr) / 4.
    else:
      ret.vEgoRaw = cp_party.vl["DI_speed"]["DI_vehicleSpeed"] * CV.KPH_TO_MS
    ret.vEgo, ret.aEgo = self.update_speed_kf(ret.vEgoRaw)

    # Gas pedal
    pedal_status = cp_party.vl["DI_systemStatus"]["DI_accelPedalPos"]
    valid_pedal_status = pedal_status if 0 <= pedal_status <= 100 else 0
    ret.gas = valid_pedal_status / 100.0
    ret.gasPressed = valid_pedal_status > 0

    # Brake pedal
    ret.brake = 0
    ret.brakePressed = cp_party.vl["ESP_status"]["ESP_driverBrakeApply"] == 2

    # Steering wheel
    epas_status = cp_party.vl["EPAS3S_sysStatus"]
    self.hands_on_level = epas_status["EPAS3S_handsOnLevel"]
    ret.steeringAngleDeg = -epas_status["EPAS3S_internalSAS"]
    ret.steeringRateDeg = -cp_ap_party.vl["SCCM_steeringAngleSensor"]["SCCM_steeringAngleSpeed"]
    ret.steeringTorque = -epas_status["EPAS3S_torsionBarTorque"]
    # EPS output torque - steering rack force converted to approximate torque
    # EPAS3S_steeringRackForce is in Newtons, scale: 50, offset: -25575
    ret.steeringTorqueEps = epas_status["EPAS3S_steeringRackForce"] / 500.0  # Approximate conversion

    ret.steeringPressed = self.update_steering_pressed(abs(ret.steeringTorque) > STEER_THRESHOLD, 5)

    eac_status = self.can_define.dv["EPAS3S_sysStatus"]["EPAS3S_eacStatus"].get(int(epas_status["EPAS3S_eacStatus"]), None)
    ret.steerFaultPermanent = eac_status == "EAC_FAULT"
    ret.steerFaultTemporary = eac_status == "EAC_INHIBITED"

    eac_error_code = self.can_define.dv["EPAS3S_sysStatus"]["EPAS3S_eacErrorCode"].get(int(epas_status["EPAS3S_eacErrorCode"]), None)
    self.steering_disengage = self.hands_on_level >= 3 or (eac_status == "EAC_INHIBITED" and
                                 eac_error_code == "EAC_ERROR_HIGH_ANGLE_RATE_SAFETY")

    # Cruise state
    cruise_state = self.can_define.dv["DI_state"]["DI_cruiseState"].get(int(cp_party.vl["DI_state"]["DI_cruiseState"]), None)
    speed_units = self.can_define.dv["DI_state"]["DI_speedUnits"].get(int(cp_party.vl["DI_state"]["DI_speedUnits"]), None)
    ui_speed_units = self.can_define.dv["DI_speed"]["DI_uiSpeedUnits"].get(int(cp_party.vl["DI_speed"]["DI_uiSpeedUnits"]), None)
    park_brake_state = self.can_define.dv["DI_state"]["DI_parkBrakeState"].get(int(cp_party.vl["DI_state"]["DI_parkBrakeState"]), None)
    vehicle_hold_state = self.can_define.dv["DI_state"]["DI_vehicleHoldState"].get(int(cp_party.vl["DI_state"]["DI_vehicleHoldState"]), None)
    traction_control_mode = self.can_define.dv["DI_systemStatus"]["DI_tractionControlMode"].get(int(cp_party.vl["DI_systemStatus"]["DI_tractionControlMode"]), None)

    autopark_state = self.can_define.dv["DI_state"]["DI_autoparkState"].get(int(cp_party.vl["DI_state"]["DI_autoparkState"]), None)
    cruise_enabled = cruise_state in ("ENABLED", "STANDSTILL", "OVERRIDE", "PRE_FAULT", "PRE_CANCEL")
    self.update_autopark_state(autopark_state, cruise_enabled)

    ret.cruiseState.enabled = cruise_enabled and not self.autopark
    if speed_units == "KPH":
      ret.cruiseState.speed = max(cp_party.vl["DI_state"]["DI_digitalSpeed"] * CV.KPH_TO_MS, 1e-3)
    elif speed_units == "MPH":
      ret.cruiseState.speed = max(cp_party.vl["DI_state"]["DI_digitalSpeed"] * CV.MPH_TO_MS, 1e-3)

    # Set vCruise and vCruiseCluster for display
    ret.vCruise = ret.cruiseState.speed
    # vCruiseCluster uses the same value but could be adjusted for display purposes
    ret.vCruiseCluster = ret.cruiseState.speed

    cluster_speed = cp_party.vl["DI_speed"]["DI_uiSpeed"]
    if cluster_speed < 255:
      if ui_speed_units == "DI_SPEED_KPH":
        ret.vEgoCluster = cluster_speed * CV.KPH_TO_MS
      elif ui_speed_units == "DI_SPEED_MPH":
        ret.vEgoCluster = cluster_speed * CV.MPH_TO_MS
    ret.cruiseState.available = cruise_state == "STANDBY" or ret.cruiseState.enabled
    ret.cruiseState.standstill = False  # This needs to be false, since we can resume from stop without sending anything special
    ret.cruiseState.nonAdaptive = False
    ret.standstill = cp_party.vl["ESP_B"]["ESP_vehicleStandstillSts"] == 1
    ret.accFaulted = cruise_state == "FAULT"
    ret.vCluRatio = (ret.vEgo / ret.vEgoCluster) if (ret.vEgoCluster > 3. and ret.vEgo > 3.) else 1.0

    fused_speed_limit = cp_ap_party.vl["DAS_status"]["DAS_fusedSpeedLimit"]
    vision_speed_limit = cp_ap_party.vl["DAS_status"]["DAS_visionOnlySpeedLimit"]
    # These signals are already scaled by the parser to kph/mph. Raw value 31 is the "NONE" sentinel,
    # which appears here as 155 because the DBC scale is 5.
    speed_limit_raw = fused_speed_limit if 0 < fused_speed_limit <= 150 else vision_speed_limit if 0 < vision_speed_limit <= 150 else 0
    if speed_units == "KPH":
      ret.speedLimit = speed_limit_raw * CV.KPH_TO_MS
    elif speed_units == "MPH":
      ret.speedLimit = speed_limit_raw * CV.MPH_TO_MS
    else:
      ret.speedLimit = 0

    # Gear
    ret.gearShifter = GEAR_MAP[self.can_define.dv["DI_systemStatus"]["DI_gear"].get(int(cp_party.vl["DI_systemStatus"]["DI_gear"]), "DI_GEAR_INVALID")]

    # Doors
    ret.doorOpen = cp_party.vl["UI_warning"]["anyDoorOpen"] == 1

    # Blinkers
    ret.leftBlinker = cp_party.vl["UI_warning"]["leftBlinkerBlinking"] in (1, 2)
    ret.rightBlinker = cp_party.vl["UI_warning"]["rightBlinkerBlinking"] in (1, 2)

    # Seatbelt
    ret.seatbeltUnlatched = cp_party.vl["UI_warning"]["buckleStatus"] != 1

    # Blindspot
    ret.leftBlindspot = cp_ap_party.vl["DAS_status"]["DAS_blindSpotRearLeft"] in (1, 2)
    ret.rightBlindspot = cp_ap_party.vl["DAS_status"]["DAS_blindSpotRearRight"] in (1, 2)

    # AEB
    ret.stockAeb = cp_ap_party.vl["DAS_control"]["DAS_aebEvent"] == 1
    fcw_warning = cp_ap_party.vl["DAS_status"]["DAS_forwardCollisionWarning"] == 1 or cp_ap_party.vl["DAS_status2"]["DAS_longCollisionWarning"] not in (0, 15)
    ret.stockFcw = fcw_warning and not ret.stockAeb

    ret.parkingBrake = park_brake_state in ("APPLIED", "PANIC_EPB", "PANIC_SKID")
    ret.brakeHoldActive = vehicle_hold_state == "STANDSTILL"
    ret.brakeLights = cp_party.vl["ESP_status"]["ESP_brakeLamp"] == 1 or ret.brakePressed
    ret.regenBraking = cp_party.vl["DI_systemStatus"]["DI_regenLight"] == 1
    ret.espDisabled = traction_control_mode in ("TC_DEV_MODE_1", "TC_DEV_MODE_2", "TC_ROLLS_MODE", "TC_DYNO_MODE")
    ret.espActive = cp_party.vl["ESP_status"]["ESP_espLampFlash"] == 1
    ret.vehicleSensorsInvalid = not wheel_speeds_valid or cp_ap_party.vl["SCCM_steeringAngleSensor"]["SCCM_steeringAngleValidity"] != 1 or epas_status["EPAS3S_internalSASQF"] == 0

    # Stock Autosteer should be off (includes FSD)
    if not (self.CP.flags & TeslaFlags.MISSING_DAS_SETTINGS):
      ret.invalidLkasSetting = cp_ap_party.vl["DAS_settings"]["DAS_autosteerEnabled"] != 0

      angle_control = cp_ap_party.vl["DAS_steeringControl"]["DAS_steeringControlType"] == 1
      if not ret.invalidLkasSetting and angle_control and not self.CP.flags & TeslaFlags.FSD_14:
        self.suspected_fsd14 = True

      if self.suspected_fsd14:
        ret.invalidLkasSetting = True
        if not self.fsd14_error_logged:
          carlog.error("FSD 14 detected, but FW not in FSD_14_FW set")
          self.fsd14_error_logged = True

    if cp_adas is not None:
      prev_touch_points = self.infotainment_touch_points
      touch_points = cp_adas.vl["UI_status2"]["UI_activeTouchPoints"]
      self.infotainment_touch_points = int(touch_points) if 0 <= touch_points <= 10 else 0
      ret.buttonEvents = create_button_events(self.infotainment_touch_points, prev_touch_points, {3: ButtonType.lkas})

      soc_ui = cp_adas.vl["ID292BMS_SOC"]["SOCUI292"]
      if 0.0 <= soc_ui <= 102.3:
        ret.fuelGauge = min(100.0, soc_ui) / 100.0

      # Charging detection - Tesla doesn't expose a direct charging signal in standard CAN
      # We can infer charging state from various conditions:
      # 1. Vehicle at standstill with ready for drive status
      # 2. High SOC rate of change (would need historical tracking)
      # For now, set to False as there's no reliable direct signal
      # Future enhancement: track SOC changes over time to infer charging
      ret.charging = False

    # Update speed limit distance calculation
    self.update_speed_limit_distance(ret)

    # pcmCruiseGap - Tesla doesn't expose following distance setting in CAN
    # Set to 0 to indicate unavailable
    ret.pcmCruiseGap = 0

    # Messages needed by carcontroller
    self.das_control = copy.copy(cp_ap_party.vl["DAS_control"])

    return ret

  @staticmethod
  def get_can_parsers(CP):
    return {
      Bus.party: CANParser(DBC[CP.carFingerprint][Bus.party], [], CANBUS.party),
      Bus.ap_party: CANParser(DBC[CP.carFingerprint][Bus.party], [], CANBUS.autopilot_party),
      Bus.adas: CANParser(DBC[CP.carFingerprint][Bus.adas], [], CANBUS.vehicle),
    }
