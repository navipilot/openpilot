import pytest

from openpilot.common.constants import CV
from openpilot.starpilot.controls.lib.starpilot_vcruise import get_active_slc_control_target


def test_active_slc_control_target_requires_set_speed_limit():
  target = get_active_slc_control_target(
    speed_limit_controller=True,
    set_speed_limit=False,
    slc_target=45.0 * CV.MPH_TO_MS,
    slc_offset=3.0 * CV.MPH_TO_MS,
    overridden_speed=0.0,
    v_ego_diff=0.4,
  )

  assert target == 0.0


def test_active_slc_control_target_applies_offset_and_cluster_diff():
  target = get_active_slc_control_target(
    speed_limit_controller=True,
    set_speed_limit=True,
    slc_target=45.0 * CV.MPH_TO_MS,
    slc_offset=3.0 * CV.MPH_TO_MS,
    overridden_speed=0.0,
    v_ego_diff=0.4,
  )

  assert target == pytest.approx((48.0 * CV.MPH_TO_MS) - 0.4)
