import numpy as np
from opendbc.can.packer import CANPacker
from opendbc.car import Bus, apply_std_steer_angle_limits, structs
from opendbc.car.interfaces import CarControllerBase
from opendbc.car.changan import changancan
from opendbc.car.changan.values import CarControllerParams, CAR
from openpilot.common.conversions import Conversions as CV


class CarController(CarControllerBase):
  def __init__(self, dbc_names, CP):
    super().__init__(dbc_names, CP)
    self.params = CarControllerParams(self.CP)
    self.packer = CANPacker(dbc_names[Bus.pt])
    self.frame = 0
    self.last_angle = 0
    self.first_start = True

    self.counter_1ba = 0
    self.counter_17e = 0
    self.counter_307 = 0
    self.counter_31a = 0

  def update(self, CC, CS, now_nanos):
    actuators = CC.actuators

    if self.first_start:
      self.counter_1ba = int(CS.counter_1ba) & 0xF
      self.counter_17e = int(CS.counter_17e) & 0xF
      self.counter_307 = int(CS.counter_307) & 0xF
      self.counter_31a = int(CS.counter_31a) & 0xF
      self.last_angle = CS.out.steeringAngleDeg
      self.first_start = False

    can_sends = []

    # 计数器步进（每帧，100Hz）
    self.counter_1ba = (self.counter_1ba + 1) & 0xF
    self.counter_17e = (self.counter_17e + 1) & 0xF

    # ── 横向控制 (Lateral) ────────────────────────────────────────
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

    # GW_1BA：转向指令（始终发送，inactive 时 active=0）
    can_sends.append(changancan.create_steering_control(
      self.packer, CS.sigs1ba, apply_angle, lat_active, self.counter_1ba
    ))

    # GW_17E：EPS 心跳（始终发送）
    can_sends.append(changancan.create_eps_control(
      self.packer, CS.sigs17e, lat_active, self.counter_17e
    ))

    # ── HUD 同步 (10Hz) ──────────────────────────────────────────
    # 纵向由原车 ACC 负责，这里只同步巡航速度显示和 ADAS 状态
    if self.frame % 10 == 0:
      self.counter_307 = (self.counter_307 + 1) & 0xF
      self.counter_31a = (self.counter_31a + 1) & 0xF

      cruise_speed_kph = CS.out.cruiseState.speed * CV.MS_TO_KPH
      can_sends.append(changancan.create_acc_set_speed(
        self.packer, CS.sigs307, self.counter_307, cruise_speed_kph
      ))
      can_sends.append(changancan.create_acc_hud(
        self.packer, CS.sigs31a, self.counter_31a,
        CS.out.cruiseState.enabled, CS.out.steeringPressed
      ))

    new_actuators = actuators.as_builder()
    new_actuators.steeringAngleDeg = float(self.last_angle)

    self.frame += 1
    return new_actuators, can_sends