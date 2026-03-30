#!/usr/bin/env python3
from __future__ import annotations

import argparse
import shutil

from pathlib import Path

import cv2
import numpy as np

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import (  # type: ignore
    CLASSIFIER_EXPORT_NAME,
    DEFAULT_WORKSPACE,
    DETECTOR_EXPORT_NAME,
    REPO_ASSET_DIR,
    ensure_dir,
    resolve_workspace,
  )
else:
  from .common import (
    CLASSIFIER_EXPORT_NAME,
    DEFAULT_WORKSPACE,
    DETECTOR_EXPORT_NAME,
    REPO_ASSET_DIR,
    ensure_dir,
    resolve_workspace,
  )


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Export trained speed-limit detector/classifier checkpoints to ONNX.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--detector-weights", type=Path, help="Path to the trained detector .pt weights.")
  parser.add_argument("--classifier-weights", type=Path, help="Path to the trained classifier .pt weights.")
  parser.add_argument("--output-dir", type=Path, help="Where exported ONNX models should be written. Defaults to <workspace>/exports.")
  parser.add_argument("--detector-imgsz", type=int, default=640, help="Detector export image size.")
  parser.add_argument("--classifier-imgsz", type=int, default=128, help="Classifier export image size.")
  parser.add_argument("--opset", type=int, default=12, help="ONNX opset.")
  parser.add_argument("--install-repo-assets", action="store_true", help="Copy exported ONNX files into starpilot/assets/vision_models.")
  parser.add_argument("--skip-verify", action="store_true", help="Skip the OpenCV DNN load/forward smoke test after export.")
  return parser.parse_args()


def export_yolo(weights_path: Path, output_path: Path, imgsz: int, opset: int, nms: bool) -> None:
  from ultralytics import YOLO

  model = YOLO(str(weights_path))
  exported_path = Path(model.export(format="onnx", imgsz=imgsz, opset=opset, simplify=False, dynamic=False, nms=nms))
  ensure_dir(output_path.parent)
  shutil.copy2(exported_path, output_path)


def verify_onnx_with_opencv(model_path: Path, input_size: int) -> None:
  net = cv2.dnn.readNetFromONNX(str(model_path))
  blob = np.zeros((1, 3, input_size, input_size), dtype=np.float32)
  net.setInput(blob)
  _ = net.forward()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  output_dir = args.output_dir.resolve() if args.output_dir else (workspace / "exports")
  ensure_dir(output_dir)

  try:
    from ultralytics import YOLO  # noqa: F401
  except Exception as exc:
    raise SystemExit(
      "Ultralytics is not installed. Run `uv sync --extra speedvision` in the repo root before exporting."
    ) from exc

  exported_paths: list[Path] = []

  if args.detector_weights:
    detector_weights = args.detector_weights.resolve()
    detector_output = output_dir / DETECTOR_EXPORT_NAME
    export_yolo(detector_weights, detector_output, args.detector_imgsz, args.opset, nms=False)
    if not args.skip_verify:
      verify_onnx_with_opencv(detector_output, args.detector_imgsz)
    exported_paths.append(detector_output)

  if args.classifier_weights:
    classifier_weights = args.classifier_weights.resolve()
    classifier_output = output_dir / CLASSIFIER_EXPORT_NAME
    export_yolo(classifier_weights, classifier_output, args.classifier_imgsz, args.opset, nms=False)
    if not args.skip_verify:
      verify_onnx_with_opencv(classifier_output, args.classifier_imgsz)
    exported_paths.append(classifier_output)

  if not exported_paths:
    raise SystemExit("Pass at least one of --detector-weights or --classifier-weights")

  if args.install_repo_assets:
    ensure_dir(REPO_ASSET_DIR)
    for exported_path in exported_paths:
      shutil.copy2(exported_path, REPO_ASSET_DIR / exported_path.name)

  for exported_path in exported_paths:
    print(f"Exported {exported_path}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
