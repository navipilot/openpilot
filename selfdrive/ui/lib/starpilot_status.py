import pyray as rl

from openpilot.selfdrive.ui.ui_state import UIStatus, UIState

CEM_DISABLED_OVERRIDE_STATUSES = {1}
CEM_MANUAL_OVERRIDE_STATUSES = {1, 2}
CEM_ACTIVE_STATUSES = {3, 4, 5, 6, 7, 8}

DISENGAGED_COLOR = rl.Color(18, 40, 57, 255)
AOL_COLOR = rl.Color(10, 186, 181, 255)
ENGAGED_COLOR = rl.Color(22, 127, 64, 255)
OVERRIDE_COLOR = rl.Color(137, 146, 141, 255)
EXPERIMENTAL_COLOR = rl.Color(218, 111, 37, 255)
CEM_OVERRIDE_COLOR = rl.Color(255, 214, 0, 255)
SWITCHBACK_COLOR = rl.Color(139, 108, 197, 255)
TRAFFIC_COLOR = rl.Color(201, 34, 49, 255)


def get_border_color(state: UIState):
  enabled = state.sm["selfdriveState"].enabled
  lateral_active = enabled or state.always_on_lateral_active
  if state.status == UIStatus.OVERRIDE:
    return OVERRIDE_COLOR
  if state.switchback_mode_enabled and lateral_active:
    return SWITCHBACK_COLOR
  if state.traffic_mode_enabled and enabled:
    return TRAFFIC_COLOR
  if state.always_on_lateral_active:
    return AOL_COLOR
  # Only color the border for CEM/experimental while actually enabled.
  if enabled and state.conditional_status in CEM_DISABLED_OVERRIDE_STATUSES:
    return CEM_OVERRIDE_COLOR
  if enabled and state.sm["selfdriveState"].experimentalMode:
    return EXPERIMENTAL_COLOR
  if state.status == UIStatus.ENGAGED:
    return ENGAGED_COLOR
  return DISENGAGED_COLOR


def get_path_edge_color(state: UIState):
  if state.conditional_status in CEM_ACTIVE_STATUSES:
    return EXPERIMENTAL_COLOR
  return get_border_color(state)


def get_screen_edge_color(state: UIState):
  enabled = state.sm["selfdriveState"].enabled
  lateral_active = enabled or state.always_on_lateral_active
  if state.status == UIStatus.OVERRIDE:
    return OVERRIDE_COLOR
  if state.switchback_mode_enabled and lateral_active:
    return SWITCHBACK_COLOR
  if state.always_on_lateral_active:
    return AOL_COLOR
  if state.conditional_status in CEM_DISABLED_OVERRIDE_STATUSES:
    return CEM_OVERRIDE_COLOR
  if state.sm["selfdriveState"].experimentalMode:
    return EXPERIMENTAL_COLOR
  if state.traffic_mode_enabled and enabled:
    return TRAFFIC_COLOR
  return get_border_color(state)


def get_experimental_mode_banner_text(state: UIState):
  conditional_enabled = state.params.get_bool("ConditionalExperimental")

  # With CEM enabled, only surface banner text for explicit manual override states.
  # Automatic CEM transitions should only be reflected by path/border coloring.
  if conditional_enabled:
    if state.conditional_status in CEM_MANUAL_OVERRIDE_STATUSES:
      return "OVERRIDDEN"
    return None

  if state.sm["selfdriveState"].experimentalMode:
    return "EXPERIMENTAL"
  return "CHILL"
