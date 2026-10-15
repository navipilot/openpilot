"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

Handing the endpoints over without handing the gadget over.

The owner holds ep0 and the UDC bind for as long as the link is enabled, so a
drive starting or ending is no longer an unplug the Jetson has to recover from.
What still changes hands is the right to read the endpoint files, or on the
cable to listen for the phone's dial, and this is the handshake for it.
"""

import shutil
import socket
import tempfile
import threading
import time
import unittest
from pathlib import Path
from unittest import mock

from jetlink.comma import lending


class LendingTest(unittest.TestCase):
  def setUp(self):
    # a short path: an AF_UNIX address is about 100 bytes and a pytest tmp_path
    # spends most of that before the filename
    self.dir = Path(tempfile.mkdtemp(dir='/tmp'))
    self.addCleanup(shutil.rmtree, self.dir, True)
    self.path = self.dir / 's'
    self.free = True          # the daemon has nothing open on the endpoints
    self.bounced = 0
    p = mock.patch.object(lending, 'RETRY', 0.01)
    self.addCleanup(p.stop)
    p.start()
    p = mock.patch.object(lending.gadget, 'bound_udc', side_effect=lambda: self.udc)
    self.addCleanup(p.stop)
    self.udc = 'udc0'
    p.start()
    # a loopback stand-in for usb0
    p = mock.patch.object(lending.gadget, 'CABLE_ADDR', ('127.0.0.1', 0))
    self.addCleanup(p.stop)
    p.start()
    self.cable_mode = False   # the owner built the gadget for a phone
    self.vacated = 0

  def lender(self, server=None, vacate=None) -> lending.Lender:
    lender = lending.Lender(lambda: self.free, self.bounce, path=self.path,
                            cable=lambda: self.cable_mode, vacate=vacate or self.vacate, server=server)
    assert lender.start()
    self.addCleanup(lender.stop)
    return lender

  def vacate(self) -> None:
    self.vacated += 1

  def listener(self) -> lending.CableListener:
    listener = lending.CableListener()
    assert listener.open()
    self.addCleanup(listener.close)
    return listener

  def dial(self, listener: lending.CableListener) -> socket.socket:
    """A phone: connects, and the owner's step takes the dial."""
    phone = socket.create_connection(listener.bound[:2], timeout=3.0)
    self.addCleanup(phone.close)
    # the accept is non-blocking and the loopback handshake can still be
    # finishing when connect returns; the owner polls every step
    assert self.until(lambda: listener.poll() == '127.0.0.1'), 'the dial was never taken'
    return phone

  def take(self, **kw):
    loan = lending.borrow(path=self.path, **kw)
    if loan is not None:
      self.addCleanup(loan.close)
    return loan

  def refused(self, lender: lending.Lender, timeout: float = 0.3) -> None:
    """A borrow that gives up, and the lender marked lent while it asked.

    Watched from here while the borrower asks rather than read after: a
    borrower that gives up closes its connection, which ends the lease, and
    the lender's thread can notice that before the assert runs.
    """
    got = []
    t = threading.Thread(target=lambda: got.append(self.take(timeout=timeout)), daemon=True)
    t.start()
    assert self.until(lambda: lender.lent or bool(got)) and lender.lent, 'the daemon must still know somebody wants it'
    t.join(3.0)
    assert got == [None]

  def bounce(self) -> bool:
    self.bounced += 1
    return True

  @staticmethod
  def until(predicate, timeout=3.0) -> bool:
    end = time.monotonic() + timeout
    while time.monotonic() < end:
      if predicate():
        return True
      time.sleep(0.01)
    return False


class Borrowing(LendingTest):
  def test_a_borrower_is_told_where_the_gadget_is(self):
    lender = self.lender()
    loan = self.take()
    assert loan is not None
    assert loan.udc == 'udc0'
    assert loan.mount == str(lending.gadget.FFS_MOUNT)
    assert lender.lent and lender.borrower == 'modeld'

  def test_nobody_listening_is_not_an_error(self):
    # the link was only just turned on, or the owner died. The caller asks
    # again later: only the owner ever holds ep0
    assert self.take(timeout=0.1) is None

  def test_the_daemon_is_told_to_get_off_the_endpoints_before_it_says_yes(self):
    # a borrow that lands while the daemon is mid-exchange: it hears about it
    # on the first ask, and answers once it has put the endpoints down
    self.free = False
    lender = self.lender()
    got = []
    t = threading.Thread(target=lambda: got.append(self.take(timeout=3.0)), daemon=True)
    t.start()
    assert self.until(lambda: lender.lent), 'the daemon was never told to let go'
    assert not got, 'lent the endpoints while they were still in use'
    self.free = True
    t.join(3.0)
    assert got and got[0] is not None

  def test_a_phones_loan_is_the_port_and_the_owner_vacates_it_first(self):
    # a hello over FunctionFS to a phone blocks 15 s: a phone's borrower is
    # never lent the endpoint files, whatever they are doing, and the owner
    # lets go of the port before it says yes
    self.cable_mode = True
    self.free = False
    self.udc = None
    lender = self.lender()
    loan = self.take(timeout=1.0)
    assert loan is not None and loan.cable
    assert self.vacated == 1 and lender.lent

  def test_a_borrow_nobody_can_answer_gives_up_and_says_so(self):
    self.free = False
    lender = self.lender()
    self.refused(lender)

  def test_the_connection_is_the_lease(self):
    # modeld is stopped at every ignition-off and SIGKILLed if it lingers;
    # dying is how it hands the link back
    lender = self.lender()
    loan = self.take()
    assert lender.lent
    loan.close()
    assert self.until(lambda: not lender.lent), 'the link never came back'

  def test_a_gadget_that_is_not_bound_yet_is_waited_for(self):
    self.udc = None
    lender = self.lender()
    self.refused(lender)
    self.udc = 'udc0'
    assert self.take(timeout=1.0) is not None


class TheCable(LendingTest):
  """A phone dials the comma; the borrower listens for it on the loan, and the
  endpoint files stay where they are. Nothing shares a socket."""

  def setUp(self):
    super().setUp()
    self.cable_mode = True

  def accepting(self, loan, timeout=3.0):
    """A borrower waiting for the phone, on its own thread; what it got lands in the list."""
    got = []

    def accept():
      try:
        got.append(loan.accept(timeout))
      except Exception as e:   # the test reads it
        got.append(e)
    t = threading.Thread(target=accept, daemon=True)
    t.start()
    assert self.until(lambda: loan.bound is not None), 'the loan never listened'
    return t, got

  def phone(self, loan) -> socket.socket:
    phone = socket.create_connection(loan.bound[:2], timeout=3.0)
    self.addCleanup(phone.close)
    return phone

  def test_the_borrower_takes_the_phones_dial_on_the_loan(self):
    lender = self.lender()
    loan = self.take()
    assert loan.cable and lender.lent and lender.borrower == 'modeld'
    t, got = self.accepting(loan)
    phone = self.phone(loan)
    t.join(3.0)
    sock = got[0]
    assert isinstance(sock, socket.socket), got
    self.addCleanup(sock.close)
    phone.sendall(b'hello')
    sock.settimeout(3.0)
    assert sock.recv(5) == b'hello'
    sock.sendall(b'ready')
    assert phone.recv(5) == b'ready'

  def test_the_next_dial_is_taken_on_the_same_listener(self):
    # a link lost mid-drive: the phone dials again, and nobody else is involved
    self.lender()
    loan = self.take()
    t, got = self.accepting(loan)
    bound = loan.bound
    self.phone(loan)
    t.join(3.0)
    got[0].close()
    t, got = self.accepting(loan)
    assert loan.bound == bound
    self.phone(loan)
    t.join(3.0)
    assert isinstance(got[0], socket.socket), got
    got[0].close()

  def test_the_newest_dial_wins(self):
    # the app restarted behind its own dead connection
    self.lender()
    loan = self.take()
    with self.assertRaises(TimeoutError):
      loan.accept(0.05)   # bound now, and nothing dialed
    first = self.phone(loan)
    second = self.phone(loan)
    time.sleep(0.05)   # both handshakes done on the loopback
    sock = loan.accept(1.0)
    self.addCleanup(sock.close)
    first.settimeout(3.0)
    assert first.recv(1) == b''
    sock.sendall(b'x')
    second.settimeout(3.0)
    assert second.recv(1) == b'x'

  def test_no_dial_in_time_is_a_timeout_and_the_listener_stays(self):
    self.lender()
    loan = self.take()
    with self.assertRaises(TimeoutError):
      loan.accept(0.1)
    assert loan.bound is not None
    self.phone(loan)   # still listening

  def test_an_address_still_held_is_tried_until_the_wait_runs_out(self):
    # the owner lets go of the port before it says yes, but a run before us
    # may still be exiting
    holder = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self.addCleanup(holder.close)
    holder.bind(('127.0.0.1', 0))
    holder.listen(1)
    with mock.patch.object(lending.gadget, 'CABLE_ADDR', holder.getsockname()):
      self.lender()
      loan = self.take()
      with self.assertRaises(OSError) as held:
        loan.accept(0.3)
      assert not isinstance(held.exception, TimeoutError)
      assert loan.bound is None
      holder.close()
      t, got = self.accepting(loan)
      self.phone(loan)
      t.join(3.0)
      assert isinstance(got[0], socket.socket), got
      got[0].close()

  def test_closing_the_loan_closes_its_listener(self):
    self.lender()
    loan = self.take()
    with self.assertRaises(TimeoutError):
      loan.accept(0.05)
    bound = loan.bound
    loan.close()
    assert loan.bound is None
    with self.assertRaises(OSError):
      socket.create_connection(bound[:2], timeout=1.0)

  def test_a_usb_loan_has_no_listener(self):
    self.cable_mode = False
    self.lender()
    loan = self.take()
    assert not loan.cable
    with self.assertRaises(RuntimeError):
      loan.accept(0.1)

  def test_the_owner_vacates_the_port_before_it_says_yes(self):
    # the owner was listening and holding the phone's dial; both go, and the
    # borrower binds the same address. The phone, hung up on, dials again
    listener = self.listener()
    self.dial(listener)
    bound = listener.bound[:2]
    with mock.patch.object(lending.gadget, 'CABLE_ADDR', bound):
      self.lender(vacate=listener.vacate)
      loan = self.take()
      assert not listener.listening, 'the owner still listens'   # what vacate does is the listener's test
      t, got = self.accepting(loan)
      assert loan.bound[:2] == bound, 'the borrower did not get the same address'
      self.phone(loan)
      t.join(3.0)
      assert isinstance(got[0], socket.socket), got
      got[0].close()


class TheListener(LendingTest):
  def test_a_newer_dial_replaces_an_older_one(self):
    listener = self.listener()
    first = self.dial(listener)
    self.dial(listener)
    assert listener.held
    assert first.recv(1) == b''

  def test_a_phone_that_hung_up_is_dropped(self):
    listener = self.listener()
    phone = self.dial(listener)
    phone.close()
    assert self.until(lambda: listener.poll() is None and not listener.held)

  def test_vacating_frees_the_port_and_expects_the_phone_back(self):
    listener = self.listener()
    phone = self.dial(listener)
    bound = listener.bound[:2]
    listener.vacate()
    assert not listener.listening and not listener.held and listener.redial_expected
    phone.settimeout(3.0)
    assert phone.recv(1) == b''
    taken = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    taken.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    taken.bind(bound)   # the port is free
    taken.close()
    # and the owner listens again on its next step, not after the bind backoff
    assert listener.open() and listener.listening
    # with nothing held there is nothing to expect back
    listener.vacate()
    assert not listener.redial_expected

  def test_nothing_waiting_is_nothing(self):
    listener = self.listener()
    assert listener.poll() is None and not listener.held
    listener.release(expect_redial=True)   # nothing held is not an error, and nothing to expect
    assert not listener.redial_expected
    assert listener.news is False
    self.dial(listener)
    assert listener.news

  def test_an_address_that_is_not_ours_yet_is_tried_again_later(self):
    # usb0 has no address until jetlink-root.sh net has run
    listener = lending.CableListener()
    self.addCleanup(listener.close)
    with mock.patch.object(lending.gadget, 'CABLE_ADDR', ('192.0.2.1', 0)):
      assert not listener.open()
      assert not listener.listening
      assert listener.next_open > time.monotonic()
    assert not listener.open(), 'retried inside the backoff'
    listener.next_open = 0.0
    assert listener.open() and listener.listening


class Bouncing(LendingTest):
  def test_a_stuck_write_reaches_the_owner(self):
    # unbinding is the only thing that dequeues a FunctionFS write nobody is
    # reading, and the unbind belongs to whoever holds ep0
    self.lender()
    loan = self.take()
    assert loan.bounce() is True
    assert self.bounced == 1

  def test_a_bounce_after_the_loan_is_over_is_not_an_error(self):
    self.lender()
    loan = self.take()
    loan.close()
    assert loan.closed and loan.bounce() is False
    assert self.bounced == 0


class TellingTheOwner(LendingTest):
  """What a borrower passes on of the server's hello, over the loan it already
  holds: the owner never speaks the protocol, and sleep_after is how it knows
  whether letting go of the gadget lets the Jetson sleep."""

  HELLO = {'protocol': 3, 'device': 'orin', 'backend': 'tensorrt', 'runtime_version': '10.16', 'sleep_after': 0.0,
           'engine_state': 'ready', 'loaded': 'a' * 64, 'cached_models': ['a' * 64], 'telemetry': {'gpu_c': 50}}

  def test_the_hello_reaches_the_owner_and_the_loan_carries_on(self):
    heard = []
    self.lender(server=lambda who, fields: heard.append((who, fields)))
    loan = self.take(name='modeld')
    assert loan.note_server(self.HELLO) is True
    assert heard == [('modeld', {'protocol': 3, 'device': 'orin', 'backend': 'tensorrt', 'runtime_version': '10.16',
                                 'sleep_after': 0.0})]
    # the answer was read, so the next exchange on the loan gets its own
    assert loan.bounce() is True and self.bounced == 1

  def test_an_older_owner_answering_unknown_op_is_ignored(self):
    lender = self.lender()   # no `server`: answered as an owner from before the op
    loan = self.take()
    assert loan.note_server(self.HELLO) is False
    assert not loan.closed and lender.lent
    assert loan.bounce() is True and self.bounced == 1

  def test_a_note_the_owner_cannot_record_keeps_the_lease(self):
    # a raise on the lender's thread would end the loan under a borrower
    # that is using the endpoints
    def broken(who, fields):
      raise RuntimeError('boom')
    lender = self.lender(server=broken)
    loan = self.take()
    with mock.patch.object(lending.gadget, 'log'):
      assert loan.note_server(self.HELLO) is True
    assert lender.lent and not loan.closed
    assert loan.bounce() is True

  def test_an_owner_that_is_gone_is_not_an_error(self):
    lender = self.lender(server=lambda who, fields: None)
    loan = self.take()
    lender.stop()
    with mock.patch.object(lending.gadget, 'log') as log:
      assert loan.note_server(self.HELLO) is False
    log.warning.assert_called_once()

  def test_a_late_answer_lets_the_loan_go_and_never_answers_a_later_request(self):
    # the exchange has no ids: read later, {'ok': True} would be a renewal's
    # "lend" (and its missing mount a KeyError) or a bounce's answer
    release = threading.Event()
    self.addCleanup(release.set)
    lender = self.lender(server=lambda who, fields: release.wait(2.0))
    loan = self.take()
    with mock.patch.object(lending, 'NOTE_TIMEOUT', 0.2), mock.patch.object(lending.gadget, 'log') as log:
      assert loan.note_server(self.HELLO) is False
    assert loan.closed
    assert 'letting the loan go' in log.warning.call_args.args[0]
    assert loan.bounce() is False
    release.set()
    assert self.until(lambda: not lender.lent), 'the owner never saw the lease end'
    again = self.take()
    assert again is not None and again.bounce() is True

  def test_an_answer_that_is_not_one_lets_the_loan_go(self):
    # a fake owner that answers the note with a line that is not an answer
    srv = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    srv.bind(str(self.path))
    srv.listen(1)
    self.addCleanup(srv.close)

    def owner():
      conn, _ = srv.accept()
      with conn:
        f = conn.makefile('rwb')
        f.readline()                                   # the borrow
        f.write(b'{"ok": true, "udc": "udc0", "mount": "/m"}\n')
        f.flush()
        f.readline()                                   # the note
        f.write(b'[1, 2]\n')
        f.flush()
        f.readline()                                   # nothing more comes: the loan went
    t = threading.Thread(target=owner, daemon=True)
    t.start()
    loan = self.take()
    with mock.patch.object(lending.gadget, 'log'):
      assert loan.note_server(self.HELLO) is False
    assert loan.closed
    t.join(3.0)
    assert not t.is_alive()

  def test_a_closed_loan_says_nothing(self):
    self.lender(server=lambda who, fields: None)
    loan = self.take()
    loan.close()
    assert loan.note_server(self.HELLO) is False


class StaleSockets(LendingTest):
  def test_a_socket_a_dead_daemon_left_is_cleared(self):
    self.path.write_text('')          # anything at the address stops bind()
    lender = self.lender()
    assert lender.listening
    assert self.take() is not None
    assert lender.lent

  def test_a_live_daemon_keeps_its_socket(self):
    # two owners is a misconfiguration, and the second must not take the
    # gadget away from the one that owns it, even when it stops
    first = self.lender()
    second = lending.Lender(lambda: self.free, self.bounce, path=self.path)
    assert second.start() is False and not second.listening
    assert second.error
    second.stop()
    assert self.path.exists()
    # macOS refuses a connect while the second's probe still fills the backlog
    # of one; Linux queues it
    assert self.until(lambda: self.take() is not None)
    assert first.lent and not second.lent

