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

Dragonpilot Search Provider

Uses api.dragonpilot.org geocoding endpoint for address search.
Falls back to Photon if not authenticated or on error.
"""

from typing import Optional

from openpilot.common.swaglog import cloudlog

from ..base import SearchProvider
from ..models import Coordinate, SearchResult
from .client import get_client


class DragonpilotSearchProvider(SearchProvider):
    """
    Dragonpilot API search provider.

    Uses device serial for authentication. Falls back to Photon on auth failure.

    API Response format:
    {
      "results": [{
        "id": "here:...",
        "title": "Taipei 101",
        "address": "No. 7, Section 5, Xinyi Road",
        "position": {"lat": 25.033, "lon": 121.565},
        "category": "landmark",
        "distance_m": 150
      }],
      "provider": "here"
    }
    """

    name = "dragonpilot"
    requires_api_key = False

    def __init__(self):
        self._client = get_client()
        self._fallback = None

    def _get_fallback(self) -> SearchProvider:
        """Get fallback Photon provider."""
        if self._fallback is None:
            from ..search.photon import PhotonSearchProvider
            self._fallback = PhotonSearchProvider()
        return self._fallback

    async def search(
        self,
        query: str,
        proximity: Optional[Coordinate] = None,
        limit: int = 10
    ) -> list[SearchResult]:
        """Search for places using Dragonpilot API (async)."""
        if not query or len(query) < 1:
            return []

        # Fall back to Photon if not authenticated
        if not self._client.is_authenticated:
            cloudlog.debug("dragonpilot search: no serial, using photon fallback")
            return await self._get_fallback().search(query, proximity, limit)

        params = {'q': query, 'limit': min(limit, 10)}

        if proximity:
            prox = proximity.to_wgs84() if hasattr(proximity, 'to_wgs84') else proximity
            params['lat'] = prox.latitude
            params['lon'] = prox.longitude

        data = await self._client.get('/v1/geocode/autocomplete', params=params)

        if data is None:
            cloudlog.debug("dragonpilot search: API error, using photon fallback")
            return await self._get_fallback().search(query, proximity, limit)

        return self._parse_results(data, proximity, limit)

    def search_sync(
        self,
        query: str,
        proximity: Optional[Coordinate] = None,
        limit: int = 10
    ) -> list[SearchResult]:
        """Search for places using Dragonpilot API (synchronous)."""
        if not query or len(query) < 1:
            return []

        # Fall back to Photon if not authenticated
        if not self._client.is_authenticated:
            cloudlog.debug("dragonpilot search: no serial, using photon fallback")
            return self._get_fallback().search_sync(query, proximity, limit)

        params = {'q': query, 'limit': min(limit, 10)}

        if proximity:
            prox = proximity.to_wgs84() if hasattr(proximity, 'to_wgs84') else proximity
            params['lat'] = prox.latitude
            params['lon'] = prox.longitude

        data = self._client.get_sync('/v1/geocode/autocomplete', params=params)

        if data is None:
            cloudlog.debug("dragonpilot search: API error, using photon fallback")
            return self._get_fallback().search_sync(query, proximity, limit)

        return self._parse_results(data, proximity, limit)

    async def reverse_geocode(self, coord: Coordinate) -> Optional[SearchResult]:
        """Get address from coordinates. Falls back to Photon."""
        return await self._get_fallback().reverse_geocode(coord)

    def _parse_results(
        self,
        data: dict,
        proximity: Optional[Coordinate],
        limit: int
    ) -> list[SearchResult]:
        """Parse API response into SearchResult list."""
        results = []

        # Handle both GeoJSON format (features) and simple format (results)
        items = data.get('features', data.get('results', []))

        for item in items:
            try:
                # GeoJSON format
                if 'geometry' in item:
                    coords = item['geometry']['coordinates']
                    lon, lat = coords[0], coords[1]  # GeoJSON is [lon, lat]
                    props = item.get('properties', {})
                    title = props.get('title', 'Unknown')
                    address = props.get('address', '')
                    place_id = props.get('id')
                    distance = props.get('distance_m')
                # Simple format
                else:
                    pos = item.get('position', {})
                    lat = pos.get('lat')
                    lon = pos.get('lon')
                    title = item.get('title', 'Unknown')
                    address = item.get('address', '')
                    place_id = item.get('id')
                    distance = item.get('distance_m')

                if lat is None or lon is None:
                    continue

                coord = Coordinate(lat, lon)

                # Calculate distance if not provided
                if distance is None and proximity:
                    prox = proximity.to_wgs84() if hasattr(proximity, 'to_wgs84') else proximity
                    distance = Coordinate(lat, lon).distance_to(prox)

                results.append(SearchResult(
                    name=title,
                    address=address,
                    coordinate=coord,
                    distance=distance,
                    place_id=place_id,
                    provider=self.name,
                    raw=item,
                ))
            except (KeyError, TypeError, IndexError):
                continue

        return results[:limit]
