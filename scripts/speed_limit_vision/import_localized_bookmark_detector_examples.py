#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import random

from pathlib import Path

import cv2

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace


LOCALIZED_MANIFEST = Path(".tmp/bookmark_sign_localization/localized_bookmarks.csv")


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Import high-confidence localized bookmark sign frames into the detector dataset.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--localized-manifest", type=Path, default=LOCALIZED_MANIFEST, help="CSV output from localize_bookmark_signs.py.")
  parser.add_argument("--manifest-out", type=Path, help="Optional output CSV manifest path.")
  parser.add_argument("--split", choices=("train", "val"), default="train", help="Target detector split.")
  parser.add_argument("--source-name", default="comma_bookmark_localized", help="Logical source name for imported localized bookmarks.")
  parser.add_argument("--source-region", default="", help="Optional region or market label, e.g. us_midwest.")
  parser.add_argument("--source-device", default="", help="Optional device identifier or platform label.")
  parser.add_argument("--source-driver", default="", help="Optional contributor/driver identifier.")
  parser.add_argument("--session-id", action="append", default=[], help="Optional session_id filter. May be specified multiple times.")
  parser.add_argument("--min-score", type=float, default=1.2, help="Minimum localization score to keep.")
  parser.add_argument("--min-width", type=int, default=25, help="Minimum bbox width in pixels.")
  parser.add_argument("--min-height", type=int, default=40, help="Minimum bbox height in pixels.")
  parser.add_argument("--require-consistent", type=int, default=2, help="Require this many matching non-empty value reads across model/ocr/full_detection.")
  parser.add_argument("--variants-per-example", type=int, default=6, help="Photometric variants to generate per localized frame.")
  parser.add_argument("--temporal-radius-s", type=float, default=0.0, help="Optional radius in seconds to sample neighboring source-video frames around each localized hit.")
  parser.add_argument("--temporal-step-s", type=float, default=0.15, help="Seconds between temporal neighbor samples when --temporal-radius-s is enabled.")
  parser.add_argument("--temporal-box-expand", type=float, default=0.14, help="Fractional box expansion to apply to temporal neighbor labels.")
  parser.add_argument("--seed", type=int, default=20260331, help="Random seed.")
  parser.add_argument("--jpeg-quality", type=int, default=95, help="JPEG quality for written detector images.")
  return parser.parse_args()


def detector_label_line(detector_class: int, x1: int, y1: int, x2: int, y2: int, image_shape: tuple[int, int, int]) -> str:
  image_h, image_w = image_shape[:2]
  x_center = ((x1 + x2) / 2) / image_w
  y_center = ((y1 + y2) / 2) / image_h
  width = (x2 - x1) / image_w
  height = (y2 - y1) / image_h
  return f"{detector_class} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n"


def remove_appledouble_files(root: Path) -> None:
  for path in root.rglob("._*"):
    if path.is_file() or path.is_symlink():
      path.unlink()


def dominant_read(row: dict[str, str]) -> tuple[str, int]:
  counts: dict[str, int] = {}
  for key in ("model_read", "ocr_read", "full_detection"):
    raw = row.get(key, "")
    value = raw.split("@", 1)[0].strip() if raw else ""
    if value:
      counts[value] = counts.get(value, 0) + 1
  if not counts:
    return "", 0
  best_value, best_count = max(counts.items(), key=lambda item: (item[1], item[0]))
  return best_value, best_count


def read_bbox(text: str) -> tuple[int, int, int, int]:
  x1, y1, x2, y2 = (int(part) for part in text.split(","))
  return x1, y1, x2, y2


def clamp_bbox(x1: int, y1: int, x2: int, y2: int, image_shape: tuple[int, int, int]) -> tuple[int, int, int, int]:
  image_h, image_w = image_shape[:2]
  x1 = max(0, min(x1, image_w - 2))
  y1 = max(0, min(y1, image_h - 2))
  x2 = max(x1 + 1, min(x2, image_w - 1))
  y2 = max(y1 + 1, min(y2, image_h - 1))
  return x1, y1, x2, y2


def expand_bbox(x1: int, y1: int, x2: int, y2: int, image_shape: tuple[int, int, int], expand_ratio: float) -> tuple[int, int, int, int]:
  width = x2 - x1
  height = y2 - y1
  x_pad = int(round(width * max(expand_ratio, 0.0) * 0.5))
  y_pad = int(round(height * max(expand_ratio, 0.0) * 0.5))
  return clamp_bbox(x1 - x_pad, y1 - y_pad, x2 + x_pad, y2 + y_pad, image_shape)


def read_frame_at(video_path: Path, target_time_s: float):
  capture = cv2.VideoCapture(str(video_path))
  fps = capture.get(cv2.CAP_PROP_FPS) or 20.0
  frame_index = max(int(round(target_time_s * fps)), 0)
  capture.set(cv2.CAP_PROP_POS_FRAMES, frame_index)
  ok, frame_bgr = capture.read()
  capture.release()
  if not ok or frame_bgr is None:
    return None
  return frame_bgr


def apply_variant(frame_bgr, variant_index: int, rng: random.Random):
  output = frame_bgr.copy()
  mode = variant_index % 6
  if mode == 0:
    alpha = rng.uniform(0.82, 0.94)
    beta = rng.randint(-18, 8)
    output = cv2.convertScaleAbs(output, alpha=alpha, beta=beta)
  elif mode == 1:
    alpha = rng.uniform(1.04, 1.14)
    beta = rng.randint(-8, 18)
    output = cv2.convertScaleAbs(output, alpha=alpha, beta=beta)
  elif mode == 2:
    output = cv2.GaussianBlur(output, (3, 3), rng.uniform(0.2, 0.8))
  elif mode == 3:
    noise = rng.normalvariate(0.0, 8.0)
    output = cv2.add(output, noise)
  elif mode == 4:
    hsv = cv2.cvtColor(output, cv2.COLOR_BGR2HSV)
    hsv[..., 2] = cv2.convertScaleAbs(hsv[..., 2], alpha=rng.uniform(0.78, 0.9), beta=0)
    output = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
  else:
    encode_params = [cv2.IMWRITE_JPEG_QUALITY, rng.randint(72, 88)]
    ok, encoded = cv2.imencode(".jpg", output, encode_params)
    if ok:
      output = cv2.imdecode(encoded, cv2.IMREAD_COLOR)
  return output


def iter_temporal_offsets(radius_s: float, step_s: float) -> list[float]:
  if radius_s <= 0.0 or step_s <= 0.0:
    return []
  steps = int(radius_s / step_s)
  offsets: list[float] = []
  for step in range(1, steps + 1):
    delta = round(step * step_s, 3)
    offsets.extend((-delta, delta))
  return sorted(offsets)


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  localized_manifest = args.localized_manifest.expanduser().resolve()
  if not localized_manifest.is_file():
    raise FileNotFoundError(localized_manifest)
  session_filter = set(args.session_id)

  image_dir = ensure_dir(workspace / "detector" / "images" / args.split)
  label_dir = ensure_dir(workspace / "detector" / "labels" / args.split)
  manifest_out = args.manifest_out.expanduser().resolve() if args.manifest_out else (ensure_dir(workspace / "review") / "localized_bookmark_detector_examples.csv")
  rng = random.Random(args.seed)
  temporal_offsets = iter_temporal_offsets(args.temporal_radius_s, args.temporal_step_s)

  records: list[dict[str, object]] = []
  kept = 0

  with localized_manifest.open("r", encoding="utf-8", newline="") as csv_file:
    reader = csv.DictReader(csv_file)
    for row in reader:
      if session_filter and row.get("session_id") not in session_filter:
        continue
      dominant_value, consistent_count = dominant_read(row)
      if consistent_count < max(args.require_consistent, 0):
        continue
      if row.get("is_regulatory", "").lower() != "true":
        continue
      score = float(row["score"])
      if score < args.min_score:
        continue
      x1, y1, x2, y2 = read_bbox(row["box"])
      width = x2 - x1
      height = y2 - y1
      if width < args.min_width or height < args.min_height:
        continue

      frame_path = Path(row["frame_path"])
      frame_bgr = cv2.imread(str(frame_path))
      if frame_bgr is None:
        continue

      class_id = int(row["class_id"])
      stem_base = f"real_localized_{row['session_id']}_{int(row['bookmark_number']):03d}"
      source_video_path = Path(row["source_video_path"]) if row.get("source_video_path") else None
      relative_time_s = float(row["relative_time_s"]) if row.get("relative_time_s") else 0.0

      source_frames = [("base", frame_bgr, (x1, y1, x2, y2))]
      if source_video_path and source_video_path.is_file():
        for offset_s in temporal_offsets:
          temporal_frame = read_frame_at(source_video_path, max(relative_time_s + offset_s, 0.0))
          if temporal_frame is None:
            continue
          temporal_bbox = expand_bbox(x1, y1, x2, y2, temporal_frame.shape, args.temporal_box_expand)
          source_frames.append((f"t{offset_s:+.2f}s".replace(".", "p"), temporal_frame, temporal_bbox))

      variants = []
      for source_suffix, source_bgr, source_bbox in source_frames:
        variants.append((source_suffix, source_bgr, source_bbox))
        for variant_index in range(max(args.variants_per_example, 0)):
          variants.append((f"{source_suffix}_var{variant_index:02d}", apply_variant(source_bgr, variant_index, rng), source_bbox))

      for suffix, variant_bgr, variant_bbox in variants:
        image_path = image_dir / f"{stem_base}_{suffix}.jpg"
        label_path = label_dir / f"{stem_base}_{suffix}.txt"
        cv2.imwrite(str(image_path), variant_bgr, [cv2.IMWRITE_JPEG_QUALITY, args.jpeg_quality])
        vx1, vy1, vx2, vy2 = variant_bbox
        label_path.write_text(detector_label_line(class_id, vx1, vy1, vx2, vy2, variant_bgr.shape), encoding="utf-8")
        records.append({
          "record_key": f"{row['session_id']}_{row['bookmark_number']}_{suffix}",
          "split": args.split,
          "source_name": row.get("source_name", args.source_name),
          "source_region": row.get("source_region", args.source_region),
          "source_device": row.get("source_device", args.source_device),
          "source_driver": row.get("source_driver", args.source_driver),
          "session_id": row["session_id"],
          "bookmark_number": row["bookmark_number"],
          "route": row["route"],
          "segment": row["segment"],
          "relative_time_s": row["relative_time_s"],
          "score": row["score"],
          "class_id": class_id,
          "dominant_value": dominant_value,
          "consistent_count": consistent_count,
          "bbox_x1": vx1,
          "bbox_y1": vy1,
          "bbox_x2": vx2,
          "bbox_y2": vy2,
          "source_frame": str(frame_path),
          "source_crop": row["crop_path"],
          "source_video": str(source_video_path) if source_video_path else "",
          "source_relative_time_s": relative_time_s,
          "dataset_image": str(image_path),
          "dataset_label": str(label_path),
        })
      kept += 1

  remove_appledouble_files(image_dir)
  remove_appledouble_files(label_dir)

  fieldnames = [
    "record_key",
    "split",
    "source_name",
    "source_region",
    "source_device",
    "source_driver",
    "session_id",
    "bookmark_number",
    "route",
    "segment",
    "relative_time_s",
    "score",
    "class_id",
    "dominant_value",
    "consistent_count",
    "bbox_x1",
    "bbox_y1",
    "bbox_x2",
    "bbox_y2",
    "source_frame",
    "source_crop",
    "source_video",
    "source_relative_time_s",
    "dataset_image",
    "dataset_label",
  ]
  with manifest_out.open("w", encoding="utf-8", newline="") as csv_file:
    writer = csv.DictWriter(csv_file, fieldnames=fieldnames)
    writer.writeheader()
    writer.writerows(records)

  print(f"Imported {kept} localized bookmark example(s)")
  print(f"Wrote {len(records)} detector image(s) into {args.split}")
  print(f"Manifest: {manifest_out}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
