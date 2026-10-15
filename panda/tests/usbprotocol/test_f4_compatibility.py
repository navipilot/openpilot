import hashlib
import ctypes
import os
import struct
import subprocess
from pathlib import Path
from unittest.mock import Mock, patch

import pytest

from panda import Panda, PandaDFU, McuType, FW_PATH
from panda.python.constants import compute_version_hash
from panda.tests.libpanda.libpanda_py import CANPacket, CANRing, make_CANPacket
from opendbc.car.structs import CarParams


@pytest.fixture(params=[(Panda.HW_TYPE_DOS, McuType.F4), (Panda.HW_TYPE_TRES, McuType.H7),
                        (Panda.HW_TYPE_RED_PANDA, McuType.H7), (Panda.HW_TYPE_CUATRO, McuType.H7)])
def device(request):
  hw_type, mcu = request.param
  panda = Panda.__new__(Panda)
  panda.get_type = Mock(return_value=hw_type)
  panda._serial = "010002000300040005000600"
  panda._handle = Mock()
  panda.bootstub = True
  return panda, mcu


def test_discovery_and_firmware_selection(device):
  panda, mcu = device
  assert panda.get_mcu_type() == mcu
  assert panda.get_type() in Panda.SUPPORTED_DEVICES
  assert panda.is_internal() == (panda.get_type() != Panda.HW_TYPE_RED_PANDA)
  panda.get_signature = Mock(return_value=b"signature")
  with patch.object(Panda, "get_signature_from_firmware", return_value=b"signature") as signature:
    assert panda.up_to_date()
    signature.assert_called_once_with(os.path.join(FW_PATH, mcu.config.app_fn))


def test_flash_selection(device):
  panda, mcu = device
  panda.up_to_date = Mock(return_value=False)
  panda.get_version = Mock(return_value="bootstub")
  with patch.object(Panda, "flash_static") as flash:
    panda.flash(code=b"test", reconnect=False)
    flash.assert_called_once_with(panda._handle, b"test", mcu_type=mcu)


def test_dfu_serial(device):
  panda, mcu = device
  expected = "000800100004" if mcu == McuType.F4 else "000800060004"
  assert panda.get_dfu_serial() == expected


def test_flash_sector_boundaries(device):
  _, mcu = device
  handle = Mock()
  handle.controlRead.return_value = b"\0" * 4 + b"\xde\xad\xd0\x0d"
  code = b"\0" * mcu.config.sector_sizes[1]
  Panda.flash_static(handle, code, mcu)
  erases = [call.args[2] for call in handle.controlWrite.call_args_list if call.args[1] == 0xb2]
  assert erases == [1]


def test_unknown_hardware_rejected():
  panda = Panda.__new__(Panda)
  panda.get_type = Mock(return_value=Panda.HW_TYPE_UNKNOWN)
  with pytest.raises(ValueError):
    panda.get_mcu_type()


def test_packet_versions(device):
  panda, _ = device
  versions = (Panda.HEALTH_PACKET_VERSION, Panda.CAN_PACKET_VERSION)
  panda._handle.controlRead.return_value = struct.pack("<II", *versions)
  assert panda.get_packets_versions() == versions
  panda._handle.controlRead.return_value = b"\0\0\0"
  assert panda.get_packets_versions() == (0, 0)
  assert Panda.HEALTH_STRUCT.size == 49
  assert compute_version_hash(Path(FW_PATH).parent / "health.h") == versions[0]


@pytest.mark.parametrize("mcu", [McuType.F4, McuType.H7])
def test_built_firmware_signature(mcu):
  image = (Path(FW_PATH) / mcu.config.app_fn).read_bytes()
  length, = struct.unpack("<I", image[:4])
  assert length + 128 == len(image)
  assert image[length - 8:length] == b"VERS" + struct.pack("<I", 2)
  public_key = (Path(FW_PATH).parent / "crypto/certs/debug.pub").read_bytes()
  import base64
  key = base64.b64decode(public_key.split()[1])
  values = []
  while key:
    size = int.from_bytes(key[:4], "big")
    values.append(key[4:4 + size])
    key = key[4 + size:]
  _, exponent, modulus = values
  decoded = pow(int.from_bytes(image[length:], "big"), int.from_bytes(exponent, "big"),
                int.from_bytes(modulus, "big")).to_bytes(128, "big")
  assert decoded == b"\0\x01" + b"\xff" * 105 + b"\0" + hashlib.sha1(image[4:length]).digest()
  assert (Path(FW_PATH) / mcu.config.bootstub_fn).stat().st_size < mcu.config.sector_sizes[0]


@pytest.mark.parametrize("mcu", [McuType.F4, McuType.H7])
def test_recovery_bootstub_selection(mcu):
  dfu = PandaDFU.__new__(PandaDFU)
  dfu._mcu_type = mcu
  dfu.program_bootstub = Mock()
  dfu.reset = Mock()
  dfu.recover()
  dfu.program_bootstub.assert_called_once_with((Path(FW_PATH) / mcu.config.bootstub_fn).read_bytes())
  dfu.reset.assert_called_once()


@pytest.mark.parametrize("h7", [False, True])
def test_camera_clock_registers(h7):
  root = Path(FW_PATH).parents[1].resolve()
  import opendbc
  output = root / "board/obj" / ("test_clock_h7" if h7 else "test_clock_f4")
  command = ["gcc", "-std=gnu11", "-Wno-pointer-to-int-cast", "-I", str(root),
             "-I", opendbc.INCLUDE_PATH, str(root / "tests/usbprotocol/test_clock_source.c"),
             "-o", str(output)]
  if h7:
    command.append("-DTEST_H7_CLOCK")
  try:
    subprocess.run(command, check=True, capture_output=True)
    subprocess.run([str(output)], check=True)
  finally:
    output.unlink(missing_ok=True)


@pytest.fixture
def f4_firmware():
  lib = ctypes.CDLL(str(Path(__file__).parents[1] / "libpanda/libpanda_f4.so"))
  lib.can_init_all.argtypes = []
  lib.can_send.argtypes = [ctypes.POINTER(CANPacket), ctypes.c_uint8, ctypes.c_bool]
  lib.can_pop.argtypes = [ctypes.POINTER(CANRing), ctypes.POINTER(CANPacket)]
  lib.can_pop.restype = ctypes.c_bool
  lib.comms_can_reset.argtypes = []
  lib.set_safety_hooks.argtypes = [ctypes.c_uint16, ctypes.c_uint16]
  lib.set_safety_hooks(CarParams.SafetyModel.allOutput, 0)
  lib.can_init_all()
  lib.comms_can_reset()
  return lib


@pytest.mark.parametrize("payload,fd,rejected", [(b"12345678", False, False), (b"12345678", True, True),
                                              (b"123456789012", False, True)])
def test_f4_classic_can_only(f4_firmware, payload, fd, rejected):
  packet = make_CANPacket(0x123, 0, payload)
  packet.fd = fd
  f4_firmware.can_set_checksum(ctypes.byref(packet))
  f4_firmware.can_send(ctypes.byref(packet), 0, True)
  queue_name = "rx_q" if rejected else "tx1_q"
  queue = ctypes.POINTER(CANRing).in_dll(f4_firmware, queue_name)
  result = CANPacket()
  assert f4_firmware.can_pop(queue, ctypes.byref(result))
  assert bool(result.rejected) == rejected
  assert result.addr == packet.addr
  assert result.data_len_code == packet.data_len_code
  assert bytes(result.data[:len(payload)]) == payload
  assert queue.contents.fifo_size == (1536 if rejected else 256)
