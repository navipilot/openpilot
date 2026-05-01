from types import SimpleNamespace

from openpilot.starpilot.controls import starpilot_card as spc


class FakeParams:
  def __init__(self, *args, **kwargs):
    self._store = {}

  def get_bool(self, key):
    return bool(self._store.get(key, False))

  def put_bool(self, key, value):
    self._store[key] = bool(value)

  def get_int(self, key, default=0):
    return int(self._store.get(key, default))

  def put_int(self, key, value):
    self._store[key] = int(value)

  def put_bool_nonblocking(self, key, value):
    self.put_bool(key, value)


class FakeSM(dict):
  def __init__(self, *args, updated=None, **kwargs):
    super().__init__(*args, **kwargs)
    self.updated = updated or {}


def test_honda_lkas_button_can_toggle_always_on_lateral(monkeypatch, tmp_path):
  monkeypatch.setattr(spc, "Params", FakeParams)
  monkeypatch.setattr(spc, "is_FrogsGoMoo", lambda: False)
  monkeypatch.setattr(spc, "ERROR_LOGS_PATH", tmp_path)

  card = spc.StarPilotCard(SimpleNamespace(brand="honda"), SimpleNamespace(alternativeExperience=0))

  car_state = SimpleNamespace(
    buttonEvents=[SimpleNamespace(type=spc.ButtonType.lkas, pressed=True)],
    cruiseState=SimpleNamespace(available=False),
    gearShifter=spc.GearShifter.drive,
    brakePressed=False,
    gasPressed=False,
    standstill=False,
    vEgo=15.0,
  )
  starpilot_car_state = SimpleNamespace(distancePressed=False)
  sm = FakeSM({
    "carControl": SimpleNamespace(longActive=False),
    "selfdriveState": SimpleNamespace(active=False, alertType=[], experimentalMode=False),
    "starpilotSelfdriveState": SimpleNamespace(alertType=[]),
    "starpilotPlan": SimpleNamespace(lateralCheck=True),
    "liveCalibration": SimpleNamespace(calPerc=100),
  }, updated={"starpilotPlan": False})
  toggles = SimpleNamespace(
    always_on_lateral_lkas=True,
    always_on_lateral_main=False,
    always_on_lateral_pause_speed=0.0,
    bookmark_via_lkas=False,
    conditional_experimental_mode=False,
    experimental_mode_via_lkas=False,
    force_coast_via_lkas=False,
    lkas_allowed_for_aol=True,
    pause_lateral_via_lkas=False,
    pause_longitudinal_via_lkas=False,
    speed_limit_controller=False,
    switchback_mode_via_lkas=False,
    traffic_mode_via_lkas=False,
  )

  ret = card.update(car_state, starpilot_car_state, sm, toggles)

  assert ret.alwaysOnLateralAllowed is True
