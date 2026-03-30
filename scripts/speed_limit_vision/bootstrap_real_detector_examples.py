#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv

from dataclasses import dataclass
from pathlib import Path

import cv2

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace


@dataclass(frozen=True)
class ExampleSpec:
  name: str
  detector_class: int
  frame_path: str | None = None
  frame_dir: str | None = None
  template_path: str | None = None
  bbox_override: tuple[int, int, int, int] | None = None


DEFAULT_EXAMPLES = (
  ExampleSpec(
    name="route_seg2_t12_speed40",
    detector_class=0,
    frame_path=".tmp/speed_route_frames_seg2_10_20/t12.png",
    template_path=".tmp/speed_route_frames_seg2_10_20/t12_sign_crop.png",
  ),
  ExampleSpec(
    name="route_seg38_school20",
    detector_class=2,
    frame_path=".tmp/route_vision/seg38_frames/frame_041.jpg",
    template_path=".tmp/route_vision/frame_041_sign_tight.jpg",
  ),
  ExampleSpec(
    name="route_seg10_early_speed40",
    detector_class=0,
    frame_path=".tmp/route_12c_seg9_10/seg10_early/frame_005.png",
    template_path=".tmp/route_12c_seg9_10/seg10_real40_crop.png",
  ),
  ExampleSpec(
    name="route_seg10_early_speed30",
    detector_class=0,
    frame_path=".tmp/route_12c_seg9_10/seg10_early/frame_012.png",
    template_path=".tmp/route_12c_seg9_10/seg10_real30_crop.png",
  ),
  ExampleSpec(
    name="route_seg10_late_speed30",
    detector_class=0,
    frame_path=".tmp/vision_iter/seg10_5fps/frame_054.png",
    template_path=".tmp/route_12c_seg9_10/seg10_real30_crop.png",
    bbox_override=(885, 250, 941, 386),
  ),
  ExampleSpec(
    name="route_seg10_late_speed30_clear",
    detector_class=0,
    frame_path=".tmp/vision_iter/seg10_5fps/frame_052.png",
    template_path=".tmp/route_12c_seg9_10/seg10_real30_crop.png",
    bbox_override=(829, 284, 870, 382),
  ),
  ExampleSpec(
    name="live_capture_speed15",
    detector_class=0,
    frame_path=".tmp/live_c4_capture/stopped_sign_road.jpg",
    template_path=".tmp/live_c4_capture/stopped_sign_crop_manual.png",
    bbox_override=(724, 258, 763, 307),
  ),
)


def detector_label_line(detector_class: int, x1: int, y1: int, x2: int, y2: int, image_shape: tuple[int, int, int]) -> str:
  image_h, image_w = image_shape[:2]
  x_center = ((x1 + x2) / 2) / image_w
  y_center = ((y1 + y2) / 2) / image_h
  width = (x2 - x1) / image_w
  height = (y2 - y1) / image_h
  return f"{detector_class} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n"


def match_template(frame: cv2.typing.MatLike, template: cv2.typing.MatLike):
  result = cv2.matchTemplate(frame, template, cv2.TM_CCOEFF_NORMED)
  _, max_value, _, max_location = cv2.minMaxLoc(result)
  template_h, template_w = template.shape[:2]
  x1, y1 = max_location
  x2 = x1 + template_w
  y2 = y1 + template_h
  return float(max_value), (x1, y1, x2, y2)


def resolve_match(spec: ExampleSpec):
  if spec.template_path is None:
    raise FileNotFoundError(f"{spec.name}: missing template_path")
  template = cv2.imread(spec.template_path)
  if template is None:
    raise FileNotFoundError(f"{spec.name}: failed to read template {spec.template_path}")

  if spec.frame_path:
    frame = cv2.imread(spec.frame_path)
    if frame is None:
      raise FileNotFoundError(f"{spec.name}: failed to read frame {spec.frame_path}")
    if spec.bbox_override is not None:
      return Path(spec.frame_path), frame, 1.0, spec.bbox_override
    confidence, bbox = match_template(frame, template)
    return Path(spec.frame_path), frame, confidence, bbox

  if spec.frame_dir:
    best = None
    for frame_path in sorted(Path(spec.frame_dir).glob("*")):
      frame = cv2.imread(str(frame_path))
      if frame is None:
        continue
      if frame.shape[0] < template.shape[0] or frame.shape[1] < template.shape[1]:
        continue
      confidence, bbox = match_template(frame, template)
      if best is None or confidence > best[2]:
        best = (frame_path, frame, confidence, bbox)
    if best is None:
      raise FileNotFoundError(f"{spec.name}: no readable frames found in {spec.frame_dir}")
    return best

  raise ValueError(f"{spec.name}: one of frame_path or frame_dir must be provided")


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Import a few known real comma sign examples into the detector dataset.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--split", default="train", choices=("train", "val"), help="Detector dataset split to populate.")
  parser.add_argument("--manifest", type=Path, help="Optional CSV manifest path. Defaults to <workspace>/review/real_detector_examples.csv.")
  return parser.parse_args()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  split = args.split
  image_dir = ensure_dir(workspace / "detector" / "images" / split)
  label_dir = ensure_dir(workspace / "detector" / "labels" / split)
  manifest_path = args.manifest.resolve() if args.manifest else (ensure_dir(workspace / "review") / "real_detector_examples.csv")

  records: list[dict[str, object]] = []
  for spec in DEFAULT_EXAMPLES:
    frame_path, frame_bgr, confidence, (x1, y1, x2, y2) = resolve_match(spec)
    stem = f"real_{spec.name}"
    image_path = image_dir / f"{stem}.jpg"
    label_path = label_dir / f"{stem}.txt"

    cv2.imwrite(str(image_path), frame_bgr, [cv2.IMWRITE_JPEG_QUALITY, 95])
    label_path.write_text(detector_label_line(spec.detector_class, x1, y1, x2, y2, frame_bgr.shape), encoding="utf-8")

    records.append({
      "name": spec.name,
      "split": split,
      "source_frame": str(frame_path),
      "template_path": spec.template_path or "",
      "template_match_confidence": round(confidence, 6),
      "detector_class": spec.detector_class,
      "bbox_x1": x1,
      "bbox_y1": y1,
      "bbox_x2": x2,
      "bbox_y2": y2,
      "dataset_image": str(image_path),
      "dataset_label": str(label_path),
    })

  with manifest_path.open("w", encoding="utf-8", newline="") as manifest_file:
    fieldnames = [
      "name",
      "split",
      "source_frame",
      "template_path",
      "template_match_confidence",
      "detector_class",
      "bbox_x1",
      "bbox_y1",
      "bbox_x2",
      "bbox_y2",
      "dataset_image",
      "dataset_label",
    ]
    writer = csv.DictWriter(manifest_file, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(records)

  print(f"Imported {len(records)} real detector examples into {split} split")
  print(f"Manifest: {manifest_path}")
  for record in records:
    print(
      f"{record['name']}: conf={record['template_match_confidence']} "
      f"bbox=({record['bbox_x1']},{record['bbox_y1']},{record['bbox_x2']},{record['bbox_y2']})"
    )
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
