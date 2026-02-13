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

Photon Search Provider.

Uses the free Photon (Komoot) geocoding API for address search.
No API key required, no rate limits.

Photon API: https://photon.komoot.io/
"""

from typing import Optional
import aiohttp
import requests

from ..base import SearchProvider
from ..models import Coordinate, SearchResult

PHOTON_URL = 'https://photon.komoot.io/api'


class PhotonSearchProvider(SearchProvider):
    """
    Free Photon geocoding provider.

    No API key required. Fast, reliable, uses OpenStreetMap data.
    """

    name = "photon"
    requires_api_key = False

    def __init__(self):
        """Initialize Photon provider."""
        pass

    async def search(
        self,
        query: str,
        proximity: Optional[Coordinate] = None,
        limit: int = 10
    ) -> list[SearchResult]:
        """Search for places using Photon API (async)."""
        if not query or len(query) < 2:
            return []

        params = {'q': query, 'limit': min(limit + 5, 15)}  # Request extra for filtering

        if proximity:
            prox = proximity.to_wgs84() if hasattr(proximity, 'to_wgs84') else proximity
            params['lat'] = prox.latitude
            params['lon'] = prox.longitude

        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(PHOTON_URL, params=params, timeout=aiohttp.ClientTimeout(total=10)) as resp:
                    if resp.status != 200:
                        return []
                    data = await resp.json()
        except Exception:
            return []

        return self._parse_results(data, proximity, limit)

    def search_sync(
        self,
        query: str,
        proximity: Optional[Coordinate] = None,
        limit: int = 10
    ) -> list[SearchResult]:
        """Search for places using Photon API (synchronous)."""
        if not query or len(query) < 2:
            return []

        params = {'q': query, 'limit': min(limit + 5, 15)}

        if proximity:
            prox = proximity.to_wgs84() if hasattr(proximity, 'to_wgs84') else proximity
            params['lat'] = prox.latitude
            params['lon'] = prox.longitude

        try:
            resp = requests.get(PHOTON_URL, params=params, timeout=10)
            if resp.status_code != 200:
                return []
            data = resp.json()
        except Exception:
            return []

        return self._parse_results(data, proximity, limit)

    async def reverse_geocode(
        self,
        coord: Coordinate
    ) -> Optional[SearchResult]:
        """Get address from coordinates using Photon reverse API."""
        wgs_coord = coord.to_wgs84() if hasattr(coord, 'to_wgs84') else coord
        url = f"{PHOTON_URL}/reverse"
        params = {'lat': wgs_coord.latitude, 'lon': wgs_coord.longitude}

        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(url, params=params, timeout=aiohttp.ClientTimeout(total=10)) as resp:
                    if resp.status != 200:
                        return None
                    data = await resp.json()
        except Exception:
            return None

        features = data.get('features', [])
        if not features:
            return None

        feature = features[0]
        props = feature.get('properties', {})
        parts = [props.get('name'), props.get('street'), props.get('city')]
        address = ', '.join(filter(None, parts))

        return SearchResult(
            name=address.split(',')[0] if address else 'Unknown',
            address=address or 'Unknown location',
            coordinate=coord,
            provider=self.name,
            raw=feature,
        )

    def _parse_results(
        self,
        data: dict,
        proximity: Optional[Coordinate],
        limit: int
    ) -> list[SearchResult]:
        """Parse Photon GeoJSON response into SearchResult list."""
        results = []

        for feature in data.get('features', []):
            try:
                coords = feature['geometry']['coordinates']
                props = feature.get('properties', {})

                # Create coordinate (Photon uses [lon, lat] GeoJSON order)
                coord = Coordinate(coords[1], coords[0])

                # Build address from properties
                parts = [
                    props.get('name'),
                    props.get('street'),
                    props.get('city'),
                    props.get('state'),
                    props.get('country')
                ]
                address = ', '.join(filter(None, parts))

                # Calculate distance if proximity provided
                distance = None
                if proximity:
                    prox = proximity.to_wgs84() if hasattr(proximity, 'to_wgs84') else proximity
                    search_coord = Coordinate(coords[1], coords[0])  # Use WGS84 for distance
                    distance = search_coord.distance_to(prox)

                # Determine display name
                name = props.get('name') or props.get('street') or (address.split(',')[0] if address else 'Unknown')

                results.append(SearchResult(
                    name=name,
                    address=address or 'Unknown location',
                    coordinate=coord,
                    distance=distance,
                    place_id=props.get('osm_id'),
                    provider=self.name,
                    raw=feature,
                ))
            except (KeyError, IndexError):
                continue

        # Sort by distance if proximity provided
        if proximity:
            results.sort(key=lambda r: r.distance if r.distance is not None else float('inf'))

        return results[:limit]
