"""Flashing evidence crosses the real Traffic source and final-plan boundary."""

from types import SimpleNamespace as ns

import pytest

import openpilot.cereal.messaging as messaging
from openpilot.sunnypilot.selfdrive.traffic_control.controller import (
  TrafficControlConfig, TrafficControlMode, TrafficControlPhase,
)
from openpilot.sunnypilot.selfdrive.traffic_control.final_plan_arbitrator import (
  FinalPlanArbitrator, TrafficPlanAction, TrafficStartBlockReason,
)
from openpilot.sunnypilot.selfdrive.traffic_control.radar_state import (
  TrafficRadarGoPolicy, TrafficRadarSource,
)
from openpilot.sunnypilot.selfdrive.traffic_control.tests.test_final_plan_arbitrator import (
  base_plan, fake_sm, plan_output,
)
from openpilot.sunnypilot.selfdrive.traffic_control.tests.test_radar_state import red_light_sm


FLASH_FRAMES = ((1.0, 2), (1.2, 4), (1.7, 2), (2.2, 4))


class FlashControlChain:
  def __init__(self, *, v_ego=8.0, distance=70.0, near_lead=False):
    self.source = TrafficRadarSource(
      TrafficControlConfig(mode=TrafficControlMode.stopGo), TrafficRadarGoPolicy.active,
    )
    self.source_sm = red_light_sm()
    self.source_sm["carState"].vEgo = v_ego
    self.source_sm["carState"].vCruise = 50.0
    self.initial_distance = distance
    # The second OFF confirms with 55.4 m left, inside the comfort horizon.
    self.arbitrator = FinalPlanArbitrator(ns(longitudinalActuatorDelay=0.3))
    self.plan_sm = fake_sm(v_ego=v_ego)
    self.plan_sm.values["carState"] = self.source_sm["carState"]
    self.plan_sm["radarState"].leadOne = ns(present=near_lead, dRel=4.0)

  def step(self, now_s, color):
    now_ns = round(now_s * 1e9)
    raw = self.source_sm["carStateSP"].teslaTrafficControl
    raw.lightState = color
    raw.validForControl = raw.lightState != 4
    raw.quality = 0 if raw.lightState == 4 else 2
    raw.frameMonoTime = now_ns
    raw.distance = self.initial_distance - self.source_sm["carState"].vEgo * (now_s - 1.0)
    message = self.source.update(self.source_sm, now_ns)
    assert message.valid
    self.target = message.trafficRadarState
    self.plan_sm.values["trafficRadarState"] = self.target
    self.plan = base_plan(a_target=0.4)
    self.original_plan = plan_output(self.plan)
    self.arbitrator.apply(self.plan, self.plan_sm, now_ns)
    companion = messaging.new_message("longitudinalPlanSP")
    self.arbitrator.annotate_plan_sp(companion.longitudinalPlanSP)
    self.display = companion.longitudinalPlanSP.teslaTrafficControl
    assert self.display.finalATarget == pytest.approx(self.plan.aTarget)
    assert self.display.shouldStop == self.plan.shouldStop
    return self.target


def test_confirmed_flash_stops_before_yellow_and_stable_green_releases_to_base():
  chain = FlashControlChain()
  for now_s, color in FLASH_FRAMES:
    target = chain.step(now_s, color)
    if now_s < 2.2:
      assert not target.targetPresent
      assert not target.plannerStartRequested
      assert plan_output(chain.plan) == chain.original_plan
      assert not chain.display.applied

  assert target.phase == int(TrafficControlPhase.flashingGreenStop)
  assert target.lightState == 4
  assert target.targetPresent and target.stopControlAllowed
  assert target.quality == 0
  assert target.oemTargetDistance == pytest.approx(target.distanceToStopPoint + 5.0)
  assert chain.display.stopReference == pytest.approx(5.0)
  assert target.stopSessionId > 0
  assert chain.display.action == int(TrafficPlanAction.stop)
  assert chain.display.applied
  assert chain.plan.aTarget < 0.0
  assert not target.plannerStartRequested
  session_id = target.stopSessionId

  for now_s in (2.7, 3.2, 3.7):
    target = chain.step(now_s, 2)
    assert target.phase == int(TrafficControlPhase.flashingGreenStop)
    assert target.stopSessionId == session_id
    assert not target.plannerStartRequested
    assert not chain.display.startApplied
    assert chain.display.action == int(TrafficPlanAction.stop)

  released = chain.step(4.2, 2)
  assert released.phase == int(TrafficControlPhase.release)
  assert released.stopSessionId == session_id
  assert plan_output(chain.plan) == chain.original_plan
  assert not chain.display.startApplied


def test_continued_flashing_keeps_one_stop_session_and_does_not_delay_stable_green_release():
  chain = FlashControlChain()
  for now_s, color in FLASH_FRAMES:
    chain.step(now_s, color)
  session_id = chain.target.stopSessionId

  for now_s, color in (
    (2.7, 2), (3.2, 4), (3.4, 4), (3.7, 2), (4.2, 4), (4.4, 4),
    (4.7, 2), (5.2, 4), (5.4, 4), (5.7, 2), (6.2, 2), (6.7, 2),
  ):
    target = chain.step(now_s, color)
    assert target.phase == int(TrafficControlPhase.flashingGreenStop)
    assert target.stopSessionId == session_id
    assert not target.plannerStartRequested
    assert chain.display.action == int(TrafficPlanAction.stop)
    assert not chain.display.startApplied

  released = chain.step(7.2, 2)
  assert released.phase == int(TrafficControlPhase.release)
  assert released.stopSessionId == session_id
  assert plan_output(chain.plan) == chain.original_plan


def test_flash_hold_release_still_blocks_active_go_for_a_near_lead():
  chain = FlashControlChain(v_ego=0.0, distance=5.0, near_lead=True)
  for now_s, color in FLASH_FRAMES:
    chain.step(now_s, color)
  for now_s in (2.7, 3.2, 3.7):
    target = chain.step(now_s, 2)
    assert not target.plannerStartRequested
    assert not chain.display.startApplied

  released = chain.step(4.2, 2)
  assert released.phase == int(TrafficControlPhase.release)
  assert released.plannerStartRequested
  assert chain.display.startBlockReason == int(TrafficStartBlockReason.physicalLead)
  assert not chain.display.startApplied
  assert plan_output(chain.plan) == chain.original_plan


def test_repeated_green_none_sequence_never_owns_stop_or_go():
  chain = FlashControlChain()
  none_frames = tuple((now_s, 0 if color == 4 else color) for now_s, color in FLASH_FRAMES)
  for now_s, color in none_frames + (
    (3.7, 2), (4.2, 0), (4.4, 0), (4.7, 2), (5.2, 0), (5.4, 0),
    (5.7, 2), (6.2, 0), (6.4, 0), (6.7, 2), (7.2, 2), (7.7, 2),
  ):
    target = chain.step(now_s, color)
    assert not target.targetPresent
    assert target.stopSessionId == 0
    assert not target.controlAllowed
    assert not target.plannerStartRequested
    assert not chain.display.applied
    assert not chain.display.startApplied
    assert plan_output(chain.plan) == chain.original_plan
