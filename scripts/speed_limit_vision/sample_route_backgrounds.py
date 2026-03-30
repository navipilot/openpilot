#!/usr/bin/env python3
from __future__ import annotations

import argparse

from pathlib import Path

import cv2

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, ensure_dir, resolve_workspace


IMAGE_SUFFIXES = {".jpg", ".jpeg", ".png", ".bmp", ".webp"}
VIDEO_SUFFIXES = {".hevc", ".mp4", ".mov", ".mkv", ".avi"}


def iter_source_files(paths: list[Path]):
  for input_path in paths:
    if input_path.is_file():
      yield input_path
      continue

    if not input_path.exists():
      continue

    for path in sorted(input_path.rglob("*")):
      if path.is_file() and path.suffix.lower() in IMAGE_SUFFIXES | VIDEO_SUFFIXES:
        yield path


def sample_images(source_files: list[Path], output_dir: Path, max_per_file: int):
  written = 0
  for file_index, image_path in enumerate(source_files, start=1):
    frame = cv2.imread(str(image_path))
    if frame is None:
      continue
    stem = image_path.stem.replace(" ", "_")
    output_path = output_dir / f"{file_index:05d}_{stem}.jpg"
    cv2.imwrite(str(output_path), frame, [cv2.IMWRITE_JPEG_QUALITY, 90])
    written += 1
    if written >= max_per_file:
      break


def sample_video(video_path: Path, output_dir: Path, seconds_between_frames: float, max_frames: int):
  cap = cv2.VideoCapture(str(video_path))
  fps = cap.get(cv2.CAP_PROP_FPS) or 20.0
  total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT) or 0)
  frame_step = max(int(round(seconds_between_frames * fps)), 1)
  frame_indices = range(0, total_frames if total_frames > 0 else frame_step * max_frames, frame_step)
  written = 0

  for frame_index in frame_indices:
    if written >= max_frames:
      break
    cap.set(cv2.CAP_PROP_POS_FRAMES, frame_index)
    ok, frame = cap.read()
    if not ok or frame is None:
      continue

    timestamp = frame_index / max(fps, 1.0)
    safe_stem = video_path.parent.name.replace(" ", "_")
    output_path = output_dir / f"{safe_stem}_{timestamp:06.2f}s.jpg"
    cv2.imwrite(str(output_path), frame, [cv2.IMWRITE_JPEG_QUALITY, 90])
    written += 1

  cap.release()
  return written


def main():
  parser = argparse.ArgumentParser(description="Sample comma route frames into a reusable background pool.")
  parser.add_argument("inputs", nargs="*", help="Route/video/image directories. Defaults to .tmp/comma_speed_training/realdata")
  parser.add_argument("--workspace", default=str(DEFAULT_WORKSPACE), help="Training workspace root.")
  parser.add_argument("--output-dir", default=None, help="Override background output directory.")
  parser.add_argument("--sample-seconds", type=float, default=1.25, help="Seconds between sampled frames for each video.")
  parser.add_argument("--max-per-video", type=int, default=6, help="Maximum frames to sample from each video.")
  parser.add_argument("--limit-files", type=int, default=0, help="Optional cap on the number of source files to sample.")
  args = parser.parse_args()

  workspace = resolve_workspace(args.workspace)
  output_dir = Path(args.output_dir).expanduser().resolve() if args.output_dir else workspace / "backgrounds"
  ensure_dir(output_dir)

  default_input = Path(".tmp/comma_speed_training/realdata")
  input_paths = [Path(path).expanduser().resolve() for path in args.inputs] if args.inputs else [default_input.resolve()]
  source_files = [path for path in iter_source_files(input_paths)]
  if args.limit_files > 0:
    source_files = source_files[:args.limit_files]

  image_files = [path for path in source_files if path.suffix.lower() in IMAGE_SUFFIXES]
  video_files = [path for path in source_files if path.suffix.lower() in VIDEO_SUFFIXES]

  if not image_files and not video_files:
    raise FileNotFoundError("No image or video sources found.")

  written = 0
  if image_files:
    sample_images(image_files, output_dir, max_per_file=max(len(image_files), 1))
    written += len(image_files)

  for video_path in video_files:
    written += sample_video(video_path, output_dir, max(args.sample_seconds, 0.1), max(args.max_per_video, 1))

  print(f"Sampled {written} background frames into {output_dir}")


if __name__ == "__main__":
  main()
