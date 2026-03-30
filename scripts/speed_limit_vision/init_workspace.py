#!/usr/bin/env python3
from __future__ import annotations

import argparse

from pathlib import Path

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import (  # type: ignore
    BOOKMARK_MANIFEST_FIELDS,
    BOOKMARK_LEADIN_MANIFEST_FIELDS,
    DEFAULT_SPEED_VALUES,
    VALUE_LABEL_FIELDS,
    DEFAULT_WORKSPACE,
    PUBLIC_CLASSIFIER_SAMPLE_FIELDS,
    PUBLIC_DETECTOR_SAMPLE_FIELDS,
    RAW_SOURCE_FIELDS,
    detector_dataset_yaml,
    ensure_dir,
    resolve_workspace,
    workspace_readme,
    write_csv_header,
    write_text,
  )
else:
  from .common import (
    BOOKMARK_MANIFEST_FIELDS,
    BOOKMARK_LEADIN_MANIFEST_FIELDS,
    DEFAULT_SPEED_VALUES,
    VALUE_LABEL_FIELDS,
    DEFAULT_WORKSPACE,
    PUBLIC_CLASSIFIER_SAMPLE_FIELDS,
    PUBLIC_DETECTOR_SAMPLE_FIELDS,
    RAW_SOURCE_FIELDS,
    detector_dataset_yaml,
    ensure_dir,
    resolve_workspace,
    workspace_readme,
    write_csv_header,
    write_text,
  )


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Initialize a speed-limit detector/classifier training workspace.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Workspace root. Defaults to .tmp/speed_limit_training under the repo root.")
  parser.add_argument("--force", action="store_true", help="Overwrite generated template files if they already exist.")
  parser.add_argument("--speed-values", nargs="+", type=int, default=list(DEFAULT_SPEED_VALUES), help="Classifier speed values to document in the workspace README.")
  return parser.parse_args()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)

  for relative_dir in (
    "detector/images/train",
    "detector/images/val",
    "detector/labels/train",
    "detector/labels/val",
    "classifier/train",
    "classifier/val",
    "review/images",
    "review/leadins/frames",
    "review/leadins/contact_sheets",
    "manifests",
    "raw",
    "staging",
    "exports",
    "runs",
  ):
    ensure_dir(workspace / relative_dir)

  write_text(workspace / "detector" / "dataset.yaml", detector_dataset_yaml(workspace), force=args.force)
  write_text(workspace / "README.md", workspace_readme(tuple(args.speed_values)), force=args.force)
  write_csv_header(workspace / "review" / "bookmarks.csv", BOOKMARK_MANIFEST_FIELDS, force=args.force)
  write_csv_header(workspace / "review" / "bookmark_leadins.csv", BOOKMARK_LEADIN_MANIFEST_FIELDS, force=args.force)
  write_csv_header(workspace / "classifier" / "value_labels.csv", VALUE_LABEL_FIELDS, force=args.force)
  write_csv_header(workspace / "manifests" / "raw_sources.csv", RAW_SOURCE_FIELDS, force=args.force)
  write_csv_header(workspace / "manifests" / "public_detector_samples.csv", PUBLIC_DETECTOR_SAMPLE_FIELDS, force=args.force)
  write_csv_header(workspace / "manifests" / "public_classifier_samples.csv", PUBLIC_CLASSIFIER_SAMPLE_FIELDS, force=args.force)

  print(f"Initialized speed-limit training workspace at {workspace}")
  print(f"  detector dataset: {workspace / 'detector/dataset.yaml'}")
  print(f"  review manifest:  {workspace / 'review/bookmarks.csv'}")
  print(f"  lead-in manifest: {workspace / 'review/bookmark_leadins.csv'}")
  print(f"  value labels:     {workspace / 'classifier/value_labels.csv'}")
  print(f"  raw sources:      {workspace / 'manifests/raw_sources.csv'}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
