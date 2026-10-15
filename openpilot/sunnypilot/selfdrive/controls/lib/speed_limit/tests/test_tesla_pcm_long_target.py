from openpilot.sunnypilot.selfdrive.controls.lib.speed_limit import resolve_pcm_long_required_max


def test_tesla_pcm_long_target_preserves_fixed_offset():
  # 限速 60 km/h 加 6 km/h 偏移后，自动设速目标必须保持为 66 km/h。
  target = resolve_pcm_long_required_max(True, 66, True, brand="tesla")
  assert round(target * 3.6) == 66
