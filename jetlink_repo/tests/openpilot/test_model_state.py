"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

What the comma puts on the wire for each kind of big model, and what modeld
reads off the model.

Both kinds get the frame and the scalars only. A stateful graph (openpilot
#38916, Cinque Terre V3 on) keeps its hidden state inside itself, and the
Jetson feeds a queued one's (up to Cinque Terre V2) back (jetlink protocol 3).
The warp and the link are faked, and tinygrad is numpy; the packing is the
code that drives.
"""
from __future__ import annotations

import contextlib
import unittest
from types import SimpleNamespace
from unittest import mock

import numpy as np

from jetlink.openpilot import model_state
from jetlink.spec import ModelSpec
from tests.openpilot import fakes

# Cinque Terre V3's output layout, read off its ONNX
SLICES = {'lane_lines': (0, 528), 'lane_lines_prob': (528, 536), 'road_edges': (536, 800), 'meta': (800, 855),
          'desire_pred': (855, 887), 'pose': (887, 899), 'wide_from_device_euler': (899, 905),
          'road_transform': (905, 917), 'plan': (917, 1907), 'lead': (1907, 2051), 'lead_prob': (2051, 2054),
          'desire_state': (2054, 2062), 'action': (2062, 2066), 'hidden_state': (2066, 18450), 'pad': (18450, 18452)}
STATEFUL = {
  'new_img': (2, 6, 128, 256), 'desire': (8,), 'traffic_convention': (1, 2), 'action_t': (1, 2),
  'state_img_q': (2, 5, 6, 128, 256), 'state_desire_q': (132, 1, 8), 'state_feat_q': (128, 1, 16384),
}
QUEUED = {
  'img': (1, 12, 128, 256), 'big_img': (1, 12, 128, 256), 'desire_pulse': (1, 25, 8),
  'traffic_convention': (1, 2), 'action_t': (1, 2), 'features_buffer': (1, 32, 32, 512),
}


def spec_for(inputs: dict) -> ModelSpec:
  outputs = {'outputs': (1, 18452)}
  outputs.update({f'next_{n}': s for n, s in inputs.items() if n.startswith('state_')})
  return ModelSpec(sha256='a' * 64, nbytes=1, frame_skip=4, input_shapes=inputs, output_shapes=outputs,
                   output_slices={k: slice(*v) for k, v in SLICES.items()}, checkpoint=None)


class FakeClient:
  def __init__(self):
    self.nonce = 'test-session'
    self.sent = []
    self.asked = []
    self.last_timings = (0, 0, 0)
    self.last_state = None
    self.dead = False
    self.t = SimpleNamespace()
    self.output = np.zeros(18452, np.float32)
    self.output[slice(*SLICES['hidden_state'])] = 0.5
    self.last_output = None
    self._in_flight = []
    # frames (by seq) whose reply is not back within the hold, and the holds
    # end() asked for
    self.late = set()
    self.holds = []
    self.drains = 0
    # frames (by frame_id) a drain leaves unanswered: a reply not back yet
    self.keep: set[int] = set()
    # what a reply asked for telemetry carries, when the server has any
    self.telemetry = None
    # frames (by frame_id) the link refuses when asked to skip if busy, and
    # whether each frame asked
    self.busy: set[int] = set()
    self.skippable = []

  def infer_begin(self, data, packed, frame_id, reset=False, want_state=False, skip_if_busy=False):
    self.skippable.append(skip_if_busy)
    if skip_if_busy and frame_id in self.busy:
      return None
    self.sent.append((np.frombuffer(bytes(data), np.uint8).copy(), np.array(packed, copy=True), frame_id, reset))
    self.asked.append(want_state)
    self._in_flight.append(frame_id)
    return frame_id

  def infer_end(self, seq, hold=None):
    self.holds.append(hold)
    if hold is not None and (seq in self.late or seq in self.keep):
      return None
    self._in_flight.remove(seq)
    self.last_output = self.output
    self._piggyback()
    return self.output

  def drain(self):
    # replies come in order: none past the first that is not back yet
    self.drains += 1
    back = next((i for i, f in enumerate(self._in_flight) if f in self.keep), len(self._in_flight))
    taken, self._in_flight = self._in_flight[:back], self._in_flight[back:]
    if taken:
      self.last_output = self.output
      self._piggyback()
    return len(taken)

  def _piggyback(self):
    if self.telemetry is not None and self.asked[-1]:
      self.last_state = self.telemetry

  @property
  def unanswered(self):
    return len(self._in_flight)


class FakeWarp:
  """warp.Warp as the model state uses it. `output` holds the warped frame
  only once wait() has returned: the GPU writes it in between."""

  def __init__(self, warped):
    self.warped = warped
    self.output = memoryview(bytearray(warped.nbytes))
    self.started = []

  def start(self, frame, big_frame, tfm, big_tfm):
    self.started.append((frame, big_frame, np.array(tfm), np.array(big_tfm)))
    self.output[:] = b'\xee' * self.output.nbytes

  def wait(self):
    self.output[:] = self.warped.tobytes()


class ModelStateTest(unittest.TestCase):
  def setUp(self):
    self.log = fakes.RecordingLog()
    self.events = []

  def make(self, spec, client, face=fakes.FACE, warped=None):
    warped = np.zeros(np.prod(spec.warped_shape), np.uint8) if warped is None else warped
    return model_state.JetlinkModelState(client, spec, FakeWarp(warped), face=face, log=self.log,
                                         event=lambda name, **fields: self.events.append((name, fields)))

  def run_frames(self, inputs: dict, n: int = 3, client=None, after_enqueue=None):
    """`n` frames driven. Returns the spec, the state, the client and the
    warped frame every frame carried."""
    spec = spec_for(inputs)
    client = client or FakeClient()
    warped = np.arange(np.prod(spec.warped_shape), dtype=np.uint64).astype(np.uint8)
    state = self.make(spec, client, warped=warped)
    self.frames(state, n, after_enqueue)
    return spec, state, client, warped

  @staticmethod
  def frames(state, n: int, after_enqueue=None) -> list:
    bufs = {k: SimpleNamespace(data=np.zeros(8, np.uint8)) for k in ('img', 'big_img')}
    outs = []
    for i in range(n):
      desire = np.zeros(8, np.float32)
      desire[3] = 1.0 if i >= 1 else 0.0   # held from frame 1: a pulse on 1 only
      args = (bufs, {'img': np.eye(3), 'big_img': np.eye(3)},
              {'desire_pulse': desire, 'traffic_convention': np.array([1, 0], np.float32),
               'action_t': np.array([0.1, 0.2], np.float32)})
      outs.append(state.run(*args, after_enqueue))
    return outs


class TestWire(ModelStateTest):
  def test_a_stateful_model_gets_the_frame_and_twelve_floats(self):
    spec, state, client, warped = self.run_frames(STATEFUL)
    self.assertEqual(state.vision_input_names, ['img', 'big_img'])
    self.assertNotIn('prev_feat', state.npy)
    for i, (data, packed, frame_id, reset) in enumerate(client.sent):
      self.assertEqual(frame_id, i + 1)
      self.assertEqual(reset, i == 0)
      np.testing.assert_array_equal(data, warped)
      self.assertEqual(packed.shape, (12,))
      np.testing.assert_array_equal(packed[8:], np.array([1, 0, 0.1, 0.2], np.float32))
    # the desire pulse is the rising edge, as openpilot's own ModelState sends it
    self.assertEqual([p[3] for _, p, _, _ in client.sent], [0.0, 1.0, 0.0])

  def test_the_warp_gets_the_camera_buffers_where_they_are_and_their_transforms(self):
    # and the frame goes out of its output once it is done (FakeWarp)
    state = self.make(spec_for(STATEFUL), FakeClient())
    bufs = {k: SimpleNamespace(data=np.zeros(8, np.uint8)) for k in ('img', 'big_img')}
    tfm = {'img': np.eye(3) * 2, 'big_img': np.eye(3) * 3}
    state.run(bufs, tfm, {'desire': np.zeros(8, np.float32), 'traffic_convention': np.zeros(2, np.float32),
                          'action_t': np.zeros(2, np.float32)})
    (frame, big_frame, got_tfm, got_big), = state.warp.started
    self.assertEqual((frame, big_frame), (bufs['img'].data.ctypes.data, bufs['big_img'].data.ctypes.data))
    np.testing.assert_array_equal(got_tfm, tfm['img'])
    np.testing.assert_array_equal(got_big, tfm['big_img'])

  def test_every_frame_is_measured_for_the_leave(self):
    # the whole frame as modeld waits on it, against the server's own total;
    # the phone's log carries only the latter without this
    spec, state, client, warped = self.run_frames(STATEFUL)
    self.assertEqual(state.trips.frames, 3)
    summary = state.trips.summary()
    self.assertEqual(summary['frames'], 3)
    self.assertEqual(summary['over'], 0)
    for key in ('span_s', 'p50_ms', 'p99_ms', 'max_ms', 'server_ms'):
      self.assertIn(key, summary)

  def test_trips_summarise_the_frames_kept(self):
    trips = model_state.Trips()
    for _ in range(99):
      trips.record(0.030, 25_000)
    trips.record(0.200, 25_000)
    summary = trips.summary()
    self.assertEqual((summary['frames'], summary['over']), (100, 1))
    self.assertEqual((summary['p50_ms'], summary['max_ms'], summary['server_ms']), (30.0, 200.0, 25.0))
    self.assertEqual(summary['p99_ms'], 30.0, 'p99 is the 99th of a hundred, as the server takes it')
    self.assertEqual(model_state.Trips().summary(), {'frames': 0, 'over': 0, 'held': 0, 'span_s': 0.0})

  def test_a_queued_model_gets_the_frame_and_twelve_floats_too(self):
    spec, state, client, warped = self.run_frames(QUEUED)
    self.assertNotIn('prev_feat', state.npy)
    for data, packed, _, _ in client.sent:
      np.testing.assert_array_equal(data, warped)
      self.assertEqual(packed.shape, (12,))
      np.testing.assert_array_equal(packed[8:], np.array([1, 0, 0.1, 0.2], np.float32))

  def test_the_warp_is_sized_from_either_layout(self):
    for inputs in (STATEFUL, QUEUED):
      self.assertEqual(spec_for(inputs).model_hw, (128, 256))

  def test_the_first_frames_and_slow_ones_are_timed_in_the_log(self):
    self.run_frames(STATEFUL, n=4)
    timed = [line for line in self.log.lines('warning') if ' warp ' in line]
    self.assertEqual([line.split()[2] for line in timed], ['1', '2', '3'])


class TestProving(ModelStateTest):
  """The second after a swap is the large model's proof: a single held frame
  in its first PROVING_FRAMES is behind, before anyone can engage."""

  def late_at(self, late: set[int], n: int):
    client = FakeClient()
    client.late = late
    spec = spec_for(STATEFUL)
    behind = []
    state = self.make(spec, client)
    for _ in range(n):
      self.frames(state, 1)
      behind.append(state.behind)
    return behind

  def test_a_held_frame_while_proving_is_behind(self):
    behind = self.late_at({7}, n=8)
    self.assertEqual(behind[:6], [None] * 6)
    self.assertEqual(behind[6], f'held frame 7 of the first {model_state.PROVING_FRAMES}')
    self.assertIsNone(behind[7], 'said once, for the frame that held')

  def test_the_last_proving_frame_still_counts_and_the_one_after_is_weather(self):
    n = model_state.PROVING_FRAMES
    self.assertIsNotNone(self.late_at({n}, n=n)[-1])
    self.assertEqual(self.late_at({n + 1}, n=n + 1), [None] * (n + 1))

  def test_a_held_frame_while_settling_is_forgiven(self):
    # the first after a join carries the history reset, a Mac's is CoreML warm-up
    n = model_state.SETTLING_FRAMES
    self.assertEqual(self.late_at(set(range(2, n + 1)), n=n), [None] * n)
    self.assertIsNotNone(self.late_at({n + 1}, n=n + 1)[-1])

  def test_a_host_that_keeps_up_proves_without_a_word(self):
    self.assertEqual(self.late_at(set(), n=model_state.PROVING_FRAMES + 5),
                     [None] * (model_state.PROVING_FRAMES + 5))


class TestHold(ModelStateTest):
  """A reply not back HOLD_FRAME into the frame is not waited for: the
  previous output goes out again and the camera frame is not dropped."""

  def _second_frame_held(self, client) -> None:
    """Three frames planning 1, 2, 2, where `client` holds the second: it
    publishes the first's output again, and the third its own."""
    state = self.make(spec_for(STATEFUL), client)
    client.output[slice(*SLICES['plan'])] = 1.0
    first, = self.frames(state, 1)
    client.output = client.output.copy()
    client.output[slice(*SLICES['plan'])] = 2.0
    held, after = self.frames(state, 2)
    self.assertTrue((first['plan'] == 1.0).all())
    self.assertTrue((held['plan'] == 1.0).all(), 'the frame before, again')
    self.assertTrue((after['plan'] == 2.0).all(), 'and the next frame its own')
    self.assertEqual(state.trips.held, 1)

  def test_a_late_reply_publishes_the_previous_output_once(self):
    client = FakeClient()
    client.late = {2}
    self._second_frame_held(client)
    self.assertTrue(any('held' in line for line in self.log.lines('warning')))

  def test_the_hold_is_the_rest_of_the_frame_budget(self):
    _, state, client, _ = self.run_frames(STATEFUL, n=3)
    self.assertIsNone(client.holds[0], 'nothing to publish again yet: the first frame waits')
    for hold in client.holds[1:]:
      self.assertGreater(hold, 0.0)
      self.assertLessEqual(hold, model_state.HOLD_FRAME)

  def test_with_the_hold_off_every_frame_waits(self):
    with mock.patch.object(model_state, 'HOLD_FRAME', None):
      _, _, client, _ = self.run_frames(STATEFUL, n=3)
    self.assertEqual(client.holds, [None, None, None])
    self.assertEqual(client.skippable, [False] * 3, 'and its send waits for the link too')

  def test_a_frame_skips_a_busy_link_only_with_an_output_to_hold(self):
    _, _, client, _ = self.run_frames(STATEFUL, n=3)
    self.assertEqual(client.skippable, [False, True, True])

  def test_a_frame_the_link_would_not_take_is_held_without_waiting(self):
    client = FakeClient()
    client.busy = {2}
    self._second_frame_held(client)
    self.assertEqual([f for _, _, f, _ in client.sent], [1, 3], 'frame 2 never went out')
    self.assertEqual(len(client.holds), 2, 'and nothing waited for it')
    self.assertTrue(any('not sent' in line for line in self.log.lines('warning')))

  def test_an_unsent_frame_leaves_the_reset_for_the_next_one(self):
    client = FakeClient()
    spec = spec_for(STATEFUL)
    state = self.make(spec, client)
    self.frames(state, 1)
    state._need_reset = True      # a swap asked the host to start over
    client.busy = {2}
    self.frames(state, 2)
    self.assertEqual([(f, reset) for _, _, f, reset in client.sent], [(1, True), (3, True)])

  def holding(self, late, n: int, **patches):
    """`n` driven frames, the replies to `late` seqs (a predicate) too late
    to wait for. The state, and what `behind` said after each frame. Past
    the proof unless `patches` say otherwise (TestProving)."""
    patches.setdefault('PROVING_FRAMES', 0)
    patches.setdefault('SETTLING_FRAMES', 0)
    client = FakeClient()
    client.late = {seq for seq in range(1, n + 1) if late(seq)}
    spec = spec_for(STATEFUL)
    behind = []
    with contextlib.ExitStack() as patched:
      patched.enter_context(mock.patch.multiple(model_state, **patches))
      state = self.make(spec, client)
      for _ in range(n):
        self.frames(state, 1)
        behind.append(state.behind)
    return state, behind

  def test_holds_in_a_row_say_behind(self):
    # the first frame has nothing to hold, so it waits; the five after are held
    state, behind = self.holding(lambda seq: True, n=1 + model_state.HOLDS_IN_A_ROW)
    self.assertEqual(behind[:-1], [None] * model_state.HOLDS_IN_A_ROW)
    self.assertEqual(behind[-1], f'held {model_state.HOLDS_IN_A_ROW} frames in a row')
    self.assertEqual(state.trips.held, model_state.HOLDS_IN_A_ROW)

  def test_a_frame_that_is_not_held_ends_the_run(self):
    run = model_state.HOLDS_IN_A_ROW - 1
    _, behind = self.holding(lambda seq: seq != 1 and seq != run + 2, n=1 + 2 * run + 1)
    self.assertEqual(behind, [None] * len(behind))

  def test_holds_spread_over_the_window_say_behind_past_the_allowance(self):
    allowed = model_state.HOLDS_ALLOWED
    _, behind = self.holding(lambda seq: seq % 2 == 0, n=2 * allowed + 2)   # never two in a row
    self.assertEqual(behind[:2 * allowed + 1], [None] * (2 * allowed + 1), 'the allowance is forgiven')
    self.assertEqual(behind[-1], f'held {allowed + 1} frames in {model_state.HOLD_WINDOW:.0f} s')

  def test_holds_further_apart_than_the_window_are_weather(self):
    # a window of nothing: every hold is the only one in it
    _, behind = self.holding(lambda seq: seq % 2 == 0, n=4 * model_state.HOLDS_ALLOWED, HOLD_WINDOW=0.0)
    self.assertEqual(behind, [None] * len(behind))


class TestTheFace(ModelStateTest):
  """What modeld reads off the model is comma's, as the adapter hands it over."""

  def test_it_carries_the_adapters_face(self):
    state = self.make(spec_for(STATEFUL), FakeClient())
    face = fakes.FACE
    self.assertIs(state.constants, face.constants)
    self.assertEqual((state.LAT_SMOOTH_SECONDS, state.LONG_SMOOTH_SECONDS), (0.0, 0.3))
    self.assertEqual(state.PLANPLUS_CONTROL, 1.0)
    # the module function, not bound to the model
    self.assertEqual(state.get_action_from_model('out', 'prev'), ('action', ('out', 'prev')))
    self.assertEqual(state.lat_delay, 0.0, "the joining model's to set, from the small model")
    self.assertIsInstance(state.parser, fakes.FakeParser)
    self.assertEqual(state.prev_desire.shape, (face.desire_len,))
    self.assertIs(state.chestnut, True)

  def test_outputs_go_through_the_parser_sliced(self):
    spec = spec_for(STATEFUL)
    client = FakeClient()
    client.output[slice(*SLICES['plan'])] = 2.0
    state = self.make(spec, client)
    bufs = {k: SimpleNamespace(data=np.zeros(8, np.uint8)) for k in ('img', 'big_img')}
    out = state.run(bufs, {'img': np.eye(3), 'big_img': np.eye(3)},
                    {'desire': np.zeros(8, np.float32), 'traffic_convention': np.zeros(2, np.float32),
                     'action_t': np.zeros(2, np.float32)})
    self.assertEqual(set(out), set(SLICES))
    self.assertEqual(out['plan'].shape, (1, 990))
    self.assertTrue((out['plan'] == 2.0).all())

  def test_closing_it_closes_its_link(self):
    client = mock.Mock()
    self.make(spec_for(STATEFUL), client).close()
    client.close.assert_called_once_with()


class TestTelemetry(ModelStateTest):
  """The server's health rides on the response to a frame that asked for it.
  modeld asked by passing a callback every second frame; with none, the model
  asks as often itself and logs what came back at 1 Hz."""

  def test_without_a_callback_a_frame_asks_when_the_log_is_due(self):
    # the log takes one a second; asking on every second frame cost nine
    # replies in ten a JSON decode nobody read
    client = FakeClient()
    client.telemetry = {'gpu_temp': 51.0}
    _, _, client, _ = self.run_frames(STATEFUL, n=6, client=client)
    self.assertEqual(client.asked, [True, False, False, False, False, False])
    self.assertEqual(len(self.events), 1)

  def test_what_came_back_is_logged_at_one_hertz(self):
    client = FakeClient()
    client.last_state = {'gpu_temp': 51.0}
    self.run_frames(STATEFUL, n=6, client=client)
    self.assertEqual(self.events, [('jetlinkTelemetry', {'dead': False, 'gpu_temp': 51.0})])

  def test_send_diagnostics_include_session_maxima_without_extra_log_frequency(self):
    client = FakeClient()
    client.last_state = {'gpu_temp': 51.0}
    client.t.last_send = {'submit_ms': 3.0, 'errno': None}
    client.t.send_totals = {'messages': 100, 'max_submit_ms': 19.0}
    self.run_frames(STATEFUL, n=6, client=client)
    sends = [fields for name, fields in self.events if name == 'jetlinkSend']
    self.assertEqual(len(sends), 1)
    self.assertEqual(sends[0]['nonce'], 'test-session')
    self.assertEqual(sends[0]['submit_ms'], 3.0)
    self.assertEqual(sends[0]['totals']['max_submit_ms'], 19.0)
    client.t.send_totals['messages'] += 1
    self.assertEqual(sends[0]['totals']['messages'], 100)

  def test_nothing_back_yet_is_nothing_logged(self):
    self.run_frames(STATEFUL, n=4)
    self.assertEqual(self.events, [])

  def test_a_callback_asks_and_is_called_instead(self):
    client = FakeClient()
    client.last_state = {'gpu_temp': 51.0}
    callback = mock.Mock()
    self.run_frames(STATEFUL, n=3, client=client, after_enqueue=callback)
    self.assertEqual(client.asked, [True, True, True])
    self.assertEqual(callback.call_count, 3)


if __name__ == '__main__':
  unittest.main()
