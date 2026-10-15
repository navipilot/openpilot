#!/usr/bin/env python3
"""
Copyright (c) 2026-, Zeph Leggett.

This file is part of jetlink and is licensed under the MIT License.
See the LICENSE file in the root directory for more details.

Prove the built Jetlink.app serves: its Swift server, in the app's own process,
speaking the wire protocol a comma speaks.

CI runs this against the signed bundle, so it fails when the app does not
launch, when the server inside it does not start, or when the protocol
regresses. The app is launched with TCP on a free port and a cache in a
temporary directory (the argument domain overrides the stored settings without
saving them), then a client says hello and pings, and SIGTERM must end the app
within 20 s. No comma and no model are involved.

  macos/scripts/smoke.py                      macos/build/Jetlink.app
  macos/scripts/smoke.py --app path/to/Jetlink.app
"""
from __future__ import annotations

import argparse
import json
import os
import signal
import socket
import subprocess
import sys
import tempfile
import time
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(REPO_ROOT))

from jetlink import protocol as P  # noqa: E402
from jetlink.transport.base import LinkError  # noqa: E402
from jetlink.transport.tcp import TcpTransport  # noqa: E402

DEFAULT_APP = REPO_ROOT / 'macos' / 'build' / 'Jetlink.app'
LISTEN_TIMEOUT = 60.0
REPLY_TIMEOUT = 15.0
EXIT_TIMEOUT = 20.0


class SmokeError(Exception):
  pass


def free_port() -> int:
  with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(('127.0.0.1', 0))
    return s.getsockname()[1]


def connect(port: int, app: subprocess.Popen) -> TcpTransport:
  deadline = time.monotonic() + LISTEN_TIMEOUT
  while time.monotonic() < deadline:
    if app.poll() is not None:
      raise SmokeError(f"the app exited with {app.returncode} before it listened")
    try:
      return TcpTransport.connect('127.0.0.1', port, timeout=1.0)
    except OSError:
      time.sleep(0.25)
  raise SmokeError(f"nothing listened on {port} in {LISTEN_TIMEOUT:.0f} s")


def expect(transport: TcpTransport, msg_type: int, seq: int):
  message = transport.recv(timeout=REPLY_TIMEOUT)
  if message.msg_type != msg_type or message.seq != seq:
    raise SmokeError(f"wanted type {msg_type} seq {seq}, got type {message.msg_type} seq {message.seq}")
  return message


def run(app_path: Path) -> None:
  binary = app_path / 'Contents' / 'MacOS' / 'Jetlink'
  if not binary.is_file():
    raise SmokeError(f"no app binary at {binary}; run make -C macos app")
  port = free_port()
  with tempfile.TemporaryDirectory(prefix='jetlink-smoke-') as cache:
    args = [str(binary), '-developerTransport', 'tcp', '-tcpPort', str(port), '-cacheDirectory', cache,
            '-startServerOnLaunch', 'YES']
    app = subprocess.Popen(args, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, text=True)
    try:
      transport = connect(port, app)
      try:
        transport.send_json(P.Msg.HELLO_REQ, 1, {'client': {'name': 'smoke', 'nonce': os.getpid()}})
        hello = json.loads(bytes(expect(transport, P.Msg.HELLO_RESP, 1).payload))
        if hello.get('protocol') != P.VERSION:
          raise SmokeError(f"the server speaks protocol {hello.get('protocol')}, this checkout {P.VERSION}")
        print(f"hello: protocol {hello['protocol']}, {hello.get('backend')} {hello.get('runtime_version')} "
              f"on {hello.get('device')}, engine {hello.get('engine_state')}")
        transport.send(P.Msg.PING, 2)
        expect(transport, P.Msg.PONG, 2)
        print("ping: pong")
      except LinkError as e:
        raise SmokeError(f"the link failed: {e}") from e
      finally:
        transport.close()
      app.send_signal(signal.SIGTERM)
      try:
        app.wait(EXIT_TIMEOUT)
      except subprocess.TimeoutExpired as e:
        raise SmokeError(f"the app did not exit within {EXIT_TIMEOUT:.0f} s of SIGTERM") from e
      print(f"exit: {app.returncode}")
    finally:
      if app.poll() is None:
        app.kill()
        app.wait()
      if app.stderr is not None:
        tail = app.stderr.read().strip().splitlines()[-20:]
        if tail:
          print("app stderr (last lines):", *tail, sep='\n  ')


def main() -> int:
  p = argparse.ArgumentParser(description=__doc__.split('\n\n')[1], formatter_class=argparse.RawDescriptionHelpFormatter)
  p.add_argument('--app', type=Path, default=DEFAULT_APP)
  args = p.parse_args()
  try:
    run(args.app)
  except SmokeError as e:
    print(f"smoke: FAILED: {e}", file=sys.stderr)
    return 1
  print("smoke: OK")
  return 0


if __name__ == '__main__':
  sys.exit(main())
