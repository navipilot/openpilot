#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import tarfile
import xml.etree.ElementTree as ET

from collections import defaultdict
from pathlib import Path

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


DEFAULT_ARCHIVE_RELATIVE = Path("external/arts_probe/Public/ARTS-V1/Challenging/challenging-dev.tar.gz")
SOURCE_NAME = "arts_challenging"
SOURCE_VERSION = "ARTS-V1 Challenging"
SOURCE_LICENSE = "Research dataset, see source distribution"

ARTS_CODE_MAP: dict[str, tuple[str, int | None]] = {
  "R2-1": ("regulatory_speed_limit", None),
  "R2-125": ("regulatory_speed_limit", 25),
  "R2-130": ("regulatory_speed_limit", 30),
  "R2-135": ("regulatory_speed_limit", 35),
  "R2-140": ("regulatory_speed_limit", 40),
  "R2-145": ("regulatory_speed_limit", 45),
  "R2-150": ("regulatory_speed_limit", 50),
  "R2-155": ("regulatory_speed_limit", 55),
  "R2-165": ("regulatory_speed_limit", 65),
  "W13-1": ("advisory_speed_limit", None),
  "W13-2": ("advisory_speed_limit", None),
  "W13-3": ("advisory_speed_limit", None),
}


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Import ARTS Challenging speed-limit samples into the training workspace.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--archive", type=Path, help="Path to challenging-dev.tar.gz. Defaults to the SSD raw-data layout.")
  parser.add_argument("--train-split", type=float, default=0.85, help="Fallback train split ratio when ARTS split files are unavailable.")
  parser.add_argument("--overwrite", action="store_true", help="Overwrite previously imported ARTS images/labels.")
  return parser.parse_args()


def default_archive_path(workspace: Path) -> Path:
  return default_raw_root(workspace) / DEFAULT_ARCHIVE_RELATIVE


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


def build_fallback_split_lookup(stem_names: list[str], train_ratio: float) -> dict[str, str]:
  cutoff = int(len(stem_names) * max(0.0, min(train_ratio, 1.0)))
  fallback: dict[str, str] = {}
  for index, stem in enumerate(sorted(stem_names)):
    fallback[stem] = "train" if index < cutoff else "val"
  return fallback


def yolo_box(size: tuple[int, int], bbox: tuple[int, int, int, int]) -> tuple[float, float, float, float]:
  width, height = size
  xmin, ymin, xmax, ymax = bbox
  box_width = max(xmax - xmin, 1)
  box_height = max(ymax - ymin, 1)
  x_center = xmin + box_width / 2.0
  y_center = ymin + box_height / 2.0
  return (
    x_center / width,
    y_center / height,
    box_width / width,
    box_height / height,
  )


def parse_annotation(xml_bytes: bytes) -> tuple[tuple[int, int], list[dict[str, object]]]:
  root = ET.fromstring(xml_bytes)
  size_node = root.find("size")
  width = int(size_node.findtext("width", default="0"))
  height = int(size_node.findtext("height", default="0"))
  parsed: list[dict[str, object]] = []
  for obj in root.findall("object"):
    sign_code = (obj.findtext("name") or "").strip()
    mapped = ARTS_CODE_MAP.get(sign_code)
    if mapped is None:
      continue
    bbox_node = obj.find("bndbox")
    xmin = int(float(bbox_node.findtext("xmin", default="0")))
    ymin = int(float(bbox_node.findtext("ymin", default="0")))
    xmax = int(float(bbox_node.findtext("xmax", default="0")))
    ymax = int(float(bbox_node.findtext("ymax", default="0")))
    class_name, speed_value = mapped
    parsed.append({
      "sign_code": sign_code,
      "class_name": class_name,
      "speed_limit_mph": speed_value,
      "bbox": (xmin, ymin, xmax, ymax),
    })
  return (width, height), parsed


def update_split_lookup(split_lookup: dict[str, str], split_name: str, text: str) -> None:
  normalized_split = "val" if split_name == "test" else split_name
  for line in text.splitlines():
    stem = line.strip()
    if stem:
      split_lookup[stem] = normalized_split


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  archive_path = args.archive.resolve() if args.archive else default_archive_path(workspace)
  if not archive_path.is_file():
    raise FileNotFoundError(f"ARTS archive not found: {archive_path}")

  detector_manifest_path = workspace / "manifests" / "public_detector_samples.csv"
  classifier_manifest_path = workspace / "manifests" / "public_classifier_samples.csv"
  value_labels_path = workspace / "classifier" / "value_labels.csv"
  raw_sources_path = workspace / "manifests" / "raw_sources.csv"

  existing_detector_rows = [row for row in read_existing_rows(detector_manifest_path) if row.get("source_name") != SOURCE_NAME]
  existing_classifier_rows = [row for row in read_existing_rows(classifier_manifest_path) if row.get("source_name") != SOURCE_NAME]
  existing_value_rows = [row for row in read_existing_rows(value_labels_path) if not (row.get("image_path") or "").startswith("detector/images/") or SOURCE_NAME not in (row.get("image_path") or "")]
  existing_source_rows = [row for row in read_existing_rows(raw_sources_path) if row.get("source_name") != SOURCE_NAME]

  detector_rows: list[dict[str, str]] = []
  classifier_rows: list[dict[str, str]] = []
  value_rows: list[dict[str, str]] = []
  split_lookup: dict[str, str] = {}
  annotations_by_stem: dict[str, dict[str, object]] = {}

  with tarfile.open(archive_path, "r:gz") as tar:
    for member in tar:
      if not member.isfile():
        continue
      if member.name == "challenging/ImageSets/Main/train.txt":
        update_split_lookup(split_lookup, "train", tar.extractfile(member).read().decode("utf-8", "ignore"))
        continue
      if member.name == "challenging/ImageSets/Main/val.txt":
        update_split_lookup(split_lookup, "val", tar.extractfile(member).read().decode("utf-8", "ignore"))
        continue
      if member.name == "challenging/ImageSets/Main/test.txt":
        update_split_lookup(split_lookup, "test", tar.extractfile(member).read().decode("utf-8", "ignore"))
        continue
      if not (member.name.startswith("challenging/Annotations/") and member.name.endswith(".xml")):
        continue
      stem = Path(member.name).stem
      image_size, parsed_boxes = parse_annotation(tar.extractfile(member).read())
      if not parsed_boxes:
        continue
      annotations_by_stem[stem] = {
        "image_size": image_size,
        "boxes": parsed_boxes,
        "annotation_name": member.name,
      }

  if not split_lookup:
    split_lookup = build_fallback_split_lookup(list(annotations_by_stem), args.train_split)

  imported_images = 0
  imported_boxes = 0
  class_counts: dict[str, int] = defaultdict(int)

  with tarfile.open(archive_path, "r:gz") as tar:
    for member in tar:
      if not member.isfile():
        continue
      if not (member.name.startswith("challenging/JPEGImages/") and member.name.endswith(".jpg")):
        continue
      stem = Path(member.name).stem
      annotation = annotations_by_stem.get(stem)
      if annotation is None:
        continue

      split = split_lookup.get(stem, "train")
      image_bytes = tar.extractfile(member).read()
      image_size = annotation["image_size"]  # type: ignore[assignment]
      parsed_boxes = annotation["boxes"]  # type: ignore[assignment]

      image_out = workspace / "detector" / "images" / split / f"{SOURCE_NAME}_{stem}.jpg"
      label_out = workspace / "detector" / "labels" / split / f"{SOURCE_NAME}_{stem}.txt"
      if args.overwrite or not image_out.exists():
        ensure_dir(image_out.parent)
        image_out.write_bytes(image_bytes)
      yolo_lines: list[str] = []
      for bbox_index, box in enumerate(parsed_boxes):
        class_name = str(box["class_name"])
        speed_value = box["speed_limit_mph"]
        class_id = DETECTOR_CLASS_NAMES.index(class_name)
        x_center, y_center, width, height = yolo_box(image_size, box["bbox"])  # type: ignore[arg-type]
        yolo_lines.append(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}")
        xmin, ymin, xmax, ymax = box["bbox"]  # type: ignore[misc]
        record_key = f"{SOURCE_NAME}:{stem}:{bbox_index}"
        detector_rows.append({
          "record_key": record_key,
          "source_name": SOURCE_NAME,
          "split": split,
          "image_path": str(image_out.relative_to(workspace)),
          "label_path": str(label_out.relative_to(workspace)),
          "annotation_path": f"{archive_path}:{annotation['annotation_name']}",
          "source_image_id": stem,
          "class_name": class_name,
          "speed_limit_mph": "" if speed_value is None else str(speed_value),
          "sign_code": str(box["sign_code"]),
          "bbox_left": str(xmin),
          "bbox_top": str(ymin),
          "bbox_right": str(xmax),
          "bbox_bottom": str(ymax),
        })
        class_counts[class_name] += 1
        imported_boxes += 1
        if speed_value is not None:
          classifier_rows.append({
            "record_key": record_key,
            "source_name": SOURCE_NAME,
            "split": split,
            "image_path": str(image_out.relative_to(workspace)),
            "speed_limit_mph": str(speed_value),
            "bbox_index": str(bbox_index),
            "label_path": str(label_out.relative_to(workspace)),
            "source_image_id": stem,
            "sign_code": str(box["sign_code"]),
          })
          value_rows.append({
            "image_path": str(image_out.relative_to(workspace)),
            "split": split,
            "speed_limit_mph": str(speed_value),
            "bbox_index": str(bbox_index),
            "padding": "0.10",
            "label_path": str(label_out.relative_to(workspace)),
          })

      if yolo_lines and (args.overwrite or not label_out.exists()):
        ensure_dir(label_out.parent)
        label_out.write_text("\n".join(yolo_lines) + "\n", encoding="utf-8")
      imported_images += 1

  source_row = {
    "source_name": SOURCE_NAME,
    "source_version": SOURCE_VERSION,
    "source_license": SOURCE_LICENSE,
    "source_type": "public_detector_and_classifier_seed",
    "raw_path": str(archive_path),
    "notes": "ARTS challenging subset imported from VOC XML. Only mapped speed-limit classes were kept.",
  }

  write_rows(raw_sources_path, RAW_SOURCE_FIELDS, existing_source_rows + [source_row])
  write_rows(detector_manifest_path, PUBLIC_DETECTOR_SAMPLE_FIELDS, existing_detector_rows + detector_rows)
  write_rows(classifier_manifest_path, PUBLIC_CLASSIFIER_SAMPLE_FIELDS, existing_classifier_rows + classifier_rows)
  write_rows(value_labels_path, VALUE_LABEL_FIELDS, existing_value_rows + value_rows)

  class_summary = ", ".join(f"{name}={count}" for name, count in sorted(class_counts.items()))
  print(f"Imported {imported_images} ARTS image(s) and {imported_boxes} box(es) from {archive_path}")
  print(f"  detector manifest:   {detector_manifest_path}")
  print(f"  classifier manifest: {classifier_manifest_path}")
  print(f"  class counts:        {class_summary or 'none'}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
