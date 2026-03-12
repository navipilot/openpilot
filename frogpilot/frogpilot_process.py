#!/usr/bin/env python3
import datetime
import json
import requests
import time

from cereal import messaging
from openpilot.common.api import Api, api_get
from openpilot.common.params import Params
from openpilot.common.realtime import DT_MDL, Priority, Ratekeeper, config_realtime_process
from openpilot.common.time_helpers import system_time_valid
from openpilot.system.athena.registration import UNREGISTERED_DONGLE_ID

from openpilot.frogpilot.assets.model_manager import MODEL_DOWNLOAD_ALL_PARAM, MODEL_DOWNLOAD_PARAM, ModelManager
from openpilot.frogpilot.assets.theme_manager import THEME_COMPONENT_PARAMS, ThemeManager
from openpilot.frogpilot.common.frogpilot_backups import backup_toggles
from openpilot.frogpilot.common.frogpilot_functions import capture_report, update_maps, update_openpilot
from openpilot.frogpilot.common.frogpilot_utilities import ThreadManager, flash_panda, is_url_pingable, lock_doors, use_konik_server
from openpilot.frogpilot.common.frogpilot_variables import ERROR_LOGS_PATH, FrogPilotVariables
from openpilot.frogpilot.controls.frogpilot_planner import FrogPilotPlanner
from openpilot.frogpilot.system.frogpilot_stats import send_stats
from openpilot.frogpilot.system.frogpilot_tracking import FrogPilotTracking

ASSET_CHECK_RATE = (1 / DT_MDL)
DRIVE_STATS_SYNC_RATE = 30

def check_assets(now, model_manager, theme_manager, thread_manager, params, params_memory, frogpilot_toggles):
  if params_memory.get_bool(MODEL_DOWNLOAD_ALL_PARAM):
    thread_manager.run_with_lock(model_manager.download_all_models)
  elif params_memory.get_bool("UpdateTinygrad"):
    thread_manager.run_with_lock(model_manager.update_tinygrad)
  else:
    model_to_download = params_memory.get(MODEL_DOWNLOAD_PARAM)
    if isinstance(model_to_download, bytes):
      model_to_download = model_to_download.decode("utf-8", errors="replace")
    if model_to_download:
      thread_manager.run_with_lock(model_manager.download_model, (model_to_download,))

  for asset_type, asset_param in THEME_COMPONENT_PARAMS.items():
    asset_to_download = params_memory.get(asset_param)
    if asset_to_download:
      thread_manager.run_with_lock(theme_manager.download_theme, (asset_type, asset_to_download, asset_param, frogpilot_toggles))

  if params_memory.get_bool("FlashPanda"):
    thread_manager.run_with_lock(flash_panda, (params_memory))

  report_data = params_memory.get("IssueReported")
  if report_data:
    capture_report(report_data["DiscordUser"], report_data["Issue"], params, vars(frogpilot_toggles))
    params_memory.remove("IssueReported")

  if params_memory.get_bool("DownloadMaps"):
    thread_manager.run_with_lock(update_maps, (now, params, params_memory, True))

def sync_drive_stats(params, session):
  try:
    dongle_id = params.get("DongleId")
    if isinstance(dongle_id, bytes):
      dongle_id = dongle_id.decode("utf-8", errors="replace")
    if not dongle_id or dongle_id == UNREGISTERED_DONGLE_ID:
      return

    token = Api(dongle_id).get_token(expiry_hours=2)
    if not token:
      return

    response = api_get(f"v1.1/devices/{dongle_id}/stats", timeout=15, access_token=token, session=session)
    if response.status_code != 200:
      print(f"Failed to sync drive stats (HTTP {response.status_code})")
      return

    stats = response.json()
    if not isinstance(stats, dict):
      return

    all_stats = stats.get("all")
    week_stats = stats.get("week")
    if not isinstance(all_stats, dict) or not isinstance(week_stats, dict):
      return

    params.put("ApiCache_DriveStats", stats)

    all_minutes = all_stats.get("minutes")
    if isinstance(all_minutes, (int, float)):
      params.put_int("KonikMinutes" if use_konik_server() else "openpilotMinutes", int(all_minutes))
  except Exception as exception:
    print(f"Failed to sync drive stats: {exception}")

def transition_offroad(frogpilot_planner, theme_manager, thread_manager, time_validated, sm, params, frogpilot_toggles):
  params.put("LastGPSPosition", json.dumps(frogpilot_planner.gps_position))

  if frogpilot_toggles.lock_doors_timer != 0:
    thread_manager.run_with_lock(lock_doors, (frogpilot_toggles.lock_doors_timer, sm, params), report=False)

  if frogpilot_toggles.random_themes:
    theme_manager.update_active_theme(time_validated, frogpilot_toggles, randomize_theme=True)

  if time_validated:
    thread_manager.run_with_lock(send_stats)

def transition_onroad(error_log):
  if error_log.is_file():
    error_log.unlink()

def update_checks(now, model_manager, theme_manager, thread_manager, params, params_memory, frogpilot_toggles, boot_run=False):
  while not (is_url_pingable("https://github.com") or is_url_pingable("https://gitlab.com")):
    time.sleep(60)

  model_manager.update_models(boot_run)
  theme_manager.update_themes(frogpilot_toggles, boot_run)

  thread_manager.run_with_lock(update_maps, (now, params, params_memory))

  if frogpilot_toggles.automatic_updates:
    thread_manager.run_with_lock(update_openpilot, (thread_manager, params))

  time.sleep(1)

def update_toggles(frogpilot_variables, started, theme_manager, thread_manager, time_validated, params, frogpilot_toggles):
  previous_holiday_themes = frogpilot_toggles.holiday_themes
  previous_random_themes = frogpilot_toggles.random_themes

  frogpilot_variables.update(theme_manager.holiday_theme, started)
  frogpilot_toggles = frogpilot_variables.frogpilot_toggles

  randomize_theme = frogpilot_toggles.holiday_themes != previous_holiday_themes
  randomize_theme |= frogpilot_toggles.random_themes != previous_random_themes

  theme_manager.theme_updated = False
  theme_manager.update_active_theme(time_validated, frogpilot_toggles, randomize_theme=randomize_theme)

  if time_validated:
    thread_manager.run_with_lock(backup_toggles, (params))

  return frogpilot_toggles

def frogpilot_thread():
  rate_keeper = Ratekeeper(1 / DT_MDL, None)

  config_realtime_process(5, Priority.CTRL_LOW)

  pm = messaging.PubMaster(["frogpilotPlan"])
  sm = messaging.SubMaster(["carControl", "carState", "controlsState", "deviceState", "driverMonitoringState",
                            "gpsLocation", "gpsLocationExternal", "liveParameters", "managerState", "modelV2",
                            "onroadEvents", "pandaStates", "radarState", "selfdriveState", "frogpilotCarState",
                            "frogpilotSelfdriveState", "frogpilotModelV2", "frogpilotOnroadEvents", "mapdOut"],
                            poll="modelV2")

  params = Params(return_defaults=True)
  params_memory = Params(memory=True)

  frogpilot_variables = FrogPilotVariables()
  model_manager = ModelManager(params, params_memory)
  theme_manager = ThemeManager(params, params_memory)
  thread_manager = ThreadManager()

  frogpilot_toggles = frogpilot_variables.frogpilot_toggles

  drive_stats_session = requests.Session()
  next_drive_stats_sync = 0.0

  run_update_checks = False
  started_previously = False
  time_validated = False

  error_log = ERROR_LOGS_PATH / "error.txt"
  if error_log.is_file():
    error_log.unlink()

  while True:
    sm.update()

    now = datetime.datetime.now(datetime.timezone.utc)

    started = sm["deviceState"].started

    if not started and started_previously:
      frogpilot_planner.shutdown()

      frogpilot_toggles = update_toggles(frogpilot_variables, started, theme_manager, thread_manager, time_validated, params, frogpilot_toggles)
      transition_offroad(frogpilot_planner, theme_manager, thread_manager, time_validated, sm, params, frogpilot_toggles)

      run_update_checks = True
    elif started and not started_previously:
      frogpilot_planner = FrogPilotPlanner(error_log, theme_manager)
      frogpilot_tracking = FrogPilotTracking(frogpilot_planner, frogpilot_toggles)

      transition_onroad(error_log)

    if started and sm.updated["modelV2"]:
      frogpilot_planner.update(now, time_validated, sm, frogpilot_toggles)
      frogpilot_planner.publish(theme_manager.theme_updated, sm, pm, frogpilot_toggles)

      frogpilot_tracking.update(now, time_validated, sm, frogpilot_toggles)
    elif not started:
      frogpilot_plan_send = messaging.new_message("frogpilotPlan")
      frogpilot_plan_send.frogpilotPlan.frogpilotToggles = json.dumps(vars(frogpilot_toggles))
      frogpilot_plan_send.frogpilotPlan.themeUpdated = theme_manager.theme_updated
      pm.send("frogpilotPlan", frogpilot_plan_send)

    started_previously = started

    if not started and time_validated and sm["deviceState"].screenBrightnessPercent > 0:
      monotonic_now = time.monotonic()
      if monotonic_now >= next_drive_stats_sync:
        thread_manager.run_with_lock(sync_drive_stats, (params, drive_stats_session), report=False)
        next_drive_stats_sync = monotonic_now + DRIVE_STATS_SYNC_RATE
    elif started:
      next_drive_stats_sync = 0.0

    if rate_keeper.frame % ASSET_CHECK_RATE == 0:
      check_assets(now, model_manager, theme_manager, thread_manager, params, params_memory, frogpilot_toggles)

    if params_memory.get_bool("FrogPilotTogglesUpdated") or theme_manager.theme_updated:
      frogpilot_toggles = update_toggles(frogpilot_variables, started, theme_manager, thread_manager, time_validated, params, frogpilot_toggles)

    run_update_checks |= params_memory.get_bool("ManualUpdateInitiated")
    run_update_checks |= now.second == 0 and (now.minute % 60 == 0 or (now.minute % 5 == 0 and frogpilot_variables.frogs_go_moo))
    run_update_checks &= time_validated

    if run_update_checks:
      theme_manager.update_active_theme(time_validated, frogpilot_toggles)
      thread_manager.run_with_lock(update_checks, (now, model_manager, theme_manager, thread_manager, params, params_memory, frogpilot_toggles))

      run_update_checks = False
    elif not time_validated:
      time_validated = system_time_valid()
      if not time_validated:
        continue

      theme_manager.update_active_theme(time_validated, frogpilot_toggles)

      thread_manager.run_with_lock(backup_toggles, (params, True))
      thread_manager.run_with_lock(send_stats)
      thread_manager.run_with_lock(update_checks, (now, model_manager, theme_manager, thread_manager, params, params_memory, frogpilot_toggles, True))

    rate_keeper.keep_time()

def main():
  frogpilot_thread()

if __name__ == "__main__":
  main()
