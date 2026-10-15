"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

conftest.py's guard sees a test reach the machine's own files, even through
code that swallows the OSError.
"""
import sys
import tempfile
import unittest
from pathlib import Path

from jetlink.comma import gadget
from jetlink.openpilot.settings import FileParams
from tests.openpilot import fakes


def guard():
  return next(m for name, m in sys.modules.items() if name.endswith('openpilot.conftest') and hasattr(m, '_guard'))


class TestTheGuard(unittest.TestCase):
  def reached(self, fn) -> list[str]:
    touched = guard()._touched
    start = len(touched)
    fn()
    found = touched[start:]
    del touched[start:]   # this test's own reach is the point, not a failure
    return found

  def test_a_read_of_the_live_store_is_caught_though_it_is_swallowed(self):
    found = self.reached(lambda: FileParams(Path('/data/params/d')).raw('JetlinkLink'))
    self.assertEqual(found, ['open /data/params/d/JetlinkLink'])

  def test_the_comma_layers_records_are_caught(self):
    found = self.reached(gadget.dormant)
    self.assertEqual(found, [f'open {gadget.DORMANT}'])

  def test_an_isolated_test_reaches_nothing(self):
    fakes.isolate(self, Path(tempfile.mkdtemp()))
    self.assertEqual(self.reached(lambda: (gadget.dormant(), gadget.host_attached(), gadget.link_kind(),
                                           gadget.gadget_error(), gadget.port_has_host())), [])
