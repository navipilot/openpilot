#!/usr/bin/env bash
set -euo pipefail

# Collects high-signal diagnostics for UI freeze incidents on-device.
# Usage:
#   ./scripts/debug_ui_freeze.sh
# Optional env:
#   OUT_DIR=/data/media/0/realdata/ui_freeze_debug

OUT_DIR="${OUT_DIR:-/data/media/0/realdata/ui_freeze_debug}"
TS="$(date +%Y%m%d_%H%M%S)"
OUT_FILE="${OUT_DIR}/ui_freeze_${TS}.log"

mkdir -p "${OUT_DIR}"

{
  echo "=== UI Freeze Debug ==="
  echo "timestamp: $(date -Ins)"
  echo "hostname: $(hostname)"
  echo "kernel: $(uname -a)"
  echo

  echo "=== Process Snapshot ==="
  ps -eo pid,ppid,stat,ni,etime,comm,args | grep -E "(manager.py|selfdrive.ui.ui|/selfdrive/ui/ui|weston|loggerd|encoderd|modeld|dmonitoringmodeld)" || true
  echo

  echo "=== Manager State (tmux service output) ==="
  if command -v tmux >/dev/null 2>&1; then
    tmux capture-pane -pt comma:0 -S -220 2>/dev/null || true
  fi
  echo

  echo "=== UI Watchdog Files (/dev/shm/wd_*) ==="
  ls -l /dev/shm/wd_* 2>/dev/null || true
  echo

  echo "=== UI PID + Watchdog Timestamp Delta ==="
  for ui_pid in $(pgrep -f "selfdrive.ui.ui|/selfdrive/ui/ui" 2>/dev/null || true); do
    echo "ui_pid=${ui_pid}"
    wd_file="/dev/shm/wd_${ui_pid}"
    if [[ -f "${wd_file}" ]]; then
      now_ns="$(python3 - <<'PY'
import time
print(int(time.monotonic_ns()))
PY
)"
      last_ns="$(python3 - "${wd_file}" <<'PY'
import struct, sys
with open(sys.argv[1], "rb") as f:
  print(struct.unpack("Q", f.read())[0])
PY
)"
      python3 - "${now_ns}" "${last_ns}" <<'PY'
import sys
now_ns = int(sys.argv[1]); last_ns = int(sys.argv[2])
print(f"watchdog_age_s={(now_ns-last_ns)/1e9:.3f}")
PY
    else
      echo "watchdog_file_missing=${wd_file}"
    fi
    echo
  done

  echo "=== /data/log (recent ui/watchdog/weston/gpu errors) ==="
  grep -RinE "Watchdog timeout for ui|Process ui crashed|ui.*exit|weston|Wayland|EGL|gbm|drm|GPU|segfault|SIGSEGV|killed process|OOM|fatal" /data/log/swaglog* 2>/dev/null | tail -n 400 || true
  echo

  echo "=== dmesg (recent high-signal lines) ==="
  dmesg 2>/dev/null | grep -Ei "drm|gpu|msm|kgsl|weston|wayland|segfault|oom|killed process|hang" | tail -n 300 || true
  echo

  echo "=== systemd journal (weston / ui if available) ==="
  journalctl -n 300 --no-pager 2>/dev/null | grep -Ei "weston|wayland|ui|kgsl|drm|gpu|segfault|oom" || true
  echo

  echo "=== open files for UI process(es) ==="
  for ui_pid in $(pgrep -f "selfdrive.ui.ui|/selfdrive/ui/ui" 2>/dev/null || true); do
    echo "--- lsof for pid ${ui_pid} ---"
    lsof -p "${ui_pid}" 2>/dev/null | tail -n 200 || true
  done
  echo

  echo "=== End ==="
} > "${OUT_FILE}" 2>&1

echo "Wrote: ${OUT_FILE}"
