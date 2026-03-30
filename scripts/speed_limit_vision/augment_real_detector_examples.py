#!/usr/bin/env python3
from __future__ import annotations

import argparse
import random

from pathlib import Path

import cv2
import numpy as np

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace


def parse_label(label_path: Path, image_shape: tuple[int, int, int]):
  lines = [line.strip() for line in label_path.read_text(encoding="utf-8").splitlines() if line.strip()]
  if len(lines) != 1:
    raise ValueError(f"Expected exactly one box in {label_path}")

  class_id, x_center, y_center, width, height = lines[0].split()
  image_height, image_width = image_shape[:2]
  x_center = float(x_center) * image_width
  y_center = float(y_center) * image_height
  width = float(width) * image_width
  height = float(height) * image_height
  x1 = x_center - width / 2
  y1 = y_center - height / 2
  x2 = x_center + width / 2
  y2 = y_center + height / 2
  return int(class_id), np.array([x1, y1, x2, y2], dtype=np.float32)


def write_label(label_path: Path, class_id: int, box: np.ndarray, image_shape: tuple[int, int, int]):
  image_height, image_width = image_shape[:2]
  x1, y1, x2, y2 = box.tolist()
  x_center = ((x1 + x2) / 2) / image_width
  y_center = ((y1 + y2) / 2) / image_height
  width = (x2 - x1) / image_width
  height = (y2 - y1) / image_height
  label_path.write_text(f"{class_id} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n", encoding="utf-8")


def random_motion_blur(image: np.ndarray, rng: random.Random):
  size = rng.choice((3, 5, 7))
  if rng.random() < 0.5:
    kernel = np.zeros((size, size), dtype=np.float32)
    kernel[size // 2, :] = 1.0 / size
  else:
    kernel = np.zeros((size, size), dtype=np.float32)
    kernel[:, size // 2] = 1.0 / size
  return cv2.filter2D(image, -1, kernel)


def augment_image(image: np.ndarray, box: np.ndarray, rng: random.Random):
  image_height, image_width = image.shape[:2]

  scale = rng.uniform(0.92, 1.08)
  translate_x = rng.uniform(-0.05, 0.05) * image_width
  translate_y = rng.uniform(-0.04, 0.04) * image_height
  center = (image_width / 2, image_height / 2)
  matrix = cv2.getRotationMatrix2D(center, rng.uniform(-1.5, 1.5), scale)
  matrix[0, 2] += translate_x
  matrix[1, 2] += translate_y

  warped = cv2.warpAffine(
    image,
    matrix,
    (image_width, image_height),
    flags=cv2.INTER_LINEAR,
    borderMode=cv2.BORDER_REPLICATE,
  )

  corners = np.array([
    [box[0], box[1], 1.0],
    [box[2], box[1], 1.0],
    [box[2], box[3], 1.0],
    [box[0], box[3], 1.0],
  ], dtype=np.float32)
  transformed = corners @ matrix.T
  x_coords = transformed[:, 0]
  y_coords = transformed[:, 1]
  warped_box = np.array([
    np.clip(np.min(x_coords), 0, image_width - 1),
    np.clip(np.min(y_coords), 0, image_height - 1),
    np.clip(np.max(x_coords), 0, image_width - 1),
    np.clip(np.max(y_coords), 0, image_height - 1),
  ], dtype=np.float32)

  alpha = rng.uniform(0.85, 1.18)
  beta = rng.uniform(-18.0, 16.0)
  augmented = cv2.convertScaleAbs(warped, alpha=alpha, beta=beta)

  if rng.random() < 0.55:
    augmented = cv2.GaussianBlur(augmented, (3, 3), rng.uniform(0.1, 1.0))
  if rng.random() < 0.35:
    augmented = random_motion_blur(augmented, rng)
  if rng.random() < 0.45:
    noise = rng.uniform(4.0, 12.0)
    augmented = np.clip(augmented.astype(np.float32) + np.random.normal(0.0, noise, augmented.shape), 0, 255).astype(np.uint8)

  return augmented, warped_box


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Oversample bootstrapped real detector frames with light augmentation.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--split", default="train", choices=("train", "val"), help="Detector split to augment.")
  parser.add_argument("--variants-per-image", type=int, default=80, help="How many augmented variants to generate for each real_*.jpg frame.")
  return parser.parse_args()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  image_dir = workspace / "detector" / "images" / args.split
  label_dir = workspace / "detector" / "labels" / args.split
  ensure_dir(image_dir)
  ensure_dir(label_dir)

  rng = random.Random(42)
  base_images = sorted(image_dir.glob("real_*.jpg"))
  created = 0
  for image_path in base_images:
    label_path = label_dir / f"{image_path.stem}.txt"
    image = cv2.imread(str(image_path))
    if image is None or not label_path.is_file():
      continue

    class_id, box = parse_label(label_path, image.shape)
    for variant_index in range(args.variants_per_image):
      augmented, warped_box = augment_image(image, box, rng)
      if warped_box[2] - warped_box[0] < 10 or warped_box[3] - warped_box[1] < 12:
        continue

      output_stem = f"{image_path.stem}_aug_{variant_index:03d}"
      output_image = image_dir / f"{output_stem}.jpg"
      output_label = label_dir / f"{output_stem}.txt"
      cv2.imwrite(str(output_image), augmented, [cv2.IMWRITE_JPEG_QUALITY, 92])
      write_label(output_label, class_id, warped_box, augmented.shape)
      created += 1

  print(f"Augmented {len(base_images)} real detector images into {created} variants")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
