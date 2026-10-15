#!/usr/bin/env python3

from __future__ import annotations

from collections import Counter
import json
import logging
from logging.handlers import RotatingFileHandler
import os
from pathlib import Path
import time
from typing import Any

import openpilot.cereal.messaging as messaging
from openpilot.common.hardware import PC
from openpilot.common.params import Params
from openpilot.sunnypilot.selfdrive.traffic_control import TRAFFIC_SIGNAL_CONTROL_PARAM
from openpilot.sunnypilot.selfdrive.traffic_control.tesla_observer import (
  TRAFFIC_CONTROL_ADDRESS,
  TRAFFIC_CONTROL_BUSES,
  TRAFFIC_CONTROL_MIN_DLC,
  TeslaTrafficControlObserver,
)


LOG_SCHEMA_VERSION = 1
LOG_FILENAME = "traffic-control.jsonl"
LOG_MAX_BYTES = 10 * 1024 * 1024
LOG_BACKUP_COUNT = 4
HEARTBEAT_NS = 2_000_000_000
PARAM_REFRESH_NS = 2_000_000_000


def default_log_root() -> Path:
  configured = os.environ.get("TRAFFIC_CONTROL_LOG_ROOT")
  if configured:
    return Path(configured)
  return Path("/tmp/traffic-control" if PC else "/data/media/0/traffic-control")


class JsonlRotatingWriter:
  def __init__(self, root: str | Path, *, max_bytes: int = LOG_MAX_BYTES,
               backup_count: int = LOG_BACKUP_COUNT) -> None:
    self.root = Path(root)
    self.root.mkdir(parents=True, exist_ok=True)
    self.path = self.root / LOG_FILENAME
    self._logger = logging.getLogger(f"traffic-control-jsonl-{os.getpid()}-{id(self)}")
    self._logger.setLevel(logging.INFO)
    self._logger.propagate = False
    self._handler = RotatingFileHandler(
      self.path, maxBytes=max_bytes, backupCount=backup_count, encoding="utf-8",
    )
    self._handler.setFormatter(logging.Formatter("%(message)s"))
    self._logger.addHandler(self._handler)

  def write(self, record: dict[str, Any]) -> None:
    payload = dict(record)
    payload.setdefault("schema", LOG_SCHEMA_VERSION)
    payload.setdefault("wall_time_ns", time.time_ns())
    payload.setdefault("mono_time_ns", time.monotonic_ns())
    self._logger.info(json.dumps(payload, ensure_ascii=False, separators=(",", ":"), sort_keys=True))

  def close(self) -> None:
    self._logger.removeHandler(self._handler)
    self._handler.close()


class RawTrafficCanTracker:
  def __init__(self) -> None:
    self.total_frames = 0
    self.accepted_frames = 0
    self.bad_dlc_frames = 0
    self.frames_by_bus: Counter[int] = Counter()
    self.last_seen_mono_ns = 0
    self.last_bus = -1
    self.last_dlc = 0
    self.last_payload = b""

  def observe(self, mono_time_ns: int, address: int, data: bytes, bus: int) -> dict[str, Any]:
    payload = bytes(data)
    self.total_frames += 1
    self.frames_by_bus[int(bus)] += 1
    self.last_seen_mono_ns = int(mono_time_ns)
    self.last_bus = int(bus)
    self.last_dlc = len(payload)
    self.last_payload = payload

    decoded = TeslaTrafficControlObserver._decode(payload)
    if decoded is None:
      self.bad_dlc_frames += 1
      classification = "invalid_dlc"
    elif bus not in TRAFFIC_CONTROL_BUSES:
      classification = "unexpected_bus"
    else:
      self.accepted_frames += 1
      classification = "accepted"

    return {
      "event": "raw_0x25d",
      "mono_time_ns": int(mono_time_ns),
      "address": hex(int(address)),
      "bus": int(bus),
      "dlc": len(payload),
      "payload": payload.hex(),
      "classification": classification,
      "decoded": decoded,
      "raw_total": self.total_frames,
      "accepted_total": self.accepted_frames,
      "bad_dlc_total": self.bad_dlc_frames,
      "frames_by_bus": dict(sorted(self.frames_by_bus.items())),
    }

  def summary(self, now_ns: int) -> dict[str, Any]:
    age_ms = ((now_ns - self.last_seen_mono_ns) / 1e6
              if self.last_seen_mono_ns > 0 and now_ns >= self.last_seen_mono_ns else None)
    return {
      "total": self.total_frames,
      "accepted": self.accepted_frames,
      "bad_dlc": self.bad_dlc_frames,
      "frames_by_bus": dict(sorted(self.frames_by_bus.items())),
      "last_bus": self.last_bus,
      "last_dlc": self.last_dlc,
      "last_payload": self.last_payload.hex(),
      "last_seen_mono_ns": self.last_seen_mono_ns,
      "last_seen_age_ms": age_ms,
    }


def diagnose_blockers(values: dict[str, Any]) -> list[str]:
  if not values.get("config_enabled", False):
    return ["control_disabled"]
  if int(values.get("raw_total", 0)) == 0:
    return ["no_0x25d_seen"]

  blockers: list[str] = []
  raw_bus = int(values.get("raw_bus", -1))
  raw_dlc = int(values.get("raw_dlc", 0))
  if not values.get("raw_available", False):
    # The raw tracker intentionally records every same-address frame, including
    # bus-128 echoes and unrelated bus-1 payloads. They are only blockers when
    # the production observer has no fresh accepted bus-2 observation.
    if raw_bus not in TRAFFIC_CONTROL_BUSES:
      blockers.append("unexpected_bus")
    if not TRAFFIC_CONTROL_MIN_DLC <= raw_dlc <= 8:
      blockers.append("invalid_dlc")
    blockers.append("raw_observation_stale")
  elif not values.get("raw_valid", False):
    blockers.append("raw_not_control_eligible")
  if not values.get("enabled", False):
    blockers.append("controls_not_enabled")
  elif not values.get("long_active", False):
    blockers.append("longitudinal_not_active")
  if values.get("gas_pressed", False):
    blockers.append("driver_gas_override")
  if int(values.get("transition_reason", 0)) == 11:
    blockers.append("speed_above_limit")
    return blockers
  if values.get("target_present", False):
    if not values.get("stop_control_allowed", False):
      blockers.append("stop_control_not_allowed")
    elif not values.get("plan_applied", False):
      blockers.append("final_plan_not_applied")
  elif (values.get("raw_available", False) and values.get("raw_valid", False)
        and values.get("enabled", False) and values.get("long_active", False)):
    blockers.append("no_confirmed_target")
  return blockers


def _service_status(sm, name: str) -> dict[str, bool]:
  return {
    "seen": bool(sm.seen[name]),
    "alive": bool(sm.alive[name]),
    "valid": bool(sm.valid[name]),
  }


def _enum_value(value) -> int:
  return int(getattr(value, "raw", value))


def _raw_snapshot(sm) -> dict[str, Any]:
  target = sm["carStateSP"].teslaTrafficControl
  return {
    "available": bool(target.available),
    "valid_for_control": bool(target.validForControl),
    "source_bus": int(target.sourceBus),
    "dlc": int(target.dlc),
    "feature_state": int(target.featureState),
    "state_machine": int(target.stateMachine),
    "control_source": int(target.controlSource),
    "control_type": int(target.controlType),
    "distance": float(target.distance),
    "light_state": int(target.lightState),
    "continuation_reason": int(target.continuationReason),
    "confirmation_type": int(target.confirmationType),
    "warning_suppression_reason": int(target.warningSuppressionReason),
    "unavailable_reason": int(target.unavailableReason),
    "vision_light": bool(target.visionLight),
    "vision_sign": bool(target.visionSign),
    "vision_road_marking": bool(target.visionRoadMarking),
    "vision_line": bool(target.visionLine),
    "frame_mono_time": int(target.frameMonoTime),
    "quality": int(target.quality),
    "raw_address": hex(int(target.rawAddress)),
    "raw_payload": bytes(target.rawPayload).hex(),
  }


def _state_snapshot(sm) -> dict[str, Any]:
  target = sm["trafficRadarState"]
  return {
    "mode": int(target.mode),
    "phase": int(target.phase),
    "light_state": int(target.lightState),
    "source_bus": int(target.sourceBus),
    "quality": int(target.quality),
    "target_present": bool(target.targetPresent),
    "confidence": float(target.confidence),
    "distance_to_stop": float(target.distanceToStopPoint),
    "raw_distance": float(target.rawDistance),
    "control_allowed": bool(target.controlAllowed),
    "stop_control_allowed": bool(target.stopControlAllowed),
    "stop_safety_allowed": bool(target.stopSafetyAllowed),
    "raw_observation_fresh": bool(target.rawObservationFresh),
    "observation_age_ms": float(target.observationAgeMs),
    "should_stop": bool(target.shouldStop),
    "planner_start_requested": bool(target.plannerStartRequested),
    "release_eligible": bool(target.releaseEligible),
    "driver_override_active": bool(target.driverOverrideActive),
    "event_id": int(target.eventId),
    "stop_session_id": int(target.stopSessionId),
    "transition_reason": int(target.eventTransitionReason),
    "transition_seq": int(target.eventTransitionSeq),
    "publish_mono_time": int(target.publishMonoTime),
  }


def _plan_snapshot(sm) -> dict[str, Any]:
  target = sm["longitudinalPlanSP"].teslaTrafficControl
  return {
    "mode": int(target.mode),
    "phase": int(target.phase),
    "active": bool(target.active),
    "applied": bool(target.applied),
    "should_stop": bool(target.shouldStop),
    "action": int(target.action),
    "light_state": int(target.lightState),
    "remaining_distance": float(target.remainingDistance),
    "raw_distance": float(target.rawDistance),
    "constraint_accel": float(target.constraintAccel),
    "base_a_target": float(target.baseATarget),
    "final_a_target": float(target.finalATarget),
    "start_requested": bool(target.startRequested),
    "start_applied": bool(target.startApplied),
    "start_block_reason": int(target.startBlockReason),
    "event_id": int(target.eventId),
    "stop_session_id": int(target.stopSessionId),
    "driver_override_active": bool(target.driverOverrideActive),
    "stop_control_allowed": bool(target.stopControlAllowed),
    "stop_safety_allowed": bool(target.stopSafetyAllowed),
    "raw_observation_fresh": bool(target.rawObservationFresh),
    "raw_observation_age_ms": float(target.rawObservationAgeMs),
  }


def _vehicle_snapshot(sm) -> dict[str, Any]:
  state = sm["carState"]
  control = sm["carControl"]
  return {
    "v_ego": float(state.vEgo),
    "a_ego": float(state.aEgo),
    "gas_pressed": bool(state.gasPressed),
    "brake_pressed": bool(state.brakePressed),
    "enabled": bool(control.enabled),
    "long_active": bool(control.longActive),
    "lat_active": bool(control.latActive),
  }


def _read_config(params: Params) -> dict[str, Any]:
  def integer(key: str, fallback: int) -> int:
    value = params.get(key, return_default=True)
    try:
      return int(value)
    except (TypeError, ValueError):
      return fallback

  return {
    "enabled": params.get_bool(TRAFFIC_SIGNAL_CONTROL_PARAM),
    "stop_reference_dm": integer("TeslaTrafficStopReference", 50),
    "max_speed_kph": integer("TeslaTrafficControlMaxSpeed", 60),
  }


def build_state_record(sm, tracker: RawTrafficCanTracker, config: dict[str, Any], now_ns: int,
                       event: str) -> dict[str, Any]:
  services = {
    name: _service_status(sm, name)
    for name in ("carStateSP", "trafficRadarState", "longitudinalPlanSP", "carState", "carControl")
  }
  raw = _raw_snapshot(sm)
  state = _state_snapshot(sm)
  plan = _plan_snapshot(sm)
  vehicle = _vehicle_snapshot(sm)
  tracker_summary = tracker.summary(now_ns)
  blockers = diagnose_blockers({
    "config_enabled": config["enabled"],
    "raw_total": tracker.total_frames,
    "raw_bus": tracker.last_bus,
    "raw_dlc": tracker.last_dlc,
    "raw_available": raw["available"],
    "raw_valid": raw["valid_for_control"],
    "enabled": vehicle["enabled"],
    "long_active": vehicle["long_active"],
    "gas_pressed": vehicle["gas_pressed"],
    "transition_reason": state["transition_reason"],
    "target_present": state["target_present"],
    "stop_control_allowed": state["stop_control_allowed"],
    "plan_applied": plan["applied"],
  })
  return {
    "event": event,
    "mono_time_ns": int(now_ns),
    "config": config,
    "services": services,
    "raw_can_tracker": tracker_summary,
    "raw_observation": raw,
    "controller_state": state,
    "final_plan": plan,
    "vehicle": vehicle,
    "blockers": blockers,
  }


def _state_signature(record: dict[str, Any]) -> tuple:
  raw = record["raw_observation"]
  state = record["controller_state"]
  plan = record["final_plan"]
  vehicle = record["vehicle"]
  services = record["services"]
  return (
    record["config"]["enabled"], tuple(record["blockers"]),
    tuple((name, value["seen"], value["alive"], value["valid"]) for name, value in services.items()),
    raw["available"], raw["valid_for_control"], raw["source_bus"], raw["dlc"], raw["control_type"],
    raw["light_state"], raw["quality"],
    state["mode"], state["phase"], state["target_present"], state["stop_control_allowed"],
    state["event_id"], state["stop_session_id"], state["transition_seq"], state["transition_reason"],
    plan["applied"], plan["action"], plan["start_requested"], plan["start_applied"],
    plan["start_block_reason"], vehicle["enabled"], vehicle["long_active"], vehicle["gas_pressed"],
  )


def main() -> None:
  requested_root = default_log_root()
  try:
    writer = JsonlRotatingWriter(requested_root)
  except OSError:
    fallback_root = Path("/tmp/traffic-control")
    writer = JsonlRotatingWriter(fallback_root)

  tracker = RawTrafficCanTracker()
  params = Params()
  config = _read_config(params)
  can_sock = messaging.sub_sock("can", conflate=False, timeout=100)
  services = ["carStateSP", "trafficRadarState", "longitudinalPlanSP", "carState", "carControl"]
  sm = messaging.SubMaster(services)
  last_signature = None
  last_record_ns = 0
  next_param_refresh_ns = 0

  writer.write({
    "event": "session_start",
    "pid": os.getpid(),
    "requested_log_root": str(requested_root),
    "active_log_root": str(writer.root),
    "log_file": str(writer.path),
  })

  try:
    while True:
      can_messages = messaging.drain_sock(can_sock, wait_for_one=True)
      for message in can_messages:
        mono_time_ns = int(message.logMonoTime)
        for frame in message.can:
          if int(frame.address) == TRAFFIC_CONTROL_ADDRESS:
            writer.write(tracker.observe(mono_time_ns, int(frame.address), bytes(frame.dat), int(frame.src)))

      sm.update(0)
      now_ns = time.monotonic_ns()
      if now_ns >= next_param_refresh_ns:
        config = _read_config(params)
        next_param_refresh_ns = now_ns + PARAM_REFRESH_NS

      record = build_state_record(sm, tracker, config, now_ns, "state_change")
      signature = _state_signature(record)
      if signature != last_signature:
        writer.write(record)
        last_signature = signature
        last_record_ns = now_ns
      elif now_ns - last_record_ns >= HEARTBEAT_NS:
        record["event"] = "heartbeat"
        writer.write(record)
        last_record_ns = now_ns
  except KeyboardInterrupt:
    pass
  finally:
    writer.write({"event": "session_end", "pid": os.getpid()})
    writer.close()


if __name__ == "__main__":
  main()
