import numpy as np
from cereal import car
from openpilot.common.realtime import DT_CTRL
from openpilot.common.filter_simple import FirstOrderFilter
from openpilot.selfdrive.controls.lib.drive_helpers import CONTROL_N
from openpilot.common.pid import PIDController
from openpilot.selfdrive.modeld.constants import ModelConstants
from opendbc.car.gm.values import CarControllerParams, GMFlags

CONTROL_N_T_IDX = ModelConstants.T_IDXS[:CONTROL_N]

LongCtrlState = car.CarControl.Actuators.LongControlState


def long_control_state_trans(CP, active, long_control_state, v_ego,
                             should_stop, brake_pressed, cruise_standstill, frogpilot_toggles):
  # Ignore cruise standstill if car has a gas interceptor.
  cruise_standstill = cruise_standstill and not CP.enableGasInterceptorDEPRECATED
  stopping_condition = should_stop
  starting_condition = (not should_stop and
                        not cruise_standstill and
                        not brake_pressed)
  started_condition = v_ego > frogpilot_toggles.vEgoStarting

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

class LongControl:
  def __init__(self, CP):
    self.CP = CP
    self.long_control_state = LongCtrlState.off
    self.experimental_mode = False
    self.pid = PIDController((CP.longitudinalTuning.kpBP, CP.longitudinalTuning.kpV),
                             (CP.longitudinalTuning.kiBP, CP.longitudinalTuning.kiV),
                             rate=1 / DT_CTRL)
    # Preserve legacy behavior when no feedforward gain is provided (default of 0.0).
    kf = getattr(CP.longitudinalTuning, 'kfDEPRECATED', 0.0)
    self.feedforward_gain = kf if kf != 0.0 else 1.0
    self._mode_setup()
    self.last_output_accel = 0.0
    self.last_a_target = 0.0
    self.integrator_hold_frames = 0
    self.is_gm_pedal_long = bool(
      CP.brand == "gm" and CP.enableGasInterceptorDEPRECATED and (CP.flags & GMFlags.PEDAL_LONG.value)
    )

  def update_mpc_mode(self, experimental_mode: bool):
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

    handoff_threshold = np.interp(v_ego, [0.0, 4.0, 12.0, 25.0], [0.35, 0.45, 0.55, 0.70])
    if abs(a_target - self.last_a_target) > handoff_threshold:
      hold_frames = int(round(np.interp(v_ego, [0.0, 4.0, 12.0, 25.0], [25.0, 20.0, 14.0, 10.0])))
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

  def update(self, active, CS, a_target, should_stop, accel_limits, frogpilot_toggles):
    """Update longitudinal control. This updates the state machine and runs a PID loop"""
    self.pid.neg_limit = accel_limits[0]
    self.pid.pos_limit = accel_limits[1]

    self.long_control_state = long_control_state_trans(self.CP, active, self.long_control_state, CS.vEgo,
                                                       should_stop, CS.brakePressed,
                                                       CS.cruiseState.standstill, frogpilot_toggles)
    if self.long_control_state == LongCtrlState.off:
      self.reset()
      output_accel = 0.

    elif self.long_control_state == LongCtrlState.stopping:
      output_accel = self.last_output_accel
      if output_accel > frogpilot_toggles.stopAccel:
        output_accel = min(output_accel, 0.0)
        output_accel -= frogpilot_toggles.stoppingDecelRate * DT_CTRL
      self.reset()

    elif self.long_control_state == LongCtrlState.starting:
      output_accel = (a_target if frogpilot_toggles.human_acceleration else frogpilot_toggles.startAccel)
      self.reset()

    else:  # LongCtrlState.pid
      error = a_target - CS.aEgo
      self.update_mpc_mode(self.experimental_mode)
      freeze_integrator = self._get_pedal_long_freeze(a_target, error, CS.vEgo, accel_limits)
      raw_output_accel = self.pid.update(error, speed=CS.vEgo,
                                         feedforward=a_target * self.feedforward_gain,
                                         freeze_integrator=freeze_integrator)

      if self.transitioning and self.prev_mode == 'acc' and self.current_mode == 'blended':
        if raw_output_accel < 0 and raw_output_accel < self.last_output_accel:
          progress = min(1.0, self.mode_transition_timer / self.mode_transition_duration)
          # Soften transition at low urgency, but keep sharp for high decel.
          urgency_denom = max(1e-3, abs(CarControllerParams.ACCEL_MIN))
          urgency = abs(raw_output_accel / urgency_denom)
          urgency_smooth = min(1.0, urgency ** 0.4)
          blend_factor = 1.0 - (1.0 - progress) * (1.0 - urgency_smooth)
          output_accel = self.last_output_accel + (raw_output_accel - self.last_output_accel) * blend_factor
        else:
          output_accel = raw_output_accel
      else:
        output_accel = raw_output_accel

    self.last_output_accel = np.clip(output_accel, accel_limits[0], accel_limits[1])
    return self.last_output_accel
