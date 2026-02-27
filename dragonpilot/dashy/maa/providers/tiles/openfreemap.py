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

OpenFreeMap Tile Provider.

Uses free OpenFreeMap tiles. No API key required.

OpenFreeMap: https://openfreemap.org/
"""

from ..base import TileProvider
from ..models import TileConfig
from ..factory import ProviderFactory
from ..config import TileProviderType


@ProviderFactory.register_tile(TileProviderType.OPENFREEMAP)
class OpenFreeMapTileProvider(TileProvider):
    """
    Free OpenFreeMap tile provider.

    No API key required. Uses OpenStreetMap data with nice styling.
    """

    name = "openfreemap"
    requires_api_key = False

    def __init__(self, api_key: str = None):
        """Initialize OpenFreeMap provider."""
        pass  # No API key needed

    def get_tile_config(self) -> TileConfig:
        """Get tile configuration."""
        return TileConfig(
            url_template="https://tiles.openfreemap.org/planet/{z}/{x}/{y}.pbf",
            style_url="https://tiles.openfreemap.org/styles/liberty",
            attribution='<a href="https://openfreemap.org">OpenFreeMap</a> | <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>',
            min_zoom=0,
            max_zoom=20,
        )

    def get_style_json(self) -> dict:
        """
        Get MapLibre GL style JSON.

        This style is optimized for navigation display with good contrast
        and road visibility.
        """
        return {
            "version": 8,
            "name": "OpenFreeMap Liberty",
            "sources": {
                "openmaptiles": {
                    "type": "vector",
                    "url": "https://tiles.openfreemap.org/planet"
                }
            },
            "glyphs": "https://tiles.openfreemap.org/fonts/{fontstack}/{range}.pbf",
            "sprite": "https://tiles.openfreemap.org/styles/liberty/sprite",
            "layers": [
                # Simplified layer config - the actual style is loaded from style_url
                # This is a fallback/reference
                {
                    "id": "background",
                    "type": "background",
                    "paint": {"background-color": "#f8f4f0"}
                }
            ]
        }
