#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv

from pathlib import Path

import cv2

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, VALUE_LABEL_FIELDS, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, VALUE_LABEL_FIELDS, ensure_dir, resolve_workspace


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Build a classifier crop dataset from detector labels and a value label manifest.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--labels-csv", type=Path, help="CSV manifest describing which labeled detector images map to which posted speed values.")
  parser.add_argument("--default-padding", type=float, default=0.10, help="Default crop padding ratio when a row does not provide one.")
  parser.add_argument("--overwrite", action="store_true", help="Overwrite existing classifier crops.")
  return parser.parse_args()


def load_rows(csv_path: Path) -> list[dict[str, str]]:
  with csv_path.open("r", encoding="utf-8", newline="") as csv_file:
    reader = csv.DictReader(csv_file)
    missing = [field for field in VALUE_LABEL_FIELDS if field not in (reader.fieldnames or [])]
    if missing:
      raise ValueError(f"Missing required CSV columns: {', '.join(missing)}")
    return [row for row in reader if (row.get("image_path") or "").strip()]


def resolve_image_path(workspace: Path, image_path_text: str) -> Path:
  image_path = Path(image_path_text).expanduser()
  if image_path.is_file():
    return image_path.resolve()

  candidate = (workspace / image_path_text).resolve()
  if candidate.is_file():
    return candidate

  basename = Path(image_path_text).name
  for search_root in (workspace / "detector" / "images", workspace / "review" / "images"):
    if not search_root.is_dir():
      continue
    for found in search_root.rglob(basename):
      if found.is_file():
        return found.resolve()

  raise FileNotFoundError(f"Image not found: {image_path_text}")


def resolve_label_path(workspace: Path, image_path: Path, label_path_text: str, split: str) -> Path:
  if label_path_text:
    label_path = Path(label_path_text).expanduser()
    if label_path.is_file():
      return label_path.resolve()
    candidate = (workspace / label_path_text).resolve()
    if candidate.is_file():
      return candidate
    raise FileNotFoundError(f"Label path not found: {label_path_text}")

  train_label = workspace / "detector" / "labels" / split / f"{image_path.stem}.txt"
  if train_label.is_file():
    return train_label.resolve()

  for split_name in ("train", "val"):
    candidate = workspace / "detector" / "labels" / split_name / f"{image_path.stem}.txt"
    if candidate.is_file():
      return candidate.resolve()

  raise FileNotFoundError(f"Detector label not found for {image_path.name}")


def parse_yolo_labels(label_path: Path) -> list[tuple[int, float, float, float, float]]:
  boxes = []
  with label_path.open("r", encoding="utf-8") as label_file:
    for raw_line in label_file:
      line = raw_line.strip()
      if not line:
        continue
      class_id, x_center, y_center, width, height = line.split(maxsplit=4)
      boxes.append((int(class_id), float(x_center), float(y_center), float(width), float(height)))
  return boxes


def crop_box(image, yolo_box: tuple[int, float, float, float, float], padding: float):
  _, x_center, y_center, width, height = yolo_box
  image_height, image_width = image.shape[:2]

  box_width = width * image_width
  box_height = height * image_height
  pad_width = box_width * padding
  pad_height = box_height * padding

  x1 = max(int(round((x_center * image_width) - box_width / 2 - pad_width)), 0)
  y1 = max(int(round((y_center * image_height) - box_height / 2 - pad_height)), 0)
  x2 = min(int(round((x_center * image_width) + box_width / 2 + pad_width)), image_width)
  y2 = min(int(round((y_center * image_height) + box_height / 2 + pad_height)), image_height)

  if x2 <= x1 or y2 <= y1:
    raise ValueError("Resolved crop has no area")
  return image[y1:y2, x1:x2]


def remove_appledouble_files(root: Path) -> None:
  for path in root.rglob("._*"):
    if path.is_file() or path.is_symlink():
      path.unlink()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  labels_csv = args.labels_csv.resolve() if args.labels_csv else (workspace / "classifier" / "value_labels.csv")
  rows = load_rows(labels_csv)

  built = 0
  for row in rows:
    split = (row.get("split") or "train").strip().lower()
    if split not in ("train", "val"):
      raise ValueError(f"Unsupported split '{split}' in {labels_csv}")

    speed_limit = (row.get("speed_limit_mph") or "").strip()
    if not speed_limit:
      raise ValueError(f"Missing speed_limit_mph for image {row['image_path']}")

    bbox_index = int((row.get("bbox_index") or "0").strip())
    padding_text = (row.get("padding") or "").strip()
    padding = float(padding_text) if padding_text else args.default_padding

    image_path = resolve_image_path(workspace, row["image_path"])
    label_path = resolve_label_path(workspace, image_path, (row.get("label_path") or "").strip(), split)
    boxes = parse_yolo_labels(label_path)
    if bbox_index >= len(boxes):
      raise IndexError(f"bbox_index {bbox_index} out of range for {label_path}")

    image = cv2.imread(str(image_path))
    if image is None:
      raise RuntimeError(f"Failed to read {image_path}")

    crop = crop_box(image, boxes[bbox_index], padding)
    output_dir = ensure_dir(workspace / "classifier" / split / speed_limit)
    output_path = output_dir / f"{image_path.stem}_bbox{bbox_index}.jpg"
    if output_path.exists() and not args.overwrite:
      continue

    cv2.imwrite(str(output_path), crop)
    built += 1

  remove_appledouble_files(workspace / "classifier")
  print(f"Built {built} classifier crop(s) into {workspace / 'classifier'}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
