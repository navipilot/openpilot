#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

usage() {
  cat <<'EOF'
Usage:
  scripts/starpilot_build_flow.sh verify
  scripts/starpilot_build_flow.sh mac [jobs]
  scripts/starpilot_build_flow.sh device [jobs]
  scripts/starpilot_build_flow.sh laptop-setup [device-host]
  scripts/starpilot_build_flow.sh laptop-device [jobs]

Modes:
  verify  Check critical StarPilot parity build guards in-tree.
  mac     Run macOS developer validation build (Qt UI + Python compile checks).
  device  Run full on-device build (comma hardware only) and set prebuilt flag.
  laptop-setup Prepare laptop device-build environment (venv + image + sysroot).
  laptop-device Run full device-target build in Linux/aarch64 container on laptop.

Notes:
  - mac mode does NOT produce device binaries.
  - device mode must be run on comma hardware (/TICI present).
  - laptop-device mode requires docker/podman and a prepared sysroot.
  - prepare sysroot via scripts/laptop_device_build.sh setup-sysroot-agnos
    or scripts/laptop_device_build.sh setup-sysroot <device-ip>.
EOF
}

require_venv() {
  if [[ ! -f "${ROOT_DIR}/.venv/bin/activate" ]]; then
    echo "Missing .venv. Initialize it first."
    exit 1
  fi
}

verify_tree() {
  local failed=0

  if ! rg -q "libs = \\['m', 'pthread'" system/camerad/SConscript; then
    echo "FAIL: system/camerad/SConscript is missing pthread link guard."
    failed=1
  else
    echo "OK: camerad link libs include pthread."
  fi

  if ! rg -q "TimezoneFinder = None" system/timed.py; then
    echo "FAIL: system/timed.py missing TimezoneFinder fallback."
    failed=1
  else
    echo "OK: timed.py has TimezoneFinder fallback."
  fi

  if ! rg -q "if tf is not None:" system/timed.py; then
    echo "FAIL: system/timed.py missing guarded timezone usage."
    failed=1
  else
    echo "OK: timed.py guards timezonefinder usage."
  fi

  if ! rg -q "if os.path.isfile\\(\"/TICI\"\\)" common/spinner.py; then
    echo "FAIL: common/spinner.py missing device-only legacy spinner path."
    failed=1
  else
    echo "OK: spinner uses device-only legacy fallback."
  fi

  if ! rg -q "if os.path.isfile\\(\"/TICI\"\\)" common/text_window.py; then
    echo "FAIL: common/text_window.py missing device-only legacy text path."
    failed=1
  else
    echo "OK: text window uses device-only legacy fallback."
  fi

  if ! source .venv/bin/activate >/dev/null 2>&1; then
    echo "FAIL: .venv is required for loggerd import checks."
    failed=1
  else
    if ! python - <<'PY'
import builtins
import importlib

real_import = builtins.__import__
def fake_import(name, *args, **kwargs):
  if name == "xattr":
    raise ModuleNotFoundError("simulated missing xattr")
  return real_import(name, *args, **kwargs)

builtins.__import__ = fake_import
importlib.import_module("openpilot.system.loggerd.xattr_cache")
print("xattr_cache import fallback OK")
PY
    then
      echo "FAIL: loggerd xattr fallback check failed."
      failed=1
    else
      echo "OK: loggerd xattr fallback works when xattr module is missing."
    fi
  fi

  if [[ "${failed}" -ne 0 ]]; then
    exit 1
  fi
}

build_mac() {
  local jobs="${1:-8}"
  require_venv
  source .venv/bin/activate

  verify_tree

  # Homebrew tools are needed for SConstruct on macOS.
  export PATH="/opt/homebrew/bin:${PATH}"
  export TMPDIR="${TMPDIR:-/tmp}"

  python -m py_compile \
    system/timed.py \
    common/spinner.py \
    common/text_window.py \
    frogpilot/system/frogpilot_stats.py

  python -m compileall -q \
    selfdrive/ui \
    system/ui \
    frogpilot/system/the_pond \
    frogpilot/system/galaxy

  SP_DISABLE_AUTO_DEVICE_SCONS=1 scons -j"${jobs}" selfdrive/ui/ui

  echo "mac build validation complete."
}

build_device() {
  local jobs="${1:-$(nproc)}"

  if [[ ! -f /TICI ]]; then
    echo "device mode must run on comma hardware (/TICI not found)."
    exit 1
  fi

  require_venv
  source .venv/bin/activate

  verify_tree
  scons -j"${jobs}"
  touch prebuilt

  echo "device build complete and prebuilt flag set."
}

main() {
  if [[ $# -lt 1 ]]; then
    usage
    exit 1
  fi

  local mode="$1"
  shift || true

  case "${mode}" in
    verify)
      verify_tree
      ;;
    mac)
      build_mac "${1:-8}"
      ;;
    device)
      build_device "${1:-$(nproc)}"
      ;;
    laptop-setup)
      scripts/laptop_device_build.sh setup "${1:-}" "${2:-comma}" "${3:-22}"
      ;;
    laptop-device)
      scripts/laptop_device_build.sh build "${1:-8}"
      ;;
    *)
      usage
      exit 1
      ;;
  esac
}

main "$@"
