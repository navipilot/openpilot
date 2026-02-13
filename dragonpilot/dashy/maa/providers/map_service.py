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

Map Service - Main entry point for all map operations.

Usage:
    from dragonpilot.dashy.maa.providers import MapService

    # In maad.py (sync):
    map_service = MapService()
    route = map_service.route_provider.get_route_sync(origin, dest)

    # In server.py (async):
    results = await MapService.get_instance().search_provider.search("Taipei 101")

    # Get tile config for frontend:
    tile_config = map_service.get_tile_config_for_js()
"""

from typing import Optional

from .config import ProviderConfig
from .factory import ProviderFactory
from .base import SearchProvider, RouteProvider, TileProvider


class MapService:
    """
    Main entry point for all map operations.

    Provides lazy-loaded access to search, routing, and tile providers.
    Configuration is read from openpilot Params.
    """

    _instance: Optional['MapService'] = None

    def __init__(self, params=None):
        """
        Initialize MapService.

        Args:
            params: Optional openpilot Params instance.
                    If None, will be created when needed.
        """
        self._params = params
        self._config: Optional[ProviderConfig] = None
        self._search_provider: Optional[SearchProvider] = None
        self._route_provider: Optional[RouteProvider] = None
        self._tile_provider: Optional[TileProvider] = None

    @classmethod
    def get_instance(cls, params=None) -> 'MapService':
        """
        Get singleton instance of MapService.

        Args:
            params: Optional Params instance for first-time initialization
        """
        if cls._instance is None:
            cls._instance = cls(params)
        return cls._instance

    @classmethod
    def reset_instance(cls):
        """Reset singleton instance. Useful for testing or config reload."""
        cls._instance = None

    def _get_params(self):
        """Get or create Params instance."""
        if self._params is None:
            from openpilot.common.params import Params
            self._params = Params()
        return self._params

    def reload_config(self):
        """
        Reload configuration from params and recreate providers.

        Call this after changing provider settings in params.
        """
        self._config = ProviderConfig.from_params(self._get_params())
        self._search_provider = None
        self._route_provider = None
        self._tile_provider = None

    @property
    def config(self) -> ProviderConfig:
        """Get current configuration, loading from params if needed."""
        if self._config is None:
            self._config = ProviderConfig.from_params(self._get_params())
        return self._config

    @property
    def search_provider(self) -> SearchProvider:
        """Get search provider, creating if needed."""
        if self._search_provider is None:
            # Import providers to trigger registration
            self._ensure_providers_imported()
            self._search_provider = ProviderFactory.create_search_provider(self.config)
        return self._search_provider

    @property
    def route_provider(self) -> RouteProvider:
        """Get route provider, creating if needed."""
        if self._route_provider is None:
            # Import providers to trigger registration
            self._ensure_providers_imported()
            self._route_provider = ProviderFactory.create_route_provider(self.config)
        return self._route_provider

    @property
    def tile_provider(self) -> TileProvider:
        """Get tile provider, creating if needed."""
        if self._tile_provider is None:
            # Import providers to trigger registration
            self._ensure_providers_imported()
            self._tile_provider = ProviderFactory.create_tile_provider(self.config)
        return self._tile_provider

    def _ensure_providers_imported(self):
        """Import provider modules to trigger registration."""
        try:
            from . import search
            from . import routing
            from . import tiles
        except ImportError:
            pass

    def get_tile_config_for_js(self) -> dict:
        """
        Get tile configuration as dict for JavaScript frontend.

        Returns dict suitable for sending to frontend via API.
        """
        tile_config = self.tile_provider.get_tile_config()
        style = self.tile_provider.get_style_json()
        return {
            'provider': self.config.tile_provider.value,
            'url_template': tile_config.url_template,
            'style': style,
            'attribution': tile_config.attribution,
            'min_zoom': tile_config.min_zoom,
            'max_zoom': tile_config.max_zoom,
        }

    def get_provider_info(self) -> dict:
        """
        Get information about current providers.

        Useful for debugging and UI display.
        """
        return {
            'search': {
                'provider': self.config.search_provider.value,
                'name': self.search_provider.name,
                'requires_api_key': self.search_provider.requires_api_key,
            },
            'route': {
                'provider': self.config.route_provider.value,
                'name': self.route_provider.name,
                'requires_api_key': self.route_provider.requires_api_key,
                'supports_traffic': self.route_provider.supports_traffic,
            },
            'tile': {
                'provider': self.config.tile_provider.value,
                'name': self.tile_provider.name,
                'requires_api_key': self.tile_provider.requires_api_key,
            },
        }
