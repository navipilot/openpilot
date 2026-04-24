#!/usr/bin/env python3
import time

import cereal.messaging as messaging
from openpilot.common.params import Params
from openpilot.common.realtime import config_realtime_process
from openpilot.system.hardware import HARDWARE
from openpilot.selfdrive.monitoring.helpers import DriverMonitoring


def dmonitoringd_thread():
  config_realtime_process([0, 1, 2, 3], 5)

  params = Params()
  pm = messaging.PubMaster(['driverMonitoringState'])

  if not HARDWARE.has_driver_camera():
    _run_no_dcam_loop(pm, params)
  else:
    _run_normal_loop(pm, params)


def _run_no_dcam_loop(pm, params):
  """10Hz wheel-touch-only monitoring for tici lite (no driver camera)."""
  sm = messaging.SubMaster(['carState', 'selfdriveState', 'carControl'], poll='carState')
  DM = DriverMonitoring(rhd_saved=params.get_bool("IsRhdDetected"), always_on=True, no_driver_camera=True)

  while True:
    sm.update(timeout=100)
    DM.run_step(sm, demo=False)
    dat = DM.get_state_packet(valid=sm.all_checks())
    pm.send('driverMonitoringState', dat)
    time.sleep(0.1)  # 10Hz


def _run_normal_loop(pm, params):
  """20Hz normal loop driven by driverStateV2 (all devices with dcam)."""
  sm = messaging.SubMaster(['driverStateV2', 'liveCalibration', 'carState', 'selfdriveState', 'modelV2',
                            'carControl'], poll='driverStateV2')
  DM = DriverMonitoring(rhd_saved=params.get_bool("IsRhdDetected"), always_on=params.get_bool("AlwaysOnDM"))
  demo_mode = False

  # 20Hz <- dmonitoringmodeld
  while True:
    sm.update()
    if not sm.updated['driverStateV2']:
      # iterate when model has new output
      continue

    valid = sm.all_checks()
    if demo_mode and sm.valid['driverStateV2']:
      DM.run_step(sm, demo=demo_mode)
    elif valid:
      DM.run_step(sm, demo=demo_mode)

    # publish
    dat = DM.get_state_packet(valid=valid)
    pm.send('driverMonitoringState', dat)

    # load live always-on toggle
    if sm['driverStateV2'].frameId % 40 == 1:
      DM.always_on = params.get_bool("AlwaysOnDM")
      demo_mode = params.get_bool("IsDriverViewEnabled")

    # save rhd virtual toggle every 5 mins
    if (sm['driverStateV2'].frameId % 6000 == 0 and not demo_mode and
     DM.wheelpos.prob_offseter.filtered_stat.n > DM.settings._WHEELPOS_FILTER_MIN_COUNT and
     DM.wheel_on_right == (DM.wheelpos.prob_offseter.filtered_stat.M > DM.settings._WHEELPOS_THRESHOLD)):
      params.put_bool_nonblocking("IsRhdDetected", DM.wheel_on_right)


def main():
  dmonitoringd_thread()


if __name__ == '__main__':
  main()
