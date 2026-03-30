#!/usr/bin/env python3
from __future__ import annotations

import argparse

from pathlib import Path

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, resolve_workspace


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Train the speed-limit detector using Ultralytics YOLO.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--data", type=Path, help="Detector dataset YAML. Defaults to <workspace>/detector/dataset.yaml.")
  parser.add_argument("--model", default="yolo11n.pt", help="Ultralytics detector checkpoint to fine-tune.")
  parser.add_argument("--epochs", type=int, default=80, help="Training epochs.")
  parser.add_argument("--imgsz", type=int, default=640, help="Training image size.")
  parser.add_argument("--batch", type=int, default=16, help="Batch size.")
  parser.add_argument("--workers", type=int, default=8, help="Data loader workers.")
  parser.add_argument("--device", default="cpu", help="Ultralytics device string, for example cpu, mps, 0, or 0,1.")
  parser.add_argument("--project", type=Path, help="Training output directory. Defaults to <workspace>/runs/detector.")
  parser.add_argument("--name", default="yolo11n-speed-limit-us", help="Run name under --project.")
  parser.add_argument("--patience", type=int, default=20, help="Early stopping patience.")
  parser.add_argument("--cache", action="store_true", help="Cache images in RAM if supported.")
  parser.add_argument("--exist-ok", action="store_true", help="Allow overwriting an existing run directory.")
  return parser.parse_args()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  data_path = args.data.resolve() if args.data else (workspace / "detector" / "dataset.yaml")
  project_path = args.project.resolve() if args.project else (workspace / "runs" / "detector")

  try:
    from ultralytics import YOLO
  except Exception as exc:
    raise SystemExit(
      "Ultralytics is not installed. Run `uv sync --extra speedvision` in the repo root before training."
    ) from exc

  model = YOLO(args.model)
  model.train(
    data=str(data_path),
    epochs=args.epochs,
    imgsz=args.imgsz,
    batch=args.batch,
    workers=args.workers,
    device=args.device,
    project=str(project_path),
    name=args.name,
    patience=args.patience,
    cache=args.cache,
    exist_ok=args.exist_ok,
  )
  print(f"Detector training complete under {project_path / args.name}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
