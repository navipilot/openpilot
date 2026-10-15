"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

The spec record: the shapes the server answered with, and whether the engine
for the sha it names is built. One record, so the two cannot name different
models.
"""
import unittest

from jetlink.spec import ModelSpec
from tests.openpilot.fakes import OpenpilotTest


def spec(sha256: str) -> ModelSpec:
  return ModelSpec(sha256=sha256, nbytes=4096, frame_skip=4, input_shapes={'features_buffer': (1, 24, 512)},
                   output_shapes={'outputs': (1, 16)}, output_slices={'plan': slice(0, 16)}, checkpoint=None)


class TestSpecRecord(OpenpilotTest):
  def test_a_stored_spec_is_ready_for_its_own_model_only(self):
    self.parts.spec.store(spec('a' * 64))
    self.assertTrue(self.parts.spec.engine_ready_for('a' * 64))
    self.assertFalse(self.parts.spec.engine_ready_for('b' * 64))
    self.assertFalse(self.parts.spec.engine_ready_for(None))
    self.assertEqual(self.parts.spec.load().sha256, 'a' * 64)

  def test_nothing_recorded_is_not_ready(self):
    self.assertIsNone(self.parts.spec.load())
    self.assertFalse(self.parts.spec.engine_ready_for('a' * 64))

  def test_clearing_keeps_the_spec(self):
    # it still sizes the warp; only the engine has to be asked for again
    self.parts.spec.store(spec('a' * 64))
    self.parts.spec.clear_ready()
    self.assertFalse(self.parts.spec.engine_ready_for('a' * 64))
    self.assertEqual(self.parts.spec.load(), spec('a' * 64))
    self.parts.spec.store(spec('a' * 64))
    self.assertTrue(self.parts.spec.engine_ready_for('a' * 64))

  def test_it_is_the_param_the_fork_declares(self):
    self.parts.spec.store(spec('a' * 64))
    self.assertEqual(self.op.store['JetlinkSpec']['sha256'], 'a' * 64)
    self.assertIs(self.op.store['JetlinkSpec']['ready'], True)

  def test_an_unreadable_record_is_no_spec_and_says_so(self):
    self.op.store['JetlinkSpec'] = {'sha256': 'a' * 64, 'ready': True}   # no shapes
    self.assertIsNone(self.parts.spec.load())
    self.assertTrue(self.op.log.has('cached spec is unreadable', 'exception'))
    # readiness does not need the shapes
    self.assertTrue(self.parts.spec.engine_ready_for('a' * 64))

  def test_something_that_is_not_a_record_is_none(self):
    self.op.store['JetlinkSpec'] = 'junk'
    self.assertIsNone(self.parts.spec.load())
    self.assertFalse(self.parts.spec.engine_ready_for('a' * 64))


if __name__ == '__main__':
  unittest.main()
