#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import re

from collections import defaultdict
from pathlib import Path

import cv2

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import (  # type: ignore
    DEFAULT_WORKSPACE,
    DETECTOR_CLASS_NAMES,
    PUBLIC_CLASSIFIER_SAMPLE_FIELDS,
    PUBLIC_DETECTOR_SAMPLE_FIELDS,
    RAW_SOURCE_FIELDS,
    VALUE_LABEL_FIELDS,
    default_raw_root,
    ensure_dir,
    resolve_workspace,
  )
else:
  from .common import (
    DEFAULT_WORKSPACE,
    DETECTOR_CLASS_NAMES,
    PUBLIC_CLASSIFIER_SAMPLE_FIELDS,
    PUBLIC_DETECTOR_SAMPLE_FIELDS,
    RAW_SOURCE_FIELDS,
    VALUE_LABEL_FIELDS,
    default_raw_root,
    ensure_dir,
    resolve_workspace,
  )


SOURCE_NAME = "glare_images"
SOURCE_VERSION = "GLARE Images"
SOURCE_LICENSE = "CC BY 4.0"

SPEED_TAG_PATTERN = re.compile(r"(speedLimit|exitSpeedAdvisory|rampSpeedAdvisory)(\d+)$")
GLARE_IGNORE_TAGS = {"speedLimit55Ahead"}


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Import GLARE image annotations into the speed-limit training workspace.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--images-root", type=Path, help="Path to the downloaded GLARE Images directory. Defaults to <raw>/glare_raw/Images.")
  parser.add_argument("--train-split", type=float, default=0.85, help="Train split ratio by origin-track hash.")
  parser.add_argument("--overwrite", action="store_true", help="Overwrite previously imported GLARE samples.")
  return parser.parse_args()


def default_images_root(workspace: Path) -> Path:
  return default_raw_root(workspace) / "glare_raw" / "Images"


def read_existing_rows(path: Path) -> list[dict[str, str]]:
  if not path.is_file():
    return []
  with path.open("r", encoding="utf-8", newline="") as csv_file:
    return list(csv.DictReader(csv_file))


def write_rows(path: Path, fieldnames: list[str], rows: list[dict[str, str]]) -> None:
  ensure_dir(path.parent)
  with path.open("w", encoding="utf-8", newline="") as csv_file:
    writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(rows)


def infer_label(tag: str) -> tuple[str, int] | None:
  if tag in GLARE_IGNORE_TAGS:
    return None
  match = SPEED_TAG_PATTERN.fullmatch(tag)
  if not match:
    return None
  tag_type, speed_value_text = match.groups()
  speed_value = int(speed_value_text)
  if tag_type == "speedLimit":
    return ("regulatory_speed_limit", speed_value)
  return ("advisory_speed_limit", speed_value)


def split_for_track(track_name: str, train_ratio: float) -> str:
  digest = hashlib.md5(track_name.encode("utf-8")).hexdigest()
  value = int(digest[:8], 16) / 0xFFFFFFFF
  return "train" if value < train_ratio else "val"


def yolo_box(image_width: int, image_height: int, xmin: int, ymin: int, xmax: int, ymax: int) -> tuple[float, float, float, float]:
  box_width = max(xmax - xmin, 1)
  box_height = max(ymax - ymin, 1)
  x_center = xmin + box_width / 2.0
  y_center = ymin + box_height / 2.0
  return (
    x_center / image_width,
    y_center / image_height,
    box_width / image_width,
    box_height / image_height,
  )


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  images_root = args.images_root.resolve() if args.images_root else default_images_root(workspace)
  annotations_csv = images_root / "allAnnotations.csv"
  if not annotations_csv.is_file():
    raise FileNotFoundError(f"GLARE allAnnotations.csv not found: {annotations_csv}")

  detector_manifest_path = workspace / "manifests" / "public_detector_samples.csv"
  classifier_manifest_path = workspace / "manifests" / "public_classifier_samples.csv"
  value_labels_path = workspace / "classifier" / "value_labels.csv"
  raw_sources_path = workspace / "manifests" / "raw_sources.csv"

  existing_detector_rows = [row for row in read_existing_rows(detector_manifest_path) if row.get("source_name") != SOURCE_NAME]
  existing_classifier_rows = [row for row in read_existing_rows(classifier_manifest_path) if row.get("source_name") != SOURCE_NAME]
  existing_value_rows = [row for row in read_existing_rows(value_labels_path) if SOURCE_NAME not in (row.get("image_path") or "")]
  existing_source_rows = [row for row in read_existing_rows(raw_sources_path) if row.get("source_name") != SOURCE_NAME]

  grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
  with annotations_csv.open("r", encoding="utf-8", newline="") as csv_file:
    reader = csv.DictReader(csv_file)
    for row in reader:
      tag = (row.get("Annotation tag") or "").strip()
      if infer_label(tag) is None:
        continue
      filename = (row.get("Filename") or "").strip()
      if filename:
        grouped[filename].append(row)

  detector_rows: list[dict[str, str]] = []
  classifier_rows: list[dict[str, str]] = []
  value_rows: list[dict[str, str]] = []
  class_counts: dict[str, int] = defaultdict(int)
  imported_images = 0
  imported_boxes = 0

  for filename in sorted(grouped):
    source_image = images_root / filename
    if not source_image.is_file():
      continue

    box_rows = grouped[filename]
    track_name = (box_rows[0].get("Origin track") or filename).strip()
    split = split_for_track(track_name, args.train_split)
    stem = Path(filename).stem
    image_out = workspace / "detector" / "images" / split / f"{SOURCE_NAME}_{stem}.png"
    label_out = workspace / "detector" / "labels" / split / f"{SOURCE_NAME}_{stem}.txt"
    image_bgr = cv2.imread(str(source_image))
    if image_bgr is None:
      continue
    image_height, image_width = image_bgr.shape[:2]

    if args.overwrite or not image_out.exists():
      ensure_dir(image_out.parent)
      image_out.write_bytes(source_image.read_bytes())

    yolo_lines: list[str] = []
    for bbox_index, row in enumerate(box_rows):
      tag = row["Annotation tag"].strip()
      inferred = infer_label(tag)
      if inferred is None:
        continue
      class_name, speed_value = inferred
      class_id = DETECTOR_CLASS_NAMES.index(class_name)
      xmin = int(float(row["Upper left corner X"]))
      ymin = int(float(row["Upper left corner Y"]))
      xmax = int(float(row["Lower right corner X"]))
      ymax = int(float(row["Lower right corner Y"]))
      x_center, y_center, width, height = yolo_box(image_width, image_height, xmin, ymin, xmax, ymax)
      yolo_lines.append(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}")

      record_key = f"{SOURCE_NAME}:{stem}:{bbox_index}"
      detector_rows.append({
        "record_key": record_key,
        "source_name": SOURCE_NAME,
        "split": split,
        "image_path": str(image_out.relative_to(workspace)),
        "label_path": str(label_out.relative_to(workspace)),
        "annotation_path": str(annotations_csv),
        "source_image_id": filename,
        "class_name": class_name,
        "speed_limit_mph": str(speed_value),
        "sign_code": tag,
        "bbox_left": str(xmin),
        "bbox_top": str(ymin),
        "bbox_right": str(xmax),
        "bbox_bottom": str(ymax),
      })
      classifier_rows.append({
        "record_key": record_key,
        "source_name": SOURCE_NAME,
        "split": split,
        "image_path": str(image_out.relative_to(workspace)),
        "speed_limit_mph": str(speed_value),
        "bbox_index": str(bbox_index),
        "label_path": str(label_out.relative_to(workspace)),
        "source_image_id": filename,
        "sign_code": tag,
      })
      value_rows.append({
        "image_path": str(image_out.relative_to(workspace)),
        "split": split,
        "speed_limit_mph": str(speed_value),
        "bbox_index": str(bbox_index),
        "padding": "0.10",
        "label_path": str(label_out.relative_to(workspace)),
      })
      class_counts[f"{class_name}:{speed_value}"] += 1
      imported_boxes += 1

    if yolo_lines and (args.overwrite or not label_out.exists()):
      ensure_dir(label_out.parent)
      label_out.write_text("\n".join(yolo_lines) + "\n", encoding="utf-8")
      imported_images += 1

  source_row = {
    "source_name": SOURCE_NAME,
    "source_version": SOURCE_VERSION,
    "source_license": SOURCE_LICENSE,
    "source_type": "public_detector_and_classifier_seed",
    "raw_path": str(images_root),
    "notes": "Imported GLARE Images/allAnnotations.csv speed-limit and advisory-speed tags.",
  }

  write_rows(raw_sources_path, RAW_SOURCE_FIELDS, existing_source_rows + [source_row])
  write_rows(detector_manifest_path, PUBLIC_DETECTOR_SAMPLE_FIELDS, existing_detector_rows + detector_rows)
  write_rows(classifier_manifest_path, PUBLIC_CLASSIFIER_SAMPLE_FIELDS, existing_classifier_rows + classifier_rows)
  write_rows(value_labels_path, VALUE_LABEL_FIELDS, existing_value_rows + value_rows)

  summary = ", ".join(f"{name}={count}" for name, count in sorted(class_counts.items()))
  print(f"Imported {imported_images} GLARE image(s) and {imported_boxes} box(es) from {images_root}")
  print(f"  detector manifest:   {detector_manifest_path}")
  print(f"  classifier manifest: {classifier_manifest_path}")
  print(f"  class counts:        {summary or 'none'}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
