import os
import pytest
import signal
import time
from pathlib import Path
import json

from cereal import car
from openpilot.common.params import Params
import openpilot.system.manager.manager as manager
from openpilot.system.manager.process import ensure_running
from openpilot.system.manager.process_config import managed_processes, procs
from openpilot.system.hardware import HARDWARE

os.environ['FAKEUPLOAD'] = "1"

MAX_STARTUP_TIME = 3
BLACKLIST_PROCS = ['manage_athenad', 'pandad', 'pigeond']


class FileBackedFakeParams:
  def __init__(self, root: Path, values: dict[str, object] | None = None):
    self.root = root
    self.root.mkdir(parents=True, exist_ok=True)
    for key, value in (values or {}).items():
      self.put(key, value)

  def get_param_path(self, key):
    return str(self.root / (key.decode() if isinstance(key, bytes) else str(key)))

  def get(self, key):
    path = Path(self.get_param_path(key))
    if not path.is_file():
      return None

    raw = path.read_bytes()
    try:
      return raw.decode("utf-8")
    except UnicodeDecodeError:
      return raw

  def get_bool(self, key):
    value = self.get(key)
    if value is None:
      return False
    if isinstance(value, bytes):
      value = value.decode("utf-8", errors="ignore")
    return str(value).strip().lower() in ("1", "true", "yes", "on")

  def put(self, key, value):
    path = Path(self.get_param_path(key))
    path.parent.mkdir(parents=True, exist_ok=True)

    if isinstance(value, bytes):
      raw = value
    elif isinstance(value, bool):
      raw = b"1" if value else b"0"
    elif isinstance(value, float):
      raw = str(float(value)).encode("utf-8")
    elif isinstance(value, (dict, list)):
      raw = json.dumps(value, separators=(",", ":")).encode("utf-8")
    else:
      raw = str(value).encode("utf-8")

    path.write_bytes(raw)

  def put_bool(self, key, value):
    self.put(key, bool(value))

  def put_float(self, key, value):
    self.put(key, float(value))


class TestManager:
  def setup_method(self):
    HARDWARE.set_power_save(False)

    # ensure clean CarParams
    params = Params()
    params.clear_all()

  def teardown_method(self):
    manager.manager_cleanup()

  def test_manager_prepare(self):
    os.environ['PREPAREONLY'] = '1'
    manager.main()

  def test_duplicate_procs(self):
    assert len(procs) == len(managed_processes), "Duplicate process names"

  def test_blacklisted_procs(self):
    # TODO: ensure there are blacklisted procs until we have a dedicated test
    assert len(BLACKLIST_PROCS), "No blacklisted procs to test not_run"

  def test_set_params_with_default_value(self):
    params = Params()
    params.clear_all()

    os.environ['PREPAREONLY'] = '1'
    manager.main()
    for k in params.all_keys():
      default_value = params.get_default_value(k)
      if default_value is not None:
        assert params.get(k) == default_value
    assert params.get("OpenpilotEnabledToggle")
    assert params.get("RouteCount") == 0

  def test_migrate_legacy_experimental_longitudinal(self):
    class FakeParams:
      def __init__(self, values):
        self.values = dict(values)

      def get(self, key):
        return self.values.get(key)

      def get_bool(self, key):
        value = self.values.get(key)
        if value is None:
          return False
        if isinstance(value, bytes):
          value = value.decode("utf-8", errors="ignore")
        if isinstance(value, str):
          return value.strip().lower() in ("1", "true", "yes", "on")
        return bool(value)

      def put_bool(self, key, value):
        self.values[key] = b"1" if value else b"0"

      def remove(self, key):
        self.values.pop(key, None)

    params = FakeParams({"ExperimentalLongitudinalEnabled": b"1"})
    params_cache = FakeParams({})

    manager.migrate_legacy_experimental_longitudinal(params, params_cache)

    assert params.get_bool("AlphaLongitudinalEnabled")
    assert params_cache.get_bool("AlphaLongitudinalEnabled")
    assert params.get("ExperimentalLongitudinalEnabled") is None
    assert params_cache.get("ExperimentalLongitudinalEnabled") is None

  def test_migrate_starpilot_default_parity_preserves_existing_values(self, tmp_path, monkeypatch):
    monkeypatch.setattr(manager, "STARPILOT_DEFAULTS_PARITY_MIGRATION_FLAG", tmp_path / "starpilot_defaults_parity_v1")

    params = FileBackedFakeParams(tmp_path / "params", {
      "AdvancedLateralTune": False,
      "ForceAutoTuneOff": False,
      "HumanAcceleration": True,
      "CEModelStopTime": 3.5,
    })
    params_cache = FileBackedFakeParams(tmp_path / "cache", {
      "NNFF": True,
    })

    manager.migrate_starpilot_default_parity(params, params_cache)

    assert not params.get_bool("AdvancedLateralTune")
    assert not params.get_bool("ForceAutoTuneOff")
    assert params.get_bool("HumanAcceleration")
    assert params.get("CEModelStopTime") == "3.5"
    assert params_cache.get_bool("NNFF")

    assert Path(params.get_param_path("HumanFollowing")).is_file()
    assert not params.get_bool("HumanFollowing")

  def test_migrate_disable_humanlike_defaults(self, tmp_path, monkeypatch):
    monkeypatch.setattr(manager, "STARPILOT_HUMANLIKE_DISABLE_MIGRATION_FLAG", tmp_path / "starpilot_humanlike_disable_v1")

    params = FileBackedFakeParams(tmp_path / "params", {
      "HumanAcceleration": True,
      "HumanFollowing": True,
    })
    params_cache = FileBackedFakeParams(tmp_path / "cache", {
      "HumanLaneChanges": True,
    })

    manager.migrate_disable_humanlike_defaults(params, params_cache)

    assert not params.get_bool("HumanAcceleration")
    assert not params.get_bool("HumanFollowing")
    assert not params.get_bool("HumanLaneChanges")
    assert not params_cache.get_bool("HumanAcceleration")
    assert not params_cache.get_bool("HumanFollowing")
    assert not params_cache.get_bool("HumanLaneChanges")

  @pytest.mark.skip("this test is flaky the way it's currently written, should be moved to test_onroad")
  def test_clean_exit(self, subtests):
    """
      Ensure all processes exit cleanly when stopped.
    """
    HARDWARE.set_power_save(False)
    manager.manager_init()

    CP = car.CarParams.new_message()
    procs = ensure_running(managed_processes.values(), True, Params(), CP, not_run=BLACKLIST_PROCS)

    time.sleep(10)

    for p in procs:
      with subtests.test(proc=p.name):
        state = p.get_process_state_msg()
        assert state.running, f"{p.name} not running"
        exit_code = p.stop(retry=False)

        assert p.name not in BLACKLIST_PROCS, f"{p.name} was started"

        assert exit_code is not None, f"{p.name} failed to exit"

        # TODO: interrupted blocking read exits with 1 in cereal. use a more unique return code
        exit_codes = [0, 1]
        if p.sigkill:
          exit_codes = [-signal.SIGKILL]
        assert exit_code in exit_codes, f"{p.name} died with {exit_code}"
