#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv

from pathlib import Path

import cv2

import starpilot.system.speed_limit_vision as slv

from scripts.speed_limit_vision import common
from scripts.speed_limit_vision import evaluate_bookmark_leadins as ebl


DEFAULT_OUTPUT_DIR = Path(".tmp/bookmark_sign_localization")


def parse_args():
  parser = argparse.ArgumentParser(description="Search around bookmarks for the most sign-like speed-limit frame and crop.")
  parser.add_argument("--clip-root", type=Path, default=ebl.DEFAULT_CLIP_ROOT, help="Copied route clip root.")
  parser.add_argument("--qlog-mtimes", type=Path, default=ebl.DEFAULT_QLOG_MTIMES, help="Text file with '<qlog path> <mtime epoch>' lines.")
  parser.add_argument("--session-root", type=Path, default=ebl.DEFAULT_SESSION_ROOT, help="Directory containing debug session folders.")
  parser.add_argument("--session-route-map", type=Path, default=common.preferred_session_route_map_path(), help="JSON file mapping debug session ids to route log ids.")
  parser.add_argument("--models-dir", type=Path, help="Directory containing speed_limit_us_detector.onnx and speed_limit_us_value_classifier.onnx.")
  parser.add_argument("--search-before", type=float, default=18.0, help="Seconds before the bookmark to scan.")
  parser.add_argument("--search-after", type=float, default=2.0, help="Seconds after the bookmark to scan.")
  parser.add_argument("--sample-every", type=float, default=0.5, help="Seconds between sampled frames.")
  parser.add_argument("--top-k", type=int, default=1, help="How many candidate frames to save per bookmark.")
  parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR, help="Output directory for frames/crops/manifest.")
  return parser.parse_args()


def configure_models(models_dir: Path | None):
  if not models_dir:
    return
  models_dir = models_dir.expanduser().resolve()
  detector_path = models_dir / "speed_limit_us_detector.onnx"
  classifier_path = models_dir / "speed_limit_us_value_classifier.onnx"
  if not detector_path.is_file():
    raise FileNotFoundError(detector_path)
  if not classifier_path.is_file():
    raise FileNotFoundError(classifier_path)
  slv.US_DETECTOR_MODEL_PATH = detector_path
  slv.US_CLASSIFIER_MODEL_PATH = classifier_path


def iter_video_samples(clip_path: Path, start_s: float, end_s: float, sample_every: float):
  capture = cv2.VideoCapture(str(clip_path))
  fps = capture.get(cv2.CAP_PROP_FPS) or 20.0
  start_frame = max(int(start_s * fps), 0)
  end_frame = max(int(end_s * fps), start_frame)

  frame_index = 0
  while frame_index < start_frame:
    ok, _ = capture.read()
    if not ok:
      capture.release()
      return
    frame_index += 1

  next_sample_s = start_s
  while frame_index <= end_frame:
    ok, frame_bgr = capture.read()
    if not ok or frame_bgr is None:
      break

    frame_time_s = frame_index / fps
    if frame_time_s + 1e-6 >= next_sample_s:
      yield frame_time_s, frame_bgr
      next_sample_s += sample_every

    frame_index += 1

  capture.release()


def iter_context_frames(clip_root: Path, window: ebl.BookmarkWindow, search_before: float, search_after: float, sample_every: float):
  ranges: list[tuple[Path, float, float]] = []
  start_s = window.segment_offset_s - search_before
  end_s = window.segment_offset_s + search_after

  if start_s < 0.0 and window.segment > 0:
    previous_clip = clip_root / f"{window.route}--{window.segment - 1}" / "fcamera.hevc"
    if previous_clip.is_file():
      ranges.append((previous_clip, max(60.0 + start_s, 0.0), 60.0))
    start_s = 0.0

  current_clip = clip_root / f"{window.route}--{window.segment}" / "fcamera.hevc"
  if current_clip.is_file():
    ranges.append((current_clip, max(start_s, 0.0), min(end_s, 60.0)))

  for clip_path, range_start_s, range_end_s in ranges:
    for source_time_s, frame_bgr in iter_video_samples(clip_path, range_start_s, range_end_s, sample_every):
      if clip_path.parent.name.endswith(f"--{window.segment - 1}"):
        relative_time_s = source_time_s - 60.0
      else:
        relative_time_s = source_time_s
      yield relative_time_s, clip_path, source_time_s, frame_bgr


def _score_expanded_candidate(daemon: slv.SpeedLimitVisionDaemon, frame_bgr, class_id: int, proposal_confidence: float, box, full_detection):
  frame_height, frame_width = frame_bgr.shape[:2]
  x1, y1, x2, y2 = box
  box_width = x2 - x1
  box_height = y2 - y1
  if box_width <= 0 or box_height <= 0:
    return None

  best = None
  for expand_left, expand_top, expand_right, expand_bottom, expansion_weight in slv.DETECTOR_CLASSIFIER_EXPANSIONS:
    expanded_x1 = max(int(x1 - box_width * expand_left), 0)
    expanded_y1 = max(int(y1 - box_height * expand_top), 0)
    expanded_x2 = min(int(x2 + box_width * expand_right), frame_width)
    expanded_y2 = min(int(y2 + box_height * expand_bottom), frame_height)
    if expanded_x2 <= expanded_x1 or expanded_y2 <= expanded_y1:
      continue

    sign_crop = frame_bgr[expanded_y1:expanded_y2, expanded_x1:expanded_x2]
    if sign_crop.size == 0:
      continue

    is_regulatory = daemon._is_regulatory_speed_sign(sign_crop) or class_id == 2
    model_read = daemon._classify_speed_limit_from_model(sign_crop)
    ocr_read = daemon._read_speed_limit_from_crop(sign_crop)
    if model_read is None and ocr_read is None:
      continue

    if class_id == 2:
      read_result = model_read or ocr_read
      if read_result is None or read_result[0] not in slv.SCHOOL_ZONE_SPEED_VALUES:
        continue
    elif not is_regulatory:
      if model_read is None or ocr_read is None or model_read[0] != ocr_read[0]:
        continue
      read_result = (model_read[0], min(model_read[1], ocr_read[1]))
    else:
      if model_read is not None and ocr_read is not None and model_read[0] == ocr_read[0]:
        read_result = (model_read[0], max(model_read[1], ocr_read[1]))
      else:
        read_result = model_read or ocr_read

    speed_limit_mph, read_confidence = read_result
    area_ratio = ((expanded_x2 - expanded_x1) * (expanded_y2 - expanded_y1)) / max(frame_width * frame_height, 1)
    score = read_confidence * 1.1 + proposal_confidence * 0.16 + expansion_weight * 0.08
    score += min(area_ratio * 6.0, 0.18)
    if is_regulatory:
      score += 0.14
    if model_read is not None and ocr_read is not None and model_read[0] == ocr_read[0]:
      score += 0.24
    if full_detection is not None and full_detection.speed_limit_mph == speed_limit_mph:
      score += 0.18 + full_detection.confidence * 0.08

    candidate = {
      "score": score,
      "box": (expanded_x1, expanded_y1, expanded_x2, expanded_y2),
      "proposal_confidence": proposal_confidence,
      "class_id": class_id,
      "is_regulatory": is_regulatory,
      "model_read": model_read,
      "ocr_read": ocr_read,
      "full_detection": full_detection,
      "read_result": read_result,
    }
    if best is None or candidate["score"] > best["score"]:
      best = candidate

  return best


def score_frame(daemon: slv.SpeedLimitVisionDaemon, frame_bgr):
  full_detection = daemon._detect_sign(frame_bgr)
  best = None

  for proposal_confidence, class_id, (x1, y1, x2, y2) in daemon._collect_detector_classifier_proposals(frame_bgr):
    if class_id == 1:
      continue

    candidate = _score_expanded_candidate(daemon, frame_bgr, class_id, proposal_confidence, (x1, y1, x2, y2), full_detection)
    if candidate is None:
      continue
    if best is None or candidate["score"] > best["score"]:
      best = candidate

  return best


def write_manifest(rows: list[dict], path: Path):
  path.parent.mkdir(parents=True, exist_ok=True)
  with path.open("w", encoding="utf-8", newline="") as handle:
    writer = csv.DictWriter(handle, fieldnames=[
      "session_id",
      "bookmark_number",
      "route",
      "segment",
      "relative_time_s",
      "source_video_path",
      "score",
      "proposal_confidence",
      "class_id",
      "is_regulatory",
      "model_read",
      "ocr_read",
      "full_detection",
      "frame_path",
      "crop_path",
      "box",
    ])
    writer.writeheader()
    writer.writerows(rows)


def main():
  args = parse_args()
  configure_models(args.models_dir)
  daemon = slv.SpeedLimitVisionDaemon(use_runtime=False)
  route_mtimes = ebl.load_qlog_mtimes(args.qlog_mtimes.expanduser().resolve())
  session_route_map = common.load_session_route_map(args.session_route_map)
  if not session_route_map:
    raise FileNotFoundError(f"No session route map found at {args.session_route_map}")
  clip_root = args.clip_root.expanduser().resolve()
  session_root = args.session_root.expanduser().resolve()
  output_dir = args.output_dir.expanduser().resolve()
  frames_dir = output_dir / "frames"
  crops_dir = output_dir / "crops"
  frames_dir.mkdir(parents=True, exist_ok=True)
  crops_dir.mkdir(parents=True, exist_ok=True)

  manifest_rows = []
  for session_id, route in session_route_map.items():
    bookmarks = ebl.load_bookmarks(session_root / session_id)
    for bookmark_number, event in enumerate(bookmarks, start=1):
      window = ebl.locate_window(route, event, route_mtimes, 5.0)
      if window is None:
        continue

      ranked = []
      for relative_time_s, source_video_path, source_time_s, frame_bgr in iter_context_frames(
        clip_root,
        window,
        args.search_before,
        args.search_after,
        args.sample_every,
      ):
        scored = score_frame(daemon, frame_bgr)
        if scored is None:
          continue
        ranked.append((scored["score"], relative_time_s, source_video_path, source_time_s, frame_bgr, scored))

      ranked.sort(key=lambda item: item[0], reverse=True)
      for rank_index, (_, relative_time_s, source_video_path, _, frame_bgr, scored) in enumerate(ranked[:max(args.top_k, 1)], start=1):
        x1, y1, x2, y2 = scored["box"]
        crop = frame_bgr[y1:y2, x1:x2]

        frame_name = f"{session_id}_bookmark_{bookmark_number:03d}_rank_{rank_index:02d}.jpg"
        crop_name = f"{session_id}_bookmark_{bookmark_number:03d}_rank_{rank_index:02d}_crop.jpg"
        frame_path = frames_dir / frame_name
        crop_path = crops_dir / crop_name
        cv2.imwrite(str(frame_path), frame_bgr, [cv2.IMWRITE_JPEG_QUALITY, 90])
        cv2.imwrite(str(crop_path), crop, [cv2.IMWRITE_JPEG_QUALITY, 90])

        def fmt_detection(result):
          if result is None:
            return ""
          return f"{result[0]}@{result[1]:.3f}"

        full_detection = scored["full_detection"]
        manifest_rows.append({
          "session_id": session_id,
          "bookmark_number": bookmark_number,
          "route": route,
          "segment": window.segment,
          "relative_time_s": f"{relative_time_s:.3f}",
          "source_video_path": str(source_video_path),
          "score": f"{scored['score']:.4f}",
          "proposal_confidence": f"{scored['proposal_confidence']:.4f}",
          "class_id": str(scored["class_id"]),
          "is_regulatory": str(bool(scored["is_regulatory"])),
          "model_read": fmt_detection(scored["model_read"]),
          "ocr_read": fmt_detection(scored["ocr_read"]),
          "full_detection": "" if full_detection is None else f"{full_detection.speed_limit_mph}@{full_detection.confidence:.3f}",
          "frame_path": str(frame_path),
          "crop_path": str(crop_path),
          "box": ",".join(str(value) for value in scored["box"]),
        })

  manifest_path = output_dir / "localized_bookmarks.csv"
  write_manifest(manifest_rows, manifest_path)
  print(f"Wrote {len(manifest_rows)} localized candidate(s) to {manifest_path}")


if __name__ == "__main__":
  main()
