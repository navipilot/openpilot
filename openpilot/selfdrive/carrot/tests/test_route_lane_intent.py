from openpilot.selfdrive.carrot.route_lane_intent import parse_route_lane_intent


NOW_MS = 1_700_000_000_000


def valid_intent(**overrides):
  intent = {
    "schema_version": 1,
    "route_generation": 7,
    "road": {
      "controlled_access": True,
      "confidence": 0.8,
    },
    "maneuver": {
      "type": "exit_right",
      "distance_m": 1_200,
    },
    "lanes": {
      "lane_count": 4,
      "index_order": "left_to_right",
      "current_lane_hint": 1,
      "preferred_lane_mask": 0b1000,
      "lanes": [{}, {}, {}, {}],
    },
    "freshness": {
      "sequence": 3,
      "source_timestamp_ms": NOW_MS - 100,
    },
  }
  intent.update(overrides)
  return intent


def test_parses_fresh_controlled_access_intent_for_diagnostics():
  parsed = parse_route_lane_intent(valid_intent(), NOW_MS)

  assert parsed.valid is True
  assert parsed.route_generation == 7
  assert parsed.maneuver_type == "exit_right"
  assert parsed.required_lane_changes_hint == 2
  assert parsed.next_lane_change_direction == "right"
  assert parsed.reject_reason == ""


def test_rejects_stale_or_future_intents():
  stale = valid_intent(freshness={"source_timestamp_ms": NOW_MS - 5_001})
  future = valid_intent(freshness={"source_timestamp_ms": NOW_MS + 1_001})

  assert parse_route_lane_intent(stale, NOW_MS).reject_reason == "stale"
  assert parse_route_lane_intent(future, NOW_MS).reject_reason == "future_timestamp"


def test_rejects_non_controlled_access_road_and_invalid_lane_mask():
  city = valid_intent(road={"controlled_access": False})
  invalid_mask = valid_intent(lanes={
    "lane_count": 4,
    "index_order": "left_to_right",
    "current_lane_hint": 1,
    "preferred_lane_mask": 0b1_0000,
    "lanes": [{}, {}, {}, {}],
  })

  assert parse_route_lane_intent(city, NOW_MS).reject_reason == "unsupported_road"
  assert parse_route_lane_intent(invalid_mask, NOW_MS).reject_reason == "invalid_preferred_lanes"


def test_requires_preferred_lane_for_exit_and_preserves_advisory_only_state():
  no_preferred_lane = valid_intent(lanes={
    "lane_count": 4,
    "index_order": "left_to_right",
    "current_lane_hint": -1,
    "preferred_lane_mask": 0,
    "lanes": [{}, {}, {}, {}],
  })

  parsed = parse_route_lane_intent(no_preferred_lane, NOW_MS)

  assert parsed.valid is False
  assert parsed.reject_reason == "missing_preferred_lanes"
