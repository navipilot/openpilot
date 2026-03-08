import random


# ============================================================
#  RadarSpeedEstimator：保持 mm/ms 输入，目标消失返回 None
# ============================================================
class RadarSpeedEstimator:
  """
  升级版鲁棒跳变过滤器（单位：mm 输入 / ms 时间戳，输出 m/s）
  """

  def __init__(self, max_acc=4.0, smooth_n=5, lost_timeout_ms=500):
    self.last_dist_m = None
    self.last_t_ms = None
    self.last_speed = None   # 修改：初始化由 0.0 改为 None，更合理
    self.max_acc = max_acc  # m/s²
    self.smooth_n = smooth_n
    self.speed_hist = []
    self.lost_timeout_ms = lost_timeout_ms # 修改点：新增丢失超时参数

  def update(self, dist_mm, t_ms):
    # 处理“距离丢失”逻辑
    if dist_mm is None:
      # 没有历史数据 -> 无法产生速度
      if self.last_t_ms is None:
        return None

      # 距离丢失但未超过超时 -> 保持上一帧速度
      if t_ms - self.last_t_ms < self.lost_timeout_ms:
        return self.last_speed   # 关键改动：不立刻恢复为 None

      # 距离丢失超过超时，真正重置速度
      self.last_dist_m = None
      self.last_t_ms = None
      self.last_speed = None
      self.speed_hist.clear()
      return None

    # 转米
    dist_m = dist_mm / 1000.0

    # 第一帧
    if self.last_dist_m is None or self.last_t_ms is None:
      self.last_dist_m = dist_m
      self.last_t_ms = t_ms
      self.last_speed = 0.0
      self._update_hist(0.0)
      return None

    # 时间差
    dt_ms = t_ms - self.last_t_ms
    if dt_ms <= 0:
      return self.last_speed  # 时间异常，保留上次速度

    dt = dt_ms / 1000.0
    raw_speed = (dist_m - self.last_dist_m) / dt

    # 加速度限制
    allowed_dv = self.max_acc * dt
    low = self.last_speed - allowed_dv
    high = self.last_speed + allowed_dv
    filtered_speed = min(max(raw_speed, low), high)

    # 保存状态
    self.last_dist_m = dist_m
    self.last_t_ms = t_ms
    self.last_speed = filtered_speed
    return self._update_hist(filtered_speed)

  def _update_hist(self, speed):
    """滑动平均（只对非 None 值）"""
    self.speed_hist.append(speed)
    if len(self.speed_hist) > self.smooth_n:
      self.speed_hist.pop(0)
    return sum(self.speed_hist) / len(self.speed_hist)


# ============================================================
#                     测试框架
# ============================================================
def run_scenario(name, distances_mm, dt_ms):
    print("\n" + "=" * 60)
    print(f"Scenario: {name}")
    print("=" * 60)
    print(f"{'t(ms)':>8} | {'dist(mm)':>10} | {'speed(m/s)':>12}")
    print("-" * 40)

    f = RadarSpeedEstimator(max_acc=4.0, smooth_n=5)

    t_ms = 0
    for d in distances_mm:
        v = f.update(d, t_ms)
        if v is None:
          v_str = "None"
        else:
          v_str = f"{v:.2f}"  # 保留 2 位小数
        print(f"{t_ms:>8} | {str(d):>10} | {v_str:>12}")
        t_ms += dt_ms


# ============================================================
#                     场景定义
# ============================================================
def simulate_all():
    dt = 100  # 每帧 100 ms

    # 1. 匀速接近（-2 m/s）
    dist1 = [8000 - i * 200 for i in range(15)]

    # 2. 匀速远离（+1.5 m/s）
    dist2 = [2000 + i * 150 for i in range(15)]

    # 3. 突然插入一个近距离物体
    dist3 = dist1[:7] + [1200] + [1150, 1100, 1050]

    # 4. 丢失 → 回来后突然远
    dist4 = dist1[:7] + [None, None, None, 9000, 8900, 8800]

    # 5. 带噪声（±30mm）的接近
    base = [8000 - i * 200 for i in range(15)]
    dist5 = [d + random.randint(-30, 30) for d in base]

    # 6. 极端跳变（测试鲁棒性）
    dist6 = dist1[:8] + [20000] + dist1[8:]

    # 7. 稳定目标，轻微抖动
    dist7 = [5000 + random.randint(-20, 20) for _ in range(15)]

    # 8. 近目标直接切换到远目标
    dist8 = [2000, 2050, 2100, 8000, 8050, 8100]

    # 9. 目标消失测试（连续 None）
    dist9 = [3000, 2950, 2900, None, None, None, None, 2800, 2750]

    # 10. 目标突然出现测试（前几帧 None，突然有目标）
    dist10 = [None, None, None, 4000, 3950, 3900, 3850]

    # 运行所有场景
    run_scenario("1. constant -2 m/s approach", dist1, dt)
    run_scenario("2. constant +1.5 m/s retreat", dist2, dt)
    run_scenario("3. sudden close target inserted", dist3, dt)
    run_scenario("4. lost then reappear far", dist4, dt)
    run_scenario("5. noisy approach (±30 mm)", dist5, dt)
    run_scenario("6. extreme invalid jump test", dist6, dt)
    run_scenario("7. stable target jitter (±20 mm)", dist7, dt)
    run_scenario("8. near target switch to far", dist8, dt)
    run_scenario("9. target disappears", dist9, dt)
    run_scenario("10. target suddenly appears", dist10, dt)


# ============================================================
#                        主入口
# ============================================================
if __name__ == "__main__":
    simulate_all()
