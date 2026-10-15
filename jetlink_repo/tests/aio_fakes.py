"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

A stand-in for jetlink.transport.aio.Aio, which only Linux has.

Each submitted request is gathered from the addresses it names, as the kernel
copies it, and written to the fd (a FIFO standing in for ep2) or kept in
`wire` when there is no fd. Completions wait in `done` until reaped, or in
`held` while `holding` is set: a host that is not taking data.
"""
from __future__ import annotations

import ctypes
import os
import time
from collections import deque

from jetlink.transport.aio import address, layout


class FakeAio:
  address = staticmethod(address)
  layout = staticmethod(layout)

  def __init__(self, fd: int | None, depth: int):
    self.fd = fd
    self.depth = depth
    self.wire = bytearray()
    self.requests: list[int] = []      # bytes in each request, as submitted
    self.done: deque[int] = deque()      # results, as reap returns them
    self.held: deque[int] = deque()
    self.holding = False
    self.fail: deque[int] = deque()    # errnos the next submits raise, one each
    self.result = None                 # a function of nbytes -> res, to fake a failed transfer
    self.submits = 0
    self.closed = False

  @property
  def outstanding(self) -> int:
    return len(self.done) + len(self.held)

  def submit(self, plan, bases, start: int, count: int) -> int:
    assert not self.closed, 'submit after close'
    assert self.outstanding + count <= self.depth, 'more requests in flight than the context holds'
    self.submits += 1
    if self.fail:
      err = self.fail.popleft()
      raise OSError(err, os.strerror(err))
    for r in range(start, start + count):
      pieces = range(plan.first[r], plan.first[r + 1])
      data = b''.join(ctypes.string_at(bases[plan.span[i]] + int(plan.offset[i]), int(plan.length[i])) for i in pieces)
      self.requests.append(len(data))
      if self.fd is None:
        self.wire += data
      else:
        view = memoryview(data)
        while view:
          view = view[os.write(self.fd, view):]
      res = len(data) if self.result is None else self.result(len(data))
      (self.held if self.holding else self.done).append(res)
    return count

  def release(self) -> None:
    """The host takes everything it was holding back."""
    self.holding = False
    self.done.extend(self.held)
    self.held.clear()

  def shutdown(self) -> None:
    """The endpoint went away: every request still queued fails."""
    self.done.extend(-108 for _ in self.held)   # ESHUTDOWN
    self.held.clear()

  def reap(self, min_nr: int = 0, timeout: float | None = 0.0):
    if len(self.done) < min_nr and timeout:
      end = time.monotonic() + timeout
      while len(self.done) < min_nr and time.monotonic() < end:
        time.sleep(0.001)
    out = list(self.done)
    self.done.clear()
    return out

  def close(self) -> None:
    assert not self.held, 'io_destroy with requests still queued blocks uninterruptibly'
    self.closed = True
