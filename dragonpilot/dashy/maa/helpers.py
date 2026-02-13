"""
Copyright (c) 2026, Rick Lan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sublicense,
for non-commercial purposes only, subject to the following conditions:

- The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.
- Commercial use (e.g. use in a product, service, or activity intended to
  generate revenue) is prohibited without explicit written permission from
  the copyright holder.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Navigation helpers for dragonpilot MAA.

Coordinate math, route parsing, and curvature computation utilities.
"""
from __future__ import annotations

import json
import math
from typing import Any, cast

from opendbc.car.common.conversions import Conversions
from openpilot.common.params import Params

DIRECTIONS = ('left', 'right', 'straight')
MODIFIABLE_DIRECTIONS = ('left', 'right')
EARTH_MEAN_RADIUS = 6371007.2

# Speed unit conversions to m/s
SPEED_CONVERSIONS = {
  'km/h': Conversions.KPH_TO_MS,
  'mph': Conversions.MPH_TO_MS,
}


class Coordinate:
  def __init__(self, latitude: float, longitude: float) -> None:
    self.latitude = latitude
    self.longitude = longitude
    self.annotations: dict[str, float] = {}

  @classmethod
  def from_mapbox_tuple(cls, t: tuple[float, float]) -> Coordinate:
    return cls(t[1], t[0])

  @classmethod
  def from_osrm_tuple(cls, t: list[float]) -> Coordinate:
    """OSRM uses [lon, lat] order."""
    return cls(t[1], t[0])

  def as_dict(self) -> dict[str, float]:
    return {'latitude': self.latitude, 'longitude': self.longitude}

  def __str__(self) -> str:
    return f'Coordinate({self.latitude}, {self.longitude})'

  def __repr__(self) -> str:
    return self.__str__()

  def __eq__(self, other) -> bool:
    if not isinstance(other, Coordinate):
      return False
    return self.latitude == other.latitude and self.longitude == other.longitude

  def __hash__(self) -> int:
    return hash((self.latitude, self.longitude))

  def __sub__(self, other: Coordinate) -> Coordinate:
    return Coordinate(self.latitude - other.latitude, self.longitude - other.longitude)

  def __add__(self, other: Coordinate) -> Coordinate:
    return Coordinate(self.latitude + other.latitude, self.longitude + other.longitude)

  def __mul__(self, c: float) -> Coordinate:
    return Coordinate(self.latitude * c, self.longitude * c)

  def dot(self, other: Coordinate) -> float:
    return self.latitude * other.latitude + self.longitude * other.longitude

  def distance_to(self, other: Coordinate) -> float:
    """Haversine distance in meters."""
    dlat = math.radians(other.latitude - self.latitude)
    dlon = math.radians(other.longitude - self.longitude)

    haversine_dlat = math.sin(dlat / 2.0)
    haversine_dlat *= haversine_dlat
    haversine_dlon = math.sin(dlon / 2.0)
    haversine_dlon *= haversine_dlon

    y = haversine_dlat \
        + math.cos(math.radians(self.latitude)) \
        * math.cos(math.radians(other.latitude)) \
        * haversine_dlon
    x = 2 * math.asin(math.sqrt(y))
    return x * EARTH_MEAN_RADIUS

  def bearing_to(self, other: Coordinate) -> float:
    """Bearing to other coordinate in degrees (0-360)."""
    lat1, lat2 = math.radians(self.latitude), math.radians(other.latitude)
    dlon = math.radians(other.longitude - self.longitude)
    x = math.sin(dlon) * math.cos(lat2)
    y = math.cos(lat1) * math.sin(lat2) - math.sin(lat1) * math.cos(lat2) * math.cos(dlon)
    bearing = math.degrees(math.atan2(x, y))
    return (bearing + 360) % 360


def minimum_distance(a: Coordinate, b: Coordinate, p: Coordinate) -> float:
  """Minimum distance from point p to line segment ab."""
  if a.distance_to(b) < 0.01:
    return a.distance_to(p)

  ap = p - a
  ab = b - a
  t = max(0.0, min(1.0, ap.dot(ab) / ab.dot(ab)))
  projection = a + ab * t
  return projection.distance_to(p)


def project_onto_segment(a: Coordinate, b: Coordinate, p: Coordinate) -> tuple[Coordinate, float, float]:
  """Project point p onto line segment ab (snap to road).

  Returns:
    (projected_point, t, distance) where:
    - projected_point: the closest point on segment ab to p
    - t: parameter 0-1 indicating position along segment (0=a, 1=b)
    - distance: distance from p to the projected point
  """
  seg_len = a.distance_to(b)
  if seg_len < 0.01:
    return a, 0.0, a.distance_to(p)

  ap = p - a
  ab = b - a
  t = max(0.0, min(1.0, ap.dot(ab) / ab.dot(ab)))
  projection = a + ab * t
  return projection, t, projection.distance_to(p)


def distance_along_geometry(geometry: list[Coordinate], pos: Coordinate) -> float:
  """Calculate distance traveled along geometry from start to closest point to pos."""
  if len(geometry) <= 2:
    return geometry[0].distance_to(pos)

  total_distance = 0.0
  total_distance_closest = 0.0
  closest_distance = 1e9

  for i in range(len(geometry) - 1):
    d = minimum_distance(geometry[i], geometry[i + 1], pos)

    if d < closest_distance:
      closest_distance = d
      total_distance_closest = total_distance + geometry[i].distance_to(pos)

    total_distance += geometry[i].distance_to(geometry[i + 1])

  return total_distance_closest


def normalize_angle(angle: float) -> float:
  """Normalize angle to -180 to 180 degrees."""
  while angle > 180:
    angle -= 360
  while angle < -180:
    angle += 360
  return angle


def calculate_turn_angle(
  current_geometry: list[Coordinate],
  next_geometry: list[Coordinate],
  samples: int = 3
) -> float:
  """
  Calculate turn angle between two road segments.

  Uses the bearing of the end of current geometry vs the bearing
  of the start of next geometry to determine turn angle.

  Args:
    current_geometry: Coordinates of current road segment (before turn)
    next_geometry: Coordinates of next road segment (after turn)
    samples: Number of points to use for bearing calculation (for stability)

  Returns:
    Turn angle in degrees. Positive = left turn, negative = right turn.
    Example: 90 = 90° left turn, -90 = 90° right turn
  """
  if len(current_geometry) < 2 or len(next_geometry) < 2:
    return 0.0

  # Get bearing of current road (last segment)
  # Use last few points for stability
  start_idx = max(0, len(current_geometry) - samples)
  current_bearing = current_geometry[start_idx].bearing_to(current_geometry[-1])

  # Get bearing of next road (first segment)
  # Use first few points for stability
  end_idx = min(samples, len(next_geometry) - 1)
  next_bearing = next_geometry[0].bearing_to(next_geometry[end_idx])

  # Calculate turn angle (positive = left, negative = right)
  # This matches openpilot convention where left is positive curvature
  angle = normalize_angle(current_bearing - next_bearing)

  return angle


def coordinate_from_param(key: str, params: Params = None) -> Coordinate | None:
  """Read coordinate from params.

  Handles both JSON type params (returns dict) and STRING type params (returns string).
  """
  if params is None:
    params = Params()

  try:
    value = params.get(key)
    if value is None:
      return None

    # JSON type params return dict directly, STRING type needs json.loads()
    if isinstance(value, str):
      pos = json.loads(value)
    else:
      pos = value

    if 'latitude' not in pos or 'longitude' not in pos:
      return None

    return Coordinate(pos['latitude'], pos['longitude'])
  except (json.JSONDecodeError, KeyError, TypeError):
    return None


def find_closest_point_on_route(pos: Coordinate, route_coords: list[Coordinate]) -> tuple[int, float]:
  """Find closest point index and distance on route."""
  if not route_coords:
    return 0, float('inf')

  min_dist = float('inf')
  closest_idx = 0

  for i in range(len(route_coords) - 1):
    # Check distance to segment, not just point
    d = minimum_distance(route_coords[i], route_coords[i + 1], pos)
    if d < min_dist:
      min_dist = d
      closest_idx = i

  return closest_idx, min_dist


def calculate_remaining_distance(route_coords: list[Coordinate], from_idx: int) -> float:
  """Calculate remaining distance along route from index."""
  if from_idx >= len(route_coords) - 1:
    return 0.0

  total = 0.0
  for i in range(from_idx, len(route_coords) - 1):
    total += route_coords[i].distance_to(route_coords[i + 1])
  return total


# --- Instruction Parsing ---

def string_to_direction(direction: str) -> str:
  """Convert direction string to standard format."""
  for d in DIRECTIONS:
    if d in direction:
      if 'slight' in direction and d in MODIFIABLE_DIRECTIONS:
        return 'slight' + d.capitalize()
      return d
  return 'none'


def maxspeed_to_ms(maxspeed: dict[str, str | float]) -> float:
  """Convert speed limit dict to m/s."""
  unit = cast(str, maxspeed['unit'])
  speed = cast(float, maxspeed['speed'])
  return SPEED_CONVERSIONS.get(unit, 1.0) * speed


def field_valid(dat: dict, field: str) -> bool:
  """Check if field exists and is not None."""
  return field in dat and dat[field] is not None


def parse_banner_instructions(banners: Any, distance_to_maneuver: float = 0.0) -> dict[str, Any] | None:
  """Parse Mapbox/OSRM banner instructions."""
  if not banners or not len(banners):
    return None

  instruction = {}

  # A segment can contain multiple banners, find one that we need to show now
  current_banner = banners[0]
  for banner in banners:
    if distance_to_maneuver < banner.get('distanceAlongGeometry', 0):
      current_banner = banner

  # Only show banner when close enough to maneuver
  instruction['showFull'] = distance_to_maneuver < current_banner.get('distanceAlongGeometry', 0)

  # Primary
  p = current_banner.get('primary', {})
  if field_valid(p, 'text'):
    instruction['maneuverPrimaryText'] = p['text']
  if field_valid(p, 'type'):
    instruction['maneuverType'] = p['type']
  if field_valid(p, 'modifier'):
    instruction['maneuverModifier'] = p['modifier']

  # Secondary
  if field_valid(current_banner, 'secondary'):
    instruction['maneuverSecondaryText'] = current_banner['secondary']['text']

  # Lane lines
  if field_valid(current_banner, 'sub'):
    lanes = []
    for component in current_banner['sub'].get('components', []):
      if component.get('type') != 'lane':
        continue

      lane = {
        'active': component.get('active', False),
        'directions': [string_to_direction(d) for d in component.get('directions', [])],
      }

      if field_valid(component, 'active_direction'):
        lane['activeDirection'] = string_to_direction(component['active_direction'])

      lanes.append(lane)
    instruction['lanes'] = lanes

  return instruction


def parse_osrm_step(step: dict) -> dict[str, Any]:
  """Parse OSRM route step into instruction format."""
  maneuver = step.get('maneuver', {})
  instruction = {
    'distance': step.get('distance', 0),
    'duration': step.get('duration', 0),
    'name': step.get('name', ''),
    'maneuverType': maneuver.get('type', ''),
    'maneuverModifier': maneuver.get('modifier', ''),
    'location': maneuver.get('location', []),  # [lon, lat]
  }
  return instruction


def classify_maneuver(maneuver_type: str, maneuver_modifier: str) -> str:
  """
  Classify OSRM maneuver as 'turn' or 'laneChange'.

  Highway exits/forks use laneChange desires (smoother).
  Intersection turns use turn desires (sharper).

  OSRM maneuver types:
  - turn: regular intersection turn
  - fork: highway split/junction
  - off ramp: highway exit
  - on ramp: highway entrance
  - merge: merging lanes
  - roundabout turn: roundabout exit
  - exit roundabout: leaving roundabout
  - continue: straight (no maneuver)
  - depart/arrive: start/end

  Returns:
    'turn' for intersection turns
    'laneChange' for highway exits/forks
    'none' for straight/no maneuver
  """
  maneuver_type = maneuver_type.lower()
  maneuver_modifier = maneuver_modifier.lower()

  # Highway exits and forks -> lane change desire
  LANE_CHANGE_TYPES = {
    'fork',           # highway fork/split
    'off ramp',       # highway exit
    'on ramp',        # highway entrance
    'merge',          # merging
    'exit rotary',    # leaving rotary
    'exit roundabout',# leaving roundabout
  }

  # Intersection turns -> turn desire
  TURN_TYPES = {
    'turn',           # regular turn
    'end of road',    # forced turn at end of road
    'rotary',         # entering rotary
    'roundabout',     # entering roundabout
    'roundabout turn',# turn within roundabout
  }

  # No maneuver
  NO_MANEUVER_TYPES = {
    'continue',
    'depart',
    'arrive',
    'new name',
    'notification',
  }

  if maneuver_type in LANE_CHANGE_TYPES:
    return 'laneChange'

  if maneuver_type in TURN_TYPES:
    # For turns, check modifier - slight turns at highway speeds might be lane changes
    if 'slight' in maneuver_modifier:
      # Slight turns could be either - default to turn but could be lane change
      # CarrotPilot uses additional context like road speed limit
      return 'turn'
    return 'turn'

  if maneuver_type in NO_MANEUVER_TYPES:
    return 'none'

  # Unknown type - default to turn
  return 'turn'


def get_turn_direction(maneuver_modifier: str) -> str:
  """
  Get turn direction from OSRM maneuver modifier.

  Returns:
    'left', 'right', or 'none'
  """
  modifier = maneuver_modifier.lower()
  if 'left' in modifier:
    return 'left'
  if 'right' in modifier:
    return 'right'
  return 'none'


# --- Curvature Computation ---

def compute_path_curvature(pos: Coordinate, bearing: float, route_coords: list[Coordinate],
                           closest_idx: int, v_ego: float, lookahead_time: float = 2.5) -> float:
  """
  Compute desired curvature from route geometry using pure pursuit.

  Args:
    pos: Current position
    bearing: Current heading in degrees
    route_coords: List of route coordinates
    closest_idx: Index of closest point on route
    v_ego: Current vehicle speed m/s
    lookahead_time: How far ahead to look in seconds

  Returns:
    Desired curvature in 1/m (positive = left turn, negative = right turn)
  """
  if not route_coords or closest_idx >= len(route_coords) - 1:
    return 0.0

  # Calculate lookahead distance (min 30m, based on speed)
  lookahead_dist = max(v_ego * lookahead_time, 30.0)

  # Find lookahead point along route
  dist_traveled = 0.0
  lookahead_idx = closest_idx

  for i in range(closest_idx, len(route_coords) - 1):
    segment_dist = route_coords[i].distance_to(route_coords[i + 1])
    if dist_traveled + segment_dist >= lookahead_dist:
      # Interpolate within segment
      remaining = lookahead_dist - dist_traveled
      ratio = remaining / segment_dist if segment_dist > 0 else 0
      lookahead_idx = i
      # Could interpolate here, but using next point is simpler
      if ratio > 0.5:
        lookahead_idx = i + 1
      break
    dist_traveled += segment_dist
    lookahead_idx = i + 1

  if lookahead_idx >= len(route_coords):
    lookahead_idx = len(route_coords) - 1

  lookahead_point = route_coords[lookahead_idx]

  # Calculate desired heading to lookahead point
  desired_bearing = pos.bearing_to(lookahead_point)

  # Calculate heading error (normalized to -180 to 180)
  heading_error = desired_bearing - bearing
  if heading_error > 180:
    heading_error -= 360
  elif heading_error < -180:
    heading_error += 360

  # Convert heading error to yaw (radians)
  yaw_error = math.radians(heading_error)

  # Distance to lookahead point
  dist_to_lookahead = pos.distance_to(lookahead_point)
  if dist_to_lookahead < 1.0:
    return 0.0

  # Pure pursuit curvature: 2 * sin(yaw_error) / lookahead_distance
  # Note: Negative because heading_error > 0 means target is to the RIGHT (clockwise),
  # and right turn should produce negative curvature
  curvature = -2.0 * math.sin(yaw_error) / dist_to_lookahead

  # Clamp to reasonable values (max ~7m radius turn)
  MAX_CURVATURE = 0.15
  return max(-MAX_CURVATURE, min(MAX_CURVATURE, curvature))


def smooth_curvature(new_curv: float, prev_curv: float, alpha: float = 0.3) -> float:
  """Exponential smoothing for curvature."""
  return alpha * new_curv + (1 - alpha) * prev_curv


def curvature_to_radius(curvature: float) -> float:
  """Convert curvature to turn radius in meters."""
  if abs(curvature) < 0.001:
    return float('inf')
  return 1.0 / abs(curvature)


def compute_turn_angle_at_index(route_coords: list[Coordinate], turn_idx: int,
                                 sample_dist: float = 20.0) -> float:
  """
  Compute turn angle at a specific point on the route.
  Uses points sample_dist meters before and after for stability.
  Returns angle in degrees (positive = left, negative = right).
  """
  if turn_idx < 1 or turn_idx >= len(route_coords) - 1:
    return 0.0

  # Find points ~sample_dist before and after the turn for stable bearing
  before_idx = turn_idx
  after_idx = turn_idx

  # Walk backwards to find before point
  dist = 0.0
  for i in range(turn_idx, 0, -1):
    dist += route_coords[i].distance_to(route_coords[i - 1])
    if dist >= sample_dist:
      before_idx = i - 1
      break
  else:
    before_idx = 0

  # Walk forwards to find after point
  dist = 0.0
  for i in range(turn_idx, len(route_coords) - 1):
    dist += route_coords[i].distance_to(route_coords[i + 1])
    if dist >= sample_dist:
      after_idx = i + 1
      break
  else:
    after_idx = len(route_coords) - 1

  if before_idx == turn_idx or after_idx == turn_idx:
    return 0.0

  # Calculate bearings using the sampled points
  p1 = route_coords[before_idx]
  p2 = route_coords[turn_idx]
  p3 = route_coords[after_idx]

  bearing1 = p1.bearing_to(p2)
  bearing2 = p2.bearing_to(p3)

  # Angle difference (positive = left, negative = right)
  angle = bearing1 - bearing2

  # Normalize to -180 to 180
  while angle > 180:
    angle -= 360
  while angle < -180:
    angle += 360

  return angle


def compute_turn_curvature_at_index(route_coords: list[Coordinate], turn_idx: int,
                                     sample_dist: float = 15.0) -> float:
  """
  Compute curvature at turn point using three-point circle fitting.
  Returns curvature in 1/m (positive = left, negative = right).
  """
  if turn_idx < 1 or turn_idx >= len(route_coords) - 1:
    return 0.0

  # Find points sample_dist before and after
  before_idx = turn_idx
  after_idx = turn_idx

  dist = 0.0
  for i in range(turn_idx, 0, -1):
    dist += route_coords[i].distance_to(route_coords[i - 1])
    if dist >= sample_dist:
      before_idx = i - 1
      break

  dist = 0.0
  for i in range(turn_idx, len(route_coords) - 1):
    dist += route_coords[i].distance_to(route_coords[i + 1])
    if dist >= sample_dist:
      after_idx = i + 1
      break

  if before_idx == turn_idx or after_idx == turn_idx:
    return 0.0

  # Three points for circle fitting
  p1 = route_coords[before_idx]
  p2 = route_coords[turn_idx]
  p3 = route_coords[after_idx]

  # Convert to local meters (approximate)
  lat_center = p2.latitude
  lon_scale = math.cos(math.radians(lat_center))
  m_per_deg = 111319.5  # meters per degree latitude

  x1 = (p1.longitude - p2.longitude) * lon_scale * m_per_deg
  y1 = (p1.latitude - p2.latitude) * m_per_deg
  x2 = 0.0
  y2 = 0.0
  x3 = (p3.longitude - p2.longitude) * lon_scale * m_per_deg
  y3 = (p3.latitude - p2.latitude) * m_per_deg

  # Calculate curvature using cross product / (product of distances)
  d12 = math.sqrt((x2 - x1)**2 + (y2 - y1)**2)
  d13 = math.sqrt((x3 - x1)**2 + (y3 - y1)**2)
  d23 = math.sqrt((x3 - x2)**2 + (y3 - y2)**2)

  if d12 < 0.1 or d13 < 0.1 or d23 < 0.1:
    return 0.0

  # Cross product (p2-p1) x (p3-p1) - z component only
  cross = (x2 - x1) * (y3 - y1) - (y2 - y1) * (x3 - x1)

  curvature = 2.0 * cross / (d12 * d13 * d23)

  # Clamp to reasonable values
  MAX_CURVATURE = 0.2  # ~5m radius
  return max(-MAX_CURVATURE, min(MAX_CURVATURE, curvature))
