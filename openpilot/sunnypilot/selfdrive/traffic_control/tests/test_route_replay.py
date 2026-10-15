import json
from pathlib import Path
from types import SimpleNamespace

import pytest

from openpilot.sunnypilot.selfdrive.traffic_control.controller import TrafficControlConfig, TrafficControlMode
from openpilot.sunnypilot.selfdrive.traffic_control.radar_state import TrafficRadarGoPolicy, TrafficRadarSource


FIXTURE = Path(__file__).parent / "fixtures" / "tesla_traffic_routes.json"


def ns(**kwargs):
  return SimpleNamespace(**kwargs)


class ReplaySubMaster:
  SERVICES = ('carStateSP', 'carState', 'carControl', 'modelV2')

  def __init__(self) -> None:
    self.seen = dict.fromkeys(self.SERVICES, True)
    self.alive = dict.fromkeys(self.SERVICES, True)
    self.valid = dict.fromkeys(self.SERVICES, True)
    self.messages = {}

  def __getitem__(self, key):
    return self.messages[key]

  def load(self, frame) -> int:
    now_ns = frame['tNs']
    available, valid_for_control, bus, control_source, distance, light, quality = frame['traffic']
    v_ego, a_ego, brake, left_blinker, right_blinker = frame['car']
    model_valid, model_distance, model_terminal_speed = frame['model']
    traffic = ns(
      available=available, validForControl=valid_for_control, sourceBus=bus, dlc=6,
      featureState=3, stateMachine=4, controlSource=control_source, controlType=3,
      distance=distance, lightState=light, continuationReason=0, confirmationType=0,
      warningSuppressionReason=0, unavailableReason=0, visionLight=True,
      visionSign=False, visionRoadMarking=False, visionLine=False,
      frameMonoTime=now_ns - frame['trafficAgeNs'], quality=quality,
    )
    self.messages = {
      'carStateSP': ns(teslaTrafficControl=traffic),
      # The retained routes were recorded while disengaged. Replay forces the
      # agreed closed-course control gates so it exercises detection only; all
      # Tesla CAN, ego motion, model and real-lead observations remain recorded.
      'carState': ns(vEgo=v_ego, aEgo=a_ego, gasPressed=False, brakePressed=brake),
      'carControl': ns(enabled=True, longActive=True,
                       leftBlinker=left_blinker, rightBlinker=right_blinker),
      'modelV2': ns(
        position=ns(x=[model_distance] * 33),
        velocity=ns(x=[model_terminal_speed] * 33),
      ),
    }
    self.valid['carStateSP'] = frame['carStateSPValid']
    self.valid['modelV2'] = model_valid
    return now_ns


@pytest.mark.parametrize(
  ("route_name", "expected_present"),
  [
    ("00000009--72ea96171f--1", 165),
    ("00000009--72ea96171f--2", 79),
    ("00000009--72ea96171f--4", 72),
  ],
)
def test_recorded_traffic_candidates_replay_deterministically(route_name, expected_present):
  routes = {route['route']: route for route in json.loads(FIXTURE.read_text())['routes']}
  source = TrafficRadarSource(
      TrafficControlConfig(
        mode=TrafficControlMode.stopGo,
      ),
    go_policy=TrafficRadarGoPolicy.active,
  )
  sm = ReplaySubMaster()
  present_targets = []
  suppressed_frames = 0
  start_requests = 0

  for frame in routes[route_name]['frames']:
    target = source.update(sm, sm.load(frame)).trafficRadarState
    suppressed_frames += int(target.suppressedByPhysicalLead)
    start_requests += int(target.plannerStartRequested)
    if target.targetPresent and target.controlAllowed:
      present_targets.append((int(target.eventId), float(target.distanceToStopPoint)))

  positive_jumps = [
    current_distance - previous_distance
    for (previous_event, previous_distance), (current_event, current_distance)
    in zip(present_targets, present_targets[1:], strict=False)
    if current_event == previous_event and current_distance > previous_distance
  ]
  assert len(present_targets) == expected_present
  assert suppressed_frames == 0
  assert max(positive_jumps, default=0.0) <= 3.0
  assert start_requests == 0
