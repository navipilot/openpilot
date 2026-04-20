from __future__ import annotations

import math

from openpilot.common.params import Params


def get_border_width(base_width: int, params: Params | None = None) -> int:
  active_params = params if params is not None else Params()

  scale = active_params.get_float("BorderWidth", return_default=True, default=100.0)
  if not math.isfinite(scale):
    scale = 100.0
  scale = min(250.0, max(25.0, scale))

  return max(1, int(round(base_width * scale / 100.0)))


def lead_indicator_enabled(params: Params | None = None) -> bool:
  active_params = params if params is not None else Params()

  enabled = not active_params.get_bool("HideLeadMarker", default=False)
  if active_params.get("LeadIndicator") is not None:
    enabled = enabled and active_params.get_bool("LeadIndicator", default=True)

  return enabled
