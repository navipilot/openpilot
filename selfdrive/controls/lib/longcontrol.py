import numpy as np
from cereal import car
from openpilot.common.realtime import DT_CTRL
from openpilot.selfdrive.controls.lib.drive_helpers import CONTROL_N
from openpilot.common.pid import PIDController
from openpilot.selfdrive.modeld.constants import ModelConstants
from openpilot.common.params import Params

CONTROL_N_T_IDX = ModelConstants.T_IDXS[:CONTROL_N]

LongCtrlState = car.CarControl.Actuators.LongControlState


def long_control_state_trans(CP, active, long_control_state, v_ego,
                             should_stop, brake_pressed, cruise_standstill, a_ego, stopping_accel, radarState):
  stopping_condition = should_stop
  starting_condition = (not should_stop and
                        not cruise_standstill and
                        not brake_pressed)
  started_condition = v_ego > CP.vEgoStarting

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
        stopping_accel = stopping_accel if stopping_accel < 0.0 else -0.5
        leadOne = radarState.leadOne
        fcw_stop = leadOne.status and leadOne.dRel < 4.0
        if a_ego > stopping_accel or fcw_stop: # and v_ego < 1.0:
          long_control_state = LongCtrlState.stopping
        if long_control_state == LongCtrlState.starting:
          long_control_state = LongCtrlState.stopping
      elif started_condition:
        long_control_state = LongCtrlState.pid
  return long_control_state

class LongControl:
  def __init__(self, CP):
    self.CP = CP
    self.long_control_state = LongCtrlState.off
    self.pid = PIDController((CP.longitudinalTuning.kpBP, CP.longitudinalTuning.kpV),
                             (CP.longitudinalTuning.kiBP, CP.longitudinalTuning.kiV),
                             k_f=CP.longitudinalTuning.kf, rate=1 / DT_CTRL)
    self.last_output_accel = 0.0


    self.params = Params()
    self.readParamCount = 0
    self.stopping_accel = 0
    self.j_lead = 0.0
    #new
    self.stopping_decel_rate = 0
    self.start_accel = 0
    self.decel_limit_v_ego_max = 0
    self.decel_limit_v_ego_min = 0
    self.decel_limit_a_ego_max = 0
    self.decel_limit_a_ego_min = 0
    self.a_ego_curr = 0
    self.a_ego_curr_init = False
    self.gas_release_smooth_max_cnt = int(5.0 / DT_CTRL)
    self.gas_release_smooth_max_last = self.gas_release_smooth_max_cnt
    self.gas_release_smooth_cnt = 0
    self.output_accel_filtered = 0.0
    self.output_accel_init = False
    self.smooth_stop_mode = 0

  def reset(self):
    self.pid.reset()
    self.a_ego_curr_init = False
    self.output_accel_init = False

  def update(self, active, CS, long_plan, accel_limits, t_since_plan, radarState):

    soft_hold_active = CS.softHoldActive > 0
    a_target_ff = long_plan.aTarget
    v_target_now = long_plan.vTargetNow
    j_target_now = long_plan.jTargetNow
    should_stop = long_plan.shouldStop

    self.readParamCount += 1
    if self.readParamCount >= 100:
      self.readParamCount = 0
      self.stopping_accel = self.params.get_float("StoppingAccel") * 0.01
      try:
        self.stopping_decel_rate = self.params.get_float("StoppingDecelRate") * 0.01
        self.start_accel = self.params.get_float("StartAccel") * 0.01
        self.decel_limit_v_ego_max = max(0.0, self.params.get_float("DecelLimitVEgoMax") * 0.1)
        self.decel_limit_v_ego_min = max(0.0, self.params.get_float("DecelLimitVEgoMin") * 0.1)
        self.decel_limit_a_ego_max = min(0.0, self.params.get_float("DecelLimitAEgoMax") * 0.01)
        self.decel_limit_a_ego_min = min(0.0, self.params.get_float("DecelLimitAEgoMin") * 0.01)
        self.gas_release_smooth_max_cnt = int(self.params.get_float("GasSmoothTime") / DT_CTRL / 10)
        self.smooth_stop_mode = self.params.get_int("SmoothStopMode")
      except Exception as e:
        self.gas_release_smooth_max_cnt = 5.0 / DT_CTRL
        self.smooth_stop_mode = 0
    elif self.readParamCount == 10:
      if len(self.CP.longitudinalTuning.kpBP) == 1 and len(self.CP.longitudinalTuning.kiBP)==1:
        longitudinalTuningKpV = self.params.get_float("LongTuningKpV") * 0.01
        longitudinalTuningKiV = self.params.get_float("LongTuningKiV") * 0.001
        self.pid._k_p = (self.CP.longitudinalTuning.kpBP, [longitudinalTuningKpV])
        self.pid._k_i = (self.CP.longitudinalTuning.kiBP, [longitudinalTuningKiV])
        self.pid._k_f = self.params.get_float("LongTuningKf") * 0.01


    """Update longitudinal control. This updates the state machine and runs a PID loop"""
    self.pid.neg_limit = accel_limits[0]
    self.pid.pos_limit = accel_limits[1]

    self.long_control_state = long_control_state_trans(self.CP, active, self.long_control_state, CS.vEgo,
                                                       should_stop, CS.brakePressed,
                                                       CS.cruiseState.standstill, CS.aEgo, self.stopping_accel, radarState)
    if active and soft_hold_active:
      self.long_control_state = LongCtrlState.stopping

    if self.long_control_state == LongCtrlState.off:
      self.reset()
      output_accel = 0.

    elif self.long_control_state == LongCtrlState.stopping:
      output_accel = self.last_output_accel

      if soft_hold_active:
        output_accel = self.CP.stopAccel

      stopAccel = self.stopping_accel if self.stopping_accel < 0.0 else self.CP.stopAccel
      if output_accel > stopAccel:
        output_accel = min(output_accel, 0.0)
        output_accel -= self.CP.stoppingDecelRate * DT_CTRL if self.stopping_decel_rate <= 0.0 else self.stopping_decel_rate * DT_CTRL #new
      self.reset()

    elif self.long_control_state == LongCtrlState.starting:
      output_accel = self.CP.startAccel if self.start_accel <= 0.0 else self.start_accel #new
      self.reset()

    else:  # LongCtrlState.pid
      #error = a_target_now - CS.aEgo
      error = v_target_now - CS.vEgo
      output_accel = self.pid.update(error, speed=CS.vEgo,
                                     feedforward=a_target_ff)

      # new 为了停车柔和，限制低速时的减速度
      if self.smooth_stop_mode == 2: #模式2
        alpha = 0.3  # 平滑系数
        if not self.output_accel_init:
          self.output_accel_filtered = output_accel
          self.output_accel_init = True
        else:
          self.output_accel_filtered = alpha * output_accel + (1 - alpha) * self.output_accel_filtered

        leadOne = radarState.leadOne
        smooth_stop = leadOne.status and leadOne.dRel < 15.0
        if self.decel_limit_v_ego_max > 0 and smooth_stop:
          if CS.vEgo < self.decel_limit_v_ego_max or (self.a_ego_curr_init and (CS.vEgo < (self.decel_limit_v_ego_max + 0.4))):
            if not self.a_ego_curr_init:
              self.a_ego_curr = min(self.output_accel_filtered, self.decel_limit_a_ego_max)
              self.a_ego_curr_init = True
            decel_limit_v_ego_min = min(self.decel_limit_v_ego_min, self.decel_limit_v_ego_max)
            min_accel = np.interp(CS.vEgo,
                                  [0.0, decel_limit_v_ego_min, self.decel_limit_v_ego_max],
                                  [self.decel_limit_a_ego_min, self.decel_limit_a_ego_min, self.a_ego_curr])
            output_accel = max(output_accel, min_accel)
          else:
            self.a_ego_curr_init = False
        else:
          self.a_ego_curr_init = False
      elif self.smooth_stop_mode == 1: #模式1
        leadOne = radarState.leadOne
        smooth_stop = leadOne.status and leadOne.dRel < 15.0
        if self.decel_limit_v_ego_max > 0 and smooth_stop:
          if CS.vEgo < self.decel_limit_v_ego_max:
            if not self.a_ego_curr_init:
              self.a_ego_curr = min(CS.aEgo, self.decel_limit_a_ego_max)
              self.a_ego_curr_init = True
            decel_limit_v_ego_min = min(self.decel_limit_v_ego_min, self.decel_limit_v_ego_max)
            min_accel = np.interp(CS.vEgo,
                                  [0.0, decel_limit_v_ego_min, self.decel_limit_v_ego_max],
                                  [self.decel_limit_a_ego_min, self.decel_limit_a_ego_min, self.a_ego_curr])
            output_accel = max(output_accel, min_accel)
          elif CS.vEgo >= (self.decel_limit_v_ego_max + 1.):
            self.a_ego_curr_init = False
        else:
          self.a_ego_curr_init = False
      else: #关闭平滑停车功能
        self.a_ego_curr_init = False

      #限制加速度
      if self.gas_release_smooth_max_cnt > 0:
        if self.gas_release_smooth_cnt > 0:
          self.gas_release_smooth_cnt -= 1
          max_accel = np.interp(self.gas_release_smooth_cnt,
                                [0, self.gas_release_smooth_max_last],
                                [accel_limits[1], 0.2])
          output_accel = min(output_accel, max_accel)

    # 踩下油门的时候重置计数
    if self.gas_release_smooth_max_cnt > 0 and CS.gasPressed:
      self.gas_release_smooth_cnt = self.gas_release_smooth_max_cnt
      self.gas_release_smooth_max_last = self.gas_release_smooth_max_cnt
    # new

    self.last_output_accel = np.clip(output_accel, accel_limits[0], accel_limits[1])
    return self.last_output_accel, a_target_ff, j_target_now
