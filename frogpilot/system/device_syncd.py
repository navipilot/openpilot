#!/usr/bin/env python3
import requests
import time

from cereal import messaging

from openpilot.common.params import Params, ParamKeyFlag, ParamKeyType
from openpilot.common.realtime import Ratekeeper
from openpilot.common.time_helpers import system_time_valid

from openpilot.frogpilot.common.frogpilot_utilities import get_frogpilot_api_error, get_frogpilot_api_info, is_url_pingable
from openpilot.frogpilot.common.frogpilot_variables import EXCLUDED_KEYS, FROGPILOT_API, update_frogpilot_toggles

POND_PRESENCE_INTERVAL_ACTIVE = 60
POND_PRESENCE_INTERVAL_IDLE = 240

REMOTE_TOGGLE_CHECK_INTERVAL_ACTIVE = 10
REMOTE_TOGGLE_CHECK_INTERVAL_IDLE = 60

POND_HEADERS = {
  "Content-Type": "application/json",
  "User-Agent": "frogpilot-api/1.0",
}


def get_pond_api_payload():
  api_token, build_metadata, device_type, dongle_id = get_frogpilot_api_info()
  if not api_token or not dongle_id:
    return None

  return {
    "api_token": api_token,
    "build_metadata": build_metadata,
    "device": device_type,
    "dongle_id": dongle_id,
    "frogpilot_dongle_id": dongle_id,
  }


def pair_to_the_pond():
  if not is_url_pingable(FROGPILOT_API):
    return None, "Network error"

  payload = get_pond_api_payload()
  if payload is None:
    return None, "Authentication failed. Please restart your device."

  try:
    response = requests.post(f"{FROGPILOT_API}/pond/pair/request", json=payload, headers=POND_HEADERS, timeout=10)
    if not response.ok:
      return None, get_frogpilot_api_error(response)

    code = response.json().get("code")
    if not code:
      return None, "Failed to retrieve pairing code"

    return code, None

  except Exception as exception:
    print(f"Failed to request pairing code: {exception}")
    return None, "Network error"


def poll_status(code, poll_active=lambda: True, max_polls=100, poll_interval=3.0):
  if not code:
    return None

  payload = get_pond_api_payload()
  if payload is None:
    return None

  for _ in range(max_polls):
    if not poll_active():
      return None

    try:
      response = requests.get(
        f"{FROGPILOT_API}/pond/pair/status",
        params={**payload, "code": code},
        headers={"User-Agent": POND_HEADERS["User-Agent"]},
        timeout=10,
      )
      if response.ok:
        status = response.json().get("status")
        if status in ("paired", "expired"):
          return status
      else:
        print(f"Failed to poll pairing status: {get_frogpilot_api_error(response)}")
    except Exception as exception:
      print(f"Failed to poll pairing status: {exception}")

    time.sleep(poll_interval)

  return "expired"


def unpair_from_the_pond():
  if not is_url_pingable(FROGPILOT_API):
    return "Network error"

  payload = get_pond_api_payload()
  if payload is None:
    return "Authentication failed. Please restart your device."

  try:
    response = requests.delete(f"{FROGPILOT_API}/pond/devices", json=payload, headers=POND_HEADERS, timeout=10)
    if not response.ok:
      return get_frogpilot_api_error(response)

    return None

  except Exception as exception:
    print(f"Failed to unpair device: {exception}")
    return "Network error"


def check_toggles(started, params, sm=None, boot_run=False):
  if not params.get_bool("PondPaired"):
    return None

  if not is_url_pingable(FROGPILOT_API):
    return None

  if not boot_run:
    if started and not sm["frogpilotCarState"].isParked:
      return None
    if sm["deviceState"].screenBrightnessPercent == 0:
      return None

  try:
    api_token, _, device_type, dongle_id = get_frogpilot_api_info()
    if not dongle_id or not api_token:
      return None

    response = requests.get(
      f"{FROGPILOT_API}/pond/toggles/pending",
      params={"dongle_id": dongle_id, "api_token": api_token},
      headers={"Content-Type": "application/json", "User-Agent": "frogpilot-api/1.0"},
      timeout=10,
    )
    response.raise_for_status()

    data = response.json()
    pond_active = data.get("pond_active") is True

    if data.get("paired") is False:
      params.put_bool("PondPaired", False)
      print("Device was unpaired remotely")
      return False

    toggles = data.get("toggles")
    if not toggles:
      return pond_active

    for key, value in toggles.items():
      if key in EXCLUDED_KEYS:
        continue
      try:
        params.check_key(key)
      except Exception:
        print(f"Skipping unknown param key: {key}")
        continue

      if value is None:
        continue

      try:
        casted_value = params.cpp2python(key, value.encode("utf-8") if isinstance(value, str) else value)
        if casted_value is not None:
          params.put(key, casted_value)
      except Exception as exception:
        print(f"Skipping remote toggle {key}: {exception}")
        continue

    update_frogpilot_toggles()

    requests.post(
      f"{FROGPILOT_API}/pond/toggles/ack",
      json={
        "api_token": api_token,
        "device": device_type,
        "frogpilot_dongle_id": dongle_id,
      },
      headers={"Content-Type": "application/json", "User-Agent": "frogpilot-api/1.0"},
      timeout=10,
    ).raise_for_status()

    print(f"Successfully applied {len(toggles)} remote toggles")
    return pond_active

  except Exception as e:
    print(f"Failed to check remote toggles: {e}")
    return None


def ping_pond_presence(interval, parked, started, state_changed):
  last_ping = getattr(ping_pond_presence, "_last_ping", 0.0)
  now = time.monotonic()
  if not state_changed and (now - last_ping) < interval:
    return

  try:
    api_token, build_metadata, device_type, dongle_id = get_frogpilot_api_info()
    if not dongle_id or not api_token:
      return

    payload = {
      "api_token": api_token,
      "build_metadata": build_metadata,
      "device": device_type,
      "dongle_id": dongle_id,
      "frogpilot_dongle_id": dongle_id,
      "is_onroad": bool(started),
      "is_parked": bool(parked),
    }

    response = requests.post(
      f"{FROGPILOT_API}/pond/presence/device",
      json=payload,
      headers={"Content-Type": "application/json", "User-Agent": "frogpilot-api/1.0"},
      timeout=10,
    )
    response.raise_for_status()
    ping_pond_presence._last_ping = now

  except Exception as e:
    print(f"Failed to update Pond presence: {e}")


def upload_toggles(params):
  if not is_url_pingable(FROGPILOT_API):
    return False

  try:
    api_token, build_metadata, device_type, dongle_id = get_frogpilot_api_info()
    if not dongle_id or not api_token:
      return False

    toggles = {}
    for key in params.all_keys():
      key_str = key.decode("utf-8") if isinstance(key, bytes) else str(key)
      if key_str in EXCLUDED_KEYS:
        continue
      if params.get_key_flag(key) & ParamKeyFlag.DONT_LOG:
        continue

      value = params.get(key)
      if value is None:
        continue

      key_type = params.get_type(key)
      if key_type == ParamKeyType.BYTES:
        value = value.decode("utf-8", "replace")
      elif key_type == ParamKeyType.TIME:
        value = value.isoformat()
      toggles[key_str] = value

    payload = {
      "api_token": api_token,
      "build_metadata": build_metadata,
      "device": device_type,
      "dongle_id": dongle_id,
      "frogpilot_dongle_id": dongle_id,
      "toggles": toggles,
    }

    response = requests.post(
      f"{FROGPILOT_API}/pond/toggles/sync",
      json=payload,
      headers={"Content-Type": "application/json", "User-Agent": "frogpilot-api/1.0"},
      timeout=10,
    )
    response.raise_for_status()

    print("Successfully uploaded toggles to FrogPilot.com")
    return True

  except Exception as e:
    print(f"Failed to upload toggles: {e}")
    return False


def pond_thread():
  rate_keeper = Ratekeeper(1, None)

  sm = messaging.SubMaster(["deviceState", "frogpilotCarState"])

  params = Params(return_defaults=True)

  boot_sync_complete = False
  pond_active = False
  previous_parked = False
  previous_started = False

  next_toggle_check_at = 0.0

  while True:
    sm.update(0)

    parked = sm["frogpilotCarState"].isParked
    started = sm["deviceState"].started
    state_changed = started != previous_started or parked != previous_parked

    if params.get_bool("PondPaired"):
      presence_interval = POND_PRESENCE_INTERVAL_ACTIVE if started or pond_active else POND_PRESENCE_INTERVAL_IDLE
      ping_pond_presence(presence_interval, parked, started, state_changed)

    if not boot_sync_complete and system_time_valid():
      boot_pond_active = check_toggles(False, params, boot_run=True)
      if boot_pond_active is not None:
        pond_active = boot_pond_active
      boot_sync_complete = True

    now = time.monotonic()
    if state_changed and parked:
      next_toggle_check_at = 0.0

    if boot_sync_complete and now >= next_toggle_check_at:
      latest_pond_active = check_toggles(started, params, sm)
      if latest_pond_active is not None:
        pond_active = latest_pond_active
      next_toggle_check_at = now + REMOTE_TOGGLE_CHECK_INTERVAL_ACTIVE if pond_active else REMOTE_TOGGLE_CHECK_INTERVAL_IDLE

    if params.get_bool("PondUploadPending"):
      if not params.get_bool("PondPaired"):
        params.put_bool("PondUploadPending", False)
      elif upload_toggles(params):
        params.put_bool("PondUploadPending", False)

    previous_parked = parked
    previous_started = started

    rate_keeper.keep_time()


def main():
  pond_thread()


if __name__ == "__main__":
  main()
