from __future__ import annotations

from collections.abc import Mapping


def _normalize_limit(limit: float) -> float:
  return limit if limit >= 1.0 else 0.0


def resolve_display_speed_limit_ms(
  slc_speed_limit: float,
  speed_limit_source: str,
  source_limits: Mapping[str, float],
  primary_priority: str,
  secondary_priority: str,
) -> float:
  slc_speed_limit = _normalize_limit(slc_speed_limit)
  if slc_speed_limit > 0.0:
    return slc_speed_limit

  normalized_limits = {source: _normalize_limit(limit) for source, limit in source_limits.items()}

  active_source_limit = normalized_limits.get(speed_limit_source, 0.0)
  if active_source_limit > 0.0:
    return active_source_limit

  available_limits = {source: limit for source, limit in normalized_limits.items() if limit > 0.0}
  if not available_limits:
    return 0.0

  if primary_priority == "Highest":
    return max(available_limits.values())
  if primary_priority == "Lowest":
    return min(available_limits.values())

  for priority in (primary_priority, secondary_priority, "Mapbox"):
    limit = available_limits.get(priority, 0.0)
    if limit > 0.0:
      return limit

  return next(iter(available_limits.values()))
