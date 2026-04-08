import math
import numpy as np
from collections import deque

from cereal import log
from opendbc.car.gm.values import CAR as GM_CAR
from opendbc.car.lateral import get_friction
from openpilot.common.constants import ACCELERATION_DUE_TO_GRAVITY, CV
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.common.pid import PIDController
from openpilot.selfdrive.controls.lib.drive_helpers import MIN_SPEED
from openpilot.selfdrive.controls.lib.latcontrol import LatControl
from openpilot.starpilot.common.testing_grounds import testing_ground

# At higher speeds (25+mph) we can assume:
# Lateral acceleration achieved by a specific car correlates to
# torque applied to the steering rack. It does not correlate to
# wheel slip, or to speed.

# This controller applies torque to achieve desired lateral
# accelerations. To compensate for the low speed effects the
# proportional gain is increased at low speeds by the PID controller.
# Additionally, there is friction in the steering wheel that needs
# to be overcome to move it at all, this is compensated for too.

KP = 0.7
KI = 0.35

INTERP_SPEEDS = [1, 1.5, 2.0, 3.0, 5, 7.5, 10, 15, 30]
KP_INTERP = [250, 120, 65, 30, 11.5, 5.5, 3.5, 2.0, KP]

LOW_SPEED_X = [0, 10, 20, 30]
LOW_SPEED_Y = [12, 10.5, 8, 5]
MAX_LAT_JERK_UP = 2.5            # m/s^3

LP_FILTER_CUTOFF_HZ = 1.2
JERK_LOOKAHEAD_SECONDS = 0.19
JERK_GAIN = 0.22
LAT_ACCEL_REQUEST_BUFFER_SECONDS = 1.0
VERSION = 2
DEBUG_TORQUE_TUNE = False
FF_SCALE_BLEND_LAT_ACCEL = 0.05
DEADZONE_BOOST_LAT_ACCEL = 0.15
UNWIND_D_DES_THRESHOLD = -1.0
UNWIND_LAT_ACCEL_NEAR_ZERO = 0.3
MIN_LATERAL_CONTROL_SPEED = 0.3

BOLT_2022_2023_CARS = (
  GM_CAR.CHEVROLET_BOLT_ACC_2022_2023,
  GM_CAR.CHEVROLET_BOLT_ACC_2022_2023_PEDAL,
  GM_CAR.CHEVROLET_BOLT_CC_2022_2023,
)
BOLT_2018_2021_CARS = (
  GM_CAR.CHEVROLET_BOLT_CC_2018_2021,
)
BOLT_2017_CARS = (
  GM_CAR.CHEVROLET_BOLT_CC_2017,
)
BOLT_CARS = BOLT_2022_2023_CARS + BOLT_2018_2021_CARS + BOLT_2017_CARS

BOLT_2017_LATERAL_TESTING_GROUND_ID = testing_ground.id_3
BOLT_2017_STEER_RATIO_TEST_SCALE = 1.045
BOLT_2017_TORQUE_SCALE_BP = [0.0, 0.2, 0.5, 1.0, 1.5, 2.5]
BOLT_2017_TORQUE_SCALE_LEFT = [1.0, 1.0, 1.065, 1.060, 1.055, 1.045]
BOLT_2017_TORQUE_SCALE_RIGHT = [1.0, 1.0, 1.035, 1.020, 0.995, 0.985]
BOLT_2017_TRANSITION_SPEED = 10.0
BOLT_2017_PHASE_SCALE = 0.12
BOLT_2017_TURN_IN_BOOST_LEFT = 0.28
BOLT_2017_TURN_IN_BOOST_RIGHT = 0.18
BOLT_2017_UNWIND_TAPER_LEFT = 0.08
BOLT_2017_UNWIND_TAPER_RIGHT = 0.28

BOLT_2018_2021_LATERAL_TESTING_GROUND_ID = testing_ground.id_4
BOLT_2018_2021_STEER_RATIO_TEST_SCALE = 1.01
BOLT_2018_2021_TORQUE_GAIN_LEFT = 0.085
BOLT_2018_2021_TORQUE_GAIN_RIGHT = 0.055
BOLT_2018_2021_TORQUE_ONSET = 0.18
BOLT_2018_2021_TORQUE_ONSET_WIDTH = 0.08
BOLT_2018_2021_TORQUE_CUTOFF = 1.05
BOLT_2018_2021_TORQUE_CUTOFF_WIDTH = 0.24
BOLT_2018_2021_JERK_TAPER_CUTOFF = 0.42
BOLT_2018_2021_TRANSITION_SPEED = 8.5
BOLT_2018_2021_PHASE_SCALE = 0.10
BOLT_2018_2021_TURN_IN_BOOST_LEFT = 0.22
BOLT_2018_2021_TURN_IN_BOOST_RIGHT = 0.12
BOLT_2018_2021_UNWIND_TAPER_GAIN_LEFT = 0.72
BOLT_2018_2021_UNWIND_TAPER_GAIN_RIGHT = 0.84
BOLT_2018_2021_FRICTION_MULT = 1.03
BOLT_2018_2021_FRICTION_LAT_RISE = 0.24
BOLT_2018_2021_FRICTION_JERK_RISE = 0.28
BOLT_2018_2021_TURN_IN_THRESHOLD_REDUCTION_LEFT = 0.16
BOLT_2018_2021_TURN_IN_THRESHOLD_REDUCTION_RIGHT = 0.13
BOLT_2018_2021_UNWIND_THRESHOLD_INCREASE_LEFT = 0.12
BOLT_2018_2021_UNWIND_THRESHOLD_INCREASE_RIGHT = 0.17
BOLT_2018_2021_TURN_IN_FRICTION_BOOST_LEFT = 0.08
BOLT_2018_2021_TURN_IN_FRICTION_BOOST_RIGHT = 0.06
BOLT_2018_2021_UNWIND_FRICTION_REDUCTION_LEFT = 0.14
BOLT_2018_2021_UNWIND_FRICTION_REDUCTION_RIGHT = 0.19

BOLT_2022_2023_LATERAL_TESTING_GROUND_ID = testing_ground.id_5
BOLT_2022_2023_FF_GAIN_LEFT = 0.13
BOLT_2022_2023_FF_GAIN_RIGHT = 0.07
BOLT_2022_2023_FF_ONSET = 0.12
BOLT_2022_2023_FF_ONSET_WIDTH = 0.07
BOLT_2022_2023_FF_CUTOFF = 1.35
BOLT_2022_2023_FF_CUTOFF_WIDTH = 0.28
BOLT_2022_2023_TRANSITION_SPEED = 9.0
BOLT_2022_2023_PHASE_SCALE = 0.12
BOLT_2022_2023_TURN_IN_BOOST_LEFT = 0.12
BOLT_2022_2023_TURN_IN_BOOST_RIGHT = 0.06
BOLT_2022_2023_UNWIND_TAPER_LEFT = 0.24
BOLT_2022_2023_UNWIND_TAPER_RIGHT = 0.20
BOLT_2022_2023_FRICTION_MULT = 1.15
BOLT_2022_2023_FRICTION_LAT_RISE = 0.22
BOLT_2022_2023_FRICTION_JERK_RISE = 0.26
BOLT_2022_2023_TURN_IN_THRESHOLD_REDUCTION_LEFT = 0.14
BOLT_2022_2023_TURN_IN_THRESHOLD_REDUCTION_RIGHT = 0.08
BOLT_2022_2023_UNWIND_THRESHOLD_INCREASE_LEFT = 0.16
BOLT_2022_2023_UNWIND_THRESHOLD_INCREASE_RIGHT = 0.13
BOLT_2022_2023_TURN_IN_FRICTION_BOOST_LEFT = 0.08
BOLT_2022_2023_TURN_IN_FRICTION_BOOST_RIGHT = 0.04
BOLT_2022_2023_UNWIND_FRICTION_REDUCTION_LEFT = 0.17
BOLT_2022_2023_UNWIND_FRICTION_REDUCTION_RIGHT = 0.14


def get_friction_threshold(v_ego: float) -> float:
  # Keep the speed-scaled friction threshold behavior.
  return float(np.interp(v_ego, [1 * CV.MPH_TO_MS, 20 * CV.MPH_TO_MS, 75 * CV.MPH_TO_MS], [0.16, 0.19, 0.27]))


def bolt_2017_lateral_testing_ground_active() -> bool:
  return testing_ground.use(BOLT_2017_LATERAL_TESTING_GROUND_ID)


def _bolt_2017_low_speed_factor(v_ego: float) -> float:
  return 1.0 / (1.0 + (max(v_ego, 0.0) / BOLT_2017_TRANSITION_SPEED) ** 2)


def _bolt_2017_transition_phase(desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  return math.tanh((desired_lateral_accel * desired_lateral_jerk) / BOLT_2017_PHASE_SCALE)


def _bolt_2017_side_value(desired_lateral_accel: float, left_value: float, right_value: float) -> float:
  return left_value if desired_lateral_accel >= 0.0 else right_value


def get_bolt_2017_base_torque_scale(desired_lateral_accel: float) -> float:
  if desired_lateral_accel == 0.0:
    return 1.0

  scale_values = BOLT_2017_TORQUE_SCALE_LEFT if desired_lateral_accel > 0.0 else BOLT_2017_TORQUE_SCALE_RIGHT
  return float(np.interp(abs(desired_lateral_accel), BOLT_2017_TORQUE_SCALE_BP, scale_values))


def get_bolt_2017_torque_scale(desired_lateral_accel: float, desired_lateral_jerk: float = 0.0, v_ego: float = 30.0) -> float:
  base_scale = get_bolt_2017_base_torque_scale(desired_lateral_accel)
  if base_scale <= 1.0 or desired_lateral_jerk == 0.0:
    return base_scale

  low_speed_factor = _bolt_2017_low_speed_factor(v_ego)
  phase = _bolt_2017_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  unwind_weight = max(-phase, 0.0)
  turn_in_boost = 1.0 + (_bolt_2017_side_value(desired_lateral_accel, BOLT_2017_TURN_IN_BOOST_LEFT, BOLT_2017_TURN_IN_BOOST_RIGHT) *
                          turn_in_weight * (0.35 + 0.65 * low_speed_factor))
  unwind_taper = 1.0 - (_bolt_2017_side_value(desired_lateral_accel, BOLT_2017_UNWIND_TAPER_LEFT, BOLT_2017_UNWIND_TAPER_RIGHT) *
                         unwind_weight * (0.45 + 0.55 * low_speed_factor))
  return 1.0 + ((base_scale - 1.0) * turn_in_boost * max(unwind_taper, 0.0))


def bolt_2018_2021_lateral_testing_ground_active() -> bool:
  return testing_ground.use(BOLT_2018_2021_LATERAL_TESTING_GROUND_ID)


def _bolt_2018_2021_sigmoid(x: float) -> float:
  return 1.0 / (1.0 + math.exp(-x))


def _bolt_2018_2021_low_speed_factor(v_ego: float) -> float:
  return 1.0 / (1.0 + (max(v_ego, 0.0) / BOLT_2018_2021_TRANSITION_SPEED) ** 2)


def _bolt_2018_2021_transition_phase(desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  return math.tanh((desired_lateral_accel * desired_lateral_jerk) / BOLT_2018_2021_PHASE_SCALE)


def _bolt_2018_2021_side_value(desired_lateral_accel: float, left_value: float, right_value: float) -> float:
  return left_value if desired_lateral_accel >= 0.0 else right_value


def _bolt_2018_2021_transition_envelope(v_ego: float, desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  lat_factor = 1.0 - math.exp(-abs(desired_lateral_accel) / BOLT_2018_2021_FRICTION_LAT_RISE)
  jerk_factor = 1.0 - math.exp(-abs(desired_lateral_jerk) / BOLT_2018_2021_FRICTION_JERK_RISE)
  return _bolt_2018_2021_low_speed_factor(v_ego) * lat_factor * jerk_factor


def get_bolt_2018_2021_torque_scale(desired_lateral_accel: float) -> float:
  if desired_lateral_accel == 0.0:
    return 1.0

  gain = BOLT_2018_2021_TORQUE_GAIN_LEFT if desired_lateral_accel > 0.0 else BOLT_2018_2021_TORQUE_GAIN_RIGHT
  abs_lateral_accel = abs(desired_lateral_accel)
  onset = _bolt_2018_2021_sigmoid((abs_lateral_accel - BOLT_2018_2021_TORQUE_ONSET) / BOLT_2018_2021_TORQUE_ONSET_WIDTH)
  cutoff = _bolt_2018_2021_sigmoid((BOLT_2018_2021_TORQUE_CUTOFF - abs_lateral_accel) / BOLT_2018_2021_TORQUE_CUTOFF_WIDTH)
  return 1.0 + gain * onset * cutoff


def get_bolt_2018_2021_dynamic_torque_scale(desired_lateral_accel: float, desired_lateral_jerk: float, v_ego: float) -> float:
  base_scale = get_bolt_2018_2021_torque_scale(desired_lateral_accel)
  extra_scale = max(base_scale - 1.0, 0.0)
  low_speed_factor = _bolt_2018_2021_low_speed_factor(v_ego)
  phase = _bolt_2018_2021_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  jerk_taper = 1.0 / (1.0 + (abs(desired_lateral_jerk) / BOLT_2018_2021_JERK_TAPER_CUTOFF) ** 2)
  turn_in_boost = 1.0 + (_bolt_2018_2021_side_value(desired_lateral_accel, BOLT_2018_2021_TURN_IN_BOOST_LEFT, BOLT_2018_2021_TURN_IN_BOOST_RIGHT) *
                          turn_in_weight * low_speed_factor)
  unwind_weight = max(-phase, 0.0)
  unwind_taper = 1.0 - (_bolt_2018_2021_side_value(desired_lateral_accel, BOLT_2018_2021_UNWIND_TAPER_GAIN_LEFT, BOLT_2018_2021_UNWIND_TAPER_GAIN_RIGHT) *
                         unwind_weight * (0.55 + 0.45 * low_speed_factor))
  return 1.0 + (extra_scale * jerk_taper * turn_in_boost * max(unwind_taper, 0.0))


def get_bolt_2018_2021_friction_threshold(v_ego: float, desired_lateral_accel: float = 0.0, desired_lateral_jerk: float = 0.0) -> float:
  base_threshold = get_friction_threshold(v_ego)
  transition_envelope = _bolt_2018_2021_transition_envelope(v_ego, desired_lateral_accel, desired_lateral_jerk)
  phase = _bolt_2018_2021_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  unwind_weight = max(-phase, 0.0)
  threshold_scale = 1.0 - (_bolt_2018_2021_side_value(desired_lateral_accel, BOLT_2018_2021_TURN_IN_THRESHOLD_REDUCTION_LEFT, BOLT_2018_2021_TURN_IN_THRESHOLD_REDUCTION_RIGHT) *
                           transition_envelope * turn_in_weight)
  threshold_scale += (_bolt_2018_2021_side_value(desired_lateral_accel, BOLT_2018_2021_UNWIND_THRESHOLD_INCREASE_LEFT, BOLT_2018_2021_UNWIND_THRESHOLD_INCREASE_RIGHT) *
                      transition_envelope * unwind_weight)
  return base_threshold * min(max(threshold_scale, 0.82), 1.12)


def get_bolt_2018_2021_friction_scale(v_ego: float, desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  transition_envelope = _bolt_2018_2021_transition_envelope(v_ego, desired_lateral_accel, desired_lateral_jerk)
  phase = _bolt_2018_2021_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  unwind_weight = max(-phase, 0.0)
  friction_scale = BOLT_2018_2021_FRICTION_MULT
  friction_scale += (_bolt_2018_2021_side_value(desired_lateral_accel, BOLT_2018_2021_TURN_IN_FRICTION_BOOST_LEFT, BOLT_2018_2021_TURN_IN_FRICTION_BOOST_RIGHT) *
                     transition_envelope * turn_in_weight)
  friction_scale -= (_bolt_2018_2021_side_value(desired_lateral_accel, BOLT_2018_2021_UNWIND_FRICTION_REDUCTION_LEFT, BOLT_2018_2021_UNWIND_FRICTION_REDUCTION_RIGHT) *
                     transition_envelope * unwind_weight)
  return min(max(friction_scale, 0.88), 1.10)


def bolt_2022_2023_lateral_testing_ground_active() -> bool:
  return testing_ground.use(BOLT_2022_2023_LATERAL_TESTING_GROUND_ID)


def _bolt_2022_2023_sigmoid(x: float) -> float:
  return 1.0 / (1.0 + math.exp(-x))


def _bolt_2022_2023_low_speed_factor(v_ego: float) -> float:
  return 1.0 / (1.0 + (max(v_ego, 0.0) / BOLT_2022_2023_TRANSITION_SPEED) ** 2)


def _bolt_2022_2023_transition_phase(desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  return math.tanh((desired_lateral_accel * desired_lateral_jerk) / BOLT_2022_2023_PHASE_SCALE)


def _bolt_2022_2023_side_value(desired_lateral_accel: float, left_value: float, right_value: float) -> float:
  return left_value if desired_lateral_accel >= 0.0 else right_value


def _bolt_2022_2023_transition_envelope(v_ego: float, desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  lat_factor = 1.0 - math.exp(-abs(desired_lateral_accel) / BOLT_2022_2023_FRICTION_LAT_RISE)
  jerk_factor = 1.0 - math.exp(-abs(desired_lateral_jerk) / BOLT_2022_2023_FRICTION_JERK_RISE)
  return _bolt_2022_2023_low_speed_factor(v_ego) * lat_factor * jerk_factor


def get_bolt_2022_2023_ff_scale(desired_lateral_accel: float, desired_lateral_jerk: float, v_ego: float) -> float:
  if desired_lateral_accel == 0.0:
    return 1.0

  gain = _bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_FF_GAIN_LEFT, BOLT_2022_2023_FF_GAIN_RIGHT)
  abs_lateral_accel = abs(desired_lateral_accel)
  onset = _bolt_2022_2023_sigmoid((abs_lateral_accel - BOLT_2022_2023_FF_ONSET) / BOLT_2022_2023_FF_ONSET_WIDTH)
  cutoff = _bolt_2022_2023_sigmoid((BOLT_2022_2023_FF_CUTOFF - abs_lateral_accel) / BOLT_2022_2023_FF_CUTOFF_WIDTH)
  extra_scale = gain * onset * cutoff
  low_speed_factor = _bolt_2022_2023_low_speed_factor(v_ego)
  phase = _bolt_2022_2023_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  unwind_weight = max(-phase, 0.0)
  turn_in_boost = 1.0 + (_bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_TURN_IN_BOOST_LEFT, BOLT_2022_2023_TURN_IN_BOOST_RIGHT) *
                          turn_in_weight * low_speed_factor)
  unwind_taper = 1.0 - (_bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_UNWIND_TAPER_LEFT, BOLT_2022_2023_UNWIND_TAPER_RIGHT) *
                         unwind_weight * (0.55 + 0.45 * low_speed_factor))
  return 1.0 + (extra_scale * turn_in_boost * max(unwind_taper, 0.0))


def get_bolt_2022_2023_friction_threshold(v_ego: float, desired_lateral_accel: float = 0.0, desired_lateral_jerk: float = 0.0) -> float:
  base_threshold = get_friction_threshold(v_ego)
  transition_envelope = _bolt_2022_2023_transition_envelope(v_ego, desired_lateral_accel, desired_lateral_jerk)
  phase = _bolt_2022_2023_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  unwind_weight = max(-phase, 0.0)
  threshold_scale = 1.0 - (_bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_TURN_IN_THRESHOLD_REDUCTION_LEFT, BOLT_2022_2023_TURN_IN_THRESHOLD_REDUCTION_RIGHT) *
                           transition_envelope * turn_in_weight)
  threshold_scale += (_bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_UNWIND_THRESHOLD_INCREASE_LEFT, BOLT_2022_2023_UNWIND_THRESHOLD_INCREASE_RIGHT) *
                      transition_envelope * unwind_weight)
  return base_threshold * min(max(threshold_scale, 0.84), 1.14)


def get_bolt_2022_2023_friction_scale(v_ego: float, desired_lateral_accel: float, desired_lateral_jerk: float) -> float:
  transition_envelope = _bolt_2022_2023_transition_envelope(v_ego, desired_lateral_accel, desired_lateral_jerk)
  phase = _bolt_2022_2023_transition_phase(desired_lateral_accel, desired_lateral_jerk)
  turn_in_weight = max(phase, 0.0)
  unwind_weight = max(-phase, 0.0)
  friction_scale = BOLT_2022_2023_FRICTION_MULT
  friction_scale += (_bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_TURN_IN_FRICTION_BOOST_LEFT, BOLT_2022_2023_TURN_IN_FRICTION_BOOST_RIGHT) *
                     transition_envelope * turn_in_weight)
  friction_scale -= (_bolt_2022_2023_side_value(desired_lateral_accel, BOLT_2022_2023_UNWIND_FRICTION_REDUCTION_LEFT, BOLT_2022_2023_UNWIND_FRICTION_REDUCTION_RIGHT) *
                     transition_envelope * unwind_weight)
  return min(max(friction_scale, 0.92), 1.22)


class LatControlTorque(LatControl):
  def __init__(self, CP, CI, dt):
    super().__init__(CP, CI, dt)
    self.torque_params = CP.lateralTuning.torque.as_builder()
    self.torque_from_lateral_accel = CI.torque_from_lateral_accel()
    self.lateral_accel_from_torque = CI.lateral_accel_from_torque()
    self.pid = PIDController([INTERP_SPEEDS, KP_INTERP], KI, rate=1/self.dt)
    self.update_limits()
    self.steering_angle_deadzone_deg = self.torque_params.steeringAngleDeadzoneDeg
    self.lat_accel_request_buffer_len = int(LAT_ACCEL_REQUEST_BUFFER_SECONDS / self.dt)
    self.lat_accel_request_buffer = deque([0.] * self.lat_accel_request_buffer_len, maxlen=self.lat_accel_request_buffer_len)
    self.lookahead_frames = int(JERK_LOOKAHEAD_SECONDS / self.dt)
    self.jerk_filter = FirstOrderFilter(0.0, 1 / (2 * np.pi * LP_FILTER_CUTOFF_HZ), self.dt)
    self.previous_measurement = 0.0
    self.measurement_rate_filter = FirstOrderFilter(0.0, 1 / (2 * np.pi * (MAX_LAT_JERK_UP - 0.5)), self.dt)
    self.low_speed_reset_threshold = max(CP.minSteerSpeed, MIN_LATERAL_CONTROL_SPEED)
    self.steer_release_i_decay = 0.8
    self.prev_steering_pressed = False
    self.debug_counter = 0
    self.prev_desired_lateral_accel = 0.0

    self.is_bolt = CP.carFingerprint in BOLT_CARS
    self.is_bolt_2022_2023 = CP.carFingerprint in BOLT_2022_2023_CARS
    self.is_bolt_2018_2021 = CP.carFingerprint in BOLT_2018_2021_CARS
    self.is_bolt_2017 = CP.carFingerprint in BOLT_2017_CARS
    self.use_bolt_ff_scaling = self.is_bolt_2022_2023 or self.is_bolt_2018_2021 or self.is_bolt_2017
    self.use_bolt_ki_multiplier = self.use_bolt_ff_scaling
    self.torque_ff_scale_pos = 1.0
    self.torque_ff_scale_neg = 1.0
    self.torque_deadzone_boost = float(getattr(self.torque_params, "kfDEPRECATED", 0.0))
    self.torque_ki_mult = 1.0
    if self.is_bolt:
      kp_scale = getattr(self.torque_params, "kp", getattr(self.torque_params, "kpDEPRECATED", 1.0))
      ki_scale = getattr(self.torque_params, "ki", getattr(self.torque_params, "kiDEPRECATED", 1.0))
      kd_scale = getattr(self.torque_params, "kd", getattr(self.torque_params, "kdDEPRECATED", 1.0))
      self.torque_ff_scale_pos = float(kp_scale)
      self.torque_ff_scale_neg = float(ki_scale)
      self.torque_ki_mult = float(kd_scale)
      if self.use_bolt_ki_multiplier and self.torque_ki_mult > 0.0 and self.torque_ki_mult != 1.0:
        self.pid._k_i = [self.pid._k_i[0], [k * self.torque_ki_mult for k in self.pid._k_i[1]]]

  def update_live_torque_params(self, latAccelFactor, latAccelOffset, friction):
    self.torque_params.latAccelFactor = latAccelFactor
    self.torque_params.latAccelOffset = latAccelOffset
    self.torque_params.friction = friction
    self.update_limits()

  def update_limits(self):
    self.pid.set_limits(self.lateral_accel_from_torque(self.steer_max, self.torque_params),
                        self.lateral_accel_from_torque(-self.steer_max, self.torque_params))

  def update(self, active, CS, VM, params, steer_limited_by_safety, desired_curvature, curvature_limited, lat_delay, calibrated_pose, model_data, starpilot_toggles):
    pid_log = log.ControlsState.LateralTorqueState.new_message()
    pid_log.version = VERSION
    if not active:
      output_torque = 0.0
      pid_log.active = False
      self.pid.reset()
      self.previous_measurement = 0.0
      self.measurement_rate_filter.x = 0.0
      self.lat_accel_request_buffer = deque([0.] * self.lat_accel_request_buffer_len, maxlen=self.lat_accel_request_buffer_len)
      self.prev_desired_lateral_accel = 0.0
    else:
      if self.prev_steering_pressed and not CS.steeringPressed:
        self.pid.i *= self.steer_release_i_decay

      measured_curvature = -VM.calc_curvature(math.radians(CS.steeringAngleDeg - params.angleOffsetDeg), CS.vEgo, params.roll)
      roll_compensation = params.roll * ACCELERATION_DUE_TO_GRAVITY
      curvature_deadzone = abs(VM.calc_curvature(math.radians(self.steering_angle_deadzone_deg), CS.vEgo, 0.0))
      lateral_accel_deadzone = curvature_deadzone * CS.vEgo ** 2

      delay_frames = int(np.clip(lat_delay / self.dt, 1, self.lat_accel_request_buffer_len))
      expected_lateral_accel = self.lat_accel_request_buffer[-delay_frames]
      future_desired_lateral_accel = desired_curvature * CS.vEgo ** 2
      self.lat_accel_request_buffer.append(future_desired_lateral_accel)
      raw_lateral_jerk = (future_desired_lateral_accel - expected_lateral_accel) / max(lat_delay, self.dt)
      raw_lateral_jerk = np.clip(raw_lateral_jerk, -MAX_LAT_JERK_UP, MAX_LAT_JERK_UP)
      desired_lateral_jerk = np.clip(self.jerk_filter.update(raw_lateral_jerk), -MAX_LAT_JERK_UP, MAX_LAT_JERK_UP)
      gravity_adjusted_future_lateral_accel = future_desired_lateral_accel - roll_compensation
      setpoint = expected_lateral_accel + desired_lateral_jerk * lat_delay
      desired_lateral_accel_rate = (setpoint - self.prev_desired_lateral_accel) / self.dt
      unwind_detected = (desired_lateral_accel_rate < UNWIND_D_DES_THRESHOLD and
                         abs(setpoint) < UNWIND_LAT_ACCEL_NEAR_ZERO)
      self.prev_desired_lateral_accel = setpoint

      measurement = measured_curvature * CS.vEgo ** 2
      measurement_rate = self.measurement_rate_filter.update((measurement - self.previous_measurement) / self.dt)
      measurement_rate = np.clip(measurement_rate, -MAX_LAT_JERK_UP, MAX_LAT_JERK_UP)
      self.previous_measurement = measurement

      low_speed_factor = (np.interp(CS.vEgo, LOW_SPEED_X, LOW_SPEED_Y) / max(CS.vEgo, MIN_SPEED)) ** 2
      current_kp = np.interp(CS.vEgo, self.pid._k_p[0], self.pid._k_p[1])
      error = setpoint - measurement
      error_with_lsf = error * (1 + low_speed_factor / max(current_kp, 1e-3))

      # do error correction in lateral acceleration space, convert at end to handle non-linear torque responses correctly
      pid_log.error = float(error_with_lsf)
      ff = gravity_adjusted_future_lateral_accel
      # latAccelOffset corrects roll compensation bias from device roll misalignment relative to car roll
      ff -= self.torque_params.latAccelOffset
      ff_scale = 1.0
      if self.use_bolt_ff_scaling:
        ff_scale = np.interp(ff, [-FF_SCALE_BLEND_LAT_ACCEL, 0.0, FF_SCALE_BLEND_LAT_ACCEL],
                             [self.torque_ff_scale_neg, 1.0, self.torque_ff_scale_pos])
        ff *= ff_scale
      bolt_2022_2023_test_active = self.is_bolt_2022_2023 and bolt_2022_2023_lateral_testing_ground_active()
      bolt_2018_2021_test_active = self.is_bolt_2018_2021 and bolt_2018_2021_lateral_testing_ground_active()
      friction_threshold = get_friction_threshold(CS.vEgo)
      friction_scale = 1.0
      if bolt_2022_2023_test_active:
        ff *= get_bolt_2022_2023_ff_scale(setpoint, desired_lateral_jerk, CS.vEgo)
        friction_threshold = get_bolt_2022_2023_friction_threshold(CS.vEgo, setpoint, desired_lateral_jerk)
        friction_scale = get_bolt_2022_2023_friction_scale(CS.vEgo, setpoint, desired_lateral_jerk)
      elif bolt_2018_2021_test_active:
        friction_threshold = get_bolt_2018_2021_friction_threshold(CS.vEgo, setpoint, desired_lateral_jerk)
        friction_scale = get_bolt_2018_2021_friction_scale(CS.vEgo, setpoint, desired_lateral_jerk)
      ff += friction_scale * get_friction(error_with_lsf + JERK_GAIN * desired_lateral_jerk, lateral_accel_deadzone, friction_threshold, self.torque_params)
      deadzone_boost_active = False
      if self.torque_deadzone_boost > 0.0 and abs(gravity_adjusted_future_lateral_accel) < DEADZONE_BOOST_LAT_ACCEL:
        boost_scale = np.interp(abs(gravity_adjusted_future_lateral_accel), [0.0, DEADZONE_BOOST_LAT_ACCEL], [1.0, 0.0])
        ff += np.sign(gravity_adjusted_future_lateral_accel) * self.torque_deadzone_boost * boost_scale
        deadzone_boost_active = True

      if CS.vEgo < self.low_speed_reset_threshold:
        self.pid.reset()
      freeze_integrator = (steer_limited_by_safety or CS.steeringPressed or
                           CS.vEgo < self.low_speed_reset_threshold or unwind_detected)
      output_lataccel = self.pid.update(pid_log.error, error_rate=-measurement_rate, speed=CS.vEgo, feedforward=ff, freeze_integrator=freeze_integrator)
      output_torque = self.torque_from_lateral_accel(output_lataccel, self.torque_params)
      if self.is_bolt_2017 and bolt_2017_lateral_testing_ground_active():
        output_torque *= get_bolt_2017_torque_scale(setpoint, desired_lateral_jerk, CS.vEgo)
      elif bolt_2018_2021_test_active:
        output_torque *= get_bolt_2018_2021_dynamic_torque_scale(setpoint, desired_lateral_jerk, CS.vEgo)

      pid_log.active = True
      pid_log.p = float(self.pid.p)
      pid_log.i = float(self.pid.i)
      pid_log.d = float(self.pid.d)
      pid_log.f = float(self.pid.f)
      pid_log.output = float(-output_torque)  # TODO: log lat accel?
      pid_log.actualLateralAccel = float(measurement)
      pid_log.desiredLateralAccel = float(setpoint)
      pid_log.desiredLateralJerk = float(desired_lateral_jerk)
      pid_log.saturated = bool(self._check_saturation(self.steer_max - abs(output_torque) < 1e-3, CS, steer_limited_by_safety, curvature_limited))

      if DEBUG_TORQUE_TUNE and self.is_bolt:
        self.debug_counter += 1
        if self.debug_counter % 50 == 0:
          print(f"bolt_torque ff_scale={ff_scale:.3f} pos={self.torque_ff_scale_pos:.3f} "
                f"neg={self.torque_ff_scale_neg:.3f} deadzone_boost_active={deadzone_boost_active}")

    self.prev_steering_pressed = CS.steeringPressed

    # TODO left is positive in this convention
    return -output_torque, 0.0, pid_log
