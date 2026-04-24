from types import SimpleNamespace

from openpilot.selfdrive.car.cruise import ButtonType
from openpilot.starpilot.controls import starpilot_card as sc


class FakeParams:
  def __init__(self, *args, **kwargs):
    self.values = {}

  def get_bool(self, key):
    return bool(self.values.get(key, False))

  def get_int(self, key, default=0):
    return int(self.values.get(key, default))

  def put_bool(self, key, value):
    self.values[key] = bool(value)

  def put_int(self, key, value):
    self.values[key] = int(value)

  def put_bool_nonblocking(self, key, value):
    self.put_bool(key, value)


class FakeSubMaster(dict):
  def __init__(self, *args, updated=None, **kwargs):
    super().__init__(*args, **kwargs)
    self.updated = updated or {}


def build_card(monkeypatch):
  monkeypatch.setattr(sc, "Params", FakeParams)
  monkeypatch.setattr(sc.messaging, "PubMaster", lambda *_args, **_kwargs: SimpleNamespace(send=lambda *_a, **_k: None))
  monkeypatch.setattr(sc, "is_FrogsGoMoo", lambda: False)

  cp = SimpleNamespace(brand="hyundai", openpilotLongitudinalControl=True)
  fpcp = SimpleNamespace(alternativeExperience=0)
  return sc.StarPilotCard(cp, fpcp)


def build_sm(long_active):
  return FakeSubMaster({
    "carControl": SimpleNamespace(longActive=long_active),
    "selfdriveState": SimpleNamespace(active=True, enabled=True, experimentalMode=False, alertType=[]),
    "starpilotPlan": SimpleNamespace(lateralCheck=True),
    "liveCalibration": SimpleNamespace(calPerc=1),
    "starpilotSelfdriveState": SimpleNamespace(alertType=[]),
  }, updated={"starpilotPlan": False})


def build_car_state(button_events):
  return SimpleNamespace(
    buttonEvents=button_events,
    cruiseState=SimpleNamespace(available=False),
    gearShifter=sc.GearShifter.drive,
    brakePressed=False,
    gasPressed=False,
    standstill=False,
    vEgo=15.0,
  )


def build_starpilot_car_state():
  return SimpleNamespace(
    distancePressed=False,
    modePressed=False,
    customPressed=False,
  )


def build_toggles(nostalgia_mode):
  return SimpleNamespace(
    nostalgia_mode=nostalgia_mode,
    always_on_lateral_lkas=False,
    always_on_lateral_main=False,
    always_on_lateral_pause_speed=0.0,
    has_canfd_media_buttons=False,
  )


def test_nostalgia_mode_paddle_pauses_longitudinal(monkeypatch):
  card = build_card(monkeypatch)
  car_state = build_car_state([SimpleNamespace(type=ButtonType.altButton2, pressed=True)])
  starpilot_car_state = build_starpilot_car_state()

  ret = card.update(car_state, starpilot_car_state, build_sm(True), build_toggles(True))

  assert ret.pauseLongitudinal is True


def test_nostalgia_mode_resume_button_clears_pause(monkeypatch):
  card = build_card(monkeypatch)
  toggles = build_toggles(True)

  card.update(
    build_car_state([SimpleNamespace(type=ButtonType.altButton2, pressed=True)]),
    build_starpilot_car_state(),
    build_sm(True),
    toggles,
  )

  ret = card.update(
    build_car_state([SimpleNamespace(type=ButtonType.resumeCruise, pressed=True)]),
    build_starpilot_car_state(),
    build_sm(False),
    toggles,
  )

  assert ret.pauseLongitudinal is False


def test_nostalgia_mode_off_ignores_paddle(monkeypatch):
  card = build_card(monkeypatch)
  car_state = build_car_state([SimpleNamespace(type=ButtonType.altButton2, pressed=True)])
  starpilot_car_state = build_starpilot_car_state()

  ret = card.update(car_state, starpilot_car_state, build_sm(True), build_toggles(False))

  assert ret.pauseLongitudinal is False
