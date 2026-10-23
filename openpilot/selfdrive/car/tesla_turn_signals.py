"""Tesla lamp execution adapted from xpilot's bounded DAS_bodyControls sessions."""
from dataclasses import dataclass

from opendbc.car.can_definitions import CanData

BODY = 0x3E9
UI = 0x311
FRONT = 0x3F5
TEMPLATE_NS = 1_500_000_000
CONTEXT_NS = 200_000_000
FEEDBACK_NS = 500_000_000
ECHO_NS = 1_200_000_000
SESSION_NS = 60_000_000_000
CANCEL_NS = 5_000_000_000
OFF_STABLE_NS = 500_000_000


def body_checksum(data: bytes | bytearray) -> int:
  if len(data) != 8:
    raise ValueError("Tesla body controls require eight bytes")
  return (0xE9 + 3 + sum(data[:7])) & 0xFF


def body_frame(template: bytes, direction: int) -> bytes:
  if direction not in (1, 2, 3) or len(template) != 8 or template[1] & 3 or body_checksum(template) != template[7]:
    raise ValueError("Invalid Tesla turn request or OEM idle template")
  data = bytearray(template)
  data[1] = (data[1] & 0xFC) | direction
  data[2] = (data[2] & 0xE1) | ((4 if direction == 3 else 8) << 1)
  data[6] = (data[6] & 15) | ((((data[6] >> 4) + 1) & 15) << 4)
  data[7] = body_checksum(data)
  return bytes(data)


@dataclass
class Session:
  request_id: int
  direction: int
  started: int
  sent: int = 0
  awaiting: bytes | None = None
  submitted: int = 0
  cancelling: bool = False
  cancel_started: int = 0
  cancel_attempts: int = 0
  cancel_echo: bool = False
  manual: bool = False
  on_seen: bool = False
  off_since: int | None = None


class TeslaTurnSignals:
  def __init__(self, configured: bool):
    self.configured = configured
    self.session: Session | None = None
    self.template: bytes | None = None
    self.template_time = 0
    self.ui = (0, 0)
    self.front = (3, 3)
    self.ui_time = self.front_time = 0
    self.context_time = 0
    self.blocked_id = 0
    self.status = "idle" if configured else "disabled"
    self.last_stalk_counts: tuple[int, int] | None = None
    self.failed = False

  @property
  def owned_direction(self) -> int:
    return self.session.direction if self.session is not None and not self.session.manual else 0

  def _feedback_fresh(self, now: int) -> bool:
    return all(t > 0 and 0 <= now - t <= FEEDBACK_NS for t in (self.ui_time, self.front_time))

  def _off(self, now: int) -> bool:
    return self._feedback_fresh(now) and self.ui == (0, 0) and self.front == (0, 0)

  def _cancel(self, now: int, reason: str) -> None:
    s = self.session
    if s is None or s.cancelling:
      return
    self.blocked_id = s.request_id
    if not s.sent:
      self.session = None
      self.status = "cancelled_before_send"
      return
    s.cancelling, s.cancel_started = True, now
    self.status = f"cancelling:{reason}"

  def _fail(self, reason: str) -> None:
    self.failed = True
    self.status = f"fault:{reason}"
    if self.session is not None:
      self.blocked_id = self.session.request_id

  def observe(self, packets, now: int) -> None:
    if not self.configured:
      return
    for timestamp, frames in packets:
      for address, payload, source in frames:
        data = bytes(payload)
        if source == 1 and address == BODY and len(data) == 8 and not data[1] & 3 and body_checksum(data) == data[7]:
          self.template, self.template_time = data, timestamp
        elif source == 0 and address == UI and len(data) == 7:
          # UI_warning is Motorola left at bit 25 and Intel right at bit 26.
          if data[0] == ((0x11 + 3 + sum(data[1:])) & 0xFF):
            self.ui = ((data[3] >> 0) & 3, (data[3] >> 2) & 3)
            self.ui_time = timestamp
        elif source == 1 and address == FRONT and len(data) == 8:
          self.front = ((data[6] >> 2) & 3, (data[6] >> 4) & 3)
          self.front_time = timestamp
        s = self.session
        if s is not None and address == BODY and data == s.awaiting and source in (0x81, 0xC1):
          s.awaiting = None
          if source == 0xC1:
            if s.cancelling:
              self._fail("panda_rejected_cancel")
            else:
              self._cancel(now, "panda_rejected")
          elif s.cancelling and data[1] & 3 == 3:
            s.cancel_echo = True
        if s is not None and self._feedback_fresh(now):
          if self.ui[s.direction - 1] in (1, 2) and self.front[s.direction - 1] == 1:
            s.on_seen = True
          if 3 in self.ui or any(v in (2, 3) for v in self.front):
            self._cancel(now, "invalid_lamp_feedback")
    self.advance(now)

  def update(self, state, control, meta, fresh: bool, now: int) -> list[CanData]:
    self.context_time = now if fresh else 0
    counts = (int(state.leftBlinkerStalkCount), int(state.rightBlinkerStalkCount))
    pressed = self.last_stalk_counts is not None and counts != self.last_stalk_counts
    self.last_stalk_counts = counts
    driver = state.turnSignalDriverValid and state.turnSignalDriverDirection != 0
    hazards = all(v in (1, 2) for v in self.ui)
    s = self.session
    if s is not None and (driver or pressed or hazards):
      s.manual = True
      self._cancel(now, "driver")

    direction = meta.autoTurnSignalDirection.raw if fresh else 0
    request_id = int(meta.autoTurnSignalRequestId) if fresh else 0
    allowed = (self.configured and fresh and control.latActive and state.canValid and
               str(state.gearShifter) == "drive" and state.turnSignalDriverValid and not driver and not pressed and not hazards)
    if not allowed or direction not in (1, 2) or not request_id:
      self._cancel(now, "intent_or_control_inactive")
    elif s is not None:
      if request_id != s.request_id or direction != s.direction:
        self._cancel(now, "request_changed")
    elif not self.failed and request_id != self.blocked_id:
      if self._off(now) and self.template is not None and 0 <= now - self.template_time <= TEMPLATE_NS:
        self.session = Session(request_id, direction, now)
        self.status = "active"
      else:
        self.status = "waiting_for_vehicle_feedback"
    self.advance(now)
    return self.take_sends(now)

  def advance(self, now: int) -> None:
    s = self.session
    if s is None:
      return
    if self.failed:
      # Retain provenance after an unconfirmed cancellation, until actually off.
      if self._off(now):
        if s.off_since is None:
          s.off_since = now
        elif now - s.off_since >= OFF_STABLE_NS:
          self.session = None
      else:
        s.off_since = None
      return
    if not s.cancelling:
      if not self.context_time or not 0 <= now - self.context_time <= CONTEXT_NS:
        self._cancel(now, "context_stale")
      elif now - s.started >= SESSION_NS:
        self._cancel(now, "session_timeout")
      elif not self._feedback_fresh(now):
        self._cancel(now, "feedback_stale")
      elif not s.on_seen and now - s.started >= 2_500_000_000:
        self._cancel(now, "lamp_not_confirmed")
    s = self.session
    if s is None:
      return
    if s.awaiting is not None and now - s.submitted >= ECHO_NS:
      s.awaiting = None
      self._cancel(now, "tx_echo_timeout")
    if s.cancelling:
      if s.manual and s.cancel_echo:
        self.status = "driver_takeover"
        self.session = None
        return
      if s.cancel_echo and self._off(now):
        if s.off_since is None:
          s.off_since = now
        elif now - s.off_since >= OFF_STABLE_NS:
          self.status = "cancelled"
          self.session = None
          return
      else:
        s.off_since = None
      if now - s.cancel_started >= CANCEL_NS:
        self._fail("cancel_not_confirmed")

  def take_sends(self, now: int, *, cancel_only: bool = False) -> list[CanData]:
    s = self.session
    if (s is None or self.failed or s.awaiting is not None or s.cancel_echo or
        (cancel_only and not s.cancelling) or self.template is None or
        not 0 <= now - self.template_time <= TEMPLATE_NS):
      return []
    if s.cancelling and s.cancel_attempts >= 3:
      return []
    data = body_frame(self.template, 3 if s.cancelling else s.direction)
    self.template = None
    s.awaiting, s.submitted = data, now
    s.sent += 1
    s.cancel_attempts += int(s.cancelling)
    return [CanData(BODY, data, 1)]
