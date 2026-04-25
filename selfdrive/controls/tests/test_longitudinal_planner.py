import sys
import types
from types import SimpleNamespace

import numpy as np
import pytest

from cereal import log
from opendbc.car.honda.interface import CarInterface
from opendbc.car.honda.values import CAR
from openpilot.selfdrive.controls.lib.longcontrol import LongCtrlState
from openpilot.selfdrive.controls.lib.longitudinal_planner import LongitudinalPlanner, get_vehicle_min_accel
from openpilot.selfdrive.modeld.constants import ModelConstants, Plan


def make_lead(*, status: bool, d_rel: float = 200.0, v_lead: float = 0.0, a_lead: float = 0.0):
  lead = log.RadarState.LeadData.new_message()
  lead.status = status
  lead.dRel = d_rel
  lead.vLead = v_lead
  lead.vLeadK = v_lead
  lead.aLeadK = a_lead
  lead.vRel = 0.0
  lead.aRel = 0.0
  lead.modelProb = 0.0
  return lead


def make_model(v_ego: float, desired_accel: float, gas_press_prob: float = 1.0):
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

  model.meta.disengagePredictions.gasPressProbs = [float(gas_press_prob)] * 6
  model.action.desiredAcceleration = desired_accel
  model.action.shouldStop = False
  return model


def make_sm(v_ego: float, desired_accel: float, min_accel: float, *, experimental_mode: bool = True,
            tracking_lead: bool = False, lead_one=None, lead_two=None,
            gas_press_prob: float = 1.0, disable_throttle: bool = False):
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
    "modelV2": make_model(v_ego, desired_accel, gas_press_prob=gas_press_prob),
    "radarState": SimpleNamespace(
      leadOne=lead_one if lead_one is not None else make_lead(status=False),
      leadTwo=lead_two if lead_two is not None else make_lead(status=False),
    ),
    "selfdriveState": SimpleNamespace(enabled=True, experimentalMode=experimental_mode, personality=0),
    "starpilotPlan": SimpleNamespace(
      vCruise=v_ego + 5.0,
      minAcceleration=min_accel,
      maxAcceleration=2.0,
      disableThrottle=disable_throttle,
      trackingLead=tracking_lead,
      accelerationJerk=5.0,
      dangerJerk=5.0,
      speedJerk=5.0,
      dangerFactor=1.0,
      tFollow=1.45,
      forcingStopLength=2,
    ),
  }


def make_toggles(model_version: str = "v11"):
  return SimpleNamespace(
    taco_tune=False,
    classic_model=False,
    tinygrad_model=True,
    model_version=model_version,
    stop_distance=6.0,
    vEgoStopping=0.5,
  )


@pytest.mark.parametrize("model_version", ["v11", "v12"])
def test_experimental_mlsim_uses_vehicle_min_accel_floor(model_version):
  v_ego = 18.0
  desired_accel = -1.0
  comfort_min_accel = -0.5

  CP = CarInterface.get_non_essential_params(CAR.HONDA_CIVIC)
  planner = LongitudinalPlanner(CP, init_v=v_ego)
  sm = make_sm(v_ego, desired_accel, comfort_min_accel)

  vehicle_min_accel = get_vehicle_min_accel(CP, v_ego)
  assert vehicle_min_accel < comfort_min_accel

  planner.update(sm, make_toggles(model_version))

  assert planner.mode == "blended"
  assert planner.mlsim
  assert planner.output_a_target == pytest.approx(desired_accel, abs=1e-3)
  assert planner.output_a_target < comfort_min_accel


@pytest.mark.parametrize("model_version", ["v11", "v12"])
def test_acc_mode_uses_close_raw_lead_when_tracking_lead_is_debounced(model_version):
  v_ego = 5.0

  CP = CarInterface.get_non_essential_params(CAR.HONDA_CIVIC)
  planner = LongitudinalPlanner(CP, init_v=v_ego)
  sm = make_sm(
    v_ego,
    desired_accel=-0.6,
    min_accel=-1.0,
    experimental_mode=False,
    tracking_lead=False,
    lead_one=make_lead(status=True, d_rel=24.0, v_lead=0.3),
  )
  sm["starpilotPlan"].vCruise = v_ego + 12.0

  planner.update(sm, make_toggles(model_version))

  assert planner.mode == "acc"
  assert planner.raw_close_lead_needs_control(sm["radarState"].leadOne, v_ego)
  assert planner.output_a_target == pytest.approx(
    planner.get_close_lead_brake_cap(sm["radarState"].leadOne, v_ego, sm["starpilotPlan"].minAcceleration)
  )


def test_modeld_action_passes_tomb_raider_longitudinal_params(monkeypatch):
  monkeypatch.setenv("DEBUG", "0")
  fake_commonmodel = types.ModuleType("openpilot.selfdrive.modeld.models.commonmodel_pyx")
  fake_commonmodel.DrivingModelFrame = object
  fake_commonmodel.CLContext = object
  monkeypatch.setitem(sys.modules, fake_commonmodel.__name__, fake_commonmodel)

  from openpilot.selfdrive.modeld import modeld

  captured = {}

  def fake_get_accel_from_plan(speeds, accels, t_idxs, *, action_t, vEgoStopping):
    captured["speeds"] = speeds
    captured["accels"] = accels
    captured["t_idxs"] = t_idxs
    captured["action_t"] = action_t
    captured["vEgoStopping"] = vEgoStopping
    return 0.4, True

  monkeypatch.setattr(modeld, "get_accel_from_plan_tomb_raider", fake_get_accel_from_plan)

  plan = np.zeros((1, ModelConstants.IDX_N, ModelConstants.PLAN_WIDTH), dtype=np.float32)
  plan[0, :, Plan.VELOCITY] = 3.0
  plan[0, :, Plan.ACCELERATION] = -0.1
  prev_action = log.ModelDataV2.Action.new_message()
  toggles = SimpleNamespace(vEgoStopping=0.42)

  action = modeld.get_action_from_model(
    {"plan": plan},
    prev_action,
    lat_action_t=0.2,
    long_action_t=0.73,
    v_ego=5.0,
    mlsim=True,
    is_v9=True,
    starpilot_toggles=toggles,
  )

  assert captured["action_t"] == pytest.approx(0.73)
  assert captured["vEgoStopping"] == pytest.approx(0.42)
  assert list(captured["t_idxs"]) == ModelConstants.T_IDXS
  np.testing.assert_allclose(captured["speeds"], 3.0)
  np.testing.assert_allclose(captured["accels"], -0.1)
  assert action.shouldStop


def test_allow_throttle_hysteresis_filters_gas_prob_chatter():
  v_ego = 10.0

  CP = CarInterface.get_non_essential_params(CAR.HONDA_CIVIC)
  planner = LongitudinalPlanner(CP, init_v=v_ego)
  sm = make_sm(v_ego, desired_accel=0.0, min_accel=-1.0, experimental_mode=False, gas_press_prob=0.5)
  toggles = make_toggles()

  planner.update(sm, toggles)
  assert planner.model_allow_throttle
  assert planner.allow_throttle

  sm["modelV2"] = make_model(v_ego, desired_accel=0.0, gas_press_prob=0.37)
  planner.update(sm, toggles)
  assert planner.model_allow_throttle
  assert planner.allow_throttle

  sm["modelV2"] = make_model(v_ego, desired_accel=0.0, gas_press_prob=0.34)
  planner.update(sm, toggles)
  assert not planner.model_allow_throttle
  assert not planner.allow_throttle

  sm["modelV2"] = make_model(v_ego, desired_accel=0.0, gas_press_prob=0.43)
  planner.update(sm, toggles)
  assert not planner.model_allow_throttle
  assert not planner.allow_throttle

  sm["modelV2"] = make_model(v_ego, desired_accel=0.0, gas_press_prob=0.46)
  planner.update(sm, toggles)
  assert planner.model_allow_throttle
  assert planner.allow_throttle
