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

Dragonpilot API Client

Simple HTTP client using device serial for authentication.
"""

import os
from typing import Optional
import aiohttp
import requests

from openpilot.system.hardware import HARDWARE

API_HOST = os.getenv('DRAGONPILOT_API_HOST', 'https://api.dragonpilot.org')

# Module-level serial cache - queried once from HARDWARE
_serial: Optional[str] = None


def _get_serial() -> Optional[str]:
    """Get device serial (cached)."""
    global _serial
    if _serial is None:
        try:
            _serial = HARDWARE.get_serial()
        except Exception:
            pass
    return _serial


class DragonpilotApiClient:
    """
    API client for api.dragonpilot.org.

    Uses device serial from HARDWARE for authentication.
    """

    def __init__(self, serial: str = None):
        self.api_host = API_HOST
        self.serial = serial if serial is not None else _get_serial()

    @property
    def is_authenticated(self) -> bool:
        return self.serial is not None

    def _headers(self) -> dict:
        headers = {'Content-Type': 'application/json'}
        if self.serial:
            headers['X-Device-Serial'] = self.serial
        return headers

    def get_sync(self, endpoint: str, params: dict = None, timeout: int = 10) -> Optional[dict]:
        try:
            resp = requests.get(f"{self.api_host}{endpoint}", params=params, headers=self._headers(), timeout=timeout)
            return resp.json() if resp.status_code == 200 else None
        except Exception:
            return None

    def post_sync(self, endpoint: str, data: dict = None, timeout: int = 10) -> Optional[dict]:
        try:
            resp = requests.post(f"{self.api_host}{endpoint}", json=data, headers=self._headers(), timeout=timeout)
            return resp.json() if resp.status_code == 200 else None
        except Exception:
            return None

    async def get(self, endpoint: str, params: dict = None, timeout: int = 10) -> Optional[dict]:
        try:
            async with aiohttp.ClientSession() as session:
                async with session.get(
                    f"{self.api_host}{endpoint}",
                    params=params,
                    headers=self._headers(),
                    timeout=aiohttp.ClientTimeout(total=timeout)
                ) as resp:
                    return await resp.json() if resp.status == 200 else None
        except Exception:
            return None

    async def post(self, endpoint: str, data: dict = None, timeout: int = 10) -> Optional[dict]:
        try:
            async with aiohttp.ClientSession() as session:
                async with session.post(
                    f"{self.api_host}{endpoint}",
                    json=data,
                    headers=self._headers(),
                    timeout=aiohttp.ClientTimeout(total=timeout)
                ) as resp:
                    return await resp.json() if resp.status == 200 else None
        except Exception:
            return None


# Singleton client instance
_client: Optional[DragonpilotApiClient] = None


def get_client() -> DragonpilotApiClient:
    """Get the shared API client instance."""
    global _client
    if _client is None:
        _client = DragonpilotApiClient()
    return _client
