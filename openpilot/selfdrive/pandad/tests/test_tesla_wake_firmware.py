from types import SimpleNamespace

from openpilot.cereal import car
from openpilot.selfdrive.pandad import pandad


class FakeParams:
  def __init__(self, values):
    self.values = values

  def get_bool(self, key):
    return bool(self.values.get(key, False))

  def get(self, key):
    return self.values.get(key)


def car_params(brand="tesla", fingerprint="TESLA_MODEL_3"):
  CP = car.CarParams.new_message()
  CP.brand = brand
  CP.carFingerprint = fingerprint
  return CP.to_bytes()


def test_tesla_wake_requires_enabled_supported_persistent_vehicle():
  supported = car_params()
  unsupported = car_params(fingerprint="TESLA_MODEL_X")

  assert pandad.tesla_wake_on_can_enabled(FakeParams({"TeslaWakeOnCAN": True, "CarParamsPersistent": supported}))
  assert not pandad.tesla_wake_on_can_enabled(FakeParams({"TeslaWakeOnCAN": False, "CarParamsPersistent": supported}))
  assert not pandad.tesla_wake_on_can_enabled(FakeParams({"TeslaWakeOnCAN": True, "CarParamsPersistent": unsupported}))
  assert not pandad.tesla_wake_on_can_enabled(FakeParams({"TeslaWakeOnCAN": True, "CarParamsPersistent": b"invalid"}))


def test_tesla_wake_firmware_name_matches_panda_mcu(tmp_path, monkeypatch):
  monkeypatch.setattr(pandad, "FW_PATH", str(tmp_path))
  params = FakeParams({"TeslaWakeOnCAN": True, "CarParamsPersistent": car_params()})
  panda = SimpleNamespace(get_mcu_type=lambda: SimpleNamespace(config=SimpleNamespace(app_fn="panda_h7.bin.signed")))

  wake_path = tmp_path / "panda_h7_tesla_wake.bin.signed"
  wake_path.write_bytes(b"firmware")
  assert pandad.get_firmware_path(panda, params) == str(wake_path)

  wake_path.unlink()
  try:
    pandad.get_firmware_path(panda, params)
  except FileNotFoundError as exc:
    assert "Tesla wake firmware is missing" in str(exc)
  else:
    raise AssertionError("missing Tesla wake firmware must fail explicitly")
