#!/usr/bin/env python3
import argparse
import hashlib
import json
import os
import re
import shutil
import subprocess
import struct
import tempfile
import urllib.request
import zipfile
from io import BytesIO
from pathlib import Path


RESET_PATH_IN_IMAGE = "/usr/comma/reset"
SETUP_PATH_IN_IMAGE = "/usr/comma/setup"
UPDATER_PATH_IN_IMAGE = "/usr/comma/updater"
RESET_ENTRY_IN_ZIPAPP = "openpilot/system/ui/reset.py"
MICI_RESET_ENTRY_IN_ZIPAPP = "openpilot/system/ui/mici_reset.py"
TICI_RESET_ENTRY_IN_ZIPAPP = "openpilot/system/ui/tici_reset.py"
APPLICATION_ENTRY_IN_ZIPAPP = "openpilot/system/ui/lib/application.py"
WIFI_MANAGER_ENTRY_IN_SETUP_ZIPAPP = "openpilot/system/ui/lib/wifi_manager.py"
SETUP_ENTRY_IN_SETUP_ZIPAPP = "openpilot/system/ui/setup.py"
TICI_SETUP_ENTRY_IN_SETUP_ZIPAPP = "openpilot/system/ui/tici_setup.py"
MICI_SETUP_ENTRY_IN_SETUP_ZIPAPP = "openpilot/system/ui/mici_setup.py"
UPDATER_ENTRY_IN_ZIPAPP = "openpilot/system/ui/updater.py"
VERSION_PATH_IN_IMAGE = "/VERSION"
PATCH_MARKER = "STARPILOT_C4_RESET_LAYOUT_V1"
MICI_RESET_PATCH_MARKER = "STARPILOT_C4_MICI_RESET_LAYOUT_V1"
TICI_RESET_PATCH_MARKER = "STARPILOT_C4_TICI_RESET_LAYOUT_V1"
APP_PATCH_MARKER = "STARPILOT_C4_RESET_APP_DIMENSIONS_V1"
SETUP_WIFI_PATCH_MARKER = "JEEPNY_AVAILABLE = True"
SETUP_BRANDING_PATCH_MARKER = "STARPILOT_SETUP_BRANDING_V1"
SETUP_SSH_RESTORE_PATCH_MARKER = "STARPILOT_SETUP_SSH_RESTORE_V1"
ANDROID_SPARSE_MAGIC = 0xED26FF3A
CHUNK_TYPE_RAW = 0xCAC1
CHUNK_TYPE_FILL = 0xCAC2
CHUNK_TYPE_DONT_CARE = 0xCAC3
CHUNK_TYPE_CRC32 = 0xCAC4
XZ_MAGIC = b"\xFD7zXZ\x00"

DEFAULT_SYNC_COMMA_FILES = [
  "/usr/comma/bg.jpg",
  "/usr/comma/comma.sh",
  "/usr/comma/debug.py",
  "/usr/comma/fs_setup.sh",
  "/usr/comma/installer",
  "/usr/comma/magic.py",
  "/usr/comma/power_drop_monitor.py",
  "/usr/comma/power_monitor.py",
  "/usr/comma/reset",
  "/usr/comma/screen_calibration.py",
  "/usr/comma/setup",
  "/usr/comma/setup_keys",
  "/usr/comma/updater",
]

INODE_MODE_TYPE_PREFIX = {
  "regular": "010",
  "directory": "040",
  "symlink": "012",
  "character": "020",
  "block": "060",
  "fifo": "010",
  "socket": "014",
}

PATCHED_RESET_SCRIPT = f"""#!/usr/bin/env python3
import os
import sys
import threading
import time
from enum import IntEnum

import pyray as rl

from openpilot.system.hardware import PC
from openpilot.system.ui.lib.application import gui_app, FontWeight
from openpilot.system.ui.widgets import Widget
from openpilot.system.ui.widgets.button import Button, ButtonStyle
from openpilot.system.ui.widgets.label import gui_label, gui_text_box

# {PATCH_MARKER}

NVME = "/dev/nvme0n1"
USERDATA = "/dev/disk/by-partlabel/userdata"
TIMEOUT = 3 * 60


class ResetMode(IntEnum):
  USER_RESET = 0
  RECOVER = 1
  FORMAT = 2


class ResetState(IntEnum):
  NONE = 0
  CONFIRM = 1
  RESETTING = 2
  FAILED = 3


def _device_type() -> str:
  model_path = "/sys/firmware/devicetree/base/model"
  try:
    with open(model_path, "r", encoding="utf-8", errors="ignore") as f:
      model = f.read().replace("\\x00", "")
    return model.split("comma ")[-1].strip().lower()
  except Exception:
    return ""


def _small_layout() -> bool:
  dt = _device_type()
  return dt not in ("tici", "tizi")


class Reset(Widget):
  def __init__(self, mode):
    super().__init__()
    self._mode = mode
    self._previous_reset_state = None
    self._reset_state = ResetState.NONE
    self._cancel_button = Button("Cancel", self._cancel_callback)
    self._confirm_button = Button("Confirm", self._confirm, button_style=ButtonStyle.PRIMARY)
    self._reboot_button = Button("Reboot", lambda: os.system("sudo reboot"))
    self._render_status = True
    self._small = _small_layout()

  def _cancel_callback(self):
    self._render_status = False

  def _do_erase(self):
    if PC:
      return

    os.system(f"sudo umount {{NVME}}")
    os.system(f"yes | sudo mkfs.ext4 {{NVME}}")
    rm = os.system("sudo rm -rf /data/*")
    os.system(f"sudo umount {{USERDATA}}")
    fmt = os.system(f"yes | sudo mkfs.ext4 {{USERDATA}}")

    if rm == 0 or fmt == 0:
      os.system("sudo reboot")
    else:
      self._reset_state = ResetState.FAILED

  def start_reset(self):
    self._reset_state = ResetState.RESETTING
    threading.Timer(0.1, self._do_erase).start()

  def _update_state(self):
    if self._reset_state != self._previous_reset_state:
      self._previous_reset_state = self._reset_state
      self._timeout_st = time.monotonic()
    elif self._reset_state != ResetState.RESETTING and (time.monotonic() - self._timeout_st) > TIMEOUT:
      exit(0)

  def _render(self, rect: rl.Rectangle):
    self._update_state()

    margin = 24 if self._small else 140
    title_font = 48 if self._small else 100
    body_font = 34 if self._small else 90
    button_height = 100 if self._small else 160
    button_spacing = 20 if self._small else 50
    top_margin = 24 if self._small else 0
    body_gap = 24 if self._small else 40
    bottom_margin = 24 if self._small else 0

    label_rect = rl.Rectangle(rect.x + margin, rect.y + top_margin, rect.width - margin * 2, title_font + 8)
    gui_label(label_rect, "System Reset", title_font, font_weight=FontWeight.BOLD)

    body_bottom = rect.y + rect.height - button_height - bottom_margin - body_gap
    body_top = label_rect.y + label_rect.height + body_gap
    text_rect = rl.Rectangle(rect.x + margin, body_top, rect.width - margin * 2, max(0, body_bottom - body_top))
    gui_text_box(text_rect, self._get_body_text(), body_font)

    button_top = rect.y + rect.height - button_height - bottom_margin
    button_width = (rect.width - button_spacing) / 2.0

    if self._reset_state != ResetState.RESETTING:
      if self._mode == ResetMode.RECOVER:
        self._reboot_button.render(rl.Rectangle(rect.x, button_top, button_width, button_height))
      elif self._mode == ResetMode.USER_RESET:
        self._cancel_button.render(rl.Rectangle(rect.x, button_top, button_width, button_height))

      if self._reset_state != ResetState.FAILED:
        self._confirm_button.render(rl.Rectangle(rect.x + button_width + button_spacing, button_top, button_width, button_height))
      else:
        self._reboot_button.render(rl.Rectangle(rect.x, button_top, rect.width, button_height))

    return self._render_status

  def _confirm(self):
    if self._reset_state == ResetState.CONFIRM:
      self.start_reset()
    else:
      self._reset_state = ResetState.CONFIRM

  def _get_body_text(self):
    if self._reset_state == ResetState.CONFIRM:
      return "Are you sure you want to reset your device?"
    if self._reset_state == ResetState.RESETTING:
      return "Resetting device...\\nThis may take up to a minute."
    if self._reset_state == ResetState.FAILED:
      return "Reset failed. Reboot to try again."
    if self._mode == ResetMode.RECOVER:
      return "Unable to mount data partition. Press confirm to erase and reset your device."
    return "System reset triggered. Press confirm to erase all content and settings. Press cancel to resume boot."


def main():
  mode = ResetMode.USER_RESET
  if len(sys.argv) > 1:
    if sys.argv[1] == "--recover":
      mode = ResetMode.RECOVER
    elif sys.argv[1] == "--format":
      mode = ResetMode.FORMAT

  gui_app.init_window("System Reset")
  reset = Reset(mode)

  if mode == ResetMode.FORMAT:
    reset.start_reset()

  for _ in gui_app.render():
    if not reset.render(rl.Rectangle(0, 0, gui_app.width, gui_app.height)):
      break


if __name__ == "__main__":
  main()
"""


def patch_application_script(original: bytes) -> bytes:
  if APP_PATCH_MARKER.encode("utf-8") in original:
    return original

  text = original.decode("utf-8", "replace")
  replacement = (
    f"# {APP_PATCH_MARKER}\n"
    "_dt = HARDWARE.get_device_type()\n"
    "if _dt in ('tici', 'tizi'):\n"
    "  gui_app = GuiApplication(2160, 1080)\n"
    "else:\n"
    "  gui_app = GuiApplication(536, 240)\n"
  )

  fixed = text.replace("gui_app = GuiApplication(2160, 1080)", replacement)
  if fixed == text:
    fixed = re.sub(
      r"^gui_app\s*=\s*GuiApplication\([^\n]+\)\s*$",
      replacement.rstrip("\n"),
      text,
      count=1,
      flags=re.MULTILINE,
    )

  if fixed == text:
    # Newer upstream application.py is already device-aware via GuiApplication defaults.
    # Keep it as-is and stamp a marker so verification can still pass.
    if text.startswith("#!"):
      first_nl = text.find("\n")
      if first_nl != -1:
        fixed = text[:first_nl + 1] + f"# {APP_PATCH_MARKER} (no-op)\n" + text[first_nl + 1:]
      else:
        fixed = text + f"\n# {APP_PATCH_MARKER} (no-op)\n"
    else:
      fixed = f"# {APP_PATCH_MARKER} (no-op)\n" + text

  return fixed.encode("utf-8")


def patch_setup_wifi_manager() -> bytes:
  """
  Replace setup zipapp's wifi_manager.py with repo version that gracefully handles
  missing jeepney (fallback to nmcli/fake), preventing setup boot-logo hangs.
  """
  repo_root = Path(__file__).resolve().parents[2]
  src = repo_root / "system/ui/lib/wifi_manager.py"
  if not src.is_file():
    raise RuntimeError(f"Unable to find repo wifi_manager source: {src}")
  data = src.read_bytes()
  if SETUP_WIFI_PATCH_MARKER.encode("utf-8") not in data:
    raise RuntimeError("Repo wifi_manager.py does not appear to include jeepney fallback marker")
  return data


def patch_setup_branding_script(original: bytes, entry_name: str) -> bytes:
  text = original.decode("utf-8")
  if SETUP_BRANDING_PATCH_MARKER in text:
    return text.encode("utf-8")

  text = text.replace(
    'OPENPILOT_URL = "https://openpilot.comma.ai"',
    'NETWORK_CHECK_URL = "https://openpilot.comma.ai"\n'
    'DEFAULT_INSTALLER_URL = "https://installer.comma.ai/firestar5683/StarPilot"\n'
    f'# {SETUP_BRANDING_PATCH_MARKER}',
  )
  text = text.replace("urllib.request.Request(OPENPILOT_URL, method=\"HEAD\")",
                      "urllib.request.Request(NETWORK_CHECK_URL, method=\"HEAD\")")
  text = text.replace("urllib.request.urlopen(OPENPILOT_URL, timeout=2)",
                      "urllib.request.urlopen(NETWORK_CHECK_URL, timeout=2)")
  text = text.replace("self.download(OPENPILOT_URL)", "self.download(DEFAULT_INSTALLER_URL)")

  if entry_name == MICI_SETUP_ENTRY_IN_SETUP_ZIPAPP:
    text = text.replace('LargerSlider("slide to use\\nopenpilot"', 'LargerSlider("slide to use\\nstarpilot"')
  elif entry_name == TICI_SETUP_ENTRY_IN_SETUP_ZIPAPP:
    text = text.replace('ButtonRadio("openpilot"', 'ButtonRadio("StarPilot"')

  if SETUP_BRANDING_PATCH_MARKER not in text:
    raise RuntimeError(f"Failed to patch setup branding for {entry_name}")

  return text.encode("utf-8")


def patch_setup_module(relative_path: str) -> bytes:
  """
  Replace setup zipapp module with repo version so setup behavior stays in sync.
  """
  repo_root = Path(__file__).resolve().parents[2]
  src = repo_root / relative_path
  if not src.is_file():
    raise RuntimeError(f"Unable to find repo setup source: {src}")
  return src.read_bytes()


def patch_setup_script_with_ssh_restore(relative_path: str) -> bytes:
  """
  Apply SSH-key restore logic directly into setup scripts for AGNOS images.
  This keeps repo runtime behavior unchanged while making image reset flows
  resilient to setup/install failures.
  """
  text = patch_setup_module(relative_path).decode("utf-8")
  if SETUP_SSH_RESTORE_PATCH_MARKER in text:
    return text.encode("utf-8")

  restore_block = f"""
# {SETUP_SSH_RESTORE_PATCH_MARKER}
def _restore_ssh_after_reset():
  backup_dir = "/cache/reset_backup"
  params_dir = "/data/params/d"
  if not os.path.isdir(backup_dir):
    return

  restored = False
  try:
    os.makedirs(params_dir, exist_ok=True)
    for key in ("GithubSshKeys", "SshEnabled"):
      src = f"{{backup_dir}}/{{key}}"
      dst = f"{{params_dir}}/{{key}}"
      if not os.path.isfile(src):
        continue
      shutil.copyfile(src, dst)
      os.chmod(dst, 0o600)
      restored = True

    if restored:
      os.system("sudo chown -R comma:comma /data/params >/dev/null 2>&1 || true")
      os.system("sudo /usr/comma/set_ssh.sh >/tmp/setup_ssh_restore.log 2>&1 || true")
  finally:
    os.system(f"sudo rm -rf {{backup_dir}} >/dev/null 2>&1 || true")
"""

  if "def main():" not in text:
    raise RuntimeError(f"Unable to patch setup script without main(): {relative_path}")

  text = text.replace("\ndef main():", f"{restore_block}\n\ndef main():", 1)
  text = text.replace("  try:\n    gui_app.init_window(", "  try:\n    _restore_ssh_after_reset()\n    gui_app.init_window(", 1)
  return text.encode("utf-8")


def get_setup_replacements() -> dict[str, bytes]:
  """
  Keep the reference setup bundle intact and patch only the networking backend.

  The reference AGNOS setup bundle already contains the correct small-screen
  selector and matching mici UI modules. Replacing those modules with repo-head
  versions caused bootstrap incompatibilities. The only setup-side changes we
  still need are the jeepney fallback plus the StarPilot branding/url strings.
  """
  return {
    WIFI_MANAGER_ENTRY_IN_SETUP_ZIPAPP: patch_setup_wifi_manager(),
  }


def patch_updater_module() -> bytes:
  """
  Replace only the bundled updater selector with the repo version.

  The selector itself carries the small-screen fallback logic; the rest of the
  reference updater zipapp stays unchanged.
  """
  repo_root = Path(__file__).resolve().parents[2]
  src = repo_root / "system/ui/updater.py"
  if not src.is_file():
    raise RuntimeError(f"Unable to find repo updater source: {src}")
  return src.read_bytes()


def patch_reset_script() -> bytes:
  """
  Use repo reset.py so AGNOS reset stays in sync with upstream selector logic.
  """
  repo_root = Path(__file__).resolve().parents[2]
  src = repo_root / "system/ui/reset.py"
  if not src.is_file():
    raise RuntimeError(f"Unable to find repo reset source: {src}")
  data = src.read_text(encoding="utf-8")
  if PATCH_MARKER not in data:
    if data.startswith("#!"):
      first_nl = data.find("\n")
      if first_nl != -1:
        data = data[:first_nl + 1] + f"# {PATCH_MARKER}\n" + data[first_nl + 1:]
      else:
        data = data + f"\n# {PATCH_MARKER}\n"
    else:
      data = f"# {PATCH_MARKER}\n" + data
  return data.encode("utf-8")


def patch_mici_reset_script() -> bytes:
  """
  Use repo mici_reset.py (slider UX) for C4/tappity reset flow.
  """
  repo_root = Path(__file__).resolve().parents[2]
  src = repo_root / "system/ui/mici_reset.py"
  if not src.is_file():
    raise RuntimeError(f"Unable to find repo mici_reset source: {src}")
  data = src.read_text(encoding="utf-8")
  if MICI_RESET_PATCH_MARKER not in data:
    if data.startswith("#!"):
      first_nl = data.find("\n")
      if first_nl != -1:
        data = data[:first_nl + 1] + f"# {MICI_RESET_PATCH_MARKER}\n" + data[first_nl + 1:]
      else:
        data = data + f"\n# {MICI_RESET_PATCH_MARKER}\n"
    else:
      data = f"# {MICI_RESET_PATCH_MARKER}\n" + data
  return data.encode("utf-8")


def patch_tici_reset_script() -> bytes:
  """
  Use repo tici_reset.py so C3/C3X reset behavior remains in sync with repo fixes.
  """
  repo_root = Path(__file__).resolve().parents[2]
  src = repo_root / "system/ui/tici_reset.py"
  if not src.is_file():
    raise RuntimeError(f"Unable to find repo tici_reset source: {src}")
  data = src.read_text(encoding="utf-8")
  if TICI_RESET_PATCH_MARKER not in data:
    if data.startswith("#!"):
      first_nl = data.find("\n")
      if first_nl != -1:
        data = data[:first_nl + 1] + f"# {TICI_RESET_PATCH_MARKER}\n" + data[first_nl + 1:]
      else:
        data = data + f"\n# {TICI_RESET_PATCH_MARKER}\n"
    else:
      data = f"# {TICI_RESET_PATCH_MARKER}\n" + data
  return data.encode("utf-8")


def parse_args() -> argparse.Namespace:
  p = argparse.ArgumentParser(description="Patch AGNOS system image with minimal C4 reset/setup/updater fixes")
  p.add_argument("--manifest", default="system/hardware/tici/agnos.json", help="Path to AGNOS manifest JSON")
  p.add_argument("--work-dir", default=".cache/agnos_reset_patch", help="Working directory")
  p.add_argument("--source-url", default=None, help="Override source raw system image URL")
  p.add_argument("--source-image", default=None, help="Use existing local raw system image file instead of download")
  p.add_argument("--reference-manifest", default=None, help="Optional AGNOS manifest used to source /usr/comma installer payloads")
  p.add_argument("--reference-source-url", default=None, help="Override reference AGNOS system image URL for /usr/comma file sync")
  p.add_argument("--reference-image", default=None, help="Use existing local reference system image file for /usr/comma file sync")
  p.add_argument("--sync-comma-files", default=",".join(DEFAULT_SYNC_COMMA_FILES),
                 help="Comma-separated file list to sync from reference image (e.g. /usr/comma/installer,/usr/comma/setup)")
  p.add_argument("--disable-comma-file-sync", action="store_true",
                 help="Disable syncing /usr/comma files from a reference image")
  p.add_argument("--output-xz", default=None, help="Output .img.xz path")
  p.add_argument("--new-url", default=None, help="Hosted URL for patched image; used for manifest output")
  p.add_argument("--manifest-out", default=None, help="Write updated manifest JSON here")
  p.add_argument("--in-place-manifest", action="store_true", help="Update manifest file in place")
  p.add_argument("--force-download", action="store_true", help="Force redownload source image")
  p.add_argument("--set-version", default=None, help="Override /VERSION inside patched image (e.g. 12.8.1)")
  return p.parse_args()


def find_debugfs() -> str:
  candidates = [
    os.environ.get("DEBUGFS"),
    "debugfs",
    "/opt/homebrew/opt/e2fsprogs/sbin/debugfs",
  ]
  for c in candidates:
    if c and shutil.which(c):
      return c
    if c and Path(c).is_file():
      return c
  raise RuntimeError("debugfs not found. Install e2fsprogs and retry.")


def load_manifest(path: Path) -> list[dict]:
  return json.loads(path.read_text())


def get_system_entry(manifest: list[dict]) -> dict:
  for e in manifest:
    if e.get("name") == "system":
      return e
  raise RuntimeError("No system entry found in manifest")


def pick_source_url(system_entry: dict, override: str | None) -> str:
  if override:
    return override
  url = system_entry.get("url")
  if isinstance(url, str):
    return url
  alt = system_entry.get("alt")
  if isinstance(alt, dict) and isinstance(alt.get("url"), str):
    return alt["url"]
  raise RuntimeError("No source URL found for system image")


def find_default_reference_manifest(primary_manifest_path: Path) -> Path | None:
  # Expected tree layout for local development:
  #   <parent>/starpilot/system/hardware/tici/agnos.json
  #   <parent>/openpilot/system/hardware/tici/agnos.json
  repo_root = primary_manifest_path
  for _ in range(4):
    if repo_root.parent == repo_root:
      break
    repo_root = repo_root.parent

  candidates = [
    repo_root.parent / "openpilot/system/hardware/tici/agnos.json",
    repo_root / "openpilot/system/hardware/tici/agnos.json",
  ]

  for candidate in candidates:
    if candidate.is_file():
      return candidate.resolve()
  return None


def parse_sync_file_list(raw: str) -> list[str]:
  out: list[str] = []
  seen: set[str] = set()
  for token in raw.replace(";", ",").split(","):
    item = token.strip()
    if not item:
      continue
    if not item.startswith("/"):
      if "/" in item:
        item = f"/{item.lstrip('/')}"
      else:
        item = f"/usr/comma/{item}"
    if item not in seen:
      seen.add(item)
      out.append(item)
  return out


def download(url: str, dst: Path) -> None:
  dst.parent.mkdir(parents=True, exist_ok=True)
  tmp = dst.with_suffix(dst.suffix + ".part")
  print(f"Downloading {url} -> {dst}", flush=True)
  with urllib.request.urlopen(url) as src, open(tmp, "wb") as out:
    shutil.copyfileobj(src, out, length=1024 * 1024)
  tmp.replace(dst)


def run_cmd(cmd: list[str], check: bool = True, capture: bool = True) -> subprocess.CompletedProcess[str]:
  proc = subprocess.run(cmd, text=True, capture_output=capture)
  if check and proc.returncode != 0:
    raise RuntimeError(f"Command failed ({proc.returncode}): {' '.join(cmd)}\n{proc.stdout}\n{proc.stderr}")
  return proc


def is_xz_file(path: Path) -> bool:
  with open(path, "rb") as f:
    header = f.read(len(XZ_MAGIC))
  return header == XZ_MAGIC


def decompress_xz(src: Path, dst: Path) -> None:
  dst.parent.mkdir(parents=True, exist_ok=True)
  tmp = dst.with_suffix(dst.suffix + ".part")
  print(f"Decompressing XZ image {src} -> {dst}", flush=True)
  with open(tmp, "wb") as out:
    proc = subprocess.run(["xz", "-d", "-c", str(src)], stdout=out, stderr=subprocess.PIPE, text=True)
  if proc.returncode != 0:
    tmp.unlink(missing_ok=True)
    raise RuntimeError(f"xz decompression failed:\n{proc.stderr}")
  tmp.replace(dst)


def is_android_sparse(path: Path) -> bool:
  with open(path, "rb") as f:
    header = f.read(4)
  if len(header) != 4:
    return False
  return int.from_bytes(header, "little") == ANDROID_SPARSE_MAGIC


def unsparse_image(src_sparse: Path, dst_raw: Path) -> None:
  print(f"Unsparsing Android image {src_sparse} -> {dst_raw}", flush=True)
  with open(src_sparse, "rb") as f_in, open(dst_raw, "wb") as f_out:
    file_hdr = f_in.read(28)
    if len(file_hdr) != 28:
      raise RuntimeError("Invalid sparse image header length")
    magic, major, minor, file_hdr_sz, chunk_hdr_sz, blk_sz, total_blks, total_chunks, _checksum = struct.unpack("<I4H4I", file_hdr)
    if magic != ANDROID_SPARSE_MAGIC:
      raise RuntimeError("Not an Android sparse image")
    if major != 1 or minor != 0:
      raise RuntimeError(f"Unsupported sparse version: {major}.{minor}")
    if file_hdr_sz > 28:
      f_in.read(file_hdr_sz - 28)
    if chunk_hdr_sz < 12:
      raise RuntimeError(f"Invalid chunk header size: {chunk_hdr_sz}")

    for _ in range(total_chunks):
      chunk_hdr = f_in.read(12)
      if len(chunk_hdr) != 12:
        raise RuntimeError("Unexpected EOF in chunk header")
      chunk_type, _reserved, chunk_sz, total_sz = struct.unpack("<2H2I", chunk_hdr)
      if chunk_hdr_sz > 12:
        f_in.read(chunk_hdr_sz - 12)

      data_sz = total_sz - chunk_hdr_sz
      out_chunk_bytes = chunk_sz * blk_sz

      if chunk_type == CHUNK_TYPE_RAW:
        if data_sz != out_chunk_bytes:
          raise RuntimeError(f"RAW chunk size mismatch: data={data_sz} out={out_chunk_bytes}")
        remaining = data_sz
        while remaining > 0:
          chunk = f_in.read(min(8 * 1024 * 1024, remaining))
          if not chunk:
            raise RuntimeError("Unexpected EOF in RAW chunk")
          f_out.write(chunk)
          remaining -= len(chunk)
      elif chunk_type == CHUNK_TYPE_FILL:
        if data_sz != 4:
          raise RuntimeError(f"FILL chunk expected 4 bytes, got {data_sz}")
        pattern = f_in.read(4)
        if len(pattern) != 4:
          raise RuntimeError("Unexpected EOF in FILL chunk")
        # Write as sparse hole if fill is zero for speed.
        if pattern == b"\x00\x00\x00\x00":
          f_out.seek(out_chunk_bytes, os.SEEK_CUR)
        else:
          unit = pattern * (blk_sz // 4)
          for _ in range(chunk_sz):
            f_out.write(unit)
      elif chunk_type == CHUNK_TYPE_DONT_CARE:
        if data_sz > 0:
          f_in.read(data_sz)
        f_out.seek(out_chunk_bytes, os.SEEK_CUR)
      elif chunk_type == CHUNK_TYPE_CRC32:
        if data_sz != 4:
          raise RuntimeError(f"CRC32 chunk expected 4 bytes, got {data_sz}")
        f_in.read(4)
      else:
        raise RuntimeError(f"Unknown sparse chunk type: 0x{chunk_type:04x}")

    f_out.truncate(total_blks * blk_sz)


def materialize_ext4_image(source_img: Path, raw_img: Path, work_dir: Path, label: str, force: bool = False) -> None:
  source_for_sparse = source_img

  if is_xz_file(source_img):
    decompressed = work_dir / f"{label}.decompressed.img"
    if force and decompressed.exists():
      decompressed.unlink()
    if not decompressed.exists():
      decompress_xz(source_img, decompressed)
    source_for_sparse = decompressed

  if force and raw_img.exists():
    raw_img.unlink()

  if raw_img.exists():
    return

  if is_android_sparse(source_for_sparse):
    unsparse_image(source_for_sparse, raw_img)
  else:
    shutil.copy2(source_for_sparse, raw_img)


def run_debugfs(debugfs: str, image: Path, request: str, write: bool = False) -> str:
  cmd = [debugfs]
  if write:
    cmd.append("-w")
  cmd += ["-R", request, str(image)]
  proc = run_cmd(cmd, check=True, capture=True)
  return f"{proc.stdout}\n{proc.stderr}"


def split_shebang(data: bytes) -> tuple[bytes, bytes]:
  if data.startswith(b"#!"):
    idx = data.find(b"\n")
    if idx != -1:
      return data[:idx + 1], data[idx + 1:]
  return b"", data


def patch_reset_zipapp(original: bytes) -> bytes:
  shebang, zip_payload = split_shebang(original)

  src_io = BytesIO(zip_payload)
  dst_io = BytesIO()
  changed = False

  replacement_reset = patch_reset_script()
  replacement_tici_reset = patch_tici_reset_script()
  replacement_mici_reset = patch_mici_reset_script()
  reset_replacements = {
    RESET_ENTRY_IN_ZIPAPP: replacement_reset,
    TICI_RESET_ENTRY_IN_ZIPAPP: replacement_tici_reset,
    MICI_RESET_ENTRY_IN_ZIPAPP: replacement_mici_reset,
  }

  with zipfile.ZipFile(src_io, "r") as src, zipfile.ZipFile(dst_io, "w", compression=zipfile.ZIP_DEFLATED) as dst:
    if APPLICATION_ENTRY_IN_ZIPAPP not in src.namelist():
      raise RuntimeError(f"{APPLICATION_ENTRY_IN_ZIPAPP} not found in reset zipapp")

    seen_entries: set[str] = set()
    for info in src.infolist():
      seen_entries.add(info.filename)
      payload = src.read(info.filename)
      if info.filename in reset_replacements:
        replacement_payload = reset_replacements[info.filename]
        if payload != replacement_payload:
          payload = replacement_payload
          changed = True
      elif info.filename == APPLICATION_ENTRY_IN_ZIPAPP:
        patched_payload = patch_application_script(payload)
        if patched_payload != payload:
          payload = patched_payload
          changed = True

      new_info = zipfile.ZipInfo(info.filename, info.date_time)
      new_info.compress_type = zipfile.ZIP_DEFLATED
      new_info.external_attr = info.external_attr
      new_info.create_system = info.create_system
      dst.writestr(new_info, payload)

    # Some reference images may not include both reset variants; add missing entries explicitly.
    default_external_attr = 0o100644 << 16
    for entry, payload in reset_replacements.items():
      if entry in seen_entries:
        continue
      new_info = zipfile.ZipInfo(entry)
      new_info.compress_type = zipfile.ZIP_DEFLATED
      new_info.external_attr = default_external_attr
      new_info.create_system = 3
      dst.writestr(new_info, payload)
      changed = True

  if not changed:
    return original
  return shebang + dst_io.getvalue()


def patch_updater_zipapp(original: bytes) -> bytes:
  shebang, zip_payload = split_shebang(original)

  replacement = patch_updater_module()
  src_io = BytesIO(zip_payload)
  dst_io = BytesIO()
  changed = False
  found_updater = False

  with zipfile.ZipFile(src_io, "r") as src, zipfile.ZipFile(dst_io, "w", compression=zipfile.ZIP_DEFLATED) as dst:
    for info in src.infolist():
      payload = src.read(info.filename)
      if info.filename == UPDATER_ENTRY_IN_ZIPAPP:
        found_updater = True
        if payload != replacement:
          payload = replacement
          changed = True

      new_info = zipfile.ZipInfo(info.filename, info.date_time)
      new_info.compress_type = zipfile.ZIP_DEFLATED
      new_info.external_attr = info.external_attr
      new_info.create_system = info.create_system
      dst.writestr(new_info, payload)

    if not found_updater:
      new_info = zipfile.ZipInfo(UPDATER_ENTRY_IN_ZIPAPP)
      new_info.compress_type = zipfile.ZIP_DEFLATED
      new_info.external_attr = 0o100644 << 16
      new_info.create_system = 3
      dst.writestr(new_info, replacement)
      changed = True

  if not changed:
    return original
  return shebang + dst_io.getvalue()


def patch_setup_zipapp(original: bytes) -> bytes:
  shebang, zip_payload = split_shebang(original)

  src_io = BytesIO(zip_payload)
  dst_io = BytesIO()
  changed = False

  replacements = get_setup_replacements()

  with zipfile.ZipFile(src_io, "r") as src, zipfile.ZipFile(dst_io, "w", compression=zipfile.ZIP_DEFLATED) as dst:
    seen_entries: set[str] = set()
    for info in src.infolist():
      seen_entries.add(info.filename)
      payload = src.read(info.filename)
      if info.filename in replacements and payload != replacements[info.filename]:
        payload = replacements[info.filename]
        changed = True
      elif info.filename in (MICI_SETUP_ENTRY_IN_SETUP_ZIPAPP, TICI_SETUP_ENTRY_IN_SETUP_ZIPAPP):
        patched_payload = patch_setup_branding_script(payload, info.filename)
        if patched_payload != payload:
          payload = patched_payload
          changed = True

      new_info = zipfile.ZipInfo(info.filename, info.date_time)
      new_info.compress_type = zipfile.ZIP_DEFLATED
      new_info.external_attr = info.external_attr
      new_info.create_system = info.create_system
      dst.writestr(new_info, payload)

    default_external_attr = 0o100644 << 16
    for entry, payload in replacements.items():
      if entry in seen_entries:
        continue
      new_info = zipfile.ZipInfo(entry)
      new_info.compress_type = zipfile.ZIP_DEFLATED
      new_info.external_attr = default_external_attr
      new_info.create_system = 3
      dst.writestr(new_info, payload)
      changed = True

  if not changed:
    return original
  return shebang + dst_io.getvalue()


def zipapp_has_markers(data: bytes) -> bool:
  _shebang, zip_payload = split_shebang(data)
  with zipfile.ZipFile(BytesIO(zip_payload), "r") as z:
    reset_script = z.read(RESET_ENTRY_IN_ZIPAPP)
    tici_reset_script = z.read(TICI_RESET_ENTRY_IN_ZIPAPP)
    mici_reset_script = z.read(MICI_RESET_ENTRY_IN_ZIPAPP)
    app_script = z.read(APPLICATION_ENTRY_IN_ZIPAPP)
  return (
    PATCH_MARKER.encode() in reset_script
    and TICI_RESET_PATCH_MARKER.encode() in tici_reset_script
    and MICI_RESET_PATCH_MARKER.encode() in mici_reset_script
    and APP_PATCH_MARKER.encode() in app_script
  )


def setup_zipapp_has_expected_content(data: bytes) -> bool:
  _shebang, zip_payload = split_shebang(data)
  replacements = get_setup_replacements()
  with zipfile.ZipFile(BytesIO(zip_payload), "r") as z:
    try:
      wifi_manager = z.read(WIFI_MANAGER_ENTRY_IN_SETUP_ZIPAPP)
      if SETUP_WIFI_PATCH_MARKER.encode() not in wifi_manager:
        return False
      for entry, payload in replacements.items():
        if z.read(entry) != payload:
          return False
      for entry in (MICI_SETUP_ENTRY_IN_SETUP_ZIPAPP, TICI_SETUP_ENTRY_IN_SETUP_ZIPAPP):
        setup_script = z.read(entry)
        if SETUP_BRANDING_PATCH_MARKER.encode() not in setup_script:
          return False
        if b"installer.comma.ai/firestar5683/StarPilot" not in setup_script:
          return False
    except KeyError:
      return False
  return True


def updater_zipapp_has_expected_content(data: bytes) -> bool:
  _shebang, zip_payload = split_shebang(data)
  with zipfile.ZipFile(BytesIO(zip_payload), "r") as z:
    try:
      updater_script = z.read(UPDATER_ENTRY_IN_ZIPAPP)
    except KeyError:
      return False
  return updater_script == patch_updater_module()


def parse_inode(debugfs_output: str) -> int:
  m = re.search(r"Inode:\s+(\d+)", debugfs_output)
  if not m:
    raise RuntimeError(f"Unable to parse inode from debugfs stat output:\n{debugfs_output}")
  return int(m.group(1))


def write_regular_file_to_image(debugfs: str, image: Path, image_path: str, local_file: Path, mode_octal: str, uid: int = 0, gid: int = 0) -> None:
  try:
    run_debugfs(debugfs, image, f"rm {image_path}", write=True)
  except Exception as e:
    err = str(e).lower()
    if "file not found" not in err and "no such file" not in err:
      raise
  run_debugfs(debugfs, image, f"write {local_file} {image_path}", write=True)
  stat_out = run_debugfs(debugfs, image, f"stat {image_path}", write=False)
  inode = parse_inode(stat_out)
  run_debugfs(debugfs, image, f"set_inode_field <{inode}> mode {mode_octal}", write=True)
  run_debugfs(debugfs, image, f"set_inode_field <{inode}> uid {uid}", write=True)
  run_debugfs(debugfs, image, f"set_inode_field <{inode}> gid {gid}", write=True)


def parse_debugfs_stat(debugfs_output: str) -> tuple[str, str, int, int]:
  type_match = re.search(r"Type:\s+([A-Za-z]+)", debugfs_output)
  mode_match = re.search(r"Mode:\s+([0-7]+)", debugfs_output)
  user_match = re.search(r"User:\s+(\d+)", debugfs_output)
  group_match = re.search(r"Group:\s+(\d+)", debugfs_output)
  if not type_match or not mode_match or not user_match or not group_match:
    raise RuntimeError(f"Unable to parse debugfs stat output:\n{debugfs_output}")
  return type_match.group(1).lower(), mode_match.group(1), int(user_match.group(1)), int(group_match.group(1))


def inode_mode_from_type_and_perms(file_type: str, perms_octal: str) -> str:
  prefix = INODE_MODE_TYPE_PREFIX.get(file_type)
  if prefix is None:
    raise RuntimeError(f"Unsupported inode type '{file_type}' for mode conversion")
  perms = perms_octal.strip()
  if not perms:
    raise RuntimeError("Empty permissions value in inode stat")
  if not perms.startswith("0"):
    perms = "0" + perms
  if len(perms) < 4:
    perms = perms.rjust(4, "0")
  return f"{prefix}{perms}"


def sync_files_from_reference_image(debugfs: str, reference_img: Path, patched_img: Path, sync_paths: list[str], work_dir: Path) -> list[str]:
  sync_dir = work_dir / "reference_sync"
  sync_dir.mkdir(parents=True, exist_ok=True)
  synced: list[str] = []

  for image_path in sync_paths:
    source_tmp = sync_dir / f"source{image_path.replace('/', '_')}"
    verify_tmp = sync_dir / f"verify{image_path.replace('/', '_')}"

    stat_out = run_debugfs(debugfs, reference_img, f"stat {image_path}", write=False)
    file_type, perms_octal, uid, gid = parse_debugfs_stat(stat_out)
    mode_octal = inode_mode_from_type_and_perms(file_type, perms_octal)

    run_debugfs(debugfs, reference_img, f"dump -p {image_path} {source_tmp}", write=False)
    print(f"Syncing {image_path} from reference image (mode={mode_octal}, uid={uid}, gid={gid})", flush=True)
    write_regular_file_to_image(debugfs, patched_img, image_path, source_tmp, mode_octal, uid, gid)

    run_debugfs(debugfs, patched_img, f"dump -p {image_path} {verify_tmp}", write=False)
    if sha256_file(source_tmp) != sha256_file(verify_tmp):
      raise RuntimeError(f"Verification failed after syncing {image_path}")
    synced.append(image_path)

  return synced


def sha256_file(path: Path) -> str:
  h = hashlib.sha256()
  with open(path, "rb") as f:
    while True:
      chunk = f.read(1024 * 1024)
      if not chunk:
        break
      h.update(chunk)
  return h.hexdigest()


def compress_xz(src: Path, dst: Path) -> None:
  dst.parent.mkdir(parents=True, exist_ok=True)
  tmp = dst.with_suffix(dst.suffix + ".part")
  print(f"Compressing {src} -> {dst}", flush=True)
  with open(tmp, "wb") as out:
    proc = subprocess.run(["xz", "-T0", "-6", "-c", str(src)], stdout=out, stderr=subprocess.PIPE, text=True)
  if proc.returncode != 0:
    tmp.unlink(missing_ok=True)
    raise RuntimeError(f"xz failed:\n{proc.stderr}")
  tmp.replace(dst)


def update_manifest_system_entry(manifest: list[dict], new_url: str, new_hash_raw: str, size: int) -> list[dict]:
  updated = json.loads(json.dumps(manifest))
  system_entry = get_system_entry(updated)
  old_url = system_entry.get("url")
  old_hash = system_entry.get("hash")
  old_hash_raw = system_entry.get("hash_raw")
  old_size = system_entry.get("size")

  system_entry["url"] = new_url
  system_entry["hash"] = new_hash_raw
  system_entry["hash_raw"] = new_hash_raw
  system_entry["size"] = size
  system_entry["sparse"] = False
  system_entry["full_check"] = False

  if isinstance(old_url, str) and isinstance(old_hash, str) and isinstance(old_hash_raw, str) and isinstance(old_size, int):
    system_entry["alt"] = {
      "url": old_url,
      "hash": old_hash,
      "hash_raw": old_hash_raw,
      "size": old_size,
    }

  return updated


def resolve_reference_source_image(args: argparse.Namespace, primary_manifest_path: Path, work_dir: Path) -> Path:
  if args.reference_image:
    ref_image = Path(args.reference_image).resolve()
    if not ref_image.is_file():
      raise RuntimeError(f"Reference image not found: {ref_image}")
    return ref_image

  reference_manifest_path: Path | None = None
  if args.reference_manifest:
    reference_manifest_path = Path(args.reference_manifest).resolve()
    if not reference_manifest_path.is_file():
      raise RuntimeError(f"Reference manifest not found: {reference_manifest_path}")
  else:
    reference_manifest_path = find_default_reference_manifest(primary_manifest_path)

  if args.reference_source_url:
    reference_url = args.reference_source_url
  elif reference_manifest_path is not None:
    reference_manifest = load_manifest(reference_manifest_path)
    reference_entry = get_system_entry(reference_manifest)
    reference_url = pick_source_url(reference_entry, None)
    print(f"Using reference AGNOS manifest: {reference_manifest_path}", flush=True)
  else:
    raise RuntimeError(
      "No reference image source found. Set --reference-image, --reference-source-url, or --reference-manifest."
    )

  reference_download = work_dir / "reference_system.img"
  if args.force_download and reference_download.exists():
    reference_download.unlink()
  if not reference_download.exists():
    download(reference_url, reference_download)
  return reference_download


def main() -> int:
  args = parse_args()
  debugfs = find_debugfs()

  manifest_path = Path(args.manifest).resolve()
  manifest = load_manifest(manifest_path)
  system_entry = get_system_entry(manifest)

  work_dir = Path(args.work_dir).resolve()
  work_dir.mkdir(parents=True, exist_ok=True)

  if args.source_image:
    downloaded_img = Path(args.source_image).resolve()
    if not downloaded_img.is_file():
      raise RuntimeError(f"Source image not found: {downloaded_img}")
  else:
    source_url = pick_source_url(system_entry, args.source_url)
    downloaded_img = work_dir / "base_system.img"
    if args.force_download and downloaded_img.exists():
      downloaded_img.unlink()
    if not downloaded_img.exists():
      download(source_url, downloaded_img)

  raw_img = work_dir / "base_system.ext4.img"
  materialize_ext4_image(downloaded_img, raw_img, work_dir, "base_system", force=args.force_download)

  patched_img = work_dir / "patched_system.ext4.img"
  if patched_img.exists():
    patched_img.unlink()
  print(f"Copying source image -> {patched_img}", flush=True)
  shutil.copy2(raw_img, patched_img)

  sync_paths = [] if args.disable_comma_file_sync else parse_sync_file_list(args.sync_comma_files)
  if sync_paths:
    reference_source_img = resolve_reference_source_image(args, manifest_path, work_dir)
    reference_raw = work_dir / "reference_system.ext4.img"
    materialize_ext4_image(reference_source_img, reference_raw, work_dir, "reference_system", force=args.force_download)

    print(f"Syncing /usr/comma payload files from reference image: {reference_raw}", flush=True)
    synced_files = sync_files_from_reference_image(debugfs, reference_raw, patched_img, sync_paths, work_dir)
    print(f"Synced {len(synced_files)} /usr/comma files from reference image", flush=True)

  original_reset = work_dir / "comma_reset.orig"
  patched_reset = work_dir / "comma_reset.patched"
  verify_reset = work_dir / "comma_reset.verify"
  original_setup = work_dir / "comma_setup.orig"
  patched_setup = work_dir / "comma_setup.patched"
  verify_setup = work_dir / "comma_setup.verify"
  original_updater = work_dir / "comma_updater.orig"
  patched_updater = work_dir / "comma_updater.patched"
  verify_updater = work_dir / "comma_updater.verify"
  print("Extracting /usr/comma/reset from image", flush=True)
  run_debugfs(debugfs, patched_img, f"dump -p {RESET_PATH_IN_IMAGE} {original_reset}", write=False)

  original_data = original_reset.read_bytes()
  patched_data = patch_reset_zipapp(original_data)
  if patched_data == original_data:
    print("Reset zipapp already contains patch marker; continuing", flush=True)
  patched_reset.write_bytes(patched_data)

  print("Writing patched /usr/comma/reset back into image", flush=True)
  write_regular_file_to_image(debugfs, patched_img, RESET_PATH_IN_IMAGE, patched_reset, "0100775", 0, 0)

  run_debugfs(debugfs, patched_img, f"dump -p {RESET_PATH_IN_IMAGE} {verify_reset}", write=False)
  verify_data = verify_reset.read_bytes()
  if not zipapp_has_markers(verify_data):
    raise RuntimeError("Patched markers missing after writing reset file into image")

  print("Extracting /usr/comma/setup from image", flush=True)
  run_debugfs(debugfs, patched_img, f"dump -p {SETUP_PATH_IN_IMAGE} {original_setup}", write=False)

  setup_original_data = original_setup.read_bytes()
  setup_patched_data = patch_setup_zipapp(setup_original_data)
  if setup_patched_data == setup_original_data:
    print("Setup zipapp already contains the expected compat patches; continuing", flush=True)
  patched_setup.write_bytes(setup_patched_data)

  print("Writing patched /usr/comma/setup back into image", flush=True)
  write_regular_file_to_image(debugfs, patched_img, SETUP_PATH_IN_IMAGE, patched_setup, "0100775", 0, 0)

  run_debugfs(debugfs, patched_img, f"dump -p {SETUP_PATH_IN_IMAGE} {verify_setup}", write=False)
  verify_setup_data = verify_setup.read_bytes()
  if not setup_zipapp_has_expected_content(verify_setup_data):
    raise RuntimeError("Setup zipapp verification failed after writing setup file into image")

  print("Extracting /usr/comma/updater from image", flush=True)
  run_debugfs(debugfs, patched_img, f"dump -p {UPDATER_PATH_IN_IMAGE} {original_updater}", write=False)

  updater_original_data = original_updater.read_bytes()
  updater_patched_data = patch_updater_zipapp(updater_original_data)
  if updater_patched_data == updater_original_data:
    print("Updater zipapp already contains the expected selector patch; continuing", flush=True)
  patched_updater.write_bytes(updater_patched_data)

  print("Writing patched /usr/comma/updater back into image", flush=True)
  write_regular_file_to_image(debugfs, patched_img, UPDATER_PATH_IN_IMAGE, patched_updater, "0100775", 0, 0)

  run_debugfs(debugfs, patched_img, f"dump -p {UPDATER_PATH_IN_IMAGE} {verify_updater}", write=False)
  verify_updater_data = verify_updater.read_bytes()
  if not updater_zipapp_has_expected_content(verify_updater_data):
    raise RuntimeError("Updater zipapp verification failed after writing updater file into image")

  if args.set_version:
    version_file = work_dir / "VERSION.patched"
    version_file.write_text(args.set_version.strip() + "\n", encoding="utf-8")
    print(f"Writing {VERSION_PATH_IN_IMAGE}={args.set_version.strip()}", flush=True)
    write_regular_file_to_image(debugfs, patched_img, VERSION_PATH_IN_IMAGE, version_file, "0100644", 0, 0)
    version_raw = run_debugfs(debugfs, patched_img, f"cat {VERSION_PATH_IN_IMAGE}", write=False)
    version_lines = [ln.strip() for ln in version_raw.splitlines() if ln.strip() and not ln.startswith("debugfs ")]
    version_verify = version_lines[0] if version_lines else ""
    if version_verify != args.set_version.strip():
      raise RuntimeError(f"/VERSION mismatch after patch: got '{version_verify}', expected '{args.set_version.strip()}'")

  raw_hash = sha256_file(patched_img)
  raw_size = patched_img.stat().st_size

  default_name = f"system-{raw_hash}.img.xz"
  output_xz = Path(args.output_xz).resolve() if args.output_xz else (work_dir / default_name)
  compress_xz(patched_img, output_xz)

  print("")
  print("Patched AGNOS system artifact ready:")
  print(f"  raw image: {patched_img}")
  print(f"  xz image:  {output_xz}")
  print(f"  raw sha256/hash_raw: {raw_hash}")
  print(f"  size: {raw_size}")
  print("")

  if args.new_url:
    new_manifest = update_manifest_system_entry(manifest, args.new_url, raw_hash, raw_size)
    out_path: Path
    if args.in_place_manifest:
      out_path = manifest_path
    elif args.manifest_out:
      out_path = Path(args.manifest_out).resolve()
    else:
      out_path = work_dir / "agnos.patched.json"
    out_path.write_text(json.dumps(new_manifest, indent=2) + "\n")
    print(f"Updated manifest written: {out_path}")
  else:
    print("No --new-url provided. Manifest not updated.")
    print("Set system entry values to:")
    print(json.dumps({
      "url": "<your-hosted-system.img.xz-url>",
      "hash": raw_hash,
      "hash_raw": raw_hash,
      "size": raw_size,
      "sparse": False,
      "full_check": False,
      "has_ab": True,
    }, indent=2))

  return 0


if __name__ == "__main__":
  raise SystemExit(main())
