#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import json
import math

from pathlib import Path

import cv2
import numpy as np

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import (  # type: ignore
    BOOKMARK_LEADIN_MANIFEST_FIELDS,
    DEFAULT_WORKSPACE,
    ensure_dir,
    preferred_clip_root,
    resolve_workspace,
    write_csv_header,
  )
else:
  from .common import (
    BOOKMARK_LEADIN_MANIFEST_FIELDS,
    DEFAULT_WORKSPACE,
    ensure_dir,
    preferred_clip_root,
    resolve_workspace,
    write_csv_header,
  )


DEFAULT_REPORT_JSON = Path(".tmp/live_route_clips/bookmark_windows_report.json")
DEFAULT_CLIP_ROOT = preferred_clip_root()


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Import sampled 5-second pre-bookmark route windows into the speed-limit training workspace.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--report-json", type=Path, default=DEFAULT_REPORT_JSON, help="JSON output from evaluate_bookmark_leadins.py.")
  parser.add_argument("--clip-root", type=Path, default=DEFAULT_CLIP_ROOT, help="Copied route clip root used by evaluate_bookmark_leadins.py.")
  parser.add_argument("--source-name", default="comma_bookmark", help="Logical source name for imported bookmark windows.")
  parser.add_argument("--source-region", default="", help="Optional region or market label, e.g. us_midwest.")
  parser.add_argument("--source-device", default="", help="Optional device identifier or platform label.")
  parser.add_argument("--source-driver", default="", help="Optional contributor/driver identifier.")
  parser.add_argument("--mode", choices=("misses", "hits", "all"), default="misses", help="Which bookmark windows to sample.")
  parser.add_argument("--sample-every", type=float, default=0.5, help="Seconds between sampled review frames.")
  parser.add_argument("--max-samples", type=int, default=12, help="Optional cap on sampled frames per bookmark window.")
  parser.add_argument("--overwrite", action="store_true", help="Overwrite existing extracted review frames and contact sheets.")
  return parser.parse_args()


def load_existing_rows(manifest_path: Path) -> dict[str, dict[str, str]]:
  if not manifest_path.exists():
    return {}

  with manifest_path.open("r", encoding="utf-8", newline="") as manifest_file:
    reader = csv.DictReader(manifest_file)
    return {row["record_key"]: row for row in reader if row.get("record_key")}


def read_frame_at(video_path: Path, target_time_s: float):
  capture = cv2.VideoCapture(str(video_path))
  fps = capture.get(cv2.CAP_PROP_FPS) or 20.0
  frame_index = max(int(round(target_time_s * fps)), 0)
  capture.set(cv2.CAP_PROP_POS_FRAMES, frame_index)
  ok, frame_bgr = capture.read()
  capture.release()
  if not ok or frame_bgr is None:
    return None
  actual_time_s = frame_index / fps
  return actual_time_s, frame_bgr


def sample_offsets(leadin_start_s: float, segment_offset_s: float, sample_every: float, max_samples: int):
  if sample_every <= 0:
    raise ValueError("sample_every must be positive")

  duration_s = max(segment_offset_s - leadin_start_s, 0.0)
  sample_count = int(math.floor(duration_s / sample_every)) + 1
  if max_samples > 0:
    sample_count = min(sample_count, max_samples)
  if sample_count <= 1:
    return [segment_offset_s]

  if max_samples > 0 and sample_count == max_samples:
    return [leadin_start_s + duration_s * index / (sample_count - 1) for index in range(sample_count)]

  offsets = []
  sample_offset_s = leadin_start_s
  while sample_offset_s <= segment_offset_s + 1e-6:
    offsets.append(sample_offset_s)
    sample_offset_s += sample_every
  return offsets[:sample_count]


def extract_window_frames(row: dict, clip_root: Path, sample_every: float, max_samples: int):
  route = row["route"]
  segment = int(row["segment"])
  segment_offset_s = float(row["segmentOffsetS"])
  leadin_start_s = float(row["leadinStartS"])
  spans_previous_segment = bool(row.get("spansPreviousSegment"))

  sampled = []
  previous_clip = clip_root / f"{route}--{segment - 1}" / "fcamera.hevc"
  current_clip = clip_root / f"{route}--{segment}" / "fcamera.hevc"

  for relative_offset_s in sample_offsets(leadin_start_s, segment_offset_s, sample_every, max_samples):
    if relative_offset_s < 0.0 and spans_previous_segment and segment > 0:
      source_video = previous_clip
      source_time_s = 60.0 + relative_offset_s
    else:
      source_video = current_clip
      source_time_s = max(relative_offset_s, 0.0)

    if not source_video.is_file():
      continue

    frame_info = read_frame_at(source_video, source_time_s)
    if frame_info is None:
      continue

    actual_time_s, frame_bgr = frame_info
    sampled.append({
      "relative_offset_s": relative_offset_s,
      "actual_time_s": actual_time_s,
      "source_video": source_video,
      "frame_bgr": frame_bgr,
    })

  return sampled


def write_contact_sheet(output_path: Path, frames: list[np.ndarray], labels: list[str], overwrite: bool):
  if output_path.exists() and not overwrite:
    return
  ensure_dir(output_path.parent)

  columns = 4
  rows = max(int(math.ceil(len(frames) / columns)), 1)
  tile_height, tile_width = 256, 456
  header_height = 26
  sheet = np.full((rows * (tile_height + header_height), columns * tile_width, 3), 24, dtype=np.uint8)

  for index, (frame_bgr, label) in enumerate(zip(frames, labels, strict=False)):
    row_index = index // columns
    column_index = index % columns
    y = row_index * (tile_height + header_height)
    x = column_index * tile_width

    resized = cv2.resize(frame_bgr, (tile_width, tile_height), interpolation=cv2.INTER_AREA)
    sheet[y + header_height:y + header_height + tile_height, x:x + tile_width] = resized
    cv2.putText(sheet, label, (x + 8, y + 18), cv2.FONT_HERSHEY_SIMPLEX, 0.55, (235, 235, 235), 1, cv2.LINE_AA)

  cv2.imwrite(str(output_path), sheet, [cv2.IMWRITE_JPEG_QUALITY, 88])


def include_row(row: dict, mode: str):
  hit = bool(row.get("hit"))
  if mode == "all":
    return "segment" in row
  if mode == "hits":
    return "segment" in row and hit
  return "segment" in row and not hit


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  report_rows = json.loads(args.report_json.expanduser().resolve().read_text(encoding="utf-8"))
  clip_root = args.clip_root.expanduser().resolve()

  frame_dir = ensure_dir(workspace / "review" / "leadins" / "frames")
  contact_sheet_dir = ensure_dir(workspace / "review" / "leadins" / "contact_sheets")
  manifest_path = workspace / "review" / "bookmark_leadins.csv"
  write_csv_header(manifest_path, BOOKMARK_LEADIN_MANIFEST_FIELDS)
  manifest_rows = load_existing_rows(manifest_path)

  imported_windows = 0
  imported_frames = 0
  for row in report_rows:
    if not include_row(row, args.mode):
      continue

    sampled_frames = extract_window_frames(row, clip_root, args.sample_every, args.max_samples)
    if not sampled_frames:
      continue

    imported_windows += 1
    window_result = "hit" if row.get("hit") else "miss"
    session_id = row["sessionId"]
    bookmark_number = int(row["bookmarkNumber"])
    route = row["route"]
    segment = int(row["segment"])
    published_values = ",".join(str(value) for value in row.get("publishedValues", []))
    candidate_values = ",".join(str(value) for value in row.get("candidateValues", []))

    contact_sheet_name = f"{session_id}_bookmark_{bookmark_number:03d}_{window_result}.jpg"
    contact_sheet_path = contact_sheet_dir / contact_sheet_name
    contact_sheet_labels = []
    contact_sheet_frames = []

    for sample_index, sample in enumerate(sampled_frames, start=1):
      frame_name = f"{session_id}_bookmark_{bookmark_number:03d}_sample_{sample_index:02d}.jpg"
      frame_path = frame_dir / frame_name
      if args.overwrite or not frame_path.exists():
        cv2.imwrite(str(frame_path), sample["frame_bgr"], [cv2.IMWRITE_JPEG_QUALITY, 90])

      imported_frames += 1
      contact_sheet_frames.append(sample["frame_bgr"])
      contact_sheet_labels.append(f"t={sample['relative_offset_s']:+.2f}s")

      record_key = f"{session_id}:bookmark:{bookmark_number}:sample:{sample_index}"
      manifest_rows[record_key] = {
        "record_key": record_key,
        "source_name": args.source_name,
        "source_region": args.source_region,
        "source_device": args.source_device,
        "source_driver": args.source_driver,
        "session_id": session_id,
        "bookmark_number": str(bookmark_number),
        "route": route,
        "segment": str(segment),
        "segment_offset_s": str(row["segmentOffsetS"]),
        "leadin_start_s": str(row["leadinStartS"]),
        "sample_offset_s": f"{sample['relative_offset_s']:.3f}",
        "window_result": window_result,
        "published_values": published_values,
        "candidate_values": candidate_values,
        "frame_path": str(frame_path.relative_to(workspace)),
        "contact_sheet_path": str(contact_sheet_path.relative_to(workspace)),
        "source_video_path": str(sample["source_video"]),
      }

    write_contact_sheet(contact_sheet_path, contact_sheet_frames, contact_sheet_labels, args.overwrite)

  with manifest_path.open("w", encoding="utf-8", newline="") as manifest_file:
    writer = csv.DictWriter(manifest_file, fieldnames=BOOKMARK_LEADIN_MANIFEST_FIELDS)
    writer.writeheader()
    for row in sorted(manifest_rows.values(), key=lambda entry: entry["record_key"]):
      writer.writerow(row)

  print(f"Imported {imported_windows} lead-in window(s) and {imported_frames} sampled frame(s) into {manifest_path}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
