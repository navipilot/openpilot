#!/usr/bin/env python3
from opendbc.car import get_safety_config, structs
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.interfaces import CarInterfaceBase
from opendbc.car.changan.values import CarControllerParams, ChanganFlags
from opendbc.car.changan.carcontroller import CarController
from opendbc.car.changan.carstate import CarState
from opendbc.car.changan.radar_interface import RadarInterface

class CarInterface(CarInterfaceBase):
  CarState = CarState
  CarController = CarController
  RadarInterface = RadarInterface

  def check_comma_nn_ff_support(self, car):
    return False

  def initialize_lat_torque_nn(self, car, eps_firmware) -> bool:
    self.lat_torque_nn_model = None
    return False

  @staticmethod
  def get_pid_accel_limits(CP, current_speed, cruise_speed):
    return CarControllerParams.ACCEL_MIN, CarControllerParams.ACCEL_MAX

  @staticmethod
  def _get_params(ret: structs.CarParams, candidate, fingerprint, car_fw, alpha_long, is_release, docs) -> structs.CarParams:

    ret.brand = "changan"
    ret.safetyConfigs = [get_safety_config(structs.CarParams.SafetyModel.changan)]

    ret.transmissionType = structs.CarParams.TransmissionType.automatic
    ret.radarUnavailable = True
    ret.enableBsm = True

    # Steering
    ret.steerActuatorDelay = 0.12
    ret.steerLimitTimer = 1.0
    ret.steerControlType = structs.CarParams.SteerControlType.angle
    ret.minSteerSpeed = 0.1

    # Longitudinal
    ret.alphaLongitudinalAvailable = True
    ret.openpilotLongitudinalControl = True
    ret.autoResumeSng = ret.openpilotLongitudinalControl

    ret.minEnableSpeed = -1.
    ret.longitudinalActuatorDelay = 0.3

    ret.vEgoStopping = 0.25
    ret.vEgoStarting = 0.25
    ret.stoppingDecelRate = 0.3
    ret.startingState = True
    ret.startAccel = 0.5
    ret.stopAccel = -0.5

    # Longitudinal Tuning (PID)
    tune = ret.longitudinalTuning
    tune.kpBP = [0., 5., 20., 40.]
    tune.kpV = [1.2, 1.0, 0.7, 0.5]
    tune.kiBP = [0., 5., 12., 20., 27.]
    tune.kiV = [0.3, 0.25, 0.2, 0.15, 0.1]

    return ret
