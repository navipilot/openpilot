#!/usr/bin/env python3
from __future__ import annotations

import argparse
import subprocess

from pathlib import Path

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import (  # type: ignore
    CLASSIFIER_EXPORT_NAME,
    DEFAULT_WORKSPACE,
    DETECTOR_EXPORT_NAME,
    resolve_workspace,
  )
else:
  from .common import (
    CLASSIFIER_EXPORT_NAME,
    DEFAULT_WORKSPACE,
    DETECTOR_EXPORT_NAME,
    resolve_workspace,
  )


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Copy exported speed-limit ONNX models to a comma device.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--detector", type=Path, help="Detector ONNX path. Defaults to <workspace>/exports/speed_limit_us_detector.onnx.")
  parser.add_argument("--classifier", type=Path, help="Classifier ONNX path. Defaults to <workspace>/exports/speed_limit_us_value_classifier.onnx.")
  parser.add_argument("--host", default="comma@192.168.3.110", help="scp target host.")
  parser.add_argument("--remote-dir", default="/data/openpilot/starpilot/assets/vision_models", help="Remote vision model directory.")
  return parser.parse_args()


def scp_file(local_path: Path, host: str, remote_dir: str) -> None:
  subprocess.run(["scp", str(local_path), f"{host}:{remote_dir}/{local_path.name}"], check=True)


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)

  detector_path = args.detector.resolve() if args.detector else (workspace / "exports" / DETECTOR_EXPORT_NAME)
  classifier_path = args.classifier.resolve() if args.classifier else (workspace / "exports" / CLASSIFIER_EXPORT_NAME)

  copied = 0
  for local_path in (detector_path, classifier_path):
    if not local_path.is_file():
      continue
    scp_file(local_path, args.host, args.remote_dir)
    print(f"Copied {local_path.name} to {args.host}:{args.remote_dir}")
    copied += 1

  if copied == 0:
    raise SystemExit("No ONNX models found to copy")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
