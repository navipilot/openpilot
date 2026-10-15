"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

A graph that keeps its own history (openpilot #38916, Cinque Terre V3 on), as
the comma sees it: the spec read off the graph, and the parity bench's
reference, which loops the graph's state itself. The server's loop is checked
over the link in tests/test_swift_server.py.
"""
from __future__ import annotations

import json
from pathlib import Path
from types import SimpleNamespace

import numpy as np
import pytest

pytest.importorskip('onnx')

from jetlink.spec import ModelSpec, spec_from_onnx
from tests import load_script, tiny_model

IMAGES = tiny_model.STATEFUL_IMAGES


@pytest.fixture(scope='module')
def model_path(tmp_path_factory):
  return tiny_model.write_stateful(tmp_path_factory.mktemp('stateful') / 'tiny_stateful.onnx')


@pytest.fixture(scope='module')
def spec(model_path):
  return spec_from_onnx(str(model_path))


class TestSpec:
  def test_the_layout_is_read_off_the_graph(self, spec):
    assert spec.stateful
    assert spec.state_pairs == tiny_model.STATE_PAIRS
    assert spec.warped_shape == (2, 6, 8, 16)
    assert spec.model_hw == (8, 16)

  def test_the_wire_carries_the_scalars_and_no_hidden_state(self, spec):
    assert list(spec.packed_shapes) == ['desire', 'traffic_convention', 'action_t']
    assert spec.packed_nelem == 8 + 2 + 2
    assert spec.output_nelem == tiny_model.N_OUT

  def test_the_wire_form_round_trips(self, spec):
    again = ModelSpec.from_dict(spec.to_dict())
    assert again.stateful and again.state_pairs == spec.state_pairs
    assert again.infer_req_nbytes == spec.infer_req_nbytes

  def test_a_queued_graph_is_unchanged(self, tmp_path):
    queued = spec_from_onnx(str(tiny_model.write(tmp_path / 'tiny.onnx')))
    assert not queued.stateful and queued.state_pairs == {}
    # prev_feat stays on the server (protocol 3), so the same three as stateful
    assert list(queued.packed_shapes) == ['desire', 'traffic_convention', 'action_t']


def test_the_parity_reference_loops_the_state_itself(spec, tmp_path):
  """verify_parity's reference on a stateful graph, with a stand-in session
  running the tiny graph so onnxruntime is not needed here."""
  vp = load_script(Path(__file__).resolve().parents[1] / 'scripts' / 'verify_parity.py')

  class Session:
    def get_inputs(self):
      return [SimpleNamespace(name=n, shape=list(s), type='tensor(uint8)' if n in IMAGES else 'tensor(float)')
              for n, s in tiny_model.STATEFUL_SHAPES.items()]

    def get_outputs(self):
      return [SimpleNamespace(name=n) for n in ('outputs', *tiny_model.STATE_PAIRS.values())]

    def run(self, _, feed):
      state = {n: feed[n] for n in tiny_model.STATE_PAIRS}
      out, nxt = tiny_model.stateful_step(state, feed['new_img'], feed['desire'],
                                          feed['traffic_convention'], feed['action_t'])
      return [out.reshape(1, -1), *(nxt[n] for n in tiny_model.STATE_PAIRS)]

  # what a capture writes: the frames sent, and what the link returned
  frames = tiny_model.stateful_frames(6, seed=9)
  want = tiny_model.stateful_reference(frames)
  (tmp_path / 'spec.json').write_text(json.dumps(spec.to_dict()))
  for i, (f, out) in enumerate(zip(frames, want, strict=True)):
    np.save(tmp_path / f'in_warped_{i}.npy', f['new_img'])
    np.save(tmp_path / f'in_packed_{i}.npy', tiny_model.packed_for(f))
    np.save(tmp_path / f'out_link_{i}.npy', out)

  assert vp.reference_stateful(spec, Session(), tmp_path, len(frames)) == 0
  for i, w in enumerate(want):
    np.testing.assert_allclose(np.load(tmp_path / f'out_ref_{i}.npy'), w, rtol=1e-6, atol=1e-6)
