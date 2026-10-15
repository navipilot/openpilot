"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

The gadget's AIO send path, against tests/aio_fakes.FakeAio: framing into
aligned requests, room, refusals, failures and closing.
"""
import errno

import pytest

from jetlink import protocol as P
from jetlink.transport import ffs
from jetlink.transport.aio import layout
from jetlink.transport.base import LinkError
from jetlink.transport.ffs import FfsTransport
from tests.aio_fakes import FakeAio

FRAME = 393216 + 65536   # a frame's payload, near the real 459 KB


@pytest.fixture
def sender():
  t = FfsTransport.__new__(FfsTransport)
  t._prepare('/nonexistent', gadget=None)
  t._ensure_epfiles = lambda: None
  t._udc_note = lambda: ''
  t.ep_in = 123
  t._aio = aio = FakeAio(None, ffs.QUEUED_LIMIT // FfsTransport.write_chunk)
  unbound = []

  def unbind(gadget=None):
    unbound.append(gadget)
    aio.shutdown()   # disabling the endpoint completes everything queued

  t.unbind = unbind
  t._unbound = unbound
  return t, aio


def _payload(size: int) -> bytes:
  return bytes(range(256)) * (size // 256) + bytes(range(size % 256))


@pytest.mark.parametrize('size', [0, 1, 16352, 16384, 32768, 393216, FRAME, 4 << 20])
@pytest.mark.parametrize('quantum', [8192, 16384])
def test_a_message_crosses_whole_in_aligned_requests(sender, size, quantum):
  t, _ = sender
  t.write_chunk = quantum
  t._aio = aio = FakeAio(None, ffs.QUEUED_LIMIT // quantum)
  payload = _payload(size)
  t.send(P.Msg.INFER_REQ, 7, (payload,), timeout=1.0)
  wire = bytes(aio.wire)
  _, _, kind, seq, _, length, _ = P.unpack_header(wire[:P.HEADER_SIZE])
  assert (kind, seq, length) == (P.Msg.INFER_REQ, 7, size)
  assert wire[P.HEADER_SIZE:P.HEADER_SIZE + size] == payload
  assert not any(wire[P.HEADER_SIZE + size:]), 'padding is zeros'
  assert len(wire) % P.GADGET_TX_ALIGN == 0
  # never a short packet: every request `quantum` bytes, which _collect
  # checks each result against
  assert set(aio.requests) == {quantum}
  if len(wire) <= ffs.QUEUED_LIMIT:
    assert aio.submits == 1, 'a message that fits goes in one io_submit'
  assert t.last_send['bytes'] == len(wire) and t.last_send['requests'] == len(aio.requests)


def test_parts_are_gathered_without_a_copy_of_the_message(sender):
  # the frame, the packed inputs and the header in their own buffers, as
  # JetlinkClient.infer_begin hands them over
  t, aio = sender
  frame, packed = bytearray(_payload(FRAME)), bytearray(_payload(5000))
  t.send(P.Msg.INFER_REQ, 1, (b'\x01' * 8, frame, packed))
  wire = bytes(aio.wire)
  assert wire[P.HEADER_SIZE:P.HEADER_SIZE + 8 + FRAME + 5000] == b'\x01' * 8 + frame + packed


def test_the_callers_buffers_are_free_once_send_returns(sender):
  # the kernel copies at io_submit, so the next frame's warp may overwrite
  # the readback straight away; this fake copies there too
  t, aio = sender
  frame = bytearray(_payload(FRAME))
  t.send(P.Msg.INFER_REQ, 1, (frame,))
  frame[:] = bytes(len(frame))
  assert bytes(aio.wire[P.HEADER_SIZE:P.HEADER_SIZE + 64]) == _payload(64)


def test_a_large_message_streams_through_bounded_room(sender):
  t, aio = sender
  t.send(P.Msg.UPLOAD_CHUNK, 1, (_payload(4 << 20),), timeout=1.0)
  assert aio.submits > 1
  assert len(aio.wire) >= 4 << 20
  # FakeAio asserts the context never held more than its depth


def test_try_send_refuses_a_third_frame_the_host_has_not_taken(sender):
  t, aio = sender
  aio.holding = True
  assert t.try_send(P.Msg.INFER_REQ, 1, (_payload(FRAME),))
  assert t.try_send(P.Msg.INFER_REQ, 2, (_payload(FRAME),)), 'one frame may go out behind another'
  sent = len(aio.wire)
  assert not t.try_send(P.Msg.INFER_REQ, 3, (_payload(FRAME),))
  assert len(aio.wire) == sent, 'a refused frame sends nothing'
  assert t.last_send['refused'] and t.last_send['backlog_kb'] > 0
  assert t.send_totals['refused'] == 1
  aio.release()
  assert t.try_send(P.Msg.INFER_REQ, 3, (_payload(FRAME),))
  assert not t._unbound, 'refusing is not a failure'


def test_a_blocking_send_waits_for_room_then_drops_the_gadget(sender):
  t, aio = sender
  aio.holding = True
  t.send(P.Msg.INFER_REQ, 1, (_payload(FRAME),))
  t.send(P.Msg.INFER_REQ, 2, (_payload(FRAME),))
  with pytest.raises(LinkError, match='took no USB data'):
    t.send(P.Msg.INFER_REQ, 3, (_payload(FRAME),), timeout=0.05)
  assert t._unbound and t.last_send['aborted']
  assert t.send_totals['aborts'] == 1
  with pytest.raises(LinkError, match='link abandoned'):
    t.send(P.Msg.PING, 4)


@pytest.mark.parametrize('result, why', [(lambda n: -errno.EPIPE, 'Broken pipe'),
                                         (lambda n: n - 1024, '7168 of 8192 bytes')])
def test_a_write_the_host_did_not_complete_fails_the_next_send(sender, result, why):
  t, aio = sender
  aio.result = result
  t.send(P.Msg.PING, 1)
  with pytest.raises(LinkError, match=f'gadget write failed: {why}'):
    t.send(P.Msg.PING, 2)
  with pytest.raises(LinkError, match='gadget write failed'):
    t.try_send(P.Msg.PING, 3)


def test_a_failed_submit_is_a_link_error_and_queues_nothing(sender):
  # FunctionFS reports a failed write as its completion; io_submit itself
  # fails only on a bad context or iocb
  t, aio = sender
  aio.fail.append(errno.EBADF)
  with pytest.raises(LinkError, match='gadget write failed'):
    t.send(P.Msg.PING, 1)
  assert t.last_send['errno'] == errno.EBADF
  with pytest.raises(LinkError, match='gadget write failed'):
    t.send(P.Msg.PING, 2)






def test_close_lets_queued_writes_finish_then_destroys_the_context(sender):
  t, aio = sender
  t.send(P.Msg.LEAVE, 1, (b'{}',))
  t._close_fds = lambda names, reader: None
  t.close()
  assert aio.closed and not t.send_totals.get('aborts') and t._aio is None


def test_close_drops_the_gadget_under_writes_the_host_never_takes(sender, monkeypatch):
  monkeypatch.setattr(ffs, 'CLOSE_FLUSH', 0.02)
  t, aio = sender
  aio.holding = True
  t.send(P.Msg.INFER_REQ, 1, (_payload(FRAME),))
  t._close_fds = lambda names, reader: None
  t.close()
  assert t.send_totals['aborts'] == 1 and t._unbound, 'only the unbind completes them'
  assert aio.closed, 'destroyed once they completed'


def test_close_leaves_a_context_it_cannot_empty_to_the_process(sender, monkeypatch):
  # io_destroy would block on requests nothing completes
  monkeypatch.setattr(ffs, 'CLOSE_FLUSH', 0.02)
  monkeypatch.setattr(ffs, 'ABORT_DRAIN', 0.02)
  t, aio = sender
  t.unbind = lambda gadget=None: t._unbound.append(gadget)   # an unbind that frees nothing
  aio.holding = True
  t.send(P.Msg.INFER_REQ, 1, (_payload(FRAME),))
  t._close_fds = lambda names, reader: None
  t.close()
  assert not aio.closed and t._aio is None


def test_a_closing_transport_refuses_to_send(sender):
  t, _ = sender
  t._closing = True
  with pytest.raises(LinkError, match='closing'):
    t.send(P.Msg.PING, 1)


def test_send_totals_keep_the_maxima_between_log_samples(sender):
  t, aio = sender
  for seq in range(3):
    t.send(P.Msg.INFER_REQ, seq, (_payload(FRAME),))
  totals = t.send_totals
  assert totals['messages'] == 3 and totals['requests'] == len(aio.requests)
  assert totals['bytes'] == len(aio.wire)
  assert {'max_submit_ms', 'max_wait_ms', 'max_backlog_ms'} <= totals.keys()


def test_requests_cover_the_message_in_order_without_a_copy():
  spans = [(1000, 32), (5000, 5 * 16384), (900000, 2 * 16384 - 32)]
  plan = layout(tuple(n for _, n in spans), 8192)
  assert plan.count == 14
  assert [int(plan.length[plan.first[r]:plan.first[r + 1]].sum()) for r in range(plan.count)] == [8192] * 14
  # every byte once, in order: the gathered (address, length) runs rebuild the spans
  runs = [(spans[j][0] + int(off), int(n)) for j, off, n in zip(plan.span, plan.offset, plan.length, strict=True)]
  merged = []
  for addr, n in runs:
    if merged and merged[-1][0] + merged[-1][1] == addr:
      merged[-1] = (merged[-1][0], merged[-1][1] + n)
    else:
      merged.append((addr, n))
  assert merged == spans
