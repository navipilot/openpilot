from openpilot.selfdrive.controls.lib.lane_localizer import LaneLocalizer


GOOD_PROBS = [0.2, 0.95, 0.95, 0.2]
EDGES = [2.0, 2.0]


def test_map_hint_is_low_confidence_until_local_tracking_accumulates():
  localizer = LaneLocalizer()

  first = localizer.update(
    route_generation=1, lane_count=3, current_lane_hint=1,
    lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
    lane_change_active=False, lane_change_direction="none",
  )
  for _ in range(13):
    latest = localizer.update(
      route_generation=1, lane_count=3, current_lane_hint=1,
      lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
      lane_change_active=False, lane_change_direction="none",
    )

  assert first.estimated_lane == 1
  assert first.valid is False
  assert latest.confidence >= 0.60
  assert latest.valid is True
  assert sum(latest.lane_probabilities) == 1.0


def test_completed_existing_lane_change_updates_shadow_lane_once():
  localizer = LaneLocalizer()
  for _ in range(14):
    current = localizer.update(
      route_generation=1, lane_count=3, current_lane_hint=1,
      lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
      lane_change_active=False, lane_change_direction="none",
    )
  changing = localizer.update(
    route_generation=1, lane_count=3, current_lane_hint=1,
    lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
    lane_change_active=True, lane_change_direction="right",
  )
  completed = localizer.update(
    route_generation=1, lane_count=3, current_lane_hint=1,
    lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
    lane_change_active=False, lane_change_direction="none",
  )

  assert current.estimated_lane == 1
  assert changing.estimated_lane == 1
  assert completed.estimated_lane == 2
  assert completed.valid is True


def test_route_change_lane_count_change_and_low_quality_degrade_confidence():
  localizer = LaneLocalizer()
  first = localizer.update(
    route_generation=1, lane_count=3, current_lane_hint=1,
    lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
    lane_change_active=False, lane_change_direction="none",
  )
  reroute = localizer.update(
    route_generation=2, lane_count=3, current_lane_hint=-1,
    lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
    lane_change_active=False, lane_change_direction="none",
  )
  changed_lanes = localizer.update(
    route_generation=2, lane_count=4, current_lane_hint=-1,
    lane_line_probs=GOOD_PROBS, road_edge_distances=EDGES,
    lane_change_active=False, lane_change_direction="none",
  )
  degraded = localizer.update(
    route_generation=2, lane_count=4, current_lane_hint=2,
    lane_line_probs=[0.1, 0.1, 0.1, 0.1], road_edge_distances=EDGES,
    lane_change_active=False, lane_change_direction="none",
  )

  assert first.estimated_lane == 1
  assert reroute.estimated_lane == -1
  assert changed_lanes.estimated_lane == -1
  assert degraded.estimated_lane == -1
  assert degraded.reason == "current_lane_unknown"
