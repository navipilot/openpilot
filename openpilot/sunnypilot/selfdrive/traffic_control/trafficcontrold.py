#!/usr/bin/env python3

import numpy as np

import openpilot.cereal.messaging as messaging
from openpilot.common.params import Params
from openpilot.common.realtime import Priority, config_realtime_process
from openpilot.sunnypilot.selfdrive.traffic_control import TRAFFIC_SIGNAL_CONTROL_PARAM
from openpilot.sunnypilot.selfdrive.traffic_control.controller import TrafficControlConfig, TrafficControlMode
from openpilot.sunnypilot.selfdrive.traffic_control.radar_state import (
  TrafficRadarGoPolicy,
  TrafficRadarSource,
)


def read_source_config(params: Params) -> tuple[TrafficControlConfig, TrafficRadarGoPolicy]:
  reference_dm = params.get("TeslaTrafficStopReference", return_default=True)
  max_speed_kph = params.get("TeslaTrafficControlMaxSpeed", return_default=True)
  try:
    reference = float(np.clip(float(reference_dm) / 10.0, 2.0, 12.0))
  except (TypeError, ValueError):
    reference = 5.0
  try:
    max_control_speed = float(np.clip(float(max_speed_kph), 20.0, 120.0)) / 3.6
  except (TypeError, ValueError):
    max_control_speed = 60.0 / 3.6
  control_enabled = params.get_bool(TRAFFIC_SIGNAL_CONTROL_PARAM)
  config = TrafficControlConfig(
    # Disabled clears controller evidence instead of running counterfactual
    # Observe sessions. Raw Tesla CAN diagnostics are produced independently.
    mode=TrafficControlMode.stopGo if control_enabled else TrafficControlMode.off,
    default_stop_reference=reference,
    max_control_speed=max_control_speed,
  )
  go_policy = TrafficRadarGoPolicy.active if control_enabled else TrafficRadarGoPolicy.passive
  return config, go_policy


def build_source(params: Params) -> TrafficRadarSource:
  config, go_policy = read_source_config(params)
  return TrafficRadarSource(config, go_policy=go_policy)


def refresh_source_config(source: TrafficRadarSource, params: Params) -> None:
  config, go_policy = read_source_config(params)
  source.controller.set_config(config)
  source.go_policy = go_policy


def main() -> None:
  config_realtime_process(5, Priority.CTRL_LOW)
  params = Params()
  source = build_source(params)
  model_updates = 0
  services = ['carControl', 'carState', 'modelV2', 'carStateSP']
  sm = messaging.SubMaster(services, poll='modelV2', ignore_alive=['carStateSP'],
                           ignore_avg_freq=['carStateSP'], ignore_valid=['carStateSP'])
  pm = messaging.PubMaster(['trafficRadarState'])

  while True:
    sm.update()
    if sm.updated['modelV2']:
      model_updates += 1
      if model_updates % 20 == 0:
        refresh_source_config(source, params)
      pm.send('trafficRadarState', source.update(sm, sm.logMonoTime['modelV2']))


if __name__ == "__main__":
  main()
