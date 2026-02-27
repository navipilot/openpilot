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

Dragonpilot Routing Provider

Uses api.dragonpilot.org routing endpoint with device serial authentication.
Falls back to OSRM if not authenticated or on error.
"""

from typing import Optional

from openpilot.common.swaglog import cloudlog

from ..base import RouteProvider
from ..models import Coordinate, Route, Step
from .client import get_client


def decode_polyline(polyline: str) -> list[tuple[float, float]]:
    """
    Decode a Google/HERE Encoded Polyline into (lat, lon) tuples.

    Algorithm: https://developers.google.com/maps/documentation/utilities/polylinealgorithm
    """
    coordinates = []
    index = 0
    lat = 0
    lng = 0

    while index < len(polyline):
        # Decode latitude
        result = 0
        shift = 0
        while True:
            b = ord(polyline[index]) - 63
            index += 1
            result |= (b & 0x1f) << shift
            shift += 5
            if b < 0x20:
                break
        lat += (~(result >> 1) if result & 1 else result >> 1)

        # Decode longitude
        result = 0
        shift = 0
        while True:
            b = ord(polyline[index]) - 63
            index += 1
            result |= (b & 0x1f) << shift
            shift += 5
            if b < 0x20:
                break
        lng += (~(result >> 1) if result & 1 else result >> 1)

        coordinates.append((lat / 1e5, lng / 1e5))

    return coordinates


# Map action to OSRM-style maneuver type/modifier
ACTION_MAP = {
    'depart': ('depart', ''),
    'arrive': ('arrive', ''),
    'turn': ('turn', ''),
    'turn-left': ('turn', 'left'),
    'turn-right': ('turn', 'right'),
    'turn-slight-left': ('turn', 'slight left'),
    'turn-slight-right': ('turn', 'slight right'),
    'turn-sharp-left': ('turn', 'sharp left'),
    'turn-sharp-right': ('turn', 'sharp right'),
    'continue': ('continue', 'straight'),
    'keep': ('continue', ''),
    'merge': ('merge', ''),
    'roundabout': ('roundabout', ''),
    'roundaboutExit': ('roundabout', 'exit'),
    'ferry': ('ferry', ''),
    'uturn': ('turn', 'uturn'),
}


class DragonpilotRouteProvider(RouteProvider):
    """
    Dragonpilot API routing provider.

    Uses device serial for authentication. Falls back to OSRM on auth failure.

    API Response format:
    {
      "route": {
        "route_id": "...",
        "distance_m": 6837,
        "duration_s": 759,
        "duration_traffic_s": 1595,
        "polyline": "...",
        "maneuvers": [{
          "instruction": "Turn left onto Main Street",
          "distance_m": 500,
          "duration_s": 60,
          "position": {"lat": 25.03, "lon": 121.56},
          "action": "turn"
        }],
        "provider": "here"
      },
      "cached": false,
      "provider": "here"
    }
    """

    name = "dragonpilot"
    requires_api_key = False
    supports_traffic = True

    def __init__(self):
        self._client = get_client()
        self._fallback = None

    def _get_fallback(self) -> RouteProvider:
        """Get fallback OSRM provider."""
        if self._fallback is None:
            from ..routing.osrm import OSRMRouteProvider
            self._fallback = OSRMRouteProvider()
        return self._fallback

    async def get_route(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]] = None,
        bearing: Optional[float] = None
    ) -> Optional[Route]:
        """Calculate route using Dragonpilot API (async)."""
        if not self._client.is_authenticated:
            cloudlog.warning("dragonpilot routing: no serial, using osrm fallback")
            return await self._get_fallback().get_route(origin, destination, waypoints, bearing)

        data = self._build_request(origin, destination, waypoints)
        response = await self._client.post('/v1/route', data=data, timeout=30)

        if response is None:
            cloudlog.warning("dragonpilot routing: API error, using osrm fallback")
            return await self._get_fallback().get_route(origin, destination, waypoints, bearing)

        return self._parse_response(response)

    def get_route_sync(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]] = None,
        bearing: Optional[float] = None
    ) -> Optional[Route]:
        """Calculate route using Dragonpilot API (synchronous)."""
        if not self._client.is_authenticated:
            cloudlog.warning("dragonpilot routing: no serial, using osrm fallback")
            return self._get_fallback().get_route_sync(origin, destination, waypoints, bearing)

        data = self._build_request(origin, destination, waypoints)
        response = self._client.post_sync('/v1/route', data=data, timeout=30)

        if response is None:
            cloudlog.warning("dragonpilot routing: API error, using osrm fallback")
            return self._get_fallback().get_route_sync(origin, destination, waypoints, bearing)

        return self._parse_response(response)

    def _build_request(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]]
    ) -> dict:
        """Build API request body."""
        origin = origin.to_wgs84() if hasattr(origin, 'to_wgs84') else origin
        destination = destination.to_wgs84() if hasattr(destination, 'to_wgs84') else destination

        data = {
            'origin': {'lat': origin.latitude, 'lon': origin.longitude},
            'destination': {'lat': destination.latitude, 'lon': destination.longitude},
        }

        if waypoints:
            data['waypoints'] = []
            for wp in waypoints:
                wp = wp.to_wgs84() if hasattr(wp, 'to_wgs84') else wp
                data['waypoints'].append({'lat': wp.latitude, 'lon': wp.longitude})

        return data

    def _parse_response(self, data: dict) -> Optional[Route]:
        """Parse API response into Route object."""
        if not data:
            return None

        # Unwrap route object
        route_data = data.get('route', data)

        # Decode full route geometry
        full_geometry = []
        polyline = route_data.get('polyline', '')
        if polyline:
            try:
                for lat, lon in decode_polyline(polyline):
                    coord = Coordinate(lat, lon)
                    full_geometry.append(coord)
            except Exception:
                pass

        # Parse maneuvers into steps
        steps = []
        maneuvers = route_data.get('maneuvers', [])

        for maneuver in maneuvers:
            pos = maneuver.get('position', {})
            action = maneuver.get('action', 'continue')

            # Map action to maneuver type/modifier
            maneuver_type, maneuver_modifier = ACTION_MAP.get(action, ('continue', ''))

            # Get maneuver point
            maneuver_point = None
            if pos.get('lat') is not None and pos.get('lon') is not None:
                maneuver_point = Coordinate(pos['lat'], pos['lon'])

            # Use full instruction text as the step name
            instruction = maneuver.get('instruction', '')

            steps.append(Step(
                distance=maneuver.get('distance_m', 0),
                duration=maneuver.get('duration_s', 0),
                name=instruction,
                maneuver_type=maneuver_type,
                maneuver_modifier=maneuver_modifier,
                geometry=[maneuver_point] if maneuver_point else [],
                speed_limit=None,
                speed_limit_sign='vienna',
                maneuver_point=maneuver_point,
            ))

        # Use traffic duration if available
        duration = route_data.get('duration_traffic_s') or route_data.get('duration_s', 0)

        return Route(
            steps=steps,
            distance=route_data.get('distance_m', 0),
            duration=duration,
            geometry=full_geometry,
            provider=self.name,
            has_traffic=route_data.get('duration_traffic_s') is not None,
            raw=data,
        )
