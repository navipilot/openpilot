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

OSRM Route Provider.

Uses the free OSRM (Open Source Routing Machine) API for routing.
No API key required.

OSRM API Docs: http://project-osrm.org/docs/v5.24.0/api/
"""

from typing import Optional
import aiohttp
import requests

from ..base import RouteProvider
from ..models import Coordinate, Route, Step

OSRM_URL = 'https://router.project-osrm.org'


class OSRMRouteProvider(RouteProvider):
    """
    Free OSRM routing provider.

    No API key required. Uses public OSRM demo server.
    Does not support traffic data.
    """

    name = "osrm"
    requires_api_key = False
    supports_traffic = False

    def __init__(self):
        """Initialize OSRM provider."""
        pass

    async def get_route(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]] = None,
        bearing: Optional[float] = None
    ) -> Optional[Route]:
        """Calculate route using OSRM API (async)."""
        url, params = self._build_request(origin, destination, waypoints, bearing)

        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(url, params=params, timeout=aiohttp.ClientTimeout(total=10)) as resp:
                    if resp.status != 200:
                        return None
                    data = await resp.json()
        except Exception:
            return None

        return self._parse_response(data)

    def get_route_sync(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]] = None,
        bearing: Optional[float] = None
    ) -> Optional[Route]:
        """Calculate route using OSRM API (synchronous for maad.py)."""
        url, params = self._build_request(origin, destination, waypoints, bearing)

        try:
            resp = requests.get(url, params=params, timeout=10)
            if resp.status_code != 200:
                return None
            data = resp.json()
            if data.get('code') != 'Ok':
                return None
        except Exception:
            return None

        return self._parse_response(data)

    def _build_request(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]],
        bearing: Optional[float]
    ) -> tuple[str, dict]:
        """Build OSRM API request URL and params."""
        # Convert to WGS-84 if needed (OSRM uses WGS-84)
        origin = origin.to_wgs84() if hasattr(origin, 'to_wgs84') and origin.system.value != 'wgs84' else origin
        destination = destination.to_wgs84() if hasattr(destination, 'to_wgs84') and destination.system.value != 'wgs84' else destination

        # Build coordinate string: lon,lat;lon,lat;...
        all_coords = [(origin.longitude, origin.latitude)]
        if waypoints:
            for wp in waypoints:
                wp = wp.to_wgs84() if hasattr(wp, 'to_wgs84') and wp.system.value != 'wgs84' else wp
                all_coords.append((wp.longitude, wp.latitude))
        all_coords.append((destination.longitude, destination.latitude))

        # Limit coordinate precision to 6 decimal places (about 0.1m accuracy)
        coords_str = ';'.join([f'{lon:.6f},{lat:.6f}' for lon, lat in all_coords])
        url = f"{OSRM_URL}/route/v1/driving/{coords_str}"

        params = {
            'overview': 'full',
            'geometries': 'geojson',
            'steps': 'true',
        }

        # Add bearing if provided (helps with route direction at start)
        # OSRM bearings format: bearing,range for each coord, separated by ;
        # Note: Disabled for now due to URL encoding issues with semicolons
        # TODO: Re-enable once we properly handle the bearings parameter
        # if bearing is not None:
        #     bearing_parts = [f"{int(bearing) % 360},90"] + [''] * (len(all_coords) - 1)
        #     url += f"?bearings={';'.join(bearing_parts)}"
        #     return url, params

        return url, params

    def _parse_response(self, data: dict) -> Optional[Route]:
        """Parse OSRM API response into Route object."""
        if data.get('code') != 'Ok' or not data.get('routes'):
            return None

        route_data = data['routes'][0]
        steps = []
        full_geometry = []

        for leg in route_data.get('legs', []):
            for step in leg.get('steps', []):
                maneuver = step.get('maneuver', {})

                # Parse geometry coordinates
                geometry = []
                for coord in step.get('geometry', {}).get('coordinates', []):
                    # OSRM uses [lon, lat] order
                    c = Coordinate(coord[1], coord[0])
                    geometry.append(c)
                    full_geometry.append(c)

                step_name = step.get('name', '')

                # Extract explicit maneuver location from OSRM
                maneuver_location = maneuver.get('location')  # [lon, lat]
                maneuver_point = None
                if maneuver_location and len(maneuver_location) == 2:
                    maneuver_point = Coordinate(maneuver_location[1], maneuver_location[0])

                steps.append(Step(
                    distance=step.get('distance', 0),
                    duration=step.get('duration', 0),
                    name=step_name,
                    maneuver_type=maneuver.get('type', ''),
                    maneuver_modifier=maneuver.get('modifier', ''),
                    geometry=geometry,
                    speed_limit=None,
                    speed_limit_sign='vienna',
                    maneuver_point=maneuver_point,
                ))

        return Route(
            steps=steps,
            distance=route_data.get('distance', 0),
            duration=route_data.get('duration', 0),
            geometry=full_geometry,
            provider=self.name,
            has_traffic=False,
            raw=route_data,
        )
