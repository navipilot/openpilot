import pyray as rl
from openpilot.selfdrive.ui.ui_state import UIStatus, UIState

CEM_OVERRIDE_STATUSES = {1, 2}
CEM_ACTIVE_STATUSES = {3, 4, 5, 6, 7, 8}

DISENGAGED_COLOR = rl.Color(18, 40, 57, 255)
AOL_COLOR = rl.Color(10, 186, 181, 255)
ENGAGED_COLOR = rl.Color(22, 127, 64, 255)
OVERRIDE_COLOR = rl.Color(137, 146, 141, 255)
EXPERIMENTAL_COLOR = rl.Color(218, 111, 37, 255)
CEM_OVERRIDE_COLOR = rl.Color(255, 214, 0, 255)
TRAFFIC_COLOR = rl.Color(201, 34, 49, 255)


def get_border_color(state: UIState):
  if state.status == UIStatus.OVERRIDE:
    return OVERRIDE_COLOR
  if state.traffic_mode_enabled and state.sm["selfdriveState"].enabled:
    return TRAFFIC_COLOR
  if state.always_on_lateral_active:
    return AOL_COLOR
  if state.conditional_status in CEM_OVERRIDE_STATUSES:
    return CEM_OVERRIDE_COLOR
  if state.sm["selfdriveState"].experimentalMode:
    return EXPERIMENTAL_COLOR
  if state.status == UIStatus.ENGAGED:
    return ENGAGED_COLOR
  return DISENGAGED_COLOR


def get_path_edge_color(state: UIState):
  if state.conditional_status in CEM_ACTIVE_STATUSES:
    return EXPERIMENTAL_COLOR
  return get_border_color(state)


def get_experimental_mode_banner_text(state: UIState):
  conditional_enabled = state.params.get_bool("ConditionalExperimental")

  # With CEM enabled, only surface banner text for explicit manual override states.
  # Automatic CEM transitions should only be reflected by path/border coloring.
  if conditional_enabled:
    if state.conditional_status in CEM_OVERRIDE_STATUSES:
      return "OVERRIDDEN"
    return None

  if state.sm["selfdriveState"].experimentalMode:
    return "EXPERIMENTAL"
  return "CHILL"
