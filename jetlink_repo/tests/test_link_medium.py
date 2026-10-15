"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

Which link a comma is on, USB 3, USB 2 or TCP, as the apps show it: what each
transport sees from the comma's end, and what the comma's hello says. The
server names the medium from that (LinkMediumTests in JetlinkKit).
"""
from __future__ import annotations

from types import SimpleNamespace

import pytest

from jetlink.client import JetlinkClient
from jetlink.transport import base
from jetlink.transport.base import udc_speed
from jetlink.transport.ffs import FfsTransport
from jetlink.transport.tcp import CABLE_ADDRESS, TcpTransport


@pytest.fixture
def udc(tmp_path, monkeypatch):
  """A fake /sys/class/udc with one controller; returns a setter for its speed."""
  controller = tmp_path / 'a600000.dwc3'
  controller.mkdir()
  monkeypatch.setattr(base, 'UDC_SYSFS', str(tmp_path))

  def speed(value: str) -> None:
    (controller / 'current_speed').write_text(value + '\n')
  speed('UNKNOWN')
  return speed


def test_the_controller_speed_is_read_until_a_host_configures_it(udc):
  assert udc_speed() is None
  udc('super-speed')
  assert udc_speed() == 'super-speed'
  assert udc_speed('a600000.dwc3') == 'super-speed'
  assert udc_speed('no-such-controller') is None


def test_the_gadget_says_usb_and_its_speed(udc):
  udc('high-speed')
  t = FfsTransport.__new__(FfsTransport)
  t.bound_udc = 'a600000.dwc3'
  assert t.link_info() == {'kind': 'usb', 'usb_speed': 'high-speed'}


def test_a_phones_dial_is_the_cable_and_a_lan_is_tcp(udc):
  udc('super-speed')
  cable = TcpTransport.__new__(TcpTransport)
  cable.sock = SimpleNamespace(getsockname=lambda: (CABLE_ADDRESS, 5599), getpeername=lambda: ('192.168.60.4', 50000))
  assert cable.link_info() == {'kind': 'cable', 'usb_speed': 'super-speed'}
  lan = TcpTransport.__new__(TcpTransport)
  lan.sock = SimpleNamespace(getsockname=lambda: ('10.0.0.5', 40000), getpeername=lambda: ('10.0.0.9', 5599))
  assert lan.link_info() == {'kind': 'tcp'}


class FakeTransport:
  def __init__(self, link):
    self.link = link
    self.sent = []

  def link_info(self):
    if isinstance(self.link, Exception):
      raise self.link
    return self.link

  def send_json(self, msg_type, seq, obj, flags=0):
    self.sent.append(obj)

  def stop_datagrams(self):
    pass


@pytest.mark.parametrize('link,expected', [({'kind': 'cable', 'usb_speed': 'super-speed'}, {'kind': 'cable', 'usb_speed': 'super-speed'}),
                                           ({}, None), (OSError('no sysfs'), None)])
def test_the_hello_carries_the_link(link, expected):
  t = FakeTransport(link)
  client = JetlinkClient(t, name='modeld')
  client._expect = lambda *a, **k: SimpleNamespace(payload=memoryview(b'{}'))
  client.hello()
  assert t.sent[0]['client'].get('link') == expected
  assert t.sent[0]['client']['name'] == 'modeld'
