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

Provider factory for creating map service instances.

Uses api.dragonpilot.org for search and routing.
"""

from .config import ProviderConfig
from .base import SearchProvider, RouteProvider, TileProvider


class ProviderFactory:
    """
    Factory for creating map providers.

    Uses Dragonpilot API providers which have built-in fallback
    to free providers (Photon/OSRM) when not authenticated.
    """

    @classmethod
    def create_search_provider(cls, config: ProviderConfig) -> SearchProvider:
        """Create search provider."""
        from .dragonpilot.search import DragonpilotSearchProvider
        return DragonpilotSearchProvider()

    @classmethod
    def create_route_provider(cls, config: ProviderConfig) -> RouteProvider:
        """Create route provider."""
        from .dragonpilot.routing import DragonpilotRouteProvider
        return DragonpilotRouteProvider()

    @classmethod
    def create_tile_provider(cls, config: ProviderConfig) -> TileProvider:
        """Create tile provider."""
        from .tiles.openfreemap import OpenFreeMapTileProvider
        return OpenFreeMapTileProvider()
