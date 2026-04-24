#!/usr/bin/env python3

from __future__ import annotations

import argparse
import hashlib
import json
import os
import shutil
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
ASSET_DIR = ROOT / "artifacts" / "runtime" / "boot_branding"
BG_ASSET = ASSET_DIR / "bg.jpg"
SPLASH_BMP_ASSET = ASSET_DIR / "splash_embedded.bmp"

BG_TARGET = Path("/usr/comma/bg.jpg")
SPLASH_TARGET = Path("/dev/block/bootdevice/by-name/splash")
SPLASH_BMP_OFFSET = 16384

STATE_DIR = Path("/data/iqpilot_boot_branding")
BACKUP_DIR = STATE_DIR / "backups"
META_PATH = BACKUP_DIR / "metadata.json"
BG_BACKUP = BACKUP_DIR / "bg.jpg.orig"
SPLASH_BACKUP = BACKUP_DIR / "splash.partition.orig.img"


def sha256_bytes(data: bytes) -> str:
  return hashlib.sha256(data).hexdigest()


def sha256_path(path: Path) -> str:
  h = hashlib.sha256()
  with path.open("rb") as f:
    for chunk in iter(lambda: f.read(1024 * 1024), b""):
      h.update(chunk)
  return h.hexdigest()


def run(cmd: list[str], capture_output: bool = False) -> str:
  res = subprocess.run(cmd, check=True, text=True, capture_output=capture_output)
  return res.stdout.strip() if capture_output else ""


def get_root_mount_options() -> str:
  return run(["findmnt", "-no", "OPTIONS", "/"], capture_output=True)


def remount_root(mode: str) -> None:
  run(["mount", "-o", mode, "/"])


def splash_size() -> int:
  return int(run(["blockdev", "--getsize64", str(SPLASH_TARGET)], capture_output=True))


def read_splash_bytes(offset: int, size: int) -> bytes:
  with SPLASH_TARGET.open("rb") as f:
    f.seek(offset)
    return f.read(size)


def write_splash_bytes(offset: int, data: bytes) -> None:
  with SPLASH_TARGET.open("r+b", buffering=0) as f:
    f.seek(offset)
    f.write(data)
    f.flush()
    os.fsync(f.fileno())


def backup_if_missing() -> None:
  BACKUP_DIR.mkdir(parents=True, exist_ok=True)

  if not BG_BACKUP.exists():
    shutil.copy2(BG_TARGET, BG_BACKUP)

  if not SPLASH_BACKUP.exists():
    with SPLASH_TARGET.open("rb") as src, SPLASH_BACKUP.open("wb") as dst:
      remaining = splash_size()
      while remaining > 0:
        chunk = src.read(min(1024 * 1024, remaining))
        if not chunk:
          break
        dst.write(chunk)
        remaining -= len(chunk)

  if not META_PATH.exists():
    splash_backup_hash = sha256_path(SPLASH_BACKUP)
    meta = {
      "bg_backup_sha256": sha256_path(BG_BACKUP),
      "splash_backup_sha256": splash_backup_hash,
      "bg_asset_sha256": sha256_path(BG_ASSET) if BG_ASSET.exists() else None,
      "splash_asset_sha256": sha256_path(SPLASH_BMP_ASSET) if SPLASH_BMP_ASSET.exists() else None,
      "splash_bmp_offset": SPLASH_BMP_OFFSET,
    }
    META_PATH.write_text(json.dumps(meta, indent=2) + "\n", encoding="utf-8")


def apply_branding() -> bool:
  if not BG_ASSET.exists() or not SPLASH_BMP_ASSET.exists():
    return False

  backup_if_missing()

  bg_asset_bytes = BG_ASSET.read_bytes()
  splash_asset_bytes = SPLASH_BMP_ASSET.read_bytes()

  current_bg_hash = sha256_path(BG_TARGET)
  current_splash_hash = sha256_bytes(read_splash_bytes(SPLASH_BMP_OFFSET, len(splash_asset_bytes)))

  bg_asset_hash = sha256_bytes(bg_asset_bytes)
  splash_asset_hash = sha256_bytes(splash_asset_bytes)

  if current_bg_hash == bg_asset_hash and current_splash_hash == splash_asset_hash:
    return False

  root_opts = get_root_mount_options()
  remounted = False
  try:
    if current_bg_hash != bg_asset_hash:
      remount_root("remount,rw")
      remounted = True
      BG_TARGET.write_bytes(bg_asset_bytes)

    if current_splash_hash != splash_asset_hash:
      write_splash_bytes(SPLASH_BMP_OFFSET, splash_asset_bytes)
      run(["sync"])
  finally:
    if remounted:
      remount_root(f"remount,{root_opts}")

  return True


def restore_branding() -> bool:
  if not BG_BACKUP.exists() or not SPLASH_BACKUP.exists():
    return False

  root_opts = get_root_mount_options()
  remounted = False
  try:
    remount_root("remount,rw")
    remounted = True
    shutil.copy2(BG_BACKUP, BG_TARGET)

    with SPLASH_BACKUP.open("rb") as src, SPLASH_TARGET.open("r+b", buffering=0) as dst:
      while True:
        chunk = src.read(1024 * 1024)
        if not chunk:
          break
        dst.write(chunk)
      dst.flush()
      os.fsync(dst.fileno())
    run(["sync"])
  finally:
    if remounted:
      remount_root(f"remount,{root_opts}")

  return True


def status() -> dict[str, object]:
  result: dict[str, object] = {
    "bg_asset_exists": BG_ASSET.exists(),
    "splash_asset_exists": SPLASH_BMP_ASSET.exists(),
    "bg_backup_exists": BG_BACKUP.exists(),
    "splash_backup_exists": SPLASH_BACKUP.exists(),
  }
  if BG_ASSET.exists() and BG_TARGET.exists():
    result["bg_asset_sha256"] = sha256_path(BG_ASSET)
    result["bg_current_sha256"] = sha256_path(BG_TARGET)
  if SPLASH_BMP_ASSET.exists() and SPLASH_TARGET.exists():
    splash_asset_bytes = SPLASH_BMP_ASSET.read_bytes()
    result["splash_asset_sha256"] = sha256_bytes(splash_asset_bytes)
    result["splash_current_sha256"] = sha256_bytes(read_splash_bytes(SPLASH_BMP_OFFSET, len(splash_asset_bytes)))
  return result


def main() -> int:
  if os.geteuid() != 0:
    raise SystemExit("apply_boot_branding.py must run as root")

  parser = argparse.ArgumentParser()
  group = parser.add_mutually_exclusive_group()
  group.add_argument("--restore", action="store_true")
  group.add_argument("--status", action="store_true")
  args = parser.parse_args()

  if args.status:
    print(json.dumps(status(), indent=2))
    return 0

  changed = restore_branding() if args.restore else apply_branding()
  print("changed" if changed else "no-change")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
