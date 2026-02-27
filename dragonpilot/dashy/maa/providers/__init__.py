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

Map Provider Abstraction Layer

Uses api.dragonpilot.org for search and routing with device JWT authentication.
Falls back to free providers (Photon/OSRM) when not authenticated.

Usage:
    from dragonpilot.dashy.maa.providers import MapService

    map_service = MapService()
    route = map_service.route_provider.get_route_sync(origin, dest)
    results = await map_service.search_provider.search("Taipei 101")
"""

from .map_service import MapService
from .models import Coordinate, SearchResult, Route, Step, TileConfig
from .base import SearchProvider, RouteProvider, TileProvider
from .dragonpilot import DragonpilotSearchProvider, DragonpilotRouteProvider, DragonpilotApiClient

__all__ = [
    'MapService',
    'Coordinate',
    'SearchResult',
    'Route',
    'Step',
    'TileConfig',
    'SearchProvider',
    'RouteProvider',
    'TileProvider',
    'DragonpilotSearchProvider',
    'DragonpilotRouteProvider',
    'DragonpilotApiClient',
]
