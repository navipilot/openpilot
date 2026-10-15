from __future__ import annotations

from dataclasses import dataclass
from enum import IntEnum

import numpy as np

from openpilot.sunnypilot.selfdrive.traffic_control.tesla_observer import (
  TRAFFIC_CONTROL_MAX_DISTANCE,
  TRAFFIC_CONTROL_STALE_NS,
  TeslaTrafficControlObservation,
)

STOP_EVIDENCE_LOSS_GRACE_S = 2.0


class TrafficControlMode(IntEnum):
  off = 0
  observe = 1  # Legacy diagnostic mode; the production switch no longer selects it.
  shadow = 2
  stopOnly = 3
  stopGo = 4


class TrafficControlPhase(IntEnum):
  off = 0
  redCandidate = 1
  approachRed = 2
  braking = 3
  hold = 4
  goCandidate = 5
  release = 6
  bypass = 7
  greenFlashCandidate = 8
  flashingGreenStop = 9
  yellowStop = 10
  yellowPass = 11
  lateRed = 12
  passed = 13


@dataclass
class TrafficControlConfig:
  mode: TrafficControlMode = TrafficControlMode.off
  default_stop_reference: float = 5.0
  comfort_brake: float = 2.4
  release_s: float = 3.0
  stationary_release_s: float = 10.0
  driver_override_cooldown_s: float = 0.75
  critical_observation_dropout_s: float = STOP_EVIDENCE_LOSS_GRACE_S
  candidate_dropout_s: float = 2.5
  max_control_distance: float = TRAFFIC_CONTROL_MAX_DISTANCE
  candidate_distance_tolerance: float = 6.0
  candidate_distance_tolerance_ratio: float = 0.08
  max_control_speed: float = float("inf")
  yellow_stop_decel: float = 2.2
  yellow_pass_decel: float = 3.0
  flash_interval_min_s: float = 0.5
  flash_interval_max_s: float = 1.5
  flash_min_half_cycle_s: float = 0.15
  far_candidate_confirm_s: float = 0.5
  farther_replacement_confirm_s: float = 1.0


@dataclass(frozen=True)
class TrafficControlDecision:
  mode: TrafficControlMode
  phase: TrafficControlPhase
  active: bool
  apply_constraint: bool
  shadow: bool
  should_stop: bool
  remaining_distance: float
  stop_reference: float
  light_state: int
  source_bus: int
  quality: int
  stop_session_id: int = 0
  driver_override_active: bool = False
  direction_unknown: bool = False
  can_remaining: float = 0.0
  station_innovation: float = 0.0
  stop_control_allowed: bool = False
  stop_safety_allowed: bool = False
  raw_observation_fresh: bool = False
  stop_direction_unknown: bool = False


class TeslaTrafficControlController:
  """CAN-color traffic-light state machine with a CP-style latched stop point."""

  ACTIVE_PHASES = (
    TrafficControlPhase.approachRed,
    TrafficControlPhase.braking,
    TrafficControlPhase.hold,
    TrafficControlPhase.flashingGreenStop,
    TrafficControlPhase.yellowStop,
  )

  def __init__(self, config: TrafficControlConfig | None = None) -> None:
    self.config = config or TrafficControlConfig()
    self.transition_seq = 0
    self.transition_reason = ""
    self.event_seq = 0
    self.event_id = 0
    self.stop_session_seq = 0
    self.stop_session_id = 0
    self.phase = TrafficControlPhase.off
    self.last_update_ns: int | None = None
    self.last_real_frame_ns = 0
    self.last_real_color = 0
    self.last_raw_distance: float | None = None
    self.last_distance_ego_station = 0.0
    self.ego_station = 0.0
    self.stop_station: float | None = None
    self.remaining_distance = 0.0
    self.stop_reference = self.config.default_stop_reference
    self.light_state = 0
    self.source_bus = 0
    self.quality = 0
    self.event_continuous = False
    self.candidate_color = 0
    self.candidate_count = 0
    self.candidate_first_ns = 0
    self.candidate_last_ns = 0
    self.candidate_distance = 0.0
    self.candidate_ego_station = 0.0
    self.pending_distance = 0.0
    self.pending_ego_station = 0.0
    self.pending_color = 0
    self.pending_count = 0
    self.pending_first_ns = 0
    self.pending_replacement_farther = False
    self.green_count = 0
    self._clear_flash_candidate()
    self.flash_latched = False
    self.stable_green_since_ns = 0
    self.yellow_latched: bool | None = None
    self.release_since_ns: int | None = None
    self.release_red_preserve_session: bool | None = None
    self.driver_override_until_ns = 0
    self.driver_override_active = False
    self.override_reconfirm_required = False
    self.override_reconfirm_count = 0
    self.direction_unknown = False
    self.stop_direction_unknown = False
    self.stop_reconfirm_required = False
    self.stop_reconfirm_count = 0
    self.raw_observation_fresh = False
    self.raw_stale_since_ns: int | None = None
    self.stop_evidence_lost_since_ns = 0
    self.can_remaining = 0.0
    self.last_distance_innovation = 0.0

  def _mark_transition(self, reason: str) -> None:
    self.transition_seq += 1
    self.transition_reason = reason

  def _clear_pending_replacement(self) -> None:
    self.pending_count = 0
    self.pending_first_ns = 0
    self.pending_replacement_farther = False

  def _clear_flash_candidate(self) -> None:
    self.first_off_ns = 0
    self.flash_color = -1
    self.flash_color_since_ns = 0
    self.flash_last_frame_ns = 0
    self.flash_off_confirmed = False
    self.flash_anchor_station: float | None = None

  def set_config(self, config: TrafficControlConfig) -> None:
    previous_mode = self.config.mode
    self.config = config
    if config.mode == TrafficControlMode.off and previous_mode != TrafficControlMode.off:
      self.reset()
    elif self.phase == TrafficControlPhase.off:
      self.stop_reference = config.default_stop_reference

  def reset(self) -> None:
    self.phase = TrafficControlPhase.off
    self.event_id = 0
    self.stop_session_id = 0
    self.last_real_frame_ns = 0
    self.last_real_color = 0
    self.last_raw_distance = None
    self.last_distance_ego_station = self.ego_station
    self.stop_station = None
    self.remaining_distance = 0.0
    self.stop_reference = self.config.default_stop_reference
    self.light_state = 0
    self.source_bus = 0
    self.quality = 0
    self.event_continuous = False
    self.candidate_color = 0
    self.candidate_count = 0
    self.candidate_first_ns = 0
    self.candidate_last_ns = 0
    self.candidate_distance = 0.0
    self.candidate_ego_station = self.ego_station
    self.pending_distance = 0.0
    self.pending_ego_station = self.ego_station
    self.pending_color = 0
    self.pending_count = 0
    self.pending_first_ns = 0
    self.pending_replacement_farther = False
    self.green_count = 0
    self._clear_flash_candidate()
    self.flash_latched = False
    self.stable_green_since_ns = 0
    self.yellow_latched = None
    self.release_since_ns = None
    self.release_red_preserve_session = None
    self.last_distance_innovation = 0.0
    self.driver_override_until_ns = 0
    self.driver_override_active = False
    self.override_reconfirm_required = False
    self.override_reconfirm_count = 0
    self.direction_unknown = False
    self.stop_direction_unknown = False
    self.stop_reconfirm_required = False
    self.stop_reconfirm_count = 0
    self.raw_observation_fresh = False
    self.raw_stale_since_ns = None
    self.stop_evidence_lost_since_ns = 0
    self.can_remaining = 0.0
    self.last_update_ns = None

  def _decision(self) -> TrafficControlDecision:
    active = self.phase in self.ACTIVE_PHASES or self.phase == TrafficControlPhase.release
    apply_constraint = (self.config.mode in (TrafficControlMode.stopOnly, TrafficControlMode.stopGo) and active
                        and not self.driver_override_active and not self.override_reconfirm_required)
    stop_safety_allowed = bool(apply_constraint and not self.stop_reconfirm_required)
    stop_fresh_enough = self.raw_observation_fresh or self.phase == TrafficControlPhase.hold
    return TrafficControlDecision(
      mode=self.config.mode,
      phase=self.phase,
      active=active,
      apply_constraint=apply_constraint,
      shadow=self.config.mode == TrafficControlMode.shadow and active,
      should_stop=self.phase == TrafficControlPhase.hold,
      remaining_distance=max(0.0, self.remaining_distance),
      stop_reference=self.stop_reference,
      light_state=self.light_state,
      source_bus=self.source_bus,
      quality=self.quality,
      stop_session_id=self.stop_session_id,
      driver_override_active=self.driver_override_active,
      direction_unknown=self.direction_unknown,
      can_remaining=self.can_remaining,
      station_innovation=self.last_distance_innovation,
      stop_control_allowed=bool(stop_safety_allowed and stop_fresh_enough),
      stop_safety_allowed=stop_safety_allowed,
      raw_observation_fresh=self.raw_observation_fresh,
      stop_direction_unknown=self.stop_direction_unknown,
    )

  def _distance_tolerance(self, a: float, b: float) -> float:
    return max(self.config.candidate_distance_tolerance,
               self.config.candidate_distance_tolerance_ratio * max(a, b))

  def _same_track(self, observation: TeslaTrafficControlObservation) -> bool:
    if self.last_raw_distance is None:
      return True
    expected = max(0.0, self.last_raw_distance - (self.ego_station - self.last_distance_ego_station))
    innovation = observation.distance - expected
    self.last_distance_innovation = innovation
    return bool(observation.source_bus == self.source_bus and
                abs(innovation) <= self._distance_tolerance(observation.distance, expected))

  def _update_stop_station(self, observation: TeslaTrafficControlObservation) -> None:
    self.can_remaining = max(0.0, observation.distance - self.stop_reference)
    sample = self.ego_station + self.can_remaining
    if self.stop_station is None:
      self.stop_station = sample
      self.remaining_distance = self.can_remaining
    else:
      predicted = max(0.0, self.stop_station - self.ego_station)
      innovation = self.can_remaining - predicted
      self.last_distance_innovation = innovation
      gain = float(np.interp(self.can_remaining, [0.0, 20.0, 50.0, 200.0], [0.90, 0.80, 0.55, 0.30]))
      ego_travel = max(0.0, self.ego_station - self.last_distance_ego_station)
      # Near the line, the roughly 2 Hz CAN cadence leaves only a handful of
      # samples to converge. Preserve the broader replacement slew limit, but
      # allow a trusted final-approach sample to correct an early stop station.
      minimum_correction = 2.0 if self.can_remaining <= 20.0 else 1.0
      max_correction = max(minimum_correction, 0.8 * ego_travel + 0.5)
      correction = float(np.clip(gain * innovation, -max_correction, max_correction))
      self.remaining_distance = max(0.0, predicted + correction)
      self.stop_station = self.ego_station + self.remaining_distance
    self.remaining_distance = max(0.0, (self.stop_station or self.ego_station) - self.ego_station)

  def _hold_distance_consistent(self, *, evidence_lost: bool = False) -> bool:
    # During an evidence gap can_remaining is deliberately frozen. The owned
    # absolute stop station still advances with ego motion and is the only
    # meaningful terminal geometry once the vehicle reaches it.
    return bool(
      self.remaining_distance <= 1.5
      and (evidence_lost or self.can_remaining <= 2.0)
    )

  def _start_stop(self, observation: TeslaTrafficControlObservation, v_ego: float,
                  phase: TrafficControlPhase, reason: str, *, preserve_session: bool = False) -> None:
    self.event_seq += 1
    self.event_id = self.event_seq
    new_session = not preserve_session or self.stop_session_id == 0
    if new_session:
      self.stop_session_seq += 1
      self.stop_session_id = self.stop_session_seq
      self.yellow_latched = phase == TrafficControlPhase.yellowStop
      self.flash_latched = phase == TrafficControlPhase.flashingGreenStop
      self._clear_flash_candidate()
      self.stable_green_since_ns = 0
      self.green_count = 0
      self.pending_distance = 0.0
      self.pending_ego_station = self.ego_station
      self.pending_color = 0
      self._clear_pending_replacement()
      self.release_since_ns = None
      self.release_red_preserve_session = None
    self.stop_evidence_lost_since_ns = 0
    self.stop_reference = self.config.default_stop_reference
    # A stop session is also the ownership boundary for its geometry. Tesla can
    # recalculate the current control-point distance when its color/track
    # changes, so a new session must never inherit a station tracked while
    # GREEN or owned by an earlier stop session.
    if new_session or self.stop_station is None:
      self.stop_station = None
      self.remaining_distance = 0.0
      self.last_distance_innovation = 0.0
    self._update_stop_station(observation)
    required = v_ego ** 2 / (2.0 * max(self.remaining_distance, 0.5))
    self.phase = (TrafficControlPhase.braking if required >= 0.5 else TrafficControlPhase.approachRed) \
      if phase == TrafficControlPhase.approachRed else phase
    self._mark_transition(reason)

  def _set_release(self, now_ns: int, reason: str = "green_release") -> None:
    self.phase = TrafficControlPhase.release
    self.release_since_ns = now_ns
    # GREEN may release and later flicker back to the same event. A signal-loss
    # release is different: motion can resume while evidence is absent, so a
    # recovered RED must own a fresh session and repeat stop feasibility.
    self.release_red_preserve_session = False if reason == "signal_lost_release" else None
    self.stop_evidence_lost_since_ns = 0
    self.candidate_color = 0
    self.candidate_count = 0
    self.candidate_first_ns = 0
    self.candidate_last_ns = 0
    self.candidate_distance = 0.0
    self.candidate_ego_station = self.ego_station
    self._clear_pending_replacement()
    self.remaining_distance = 0.0
    self._mark_transition(reason)

  def _update_stop_evidence_loss(self, now_ns: int) -> None:
    if self.phase == TrafficControlPhase.release:
      self.stop_evidence_lost_since_ns = 0
      if (self.release_since_ns is not None
          and now_ns - self.release_since_ns >= int(self.config.release_s * 1e9)):
        self.reset()
        self.last_update_ns = now_ns
      return
    ordinary_stop = self.phase in (
      TrafficControlPhase.approachRed,
      TrafficControlPhase.braking,
      TrafficControlPhase.yellowStop,
    )
    if not ordinary_stop:
      self.stop_evidence_lost_since_ns = 0
      return
    if self.stop_evidence_lost_since_ns == 0:
      self.stop_evidence_lost_since_ns = now_ns
      return
    if now_ns - self.stop_evidence_lost_since_ns < int(STOP_EVIDENCE_LOSS_GRACE_S * 1e9):
      return
    self._set_release(now_ns, "signal_lost_release")

  def _observe_flash_pattern(self, observation: TeslaTrafficControlObservation) -> bool:
    """Confirm on the second OFF edge after sustained OFF and GREEN.

    Only real CAN frames prove duration. Repeated planner snapshots cannot
    turn one OFF edge into a repeated flash. This detector never owns STOP;
    it emits one confirmation and the existing latched STOP owns its exit.
    """
    # DBC NONE (0) means no recognized light, not an extinguished light. Only
    # explicit OFF (4) alternating with GREEN (2) can prove a flashing signal.
    color = observation.light_state
    frame_ns = observation.frame_mono_time
    flash_eligible = observation.valid_for_control or (color == 4 and observation.control_type == 3)
    if (color not in (2, 4) or not flash_eligible
        or not 0.0 <= observation.distance <= self.config.max_control_distance):
      self._clear_flash_candidate()
      self.stable_green_since_ns = 0
      return False
    if self.flash_latched:
      # Do not repeatedly confirm an owned flash STOP: doing so would bypass
      # its stable-GREEN exit timer on every later blink.
      return False
    if self.phase in (TrafficControlPhase.bypass, TrafficControlPhase.yellowPass):
      self._clear_flash_candidate()
      return False
    if frame_ns <= self.flash_last_frame_ns:
      return False
    if (self.flash_last_frame_ns and frame_ns - self.flash_last_frame_ns > TRAFFIC_CONTROL_STALE_NS
        or not self._same_track(observation)):
      self._clear_flash_candidate()
    if self.flash_anchor_station is not None:
      expected = max(0.0, self.flash_anchor_station - self.ego_station)
      if abs(observation.distance - expected) > self._distance_tolerance(observation.distance, expected):
        # Small per-frame jumps must not cumulatively bridge different control
        # points or a frozen distance while the vehicle is still moving.
        self._clear_flash_candidate()
    confirmed = False
    min_half_ns = int(self.config.flash_min_half_cycle_s * 1e9)
    max_half_ns = int(self.config.flash_interval_max_s * 1e9) - min_half_ns
    duration_ns = frame_ns - self.flash_color_since_ns
    if self.flash_color == 4 and self.first_off_ns and not self.flash_off_confirmed:
      if duration_ns > max_half_ns or (color == 2 and duration_ns < min_half_ns):
        self._clear_flash_candidate()
      elif duration_ns >= min_half_ns:
        self.flash_off_confirmed = True

    if color != self.flash_color:
      if color == 4:
        valid_green = self.flash_color == 2 and duration_ns >= min_half_ns
        period_ns = frame_ns - self.first_off_ns
        valid_period = (
          int(self.config.flash_interval_min_s * 1e9) <= period_ns
          <= int(self.config.flash_interval_max_s * 1e9)
        )
        # A complete GREEN/OFF/GREEN cycle plus the next OFF edge proves
        # repetition. Do not wait for that second OFF to finish or a third
        # pulse: at 2 Hz those extra samples consume the stopping margin.
        confirmed = bool(valid_green and valid_period and self.flash_off_confirmed)
        if not valid_green or (self.first_off_ns and not valid_period):
          self._clear_flash_candidate()
        if valid_green:
          self.first_off_ns = frame_ns
          self.flash_off_confirmed = False
      self.flash_color = color
      self.flash_color_since_ns = frame_ns
    elif self.first_off_ns and duration_ns > max_half_ns:
      self._clear_flash_candidate()
      self.flash_color = color
      self.flash_color_since_ns = frame_ns

    self.flash_last_frame_ns = frame_ns
    if self.flash_anchor_station is None and color == 2:
      self.flash_anchor_station = self.ego_station + observation.distance
    return confirmed

  def _far_stop_candidate(self, observation: TeslaTrafficControlObservation, v_ego: float) -> bool:
    if observation.light_state not in (1, 3):
      return False
    usable_distance = max(observation.distance - self.stop_reference, 0.5)
    required_decel = v_ego ** 2 / (2.0 * usable_distance)
    return required_decel < 0.5

  def _candidate_confirmed(self, observation: TeslaTrafficControlObservation, v_ego: float,
                           count: int, first_ns: int) -> bool:
    if count < 2:
      return False
    if not self._far_stop_candidate(observation, v_ego):
      return True
    return observation.frame_mono_time - first_ns >= int(self.config.far_candidate_confirm_s * 1e9)

  def _update_candidate(self, observation: TeslaTrafficControlObservation, v_ego: float) -> bool:
    expected = max(0.0, self.candidate_distance - (self.ego_station - self.candidate_ego_station))
    same = bool(
      self.candidate_count > 0
      and self.candidate_color == observation.light_state
      and abs(observation.distance - expected) <=
      self._distance_tolerance(observation.distance, expected)
    )
    if same:
      self.candidate_count += 1
    else:
      self.candidate_color = observation.light_state
      self.candidate_count = 1
      self.candidate_first_ns = observation.frame_mono_time
    self.candidate_distance = observation.distance
    self.candidate_ego_station = self.ego_station
    self.candidate_last_ns = observation.frame_mono_time
    return self._candidate_confirmed(
      observation, v_ego, self.candidate_count, self.candidate_first_ns,
    )

  def _pending_replacement_confirmed(self, observation: TeslaTrafficControlObservation, v_ego: float) -> bool:
    expected = max(0.0, self.pending_distance - (self.ego_station - self.pending_ego_station))
    same = bool(
      self.pending_count > 0
      and self.pending_color == observation.light_state
      and abs(observation.distance - expected) <= self._distance_tolerance(observation.distance, expected)
    )
    self.pending_count = self.pending_count + 1 if same else 1
    if not same:
      self.pending_first_ns = observation.frame_mono_time
      self.pending_replacement_farther = self.last_distance_innovation > 0.0
    self.pending_distance = observation.distance
    self.pending_ego_station = self.ego_station
    self.pending_color = observation.light_state
    confirmed = self._candidate_confirmed(
      observation, v_ego, self.pending_count, self.pending_first_ns,
    )
    if not confirmed or not self.pending_replacement_farther:
      return confirmed
    # Moving an owned stop point farther away relaxes braking and can alternate
    # the final plan between STOP and RELEASE. Require an extra real sample and
    # a full second; a nearer correction keeps the faster safety response.
    return bool(
      self.pending_count >= 3
      and observation.frame_mono_time - self.pending_first_ns >=
      int(self.config.farther_replacement_confirm_s * 1e9)
    )

  def update(self, observation: TeslaTrafficControlObservation, now_ns: int, *, v_ego: float, a_ego: float,
             enabled: bool, long_active: bool, gas_pressed: bool,
             brake_pressed: bool, turn_signal_active: bool,
             stop_direction_unknown: bool | None = None) -> TrafficControlDecision:
    del a_ego
    dt = 0.0 if self.last_update_ns is None else max(0.0, min((now_ns - self.last_update_ns) / 1e9, 0.5))
    self.last_update_ns = now_ns
    self.ego_station += max(0.0, v_ego) * dt
    if self.stop_station is not None:
      self.remaining_distance = max(0.0, self.stop_station - self.ego_station)
    self.event_continuous = False

    if self.config.mode == TrafficControlMode.off:
      self.reset()
      return self._decision()
    was_override_active = self.driver_override_active
    if gas_pressed:
      self.driver_override_until_ns = now_ns + int(self.config.driver_override_cooldown_s * 1e9)
    self.driver_override_active = gas_pressed or now_ns < self.driver_override_until_ns
    if self.driver_override_active:
      self.override_reconfirm_required = True
      self.override_reconfirm_count = 0
    elif was_override_active:
      self.override_reconfirm_required = True
      self.override_reconfirm_count = 0
    # Tesla 0x25D already represents the signal selected for the current lane.
    # Turn indicators and model manoeuvre intent remain diagnostic inputs only;
    # they must not veto an authoritative current-lane STOP or RELEASE.
    del turn_signal_active, stop_direction_unknown
    self.direction_unknown = False
    self.stop_direction_unknown = False
    current_stop_event = bool(
      self.phase in (*self.ACTIVE_PHASES, TrafficControlPhase.redCandidate,
                     TrafficControlPhase.greenFlashCandidate, TrafficControlPhase.release)
      or (self.first_off_ns > 0 and self.flash_color == 2)
      or (observation.available and observation.valid_for_control
          and observation.light_state in (1, 3)
          and observation.distance <= self.config.max_control_distance)
    )
    if gas_pressed and self.phase != TrafficControlPhase.bypass and current_stop_event:
      self.phase = TrafficControlPhase.bypass
      self.stop_session_id = 0
      self._mark_transition("driver_bypass")
    self.raw_observation_fresh = observation.available
    if self.raw_observation_fresh:
      stale_gap = bool(
        self.raw_stale_since_ns is not None
        and now_ns - self.raw_stale_since_ns >= int(self.config.critical_observation_dropout_s * 1e9)
      )
      real_frame_gap = bool(
        self.last_real_frame_ns > 0 and observation.frame_mono_time > self.last_real_frame_ns
        and observation.frame_mono_time - self.last_real_frame_ns >= int(self.config.critical_observation_dropout_s * 1e9)
      )
      long_dropout = stale_gap or real_frame_gap
      if long_dropout:
        # Confirmation counters describe consecutive real CAN evidence and
        # cannot span a multi-second transport gap.
        self.green_count = 0
        self.stable_green_since_ns = 0
        self.candidate_count = 0
        self.candidate_first_ns = 0
        self.candidate_last_ns = 0
        self._clear_pending_replacement()
        if not self.flash_latched:
          self._clear_flash_candidate()
      if long_dropout and self.phase in self.ACTIVE_PHASES:
        self.stop_reconfirm_required = True
        self.stop_reconfirm_count = 0
      if long_dropout and self.phase == TrafficControlPhase.release:
        # A release is only valid while the confirming green observation is
        # continuous. Do not let one recovery frame revive an old GO session.
        self.reset()
        self.last_update_ns = now_ns
        return self._decision()
      self.raw_stale_since_ns = None
    elif self.raw_stale_since_ns is None:
      self.raw_stale_since_ns = self.last_real_frame_ns if self.last_real_frame_ns > 0 else now_ns
    if not enabled or (self.config.mode in (TrafficControlMode.stopOnly, TrafficControlMode.stopGo) and not long_active):
      self.reset()
      return self._decision()
    new_frame = bool(observation.available and observation.frame_mono_time > 0 and
                     observation.frame_mono_time != self.last_real_frame_ns and
                     observation.source_bus == 2 and observation.dlc >= 6 and
                     observation.control_type == 3 and observation.light_state in (0, 1, 2, 3, 4))
    if not new_frame:
      unsupported_raw_frame = bool(
        observation.available and observation.frame_mono_time > 0
        and observation.frame_mono_time != self.last_real_frame_ns
        and observation.source_bus == 2 and observation.dlc >= 6
        and (observation.control_type != 3 or observation.light_state not in (0, 1, 2, 3, 4))
      )
      if unsupported_raw_frame:
        # A fresh tuple with unsupported semantics is evidence loss, not a CAN
        # transport dropout and never valid geometry for an owned moving STOP.
        self._clear_pending_replacement()
        self._clear_flash_candidate()
        self.stable_green_since_ns = 0
        self._update_stop_evidence_loss(now_ns)
      if (not observation.available and self.last_real_frame_ns > 0
          and now_ns - self.last_real_frame_ns > TRAFFIC_CONTROL_STALE_NS):
        self._clear_flash_candidate()
        self.stable_green_since_ns = 0
      if self.phase in (TrafficControlPhase.redCandidate, TrafficControlPhase.greenFlashCandidate):
        if self.candidate_last_ns and now_ns - self.candidate_last_ns > int(self.config.candidate_dropout_s * 1e9):
          self.phase = TrafficControlPhase.off
          self.candidate_count = 0
          self.candidate_first_ns = 0
      transport_evidence_lost = bool(
        not observation.available and self.raw_stale_since_ns is not None
      )
      if (v_ego < 0.3 and self.phase in self.ACTIVE_PHASES
          and self._hold_distance_consistent(
            evidence_lost=bool(self.stop_evidence_lost_since_ns) or transport_evidence_lost,
          )):
        self.phase = TrafficControlPhase.hold
      if self.stop_evidence_lost_since_ns:
        self._update_stop_evidence_loss(now_ns)
      return self._decision()

    previous_color = self.last_real_color
    if (self.last_real_frame_ns > 0
        and observation.frame_mono_time - self.last_real_frame_ns > TRAFFIC_CONTROL_STALE_NS):
      self._clear_flash_candidate()
      self.stable_green_since_ns = 0
    self.last_real_frame_ns = observation.frame_mono_time
    self.light_state = observation.light_state
    self.source_bus = observation.source_bus
    self.quality = observation.quality
    flashing_pattern = self._observe_flash_pattern(observation)

    if self.override_reconfirm_required and not self.driver_override_active:
      if observation.light_state in (1, 3) and observation.distance <= self.config.max_control_distance:
        self.override_reconfirm_count += 1
        if self.override_reconfirm_count >= 2:
          self.override_reconfirm_required = False
      else:
        self.override_reconfirm_count = 0

    if observation.distance > self.config.max_control_distance:
      wrapped = bool(self.last_raw_distance is not None and self.last_raw_distance <= 2.0 and observation.distance >= 250.0)
      if observation.distance >= 250.0 and self.phase == TrafficControlPhase.yellowPass:
        self.yellow_latched = None
        self.phase = TrafficControlPhase.passed
      if wrapped:
        self.phase = TrafficControlPhase.passed
        self.stop_session_id = 0
        self.stop_station = None
        self.yellow_latched = None
        self.flash_latched = False
        self._clear_flash_candidate()
        self.stable_green_since_ns = 0
        self.stop_evidence_lost_since_ns = 0
        self.remaining_distance = 0.0
        self._mark_transition("distance_wrap_passed")
      else:
        self._clear_pending_replacement()
        if (v_ego < 0.3 and self.phase in self.ACTIVE_PHASES
            and self._hold_distance_consistent(evidence_lost=bool(self.stop_evidence_lost_since_ns))):
          self.phase = TrafficControlPhase.hold
          self._mark_transition("stationary_hold")
        self._update_stop_evidence_loss(now_ns)
      self.last_real_color = observation.light_state
      return self._decision()

    if observation.light_state not in (0, 4):
      self.stop_evidence_lost_since_ns = 0

    if self.phase == TrafficControlPhase.bypass:
      self.last_raw_distance = observation.distance
      self.last_distance_ego_station = self.ego_station
      self.last_real_color = observation.light_state
      return self._decision()

    if self.phase == TrafficControlPhase.yellowPass:
      # Yellow PASS is a one-time decision for this current-lane event. A
      # later RED cannot reacquire Traffic ownership near the line.
      self.last_raw_distance = observation.distance
      self.last_distance_ego_station = self.ego_station
      self.last_real_color = observation.light_state
      return self._decision()

    if (self.phase not in (*self.ACTIVE_PHASES, TrafficControlPhase.release)
        and v_ego > self.config.max_control_speed):
      self.candidate_count = 0
      self.candidate_first_ns = 0
      self.phase = TrafficControlPhase.bypass
      self.last_raw_distance = observation.distance
      self.last_distance_ego_station = self.ego_station
      self._mark_transition("speed_above_limit")
      self.last_real_color = observation.light_state
      return self._decision()

    if flashing_pattern:
      if self.stop_reconfirm_required and not self.stop_direction_unknown:
        self.stop_reconfirm_required = False
        self.stop_reconfirm_count = 0
      self._start_stop(observation, v_ego, TrafficControlPhase.flashingGreenStop, "flashing_green_stop")
      if observation.light_state == 2:
        self.stable_green_since_ns = observation.frame_mono_time
      self.last_raw_distance = observation.distance
      self.last_distance_ego_station = self.ego_station
      self.last_real_color = observation.light_state
      return self._decision()

    same_track = self._same_track(observation)
    self.event_continuous = same_track
    if (observation.light_state == 2 and self.phase in self.ACTIVE_PHASES
        and not self.flash_latched):
      # Tesla bus-2 color is authoritative for the current lane. Geometry
      # continuity may protect a stop station from a one-frame distance jump,
      # but it must not permanently veto an ordinary GREEN release. Moving
      # releases on the first real GREEN; standstill still requires two
      # distinct real frames.
      self.green_count = self.green_count + 1 if previous_color == 2 else 1
      self._clear_pending_replacement()
      if (v_ego >= 0.3 or self.green_count >= 2) and not brake_pressed:
        if not same_track:
          self.stop_station = None
        self.can_remaining = max(0.0, observation.distance - self.stop_reference)
        self.last_raw_distance = observation.distance
        self.last_distance_ego_station = self.ego_station
        self.last_real_color = observation.light_state
        self._set_release(now_ns)
      else:
        self.last_real_color = observation.light_state
      return self._decision()

    if observation.light_state in (0, 4) and self.phase in (*self.ACTIVE_PHASES, TrafficControlPhase.release):
      # NONE/OFF are fresh raw data, but neither is STOP geometry nor a GREEN
      # release. Freeze the last confirmed stop point through short OEM color
      # gaps, then smoothly return an ordinary moving STOP to the base planner.
      # A terminal HOLD and a confirmed flashing-green STOP remain latched.
      self._clear_pending_replacement()
      self.stable_green_since_ns = 0
      if (v_ego < 0.3 and self.phase in self.ACTIVE_PHASES
          and self._hold_distance_consistent(evidence_lost=bool(self.stop_evidence_lost_since_ns))):
        self.phase = TrafficControlPhase.hold
        self._mark_transition("stationary_hold")
      self._update_stop_evidence_loss(now_ns)
      self.last_real_color = observation.light_state
      return self._decision()

    if not same_track and self.phase in self.ACTIVE_PHASES:
      self.stable_green_since_ns = 0
      # A single discontinuous tuple cannot replace or release the current
      # lane event. A nearer track confirms in two motion-consistent frames;
      # a farther track needs three frames spanning one second because it
      # relaxes braking. Start a fresh session so geometry and feasibility are
      # both recomputed. HOLD and flashing STOP stay latched and can only be
      # released by their existing GREEN/driver rules.
      replaceable_phase = self.phase in (
        TrafficControlPhase.approachRed,
        TrafficControlPhase.braking,
        TrafficControlPhase.yellowStop,
      )
      replace = bool(
        replaceable_phase and observation.light_state in (1, 3)
        and self._pending_replacement_confirmed(observation, v_ego)
      )
      if not replace:
        if observation.light_state != 2:
          self.green_count = 0
        self.last_real_color = observation.light_state
        return self._decision()
      self.last_raw_distance = observation.distance
      replacement_phase = (TrafficControlPhase.yellowStop if observation.light_state == 3
                           else TrafficControlPhase.approachRed)
      self._start_stop(observation, v_ego, replacement_phase, "candidate_replaced")
      self.event_continuous = True
      self.last_distance_ego_station = self.ego_station
      self.stop_reconfirm_required = False
      self.stop_reconfirm_count = 0
      self._clear_pending_replacement()
      self.last_real_color = observation.light_state
      return self._decision()
    if same_track:
      self._clear_pending_replacement()

    if self.stop_reconfirm_required and not self.stop_direction_unknown:
      if observation.light_state in (1, 3):
        self.stop_reconfirm_count += 1
        if self.stop_reconfirm_count >= 2:
          self.stop_reconfirm_required = False
          self.stop_reconfirm_count = 0
          if self.phase in (
            TrafficControlPhase.approachRed,
            TrafficControlPhase.braking,
            TrafficControlPhase.yellowStop,
          ):
            recovery_phase = (
              TrafficControlPhase.yellowStop if observation.light_state == 3
              else TrafficControlPhase.approachRed
            )
            self._start_stop(observation, v_ego, recovery_phase, "dropout_reconfirmed")
            self.event_continuous = True
            self.last_raw_distance = observation.distance
            self.last_distance_ego_station = self.ego_station
            self.last_real_color = observation.light_state
            return self._decision()
      else:
        self.stop_reconfirm_count = 0

    self._update_stop_station(observation)
    self.last_raw_distance = observation.distance
    self.last_distance_ego_station = self.ego_station

    color = observation.light_state
    confirmed = self._update_candidate(observation, v_ego)

    if self.phase in self.ACTIVE_PHASES:
      # A confirmed red/yellow ends the flashing-green interpretation without
      # ending the stop session. The ordinary stop state can then release on a
      # later stable green instead of keeping flash_latched forever.
      if self.flash_latched and color in (1, 3) and confirmed:
        self.flash_latched = False
      if self.flash_latched:
        if color == 2:
          if self.stable_green_since_ns == 0:
            self.stable_green_since_ns = observation.frame_mono_time
          stable_green_ns = observation.frame_mono_time - self.stable_green_since_ns
          if stable_green_ns >= int(self.config.flash_interval_max_s * 1e9):
            self.flash_latched = False
            self._clear_flash_candidate()
            self._set_release(now_ns)
          else:
            self.phase = (TrafficControlPhase.hold if v_ego < 0.3 and self._hold_distance_consistent()
                          else TrafficControlPhase.flashingGreenStop)
        else:
          self.stable_green_since_ns = 0
          self.phase = (TrafficControlPhase.hold if v_ego < 0.3 and self._hold_distance_consistent()
                        else TrafficControlPhase.flashingGreenStop)
      else:
        self.green_count = 0
        if color == 3:
          self.phase = TrafficControlPhase.yellowStop
        elif color == 1 and self.phase != TrafficControlPhase.hold:
          required = v_ego ** 2 / (2.0 * max(self.remaining_distance, 0.5))
          self.phase = TrafficControlPhase.braking if required >= 0.5 else TrafficControlPhase.approachRed
      if v_ego < 0.3 and self._hold_distance_consistent() and self.phase != TrafficControlPhase.release:
        self.phase = TrafficControlPhase.hold
        self._mark_transition("stationary_hold")
      self.last_real_color = observation.light_state
      return self._decision()

    if self.phase == TrafficControlPhase.release:
      if color == 1:
        if self.release_red_preserve_session is None:
          self.release_red_preserve_session = same_track
        else:
          self.release_red_preserve_session = self.release_red_preserve_session and same_track
        if confirmed:
          self._start_stop(
            observation, v_ego, TrafficControlPhase.approachRed, "red_after_release",
            preserve_session=bool(self.release_red_preserve_session),
          )
          self.release_red_preserve_session = None
      elif self.release_since_ns is not None:
        release_timeout_s = (
          self.config.stationary_release_s if color == 2 and v_ego < 0.3
          else self.config.release_s
        )
        if now_ns - self.release_since_ns >= int(release_timeout_s * 1e9):
          self.reset()
          self.last_update_ns = now_ns
      self.last_real_color = observation.light_state
      return self._decision()

    if color == 1:
      self.phase = TrafficControlPhase.redCandidate
      if confirmed:
        self._start_stop(observation, v_ego, TrafficControlPhase.approachRed, "stop_confirmed")
    elif color == 3:
      required = v_ego ** 2 / (2.0 * max(observation.distance - self.stop_reference, 0.5))
      if self.yellow_latched is None and confirmed:
        if required <= self.config.yellow_stop_decel:
          self.yellow_latched = True
        elif required >= self.config.yellow_pass_decel:
          self.yellow_latched = False
        else:
          self.yellow_latched = required <= self.config.comfort_brake
      if self.yellow_latched is True:
        self._start_stop(observation, v_ego, TrafficControlPhase.yellowStop, "yellow_stop")
      elif self.yellow_latched is False:
        self.phase = TrafficControlPhase.yellowPass
        self._mark_transition("yellow_pass")
      else:
        self.phase = TrafficControlPhase.redCandidate
    elif color == 2:
      self.green_count = self.green_count + 1 if previous_color == 2 else 1
      if self.phase != TrafficControlPhase.greenFlashCandidate and self.green_count >= 2:
        self.phase = TrafficControlPhase.off
        self.yellow_latched = None
        self.flash_latched = False
        self.stable_green_since_ns = 0
    elif color in (0, 4) and self.phase != TrafficControlPhase.greenFlashCandidate:
      self.phase = TrafficControlPhase.off
      self.yellow_latched = None
      self.flash_latched = False
      self.stable_green_since_ns = 0

    self.last_real_color = observation.light_state
    return self._decision()
