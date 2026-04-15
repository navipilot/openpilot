from types import SimpleNamespace

import pytest

from cereal import log
from opendbc.car.honda.interface import CarInterface
from opendbc.car.honda.values import CAR
from openpilot.selfdrive.controls.lib.longcontrol import LongCtrlState
from openpilot.selfdrive.controls.lib.longitudinal_planner import LongitudinalPlanner, get_vehicle_min_accel
from openpilot.selfdrive.modeld.constants import ModelConstants


def make_lead(*, status: bool, d_rel: float = 200.0, v_lead: float = 0.0):
  lead = log.RadarState.LeadData.new_message()
  lead.status = status
  lead.dRel = d_rel
  lead.vLead = v_lead
  lead.vLeadK = v_lead
  lead.aLeadK = 0.0
  lead.vRel = 0.0
  lead.aRel = 0.0
  lead.modelProb = 0.0
  return lead


def make_model(v_ego: float, desired_accel: float):
  model = log.ModelDataV2.new_message()
  t_idxs = ModelConstants.T_IDXS

  model.position.x = [float(v_ego * t) for t in t_idxs]
  model.position.y = [0.0] * len(t_idxs)
  model.position.z = [0.0] * len(t_idxs)
  model.position.t = [float(t) for t in t_idxs]

  model.velocity.x = [float(v_ego)] * len(t_idxs)
  model.velocity.y = [0.0] * len(t_idxs)
  model.velocity.z = [0.0] * len(t_idxs)
  model.velocity.t = [float(t) for t in t_idxs]

  model.acceleration.x = [0.0] * len(t_idxs)
  model.acceleration.y = [0.0] * len(t_idxs)
  model.acceleration.z = [0.0] * len(t_idxs)
  model.acceleration.t = [float(t) for t in t_idxs]

  model.meta.disengagePredictions.gasPressProbs = [1.0] * 6
  model.action.desiredAcceleration = desired_accel
  model.action.shouldStop = False
  return model


def make_sm(v_ego: float, desired_accel: float, min_accel: float):
  return {
    "carControl": SimpleNamespace(orientationNED=[0.0, 0.0, 0.0]),
    "carState": SimpleNamespace(
      vEgo=v_ego,
      vEgoCluster=v_ego,
      aEgo=0.0,
      vCruise=100.0,
      standstill=False,
      steeringAngleDeg=0.0,
    ),
    "controlsState": SimpleNamespace(
      longControlState=LongCtrlState.pid,
      forceDecel=False,
    ),
    "liveParameters": SimpleNamespace(angleOffsetDeg=0.0),
    "modelV2": make_model(v_ego, desired_accel),
    "radarState": SimpleNamespace(
      leadOne=make_lead(status=False),
      leadTwo=make_lead(status=False),
    ),
    "selfdriveState": SimpleNamespace(enabled=True, experimentalMode=True, personality=0),
    "starpilotPlan": SimpleNamespace(
      vCruise=v_ego + 5.0,
      minAcceleration=min_accel,
      maxAcceleration=2.0,
      disableThrottle=False,
      trackingLead=False,
      accelerationJerk=5.0,
      dangerJerk=5.0,
      speedJerk=5.0,
      dangerFactor=1.0,
      tFollow=1.45,
      forcingStopLength=2,
    ),
  }


def make_toggles():
  return SimpleNamespace(
    taco_tune=False,
    classic_model=False,
    tinygrad_model=True,
    model_version="v11",
    stop_distance=6.0,
    vEgoStopping=0.5,
  )


def test_experimental_mlsim_uses_vehicle_min_accel_floor():
  v_ego = 18.0
  desired_accel = -1.0
  comfort_min_accel = -0.5

  CP = CarInterface.get_non_essential_params(CAR.HONDA_CIVIC)
  planner = LongitudinalPlanner(CP, init_v=v_ego)
  sm = make_sm(v_ego, desired_accel, comfort_min_accel)

  vehicle_min_accel = get_vehicle_min_accel(CP, v_ego)
  assert vehicle_min_accel < comfort_min_accel

  planner.update(sm, make_toggles())

  assert planner.mode == "blended"
  assert planner.mlsim
  assert planner.output_a_target == pytest.approx(desired_accel, abs=1e-3)
  assert planner.output_a_target < comfort_min_accel
