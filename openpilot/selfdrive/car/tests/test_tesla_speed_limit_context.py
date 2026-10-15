from types import SimpleNamespace

from openpilot.selfdrive.car.card import get_tesla_speed_limit_context


class FakeSubMaster:
  def __init__(self, assist_enabled: bool, recv_time: float = 10.0):
    self.seen = {"longitudinalPlanSP": True}
    self.valid = {"longitudinalPlanSP": True}
    self.recv_time = {"longitudinalPlanSP": recv_time}
    self.plan = SimpleNamespace(speedLimit=SimpleNamespace(
      assist=SimpleNamespace(enabled=assist_enabled),
      resolver=SimpleNamespace(speedLimitValid=True, speedLimitLastValid=False, speedLimitFinalLast=25.0),
    ))

  def __getitem__(self, service):
    assert service == "longitudinalPlanSP"
    return self.plan


def test_tesla_speed_limit_target_requires_assist_mode():
  assert get_tesla_speed_limit_context(FakeSubMaster(False), 10.1) == (0.0, False, 10.0)
  assert get_tesla_speed_limit_context(FakeSubMaster(True), 10.1) == (25.0, True, 10.0)


def test_tesla_speed_limit_target_rejects_stale_or_missing_limit():
  sm = FakeSubMaster(True, recv_time=9.7)
  assert get_tesla_speed_limit_context(sm, 10.1) == (0.0, False, 9.7)

  sm.recv_time["longitudinalPlanSP"] = 10.0
  sm.plan.speedLimit.resolver.speedLimitValid = False
  assert get_tesla_speed_limit_context(sm, 10.1) == (0.0, False, 10.0)
