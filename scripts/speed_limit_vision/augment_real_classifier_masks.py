#!/usr/bin/env python3
from __future__ import annotations

import argparse
import random

from dataclasses import dataclass
from pathlib import Path

import cv2

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
  from generate_value_roi_classifier_dataset import augment_mask, extract_value_mask  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace
  from .generate_value_roi_classifier_dataset import augment_mask, extract_value_mask


@dataclass(frozen=True)
class ExampleSpec:
  name: str
  speed_limit_mph: int
  image_path: str | None = None
  frame_path: str | None = None
  bbox: tuple[int, int, int, int] | None = None


DEFAULT_EXAMPLES = (
  ExampleSpec(
    name="live15_runtime",
    speed_limit_mph=15,
    frame_path=".tmp/live_c4_capture/stopped_sign_road.jpg",
    bbox=(725, 253, 768, 314),
  ),
  ExampleSpec(
    name="school20_crop",
    speed_limit_mph=20,
    image_path=".tmp/route_vision/frame_041_sign_tight.jpg",
  ),
  ExampleSpec(
    name="town30_crop",
    speed_limit_mph=30,
    image_path=".tmp/route_12c_seg9_10/seg10_real30_crop.png",
  ),
  ExampleSpec(
    name="town30_late_runtime",
    speed_limit_mph=30,
    frame_path=".tmp/vision_iter/seg10_5fps/frame_054.png",
    bbox=(887, 275, 931, 378),
  ),
  ExampleSpec(
    name="town40_crop",
    speed_limit_mph=40,
    image_path=".tmp/route_12c_seg9_10/seg10_real40_crop.png",
  ),
  ExampleSpec(
    name="highway40_crop",
    speed_limit_mph=40,
    image_path=".tmp/speed_route_frames_seg2_10_20/t12_sign_crop.png",
  ),
)


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Inject curated real runtime-style masks into the classifier dataset.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--variants-per-example", type=int, default=80, help="Augmented mask variants to generate per example.")
  parser.add_argument("--seed", type=int, default=20260330, help="Random seed.")
  return parser.parse_args()


def load_crop(spec: ExampleSpec):
  if spec.image_path:
    image = cv2.imread(spec.image_path)
    if image is None:
      raise FileNotFoundError(spec.image_path)
    return image
  if spec.frame_path and spec.bbox:
    frame = cv2.imread(spec.frame_path)
    if frame is None:
      raise FileNotFoundError(spec.frame_path)
    x1, y1, x2, y2 = spec.bbox
    crop = frame[y1:y2, x1:x2]
    if crop.size == 0:
      raise ValueError(f"{spec.name}: empty crop for bbox {spec.bbox}")
    return crop
  raise ValueError(f"{spec.name}: provide image_path or frame_path+bbox")


def save_mask(workspace: Path, split: str, speed_limit_mph: int, stem: str, mask_bgr) -> None:
  output_dir = ensure_dir(workspace / "classifier" / split / str(speed_limit_mph))
  cv2.imwrite(str(output_dir / f"{stem}.png"), mask_bgr)


def remove_appledouble_files(root: Path) -> None:
  for path in root.rglob("._*"):
    if path.is_file() or path.is_symlink():
      path.unlink()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  rng = random.Random(args.seed)
  written = 0

  for spec in DEFAULT_EXAMPLES:
    crop = load_crop(spec)
    mask = extract_value_mask(crop)
    if mask is None:
      print(f"{spec.name}: skipped, no mask extracted")
      continue

    base_mask = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
    save_mask(workspace, "train", spec.speed_limit_mph, f"real_runtime_{spec.name}_base", base_mask)
    written += 1

    for variant_index in range(max(args.variants_per_example, 0)):
      split = "val" if variant_index % 7 == 0 else "train"
      augmented = augment_mask(mask, rng)
      save_mask(workspace, split, spec.speed_limit_mph, f"real_runtime_{spec.name}_{variant_index:03d}", augmented)
      written += 1

    print(f"{spec.name}: added {1 + max(args.variants_per_example, 0)} mask(s) for {spec.speed_limit_mph} mph")

  remove_appledouble_files(workspace / "classifier" / "train")
  remove_appledouble_files(workspace / "classifier" / "val")
  print(f"Wrote {written} classifier mask image(s)")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
