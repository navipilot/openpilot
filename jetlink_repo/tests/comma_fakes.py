"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

What the comma tests share: jetlink-root.sh run without sudo, through its
override variables, on a fake /proc/sys and a fake port lever under one
directory.
"""
from __future__ import annotations

import os
import re
import subprocess
from pathlib import Path

from jetlink.comma import root

# stock AGNOS: the dirty limits in ratio mode, so both *_bytes keys read 0
STOCK = {
  'vm.dirty_bytes': '0',
  'vm.dirty_background_bytes': '0',
  'vm.dirty_ratio': '20',
  'vm.dirty_background_ratio': '10',
  'vm.extra_free_kbytes': '0',
  'net.core.wmem_max': '229376',
  'net.core.rmem_max': '229376',
}
# what vm apply writes, read off the script so the values are said once
TUNED = dict(pair.split('=') for pair in
             re.search(r'^VM_SYSCTLS=\((.*?)\)$', root.SCRIPT.read_text(), re.M | re.S).group(1).split())


def proc_sys(tmp: Path, values: dict[str, str]) -> None:
  """Write `values` into the fake /proc/sys under `tmp`."""
  for key, value in values.items():
    f = tmp / 'sys' / key.replace('.', '/')
    f.parent.mkdir(parents=True, exist_ok=True)
    f.write_text(value + '\n')


def read_sys(tmp: Path, key: str) -> str:
  return (tmp / 'sys' / key.replace('.', '/')).read_text().strip()


def read_all(tmp: Path, keys) -> dict[str, str]:
  return {k: read_sys(tmp, k) for k in keys}


def record(tmp: Path) -> Path:
  """Where vm apply keeps the stock values it found."""
  return tmp / 'sysctl-prev'


def voter(tmp: Path) -> Path:
  """The port's DISABLE_POWER_ROLE_SWITCH voter; the script finds none until it is made."""
  return tmp / 'voter'


def usb_icl(tmp: Path) -> Path:
  """The charger's USB_ICL voter; the script finds none until it is made."""
  return tmp / 'usb_icl'


def usbpd(tmp: Path) -> Path:
  """The policy engine's usbpd0 directory; the script finds none until it is made."""
  return tmp / 'usbpd0'


def dual_role(tmp: Path) -> Path:
  """The dual_role class's otg_default; the script finds none until it is made."""
  return tmp / 'otg_default'


def udc_glue(tmp: Path) -> Path:
  """dwc3's glue for usb0; the script finds none until it is made."""
  return tmp / 'a600000.ssusb'


def pe_params(tmp: Path) -> Path:
  """The policy engine's module parameters; the script finds none until they are made."""
  return tmp / 'policy_engine'


def ffs_log_off(tmp: Path) -> Path:
  """FunctionFS's IPC debug log switch, 1 for off; a plain file the script writes."""
  return tmp / 'f_fs_log_disable'


def cable_netdev(tmp: Path, name: str = 'usb1') -> Path:
  """The NCM function's netdev, named in the gadget as f_ncm names it at bind;
  returns its receive queue's rps_cpus, a plain file the script writes."""
  function = tmp / 'gadget' / 'functions' / 'ncm.usb0'
  function.mkdir(parents=True)
  (function / 'ifname').write_text(f'{name}\n')
  queue = tmp / 'net' / name / 'queues' / 'rx-0'
  queue.mkdir(parents=True)
  mask = queue / 'rps_cpus'
  mask.write_text('00\n')
  return mask


def run_script(tmp: Path, *args: str, timeout: float | None = None, iptables: Path | None = None) -> subprocess.CompletedProcess:
  """jetlink-root.sh *args, as the user running the tests, on the fakes under `tmp`."""
  env = {
    **os.environ,
    'JETLINK_PROC_SYS': str(tmp / 'sys'),
    'JETLINK_SYSCTL_PREV': str(record(tmp)),
    'JETLINK_POWER_ROLE_VOTER': str(voter(tmp)),
    'JETLINK_USB_ICL_VOTER': str(usb_icl(tmp)),
    'JETLINK_USBPD': str(usbpd(tmp)),
    'JETLINK_DUAL_ROLE': str(dual_role(tmp)),
    'JETLINK_UDC_GLUE': str(udc_glue(tmp)),
    'JETLINK_PE_PARAMS': str(pe_params(tmp)),
    'JETLINK_FFS_LOG_OFF': str(ffs_log_off(tmp)),
    'JETLINK_GADGET': str(tmp / 'gadget'),
    'JETLINK_SYS_NET': str(tmp / 'net'),
    # nothing on this machine's tables: a fake, or a name nothing answers to
    'JETLINK_IPTABLES': str(iptables or tmp / 'no-iptables'),
  }
  return subprocess.run(['bash', str(root.SCRIPT), *args], env=env, capture_output=True, text=True, timeout=timeout)
