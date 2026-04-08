from cereal import car
import numpy as np
from openpilot.common.realtime import DT_CTRL
from openpilot.selfdrive.controls.lib.drive_helpers import CONTROL_N
from openpilot.common.pid import PIDController
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.common.filter_simple import FirstOrderFilter
from opendbc.car.gm.values import CarControllerParams, GMFlags
from openpilot.starpilot.common.testing_grounds import testing_ground

CONTROL_N_T_IDX = ModelConstants.T_IDXS[:CONTROL_N]
clip = np.clip
interp = np.interp

LongCtrlState = car.CarControl.Actuators.LongControlState

def apply_deadzone(error, deadzone):
  if error > deadzone:
    error -= deadzone
  elif error < -deadzone:
    error += deadzone
  else:
    error = 0.0
  return error


def long_control_state_trans(CP, active, long_control_state, v_ego,
                             should_stop, brake_pressed, cruise_standstill, starpilot_toggles):
  # Ignore cruise standstill if car has a gas interceptor
  cruise_standstill = cruise_standstill and not CP.enableGasInterceptorDEPRECATED
  stopping_condition = should_stop
  starting_condition = (not should_stop and
                        not cruise_standstill and
                        not brake_pressed)
  started_condition = v_ego > starpilot_toggles.vEgoStarting

  if not active:
    long_control_state = LongCtrlState.off

  else:
    if long_control_state == LongCtrlState.off:
      if not starting_condition:
        long_control_state = LongCtrlState.stopping
      else:
        if starting_condition and CP.startingState:
          long_control_state = LongCtrlState.starting
        else:
          long_control_state = LongCtrlState.pid

    elif long_control_state == LongCtrlState.stopping:
      if starting_condition and CP.startingState:
        long_control_state = LongCtrlState.starting
      elif starting_condition:
        long_control_state = LongCtrlState.pid

    elif long_control_state in [LongCtrlState.starting, LongCtrlState.pid]:
      if stopping_condition:
        long_control_state = LongCtrlState.stopping
      elif started_condition:
        long_control_state = LongCtrlState.pid
  return long_control_state

def long_control_state_trans_old_long(CP, active, long_control_state, v_ego, v_target,
                                      v_target_1sec, brake_pressed, cruise_standstill, starpilot_toggles):
  accelerating = v_target_1sec > v_target
  planned_stop = (v_target < starpilot_toggles.vEgoStopping and
                  v_target_1sec < starpilot_toggles.vEgoStopping and
                  not accelerating)
  stay_stopped = (v_ego < starpilot_toggles.vEgoStopping and
                  (brake_pressed or cruise_standstill))
  stopping_condition = planned_stop or stay_stopped

  starting_condition = (v_target_1sec > starpilot_toggles.vEgoStarting and
                        accelerating and
                        not cruise_standstill and
                        not brake_pressed)
  started_condition = v_ego > starpilot_toggles.vEgoStarting

  if not active:
    long_control_state = LongCtrlState.off

  else:
    if long_control_state in (LongCtrlState.off, LongCtrlState.pid):
      long_control_state = LongCtrlState.pid
      if stopping_condition:
        long_control_state = LongCtrlState.stopping

    elif long_control_state == LongCtrlState.stopping:
      if starting_condition and CP.startingState:
        long_control_state = LongCtrlState.starting
      elif starting_condition:
        long_control_state = LongCtrlState.pid

    elif long_control_state == LongCtrlState.starting:
      if stopping_condition:
        long_control_state = LongCtrlState.stopping
      elif started_condition:
        long_control_state = LongCtrlState.pid

  return long_control_state

class LongControl:
  def __init__(self, CP):
    self.CP = CP
    self.long_control_state = LongCtrlState.off
    self.experimental_mode = False
    self.pid = PIDController((CP.longitudinalTuning.kpBP, CP.longitudinalTuning.kpV),
                             (CP.longitudinalTuning.kiBP, CP.longitudinalTuning.kiV),
                             rate=1 / DT_CTRL)
    # Preserve legacy behaviour when no feedforward gain is provided (default of 0.0)
    kf = getattr(CP.longitudinalTuning, 'kfDEPRECATED', 0.0)
    self.feedforward_gain = kf if kf != 0.0 else 1.0
    self.v_pid = 0.0
    self._mode_setup()
    self.last_output_accel = 0.0
    self.last_a_target = 0.0
    self.integrator_hold_frames = 0
    self.is_gm_pedal_long = bool(
      CP.brand == "gm" and CP.enableGasInterceptorDEPRECATED and (CP.flags & GMFlags.PEDAL_LONG.value)
    )
    self.is_volt_interceptor = bool(
      CP.brand == "gm" and CP.enableGasInterceptorDEPRECATED and str(CP.carFingerprint).startswith("CHEVROLET_VOLT")
    )

  def update_mpc_mode(self, experimental_mode):
    new_mode = 'blended' if experimental_mode else 'acc'

    if self.transitioning and self.prev_mode == 'blended' and self.current_mode == 'acc':
      self.mode_transition_timer = 0.0

    if new_mode != self.current_mode:
      self.prev_mode = self.current_mode
      self.transitioning = True
      self.mode_transition_timer = 0.0
      self.mode_transition_filter.x = self.last_output_accel

      self.current_mode = new_mode

    if self.transitioning:
      self.mode_transition_timer += DT_CTRL
      if self.mode_transition_timer >= self.mode_transition_duration:
        self.transitioning = False

  def _mode_setup(self):
    self.prev_mode = 'acc'
    self.current_mode = 'acc'
    self.mode_transition_filter = FirstOrderFilter(0.0, 0.5, DT_CTRL)
    self.mode_transition_timer = 0.0
    self.mode_transition_duration = 1.0
    self.transitioning = False

  def reset(self):
    self.pid.reset()
    self.last_a_target = 0.0
    self.integrator_hold_frames = 0

  def _get_pedal_long_freeze(self, a_target, error, v_ego, accel_limits):
    if not self.is_gm_pedal_long:
      self.last_a_target = a_target
      self.integrator_hold_frames = 0
      return False

    handoff_threshold = interp(v_ego, [0.0, 4.0, 12.0, 25.0], [0.35, 0.45, 0.55, 0.70])
    if abs(a_target - self.last_a_target) > handoff_threshold:
      hold_frames = int(round(interp(v_ego, [0.0, 4.0, 12.0, 25.0], [25.0, 20.0, 14.0, 10.0])))
      self.integrator_hold_frames = max(self.integrator_hold_frames, hold_frames)
    self.last_a_target = a_target

    if self.integrator_hold_frames > 0:
      self.integrator_hold_frames -= 1

    sat_buffer = 0.03
    at_neg_sat = self.last_output_accel <= (accel_limits[0] + sat_buffer)
    at_pos_sat = self.last_output_accel >= (accel_limits[1] - sat_buffer)
    sat_pushing_lower = at_neg_sat and error < -0.05
    sat_pushing_upper = at_pos_sat and error > 0.05

    return self.integrator_hold_frames > 0 or sat_pushing_lower or sat_pushing_upper

  def _shape_volt_test_tune_integrator(self, error, v_ego):
    if not (self.is_volt_interceptor and testing_ground.use_2):
      return

    # Bleed stale I quickly when the target reverses against stored integrator.
    if self.pid.i * error < 0.0 and abs(error) > 0.05:
      bleed = interp(v_ego, [0.0, 4.0, 12.0, 25.0], [0.86, 0.90, 0.94, 0.97])
      self.pid.i *= bleed

  def update(self, active, CS, a_target, should_stop, accel_limits, starpilot_toggles):
    """Update longitudinal control. This updates the state machine and runs a PID loop"""
    self.pid.neg_limit = accel_limits[0]
    self.pid.pos_limit = accel_limits[1]

    self.long_control_state = long_control_state_trans(self.CP, active, self.long_control_state, CS.vEgo,
                                                       should_stop, CS.brakePressed,
                                                       CS.cruiseState.standstill, starpilot_toggles)
    if self.long_control_state == LongCtrlState.off:
      self.reset()
      output_accel = 0.

    elif self.long_control_state == LongCtrlState.stopping:
      output_accel = self.last_output_accel
      if output_accel > starpilot_toggles.stopAccel:
        output_accel = min(output_accel, 0.0)
        output_accel -= starpilot_toggles.stoppingDecelRate * DT_CTRL
      self.reset()

    elif self.long_control_state == LongCtrlState.starting:
      output_accel = (a_target if starpilot_toggles.human_acceleration else starpilot_toggles.startAccel)
      self.reset()

    else:  # LongCtrlState.pid
      error = a_target - CS.aEgo
      self.update_mpc_mode(self.experimental_mode)
      self._shape_volt_test_tune_integrator(error, CS.vEgo)
      feedforward = a_target * self.feedforward_gain
      freeze_integrator = self._get_pedal_long_freeze(a_target, error, CS.vEgo, accel_limits)
      raw_output_accel = self.pid.update(error, speed=CS.vEgo, feedforward=feedforward,
                                         freeze_integrator=freeze_integrator)


      if self.transitioning and self.prev_mode == 'acc' and self.current_mode == 'blended':
        if raw_output_accel < 0 and raw_output_accel < self.last_output_accel:
          progress = min(1.0, self.mode_transition_timer / self.mode_transition_duration)
          # Soften transition at low urgency, but keep sharp for high decel
          # 20% smoother for chill decel (lower exponent)
          urgency = abs(raw_output_accel / CarControllerParams.ACCEL_MIN)
          urgency_smooth = min(1.0, urgency ** 0.4)  # 20% smoother for chill decel
          blend_factor = 1.0 - (1.0 - progress) * (1.0 - urgency_smooth)
          output_accel = self.last_output_accel + (raw_output_accel - self.last_output_accel) * blend_factor
        else:
          output_accel = raw_output_accel
      else:
        output_accel = raw_output_accel

    self.last_output_accel = clip(output_accel, accel_limits[0], accel_limits[1])
    return self.last_output_accel

  def reset_old_long(self, v_pid):
    """Reset PID controller and change setpoint"""
    self.pid.reset()
    self.v_pid = v_pid
    self.last_a_target = 0.0
    self.integrator_hold_frames = 0

  def update_old_long(self, active, CS, long_plan, accel_limits, t_since_plan, starpilot_toggles):
    """Update longitudinal control. This updates the state machine and runs a PID loop"""
    # Interp control trajectory
    speeds = long_plan.speeds
    if len(speeds) == CONTROL_N:
      v_target_now = interp(t_since_plan, CONTROL_N_T_IDX, speeds)
      a_target_now = interp(t_since_plan, CONTROL_N_T_IDX, long_plan.accels)

      v_target = interp(starpilot_toggles.longitudinalActuatorDelay + t_since_plan, CONTROL_N_T_IDX, speeds)
      a_target = 2 * (v_target - v_target_now) / starpilot_toggles.longitudinalActuatorDelay - a_target_now

      v_target_1sec = interp(starpilot_toggles.longitudinalActuatorDelay + t_since_plan + 1.0, CONTROL_N_T_IDX, speeds)
    else:
      v_target = 0.0
      v_target_now = 0.0
      v_target_1sec = 0.0
      a_target = 0.0

    self.pid.neg_limit = accel_limits[0]
    self.pid.pos_limit = accel_limits[1]

    output_accel = self.last_output_accel
    self.long_control_state = long_control_state_trans_old_long(self.CP, active, self.long_control_state, CS.vEgo,
                                                                v_target, v_target_1sec, CS.brakePressed,
                                                                CS.cruiseState.standstill, starpilot_toggles)

    if self.long_control_state == LongCtrlState.off:
      self.reset_old_long(CS.vEgo)
      output_accel = 0.

    elif self.long_control_state == LongCtrlState.stopping:
      if output_accel > starpilot_toggles.stopAccel:
        output_accel = min(output_accel, 0.0)
        output_accel -= starpilot_toggles.stoppingDecelRate * DT_CTRL
      self.reset_old_long(CS.vEgo)

    elif self.long_control_state == LongCtrlState.starting:
      output_accel = starpilot_toggles.startAccel
      self.reset_old_long(CS.vEgo)

    elif self.long_control_state == LongCtrlState.pid:
      self.v_pid = v_target_now

      # Toyota starts braking more when it thinks you want to stop
      # Freeze the integrator so we don't accelerate to compensate, and don't allow positive acceleration
      # TODO too complex, needs to be simplified and tested on toyotas
      prevent_overshoot = not self.CP.stoppingControl and CS.vEgo < 1.5 and v_target_1sec < 0.7 and v_target_1sec < self.v_pid
      deadzone = interp(CS.vEgo, self.CP.longitudinalTuning.deadzoneBP, self.CP.longitudinalTuning.deadzoneV)
      error = self.v_pid - CS.vEgo
      error_deadzone = apply_deadzone(error, deadzone)
      freeze_integrator = prevent_overshoot or self._get_pedal_long_freeze(a_target, error_deadzone, CS.vEgo, accel_limits)
      feedforward = a_target * self.feedforward_gain
      output_accel = self.pid.update(error_deadzone, speed=CS.vEgo,
                                     feedforward=feedforward,
                                     freeze_integrator=freeze_integrator)

    self.last_output_accel = clip(output_accel, accel_limits[0], accel_limits[1])

    return self.last_output_accel
