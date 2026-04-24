"""
Copyright © IQ.Lvbs, apart of Project Teal Lvbs, All Rights Reserved, licensed under https://konn3kt.com/tos
"""
from types import SimpleNamespace

from opendbc.car.volkswagen.carcontroller import MQBStandstillManager


def build_state(*, brake_pressed=False, v_ego=0.0, esp_hold=False, esp_stopping=False,
                rolling_backward=False, rolling_forward=False, grade=0.0, sum_wegimpulse=0):
  out = SimpleNamespace(brakePressed=brake_pressed, vEgo=v_ego)
  return SimpleNamespace(
    out=out,
    esp_hold_confirmation=esp_hold,
    esp_stopping=esp_stopping,
    rolling_backward=rolling_backward,
    rolling_forward=rolling_forward,
    grade=grade,
    sum_wegimpulse=sum_wegimpulse,
  )


def test_brake_pressed_disables_long_active():
  mgr = MQBStandstillManager()
  long_active, *_ = mgr.update(build_state(brake_pressed=True), True, 0.0, False, False, 5.0)
  assert not long_active


def test_rollback_forces_full_brake():
  mgr = MQBStandstillManager()
  state = build_state(rolling_backward=True)

  long_active, accel, stopping, starting, *_ = mgr.update(state, True, -0.2, False, True, 5.0)

  assert long_active
  assert accel == -3.5
  assert stopping
  assert not starting


def test_stop_commit_triggers_below_safe_speed():
  mgr = MQBStandstillManager()
  grade = 20.0
  safe_speed = mgr.get_theoretical_safe_speed(grade)

  long_active, accel, stopping, starting, *_ = mgr.update(
    build_state(v_ego=safe_speed * 0.5, grade=grade), True, 0.0, False, False, 0.0,
  )

  assert long_active
  assert mgr.stop_commit_active
  assert accel == -3.5
  assert stopping
  assert not starting


def test_stop_commit_can_transition_to_start_commit():
  mgr = MQBStandstillManager()
  grade = 20.0
  safe_speed = mgr.get_theoretical_safe_speed(grade)

  mgr.update(build_state(v_ego=safe_speed * 0.5, grade=grade), True, -0.2, False, False, 0.0)
  assert mgr.stop_commit_active

  long_active, accel, stopping, starting, *_ = mgr.update(
    build_state(v_ego=safe_speed * 0.5, grade=grade), True, 0.5, True, False, safe_speed * 2.0,
  )

  assert long_active
  assert mgr.start_commit_active
  assert not mgr.stop_commit_active
  assert accel >= 0.2
  assert starting
  assert not stopping


def test_esp_stopping_uses_acc07_start_override():
  mgr = MQBStandstillManager()

  long_active, accel, stopping, starting, esp_starting_override, esp_stopping_override = mgr.update(
    build_state(v_ego=0.1, esp_stopping=True), True, -0.2, True, False, 0.0,
  )

  assert long_active
  assert accel == -0.2
  assert stopping
  assert not starting
  assert esp_starting_override is True
  assert esp_stopping_override is False


def test_start_commit_wins_when_both_commits_are_active():
  mgr = MQBStandstillManager()
  mgr.stop_commit_active = True
  mgr.start_commit_active = True

  long_active, accel, stopping, starting, esp_starting_override, esp_stopping_override = mgr.update(
    build_state(v_ego=0.0), True, 0.5, False, False, 5.0,
  )

  assert long_active
  assert accel >= 0.2
  assert starting
  assert not stopping
  assert esp_starting_override is None
  assert esp_stopping_override is None


def test_start_commit_uses_acc07_start_override():
  mgr = MQBStandstillManager()
  mgr.start_commit_active = True

  _, _, stopping, starting, esp_starting_override, esp_stopping_override = mgr.update(
    build_state(v_ego=0.0), True, 0.5, False, False, 5.0,
  )

  assert starting
  assert not stopping
  assert esp_starting_override is True
  assert esp_stopping_override is False


def test_can_stop_forever_clears_above_95_kph():
  mgr = MQBStandstillManager()

  mgr.update(build_state(v_ego=0.0, esp_stopping=True), True, -0.2, True, False, 0.0)
  assert mgr.can_stop_forever

  mgr.update(build_state(v_ego=(9.5 / 3.6) + 0.1), True, 0.5, False, True, 5.0)
  assert not mgr.can_stop_forever


def test_standstill_uses_acc07_stop_override():
  mgr = MQBStandstillManager()
  state = build_state(v_ego=0.0, sum_wegimpulse=0)

  esp_starting_override = None
  esp_stopping_override = None
  for _ in range(MQBStandstillManager.WEGIMPULSE_STILLNESS_FRAMES + 1):
    _, _, _, _, esp_starting_override, esp_stopping_override = mgr.update(state, True, -0.2, True, False, 0.0)

  assert esp_starting_override is False
  assert esp_stopping_override is True
