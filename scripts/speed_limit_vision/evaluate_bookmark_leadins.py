#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import re

from dataclasses import dataclass
from datetime import datetime
from pathlib import Path

import cv2

import starpilot.system.speed_limit_vision as slv

from scripts.speed_limit_vision import common


DEFAULT_SESSION_ROOT = Path(".tmp/live_drive_debug")
DEFAULT_CLIP_ROOT = common.preferred_clip_root()
DEFAULT_QLOG_MTIMES = common.preferred_qlog_mtimes_path()
EVENT_TYPES = ("candidate", "publish", "stale_clear")


@dataclass(frozen=True)
class BookmarkWindow:
  bookmark_number: int
  route: str
  segment: int
  segment_offset_s: float
  leadin_start_s: float
  spans_previous_segment: bool


class LiveReplayDaemon(slv.SpeedLimitVisionDaemon):
  def __init__(self):
    super().__init__(use_runtime=False)
    self.now = 0.0
    self.captured_events: list[dict] = []

  def _write_debug_event(self, event_type, frame_bgr=None, snapshot_prefix=None, **fields):
    if event_type not in EVENT_TYPES:
      return
    record = {"event": event_type, "t": round(self.now, 3)}
    record.update(fields)
    self.captured_events.append(record)

  def _publish_status(self, status, clear_speed=False):
    if clear_speed:
      self._clear_detection()

  def process_frame(self, now, frame_bgr):
    self.now = now
    slv.time.monotonic = lambda now=now: now
    self.current_frame_bgr = frame_bgr

    inference_interval = slv.FOLLOWUP_INFERENCE_INTERVAL if now < self.followup_until else slv.INFERENCE_INTERVAL
    if now - self.last_inference_at < inference_interval:
      if self.published_speed_limit_mph > 0 and self._published_detection_stale(now):
        self._write_debug_event("stale_clear", reason="hold_timeout")
        self._publish_status("Scanning replay", clear_speed=True)
      return

    self.last_inference_at = now
    detection = self._detect_sign(frame_bgr)
    if detection is not None:
      self._update_detection(detection)
    elif self.published_speed_limit_mph > 0 and self._published_detection_stale(now):
      self._write_debug_event("stale_clear", reason="no_detection")
      self._publish_status("Scanning replay", clear_speed=True)


def parse_args():
  parser = argparse.ArgumentParser(description="Replay 5-second pre-bookmark sign windows through the live speed-limit vision path.")
  parser.add_argument("--session-root", type=Path, default=DEFAULT_SESSION_ROOT, help="Directory containing debug session folders.")
  parser.add_argument("--clip-root", type=Path, default=DEFAULT_CLIP_ROOT, help="Directory containing copied route clips under <route>--<seg>/fcamera.hevc.")
  parser.add_argument("--qlog-mtimes", type=Path, default=DEFAULT_QLOG_MTIMES, help="Text file with '<qlog path> <mtime epoch>' lines.")
  parser.add_argument("--session-route-map", type=Path, default=common.preferred_session_route_map_path(), help="JSON file mapping debug session ids to route log ids.")
  parser.add_argument("--models-dir", type=Path, help="Directory containing speed_limit_us_detector.onnx and speed_limit_us_value_classifier.onnx.")
  parser.add_argument("--lead-in", type=float, default=5.0, help="Seconds before each bookmark to replay.")
  parser.add_argument("--sample-fps", type=float, help="Optional decode sample rate. Use 5 for faster bookmark sweeps that still match the live inference cadence.")
  parser.add_argument("--session", action="append", help="Optional session id filter. Repeat to run more than one.")
  parser.add_argument("--bookmark", action="append", type=int, help="Optional bookmark number filter within the selected sessions.")
  parser.add_argument("--json-out", type=Path, help="Optional path to write the summary JSON.")
  return parser.parse_args()


def load_qlog_mtimes(path: Path):
  route_mtimes: dict[str, dict[int, int]] = {}
  pattern = re.compile(r"/([^/]+)--(\d+)/(qlog(?:\.(?:zst|bz2))?)$")
  for line in path.read_text(encoding="utf-8").splitlines():
    line = line.strip()
    if not line:
      continue
    qlog_path, timestamp = line.rsplit(" ", 1)
    match = pattern.search(qlog_path)
    if match is None:
      continue
    route, segment = match.group(1), int(match.group(2))
    route_mtimes.setdefault(route, {})[segment] = int(timestamp)
  return route_mtimes


def load_bookmarks(session_path: Path):
  events = []
  with (session_path / "events.jsonl").open("r", encoding="utf-8") as handle:
    for line in handle:
      line = line.strip()
      if not line:
        continue
      event = json.loads(line)
      if event.get("event") in ("bookmark", "auto_bookmark"):
        events.append(event)
  return events


def locate_window(route: str, event: dict, route_mtimes: dict[str, dict[int, int]], lead_in: float):
  event_wall_time = datetime.fromisoformat(event["wallTime"]).timestamp()
  segment_mtimes = route_mtimes.get(route, {})
  for segment, start_epoch in sorted(segment_mtimes.items()):
    if start_epoch <= event_wall_time < start_epoch + 60:
      offset_s = event_wall_time - start_epoch
      return BookmarkWindow(
        bookmark_number=0,
        route=route,
        segment=segment,
        segment_offset_s=offset_s,
        leadin_start_s=offset_s - lead_in,
        spans_previous_segment=offset_s - lead_in < 0.0,
      )
  return None


def iter_video_window(path: Path, start_s: float, end_s: float, sample_fps: float | None = None):
  capture = cv2.VideoCapture(str(path))
  fps = capture.get(cv2.CAP_PROP_FPS) or 20.0
  start_frame = max(int(start_s * fps), 0)
  end_frame = max(int(end_s * fps), start_frame)
  frame_step = 1
  if sample_fps is not None and sample_fps > 0.0 and sample_fps < fps:
    frame_step = max(int(round(fps / sample_fps)), 1)
  capture.set(cv2.CAP_PROP_POS_FRAMES, start_frame)
  frame_index = start_frame

  while frame_index <= end_frame:
    ok, frame_bgr = capture.read()
    if not ok:
      break
    frame_time_s = frame_index / fps
    if start_s <= frame_time_s <= end_s:
      yield frame_time_s, frame_bgr
    frame_index += 1

    skipped = 1
    while skipped < frame_step and frame_index <= end_frame:
      ok = capture.grab()
      if not ok:
        capture.release()
        return
      frame_index += 1
      skipped += 1

  capture.release()


def replay_window(window: BookmarkWindow, clip_root: Path, sample_fps: float | None = None):
  daemon = LiveReplayDaemon()
  elapsed_base_s = 0.0
  replayed_frames = 0
  segments: list[tuple[Path, float, float]] = []

  if window.spans_previous_segment:
    previous_segment = window.segment - 1
    if previous_segment >= 0:
      previous_clip = clip_root / f"{window.route}--{previous_segment}" / "fcamera.hevc"
      previous_start_s = 60.0 + window.leadin_start_s
      segments.append((previous_clip, max(previous_start_s, 0.0), 60.0))
      elapsed_base_s += max(60.0 - max(previous_start_s, 0.0), 0.0)
    current_start_s = 0.0
  else:
    current_start_s = window.leadin_start_s

  current_clip = clip_root / f"{window.route}--{window.segment}" / "fcamera.hevc"
  segments.append((current_clip, max(current_start_s, 0.0), window.segment_offset_s))

  cumulative_offset_s = 0.0
  for clip_path, start_s, end_s in segments:
    if not clip_path.is_file():
      return {
        "missingClip": str(clip_path),
        "events": [],
        "replayedFrames": replayed_frames,
      }

    for frame_time_s, frame_bgr in iter_video_window(clip_path, start_s, end_s, sample_fps=sample_fps):
      replay_time_s = cumulative_offset_s + (frame_time_s - start_s)
      daemon.process_frame(replay_time_s, frame_bgr)
      replayed_frames += 1

    cumulative_offset_s += max(end_s - start_s, 0.0)

  candidate_values = [event["candidateSpeedLimitMph"] for event in daemon.captured_events if event["event"] == "candidate"]
  published_values = [event["speedLimitMph"] for event in daemon.captured_events if event["event"] == "publish"]
  return {
    "events": daemon.captured_events,
    "candidateValues": candidate_values,
    "publishedValues": published_values,
    "replayedFrames": replayed_frames,
    "hit": bool(candidate_values or published_values),
  }


def main():
  args = parse_args()
  selected_sessions = set(args.session or [])
  selected_bookmarks = set(args.bookmark or [])
  route_mtimes = load_qlog_mtimes(args.qlog_mtimes.expanduser().resolve())
  session_route_map = common.load_session_route_map(args.session_route_map)
  if not session_route_map:
    raise FileNotFoundError(f"No session route map found at {args.session_route_map}")

  if args.models_dir:
    models_dir = args.models_dir.expanduser().resolve()
    detector_path = models_dir / "speed_limit_us_detector.onnx"
    classifier_path = models_dir / "speed_limit_us_value_classifier.onnx"
    if not detector_path.is_file():
      raise FileNotFoundError(detector_path)
    if not classifier_path.is_file():
      raise FileNotFoundError(classifier_path)
    slv.US_DETECTOR_MODEL_PATH = detector_path
    slv.US_CLASSIFIER_MODEL_PATH = classifier_path

  summaries = []
  for session_id, route in session_route_map.items():
    if selected_sessions and session_id not in selected_sessions:
      continue

    session_path = args.session_root.expanduser().resolve() / session_id
    if not session_path.is_dir():
      continue

    bookmarks = load_bookmarks(session_path)
    for bookmark_number, event in enumerate(bookmarks, start=1):
      if selected_bookmarks and bookmark_number not in selected_bookmarks:
        continue

      window = locate_window(route, event, route_mtimes, args.lead_in)
      if window is None:
        summary = {
          "sessionId": session_id,
          "bookmarkNumber": bookmark_number,
          "route": route,
          "status": "unmapped",
        }
      else:
        window = BookmarkWindow(
          bookmark_number=bookmark_number,
          route=window.route,
          segment=window.segment,
          segment_offset_s=window.segment_offset_s,
          leadin_start_s=window.leadin_start_s,
          spans_previous_segment=window.spans_previous_segment,
        )
        replay_summary = replay_window(window, args.clip_root.expanduser().resolve(), sample_fps=args.sample_fps)
        summary = {
          "sessionId": session_id,
          "bookmarkNumber": bookmark_number,
          "route": route,
          "segment": window.segment,
          "segmentOffsetS": round(window.segment_offset_s, 3),
          "leadinStartS": round(window.leadin_start_s, 3),
          "spansPreviousSegment": window.spans_previous_segment,
        }
        summary.update(replay_summary)

      summaries.append(summary)

  hit_count = sum(1 for summary in summaries if summary.get("hit"))
  print(f"Bookmarks with detections in lead-in: {hit_count}/{len(summaries)}")
  for summary in summaries:
    if summary.get("status") == "unmapped":
      print(f"{summary['sessionId']} bookmark {summary['bookmarkNumber']:02d}: unmapped")
      continue

    result = "hit" if summary.get("hit") else "miss"
    publish_values = ",".join(str(value) for value in summary.get("publishedValues", [])) or "-"
    candidate_values = ",".join(str(value) for value in summary.get("candidateValues", [])) or "-"
    note = ""
    if summary.get("missingClip"):
      note = f" missing={summary['missingClip']}"
    print(
      f"{summary['sessionId']} bookmark {summary['bookmarkNumber']:02d}: "
      f"seg {summary['segment']} @ {summary['segmentOffsetS']:.2f}s "
      f"lead-in [{summary['leadinStartS']:.2f}s, {summary['segmentOffsetS']:.2f}s] "
      f"{result} publish={publish_values} candidate={candidate_values}{note}"
    )

  if args.json_out:
    args.json_out.expanduser().resolve().write_text(json.dumps(summaries, indent=2), encoding="utf-8")


if __name__ == "__main__":
  main()
