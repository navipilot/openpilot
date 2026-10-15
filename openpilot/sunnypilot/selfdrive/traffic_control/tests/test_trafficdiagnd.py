import json

import openpilot.cereal.messaging as messaging
from openpilot.sunnypilot.selfdrive.traffic_control.trafficdiagnd import (
  JsonlRotatingWriter,
  RawTrafficCanTracker,
  _read_config,
  build_state_record,
  diagnose_blockers,
)


def test_jsonl_writer_rotates_and_keeps_valid_records(tmp_path):
  writer = JsonlRotatingWriter(tmp_path, max_bytes=320, backup_count=2)
  for sequence in range(30):
    writer.write({"event": "heartbeat", "sequence": sequence, "payload": "x" * 48})
  writer.close()

  files = sorted(tmp_path.glob("traffic-control.jsonl*"))
  assert 1 < len(files) <= 3
  records = [json.loads(line) for path in files for line in path.read_text().splitlines()]
  assert records
  assert all(record["schema"] == 1 for record in records)
  assert any(record["sequence"] == 29 for record in records)


def test_raw_tracker_records_wrong_bus_and_malformed_dlc():
  tracker = RawTrafficCanTracker()

  wrong_bus = tracker.observe(1_000, 0x25D, bytes.fromhex("0018000100000000"), 1)
  assert wrong_bus["classification"] == "unexpected_bus"
  assert wrong_bus["bus"] == 1
  assert tracker.total_frames == 1
  assert tracker.frames_by_bus == {1: 1}

  malformed = tracker.observe(2_000, 0x25D, b"\x00" * 4, 2)
  assert malformed["classification"] == "invalid_dlc"
  assert tracker.bad_dlc_frames == 1
  assert tracker.frames_by_bus == {1: 1, 2: 1}


def test_blockers_identify_the_stage_that_prevents_control():
  assert diagnose_blockers({"config_enabled": False}) == ["control_disabled"]

  no_can = diagnose_blockers({"config_enabled": True, "raw_total": 0})
  assert no_can == ["no_0x25d_seen"]

  wrong_bus = diagnose_blockers({
    "config_enabled": True, "raw_total": 5, "raw_bus": 1, "raw_dlc": 8,
    "raw_available": False,
  })
  assert "unexpected_bus" in wrong_bus

  mirrored_bus = diagnose_blockers({
    "config_enabled": True, "raw_total": 5, "raw_bus": 128, "raw_dlc": 6,
    "raw_available": True, "raw_valid": True, "enabled": True, "long_active": True,
    "target_present": False, "plan_applied": False,
  })
  assert "unexpected_bus" not in mirrored_bus

  gated = diagnose_blockers({
    "config_enabled": True, "raw_total": 5, "raw_bus": 2, "raw_dlc": 8,
    "raw_available": True, "raw_valid": True, "enabled": True, "long_active": True,
    "transition_reason": 11, "target_present": False, "plan_applied": False,
  })
  assert gated == ["speed_above_limit"]

  final_plan = diagnose_blockers({
    "config_enabled": True, "raw_total": 5, "raw_bus": 2, "raw_dlc": 8,
    "raw_available": True, "raw_valid": True, "enabled": True, "long_active": True,
    "transition_reason": 1, "target_present": True, "stop_control_allowed": True,
    "plan_applied": False,
  })
  assert final_plan == ["final_plan_not_applied"]

  active = diagnose_blockers({
    "config_enabled": True, "raw_total": 5, "raw_bus": 2, "raw_dlc": 8,
    "raw_available": True, "raw_valid": True, "enabled": True, "long_active": True,
    "target_present": True, "stop_control_allowed": True, "plan_applied": True,
  })
  assert active == []


def test_config_values_are_json_serializable_integers():
  class FakeParams:
    def get_bool(self, key):
      assert key == "TeslaTrafficSignalControlEnabled"
      return True

    def get(self, key, return_default=False):
      assert return_default
      return {"TeslaTrafficStopReference": b"55", "TeslaTrafficControlMaxSpeed": b"65"}[key]

  config = _read_config(FakeParams())
  assert config == {"enabled": True, "stop_reference_dm": 55, "max_speed_kph": 65}
  json.dumps(config)


def test_state_record_matches_live_cereal_schema_and_is_json_serializable():
  services = ("carStateSP", "trafficRadarState", "longitudinalPlanSP", "carState", "carControl")

  class FakeSubMaster:
    def __init__(self):
      self.data = {name: getattr(messaging.new_message(name), name) for name in services}
      self.seen = dict.fromkeys(services, True)
      self.alive = dict.fromkeys(services, True)
      self.valid = dict.fromkeys(services, True)

    def __getitem__(self, name):
      return self.data[name]

  sm = FakeSubMaster()
  tracker = RawTrafficCanTracker()
  tracker.observe(1_000_000_000, 0x25D, bytes.fromhex("0018500100000000"), 2)
  record = build_state_record(
    sm, tracker,
    {"enabled": True, "stop_reference_dm": 50, "max_speed_kph": 60},
    1_100_000_000, "heartbeat",
  )

  assert record["event"] == "heartbeat"
  assert record["raw_can_tracker"]["total"] == 1
  assert set(record) >= {"services", "raw_observation", "controller_state", "final_plan", "vehicle", "blockers"}
  json.dumps(record)
