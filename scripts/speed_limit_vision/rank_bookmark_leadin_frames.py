#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv

from collections import defaultdict
from pathlib import Path

import cv2

import starpilot.system.speed_limit_vision as slv


DEFAULT_MANIFEST = Path(".tmp/speed_limit_training/review/bookmark_leadins.csv")
DEFAULT_OUTPUT = Path(".tmp/speed_limit_training/review/bookmark_leadin_shortlist.csv")


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Rank sampled pre-bookmark lead-in frames by sign-likelihood for labeling.")
  parser.add_argument("--manifest", type=Path, default=DEFAULT_MANIFEST, help="CSV from import_bookmark_leadins.py.")
  parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT, help="Output CSV of top-ranked frames per bookmark.")
  parser.add_argument("--top-k", type=int, default=3, help="How many frames to keep per bookmark window.")
  return parser.parse_args()


def load_rows(path: Path):
  with path.open("r", encoding="utf-8", newline="") as handle:
    return [row for row in csv.DictReader(handle) if row.get("frame_path")]


def score_frame(daemon: slv.SpeedLimitVisionDaemon, frame_bgr):
  detection = daemon._detect_sign_from_detector_classifier(frame_bgr)
  ocr_detection = daemon._detect_sign_from_ocr_candidates(frame_bgr)
  proposals = daemon._collect_detector_classifier_proposals(frame_bgr)

  best_proposal_score = 0.0
  best_box = None
  frame_height, frame_width = frame_bgr.shape[:2]
  for proposal_confidence, class_id, (x1, y1, x2, y2) in proposals[:8]:
    box_width = x2 - x1
    box_height = y2 - y1
    if box_width <= 0 or box_height <= 0:
      continue
    crop = frame_bgr[y1:y2, x1:x2]
    regulatory_bonus = 0.15 if daemon._is_regulatory_speed_sign(crop) else 0.0
    class_bonus = 0.10 if class_id in (0, 2) else 0.0
    area_bonus = min((box_width * box_height) / max(frame_width * frame_height, 1), 0.04)
    score = proposal_confidence + regulatory_bonus + class_bonus + area_bonus
    if score > best_proposal_score:
      best_proposal_score = score
      best_box = (x1, y1, x2, y2)

  score = best_proposal_score
  reason = "proposal"
  predicted_speed = ""
  if ocr_detection is not None:
    score = max(score, 0.55 + ocr_detection.confidence)
    reason = "ocr"
    predicted_speed = str(ocr_detection.speed_limit_mph)
  if detection is not None:
    score = max(score, 1.0 + detection.confidence)
    reason = "detector"
    predicted_speed = str(detection.speed_limit_mph)

  return {
    "score": round(score, 4),
    "reason": reason,
    "predicted_speed": predicted_speed,
    "best_box": "" if best_box is None else ",".join(str(v) for v in best_box),
  }


def main() -> int:
  args = parse_args()
  manifest_path = args.manifest.expanduser().resolve()
  output_path = args.output.expanduser().resolve()
  rows = load_rows(manifest_path)
  daemon = slv.SpeedLimitVisionDaemon(use_runtime=False)

  grouped_rows = defaultdict(list)
  for row in rows:
    grouped_rows[(row["session_id"], row["bookmark_number"])].append(row)

  shortlist_rows = []
  for (_, _), group_rows in grouped_rows.items():
    scored_rows = []
    for row in group_rows:
      frame_path = (manifest_path.parents[1] / row["frame_path"]).resolve()
      frame_bgr = cv2.imread(str(frame_path))
      if frame_bgr is None:
        continue
      scored_rows.append((score_frame(daemon, frame_bgr), row))

    scored_rows.sort(key=lambda item: (-item[0]["score"], float(item[1]["sample_offset_s"])))
    for rank, (scored, row) in enumerate(scored_rows[:max(args.top_k, 1)], start=1):
      shortlist_rows.append({
        "session_id": row["session_id"],
        "bookmark_number": row["bookmark_number"],
        "rank": rank,
        "score": scored["score"],
        "reason": scored["reason"],
        "predicted_speed": scored["predicted_speed"],
        "sample_offset_s": row["sample_offset_s"],
        "frame_path": row["frame_path"],
        "contact_sheet_path": row["contact_sheet_path"],
        "window_result": row["window_result"],
        "best_box": scored["best_box"],
      })

  output_path.parent.mkdir(parents=True, exist_ok=True)
  with output_path.open("w", encoding="utf-8", newline="") as handle:
    writer = csv.DictWriter(handle, fieldnames=[
      "session_id",
      "bookmark_number",
      "rank",
      "score",
      "reason",
      "predicted_speed",
      "sample_offset_s",
      "frame_path",
      "contact_sheet_path",
      "window_result",
      "best_box",
    ])
    writer.writeheader()
    writer.writerows(shortlist_rows)

  print(f"Wrote {len(shortlist_rows)} shortlisted frame(s) to {output_path}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
