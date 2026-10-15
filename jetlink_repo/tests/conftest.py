"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.
"""
import pytest

from tests.aio_fakes import FakeAio


@pytest.fixture(autouse=True)
def fake_aio(request, monkeypatch):
  """Gadget writes go through FakeAio: only Linux has the real one, and only a
  comma has the endpoint it was made for. tests/test_aio.py, marked real_aio,
  runs the real one on Linux."""
  if request.node.get_closest_marker('real_aio'):
    return
  from jetlink.transport import ffs
  monkeypatch.setattr(ffs, '_open_aio', FakeAio)
