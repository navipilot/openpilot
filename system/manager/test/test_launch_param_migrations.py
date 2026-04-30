from pathlib import Path

from openpilot.system.manager.launch_param_migrations import (
  DEFAULT_STEER_KP,
  LAUNCH_PARAM_MIGRATION_MARKER,
  apply_launch_param_migrations,
)


class FileBackedFakeParams:
  def __init__(self, root: Path):
    self.root = root
    self.root.mkdir(parents=True, exist_ok=True)

  def get_param_path(self, key=""):
    if key:
      return str(self.root / (key.decode() if isinstance(key, bytes) else str(key)))
    return str(self.root)

  def get(self, key):
    path = Path(self.get_param_path(key))
    if not path.is_file():
      return None
    return path.read_text(encoding="utf-8")

  def get_bool(self, key):
    value = self.get(key)
    return value == "1"

  def get_float(self, key):
    value = self.get(key)
    return float(value) if value is not None else 0.0

  def put_bool(self, key, value):
    Path(self.get_param_path(key)).write_text("1" if value else "0", encoding="utf-8")

  def put_float(self, key, value):
    Path(self.get_param_path(key)).write_text(str(float(value)), encoding="utf-8")


def test_apply_launch_param_migrations_sets_branch_defaults_once(tmp_path):
  params = FileBackedFakeParams(tmp_path / "params")

  params.put_bool("LongPitch", False)
  params.put_float("SteerKP", 0.7)
  params.put_float("SteerKPStock", 1.0)

  apply_launch_param_migrations(params)

  assert params.get_bool("LongPitch")
  assert params.get_float("SteerKP") == DEFAULT_STEER_KP
  assert params.get_float("SteerKPStock") == DEFAULT_STEER_KP
  assert (tmp_path / "params" / LAUNCH_PARAM_MIGRATION_MARKER).is_file()


def test_apply_launch_param_migrations_initializes_use_prebuilt(tmp_path):
  params = FileBackedFakeParams(tmp_path / "params")

  apply_launch_param_migrations(params)

  assert params.get_bool("UsePrebuilt")


def test_apply_launch_param_migrations_does_not_overwrite_use_prebuilt(tmp_path):
  params = FileBackedFakeParams(tmp_path / "params")
  params.put_bool("UsePrebuilt", False)

  apply_launch_param_migrations(params)

  assert not params.get_bool("UsePrebuilt")


def test_apply_launch_param_migrations_does_not_reapply_after_marker(tmp_path):
  params = FileBackedFakeParams(tmp_path / "params")
  marker = tmp_path / "params" / LAUNCH_PARAM_MIGRATION_MARKER

  params.put_bool("LongPitch", False)
  params.put_float("SteerKP", 0.65)
  params.put_float("SteerKPStock", DEFAULT_STEER_KP)
  marker.touch()

  apply_launch_param_migrations(params, marker)

  assert not params.get_bool("LongPitch")
  assert params.get_float("SteerKP") == 0.65
  assert params.get_float("SteerKPStock") == DEFAULT_STEER_KP
