"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

The comma's USB-C port: when it is held at sink, when it is let go, and what it
leaves alone. A USB-A host and a chestnut must see no change at all; a C-to-C
host that lost the toss gets one hold, and only for as long as it is plugged
in; one that powers the comma and still came out the device is asked over USB
PD to host, a few times a plug. And a sink that is the device has its device
side on, whatever the charger detection made of the far end. With phone
charging turned on, an iPhone that powers the comma over USB PD is asked to
charge from it instead, once a plug, unless the comma is hot, and handed the
source role back if the link goes down while it charges.
"""
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from jetlink.comma import port, root
from tests.openpilot.fakes import CHESTNUT_IDS, PORT_FILES

SWAP = port.SWAP_AFTER
RELEASE = port.RELEASE_AFTER
RETRY = port.RETRY_AFTER
TRIES = port.SWAP_TRIES
# the data role each power role starts a plug with: the source is the host
DATA = {'source': 'dfp', 'sink': 'ufp', 'none': 'none'}
CHESTNUT = (0xADD1, 0x0001)
CHESTNUT_ROM = (0x174C, 0x2464)
IPHONE = (0x05AC, 0x12A8)
JETSON_GADGET = (0x0955, 0x7020)


class PortTest(unittest.TestCase):
  def setUp(self):
    tmp = tempfile.TemporaryDirectory()
    self.addCleanup(tmp.cleanup)
    self.tmp = Path(tmp.name)
    self.role, self.data, self.mode, self.devices = (self.tmp / PORT_FILES[name] for name in
                                                      ('POWER_ROLE', 'DATA_ROLE', 'UDC_MODE', 'USB_DEVICES'))
    self.devices.mkdir()
    self.plug('none')
    self.mode.write_text('none\n')   # the device side, off with nothing plugged in
    self.script = mock.Mock(return_value=True)
    self.udc = mock.Mock(side_effect=self.turn_device_side)
    patchers = [mock.patch.object(port, name, self.tmp / f) for name, f in PORT_FILES.items()]
    patchers += [mock.patch.object(port, 'run_script', self.script),
                 mock.patch.object(port, 'run_udc', self.udc),
                 mock.patch.object(port.gadget, 'log')]
    for p in patchers:
      self.addCleanup(p.stop)
      p.start()
    self.log = port.gadget.log
    # openpilot's chestnut ids, as the fork's adapter hands them over
    self.port = port.Port(CHESTNUT_IDS)
    self.now = 100.0
    # what is always there: the root hub, and the modem on the other controller
    self.enumerate('usb1', (0x1D6B, 0x0002))
    self.enumerate('1-1', (0x2C7C, 0x6007))
    self.enumerate('1-1:1.0', None)

  def turn_device_side(self, command: str) -> bool:
    """jetlink-root.sh udc start|stop: the glue's mode, as on the comma."""
    self.mode.write_text({'start': 'peripheral', 'stop': 'none'}[command] + '\n')
    return True

  def plug(self, role: str, data: str | None = None) -> None:
    """The policy engine's roles; the data role is the one the power role
    starts a plug with unless named."""
    self.role.write_text(role + '\n')
    self.data.write_text((data or DATA[role]) + '\n')

  def enumerate(self, name: str, ids: tuple[int, int] | None) -> None:
    """An entry in the fake /sys/bus/usb/devices; an interface has no ids."""
    d = self.devices / name
    d.mkdir()
    if ids is not None:
      (d / 'idVendor').write_text(f'{ids[0]:04x}\n')
      (d / 'idProduct').write_text(f'{ids[1]:04x}\n')

  # the phone charging param, as the owner reads it every step
  charge = False

  def run_for(self, seconds: float, configured: bool = False, ios: bool = False) -> None:
    end = self.now + seconds
    while self.now < end:
      self.port.update(now=self.now, configured=configured, charge=ios and self.charge)
      self.now += 0.5

  def commands(self, script: mock.Mock | None = None) -> list[str]:
    """What the port asked jetlink-root.sh port (or `script`) to do, in order."""
    return [c.args[0] for c in (script or self.script).call_args_list]


class TestAfterAnOwnerDied(PortTest):
  """An owner started after one that died can find a borrower still on the
  gadget the dead one presented, maybe through a hold it made. The first
  update leaves the port alone under that link, and clears it once it goes."""

  def test_a_live_link_keeps_the_port_as_it_is_until_it_goes(self):
    self.plug('sink')
    with mock.patch.object(port.gadget, 'host_attached', return_value=True):
      self.run_for(3)
    self.assertEqual(self.commands(), [])
    self.log.warning.assert_called_once()
    with mock.patch.object(port.gadget, 'host_attached', return_value=False):
      self.run_for(3)
    self.assertEqual(self.commands(), ['off'])

  def test_a_boot_with_nothing_on_the_gadget_clears_at_once(self):
    with mock.patch.object(port.gadget, 'host_attached', return_value=False):
      self.run_for(1)
    self.assertEqual(self.commands(), ['off'])


class TestHosts(PortTest):
  def test_a_usb_a_host_changes_nothing(self):
    self.plug('sink')
    self.run_for(60)
    self.assertEqual(self.commands(), ['off'], "the comma is already the device on an A-to-C cable")

  def test_a_mac_that_lost_the_toss_is_made_the_host(self):
    self.plug('source')
    self.run_for(SWAP - 0.5)
    self.assertEqual(self.commands(), ['off'], "a chestnut gets its chance to enumerate first")
    self.run_for(1)
    self.assertEqual(self.commands(), ['off', 'hold'])
    # the hold is an unplug and replug, and the Mac comes back as the source
    self.plug('none')
    self.run_for(1)
    self.plug('sink')
    self.run_for(60)
    self.assertEqual(self.commands(), ['off', 'hold'], "held for as long as the host stays plugged in")

  def came_up_as_the_device(self, ids: tuple[int, int]) -> None:
    self.plug('source')
    self.enumerate('2-1', ids)
    self.run_for(SWAP + 1)
    self.assertEqual(self.commands(), ['off', 'hold'])

  def test_an_iphone_that_came_up_as_the_device_is_made_the_host(self):
    self.came_up_as_the_device(IPHONE)

  def test_a_jetsons_usb_c_port_that_came_up_as_the_device_is_made_the_host(self):
    self.came_up_as_the_device(JETSON_GADGET)

  def test_unplugging_the_host_lets_the_port_go(self):
    self.plug('source')
    self.run_for(SWAP + 1)
    self.plug('sink')
    self.run_for(10)
    self.plug('none')
    self.run_for(RELEASE - 0.5)
    self.assertEqual(self.commands(), ['off', 'hold'], "a replug is not an unplug")
    self.run_for(1)
    self.assertEqual(self.commands(), ['off', 'hold', 'off'])
    self.assertFalse(self.port.held)

  def test_the_next_mac_gets_the_same_treatment(self):
    for _ in range(2):
      self.plug('source')
      self.run_for(SWAP + 1)
      self.plug('sink')
      self.run_for(5)
      self.plug('none')
      self.run_for(RELEASE + 1)
    self.assertEqual(self.commands(), ['off', 'hold', 'off', 'hold', 'off'])


class TestPoweredByTheDevice(PortTest):
  """The far end came out the device, then took the source role with a PR_Swap,
  which leaves the comma the sink and still the host. No hold can change that;
  a DR_Swap or a hard reset can."""

  ASKED = ['off'] + ['device'] * TRIES + ['reset']

  def took_the_power_role(self) -> None:
    self.plug('source')
    self.run_for(1)
    self.plug('sink', 'dfp')

  def test_it_is_asked_to_take_the_host_role(self):
    self.took_the_power_role()
    self.run_for(SWAP - 0.5)
    self.assertEqual(self.commands(), ['off'], "it gets its chance to swap by itself first")
    self.run_for(1)
    self.assertEqual(self.commands(), ['off', 'device'])
    self.plug('sink', 'ufp')   # it took it
    self.run_for(60)
    self.assertEqual(self.commands(), ['off', 'device'])

  def test_a_refusal_is_asked_again_then_reset_then_left(self):
    self.took_the_power_role()
    self.run_for(SWAP + RETRY * (TRIES + 1) + 0.5)
    self.assertEqual(self.commands(), self.ASKED)
    self.run_for(60)
    self.assertEqual(self.commands(), self.ASKED, "a plug is asked a bounded number of times")
    gave_up = [c for c in self.log.warning.call_args_list if 'until the next plug' in c.args[0]]
    self.assertEqual(len(gave_up), 1)
    self.assertIn('hub', gave_up[0].args[0])

  def test_a_reset_that_worked_ends_it(self):
    self.took_the_power_role()
    self.run_for(SWAP + RETRY * TRIES + 0.5)
    self.assertEqual(self.commands()[-1], 'reset')
    self.plug('sink', 'ufp')   # by the spec a sink is the device after a hard reset
    self.run_for(60)
    self.assertEqual(self.commands(), self.ASKED)

  def test_the_next_plug_is_asked_afresh(self):
    self.took_the_power_role()
    self.run_for(60)
    self.plug('none')
    self.run_for(port.UNPLUGGED + 0.5)
    self.took_the_power_role()
    self.run_for(SWAP + 0.5)
    self.assertEqual(self.commands(), self.ASKED + ['device'])

  def test_a_flicker_is_not_a_new_plug(self):
    self.took_the_power_role()
    self.run_for(60)
    # a hard reset drops VBUS for a moment, and the role can read none
    self.plug('none')
    self.run_for(0.5)
    self.plug('sink', 'dfp')
    self.run_for(60)
    self.assertEqual(self.commands(), self.ASKED)

  def test_a_chestnut_that_powers_us_is_left_alone(self):
    self.enumerate('2-1', CHESTNUT)
    self.plug('sink', 'dfp')
    self.run_for(60)
    self.assertEqual(self.commands(), ['off'])

  def test_the_device_end_is_never_asked(self):
    for role in ('sink', 'source'):
      self.plug(role, 'ufp')
      self.run_for(1)
    self.assertNotIn('device', self.commands())
    self.assertNotIn('reset', self.commands())


class TestTheRecord(PortTest):
  def test_each_change_of_roles_is_one_line(self):
    (self.tmp / 'contract').write_text('explicit\n')
    (self.tmp / 'typec_mode').write_text('Source attached (default current)\n')
    (self.tmp / 'real_type').write_text('USB_FLOAT\n')
    self.run_for(5)
    self.plug('source')
    self.run_for(1)
    self.plug('sink', 'dfp')
    self.run_for(1)
    self.plug('sink', 'ufp')
    self.run_for(5)
    lines = [c.args[0] % c.args[1:] for c in self.log.info.call_args_list]
    self.assertEqual(len(lines), 4, lines)
    self.assertEqual(lines[0], "jetlink: USB-C port none, no data role; USB PD contract explicit; "
                               "Type-C Source attached (default current); charger detection USB_FLOAT")
    self.assertTrue(lines[2].startswith("jetlink: USB-C port sink, host;"), lines[2])
    self.assertTrue(lines[3].startswith("jetlink: USB-C port sink, device;"), lines[3])

  def test_no_policy_engine_says_nothing(self):
    self.role.unlink()
    self.run_for(5)
    self.log.info.assert_not_called()


class TestTheDeviceSide(PortTest):
  """A host that powers the port, with the comma its device: the device side
  must be on, whatever the charger detection read, and off once the port is
  empty."""

  def test_a_device_side_already_on_is_left(self):
    self.plug('sink')
    self.mode.write_text('peripheral\n')   # the policy engine turned it on
    self.run_for(60)
    self.assertEqual(self.commands(self.udc), [])

  def test_one_still_off_is_turned_on_and_off_once_the_port_is_empty(self):
    self.plug('sink')
    self.run_for(SWAP - 0.5)
    self.assertEqual(self.commands(self.udc), [], "the policy engine gets its chance first")
    self.run_for(60)
    self.assertEqual(self.commands(self.udc), ['start'])
    self.plug('none')
    self.run_for(port.UNPLUGGED - 0.5)
    self.assertEqual(self.commands(self.udc), ['start'], "a flicker is not an unplug")
    self.run_for(60)
    self.assertEqual(self.commands(self.udc), ['start', 'stop'])

  def test_one_that_goes_off_again_is_turned_on_again_no_sooner_than_dwc3_allows(self):
    self.plug('sink')
    self.run_for(SWAP + 0.5)
    self.mode.write_text('none\n')   # floating lines and the glue's knob not set: dwc3's 10 s
    self.run_for(port.RESTART_AFTER - 1)
    self.assertEqual(self.commands(self.udc), ['start'])
    self.run_for(1)
    self.assertEqual(self.commands(self.udc), ['start', 'start'])

  def test_a_configured_gadget_is_not_looked_at(self):
    self.plug('sink')
    self.run_for(60, configured=True)
    self.assertEqual(self.commands(self.udc), [])

  def test_an_owner_after_one_that_left_it_on_turns_it_off(self):
    self.mode.write_text('peripheral\n')
    self.run_for(60)
    self.assertEqual(self.commands(self.udc), ['stop'])

  def test_a_stop_never_turns_it_off(self):
    # the stop that comes as a chestnut turns up: the comma is about to host it
    self.plug('sink')
    self.run_for(SWAP + 1)
    self.port.off()
    self.plug('source')
    self.run_for(port.UNPLUGGED + 1)
    self.assertNotIn('stop', self.commands(self.udc))

  def test_only_the_device_end_of_a_sink(self):
    for role, data in (('source', 'dfp'), ('source', 'ufp'), ('sink', 'dfp')):
      self.plug(role, data)
      self.run_for(SWAP + 1)
    self.assertNotIn('start', self.commands(self.udc))

  def test_no_glue_is_left_alone(self):
    self.mode.unlink()
    self.plug('sink')
    self.run_for(60)
    self.plug('none')
    self.run_for(60)
    self.assertEqual(self.commands(self.udc), [])


class TestAccessories(PortTest):
  """A chestnut is never taken for a host; anything else that cannot host is
  let go without being cycled."""

  def left_alone(self, ids: tuple[int, int]) -> None:
    self.plug('source')
    self.enumerate('2-1', ids)
    with mock.patch.object(port, 'chestnut_attached', wraps=port.chestnut_attached) as scan:
      self.run_for(60)
    self.assertEqual(self.commands(), ['off'])
    self.assertEqual(scan.call_count, 1, "one sysfs read a cycle, not a directory walk")

  def test_a_chestnut_is_left_alone_and_looked_for_once(self):
    self.left_alone(CHESTNUT)

  def test_a_chestnut_being_flashed_is_left_alone(self):
    self.left_alone(CHESTNUT_ROM)

  def test_the_ids_the_caller_names_are_the_chestnut(self):
    # the fork's adapter hands over openpilot's own; nothing else is one then
    self.port = port.Port(chestnut_ids={JETSON_GADGET})
    self.left_alone(JETSON_GADGET)

  def test_ids_the_caller_did_not_name_are_not_a_chestnut(self):
    self.port = port.Port(chestnut_ids={JETSON_GADGET})
    self.plug('source')
    self.enumerate('2-1', CHESTNUT)
    self.run_for(SWAP + 1)
    self.assertEqual(self.commands(), ['off', 'hold'])

  def test_a_sink_that_cannot_host_is_not_cycled(self):
    self.plug('source')
    self.run_for(SWAP + 1)
    self.plug('none')   # held at sink, a sink-only accessory has nothing to attach to
    self.run_for(RELEASE + 0.5)
    self.assertEqual(self.commands(), ['off', 'hold', 'off'])
    # dual role again, and it is back after a toggle; a poll can land in the gap
    self.run_for(0.5)
    self.plug('source')
    self.run_for(60)
    self.assertEqual(self.commands(), ['off', 'hold', 'off'])

  def test_a_new_plug_is_judged_afresh(self):
    self.plug('source')
    self.enumerate('2-1', CHESTNUT)
    self.run_for(SWAP + 1)
    self.plug('none')
    (self.devices / '2-1' / 'idVendor').unlink()
    self.run_for(port.UNPLUGGED + 0.5)
    self.plug('source')
    self.run_for(SWAP + 1)
    self.assertEqual(self.commands(), ['off', 'hold'])

  def test_a_hold_that_failed_is_not_retried_on_the_same_plug(self):
    self.script.side_effect = lambda command: command != 'hold'
    self.plug('source')
    self.run_for(60)
    self.assertEqual(self.commands(), ['off', 'hold'])
    self.assertFalse(self.port.held)


class TestTheLink(PortTest):
  def test_turning_the_link_off_undoes_a_hold_once(self):
    self.plug('source')
    self.run_for(SWAP + 1)
    self.port.off()
    self.port.off()
    self.assertEqual(self.commands(), ['off', 'hold', 'off'])
    self.assertFalse(self.port.held)

  def test_stopping_outside_a_hold_runs_nothing(self):
    self.plug('sink')
    self.run_for(5)
    self.port.off()
    self.assertEqual(self.commands(), ['off'])

  def test_a_failed_first_off_does_not_stop_the_watch(self):
    # both commas have the lever, so a failure is a failure, which root.run
    # has logged, and not a device without one
    self.script.side_effect = lambda command: command != 'off'
    self.plug('source')
    self.run_for(SWAP + 1)
    self.assertEqual(self.commands(), ['off', 'hold'])

  def test_no_policy_engine_is_no_role(self):
    self.role.unlink()
    self.run_for(60)
    self.assertEqual(self.commands(), ['off'])


class TestChargingThePhone(PortTest):
  """An iPhone on a direct cable comes back from a hold as the source and the
  host. On iOS it is asked once a plug, over USB PD, to charge from the comma."""

  ASKED = ['off', 'hold', 'off', 'source']
  charge = True

  def setUp(self):
    super().setUp()
    self.contract = self.tmp / 'contract'
    self.contract.write_text('explicit\n')
    self.thermal = self.tmp / 'thermal'
    self.zone('thermal_zone0', 'cpu2-gold-usr', 60.0)
    self.zone('thermal_zone1', 'pm8998_tz', 50.0)
    self.zone('thermal_zone2', 'battery', 120.0)   # not one hardwared looks at
    self.script.side_effect = self.swap

  def zone(self, name: str, kind: str, temp: float) -> None:
    d = self.thermal / name
    d.mkdir(parents=True, exist_ok=True)
    (d / 'type').write_text(kind + '\n')
    (d / 'temp').write_text(f'{int(temp * 1000)}\n')

  def swap(self, command: str) -> bool:
    """jetlink-root.sh port: a PR_Swap the phone accepts leaves the comma the
    source and still the device."""
    if command == 'source':
      self.plug('source', 'ufp')
    elif command == 'sink':
      self.plug('sink', 'ufp')
    return True

  def held_iphone(self, ios: bool = True) -> None:
    """Plugged in, held, back as the source and the host, the gadget configured."""
    self.plug('source')
    self.run_for(SWAP + 1)
    self.plug('sink', 'ufp')
    self.run_for(SWAP + 1, configured=True, ios=ios)

  def lines(self) -> list[str]:
    return [c.args[0] % c.args[1:] for c in self.log.warning.call_args_list]

  def test_it_is_asked_to_charge_and_then_left_alone(self):
    self.held_iphone()
    self.assertEqual(self.commands(), self.ASKED, "the hold let go first: the voter gates the swap")
    self.assertFalse(self.port.held)
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED, "the source that is the device is never held")
    self.assertIn("jetlink: charging the iPhone", self.lines())

  def test_off_keeps_a_working_iphone_link_in_its_negotiated_power_role(self):
    self.charge = False
    self.held_iphone()
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), ['off', 'hold'])
    self.assertTrue(self.port.held)
    self.assertFalse(self.port.charge_asked)
    # A transient loss and reconfiguration must not enable charging either.
    self.run_for(5, ios=True)
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), ['off', 'hold'])

  def test_off_leaves_a_phone_already_charging_as_it_is(self):
    self.charge = False
    self.plug('source', 'ufp')
    with mock.patch.object(port.gadget, 'host_attached', return_value=True):
      self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), [])

  def test_it_waits_for_the_gadget(self):
    self.plug('sink', 'ufp')
    self.mode.write_text('peripheral\n')
    self.run_for(60, ios=True)
    self.assertNotIn('source', self.commands())
    self.run_for(SWAP + 1, configured=True, ios=True)
    self.assertEqual(self.commands(), ['off', 'source'])

  def test_without_usb_pd_it_is_never_asked(self):
    self.contract.write_text('implicit\n')
    self.held_iphone()
    self.run_for(60, configured=True, ios=True)
    self.assertNotIn('source', self.commands())

  def test_on_usb_it_is_never_asked(self):
    self.held_iphone(ios=False)
    self.run_for(60, configured=True)
    self.assertEqual(self.commands(), ['off', 'hold'])

  def test_a_refusal_is_asked_once_a_plug(self):
    self.script.side_effect = lambda command: command != 'source'
    self.held_iphone()
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED)
    self.assertIn("jetlink: the iPhone kept the source role; not asking again until the next plug", self.lines())

  def test_a_phone_that_takes_the_power_back_is_not_asked_again(self):
    self.held_iphone()
    self.plug('sink', 'ufp')
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED, "the two ends would swap in a loop")
    self.assertIn("jetlink: the iPhone took the source role back; not asking again until the next plug",
                  self.lines())

  def test_the_next_plug_is_asked_afresh(self):
    self.held_iphone()
    self.plug('none')
    self.run_for(port.UNPLUGGED + 0.5)
    self.plug('sink', 'ufp')   # dual role again since the swap, it may come back as the source
    self.run_for(SWAP + 1, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED + ['source'])

  def test_a_hot_comma_waits_until_it_cools(self):
    self.zone('thermal_zone0', 'cpu2-gold-usr', port.HOT_C + 5)
    self.held_iphone()
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), ['off', 'hold'])
    hot = [line for line in self.lines() if 'waits until it is below' in line]
    self.assertEqual(len(hot), 1, "said once")
    self.zone('thermal_zone0', 'cpu2-gold-usr', port.HOT_C - 5)
    self.run_for(1, configured=True, ios=True)
    self.assertEqual(self.commands(), ['off', 'hold'], "smoothed: one cooler reading is not enough")
    self.run_for(10, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED)

  def test_a_spike_does_not_hold_it_up_for_long(self):
    self.zone('thermal_zone0', 'cpu2-gold-usr', port.HOT_C + 5)
    self.plug('source')
    self.run_for(SWAP + 1)
    self.plug('sink', 'ufp')
    self.run_for(SWAP + 0.5, configured=True, ios=True)   # one hot reading
    self.zone('thermal_zone0', 'cpu2-gold-usr', 70.0)
    self.run_for(5, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED)

  def test_no_thermal_zones_is_not_hot(self):
    import shutil
    shutil.rmtree(self.thermal)
    self.held_iphone()
    self.assertEqual(self.commands(), self.ASKED)

  def test_a_link_lost_while_charging_hands_the_phone_the_source_role_back(self):
    self.held_iphone()
    self.run_for(5, configured=True, ios=True)
    self.run_for(5, ios=True)
    self.assertEqual(self.commands(), self.ASKED + ['sink'])
    dropped = [line for line in self.lines() if 'the link went down' in line]
    self.assertEqual(len(dropped), 1, self.lines())
    self.assertIn("jetlink: the iPhone powers the comma again; not charging it until the next plug", self.lines())
    self.assertFalse(any('took the source role back' in line for line in self.lines()),
                     "the swap back is the comma's, not the phone's")
    # the link comes back with the phone powering the port, and stays that way
    self.run_for(60, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED + ['sink'])

  def test_a_refused_swap_back_is_said_and_not_retried(self):
    self.script.side_effect = lambda command: self.swap(command) if command != 'sink' else False
    self.held_iphone()
    self.run_for(5, configured=True, ios=True)
    self.run_for(10, ios=True)
    self.assertEqual(self.commands(), self.ASKED + ['sink'])
    self.assertIn("jetlink: the iPhone kept charging from the comma; unplug the cable to start over", self.lines())

  def test_a_link_that_stays_up_keeps_charging(self):
    self.held_iphone()
    self.run_for(120, configured=True, ios=True)
    self.assertEqual(self.commands(), self.ASKED)
    self.assertTrue(self.port.charging)


class TestTheScript(unittest.TestCase):
  def test_it_runs_the_root_script_on_the_short_timeout(self):
    # short: the owner lets the port go before it closes FunctionFS, inside
    # manager's 5 s
    with mock.patch.object(root, 'run', return_value=True) as run:
      self.assertTrue(port.run_script('hold'))
    run.assert_called_once_with('port', 'hold', timeout=root.PORT_TIMEOUT)

  def test_a_power_role_swap_gets_the_time_the_kernel_waits(self):
    with mock.patch.object(root, 'run', return_value=True) as run:
      self.assertTrue(port.run_script('source'))
      self.assertTrue(port.run_script('sink'))
    self.assertEqual(run.call_args_list, [mock.call('port', 'source', timeout=root.SWAP_TIMEOUT),
                                          mock.call('port', 'sink', timeout=root.SWAP_TIMEOUT)])
