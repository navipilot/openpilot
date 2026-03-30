import importlib.util
import unittest
from pathlib import Path


MODULE_PATH = Path(__file__).resolve().parents[1] / "mici" / "onroad" / "speed_limit_utils.py"
SPEC = importlib.util.spec_from_file_location("speed_limit_utils_under_test", MODULE_PATH)
MODULE = importlib.util.module_from_spec(SPEC)
assert SPEC is not None and SPEC.loader is not None
SPEC.loader.exec_module(MODULE)
resolve_display_speed_limit_ms = MODULE.resolve_display_speed_limit_ms


class TestSpeedLimitUtils(unittest.TestCase):
  def test_resolve_display_speed_limit_prefers_slc_target(self):
    speed_limit = resolve_display_speed_limit_ms(
      slc_speed_limit=24.6,
      speed_limit_source="Dashboard",
      source_limits={
        "Dashboard": 22.4,
        "Map Data": 20.1,
        "Vision": 0.0,
        "Mapbox": 0.0,
      },
      primary_priority="Map Data",
      secondary_priority="Dashboard",
    )

    self.assertEqual(speed_limit, 24.6)

  def test_resolve_display_speed_limit_uses_active_source_when_display_only(self):
    speed_limit = resolve_display_speed_limit_ms(
      slc_speed_limit=0.0,
      speed_limit_source="Dashboard",
      source_limits={
        "Dashboard": 22.4,
        "Map Data": 20.1,
        "Vision": 0.0,
        "Mapbox": 0.0,
      },
      primary_priority="Map Data",
      secondary_priority="Vision",
    )

    self.assertEqual(speed_limit, 22.4)

  def test_resolve_display_speed_limit_honors_priority_order(self):
    speed_limit = resolve_display_speed_limit_ms(
      slc_speed_limit=0.0,
      speed_limit_source="None",
      source_limits={
        "Dashboard": 22.4,
        "Map Data": 20.1,
        "Vision": 24.6,
        "Mapbox": 0.0,
      },
      primary_priority="Map Data",
      secondary_priority="Vision",
    )

    self.assertEqual(speed_limit, 20.1)

  def test_resolve_display_speed_limit_falls_back_to_mapbox(self):
    speed_limit = resolve_display_speed_limit_ms(
      slc_speed_limit=0.0,
      speed_limit_source="None",
      source_limits={
        "Dashboard": 0.0,
        "Map Data": 0.0,
        "Vision": 0.0,
        "Mapbox": 17.9,
      },
      primary_priority="Map Data",
      secondary_priority="Dashboard",
    )

    self.assertEqual(speed_limit, 17.9)
