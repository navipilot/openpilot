#!/usr/bin/env python3
from __future__ import annotations

import argparse
import random
import shutil

from pathlib import Path

import cv2
import numpy as np

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_SPEED_VALUES, DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
  from generate_synthetic_us_speed_limits import KNOWN_REAL_CROPS, augment_sign, render_regulatory_sign  # type: ignore
else:
  from .common import DEFAULT_SPEED_VALUES, DEFAULT_WORKSPACE, ensure_dir, resolve_workspace
  from .generate_synthetic_us_speed_limits import KNOWN_REAL_CROPS, augment_sign, render_regulatory_sign


VALUE_TEMPLATE_ROIS = (
  (0.35, 0.82, 0.15, 0.78),
  (0.45, 0.85, 0.18, 0.78),
  (0.40, 0.84, 0.18, 0.75),
)


def normalize_binary_mask(binary_mask: np.ndarray, size=(72, 96), padding=6):
  points = cv2.findNonZero(binary_mask)
  if points is None:
    return None

  x, y, width, height = cv2.boundingRect(points)
  digit = binary_mask[y:y + height, x:x + width]
  target_w, target_h = size
  scale = min((target_w - padding * 2) / max(width, 1), (target_h - padding * 2) / max(height, 1))
  resized_w = max(int(round(width * scale)), 1)
  resized_h = max(int(round(height * scale)), 1)
  resized = cv2.resize(digit, (resized_w, resized_h), interpolation=cv2.INTER_NEAREST)

  canvas = np.zeros((target_h, target_w), dtype=np.uint8)
  offset_x = (target_w - resized_w) // 2
  offset_y = (target_h - resized_h) // 2
  canvas[offset_y:offset_y + resized_h, offset_x:offset_x + resized_w] = resized
  return canvas


def extract_value_mask(sign_bgr: np.ndarray):
  gray = cv2.cvtColor(sign_bgr, cv2.COLOR_BGR2GRAY)
  height, width = gray.shape
  best_mask = None
  best_fill = 0.0

  for top_ratio, bottom_ratio, left_ratio, right_ratio in VALUE_TEMPLATE_ROIS:
    roi = gray[int(height * top_ratio):int(height * bottom_ratio), int(width * left_ratio):int(width * right_ratio)]
    if roi.size == 0:
      continue

    clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(8, 8)).apply(roi)
    _, binary = cv2.threshold(clahe, 0, 255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)
    binary = cv2.morphologyEx(binary, cv2.MORPH_OPEN, np.ones((2, 2), dtype=np.uint8))

    num_labels, labels, stats, _ = cv2.connectedComponentsWithStats(binary, 8)
    mask = np.zeros_like(binary)
    for label_idx in range(1, num_labels):
      x, y, comp_w, comp_h, area = stats[label_idx]
      if area < roi.shape[0] * roi.shape[1] * 0.01:
        continue
      if y < binary.shape[0] * 0.08:
        continue
      if comp_h < binary.shape[0] * 0.18:
        continue
      if comp_w > binary.shape[1] * 0.75:
        continue
      mask[labels == label_idx] = 255

    normalized = normalize_binary_mask(mask, size=(72, 96))
    if normalized is None:
      continue

    fill_ratio = float(np.count_nonzero(normalized)) / normalized.size
    if fill_ratio > best_fill:
      best_fill = fill_ratio
      best_mask = normalized

  return best_mask


def perspective_jitter(sign_rgba, rng: random.Random):
  sign = np.array(sign_rgba)
  sign_h, sign_w = sign.shape[:2]
  pad = max(sign_w, sign_h) // 5
  canvas = np.zeros((sign_h + pad * 2, sign_w + pad * 2, 4), dtype=np.uint8)
  canvas[pad:pad + sign_h, pad:pad + sign_w] = sign
  sign_h, sign_w = canvas.shape[:2]

  src = np.float32([[0, 0], [sign_w - 1, 0], [sign_w - 1, sign_h - 1], [0, sign_h - 1]])
  jitter_x = sign_w * 0.08
  jitter_y = sign_h * 0.08
  dst = src + np.float32([
    [rng.uniform(-jitter_x, jitter_x), rng.uniform(-jitter_y, jitter_y)],
    [rng.uniform(-jitter_x, jitter_x), rng.uniform(-jitter_y, jitter_y)],
    [rng.uniform(-jitter_x, jitter_x), rng.uniform(-jitter_y, jitter_y)],
    [rng.uniform(-jitter_x, jitter_x), rng.uniform(-jitter_y, jitter_y)],
  ])
  matrix = cv2.getPerspectiveTransform(src, dst)
  warped = cv2.warpPerspective(canvas, matrix, (sign_w, sign_h), flags=cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT, borderValue=(0, 0, 0, 0))
  ys, xs = np.where(warped[:, :, 3] > 0)
  if len(xs) == 0 or len(ys) == 0:
    return canvas
  return warped[ys.min():ys.max() + 1, xs.min():xs.max() + 1]


def augment_mask(mask: np.ndarray, rng: random.Random):
  canvas = np.zeros((128, 128), dtype=np.uint8)
  resized = cv2.resize(mask, None, fx=rng.uniform(0.85, 1.15), fy=rng.uniform(0.85, 1.15), interpolation=cv2.INTER_NEAREST)
  offset_x = max((canvas.shape[1] - resized.shape[1]) // 2 + rng.randint(-8, 8), 0)
  offset_y = max((canvas.shape[0] - resized.shape[0]) // 2 + rng.randint(-8, 8), 0)
  end_x = min(offset_x + resized.shape[1], canvas.shape[1])
  end_y = min(offset_y + resized.shape[0], canvas.shape[0])
  canvas[offset_y:end_y, offset_x:end_x] = resized[:end_y - offset_y, :end_x - offset_x]

  if rng.random() < 0.45:
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (rng.choice((1, 2, 3)), rng.choice((1, 2, 3))))
    operation = cv2.MORPH_DILATE if rng.random() < 0.5 else cv2.MORPH_ERODE
    canvas = cv2.morphologyEx(canvas, operation, kernel)
  if rng.random() < 0.55:
    canvas = cv2.GaussianBlur(canvas, (3, 3), rng.uniform(0.1, 1.0))
  if rng.random() < 0.35:
    noise = np.random.normal(0.0, rng.uniform(2.0, 9.0), canvas.shape).astype(np.float32)
    canvas = np.clip(canvas.astype(np.float32) + noise, 0, 255).astype(np.uint8)

  return cv2.cvtColor(canvas, cv2.COLOR_GRAY2BGR)


def save_mask(base_dir: Path, split: str, speed_value: int, image_bgr: np.ndarray, stem: str):
  output_dir = ensure_dir(base_dir / split / str(speed_value))
  cv2.imwrite(str(output_dir / f"{stem}.png"), image_bgr)


def main():
  parser = argparse.ArgumentParser(description="Generate a value-ROI classifier dataset from synthetic U.S. speed-limit signs.")
  parser.add_argument("--workspace", default=str(DEFAULT_WORKSPACE), help="Training workspace root.")
  parser.add_argument("--train-per-class", type=int, default=1800, help="Synthetic training samples per value.")
  parser.add_argument("--val-per-class", type=int, default=260, help="Synthetic validation samples per value.")
  parser.add_argument("--real-augmentations", type=int, default=28, help="Augmented mask samples to create per known real crop.")
  parser.add_argument("--seed", type=int, default=20260330, help="Random seed.")
  args = parser.parse_args()

  workspace = resolve_workspace(args.workspace)
  classifier_dir = workspace / "classifier"
  if classifier_dir.exists():
    shutil.rmtree(classifier_dir)
  ensure_dir(classifier_dir / "train")
  ensure_dir(classifier_dir / "val")

  rng = random.Random(args.seed)
  speed_values = tuple(DEFAULT_SPEED_VALUES)

  for split, per_class in (("train", max(args.train_per_class, 0)), ("val", max(args.val_per_class, 0))):
    for speed_value in speed_values:
      for index in range(per_class):
        school_zone = speed_value in (15, 20, 25) and rng.random() < 0.45
        sign_rgba = render_regulatory_sign(speed_value, school_zone=school_zone, seed=rng.randint(0, 1_000_000))
        sign_rgba = augment_sign(sign_rgba, rng)
        sign_rgba = perspective_jitter(sign_rgba, rng)
        sign_bgr = cv2.cvtColor(sign_rgba[:, :, :3], cv2.COLOR_RGB2BGR)
        mask = extract_value_mask(sign_bgr)
        if mask is None:
          continue
        output = augment_mask(mask, rng)
        save_mask(classifier_dir, split, speed_value, output, f"{split}_{speed_value}_{index:05d}")

  repo_root = Path(__file__).resolve().parents[2]
  imported_real = 0
  for relative_path, speed_value in KNOWN_REAL_CROPS:
    crop_path = repo_root / relative_path
    if not crop_path.is_file():
      continue
    crop_bgr = cv2.imread(str(crop_path))
    if crop_bgr is None:
      continue
    mask = extract_value_mask(crop_bgr)
    if mask is None:
      continue
    for augmentation_index in range(max(args.real_augmentations, 1)):
      split = "val" if augmentation_index % 5 == 0 else "train"
      output = augment_mask(mask, rng)
      save_mask(classifier_dir, split, speed_value, output, f"real_{speed_value}_{imported_real:03d}_{augmentation_index:03d}")
    imported_real += 1

  print(f"Generated ROI classifier dataset in {classifier_dir}")
  print(f"Imported real crops: {imported_real}")


if __name__ == "__main__":
  main()
