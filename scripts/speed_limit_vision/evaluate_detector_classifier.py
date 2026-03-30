#!/usr/bin/env python3
from __future__ import annotations

import argparse

from pathlib import Path

import cv2
import numpy as np

from ultralytics import YOLO

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_SPEED_VALUES  # type: ignore
  from generate_value_roi_classifier_dataset import extract_value_mask  # type: ignore
else:
  from .common import DEFAULT_SPEED_VALUES
  from .generate_value_roi_classifier_dataset import extract_value_mask


DETECTOR_SIGN_CLASSES = {
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


def iter_frames(path: Path):
  if path.is_dir():
    for frame_path in sorted(path.glob("frame_*")):
      frame = cv2.imread(str(frame_path))
      if frame is not None:
        yield frame_path.name, frame
    return

  cap = cv2.VideoCapture(str(path))
  fps = cap.get(cv2.CAP_PROP_FPS) or 20.0
  frame_index = 0
  while True:
    ok, frame = cap.read()
    if not ok:
      break
    yield f"t={frame_index / fps:06.2f}s", frame
    frame_index += 1
  cap.release()


def crop_with_margin(frame: np.ndarray, xyxy: np.ndarray, margin_ratio: float = 0.16):
  frame_h, frame_w = frame.shape[:2]
  x1, y1, x2, y2 = xyxy.astype(int)
  box_w = x2 - x1
  box_h = y2 - y1
  margin_x = int(box_w * margin_ratio)
  margin_y = int(box_h * margin_ratio)
  left = max(x1 - margin_x, 0)
  top = max(y1 - margin_y, 0)
  right = min(x2 + margin_x, frame_w)
  bottom = min(y2 + margin_y, frame_h)
  return frame[top:bottom, left:right]


def main():
  parser = argparse.ArgumentParser(description="Evaluate a detector + value classifier pair on saved frames or route video.")
  parser.add_argument("path", help="Frame directory or video path.")
  parser.add_argument("--detector", required=True, help="Ultralytics detector checkpoint (.pt).")
  parser.add_argument("--classifier", required=True, help="Ultralytics classifier checkpoint (.pt).")
  parser.add_argument("--conf", type=float, default=0.10, help="Detector confidence threshold.")
  parser.add_argument("--imgsz", type=int, default=960, help="Detector image size.")
  parser.add_argument("--device", default="mps", help="Inference device, such as mps or cpu.")
  parser.add_argument("--max-frames", type=int, default=0, help="Optional frame cap.")
  args = parser.parse_args()

  detector = YOLO(args.detector)
  classifier = YOLO(args.classifier)
  path = Path(args.path).expanduser().resolve()
  speed_values = tuple(DEFAULT_SPEED_VALUES)

  for frame_index, (label, frame_bgr) in enumerate(iter_frames(path), start=1):
    if args.max_frames > 0 and frame_index > args.max_frames:
      break

    detector_result = detector.predict(source=frame_bgr, conf=args.conf, imgsz=args.imgsz, device=args.device, verbose=False)[0]
    if detector_result.boxes is None or len(detector_result.boxes) == 0:
      continue

    printed = False
    for box, cls, conf in zip(detector_result.boxes.xyxy.cpu().numpy(), detector_result.boxes.cls.cpu().numpy(), detector_result.boxes.conf.cpu().numpy()):
      class_name = detector.names[int(cls)]
      if class_name not in DETECTOR_SIGN_CLASSES:
        continue

      crop = crop_with_margin(frame_bgr, box)
      if crop.size == 0:
        continue

      mask = extract_value_mask(crop)
      if mask is None:
        continue
      classifier_input = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
      classifier_result = classifier.predict(source=classifier_input, imgsz=128, device=args.device, verbose=False)[0]
      probabilities = classifier_result.probs
      if probabilities is None:
        continue

      top_index = int(probabilities.top1)
      if top_index >= len(speed_values):
        continue
      predicted_value = speed_values[top_index]
      predicted_confidence = float(probabilities.top1conf)

      if not printed:
        print(label)
        printed = True
      print(
        f"  detector={class_name} det_conf={float(conf):.3f} "
        f"classifier={predicted_value} cls_conf={predicted_confidence:.3f} "
        f"box={[round(float(v), 1) for v in box.tolist()]}"
      )


if __name__ == "__main__":
  main()
