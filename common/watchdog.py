import os
import time
import struct
from openpilot.system.hardware.hw import Paths
from openpilot.common.swaglog import cloudlog

WATCHDOG_FN = f"{Paths.shm_path()}/wd_"
_LAST_KICK = 0.0
_LAST_ERROR_LOG = 0.0

def kick_watchdog():
  global _LAST_KICK, _LAST_ERROR_LOG
  current_time = time.monotonic()

  if current_time - _LAST_KICK < 1.0:
    return True

  try:
    with open(f"{WATCHDOG_FN}{os.getpid()}", 'wb') as f:
      f.write(struct.pack('<Q', int(current_time * 1e9)))
      f.flush()
    _LAST_KICK = current_time
    return True
  except OSError as e:
    if current_time - _LAST_ERROR_LOG >= 5.0:
      cloudlog.error(f"watchdog kick failed for pid {os.getpid()}: {e}")
      _LAST_ERROR_LOG = current_time
    return False
