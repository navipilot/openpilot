import numpy as np
from opendbc.can.packer import CANPacker
from opendbc.car import Bus, apply_std_steer_angle_limits, structs
from opendbc.car.interfaces import CarControllerBase
from opendbc.car.changan import changancan
from opendbc.car.changan.values import CarControllerParams, CAR
from openpilot.common.realtime import DT_CTRL
from openpilot.common.conversions import Conversions as CV


class CarController(CarControllerBase):
  def __init__(self, dbc_names, CP):
    super().__init__(dbc_names, CP)
    self.params = CarControllerParams(self.CP)
    self.packer = CANPacker(dbc_names[Bus.pt])
    self.frame = 0
    self.last_angle = 0
    self.last_acctrq = -5000
    self.first_start = True

    self.counter_244 = 0
    self.counter_1ba = 0
    self.counter_17e = 0
    self.counter_307 = 0
    self.counter_31a = 0

  def update(self, CC, CS, now_nanos):
    actuators = CC.actuators
    hud_control = CC.hudControl

    if self.first_start:
      self.counter_244 = int(CS.counter_244) & 0xF
      self.counter_1ba = int(CS.counter_1ba) & 0xF
      self.counter_17e = int(CS.counter_17e) & 0xF
      self.counter_307 = int(CS.counter_307) & 0xF
      self.counter_31a = int(CS.counter_31a) & 0xF
      self.last_angle = CS.out.steeringAngleDeg
      self.first_start = False

    can_sends = []

    # 计数器步进
    self.counter_1ba = (self.counter_1ba + 1) & 0xF
    self.counter_17e = (self.counter_17e + 1) & 0xF

    # 1. 横向控制 (Lateral Control)
    lat_active = CC.latActive and not CS.steeringPressed

    if lat_active:
      apply_angle = actuators.steeringAngleDeg + CS.out.steeringAngleOffsetDeg
      apply_angle = apply_std_steer_angle_limits(
        apply_angle, self.last_angle, CS.out.vEgoRaw,
        CS.out.steeringAngleDeg, lat_active, self.params.ANGLE_LIMITS
      )
      apply_angle = np.clip(apply_angle, -self.params.MAX_STEERING_ANGLE, self.params.MAX_STEERING_ANGLE)
    else:
      apply_angle = CS.out.steeringAngleDeg

    self.last_angle = apply_angle

    can_sends.append(changancan.create_steering_control(
      self.packer, CS.sigs1ba, apply_angle, lat_active, self.counter_1ba
    ))
    can_sends.append(changancan.create_eps_control(
      self.packer, CS.sigs17e, lat_active, self.counter_17e
    ))

    # 2. 纵向控制 (Longitudinal Control, 50Hz)
    if self.frame % 2 == 0:
      self.counter_244 = (self.counter_244 + 1) & 0xF

      accel = np.clip(actuators.accel, self.params.ACCEL_MIN, self.params.ACCEL_MAX)
      acctrq = -5000

      if CC.longActive:
        can_sends.append(changancan.create_acc_control(
          self.packer, CS.sigs244, accel, self.counter_244, CC.longActive, acctrq
        ))
      else:
        can_sends.append(changancan.create_acc_control(
          self.packer, CS.sigs244, 0.0, self.counter_244, False, -5000
        ))

    # 3. HUD 控制 (10Hz)
    if self.frame % 10 == 0:
      self.counter_307 = (self.counter_307 + 1) & 0xF
      self.counter_31a = (self.counter_31a + 1) & 0xF

      cruise_speed_kph = CS.out.cruiseState.speed * CV.MS_TO_KPH
      can_sends.append(changancan.create_acc_set_speed(
        self.packer, CS.sigs307, self.counter_307, cruise_speed_kph
      ))
      can_sends.append(changancan.create_acc_hud(
        self.packer, CS.sigs31a, self.counter_31a, CC.longActive, CS.out.steeringPressed
      ))

    new_actuators = actuators.as_builder()
    new_actuators.steeringAngleDeg = float(self.last_angle)
    new_actuators.accel = float(actuators.accel)

    self.frame += 1
    return new_actuators, can_sends
