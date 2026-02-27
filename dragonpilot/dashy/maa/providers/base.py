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

Abstract base classes for map providers.

These interfaces define the contract that all provider implementations must follow.
"""

from abc import ABC, abstractmethod
from typing import Optional
import asyncio

from .models import Coordinate, SearchResult, Route, TileConfig


class SearchProvider(ABC):
    """
    Abstract search/geocoding provider.

    Implementations: PhotonSearchProvider, MapboxSearchProvider, etc.
    """

    name: str = "base"
    requires_api_key: bool = False

    @abstractmethod
    async def search(
        self,
        query: str,
        proximity: Optional[Coordinate] = None,
        limit: int = 10
    ) -> list[SearchResult]:
        """
        Search for places by query string.

        Args:
            query: Search query (address, place name, etc.)
            proximity: Optional coordinate to bias results toward
            limit: Maximum number of results

        Returns:
            List of SearchResult objects sorted by relevance/distance
        """
        pass

    @abstractmethod
    async def reverse_geocode(
        self,
        coord: Coordinate
    ) -> Optional[SearchResult]:
        """
        Get address/place information from coordinates.

        Args:
            coord: Coordinate to reverse geocode

        Returns:
            SearchResult with address info, or None if not found
        """
        pass

    def search_sync(
        self,
        query: str,
        proximity: Optional[Coordinate] = None,
        limit: int = 10
    ) -> list[SearchResult]:
        """Synchronous wrapper for search()."""
        loop = asyncio.new_event_loop()
        try:
            return loop.run_until_complete(self.search(query, proximity, limit))
        finally:
            loop.close()


class RouteProvider(ABC):
    """
    Abstract routing provider.

    Implementations: OSRMRouteProvider, MapboxRouteProvider, etc.
    """

    name: str = "base"
    requires_api_key: bool = False
    supports_traffic: bool = False

    @abstractmethod
    async def get_route(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]] = None,
        bearing: Optional[float] = None
    ) -> Optional[Route]:
        """
        Calculate route between points.

        Args:
            origin: Starting coordinate
            destination: Ending coordinate
            waypoints: Optional intermediate waypoints
            bearing: Optional current heading in degrees (for better route start)

        Returns:
            Route object with steps and geometry, or None if routing fails
        """
        pass

    def get_route_sync(
        self,
        origin: Coordinate,
        destination: Coordinate,
        waypoints: Optional[list[Coordinate]] = None,
        bearing: Optional[float] = None
    ) -> Optional[Route]:
        """
        Synchronous wrapper for get_route().

        Use this in maad.py where async is not available.
        """
        loop = asyncio.new_event_loop()
        try:
            return loop.run_until_complete(
                self.get_route(origin, destination, waypoints, bearing)
            )
        finally:
            loop.close()


class TileProvider(ABC):
    """
    Abstract map tile provider.

    Implementations: OpenFreeMapTileProvider, MapboxTileProvider, etc.
    """

    name: str = "base"
    requires_api_key: bool = False

    @abstractmethod
    def get_tile_config(self) -> TileConfig:
        """
        Get tile URL template and configuration.

        Returns:
            TileConfig with URL template and attribution
        """
        pass

    @abstractmethod
    def get_style_json(self) -> dict:
        """
        Get MapLibre GL style JSON for this provider.

        Returns:
            Style JSON dict for MapLibre GL JS
        """
        pass
