"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

The comma's gadget: what carries the link, the gadget built and brought up
through the root script, the wait for a host, and the files the owner and the
root script leave in /dev/shm. The settings it is told are
jetlink.openpilot.settings' (tests/openpilot/test_interface.py).
"""
import json
import os
import subprocess
import sys
import tempfile
import unittest
import unittest.mock
from pathlib import Path

from jetlink.comma import gadget, root

REPO = Path(__file__).resolve().parents[1]
# what the gadget owner must never end up importing. swaglog pulls numpy, capnp
# and zmq in to publish a log line and costs 28 MB; params imports swaglog.
# Measured on the comma: the owner plus the transport is 10.4 MB, against
# 47.5 MB for the daemon that imported the world
HEAVY = ('numpy', 'capnp', 'zmq', 'cereal', 'openpilot')


class TestNothingHeavyIsReachable(unittest.TestCase):
  """The owner is only small while this holds, so it is a test and not a note."""

  def test_the_owner_and_the_transport_it_opens_stay_out_of_the_heavy_half(self):
    # the owner brings the gadget, the lease, the port and root with it; the
    # FunctionFS transport it opens is imported by name, since the owner
    # imports it lazily
    roots = 'sorted({m.split(".")[0] for m in sys.modules})'
    code = f'import sys, json, jetlink.comma.owner, jetlink.transport.ffs; print(json.dumps({roots}))'
    env = {**os.environ, 'PYTHONPATH': str(REPO)}
    out = subprocess.run([sys.executable, '-c', code], capture_output=True, text=True,
                         env=env, cwd=str(REPO), timeout=120)
    self.assertEqual(out.returncode, 0, out.stderr)
    found = set(json.loads(out.stdout))
    self.assertEqual(sorted(found & set(HEAVY)), [],
                     'everything the owner imports runs for the whole drive; see the gadget docstring')


class TestLinkKind(unittest.TestCase):
  """What carries the link: the gadget the owner built, 'cable' for iOS and
  'usb' otherwise, with the caller's setting standing in until the owner has said."""

  def setUp(self):
    self.tmp = Path(tempfile.mkdtemp())
    for name, value in (('LINK', self.tmp / 'link'), ('UDC_PATH', self.tmp / 'udc'),
                        ('NET_STATUS', self.tmp / 'net')):
      p = unittest.mock.patch.object(gadget, name, value)
      self.addCleanup(p.stop)
      p.start()

  def test_nothing_recorded_is_usb(self):
    self.assertEqual(gadget.link_kind(), 'usb')
    self.assertIsNone(gadget.link_peer())

  def test_ios_is_the_cable_before_any_dial(self):
    self.assertEqual(gadget.link_kind('ios'), 'cable')
    self.assertIsNone(gadget.link_peer())

  def test_the_owners_record_decides_over_the_setting(self):
    # a setting moved while somebody borrowed waits for the car to park; until
    # the owner rebuilds, the gadget is what it built
    gadget.note_link('usb')
    self.assertEqual(gadget.link_kind('ios'), 'usb')
    gadget.note_link('cable', '192.168.60.3')
    self.assertEqual(gadget.link_kind('usb'), 'cable')
    gadget.clear_link()
    self.assertEqual(gadget.link_kind('usb'), 'usb', 'no owner yet: the setting')
    self.assertEqual(gadget.link_kind('ios'), 'cable', 'no owner yet: the setting')

  def test_a_caller_that_read_the_setting_hands_it_over(self):
    # jetlink.openpilot reads the setting once, off the directory the fork's
    # adapter names, and passes it; this module knows no param
    self.assertEqual(gadget.link_kind('ios'), 'cable')
    self.assertEqual(gadget.link_kind('usb'), 'usb')
    self.assertEqual(gadget.link_kind('off'), 'usb')
    gadget.note_link('usb')
    self.assertEqual(gadget.link_kind('ios'), 'usb', "the owner's record still decides")

  def test_the_cc_pin_is_one_reading_for_every_reader(self):
    cc = self.tmp / 'cc'
    with unittest.mock.patch.object(gadget, 'CC_ORIENTATION', cc):
      self.assertIsNone(gadget.cc_orientation(), 'a kernel that does not say')
      self.assertFalse(gadget.port_has_host())
      for raw, value in (('0\n', 0), ('1', 1), ('2\n', 2), ('junk', None)):
        cc.write_text(raw)
        self.assertEqual(gadget.cc_orientation(), value, raw)
        self.assertEqual(gadget.port_has_host(), bool(value), raw)

  def test_a_dial_is_recorded_with_the_phone_and_cleared(self):
    gadget.note_link('cable', '192.168.60.3')
    self.assertEqual(gadget.link_peer(), '192.168.60.3')
    gadget.clear_link()
    self.assertIsNone(gadget.link_peer())
    gadget.clear_link()   # twice is not an error

  def test_on_the_cable_there_is_no_host_to_wait_for(self):
    # the connect already reached the phone; the UDC is configured by it, and
    # it is the dial that proved it
    with unittest.mock.patch.object(gadget, 'udc_state', return_value='powered'), \
         unittest.mock.patch.object(gadget.time, 'sleep', side_effect=AssertionError('waited')):
      self.assertTrue(gadget.wait_for_host(5.0, report=lambda: self.fail('reported a wait'), mode='ios'))
      self.assertFalse(gadget.wait_for_host(0.0, mode='usb'))
      self.assertFalse(gadget.wait_for_host(0.0))

  def test_the_cable_needs_the_gadget_too(self):
    # a phone on the cable is on the gadget's own network interface
    with unittest.mock.patch.object(gadget, 'FFS_MOUNT', self.tmp / 'ffs'), \
         unittest.mock.patch.object(gadget, 'GADGET_STATUS', self.tmp / 'status'):
      gadget.note_link('cable', '192.168.60.3')
      self.assertFalse(gadget.link_configured())
      (self.tmp / 'ffs').mkdir()
      (self.tmp / 'ffs' / 'ep0').touch()
      self.assertTrue(gadget.link_configured())

  def test_the_bus_speed_is_read_off_the_bound_udc(self):
    with unittest.mock.patch.object(gadget, 'bound_udc', return_value=None):
      self.assertIsNone(gadget.usb_speed())
    with unittest.mock.patch.object(gadget, 'bound_udc', return_value='a600000.dwc3'):
      self.assertIsNone(gadget.usb_speed())
      (gadget.UDC_PATH / 'a600000.dwc3').mkdir(parents=True)
      (gadget.UDC_PATH / 'a600000.dwc3' / 'current_speed').write_text('super-speed\n')
      self.assertEqual(gadget.usb_speed(), 'super-speed')

  def test_the_network_status_is_the_scripts(self):
    self.assertIsNone(gadget.net_status())
    gadget.NET_STATUS.write_text('ok 192.168.60.1 usb1\n')
    self.assertEqual(gadget.net_status(), 'ok 192.168.60.1 usb1')


class TestTheTwoGadgets(unittest.TestCase):
  """The setting picks the gadget: the plain one for USB, the composite one
  with a network interface for iOS."""

  def test_setup_passes_the_setting(self):
    with unittest.mock.patch.object(root, 'run', return_value=True) as run, \
         unittest.mock.patch.object(gadget, 'link_configured', return_value=True):
      self.assertTrue(gadget.setup_gadget(True))
      run.assert_called_with('gadget', '--ios')
      self.assertTrue(gadget.setup_gadget(False))
      run.assert_called_with('gadget')

  def test_the_built_gadget_is_read_from_its_config(self):
    tmp = Path(tempfile.mkdtemp())
    config = tmp / 'configs' / 'c.1'
    config.mkdir(parents=True)
    (config / 'ffs.jetlink').touch()
    with unittest.mock.patch.object(gadget, 'GADGET_PATH', tmp):
      self.assertFalse(gadget.built_for_ios())
      (config / 'ncm.usb0').symlink_to(tmp / 'functions' / 'ncm.usb0')
      self.assertTrue(gadget.built_for_ios())


  def test_another_bound_gadget_is_named(self):
    configfs = Path(tempfile.mkdtemp())
    ours, adb = configfs / 'jetlink', configfs / 'g1'
    for g in (ours, adb):
      g.mkdir()
      (g / 'UDC').write_text('\n')
    with unittest.mock.patch.object(gadget, 'GADGET_PATH', ours):
      self.assertIsNone(gadget.other_gadget())
      (ours / 'UDC').write_text('a600000.dwc3\n')
      self.assertIsNone(gadget.other_gadget())
      (adb / 'UDC').write_text('a600000.dwc3\n')
      self.assertEqual(gadget.other_gadget(), 'g1')

class TestGadgetSetup(unittest.TestCase):
  """The owner creates the gadget, and brings its network up after a bind,
  through the root script."""

  def setUp(self):
    self.tmp = Path(tempfile.mkdtemp())

  def test_a_failed_setup_is_a_false_not_a_raise(self):
    # the script has already written the reason to the status file
    with unittest.mock.patch.object(root, 'run', return_value=False), \
         unittest.mock.patch.object(gadget, 'link_configured', side_effect=AssertionError('looked')):
      self.assertFalse(gadget.setup_gadget(False))

  def test_the_network_is_brought_up_by_the_same_script_and_judged_by_its_status(self):
    # the netdev exists only once the UDC is bound, so this runs after the
    # owner's bind rather than with the gadget
    status = self.tmp / 'net'
    with unittest.mock.patch.object(root, 'run', return_value=True) as run, \
         unittest.mock.patch.object(gadget, 'NET_STATUS', status):
      self.assertFalse(gadget.net_up())          # the script wrote nothing
      run.assert_called_once_with('net')
      status.write_text('error: no netdev yet; it appears when the owner binds the UDC (then run net)\n')
      self.assertFalse(gadget.net_up())
      status.write_text('ok 192.168.60.1 usb1\n')
      self.assertTrue(gadget.net_up())
    with unittest.mock.patch.object(root, 'run', return_value=False), \
         unittest.mock.patch.object(gadget, 'NET_STATUS', status):
      self.assertFalse(gadget.net_up())


class TestTheLogger(unittest.TestCase):
  def test_the_root_scripts_failures_go_where_the_gadgets_lines_do(self):
    # the owner logs to its own file and the fork's processes to cloudlog;
    # a sudo that failed must land in the same place
    for module in (gadget, root):
      p = unittest.mock.patch.object(module, 'log', module.log)
      self.addCleanup(p.stop)
      p.start()
    logger = unittest.mock.Mock()
    gadget.set_logger(logger)
    self.assertIs(gadget.log, logger)
    self.assertIs(root.log, logger)


class FakeClock:
  """monotonic and sleep, so a 45 s wait costs no wall clock."""

  def __init__(self, now: float = 1000.0):
    self.now = now
    self.slept = 0.0

  def monotonic(self) -> float:
    return self.now

  def sleep(self, seconds: float) -> None:
    step = max(seconds, 0.01)
    self.now += step
    self.slept += step


class TestWaitForHost(unittest.TestCase):
  """Waiting for the Jetson to enumerate the gadget, which stays bound: every
  unbind is an unplug the far end has to recover from, and the one case that
  needs an edge is a bus that stalled half enumerated."""

  # the fork's backend.CONNECT_TIMEOUT, what modeld's join waits
  CONNECT_TIMEOUT = 45.0

  def setUp(self):
    self.clock = FakeClock()
    self.bounced = []
    # USB: no owner's record, and no setting handed over
    for name, value in (('time', self.clock), ('LINK', Path(tempfile.mkdtemp()) / 'link')):
      p = unittest.mock.patch.object(gadget, name, value)
      self.addCleanup(p.stop)
      p.start()

  def bus(self, udc: str, cc: bool = True) -> None:
    for name, value in (('udc_state', udc), ('port_has_host', cc)):
      p = unittest.mock.patch.object(gadget, name, return_value=value)
      self.addCleanup(p.stop)
      p.start()

  def wait(self, seconds: float = CONNECT_TIMEOUT) -> bool:
    return gadget.wait_for_host(seconds, bounce=lambda: self.bounced.append(True))

  def test_a_host_that_is_already_there_is_not_waited_for(self):
    self.bus('configured')
    self.assertIs(self.wait(), True)
    self.assertEqual(self.clock.slept, 0.0)

  def test_a_bus_that_stalls_half_enumerated_is_bounced_once(self):
    # A jetson whose hubs are not armed for remote wakeup answers the bind
    # with a bus reset and stops there; only another connect moves it.
    self.bus('default')
    self.assertIs(self.wait(), False)
    self.assertEqual(len(self.bounced), 1)

  def test_the_bounce_waits_out_a_normal_enumeration(self):
    self.bus('addressed')
    self.wait(gadget.STALLED_ENUMERATION / 2)
    self.assertEqual(self.bounced, [])

  def test_nothing_on_the_cable_is_not_a_stall(self):
    # No host on the CC pin: there is nobody to enumerate us and bouncing the
    # gadget would only cost the next one its bind.
    self.bus('not attached', cc=False)
    self.assertIs(self.wait(), False)
    self.assertEqual(self.bounced, [])

  def test_a_jetson_still_booting_is_left_alone(self):
    # Powered but not yet driving the bus: the UDC never leaves powered.
    self.bus('powered')
    self.assertIs(self.wait(), False)
    self.assertEqual(self.bounced, [])

  def test_a_bounce_that_fails_does_not_end_the_wait(self):
    self.bus('default')
    self.assertIs(gadget.wait_for_host(self.CONNECT_TIMEOUT,
                                       bounce=unittest.mock.Mock(side_effect=OSError('no such device'))), False)

  def test_a_host_that_turns_up_late_is_still_joined(self):
    states = ['powered'] * 3 + ['configured']
    with unittest.mock.patch.object(gadget, 'udc_state', side_effect=lambda: states.pop(0) if states else 'configured'), \
         unittest.mock.patch.object(gadget, 'port_has_host', return_value=True):
      self.assertIs(self.wait(), True)

  def test_a_caller_that_is_going_away_is_not_kept_waiting(self):
    self.bus('powered')
    self.assertIs(gadget.wait_for_host(self.CONNECT_TIMEOUT, should_stop=lambda: True), False)
    self.assertEqual(self.clock.slept, 0.0)

  def test_the_wait_says_once_that_it_is_waiting(self):
    self.bus('powered')
    said = []
    gadget.wait_for_host(2.0, report=lambda: said.append(True))
    self.assertEqual(said, [True])


class TestGadgetStatus(unittest.TestCase):
  """The gadget is set up by root, from the owner through jetlink-root.sh. This
  file is the only way the reason for a failure reaches anything a user can see."""

  def setUp(self):
    tmp = Path(tempfile.mkdtemp())
    self.status = tmp / 'jetlink-gadget'
    for name, value in (('GADGET_STATUS', self.status), ('LENDER_STATUS', tmp / 'jetlink-lender')):
      p = unittest.mock.patch.object(gadget, name, value)
      self.addCleanup(p.stop)
      p.start()

  def test_missing_file_is_not_an_error(self):
    # A build that never ran the setup at all reads the same as not installed.
    self.assertIsNone(gadget.gadget_error())

  def test_ok_is_not_an_error(self):
    self.status.write_text('ok\n')
    self.assertIsNone(gadget.gadget_error())

  def test_empty_is_not_an_error(self):
    self.status.write_text('')
    self.assertIsNone(gadget.gadget_error())

  def test_reason_is_unwrapped(self):
    self.status.write_text('error: kernel has no USB gadget support\n')
    self.assertEqual(gadget.gadget_error(), 'kernel has no USB gadget support')

  def test_bare_reason_survives(self):
    self.status.write_text('something went wrong')
    self.assertEqual(gadget.gadget_error(), 'something went wrong')

  def test_unreadable_status_is_not_an_error(self):
    # Path.exists() and read_text() raise rather than return on a root-only
    # path; an availability check must never take a process down over one.
    with unittest.mock.patch.object(Path, 'read_text', side_effect=PermissionError):
      self.assertIsNone(gadget.gadget_error())

  def test_a_lender_that_cannot_listen_is_an_error_but_not_a_build_failure(self):
    # the owner's own record, beside root's: the gadget is there, and nothing
    # can borrow it
    self.status.write_text('ok\n')
    gadget.note_lender_error('[Errno 30] Read-only file system')
    self.assertEqual(gadget.gadget_error(), 'the lender could not listen: [Errno 30] Read-only file system')
    self.assertIsNone(gadget.build_error())
    self.status.write_text('error: kernel has no USB gadget support\n')
    self.assertEqual(gadget.gadget_error(), 'kernel has no USB gadget support', 'the build failure comes first')
    gadget.note_lender_error(None)
    gadget.note_lender_error(None)   # twice is not an error
    self.status.write_text('ok\n')
    self.assertIsNone(gadget.gadget_error())


class TestDormant(unittest.TestCase):
  """The owner's marker while it has let the gadget go on purpose, and
  hardwared's request to power the Jetson off."""

  def setUp(self):
    self.tmp = Path(tempfile.mkdtemp())
    for name in ('DORMANT', 'SHUTDOWN_REQUEST'):
      p = unittest.mock.patch.object(gadget, name, self.tmp / name.lower())
      self.addCleanup(p.stop)
      p.start()

  def test_marker_from_a_live_process_counts(self):
    gadget.set_dormant(True)
    self.assertTrue(gadget.dormant())
    gadget.set_dormant(False)
    self.assertFalse(gadget.dormant())

  def test_marker_from_a_dead_process_is_a_leftover(self):
    gadget.DORMANT.write_text('4194304')  # above pid_max
    self.assertFalse(gadget.dormant())

  def test_garbage_is_not_dormant(self):
    gadget.DORMANT.write_text('not a pid')
    self.assertFalse(gadget.dormant())

  def test_shutdown_request_round_trip(self):
    self.assertIsNone(gadget.pending_shutdown())
    self.assertTrue(gadget.request_shutdown('car battery'))
    self.assertEqual(gadget.pending_shutdown(), 'car battery')
    gadget.finish_shutdown()
    self.assertIsNone(gadget.pending_shutdown())
