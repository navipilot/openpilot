"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

A guard: no jetlink.openpilot test may touch the machine's own params store
or device files. A suite run on a comma once cleared the live JetlinkSpec and
turned the accelerator off without a word; a read is harmless, but a test
that reads the live store passes or fails on whatever the device has set.

Many of the paths are read inside `except OSError`, so refusing the access
would only hide it. The guard records every open, listing, socket connect or
root-script run that reaches a forbidden path, and fails the test after it
runs. tests/openpilot/fakes.isolate points everything at a temporary
directory instead.
"""
from __future__ import annotations

import os
import sys
from pathlib import Path

import pytest

FORBIDDEN = ('/data/', '/dev/shm/', '/dev/ffs', '/sys/', str(Path.home() / '.comma') + os.sep)
_touched: list[str] | None = None


def _path(value) -> str | None:
  if isinstance(value, (str, bytes, os.PathLike)):
    return os.path.abspath(os.fsdecode(value))
  return None


def _guard(event: str, args) -> None:
  if _touched is None:
    return
  if event in ('open', 'os.listdir', 'os.scandir'):
    path = _path(args[0]) if args else None
  elif event == 'socket.connect':
    path = _path(args[1]) if len(args) > 1 else None
  elif event == 'subprocess.Popen':
    # the root script is sudo on a comma
    argv = args[1] if len(args) > 1 and isinstance(args[1], (list, tuple)) else []
    path = next((str(a) for a in argv if 'jetlink-root.sh' in str(a) or str(a) == 'sudo'), None)
    if path is not None:
      _touched.append(f"{event} {path}")
    return
  else:
    return
  if path is not None and (path + os.sep).startswith(FORBIDDEN):
    _touched.append(f"{event} {path}")


sys.addaudithook(_guard)


@pytest.fixture(autouse=True)
def nothing_of_the_machines_own():
  global _touched
  _touched = []
  try:
    yield
    touched, _touched = _touched, None
    assert not touched, f"the test reached the machine's own files: {sorted(set(touched))}"
  finally:
    _touched = None
