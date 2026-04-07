import os
import sys
import time
import traceback
import threading
from pathlib import Path

from openpilot.common.swaglog import cloudlog


def _default_dump_dir() -> Path:
  for candidate in ("/data/log", "/tmp"):
    if os.path.isdir(candidate) and os.access(candidate, os.W_OK):
      return Path(candidate)
  return Path.cwd()


class UIStallMonitor:
  def __init__(self, name: str):
    self._name = name
    self._threshold_s = float(os.getenv("UI_STALL_PROBE_MAX_DT", "5"))
    self._poll_s = float(os.getenv("UI_STALL_PROBE_POLL_DT", "0.25"))
    self._dump_dir = _default_dump_dir()
    self._main_thread_id = threading.get_ident()

    now = time.monotonic()
    self._last_progress = now
    self._phase = "init"
    self._phase_entered = now
    self._stall_reported = False
    self._stalled_since = now
    self._stalled_phase = self._phase

    self._lock = threading.Lock()
    self._stop_event = threading.Event()
    self._thread = threading.Thread(target=self._run, name=f"{name}_stall_probe", daemon=True)

  def start(self) -> None:
    if self._threshold_s <= 0.0:
      return
    self._thread.start()

  def stop(self) -> None:
    if self._threshold_s <= 0.0:
      return
    self._stop_event.set()
    self._thread.join(timeout=1.0)

  def progress(self, phase: str) -> None:
    now = time.monotonic()
    recovered = None

    with self._lock:
      if phase != self._phase:
        self._phase = phase
        self._phase_entered = now
      self._last_progress = now

      if self._stall_reported:
        recovered = (now - self._stalled_since, self._stalled_phase, phase)
        self._stall_reported = False

    if recovered is not None:
      stalled_for_s, stalled_phase, current_phase = recovered
      cloudlog.warning(f"{self._name} stall recovered after {stalled_for_s:.1f}s (stalled_phase={stalled_phase}, current_phase={current_phase})")

  def _run(self) -> None:
    while not self._stop_event.wait(self._poll_s):
      now = time.monotonic()
      with self._lock:
        stalled_for_s = now - self._last_progress
        phase = self._phase
        phase_for_s = now - self._phase_entered
        already_reported = self._stall_reported

      if stalled_for_s < self._threshold_s or already_reported:
        continue

      dump = self._build_dump(now, phase, stalled_for_s, phase_for_s)
      dump_path = self._write_dump(dump)
      with self._lock:
        self._stall_reported = True
        self._stalled_since = now
        self._stalled_phase = phase

      preview = self._main_thread_preview()
      path_s = str(dump_path) if dump_path is not None else "<write_failed>"
      cloudlog.error(f"{self._name} main loop stalled for {stalled_for_s:.1f}s in phase={phase} (phase_for={phase_for_s:.1f}s) dump={path_s}\n{preview}")

  def _build_dump(self, now: float, phase: str, stalled_for_s: float, phase_for_s: float) -> str:
    frames = sys._current_frames()
    threads = {thread.ident: thread for thread in threading.enumerate()}
    lines = [
      f"name={self._name}",
      f"pid={os.getpid()}",
      f"wall_time={time.strftime('%Y-%m-%dT%H:%M:%S%z')}",
      f"monotonic={now:.6f}",
      f"stalled_for_s={stalled_for_s:.3f}",
      f"phase={phase}",
      f"phase_for_s={phase_for_s:.3f}",
      "",
    ]

    ordered_idents = sorted(frames.keys(), key=lambda ident: ident != self._main_thread_id)
    for ident in ordered_idents:
      thread = threads.get(ident)
      name = thread.name if thread is not None else "<unknown>"
      daemon = thread.daemon if thread is not None else None
      lines.append(f"Thread {name} ident={ident} daemon={daemon}")
      lines.extend(traceback.format_stack(frames[ident]))
      lines.append("")

    return "".join(line if line.endswith("\n") else f"{line}\n" for line in lines)

  def _main_thread_preview(self) -> str:
    frame = sys._current_frames().get(self._main_thread_id)
    if frame is None:
      return "main_thread_stack=<unavailable>"
    stack_lines = traceback.format_stack(frame)
    preview = "".join(stack_lines[-8:]).strip()
    return f"main_thread_stack:\n{preview}" if preview else "main_thread_stack=<empty>"

  def _write_dump(self, dump: str) -> Path | None:
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    path = self._dump_dir / f"{self._name}_stall_{os.getpid()}_{timestamp}.log"
    try:
      path.write_text(dump)
      return path
    except OSError as e:
      cloudlog.error(f"failed to write {self._name} stall dump to {path}: {e}")
      return None
