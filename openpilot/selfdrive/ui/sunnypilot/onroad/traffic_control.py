from __future__ import annotations

from dataclasses import dataclass

import pyray as rl

from openpilot.selfdrive.ui.ui_state import ui_state
from openpilot.selfdrive.ui.onroad.hud_renderer import UI_CONFIG
from openpilot.sunnypilot.selfdrive.traffic_control.controller import TrafficControlPhase
from openpilot.system.ui.lib.application import gui_app
from openpilot.system.ui.widgets import Widget


RED = rl.Color(255, 72, 72, 255)
AMBER = rl.Color(255, 190, 50, 255)
GREEN = rl.Color(53, 220, 118, 255)
LAMP_OFF = rl.Color(65, 69, 76, 220)
BORDER = rl.Color(255, 255, 255, 38)
TRAFFIC_CARD_WIDTH = 64.0
TRAFFIC_CARD_HEIGHT = 128.0
TRAFFIC_CARD_TOP_OFFSET = 47.0
TRAFFIC_LIGHT_HOUSING_WIDTH = 52.0
TRAFFIC_LIGHT_HOUSING_HEIGHT = 116.0
TRAFFIC_LIGHT_RADIUS = 10.0
CONTROL_OUTLINE = rl.Color(64, 156, 255, 255)


def traffic_card_rect(rect: rl.Rectangle) -> rl.Rectangle:
  return rl.Rectangle(
    rect.x + 46.0,
    rect.y + UI_CONFIG.header_height + TRAFFIC_CARD_TOP_OFFSET,
    TRAFFIC_CARD_WIDTH,
    TRAFFIC_CARD_HEIGHT,
  )


def traffic_control_highlighted(state: TrafficSignalDisplayState) -> bool:
  # Blue means control is applied in the current plan.
  return bool(state.visible and state.control_active and not state.driver_override_active
              and state.phase not in (int(TrafficControlPhase.off), int(TrafficControlPhase.passed)))


@dataclass(frozen=True)
class TrafficSignalDisplayState:
  visible: bool = False
  has_signal: bool = False
  control_active: bool = False
  driver_override_active: bool = False
  light_state: int = 0
  phase: int = int(TrafficControlPhase.off)
  flashing: bool = False

  @classmethod
  def from_plan(cls, target, *, valid: bool = True) -> TrafficSignalDisplayState:
    if not valid:
      return cls()
    phase = int(target.phase)
    mode = int(target.mode)
    light = int(target.lightState)
    raw_distance = float(target.rawDistance)
    # Explicit DBC OFF (4) has no ordinary RED/GREEN quality. Only a confirmed
    # flash STOP may animate it as the dark half of a flashing-green signal.
    confirmed_off_flash = phase == int(TrafficControlPhase.flashingGreenStop) and light == 4
    has_signal = bool(
      (int(target.quality) > 0 or confirmed_off_flash)
      and 0.0 <= raw_distance <= 200.0
      and (0 <= light <= 3 or confirmed_off_flash)
      and phase != int(TrafficControlPhase.passed)
    )
    return cls(
      visible=mode == 4,
      has_signal=has_signal,
      control_active=bool(target.applied),
      driver_override_active=bool(target.driverOverrideActive),
      light_state=light,
      phase=phase,
      flashing=has_signal and phase == int(TrafficControlPhase.flashingGreenStop),
    )


_latest_traffic_display_state = TrafficSignalDisplayState()
_latest_traffic_display_frame = 0


def write_traffic_ui_debug(target, sm) -> None:
  plan_available = bool(
    sm.seen["longitudinalPlanSP"]
    and sm.alive["longitudinalPlanSP"]
    and sm.valid["longitudinalPlanSP"]
  )
  target.trafficPlanAvailable = plan_available
  if plan_available:
    plan = sm["longitudinalPlanSP"].teslaTrafficControl
    target.trafficPlanMonoTime = int(sm.logMonoTime["longitudinalPlanSP"])
    target.trafficPlanLightState = int(plan.lightState)
    target.trafficPlanPhase = int(plan.phase)
  target.trafficDisplayFrame = _latest_traffic_display_frame
  target.trafficDisplayedVisible = _latest_traffic_display_state.visible
  target.trafficDisplayedHasSignal = _latest_traffic_display_state.has_signal
  target.trafficDisplayedLightState = _latest_traffic_display_state.light_state
  target.trafficDisplayedPhase = _latest_traffic_display_state.phase
  target.trafficDisplayedControlActive = _latest_traffic_display_state.control_active


class TrafficControlRenderer(Widget):
  """Icon-only traffic signal driven by the already-published final plan."""

  def __init__(self) -> None:
    super().__init__()
    self.state = TrafficSignalDisplayState()

  def update(self) -> None:
    global _latest_traffic_display_frame, _latest_traffic_display_state
    sm = ui_state.sm
    if not sm.alive["longitudinalPlanSP"] or not sm.valid["longitudinalPlanSP"]:
      self.state = TrafficSignalDisplayState()
      _latest_traffic_display_state = self.state
      _latest_traffic_display_frame = gui_app.frame
      return
    # updated is a one-poll pulse, not an acknowledgement by this renderer.
    # A skipped HUD frame must not leave an older lamp/control outline cached
    # after a newer healthy plan has already arrived.
    self.state = TrafficSignalDisplayState.from_plan(
      sm["longitudinalPlanSP"].teslaTrafficControl,
      valid=bool(sm.valid["longitudinalPlanSP"]),
    )
    _latest_traffic_display_state = self.state
    _latest_traffic_display_frame = gui_app.frame

  def _render(self, rect: rl.Rectangle) -> None:
    if not self.state.visible:
      return

    icon = traffic_card_rect(rect)
    housing = rl.Rectangle(icon.x + 6, icon.y + 6, TRAFFIC_LIGHT_HOUSING_WIDTH, TRAFFIC_LIGHT_HOUSING_HEIGHT)
    highlighted = traffic_control_highlighted(self.state)
    if highlighted:
      rl.draw_rectangle_rounded(icon, 0.6, 12, rl.Color(CONTROL_OUTLINE.r, CONTROL_OUTLINE.g, CONTROL_OUTLINE.b, 38))
    rl.draw_rectangle_rounded(housing, 0.6, 12, rl.Color(12, 15, 19, 210))
    rl.draw_rectangle_rounded_lines_ex(housing, 0.6, 12, 2.5 if highlighted else 1.0,
                                       CONTROL_OUTLINE if highlighted else BORDER)

    for index, (color, light) in enumerate(zip((RED, AMBER, GREEN), (1, 3, 2), strict=True)):
      center = rl.Vector2(housing.x + housing.width / 2, housing.y + housing.height * (2 * index + 1) / 6)
      active = self.state.has_signal and self.state.light_state == light
      if self.state.has_signal and self.state.flashing and light == 2:
        active = int(gui_app.frame / max(1, gui_app.target_fps // 2)) % 2 == 0
      if active:
        rl.draw_circle_v(center, TRAFFIC_LIGHT_RADIUS + 3, rl.Color(color.r, color.g, color.b, 35))
      rl.draw_circle_v(center, TRAFFIC_LIGHT_RADIUS, color if active else LAMP_OFF)
