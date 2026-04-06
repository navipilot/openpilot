from types import SimpleNamespace
from unittest.mock import patch

from openpilot.selfdrive.controls import radard


def make_model_data():
  lane_lines = [
    SimpleNamespace(x=[0.0, 150.0], y=[-3.5, -3.5]),
    SimpleNamespace(x=[0.0, 150.0], y=[-1.8, -1.8]),
    SimpleNamespace(x=[0.0, 150.0], y=[1.8, 1.8]),
    SimpleNamespace(x=[0.0, 150.0], y=[3.5, 3.5]),
  ]
  meta = SimpleNamespace(
    laneChangeState=radard.LaneChangeState.off,
    laneChangeDirection=radard.LaneChangeDirection.none,
  )
  return SimpleNamespace(laneLines=lane_lines, meta=meta)


def make_track(d_rel):
  track = radard.Track(123, 20.0, radard.KalmanParams(0.05))
  track.update(d_rel=d_rel, y_rel=0.0, v_rel=0.0, v_lead=20.0, measured=1.0)
  return track


def make_radar_only_lead(tracks):
  return radard.get_lead(
    v_ego=20.0,
    ready=False,
    tracks=tracks,
    lead_msg=SimpleNamespace(prob=0.0),
    model_v_ego=20.0,
    model_data=make_model_data(),
    standstill=False,
    starpilot_plan=SimpleNamespace(increasedStoppedDistance=0.0),
    starpilot_toggles=SimpleNamespace(lead_detection_probability=0.35, human_lane_changes=False),
  )


def test_close_radar_only_far_lead_is_blocked_in_testing_ground_2():
  track = make_track(5.0)

  with patch.object(radard, "testing_ground", SimpleNamespace(use_2=False)):
    lead = {"status": False}
    for _ in range(20):
      lead = make_radar_only_lead({track.identifier: track})
    assert lead["status"]
    assert lead["radarTrackId"] == track.identifier

  track = make_track(5.0)
  with patch.object(radard, "testing_ground", SimpleNamespace(use_2=True)):
    lead = {"status": False}
    for _ in range(20):
      lead = make_radar_only_lead({track.identifier: track})
    assert not lead["status"]


def test_far_radar_only_lead_survives_testing_ground_2():
  track = make_track(80.0)

  with patch.object(radard, "testing_ground", SimpleNamespace(use_2=True)):
    lead = {"status": False}
    for _ in range(20):
      lead = make_radar_only_lead({track.identifier: track})
    assert lead["status"]
    assert lead["radarTrackId"] == track.identifier
