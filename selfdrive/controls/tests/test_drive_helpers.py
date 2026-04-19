from openpilot.selfdrive.controls.lib.drive_helpers import get_lateral_active


def test_get_lateral_active_requires_enabled_without_aol():
  assert not get_lateral_active(False, True, False, False, False, False, False, True)


def test_get_lateral_active_allows_aol_while_disabled():
  assert get_lateral_active(False, False, True, False, False, False, False, True)
