"""
Copyright (c) 2021-, Haibin Wen, sunnypilot, and a number of other contributors.

This file is part of sunnypilot and is licensed under the MIT License.
See the LICENSE.md file in the root directory for more details.
"""
LIMIT_ADAPT_ACC = -1.  # m/s^2 Ideal acceleration for the adapting (braking) phase when approaching speed limits.
LIMIT_MAX_MAP_DATA_AGE = 10.  # s Maximum time to hold to map data, then consider it invalid inside limits controllers.

# Speed Limit Assist constants
PCM_LONG_REQUIRED_MAX_SET_SPEED = {
  True: (33.3333, 36.1111),  # km/h, (120, 130)
  False: (31.2928, 35.7632),  # mph, (70, 80)
}

# Tesla 使用仪表显示的巡航设定速度确认限速辅助。
# 速度滚轮每次变化一个显示单位，因此保留 1 km/h 或 1 mph 的目标精度。
TESLA_PCM_LONG_REQUIRED_MAX_SET_SPEED = {
  True: tuple((speed, speed / 3.6) for speed in range(20, 131)),
  False: tuple((speed, speed * 0.44704) for speed in range(15, 91)),
}

CONFIRM_SPEED_THRESHOLD = {
  True: 80,   # km/h
  False: 50,  # mph
}


def resolve_pcm_long_required_max(metric: bool, limit_conv: int, has_speed_limit: bool, *, brand: str) -> float:
  if brand != "tesla":
    # 非 Tesla 车型保持原有逻辑，避免改变现有行为。
    cst_low, cst_high = PCM_LONG_REQUIRED_MAX_SET_SPEED[metric]
    return cst_low if has_speed_limit and limit_conv < CONFIRM_SPEED_THRESHOLD[metric] else cst_high

  segments = TESLA_PCM_LONG_REQUIRED_MAX_SET_SPEED[metric]
  if not has_speed_limit:
    # 没有有效限速时使用最高目标，避免把无效的零限速作为设定速度。
    return segments[-1][1]

  # Tesla 的目标跟随实际限速，并向上匹配到仪表支持的速度档位。
  return next((value for threshold, value in segments if limit_conv <= threshold), segments[-1][1])
