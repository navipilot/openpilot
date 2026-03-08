#!/usr/bin/env python3
"""
Write mock CarParams and CarParamsSP to the param store so the UI
can be launched with all features available for debugging.

Sets infrastructure params to unlock feature availability,
then clears individual toggles so they start at defaults.

Usage:
  PYTHONPATH=. .venv/bin/python selfdrive/ui/tests/setup_mock_params.py
  BIG=0 SCALE=1 .venv/bin/python selfdrive/ui/ui.py
"""
from cereal import car, custom
from openpilot.common.params import Params


# All individual feature params to reset to defaults
PARAMS_TO_CLEAR = [
  # Cruise
  "DynamicExperimentalControl",
  "SmartCruiseControlVision",
  "SmartCruiseControlMap",
  "CustomAccIncrementsEnabled",
  "CustomAccShortPressIncrement",
  "CustomAccLongPressIncrement",
  "SpeedLimitMode",
  "SpeedLimitPolicy",
  "SpeedLimitOffsetType",
  "SpeedLimitValueOffset",
  # Steering
  "Mads",
  "MadsMainCruiseAllowed",
  "MadsUnifiedEngagementMode",
  "MadsSteeringMode",
  "AutoLaneChangeTimer",
  "AutoLaneChangeBsmDelay",
  "BlinkerPauseLateralControl",
  "BlinkerMinLateralControlSpeed",
  "BlinkerLateralReengageDelay",
  "EnforceTorqueControl",
  "NeuralNetworkLateralControl",
  "LiveTorqueParamsToggle",
  "LiveTorqueParamsRelaxedToggle",
  "CustomTorqueParams",
  "TorqueParamsOverrideEnabled",
  "TorqueParamsOverrideLatAccelFactor",
  "TorqueParamsOverrideFriction",
  # Visuals
  "BlindSpot",
  "TorqueBar",
  "RainbowMode",
  "StandstillTimer",
  "RoadNameToggle",
  "GreenLightAlert",
  "LeadDepartAlert",
  "TrueVEgoUI",
  "HideVEgoUI",
  "ShowTurnSignals",
  "RocketFuel",
  "ChevronInfo",
  "DevUIInfo",
  # Display
  "OnroadScreenOffBrightness",
  "OnroadScreenOffTimer",
  "InteractivityTimeout",
]


def main():
  params = Params()

  # Clear all individual feature params so they fall back to defaults
  for key in PARAMS_TO_CLEAR:
    params.remove(key)

  # --- Infrastructure params that unlock feature availability ---
  # No openpilotLongitudinalControl so ICBM can be enabled.
  # ICBM + IntelligentCruiseButtonManagement unlocks has_icbm,
  # which enables custom ACC, SCC, etc. without needing has_long.
  cp = car.CarParams.new_message(
    enableBsm=True,
    brand="debug",
  )
  params.put("CarParamsPersistent", cp.to_bytes())

  cp_sp = custom.CarParamsSP.new_message(
    intelligentCruiseButtonManagementAvailable=True,
  )
  params.put("CarParamsSPPersistent", cp_sp.to_bytes())

  # Enable ICBM so has_icbm=True, unlocking downstream features
  params.put_bool("IntelligentCruiseButtonManagement", True)

  print("Mock params written — all features available at defaults")


if __name__ == "__main__":
  main()
