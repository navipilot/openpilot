from opendbc.car.can_definitions import CanData
from opendbc.sunnypilot.car.tesla.values import TeslaFlagsSP


SWITCH_STATUS_ADDRESS = 0x3C2
VEHICLE_BUS = 1
TEMPLATE_MAX_AGE_NS = 1_500_000_000
MIN_TX_INTERVAL_NS = 500_000_000
FEEDBACK_TIMEOUT_NS = 1_200_000_000
TARGET_STABLE_NS = 500_000_000
KPH_TO_MS = 1.0 / 3.6
MPH_TO_MS = 0.44704


def create_speed_wheel_frame(template: bytes, tick: int) -> bytes:
  # 克隆原车空闲帧，只替换右滚轮的有符号单格值。
  if len(template) != 8 or (template[0] & 0x03) != 1 or (template[3] & 0x3F) != 0:
    raise ValueError("Tesla speed-wheel template must be an idle 0x3C2 mux-1 frame")
  if tick not in (-1, 1):
    raise ValueError("Tesla speed-wheel tick must be -1 or +1")

  data = bytearray(template)
  data[3] = (data[3] & 0xC0) | (tick & 0x3F)
  return bytes(data)


class TeslaSpeedLimitController:
  def __init__(self, CP_SP):
    self.configured = bool(CP_SP.flags & TeslaFlagsSP.AUTO_SPEED_LIMIT)
    self.last_tx_nanos = 0
    self.pending_since_nanos = 0
    self.pending_direction = 0
    self.pending_speed_display = 0
    self.planned_target_display = 0
    self.current_display = 0
    self.target_display = 0
    self.remaining_steps = 0
    self.feedback_blocked_signature = None
    self.manual_adjustment_counter_seen = None
    self.resume_gesture_counter_seen = None
    self.manual_override_active = False
    self.manual_resume_feedback_guard_until_nanos = 0
    self.last_current_display = None
    self.target_change_nanos = 0
    self.target_stabilizing = False

  def _reset_pending(self) -> None:
    self.pending_since_nanos = 0
    self.pending_direction = 0

  def _reset(self, *, clear_manual_override: bool) -> None:
    self._reset_pending()
    self.remaining_steps = 0
    self.feedback_blocked_signature = None
    self.last_current_display = None
    self.planned_target_display = 0
    self.target_change_nanos = 0
    self.target_stabilizing = False
    if clear_manual_override:
      self.manual_resume_feedback_guard_until_nanos = 0
      self.manual_override_active = False

  def _sync_manual_counters(self, CS) -> tuple[bool, bool]:
    manual_counter = int(getattr(CS, "tesla_manual_speed_adjustment_counter", 0))
    resume_counter = int(getattr(CS, "tesla_speed_auto_resume_gesture_counter", 0))
    if self.manual_adjustment_counter_seen is None:
      self.manual_adjustment_counter_seen = manual_counter
      self.resume_gesture_counter_seen = resume_counter
      return False, False

    manual_changed = manual_counter != self.manual_adjustment_counter_seen
    resume_changed = resume_counter != self.resume_gesture_counter_seen
    self.manual_adjustment_counter_seen = manual_counter
    self.resume_gesture_counter_seen = resume_counter
    return manual_changed, resume_changed

  @staticmethod
  def _to_display_speed(speed_ms: float, speed_units: str) -> int:
    unit_ms = MPH_TO_MS if speed_units == "MPH" else KPH_TO_MS
    return int(max(0.0, speed_ms) / unit_ms + 0.5)

  def update(self, CC, CS, now_nanos: int) -> list[CanData]:
    manual_changed, resume_changed = self._sync_manual_counters(CS)
    if not self.configured or not CC.enabled or CC.cruiseControl.cancel or not CS.out.cruiseState.enabled:
      self._reset(clear_manual_override=True)
      return []
    if CS.out.brakePressed or not getattr(CS, "tesla_speed_limit_target_valid", False):
      self._reset(clear_manual_override=False)
      return []

    current_display = self._to_display_speed(float(CS.out.cruiseState.speedCluster), str(getattr(CS, "tesla_speed_units", "KPH")))
    target_display = self._to_display_speed(float(CS.tesla_speed_limit_target), str(getattr(CS, "tesla_speed_units", "KPH")))
    self.current_display = current_display
    self.target_display = target_display
    signature = (target_display, current_display)

    # 按车辆显示单位取整，防止 m/s 浮点误差造成来回滚动。
    target_changed = target_display != self.planned_target_display
    if target_changed:
      self._reset_pending()
      self.feedback_blocked_signature = None
      self.planned_target_display = target_display
      self.target_change_nanos = now_nanos
      self.target_stabilizing = True
      self.manual_resume_feedback_guard_until_nanos = 0
      self.manual_override_active = False

    # 人工滚轮操作优先：普通操作暂停自动同步，双向手势明确恢复。
    if resume_changed:
      self.manual_resume_feedback_guard_until_nanos = now_nanos + FEEDBACK_TIMEOUT_NS
      self.manual_override_active = False
    elif manual_changed:
      self.manual_resume_feedback_guard_until_nanos = 0
      self.manual_override_active = True
      self._reset_pending()

    resume_guard = now_nanos < self.manual_resume_feedback_guard_until_nanos
    external_change = (self.last_current_display is not None and current_display != self.last_current_display and
                       not self.pending_direction and self.feedback_blocked_signature is None and
                       not self.target_stabilizing and not resume_guard and not target_changed and
                       not manual_changed and not resume_changed)
    self.last_current_display = current_display
    if external_change:
      self.manual_override_active = True

    if self.manual_override_active:
      self.remaining_steps = 0
      return []

    # 等待解析器目标稳定，避免“基础限速→带偏移限速”更新期间先向错误方向滚动。
    if self.target_stabilizing:
      if now_nanos - self.target_change_nanos < TARGET_STABLE_NS:
        self.remaining_steps = target_display - current_display
        return []
      self.target_stabilizing = False

    # 每次只发一格；收到仪表设定速度反馈后才允许下一格，超时则停止重试。
    if self.pending_direction:
      feedback_received = current_display != self.pending_speed_display
      feedback_timed_out = now_nanos - self.pending_since_nanos >= FEEDBACK_TIMEOUT_NS
      if not feedback_received and not feedback_timed_out:
        return []
      self._reset_pending()
      if not feedback_received:
        self.feedback_blocked_signature = signature
        return []

    if self.feedback_blocked_signature is not None:
      if signature == self.feedback_blocked_signature:
        return []
      self.feedback_blocked_signature = None

    self.remaining_steps = target_display - current_display
    if self.remaining_steps == 0:
      return []
    if self.last_tx_nanos and now_nanos - self.last_tx_nanos < MIN_TX_INTERVAL_NS:
      return []

    template = getattr(CS, "tesla_speed_button_template", None)
    template_nanos = int(getattr(CS, "tesla_speed_button_template_nanos", 0))
    if template is None or now_nanos - template_nanos > TEMPLATE_MAX_AGE_NS:
      return []

    direction = 1 if self.remaining_steps > 0 else -1
    data = create_speed_wheel_frame(template, direction)
    self.last_tx_nanos = now_nanos
    self.pending_since_nanos = now_nanos
    self.pending_direction = direction
    self.pending_speed_display = current_display
    return [CanData(SWITCH_STATUS_ADDRESS, data, VEHICLE_BUS)]
