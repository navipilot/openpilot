#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import time

from datetime import datetime, timezone
from pathlib import Path

import cv2

from ultralytics import YOLO

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
  from generate_value_roi_classifier_dataset import extract_value_mask  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace
  from .generate_value_roi_classifier_dataset import extract_value_mask


EVAL_WINDOWS = (
  ("full", (0.0, 0.0, 1.0, 1.0)),
  ("roi3", (0.40, 0.12, 0.92, 0.86)),
)
SIGN_CLASSES = {
  "regulatory_speed_limit",
  "school_zone_speed_limit",
  "speedLimit15",
  "speedLimit20",
  "speedLimit25",
  "speedLimit30",
  "speedLimit35",
  "speedLimit40",
  "speedLimit45",
  "speedLimit50",
  "speedLimit55",
  "speedLimit60",
  "speedLimit65",
  "speedLimit70",
  "speedLimit75",
  "schoolSpeedLimit25",
  "speedLimit55Ahead",
}
REAL_CASES = (
  ("live15", ".tmp/live_c4_capture/stopped_sign_road.jpg", 15),
  ("school20", ".tmp/route_vision/seg38_frames/frame_041.jpg", 20),
  ("highway40", ".tmp/speed_route_frames_seg2_10_20/t12.png", 40),
  ("town40", ".tmp/route_12c_seg9_10/seg10_early/frame_005.png", 40),
  ("town30", ".tmp/route_12c_seg9_10/seg10_early/frame_012.png", 30),
  ("town30_late", ".tmp/vision_iter/seg10_5fps/frame_054.png", 30),
)


def detect_best(detector: YOLO, classifier: YOLO, frame_bgr):
  frame_height, frame_width = frame_bgr.shape[:2]
  best = None
  for window_name, (left_ratio, top_ratio, right_ratio, bottom_ratio) in EVAL_WINDOWS:
    left = int(frame_width * left_ratio)
    top = int(frame_height * top_ratio)
    right = int(frame_width * right_ratio)
    bottom = int(frame_height * bottom_ratio)
    roi = frame_bgr[top:bottom, left:right]
    if roi.size == 0:
      continue

    detector_result = detector.predict(source=roi, conf=0.03, imgsz=640, device="cpu", verbose=False)[0]
    if detector_result.boxes is None:
      continue

    for box, cls, det_conf in zip(
      detector_result.boxes.xyxy.cpu().numpy(),
      detector_result.boxes.cls.cpu().numpy(),
      detector_result.boxes.conf.cpu().numpy(),
    ):
      class_name = detector.names[int(cls)]
      if class_name not in SIGN_CLASSES:
        continue

      x1, y1, x2, y2 = box.astype(int)
      x1 += left
      x2 += left
      y1 += top
      y2 += top
      box_width = x2 - x1
      box_height = y2 - y1
      if box_width <= 0 or box_height <= 0:
        continue

      expand = 0.18
      crop_left = max(x1 - int(box_width * expand), 0)
      crop_top = max(y1 - int(box_height * expand), 0)
      crop_right = min(x2 + int(box_width * expand), frame_width)
      crop_bottom = min(y2 + int(box_height * expand), frame_height)
      crop = frame_bgr[crop_top:crop_bottom, crop_left:crop_right]
      if crop.size == 0:
        continue

      mask = extract_value_mask(crop)
      if mask is None:
        continue

      classifier_result = classifier.predict(source=cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR), imgsz=128, device="cpu", verbose=False)[0]
      probabilities = classifier_result.probs
      if probabilities is None:
        continue

      predicted_value = int(classifier_result.names[int(probabilities.top1)])
      classifier_confidence = float(probabilities.top1conf)
      score = min(classifier_confidence * 0.82 + float(det_conf) * 0.26, 0.95)
      candidate = {
        "window": window_name,
        "detectorClass": class_name,
        "detectorConfidence": round(float(det_conf), 4),
        "predictedValue": predicted_value,
        "classifierConfidence": round(classifier_confidence, 4),
        "score": round(score, 4),
        "box": [int(x1), int(y1), int(x2), int(y2)],
      }
      if best is None or candidate["score"] > best["score"]:
        best = candidate
  return best


def evaluate_once(detector_weights: Path, classifier_weights: Path):
  detector = YOLO(str(detector_weights))
  classifier = YOLO(str(classifier_weights))
  records = []
  for label, frame_path, expected in REAL_CASES:
    frame = cv2.imread(frame_path)
    best = detect_best(detector, classifier, frame) if frame is not None else None
    correct = best is None if expected is None else bool(best is not None and best["predictedValue"] == expected)
    records.append({
      "case": label,
      "frame": frame_path,
      "expected": expected,
      "best": best,
      "correct": correct,
    })
  return records


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Watch a detector run and evaluate improved checkpoints on the saved comma examples.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--detector-weights", type=Path, required=True, help="Detector weights to watch, usually runs/.../weights/best.pt.")
  parser.add_argument("--classifier-weights", type=Path, required=True, help="Classifier weights to use for evaluation.")
  parser.add_argument("--interval", type=float, default=30.0, help="Polling interval in seconds.")
  parser.add_argument("--output", type=Path, help="JSONL output log path. Defaults to <workspace>/review/detector_watch.jsonl.")
  return parser.parse_args()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  detector_weights = args.detector_weights.resolve()
  classifier_weights = args.classifier_weights.resolve()
  output_path = args.output.resolve() if args.output else (ensure_dir(workspace / "review") / "detector_watch.jsonl")
  ensure_dir(output_path.parent)

  last_mtime = None
  while True:
    if detector_weights.is_file():
      mtime = detector_weights.stat().st_mtime
      if last_mtime is None or mtime > last_mtime:
        last_mtime = mtime
        records = evaluate_once(detector_weights, classifier_weights)
        payload = {
          "timestamp": datetime.now(timezone.utc).isoformat(),
          "detectorWeights": str(detector_weights),
          "classifierWeights": str(classifier_weights),
          "records": records,
        }
        with output_path.open("a", encoding="utf-8") as output_file:
          output_file.write(json.dumps(payload, separators=(",", ":")) + "\n")
        print(json.dumps(payload, indent=2))
    time.sleep(max(args.interval, 1.0))


if __name__ == "__main__":
  raise SystemExit(main())
