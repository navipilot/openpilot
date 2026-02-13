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

Data models for map providers.

These models provide a common interface for search results, routes, and coordinates
across different providers (OSRM, Mapbox, Google, AMap).
"""

from __future__ import annotations
import math
from dataclasses import dataclass, field
from typing import Optional
from enum import Enum


EARTH_MEAN_RADIUS = 6371007.2  # meters


class CoordinateSystem(Enum):
    """Coordinate reference system."""
    WGS84 = "wgs84"    # Standard GPS coordinates (used globally)
    GCJ02 = "gcj02"    # China encrypted coordinates (required for China maps)


@dataclass
class Coordinate:
    """
    Geographic coordinate with optional coordinate system awareness.

    Supports WGS-84 (standard GPS) and GCJ-02 (China) coordinate systems.
    Provides distance and bearing calculations.
    """
    latitude: float
    longitude: float
    system: CoordinateSystem = CoordinateSystem.WGS84
    annotations: dict = field(default_factory=dict)

    def distance_to(self, other: Coordinate) -> float:
        """Calculate Haversine distance to another coordinate in meters."""
        dlat = math.radians(other.latitude - self.latitude)
        dlon = math.radians(other.longitude - self.longitude)

        haversine_dlat = math.sin(dlat / 2.0)
        haversine_dlat *= haversine_dlat
        haversine_dlon = math.sin(dlon / 2.0)
        haversine_dlon *= haversine_dlon

        y = haversine_dlat + \
            math.cos(math.radians(self.latitude)) * \
            math.cos(math.radians(other.latitude)) * \
            haversine_dlon
        x = 2 * math.asin(math.sqrt(y))
        return x * EARTH_MEAN_RADIUS

    def bearing_to(self, other: Coordinate) -> float:
        """Calculate bearing to another coordinate in degrees (0-360)."""
        lat1, lat2 = math.radians(self.latitude), math.radians(other.latitude)
        dlon = math.radians(other.longitude - self.longitude)
        x = math.sin(dlon) * math.cos(lat2)
        y = math.cos(lat1) * math.sin(lat2) - math.sin(lat1) * math.cos(lat2) * math.cos(dlon)
        bearing = math.degrees(math.atan2(x, y))
        return (bearing + 360) % 360

    def to_wgs84(self) -> Coordinate:
        """Convert to WGS-84 coordinate system."""
        if self.system == CoordinateSystem.WGS84:
            return self
        from .utils.coordinates import gcj02_to_wgs84
        lat, lon = gcj02_to_wgs84(self.latitude, self.longitude)
        return Coordinate(lat, lon, CoordinateSystem.WGS84, self.annotations.copy())

    def to_gcj02(self) -> Coordinate:
        """Convert to GCJ-02 coordinate system (for China maps)."""
        if self.system == CoordinateSystem.GCJ02:
            return self
        from .utils.coordinates import wgs84_to_gcj02
        lat, lon = wgs84_to_gcj02(self.latitude, self.longitude)
        return Coordinate(lat, lon, CoordinateSystem.GCJ02, self.annotations.copy())

    def as_dict(self) -> dict:
        """Convert to dictionary."""
        return {'latitude': self.latitude, 'longitude': self.longitude}

    def __str__(self) -> str:
        return f'Coordinate({self.latitude:.6f}, {self.longitude:.6f})'

    def __repr__(self) -> str:
        return self.__str__()

    def __eq__(self, other) -> bool:
        if not isinstance(other, Coordinate):
            return False
        return self.latitude == other.latitude and self.longitude == other.longitude

    def __hash__(self) -> int:
        return hash((self.latitude, self.longitude))


@dataclass
class SearchResult:
    """
    Normalized search/geocoding result.

    All search providers return results in this format.
    """
    name: str                              # Display name (e.g., "Taipei 101")
    address: str                           # Full address string
    coordinate: Coordinate                 # Location
    distance: Optional[float] = None       # Distance from search proximity (meters)
    place_id: Optional[str] = None         # Provider-specific place ID
    provider: str = ""                     # Provider name (e.g., "photon", "mapbox")
    raw: dict = field(default_factory=dict)  # Raw provider response for debugging


@dataclass
class Step:
    """
    Single navigation step/maneuver in a route.

    Represents one turn or road segment with its geometry.
    """
    distance: float                        # Step distance in meters
    duration: float                        # Step duration in seconds
    duration_typical: Optional[float] = None  # Typical duration (with traffic)
    name: str = ""                         # Road/street name
    maneuver_type: str = ""                # Type: turn, fork, off ramp, merge, etc.
    maneuver_modifier: str = ""            # Direction: left, right, slight left, etc.
    geometry: list[Coordinate] = field(default_factory=list)  # Path coordinates
    speed_limit: Optional[float] = None    # Speed limit in m/s
    speed_limit_sign: str = "vienna"       # Sign type: vienna or mutcd
    maneuver_point: Optional[Coordinate] = None  # Explicit maneuver location from OSRM


@dataclass
class Route:
    """
    Complete navigation route.

    Contains all steps from origin to destination with total distance/duration.
    """
    steps: list[Step]                      # List of navigation steps
    distance: float                        # Total distance in meters
    duration: float                        # Total duration in seconds
    duration_typical: Optional[float] = None  # Typical duration (with traffic)
    geometry: list[Coordinate] = field(default_factory=list)  # Full route polyline
    provider: str = ""                     # Provider name
    has_traffic: bool = False              # Whether duration includes traffic
    raw: dict = field(default_factory=dict)  # Raw provider response


@dataclass
class TileConfig:
    """
    Map tile configuration for frontend display.
    """
    url_template: str                      # URL template with {z}/{x}/{y}
    style_url: Optional[str] = None        # MapLibre style URL
    attribution: str = ""                  # Map attribution text
    min_zoom: int = 0
    max_zoom: int = 22
