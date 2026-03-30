#!/usr/bin/env python3
from __future__ import annotations

import argparse

from dataclasses import dataclass
from pathlib import Path

import cv2

import starpilot.system.speed_limit_vision as slv


@dataclass(frozen=True)
class RuntimeCase:
  name: str
  frame_path: str
  expected_speed_limit_mph: int


DEFAULT_CASES = (
  RuntimeCase("live15", ".tmp/live_c4_capture/stopped_sign_road.jpg", 15),
  RuntimeCase("school20", ".tmp/route_vision/seg38_frames/frame_041.jpg", 20),
  RuntimeCase("highway40", ".tmp/speed_route_frames_seg2_10_20/t12.png", 40),
  RuntimeCase("town40", ".tmp/route_12c_seg9_10/seg10_early/frame_005.png", 40),
  RuntimeCase("town30", ".tmp/route_12c_seg9_10/seg10_early/frame_012.png", 30),
  RuntimeCase("town30_late", ".tmp/vision_iter/seg10_5fps/frame_054.png", 30),
  RuntimeCase("town30_late_earlier", ".tmp/vision_iter/seg10_5fps/frame_052.png", 30),
)


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Evaluate the StarPilot speed-limit runtime path on known saved-frame cases.")
  parser.add_argument(
    "--models-dir",
    type=Path,
    default=Path("starpilot/assets/vision_models"),
    help="Directory containing speed_limit_us_detector.onnx and speed_limit_us_value_classifier.onnx.",
  )
  parser.add_argument(
    "--case",
    action="append",
    dest="selected_cases",
    help="Optional case name filter. Repeat to run more than one case.",
  )
  parser.add_argument("--strict", action="store_true", help="Exit non-zero if any evaluated case misses the expected value.")
  return parser.parse_args()


def resolve_cases(selected_cases: list[str] | None):
  if not selected_cases:
    return DEFAULT_CASES

  selected = set(selected_cases)
  cases = tuple(case for case in DEFAULT_CASES if case.name in selected)
  missing = sorted(selected - {case.name for case in cases})
  if missing:
    raise ValueError(f"Unknown case names: {', '.join(missing)}")
  return cases


def main() -> int:
  args = parse_args()
  models_dir = args.models_dir.expanduser().resolve()
  detector_path = models_dir / "speed_limit_us_detector.onnx"
  classifier_path = models_dir / "speed_limit_us_value_classifier.onnx"
  if not detector_path.is_file():
    raise FileNotFoundError(detector_path)
  if not classifier_path.is_file():
    raise FileNotFoundError(classifier_path)

  slv.US_DETECTOR_MODEL_PATH = detector_path
  slv.US_CLASSIFIER_MODEL_PATH = classifier_path
  daemon = slv.SpeedLimitVisionDaemon(use_runtime=False)

  failures = 0
  for case in resolve_cases(args.selected_cases):
    image_path = Path(case.frame_path).expanduser().resolve()
    frame_bgr = cv2.imread(str(image_path))
    if frame_bgr is None:
      raise FileNotFoundError(image_path)

    detection = daemon._detect_sign(frame_bgr)
    predicted_speed = detection.speed_limit_mph if detection is not None else None
    confidence = round(detection.confidence, 4) if detection is not None else None
    passed = predicted_speed == case.expected_speed_limit_mph
    if not passed:
      failures += 1

    print(
      f"{case.name}: expected={case.expected_speed_limit_mph} "
      f"predicted={predicted_speed} confidence={confidence} "
      f"{'PASS' if passed else 'FAIL'}"
    )

  return 1 if args.strict and failures else 0


if __name__ == "__main__":
  raise SystemExit(main())
