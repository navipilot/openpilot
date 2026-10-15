"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

When the warp JIT is trusted, and when it is not; the calls every frame and
the build make of it; and the small model's reset for a fallback.

The compile needs a GPU and the fork's tinygrad, and the fork's tests run it
for real (the build's targets, a real capture's input names, prepare_reset on
a real TinyJit). What is left here is the file on disk being wrong, a TinyJit
from another tinygrad or one pickled before it captured, each a raise into the
small-model fallback, and the plumbing, over a tinygrad made of numpy.
"""
import argparse
import pickle
import sys
import unittest
from pathlib import Path
from types import ModuleType, SimpleNamespace
from unittest import mock

import numpy as np

from jetlink.openpilot import warp
from tests.openpilot import fakes
from tests.openpilot.fakes import OpenpilotTest

GEOM = fakes.TICI


class FakeCaptured:
  def __init__(self, names):
    self.expected_names = names


class FakeJit:
  """Just enough of a TinyJit to be pickled and inspected."""

  def __init__(self, names):
    self.captured = FakeCaptured(names) if names is not None else None


class RecordingGraph:
  """A warp graph that records how it was called; picklable, as a JIT is."""

  def __init__(self):
    self.calls = []

  def __call__(self, **kwargs):
    self.calls.append(sorted(kwargs))
    return fakes.FakeTensor(np.zeros(1))


class WarpTest(OpenpilotTest):
  def setUp(self):
    super().setUp()
    self.warps = self.parts.warps

  def write(self, geom=GEOM, body=b'pickle'):
    pkl = self.warps.path(*geom)
    pkl.parent.mkdir(parents=True, exist_ok=True)
    pkl.write_bytes(body)
    return pkl


class TestValidity(WarpTest):
  def test_a_warp_that_is_there_is_used(self):
    """A bare pickle is what the build writes, and all it writes; asking for a
    sidecar would reject every warp the build produces."""
    self.write()
    self.assertFalse(self.warps.path(*GEOM).with_suffix('.json').exists())
    self.assertTrue(self.warps.is_cached(*GEOM))

  def test_nothing_cached_is_a_miss(self):
    self.assertFalse(self.warps.is_cached(*GEOM))
    self.assertFalse(self.warps.built())

  def test_where_it_lives_is_the_adapters_to_say(self):
    # in the fork's tree, never inside the jetlink submodule: a pickle there
    # leaves the submodule dirty and the updater fighting it
    self.assertEqual(self.warps.path(*GEOM), self.op.warp_path(*GEOM))

  def test_another_camera_does_not_answer_for_this_one(self):
    """A device that changed camera, or a cache copied between devices. The
    warp is baked against the frame geometry, so the wrong one is silently
    wrong rather than an error."""
    self.write(geom=fakes.MICI)
    self.assertFalse(self.warps.is_cached(*GEOM))
    self.assertFalse(self.warps.built())

  def test_another_model_input_size_does_not_answer_either(self):
    self.write(geom=(1928, 1208, 256, 128))
    self.assertFalse(self.warps.is_cached(*GEOM))

  def test_built_is_for_this_devices_camera_and_holds_for_the_process(self):
    # nothing compiles one at runtime and the build runs before manager
    self.write()
    self.op.geometry = fakes.TICI
    self.assertTrue(self.warps.built())
    self.warps.path(*GEOM).unlink()
    self.assertTrue(self.warps.built())

  def test_the_geometry_is_the_adapters(self):
    self.op.geometry = fakes.MICI
    self.assertEqual(self.warps.geometry(), fakes.MICI)


class TestLoad(WarpTest):
  def test_a_miss_raises_rather_than_returning_none(self):
    """modeld's big-model load is wrapped in the fallback to the small model.
    Raising lands there; returning None would reach the car."""
    with self.assertRaisesRegex(RuntimeError, 'no warp built for 1928x1208 -> 512x256'):
      self.warps.load(*GEOM)

  def test_a_pickle_that_will_not_load_raises(self):
    """The incompatible-tinygrad case: unpickling fails and modeld's big-model load falls back."""
    self.write(body=b'not a pickle at all')
    with self.assertRaises(pickle.UnpicklingError):
      self.warps.load(*GEOM)


class TestLoadValidation(WarpTest):
  """A pickle that loads is not yet a warp that can be trusted."""

  def test_a_good_warp_loads(self):
    self.write(body=pickle.dumps(FakeJit(warp.WARP_INPUT_NAMES)))
    self.assertIsInstance(self.warps.load(*GEOM), FakeJit)

  def test_the_wrong_call_convention_is_refused_at_load(self):
    # This is the shape of the bug that reached the car: captured positionally,
    # called by keyword, JitError on the first frame of the drive.
    self.write(body=pickle.dumps(FakeJit([0, 1, 2, 3])))
    with self.assertRaises(RuntimeError) as e:
      self.warps.load(*GEOM)
    self.assertIn('call_warp passes', str(e.exception))

  def test_an_uncaptured_jit_is_refused(self):
    # Pickled before TinyJit captured: loads fine, computes nothing.
    self.write(body=pickle.dumps(FakeJit(None)))
    with self.assertRaises(RuntimeError) as e:
      self.warps.load(*GEOM)
    self.assertIn('computes nothing', str(e.exception))


class TestInitDevice(unittest.TestCase):
  """prepare() runs this before modeld goes realtime: tinygrad's compile pool
  is otherwise created on the warp's first call, and its handler threads then
  sit at FIFO 54 on the frame loop's core."""

  def setUp(self):
    self.pool = mock.Mock(name='get_worker_pool')
    p = mock.patch.dict(sys.modules, fakes.fake_tinygrad(get_worker_pool=self.pool))
    p.start()
    self.addCleanup(p.stop)
    self.log = fakes.RecordingLog()

  def test_the_compile_pool_is_created_with_the_device(self):
    warp.init_device(self.log)
    self.pool.assert_called_once_with()
    self.assertEqual(self.log.records, [])

  def test_a_pool_that_will_not_start_is_logged_not_raised(self):
    # An older tinygrad without the module, or PARALLEL=0, must not veto the accelerator.
    self.pool.side_effect = RuntimeError('no pool')
    warp.init_device(self.log)
    self.assertEqual(len(self.log.lines('exception')), 1)

  def test_a_device_that_will_not_come_up_is_logged_not_raised(self):
    with mock.patch.object(fakes.FakeTensor, 'realize', side_effect=RuntimeError('no gpu')):
      warp.init_device(self.log)
    self.assertTrue(self.log.has('could not bring the gpu up', 'exception'))
    self.pool.assert_called_once_with()


class TestCallConvention(unittest.TestCase):
  """The compile and the per-frame call have to name the JIT's inputs the same way.

  TinyJit refuses a call whose names differ from the capture, so a positional
  compile and a keyword call raise JitError on the first frame of a drive.
  """

  def test_call_warp_passes_everything_by_keyword(self):
    seen = {}

    def recorder(*args, **kwargs):
      seen['args'], seen['kwargs'] = args, kwargs
      return 'warped'

    out = warp.call_warp(recorder, 'T', 'BT', 'F', 'BF')
    self.assertEqual(out, 'warped')
    self.assertEqual(seen['args'], (), "positional args make TinyJit capture [0, 1, 2, 3]")
    self.assertEqual(seen['kwargs'], {'tfm': 'T', 'big_tfm': 'BT', 'frame': 'F', 'big_frame': 'BF'})
    self.assertEqual(sorted(seen['kwargs']), warp.WARP_INPUT_NAMES)

  def test_every_call_site_goes_through_the_helper(self):
    # a second direct call site would diverge again. Read the sources
    pkg = Path(warp.__file__).parent
    for name in ('warp.py', 'model_state.py', 'joining.py'):
      for line in (pkg / name).read_text().splitlines():
        stripped = line.strip()
        if ('warp_jit(' in stripped or 'self.warp(' in stripped or 'warp(tfm' in stripped) \
            and 'call_warp' not in stripped and not stripped.startswith(('def ', 'return warp(tfm')):
          self.fail(f"{name} calls the warp JIT directly: {stripped}")


class UopTensor(fakes.FakeTensor):
  """A FakeTensor that stands for its own buffer, which is what a warp's
  capture is handed (uop.base)."""

  def __init__(self, data=None, device=None, dtype=None):
    super().__init__(data, device, dtype)
    self.uop = SimpleNamespace(base=self)

  @staticmethod
  def from_blob(ptr, shape, dtype=None, device=None):
    blob = UopTensor(device=device)
    blob.ptr, blob.shape = ptr, shape
    return blob


class FakeQcomDevice:
  """A tinygrad device as Warp uses it. An allocation's record holds the
  flags the kernel kept: the request masked by `keep`, plus some of its own."""

  def __init__(self):
    self.keep = ~0
    self.allocs, self.freed = [], []
    self.synchronize = mock.Mock(name='synchronize')

  def _gpu_alloc(self, size, flags=0):
    mem = SimpleNamespace(size=size, meta=(SimpleNamespace(flags=flags & self.keep | 0x100c0000), True))
    self.allocs.append(mem)
    return mem

  def _gpu_free(self, mem):
    self.freed.append(mem)


class FakeOutput:
  """The warp JIT's output Buffer, write-combined as tinygrad allocates it."""

  def __init__(self, device):
    self.device, self.nbytes = device, 64
    self._buf = SimpleNamespace(meta=(SimpleNamespace(flags=0x100c0000), True))
    self.bytes = bytearray(64)

  def deallocate(self):
    self._buf = None

  def allocate(self, opaque=None):
    assert self._buf is None, "can't allocate already allocated buffer"
    self._buf = opaque
    return self

  def as_memoryview(self, force_zero_copy=False, no_sync=False):
    assert force_zero_copy and no_sync, 'a copy, or a synchronize the caller makes itself'
    return memoryview(self.bytes)


class FakeDest:
  """A QCOM buffer of the capture's own, the CPU's view of it."""

  def __init__(self, data):
    self.data = data

  def as_memoryview(self, force_zero_copy=False):
    assert force_zero_copy
    return memoryview(self.data)


class FakeCapture:
  """A captured warp: what TinyJit calls went in (each with the output's
  allocation as the call found it), and what replays. Its steps are tinygrad's
  for QCOM: copy tfm, copy big_tfm (each from its input slot into a buffer of
  the capture's own, `dests`), then the graph, which replays here too."""

  def __init__(self, out):
    self.out = out
    self.ret = SimpleNamespace(uop=SimpleNamespace(base=SimpleNamespace(buffer=out)))
    self.calls, self.replays = [], []
    self.dests = {name: bytearray(36) for name in ('tfm', 'big_tfm')}

    def copy(name):
      return SimpleNamespace(src=(SimpleNamespace(op='COPY'),), slot=warp.WARP_INPUT_NAMES.index(name),
                             dest=SimpleNamespace(buffer=FakeDest(self.dests[name])))
    self.graph = SimpleNamespace(op='CUSTOM_FUNCTION', arg='graph')
    self._linear = SimpleNamespace(src=(copy('tfm'), copy('big_tfm'), SimpleNamespace(src=(self.graph,))))

  def resolve_params(self, step, inputs):
    return step.dest, inputs[step.slot]

  def get_graph_runtime(self, ast, inputs):
    assert ast is self.graph
    return self

  def tfm(self, name):
    return np.frombuffer(self.dests[name], dtype=np.float32).reshape(3, 3)

  def jit(self, **kwargs):
    self.calls.append((kwargs, self.out._buf))
    return self.ret

  def __call__(self, bufs, var_vals):
    self.replays.append((bufs, var_vals))
    return self.ret


class TestWarp(unittest.TestCase):
  """The warp as the frame loop runs it: its output in memory the CPU reads
  through its cache, checked once and replayed after, camera buffers as
  tensors once."""

  def setUp(self):
    self.devices = {'QCOM': FakeQcomDevice(), 'CPU': FakeQcomDevice()}
    self.modules = modules = fakes.fake_tinygrad()
    modules['tinygrad.device'].Device = self.devices
    modules['tinygrad.tensor'].Tensor = UopTensor
    modules['tinygrad.uop'] = ModuleType('tinygrad.uop')
    modules['tinygrad.uop.ops'] = ModuleType('tinygrad.uop.ops')
    modules['tinygrad.uop.ops'].Ops = SimpleNamespace(COPY='COPY', CUSTOM_FUNCTION='CUSTOM_FUNCTION', CALL='CALL', LINEAR='LINEAR')
    modules['tinygrad.uop.ops'].UOp = lambda op, src=(): SimpleNamespace(op=op, src=src)
    modules['tinygrad.engine.realize'] = ModuleType('tinygrad.engine.realize')
    modules['tinygrad.engine.realize'].run_linear = mock.Mock()
    p = mock.patch.dict(sys.modules, modules)
    p.start()
    self.addCleanup(p.stop)
    self.log = fakes.RecordingLog()

  def make(self, device='QCOM'):
    capture = FakeCapture(FakeOutput(device))
    realize = self.modules['tinygrad.engine.realize']
    realize.resolve_params, realize.get_graph_runtime = capture.resolve_params, capture.get_graph_runtime
    jit = mock.Mock(side_effect=capture.jit)
    jit.captured = capture
    return warp.Warp(jit, 1234, self.log), capture

  def test_the_output_is_coherent_before_the_first_call(self):
    # the first call binds the graph to its buffers' addresses
    w, capture = self.make()
    qcom = self.devices['QCOM']
    self.assertEqual(len(qcom.allocs), 1)
    self.assertTrue(all(found is qcom.allocs[0] for _, found in capture.calls))
    self.assertEqual(qcom.allocs[0].meta[0].flags & warp.COHERENT_WRITEBACK, warp.COHERENT_WRITEBACK)
    self.assertEqual(w.output.nbytes, 64)
    self.assertIs(w.wait, qcom.synchronize)

  def test_memory_the_kernel_would_not_make_coherent_is_refused(self):
    # write-back memory that is not coherent read stale frames, 396 of 400
    for withheld in (1 << 31, 1 << 26):   # coherency; write-back
      self.devices['QCOM'].keep = ~withheld
      with self.assertRaisesRegex(RuntimeError, 'IO-coherent'):
        self.make()
      self.assertIs(self.devices['QCOM'].freed[-1], self.devices['QCOM'].allocs[-1])

  def test_a_cpu_warp_keeps_its_output(self):
    # the fork's frame-path test runs the real warp on tinygrad's CPU device
    self.make('CPU')
    self.assertEqual(self.devices['CPU'].allocs, [])

  def test_it_is_warmed_through_tinygrad_on_zero_frames_of_the_cameras_size(self):
    _, capture = self.make()
    self.assertEqual(len(capture.calls), 2)
    kwargs, _ = capture.calls[0]
    self.assertEqual(sorted(kwargs), warp.WARP_INPUT_NAMES)
    self.assertEqual(kwargs['frame'].shape, (1234,))
    self.assertEqual(capture.replays, [])
    self.devices['QCOM'].synchronize.assert_called_once_with()

  def test_a_frame_runs_the_graph_alone_with_its_buffers(self):
    # the transforms go straight into the buffers the capture copies them to
    w, capture = self.make()
    tfm, big_tfm = np.eye(3) * 2, np.eye(3) * 3
    w.start(0x1000, 0x2000, tfm, big_tfm)
    (bufs, var_vals), = capture.replays
    self.assertEqual(var_vals, {})
    big_frame, _, frame, _ = bufs   # sorted names, as the capture took them
    self.assertEqual((frame.ptr, big_frame.ptr), (0x1000, 0x2000))
    self.assertEqual((frame.shape, frame.device), ((1234,), 'QCOM'))
    np.testing.assert_array_equal(capture.tfm('tfm'), tfm)
    np.testing.assert_array_equal(capture.tfm('big_tfm'), big_tfm)
    self.assertEqual(len(capture.calls), 2, 'no call through TinyJit after the warm-up')

  def test_an_hcq_capture_replays_the_whole_capture(self):
    capture = FakeCapture(FakeOutput('QCOM'))
    hcq_call = SimpleNamespace(op='CALL', src=(SimpleNamespace(op='PROGRAM'),))
    step0_wrapped = SimpleNamespace(op='AFTER', src=(capture._linear.src[0],), without_after=capture._linear.src[0])
    capture._linear.src = (step0_wrapped, capture._linear.src[1], hcq_call)
    capture.linear = capture._linear
    jit = mock.Mock(side_effect=capture.jit)
    jit.captured = capture
    w = warp.Warp(jit, 1234, self.log)
    tfm, big_tfm = np.eye(3) * 2, np.eye(3) * 3
    w.start(0x1000, 0x2000, tfm, big_tfm)
    (bufs, _), = capture.replays
    big_frame, big_tfm_buf, frame, tfm_buf = bufs
    self.assertEqual((frame.ptr, big_frame.ptr), (0x1000, 0x2000))
    np.testing.assert_array_equal(tfm_buf.array, tfm)
    np.testing.assert_array_equal(big_tfm_buf.array, big_tfm)
    self.assertEqual(len(capture.calls), 2, 'no call through TinyJit after the warm-up')

  def test_a_capture_of_other_steps_is_refused(self):
    capture = FakeCapture(FakeOutput('QCOM'))
    capture._linear.src = capture._linear.src[2:]
    realize = self.modules['tinygrad.engine.realize']
    realize.resolve_params, realize.get_graph_runtime = capture.resolve_params, capture.get_graph_runtime
    jit = mock.Mock(side_effect=capture.jit)
    jit.captured = capture
    with self.assertRaisesRegex(RuntimeError, 'two transform copies and a graph'):
      warp.Warp(jit, 1234, self.log)

  def test_a_cpu_warp_replays_the_whole_capture(self):
    # the fork's frame-path test runs the real warp on tinygrad's CPU device
    w, capture = self.make('CPU')
    w.start(0x1000, 0x2000, np.eye(3) * 2, np.eye(3) * 3)
    (bufs, _), = capture.replays
    np.testing.assert_array_equal(bufs[3].array, np.eye(3) * 2)
    np.testing.assert_array_equal(bufs[1].array, np.eye(3) * 3)

  def test_a_camera_buffer_becomes_a_tensor_once(self):
    w, capture = self.make()
    for _ in range(3):
      w.start(0x1000, 0x2000, np.eye(3), np.eye(3))
    first, *rest = [bufs for bufs, _ in capture.replays]
    for bufs in rest:
      self.assertTrue(all(a is b for a, b in zip(first, bufs, strict=True)))

  def test_buffers_past_two_generations_are_logged_once(self):
    w, _ = self.make()
    with mock.patch.object(warp, 'FRAMES_WARN', 3):
      for i in range(6):
        w.start(0x1000 * (i + 1), 0x100000, np.eye(3), np.eye(3))
    self.assertEqual(len([line for line in self.log.lines('warning') if 'camera buffers' in line]), 1)


class TestPrepareReset(unittest.TestCase):
  """The small model's history is zeroed in place on a fallback: the JIT's
  buffers keep their identities and nothing is compiled on the failure frame.
  The fork runs this against a real TinyJit."""

  def setUp(self):
    p = mock.patch.dict(sys.modules, fakes.fake_tinygrad())
    p.start()
    self.addCleanup(p.stop)

  def queue(self, device='CPU'):
    return fakes.FakeTensor(np.ones((4, 8), np.float32), device=device)

  def test_stock_modeld_s_model(self):
    model = SimpleNamespace(input_queues={k: self.queue() for k in ('img_q', 'feat_q')},
                            prev_desire=np.ones(8), npy={'prev_feat': np.ones(32), 'desire': np.ones(8)})
    warp.prepare_reset(model)()
    for q in model.input_queues.values():
      np.testing.assert_array_equal(q.array, 0)
    np.testing.assert_array_equal(model.prev_desire, 0)
    for v in model.npy.values():
      np.testing.assert_array_equal(v, 0)

  def test_a_modeld_v2_bundle_and_its_npy_tensor(self):
    # numpy inputs under numpy_inputs; the NPY tensor is not a queue, and its
    # numpy views are what zero it
    packed = np.ones(16, np.float32)
    npy = self.queue(device='NPY')
    model = SimpleNamespace(input_queues={'img_q': self.queue(), 'packed_npy_inputs': npy},
                            prev_desire=np.ones(8), numpy_inputs={'desire': packed[:8], 'rest': packed[8:]})
    with mock.patch.object(npy, 'assign', side_effect=AssertionError('an NPY tensor is not a queue')):
      warp.prepare_reset(model)()
    np.testing.assert_array_equal(model.input_queues['img_q'].array, 0)
    np.testing.assert_array_equal(packed, 0)


class TestTheBuild(OpenpilotTest):
  """python -m jetlink.openpilot.warp, as the fork's build runs it per camera."""

  def test_it_builds_the_adapters_graph_for_the_camera(self):
    out = self.tmp / 'warp.pkl'
    with mock.patch.object(warp, 'compile_warp', return_value=out) as compile_warp, \
         mock.patch('jetlink.openpilot.interface.load_adapter', return_value=self.op) as load, \
         mock.patch.object(self.op, 'make_warp', wraps=self.op.make_warp) as make_warp:
      warp.main(['--adapter', 'x.adapter', '--camera', '1344x760', '--model', '512x256', '--output', str(out)])
    load.assert_called_once_with('x.adapter')
    make_warp.assert_called_once_with(1344, 760, 512, 256)
    graph, frame_size = compile_warp.call_args.args[:2]
    self.assertEqual(frame_size, fakes.frame_size(1344, 760))
    self.assertEqual(compile_warp.call_args.args[2], out)

  def test_comma_s_graph_is_made_before_anything_of_tinygrad_is_imported(self):
    # comma's graph module patches tinygrad's firmware fetch as it loads
    order = []

    def make_warp(*args):
      order.append(('make_warp', sorted(m for m in sys.modules if m.split('.')[0] == 'tinygrad')))
      return RecordingGraph(), 64

    tinygrad_free = {m: v for m, v in sys.modules.items() if m.split('.')[0] != 'tinygrad'}
    with mock.patch.dict(sys.modules, tinygrad_free, clear=True), \
         mock.patch.object(self.op, 'make_warp', side_effect=make_warp), \
         mock.patch('jetlink.openpilot.interface.load_adapter', return_value=self.op), \
         mock.patch.object(warp, 'compile_warp', side_effect=lambda *a: order.append(('compile', None))):
      warp.main(['--adapter', 'x', '--camera', '1928x1208', '--model', '512x256', '--output', str(self.tmp / 'w.pkl')])
    self.assertEqual(order, [('make_warp', []), ('compile', None)])

  def test_sizes_are_w_by_h(self):
    self.assertEqual(warp.size('1928x1208'), (1928, 1208))
    for bad in ('1928', 'x', '1928xabc'):
      with self.subTest(bad), self.assertRaises(argparse.ArgumentTypeError):
        warp.size(bad)

  def test_everything_is_required(self):
    with self.assertRaises(SystemExit):
      warp.main(['--adapter', 'x', '--camera', '1x1'])

  def test_the_pickle_is_taken_after_the_capture_and_replaced_whole(self):
    graph = RecordingGraph()
    with mock.patch.dict(sys.modules, fakes.fake_tinygrad()):
      out = warp.compile_warp(graph, 64, self.tmp / 'models' / 'warp.pkl')
    self.assertEqual(graph.calls, [warp.WARP_INPUT_NAMES] * 3, 'TinyJit captures on the second call')
    self.assertTrue(out.is_file())
    self.assertFalse(out.with_suffix('.pkl.tmp').exists())
    self.assertEqual(pickle.loads(out.read_bytes()).calls, graph.calls)


if __name__ == "__main__":
  unittest.main()
