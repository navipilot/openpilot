import math
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

    self.counter_244 = 0
    self.counter_1ba = 0
    self.counter_17e = 0
    self.counter_307 = 0
    self.counter_31a = 0

    self.last_apply_accel = 0.0
    self.last_acctrq = -5000
    self.stop_lead_distance = 0.0
    self.last_speed = 0.0
    self.expected_accel = 0.0
    self.actual_accel_filtered = 0.0
    self.slope_compensation = 0.0
    self.expected_daccel = 0.0
    self.actual_daccel_filtered = 0.0
    self.slope_daccel = 0.0

  def update(self, CC, CS, now_nanos):
    actuators = CC.actuators
    hud = CC.hudControl

    if self.first_start:
      self.counter_244 = int(CS.counter_244) & 0xF if hasattr(CS, 'counter_244') else 0
      self.counter_1ba = int(CS.counter_1ba) & 0xF
      self.counter_17e = int(CS.counter_17e) & 0xF
      self.counter_307 = int(CS.counter_307) & 0xF
      self.counter_31a = int(CS.counter_31a) & 0xF
      self.last_angle = CS.out.steeringAngleDeg
      self.first_start = False

    can_sends = []

    # 计数器步进（每帧 100Hz）
    self.counter_1ba = (self.counter_1ba + 1) & 0xF
    self.counter_17e = (self.counter_17e + 1) & 0xF

    # ── 横向控制 ─────────────────────────────────────────────────
    lat_active = CC.latActive and not CS.out.steeringPressed

    if lat_active:
      apply_angle = actuators.steeringAngleDeg + CS.out.steeringAngleOffsetDeg
      if not math.isfinite(apply_angle):
        apply_angle = CS.out.steeringAngleDeg
      else:
        apply_angle = apply_std_steer_angle_limits(
          apply_angle, self.last_angle, CS.out.vEgoRaw,
          CS.out.steeringAngleDeg, lat_active, self.params.ANGLE_LIMITS
        )
        apply_angle = np.clip(apply_angle, -self.params.MAX_STEERING_ANGLE, self.params.MAX_STEERING_ANGLE)
    else:
      apply_angle = CS.out.steeringAngleDeg if math.isfinite(CS.out.steeringAngleDeg) else 0.0

    self.last_angle = apply_angle

    # GW_1BA：转向指令（始终发送，每帧）
    can_sends.append(changancan.create_1BA_command(
      self.packer, CS.sigs1ba, apply_angle, lat_active, self.counter_1ba
    ))

    # GW_17E：EPS 心跳（始终发送，每帧，双总线）
    can_sends.extend(changancan.create_17E_command(
      self.packer, CS.sigs17e, CC.latActive, self.counter_17e
    ))

    # ── HUD 同步 (50Hz: frame % 2 == 0) ─────────────────────────────────
    if self.frame % 2 == 0:
      self.counter_307 = (self.counter_307 + 1) & 0xF
      self.counter_31a = (self.counter_31a + 1) & 0xF
      self.counter_244 = (self.counter_244 + 1) & 0xF

      # ── 纵向控制 ───────────────────────────────────────────────
      raw_accel = actuators.accel if math.isfinite(actuators.accel) else 0.0
      accel = np.clip(raw_accel,
                      self.params.ACCEL_MIN, self.params.ACCEL_MAX)

      if accel < 0:
        self.expected_daccel = accel
        self.actual_daccel_filtered = 0.9 * self.actual_daccel_filtered + 0.1 * CS.out.aEgo
        self.slope_daccel = 0.15 if self.actual_daccel_filtered > 0.8 * self.expected_daccel else 0.0
        accel -= self.slope_daccel
        accel = np.clip(accel,
                        self.last_apply_accel - 0.2,
                        self.last_apply_accel + 0.1)
        if self.last_apply_accel >= 0 and hud.leadVisible and hud.leadDistanceBars < 30:
          accel = -0.4
        accel = max(accel, -3.5)

        v_ego_kph = CS.out.vEgoRaw * CV.MS_TO_KPH
        if v_ego_kph == 0 and self.last_speed > 0 and hud.leadVisible and hud.leadDistanceBars > 0:
          self.stop_lead_distance = hud.leadDistanceBars
        if self.stop_lead_distance != 0 and v_ego_kph == 0 and self.last_speed == 0 and hud.leadVisible:
          if self.stop_lead_distance - hud.leadDistanceBars > 1:
            accel = 0.5
        if v_ego_kph > 0:
          self.stop_lead_distance = 0.0

      if accel > 0:
        speed_kph = CS.out.vEgoRaw * CV.MS_TO_KPH
        if speed_kph > 110:
          p, i = 1100, 150
        elif speed_kph > 90:
          p, i = 800, 120
        elif speed_kph > 70:
          p, i = 800, 100
        elif speed_kph > 50:
          p, i = 800, 80
        elif speed_kph > 10:
          p, i = 500, 50
        else:
          p, i = 400, 50
        acc_trq = int(abs(accel) / 0.05) * i + p - 5000

        self.expected_accel = accel
        self.actual_accel_filtered = 0.9 * self.actual_accel_filtered + 0.1 * CS.out.aEgo
        if self.actual_accel_filtered < 0.4 * self.expected_accel:
          self.slope_compensation += 10
        else:
          self.slope_compensation = max(0, self.slope_compensation - 10)
        acc_trq += self.slope_compensation
        acc_trq = min(acc_trq, -10)
        acc_trq = np.clip(acc_trq,
                          self.last_acctrq - 300,
                          self.last_acctrq + 100)
      else:
        acc_trq = -5000

      if CS.out.vEgoRaw * CV.MS_TO_KPH > 0:
        self.last_speed = CS.out.vEgoRaw * CV.MS_TO_KPH
      accel = int(accel / 0.05) * 0.05

      # GW_244：纵向 ACC（仅 Z6_IDD）
      if self.CP.carFingerprint == CAR.CHANGAN_Z6_IDD:
        can_sends.append(changancan.create_244_command_idd(
          self.packer, CS.sigs244, accel, self.counter_244,
          CC.latActive, acc_trq, CS.out.vEgoRaw
        ))

      # GW_307 / GW_31A：HUD 显示
      can_sends.append(changancan.create_307_command(
        self.packer, CS.sigs307, self.counter_307,
        CS.out.cruiseState.speedCluster * CV.MS_TO_KPH
      ))
      can_sends.append(changancan.create_31A_command(
        self.packer, CS.sigs31a, self.counter_31a,
        CC.latActive, CS.out.steeringPressed
      ))

    new_actuators = actuators.as_builder()
    new_actuators.steeringAngleDeg = float(self.last_angle)

    self.frame += 1
    return new_actuators, can_sends
