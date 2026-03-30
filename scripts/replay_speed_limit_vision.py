#!/usr/bin/env python3
from __future__ import annotations

import argparse
from pathlib import Path

import cv2

import starpilot.system.speed_limit_vision as slv


class ReplayDaemon(slv.SpeedLimitVisionDaemon):
  def __init__(self):
    super().__init__(use_runtime=False)
    self.now = 0.0

  def _write_debug_event(self, event_type, frame_bgr=None, snapshot_prefix=None, **fields):
    if event_type in ("candidate", "publish", "stale_clear"):
      print(f"t={self.now:6.2f}s {event_type:12} {fields}")

  def _publish_status(self, status, clear_speed=False):
    if clear_speed:
      self._clear_detection()

  def _publish_detection(self, speed_limit_mph, confidence, status_prefix):
    super()._publish_detection(speed_limit_mph, confidence, status_prefix)

  def process_frame(self, now, frame_bgr):
    self.now = now
    slv.time.monotonic = lambda now=now: now
    self.current_frame_bgr = frame_bgr

    detection = self._detect_sign(frame_bgr)
    if detection is not None:
      self._update_detection(detection)
    elif self.published_speed_limit_mph > 0 and self._published_detection_stale(now):
      print(f"t={self.now:6.2f}s stale_clear   {{'reason': 'no_detection'}}")
      self._clear_detection()


def iter_directory_frames(path: Path, fps: float):
  for index, frame_path in enumerate(sorted(path.glob("frame_*.png")), start=1):
    frame = cv2.imread(str(frame_path))
    if frame is None:
      continue
    yield (index - 1) / fps, frame


def iter_video_frames(path: Path):
  cap = cv2.VideoCapture(str(path))
  fps = cap.get(cv2.CAP_PROP_FPS) or 25.0
  frame_index = 0
  while True:
    ok, frame = cap.read()
    if not ok:
      break
    yield frame_index / fps, frame
    frame_index += 1
  cap.release()


def main():
  parser = argparse.ArgumentParser(description="Replay StarPilot speed-limit vision on saved video or extracted frames.")
  parser.add_argument("path", help="Path to an fcamera.hevc file or a directory of frame_XXX.png images.")
  parser.add_argument("--frames-fps", type=float, default=5.0, help="FPS to assume when replaying an extracted frame directory.")
  parser.add_argument("--start", type=float, default=0.0, help="Skip frames before this timestamp in seconds.")
  parser.add_argument("--end", type=float, default=None, help="Stop once this timestamp in seconds is exceeded.")
  args = parser.parse_args()

  path = Path(args.path)
  if not path.exists():
    raise FileNotFoundError(path)

  daemon = ReplayDaemon()
  frame_iter = iter_directory_frames(path, max(args.frames_fps, 0.1)) if path.is_dir() else iter_video_frames(path)
  for now, frame_bgr in frame_iter:
    if now < args.start:
      continue
    if args.end is not None and now > args.end:
      break
    daemon.process_frame(now, frame_bgr)


if __name__ == "__main__":
  main()
