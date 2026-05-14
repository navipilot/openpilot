import math
import numpy as np
from opendbc.can import CANPacker
from opendbc.car import Bus, DT_CTRL
from opendbc.car.interfaces import CarControllerBase
from opendbc.car.tesla.teslacan import TeslaCAN
from opendbc.car.tesla.values import CarControllerParams
from opendbc.car.vehicle_model import VehicleModel


def get_safety_CP():
  # Match Tesla safety tuning to Model Y limits, same as upstream Tesla implementation.
  from opendbc.car.tesla.interface import CarInterface
  return CarInterface.get_non_essential_params("TESLA_MODEL_Y")


def apply_steer_angle_limits_vm(apply_angle, apply_angle_last, v_ego, steering_angle, lat_active, limits, vehicle_model):
  if not lat_active:
    return float(np.clip(steering_angle, -limits.ANGLE_LIMITS.STEER_ANGLE_MAX, limits.ANGLE_LIMITS.STEER_ANGLE_MAX))

  clipped_speed = max(v_ego, 0.3)
  dt = DT_CTRL * limits.STEER_STEP

  desired_curvature = vehicle_model.calc_curvature(math.radians(apply_angle), clipped_speed, 0.0)
  current_curvature = vehicle_model.calc_curvature(math.radians(apply_angle_last), clipped_speed, 0.0)

  max_curvature_rate = limits.MAX_LATERAL_JERK / (clipped_speed ** 2)
  limited_curvature = float(np.clip(desired_curvature,
                                    current_curvature - max_curvature_rate * dt,
                                    current_curvature + max_curvature_rate * dt))

  max_curvature = limits.MAX_LATERAL_ACCEL / (clipped_speed ** 2)
  limited_curvature = float(np.clip(limited_curvature, -max_curvature, max_curvature))

  target_angle = math.degrees(vehicle_model.get_steer_from_curvature(limited_curvature, clipped_speed, 0.0))
  limited_angle = float(np.clip(target_angle,
                                apply_angle_last - limits.MAX_ANGLE_RATE,
                                apply_angle_last + limits.MAX_ANGLE_RATE))

  return float(np.clip(limited_angle,
                       -limits.ANGLE_LIMITS.STEER_ANGLE_MAX,
                       limits.ANGLE_LIMITS.STEER_ANGLE_MAX))


class CarController(CarControllerBase):
  def __init__(self, dbc_names, CP):
    super().__init__(dbc_names, CP)
    self.apply_angle_last = 0
    self.packer = CANPacker(dbc_names[Bus.party])
    self.tesla_can = TeslaCAN(CP, self.packer)
    self.VM = VehicleModel(get_safety_CP())

  def update(self, CC, CS, now_nanos):
    actuators = CC.actuators
    can_sends = []

    cruise_cancel = CC.cruiseControl.cancel
    lat_active = CC.latActive and not CS.steering_disengage

    if self.frame % 2 == 0:
      self.apply_angle_last = apply_steer_angle_limits_vm(actuators.steeringAngleDeg, self.apply_angle_last, CS.out.vEgoRaw,
                                                          CS.out.steeringAngleDeg, lat_active, CarControllerParams, self.VM)

      can_sends.append(self.tesla_can.create_steering_control(self.apply_angle_last, lat_active, (self.frame // 2) % 16))

    if self.frame % 10 == 0:
      can_sends.append(self.tesla_can.create_steering_allowed((self.frame // 10) % 16))

    # Longitudinal control
    if self.CP.openpilotLongitudinalControl:
      if self.frame % 4 == 0:
        state = 13 if cruise_cancel else 4  # 4=ACC_ON, 13=ACC_CANCEL_GENERIC_SILENT
        accel = float(np.clip(actuators.accel, CarControllerParams.ACCEL_MIN, CarControllerParams.ACCEL_MAX))
        cntr = (self.frame // 4) % 8
        can_sends.append(self.tesla_can.create_longitudinal_command(state, accel, cntr, CS.out.vEgo, CC.longActive))

    else:
      # Increment counter so cancel is prioritized even without openpilot longitudinal
      if cruise_cancel:
        cntr = (CS.das_control["DAS_controlCounter"] + 1) % 8
        can_sends.append(self.tesla_can.create_longitudinal_command(13, 0, cntr, CS.out.vEgo, False))

    # TODO: HUD control
    new_actuators = actuators.as_builder()
    new_actuators.steeringAngleDeg = self.apply_angle_last

    self.frame += 1
    return new_actuators, can_sends
