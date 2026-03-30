#!/usr/bin/env python3
from __future__ import annotations

import argparse
import random

from collections import defaultdict
from pathlib import Path

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, DETECTOR_CLASS_NAMES, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, DETECTOR_CLASS_NAMES, ensure_dir, resolve_workspace


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Create a rebalanced detector dataset rooted in symlinks to the main workspace.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--output-root", type=Path, help="Output dataset root. Defaults to <workspace>/detector_rebalanced.")
  parser.add_argument("--max-other-train", type=int, default=4000, help="Maximum number of non-real train images to keep.")
  parser.add_argument("--real-val-count", type=int, default=0, help="Hold out this many real train images as extra validation examples.")
  parser.add_argument(
    "--source-cap",
    action="append",
    default=[],
    metavar="PREFIX=COUNT",
    help="Cap train images for a filename prefix, for example arts_challenging=1200 or lisa_traffic_sign=1000.",
  )
  parser.add_argument("--seed", type=int, default=0, help="Random seed for sampling non-real images.")
  parser.add_argument("--copy", action="store_true", help="Copy files instead of creating symlinks.")
  return parser.parse_args()


def link_or_copy(src: Path, dst: Path, copy_files: bool) -> None:
  if dst.exists() or dst.is_symlink():
    dst.unlink()
  ensure_dir(dst.parent)
  if copy_files:
    dst.write_bytes(src.read_bytes())
  else:
    dst.symlink_to(src.resolve())


def write_dataset_yaml(dataset_root: Path) -> Path:
  yaml_lines = [
    f"path: {dataset_root}",
    "train: images/train",
    "val: images/val",
    "names:",
  ]
  for index, class_name in enumerate(DETECTOR_CLASS_NAMES):
    yaml_lines.append(f"  {index}: {class_name}")
  dataset_yaml = dataset_root / "dataset.yaml"
  dataset_yaml.write_text("\n".join(yaml_lines) + "\n", encoding="utf-8")
  return dataset_yaml


def remove_appledouble_files(root: Path) -> None:
  for path in root.rglob("._*"):
    if path.is_file() or path.is_symlink():
      path.unlink()


def visible_file_count(root: Path) -> int:
  return sum(1 for path in root.glob("*") if path.is_file() and not path.name.startswith("._"))


def safe_unlink(path: Path) -> None:
  try:
    path.unlink()
  except FileNotFoundError:
    pass


def parse_source_caps(values: list[str]) -> dict[str, int]:
  caps: dict[str, int] = {}
  for value in values:
    if "=" not in value:
      raise ValueError(f"Invalid --source-cap '{value}', expected PREFIX=COUNT")
    prefix, count_text = value.split("=", 1)
    prefix = prefix.strip()
    if not prefix:
      raise ValueError(f"Invalid --source-cap '{value}', missing prefix")
    caps[prefix] = max(int(count_text), 0)
  return caps


def prefix_for_path(path: Path) -> str:
  name = path.name
  if name.startswith("real_"):
    return "real"
  if name.startswith("arts_challenging_"):
    return "arts_challenging"
  if name.startswith("lisa_traffic_sign_"):
    return "lisa_traffic_sign"
  if name.startswith("glare_images_"):
    return "glare_images"
  return name.split("_", 1)[0]


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  source_root = workspace / "detector"
  output_root = args.output_root.resolve() if args.output_root else (workspace / "detector_rebalanced")
  source_caps = parse_source_caps(args.source_cap)
  rng = random.Random(args.seed)

  train_images = sorted((source_root / "images" / "train").glob("*"))
  train_labels = source_root / "labels" / "train"
  val_image_dir = source_root / "images" / "val"
  val_label_dir = source_root / "labels" / "val"

  real_images = [path for path in train_images if path.name.startswith("real_")]
  other_images = [path for path in train_images if not path.name.startswith("real_")]
  rng.shuffle(real_images)
  heldout_real_images = sorted(real_images[:max(args.real_val_count, 0)], key=lambda path: path.name)
  target_real_train_images = sorted(real_images[max(args.real_val_count, 0):], key=lambda path: path.name)

  grouped_other_images: dict[str, list[Path]] = defaultdict(list)
  for image_path in other_images:
    grouped_other_images[prefix_for_path(image_path)].append(image_path)

  sampled_other_images: list[Path] = []
  uncapped_other_images: list[Path] = []
  for prefix, paths in grouped_other_images.items():
    rng.shuffle(paths)
    if prefix in source_caps:
      sampled_other_images.extend(paths[:source_caps[prefix]])
    else:
      uncapped_other_images.extend(paths)

  rng.shuffle(uncapped_other_images)
  remaining_slots = max(args.max_other_train - len(sampled_other_images), 0)
  sampled_other_images.extend(uncapped_other_images[:remaining_slots])

  target_train_images = target_real_train_images + sampled_other_images
  target_train_images.sort(key=lambda path: path.name)

  output_train_image_dir = ensure_dir(output_root / "images" / "train")
  output_train_label_dir = ensure_dir(output_root / "labels" / "train")
  output_val_image_dir = ensure_dir(output_root / "images" / "val")
  output_val_label_dir = ensure_dir(output_root / "labels" / "val")

  for existing in output_train_image_dir.glob("*"):
    safe_unlink(existing)
  for existing in output_train_label_dir.glob("*"):
    safe_unlink(existing)
  for existing in output_val_image_dir.glob("*"):
    safe_unlink(existing)
  for existing in output_val_label_dir.glob("*"):
    safe_unlink(existing)

  for image_path in target_train_images:
    label_path = train_labels / f"{image_path.stem}.txt"
    if not label_path.is_file():
      continue
    link_or_copy(image_path, output_train_image_dir / image_path.name, args.copy)
    link_or_copy(label_path, output_train_label_dir / label_path.name, args.copy)

  for image_path in sorted(val_image_dir.glob("*")):
    label_path = val_label_dir / f"{image_path.stem}.txt"
    if not label_path.is_file():
      continue
    link_or_copy(image_path, output_val_image_dir / image_path.name, args.copy)
    link_or_copy(label_path, output_val_label_dir / label_path.name, args.copy)

  for image_path in heldout_real_images:
    label_path = train_labels / f"{image_path.stem}.txt"
    if not label_path.is_file():
      continue
    link_or_copy(image_path, output_val_image_dir / image_path.name, args.copy)
    link_or_copy(label_path, output_val_label_dir / label_path.name, args.copy)

  remove_appledouble_files(output_root)
  dataset_yaml = write_dataset_yaml(output_root)
  print(f"Created rebalanced detector dataset at {output_root}")
  print(f"Dataset YAML: {dataset_yaml}")
  print(f"Train images: {visible_file_count(output_train_image_dir)}")
  print(f"  real train: {len(target_real_train_images)}")
  print(f"  real held out to val: {len(heldout_real_images)}")
  print(f"  sampled other: {len(sampled_other_images)}")
  if source_caps:
    print(f"  source caps: {source_caps}")
  print(f"Val images: {visible_file_count(output_val_image_dir)}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
