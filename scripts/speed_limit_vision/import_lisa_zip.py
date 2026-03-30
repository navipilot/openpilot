#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import hashlib
import io

from collections import defaultdict
from pathlib import Path
from zipfile import ZipFile

import cv2
import numpy as np

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


SOURCE_NAME = "lisa_traffic_sign"
SOURCE_VERSION = "Kaggle omkarnadkarni/lisa-traffic-sign"
SOURCE_LICENSE = "See Kaggle dataset page"
DEFAULT_ZIP_RELATIVE = Path("lisa/lisa_traffic_sign.zip")

LISA_LABEL_MAP: dict[str, tuple[str, int | None] | None] = {
  "speedLimit15": ("regulatory_speed_limit", 15),
  "speedLimit25": ("regulatory_speed_limit", 25),
  "speedLimit30": ("regulatory_speed_limit", 30),
  "speedLimit35": ("regulatory_speed_limit", 35),
  "speedLimit40": ("regulatory_speed_limit", 40),
  "speedLimit45": ("regulatory_speed_limit", 45),
  "speedLimit50": ("regulatory_speed_limit", 50),
  "speedLimit55": ("regulatory_speed_limit", 55),
  "speedLimit65": ("regulatory_speed_limit", 65),
  "speedLimitUrdbl": ("regulatory_speed_limit", None),
  "schoolSpeedLimit25": ("school_zone_speed_limit", 25),
  "rampSpeedAdvisory20": ("advisory_speed_limit", 20),
  "rampSpeedAdvisory35": ("advisory_speed_limit", 35),
  "rampSpeedAdvisory40": ("advisory_speed_limit", 40),
  "rampSpeedAdvisory45": ("advisory_speed_limit", 45),
  "rampSpeedAdvisory50": ("advisory_speed_limit", 50),
  "rampSpeedAdvisoryUrdbl": ("advisory_speed_limit", None),
  "truckSpeedLimit55": None,
}


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Import speed-related LISA samples from the Kaggle ZIP into the training workspace.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--zip-path", type=Path, help="Path to lisa_traffic_sign.zip. Defaults to the SSD raw-data layout.")
  parser.add_argument("--train-split", type=float, default=0.85, help="Train split ratio by origin-track hash.")
  parser.add_argument("--overwrite", action="store_true", help="Overwrite previously imported LISA samples.")
  return parser.parse_args()


def default_zip_path(workspace: Path) -> Path:
  return default_raw_root(workspace) / DEFAULT_ZIP_RELATIVE


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


def parse_lisa_csv(csv_text: str) -> dict[str, list[dict[str, str]]]:
  grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
  reader = csv.DictReader(io.StringIO(csv_text), delimiter=";")
  for row in reader:
    tag = (row.get("Annotation tag") or "").strip()
    mapped = LISA_LABEL_MAP.get(tag)
    if mapped is None:
      continue
    filename = (row.get("Filename") or "").strip()
    if filename:
      grouped[filename].append(row)
  return grouped


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  zip_path = args.zip_path.resolve() if args.zip_path else default_zip_path(workspace)
  if not zip_path.is_file():
    raise FileNotFoundError(f"LISA ZIP not found: {zip_path}")

  detector_manifest_path = workspace / "manifests" / "public_detector_samples.csv"
  classifier_manifest_path = workspace / "manifests" / "public_classifier_samples.csv"
  value_labels_path = workspace / "classifier" / "value_labels.csv"
  raw_sources_path = workspace / "manifests" / "raw_sources.csv"

  existing_detector_rows = [row for row in read_existing_rows(detector_manifest_path) if row.get("source_name") != SOURCE_NAME]
  existing_classifier_rows = [row for row in read_existing_rows(classifier_manifest_path) if row.get("source_name") != SOURCE_NAME]
  existing_value_rows = [row for row in read_existing_rows(value_labels_path) if SOURCE_NAME not in (row.get("image_path") or "")]
  existing_source_rows = [row for row in read_existing_rows(raw_sources_path) if row.get("source_name") != SOURCE_NAME]

  detector_rows: list[dict[str, str]] = []
  classifier_rows: list[dict[str, str]] = []
  value_rows: list[dict[str, str]] = []
  class_counts: dict[str, int] = defaultdict(int)
  imported_images = 0
  imported_boxes = 0

  with ZipFile(zip_path) as zip_file:
    csv_members = sorted(name for name in zip_file.namelist() if name.endswith("frameAnnotations.csv"))
    for csv_member in csv_members:
      csv_text = zip_file.read(csv_member).decode("utf-8", "ignore")
      grouped = parse_lisa_csv(csv_text)
      csv_parent = Path(csv_member).parent
      drive_slug = Path(csv_member).parts[0]

      for filename in sorted(grouped):
        source_image_member = str(csv_parent / filename)
        try:
          image_bytes = zip_file.read(source_image_member)
        except KeyError:
          continue

        image_array = cv2.imdecode(np.frombuffer(image_bytes, dtype=np.uint8), cv2.IMREAD_COLOR)
        if image_array is None:
          continue
        image_height, image_width = image_array.shape[:2]
        box_rows = grouped[filename]
        track_name = (box_rows[0].get("Origin track") or filename).strip()
        split = split_for_track(f"{drive_slug}:{track_name}", args.train_split)
        stem = Path(filename).stem
        out_stem = f"{SOURCE_NAME}_{drive_slug}_{stem}"
        image_out = workspace / "detector" / "images" / split / f"{out_stem}.png"
        label_out = workspace / "detector" / "labels" / split / f"{out_stem}.txt"

        if args.overwrite or not image_out.exists():
          ensure_dir(image_out.parent)
          image_out.write_bytes(image_bytes)

        yolo_lines: list[str] = []
        valid_boxes = 0
        for bbox_index, row in enumerate(box_rows):
          tag = row["Annotation tag"].strip()
          mapped = LISA_LABEL_MAP.get(tag)
          if mapped is None:
            continue
          class_name, speed_value = mapped
          class_id = DETECTOR_CLASS_NAMES.index(class_name)
          xmin = int(float(row["Upper left corner X"]))
          ymin = int(float(row["Upper left corner Y"]))
          xmax = int(float(row["Lower right corner X"]))
          ymax = int(float(row["Lower right corner Y"]))
          x_center, y_center, width, height = yolo_box(image_width, image_height, xmin, ymin, xmax, ymax)
          yolo_lines.append(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}")

          record_key = f"{SOURCE_NAME}:{drive_slug}:{stem}:{bbox_index}"
          detector_rows.append({
            "record_key": record_key,
            "source_name": SOURCE_NAME,
            "split": split,
            "image_path": str(image_out.relative_to(workspace)),
            "label_path": str(label_out.relative_to(workspace)),
            "annotation_path": f"{zip_path}:{csv_member}",
            "source_image_id": f"{drive_slug}/{filename}",
            "class_name": class_name,
            "speed_limit_mph": "" if speed_value is None else str(speed_value),
            "sign_code": tag,
            "bbox_left": str(xmin),
            "bbox_top": str(ymin),
            "bbox_right": str(xmax),
            "bbox_bottom": str(ymax),
          })
          if speed_value is not None:
            classifier_rows.append({
              "record_key": record_key,
              "source_name": SOURCE_NAME,
              "split": split,
              "image_path": str(image_out.relative_to(workspace)),
              "speed_limit_mph": str(speed_value),
              "bbox_index": str(bbox_index),
              "label_path": str(label_out.relative_to(workspace)),
              "source_image_id": f"{drive_slug}/{filename}",
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
          class_counts[f"{class_name}:{'' if speed_value is None else speed_value}"] += 1
          imported_boxes += 1
          valid_boxes += 1

        if valid_boxes and (args.overwrite or not label_out.exists()):
          ensure_dir(label_out.parent)
          label_out.write_text("\n".join(yolo_lines) + "\n", encoding="utf-8")
          imported_images += 1

  source_row = {
    "source_name": SOURCE_NAME,
    "source_version": SOURCE_VERSION,
    "source_license": SOURCE_LICENSE,
    "source_type": "public_detector_and_classifier_seed",
    "raw_path": str(zip_path),
    "notes": "Imported speed-related LISA samples directly from the Kaggle ZIP. Ignores truckSpeedLimit55.",
  }

  write_rows(raw_sources_path, RAW_SOURCE_FIELDS, existing_source_rows + [source_row])
  write_rows(detector_manifest_path, PUBLIC_DETECTOR_SAMPLE_FIELDS, existing_detector_rows + detector_rows)
  write_rows(classifier_manifest_path, PUBLIC_CLASSIFIER_SAMPLE_FIELDS, existing_classifier_rows + classifier_rows)
  write_rows(value_labels_path, VALUE_LABEL_FIELDS, existing_value_rows + value_rows)

  summary = ", ".join(f"{name}={count}" for name, count in sorted(class_counts.items()))
  print(f"Imported {imported_images} LISA image(s) and {imported_boxes} box(es) from {zip_path}")
  print(f"  detector manifest:   {detector_manifest_path}")
  print(f"  classifier manifest: {classifier_manifest_path}")
  print(f"  class counts:        {summary or 'none'}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
