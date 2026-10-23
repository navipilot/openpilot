import ast
from pathlib import Path
from types import SimpleNamespace as NS

import pytest

from openpilot.cereal import car, log
from openpilot.selfdrive.car.tests.test_tesla_turn_signals import N, feedback
from openpilot.selfdrive.car.tesla_turn_signals import TeslaTurnSignals
from openpilot.selfdrive.controls.lib.lateral_readiness import LateralStartupGate
from openpilot.selfdrive.pandad import can_list_to_can_capnp


@pytest.mark.parametrize("gate", ["ready", "startup", "dashcam", "stale_model", "stale_control"])
def test_final_card_boundary_uses_gated_control_and_fresh_intent(gate):
  path = Path(__file__).parents[1] / "card.py"
  tree = ast.parse(path.read_text())
  method = next(n for n in ast.walk(tree) if isinstance(n, ast.FunctionDef) and n.name == "controls_update")
  ns = {"car": car, "REPLAY": False, "can_list_to_can_capnp": can_list_to_can_capnp,
        "time": NS(monotonic=lambda: N / 1e9, monotonic_ns=lambda: N)}
  exec(compile(ast.Module(body=[method], type_ignores=[]), str(path), "exec"), ns)
  meta = log.ModelDataV2.new_message()
  meta.meta.autoTurnSignalDirection = "left"
  meta.meta.autoTurnSignalRequestId = 42
  messages = {"modelV2": meta.as_reader(), "radarState": None}

  class Messages(dict):
    def all_alive(self, services):
      return True

  sm = Messages(messages)
  sm.valid = sm.alive = sm.seen = dict.fromkeys(("modelV2", "carControl", "radarState"), True)
  sm.recv_time = dict.fromkeys(sm.valid, N / 1e9)
  if gate.startswith("stale"):
    sm.recv_time["modelV2" if gate == "stale_model" else "carControl"] -= 1
  if gate == "startup":
    # A real, unready gate must run before the lamp update.
    sm.all_checks = lambda services: False
  signals = TeslaTurnSignals(True)
  feedback(signals, N)
  params = NS(get_bool=lambda name: gate == "dashcam")
  applied, sent = [], []

  def apply(*args):
    applied.append(args)
    return car.CarControl.Actuators.new_message().as_reader(), []

  controller = NS(apply=apply)
  stages = ("apply", "sendcan", "total")
  card = NS(initialized_prev=True, params=params, sm=sm, CI=controller,
            pm=NS(send=lambda *args: sent.append(args)), tesla_signals=signals,
            lateral_startup=LateralStartupGate(ready=gate != "startup"), card_diag_recv_ns=N,
            card_diag_stage_names=stages, card_diag_stage_current=dict.fromkeys(stages, 0),
            card_diag_stage_sum_us=dict.fromkeys(stages, 0), card_diag_stage_max_us=dict.fromkeys(stages, 0),
            card_diag_process_max_us=0, card_diag_slow_process=0, card_diag_frames=0)
  cs = car.CarState.new_message(canValid=True, gearShifter="drive", turnSignalDriverValid=True)
  cc = car.CarControl.new_message(enabled=True, latActive=True)
  ns["controls_update"](card, cs.as_reader(), cc.as_reader())
  with log.Event.from_bytes(sent[-1][1]) as event:
    assert len(event.sendcan) == (1 if gate == "ready" else 0)
  assert applied[-1][0].latActive == (gate not in ("startup", "dashcam"))
  assert cc.latActive  # Do not mutate the subscribed command.
