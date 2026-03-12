#!/usr/bin/env python3
import os
import subprocess
from pathlib import Path

# NOTE: Do NOT import anything here that needs be built (e.g. params)
from openpilot.common.basedir import BASEDIR
from openpilot.common.spinner import Spinner
from openpilot.common.text_window import TextWindow
from openpilot.common.swaglog import cloudlog, add_file_handler
from openpilot.system.hardware import HARDWARE, AGNOS
from openpilot.system.version import get_build_metadata

MAX_CACHE_SIZE = 4e9 if "CI" in os.environ else 2e9
CACHE_DIR = Path("/data/scons_cache" if AGNOS else "/tmp/scons_cache")

TOTAL_SCONS_NODES = 2705
MAX_BUILD_PROGRESS = 100

def build(spinner: Spinner, dirty: bool = False, minimal: bool = False) -> None:
  env = os.environ.copy()
  env['SCONS_PROGRESS'] = "1"
  nproc = os.cpu_count()
  if nproc is None:
    nproc = 2

  extra_args = ["--minimal"] if minimal else []

  if AGNOS:
    HARDWARE.set_power_save(False)
    os.sched_setaffinity(0, range(8))  # ensure we can use the isolcpus cores
    # Extremely conservative on-device compile to avoid lowmemorykiller SIGKILLs.
    attempts = [1]
  else:
    attempts = [nproc, max(1, nproc // 2), 1]

  # Preserve order while de-duplicating.
  attempts = list(dict.fromkeys(max(1, int(n)) for n in attempts))

  # building with all cores can result in using too much memory,
  # so retry with less parallelism.
  compile_output: list[bytes] = []
  last_returncode = 0

  def run_scons(n: int, cache_args: list[str]) -> int:
    nonlocal compile_output, spinner, env
    scons: subprocess.Popen = subprocess.Popen(["scons", f"-j{int(n)}", *cache_args, *extra_args], cwd=BASEDIR, env=env, stderr=subprocess.PIPE)
    assert scons.stderr is not None

    # Read progress from stderr and update spinner
    while scons.poll() is None:
      try:
        line = scons.stderr.readline()
        if line is None:
          continue
        line = line.rstrip()

        prefix = b'progress: '
        if line.startswith(prefix):
          i = int(line[len(prefix):])
          spinner.update_progress(MAX_BUILD_PROGRESS * min(1., i / TOTAL_SCONS_NODES), 100.)
        elif len(line):
          compile_output.append(line)
          print(line.decode('utf8', 'replace'))
      except Exception:
        pass

    if scons.returncode != 0 and scons.stderr is not None:
      compile_output += scons.stderr.read().split(b'\n')
    return scons.returncode

  for n in attempts:
    compile_output.clear()
    last_returncode = run_scons(n, ["--cache-populate"])
    if last_returncode == 0:
      break

  if last_returncode != 0:
    # OOM-ish builds often fail with "Error -9" when clang gets SIGKILL.
    # Do one final conservative retry before surfacing a hard failure.
    output_blob = b"\n".join(compile_output)
    if AGNOS and b"Error -9" in output_blob:
      cloudlog.warning("scons likely OOM-killed (Error -9), retrying with -j1 --cache-disable")
      print("Build retry: detected Error -9, retrying with -j1 --cache-disable")
      compile_output.clear()
      last_returncode = run_scons(1, ["--cache-disable"])

  if last_returncode != 0:
    # Build failed log errors
    error_s = b"\n".join(compile_output).decode('utf8', 'replace')
    add_file_handler(cloudlog)
    cloudlog.error("scons build failed\n" + error_s)

    # Show TextWindow
    spinner.close()
    if not os.getenv("CI"):
      with TextWindow("openpilot failed to build\n \n" + error_s) as t:
        t.wait_for_exit()
    exit(1)

  # enforce max cache size
  cache_files = [f for f in CACHE_DIR.rglob('*') if f.is_file()]
  cache_files.sort(key=lambda f: f.stat().st_mtime)
  cache_size = sum(f.stat().st_size for f in cache_files)
  for f in cache_files:
    if cache_size < MAX_CACHE_SIZE:
      break
    cache_size -= f.stat().st_size
    f.unlink()


if __name__ == "__main__":
  spinner = Spinner()
  spinner.update_progress(0, 100)
  build_metadata = get_build_metadata()
  build(spinner, build_metadata.openpilot.is_dirty, minimal = AGNOS)
