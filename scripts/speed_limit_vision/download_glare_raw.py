#!/usr/bin/env python3
from __future__ import annotations

import argparse
import csv
import importlib

from pathlib import Path

import gdown

if __package__ in (None, ""):
  import sys
  sys.path.insert(0, str(Path(__file__).resolve().parent))
  from common import DEFAULT_WORKSPACE, default_raw_root, ensure_dir, resolve_workspace  # type: ignore
else:
  from .common import DEFAULT_WORKSPACE, default_raw_root, ensure_dir, resolve_workspace


GLARE_ROOT_URL = "https://drive.google.com/drive/folders/1gmoOSgvjR4DP7jGfGS_xAmxcMShyeThx?usp=sharing"
DEFAULT_PREFIXES = ("Images/", "Tracks/")
MANIFEST_FIELDS = ["file_id", "relative_path", "local_path"]


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Download the raw GLARE image/track files without checkpoint artifacts.")
  parser.add_argument("--workspace", type=Path, default=DEFAULT_WORKSPACE, help="Training workspace root.")
  parser.add_argument("--output-root", type=Path, help="Download destination. Defaults to <raw>/glare_raw.")
  parser.add_argument("--prefix", dest="prefixes", nargs="+", default=list(DEFAULT_PREFIXES), help="Path prefixes to keep from the GLARE Drive tree.")
  parser.add_argument("--manifest", type=Path, help="Optional CSV path for the filtered file manifest.")
  parser.add_argument("--list-only", action="store_true", help="List matching files without downloading them.")
  parser.add_argument("--resume", action="store_true", help="Resume partial downloads and skip completed files.")
  return parser.parse_args()


def main() -> int:
  args = parse_args()
  workspace = resolve_workspace(args.workspace)
  output_root = args.output_root.resolve() if args.output_root else (default_raw_root(workspace) / "glare_raw")
  manifest_path = args.manifest.resolve() if args.manifest else (output_root / "manifest.csv")
  ensure_dir(output_root)

  download_folder_mod = importlib.import_module("gdown.download_folder")
  entries = download_folder_mod.download_folder(
    url=GLARE_ROOT_URL,
    skip_download=True,
    quiet=True,
    remaining_ok=True,
  )
  if entries is None:
    raise RuntimeError("Failed to enumerate the GLARE Drive tree")

  selected = [entry for entry in entries if any(entry.path.startswith(prefix) for prefix in args.prefixes)]
  print(f"Matched {len(selected)} GLARE file(s) under prefixes: {', '.join(args.prefixes)}")

  with manifest_path.open("w", encoding="utf-8", newline="") as csv_file:
    writer = csv.DictWriter(csv_file, fieldnames=MANIFEST_FIELDS)
    writer.writeheader()
    for entry in selected:
      local_path = output_root / entry.path
      writer.writerow({
        "file_id": entry.id,
        "relative_path": entry.path,
        "local_path": str(local_path),
      })
      if args.list_only:
        continue
      ensure_dir(local_path.parent)
      gdown.download(
        id=entry.id,
        output=str(local_path),
        quiet=False,
        resume=args.resume,
      )

  print(f"GLARE manifest: {manifest_path}")
  print(f"GLARE output:   {output_root}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
