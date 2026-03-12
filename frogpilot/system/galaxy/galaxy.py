#!/usr/bin/env python3
import hmac
import json
import platform
import shutil
import signal
import subprocess
import tarfile
import time
import threading
import urllib.request
from http.server import HTTPServer, BaseHTTPRequestHandler
from pathlib import Path

from openpilot.common.params import Params
from openpilot.system.hardware import HARDWARE
from openpilot.system.hardware.hw import Paths

if HARDWARE.get_device_type() == "pc":
  GALAXY_DIR = Path(Paths.comma_home()) / "frogpilot" / "data" / "galaxy"
else:
  GALAXY_DIR = Path("/data/galaxy")
FRPC_VERSION = "0.67.0"
FRPC_LOG = GALAXY_DIR / "frpc.log"
AUTH_PORT = 8083
BUNDLED_FRPC_DIR = Path(__file__).resolve().parent / "bin"

process = None
auth_server = None

VALID_PATHS = {"/glxylogin": "glxyauth", "/glxyverify": "glxysession"}


class AuthHandler(BaseHTTPRequestHandler):
  """Validates login hashes and verifies session tokens."""
  dongle_id = ""

  def do_POST(self):
    target = VALID_PATHS.get(self.path)
    if not target:
      self.send_response(404)
      self.end_headers()
      return

    file_path = GALAXY_DIR / target
    if not file_path.exists():
      self.send_response(404)
      self.end_headers()
      return

    try:
      length = int(self.headers.get("Content-Length", 0))
      body = self.rfile.read(length).decode("utf-8").strip()
      if hmac.compare_digest(body, file_path.read_text().strip()):
        self.send_response(200)
        if self.path == "/glxylogin":
          self.send_header("Content-Type", "application/json")
          self.end_headers()
          token = (GALAXY_DIR / "glxysession").read_text().strip()
          self.wfile.write(json.dumps({"dongle_id": self.dongle_id, "token": token}).encode())
        else:
          self.end_headers()
        return
    except Exception:
      pass

    self.send_response(403)
    self.end_headers()

  def log_message(self, format, *args):
    pass


def start_auth_server():
  global auth_server
  if auth_server is not None:
    return
  auth_server = HTTPServer(("127.0.0.1", AUTH_PORT), AuthHandler)
  thread = threading.Thread(target=auth_server.serve_forever, daemon=True)
  thread.start()
  print(f"Galaxy: Auth server listening on 127.0.0.1:{AUTH_PORT}")


def cleanup_frpc(*_):
  global process
  if process is not None and process.poll() is None:
    process.terminate()
    try:
      process.wait(timeout=5)
    except subprocess.TimeoutExpired:
      process.kill()
    process = None


def get_arch_url():
  arch = platform.machine()
  system = platform.system()

  if system == "Darwin":
    if arch in ("aarch64", "arm64"):
      return f"https://github.com/fatedier/frp/releases/download/v{FRPC_VERSION}/frp_{FRPC_VERSION}_darwin_arm64.tar.gz", f"frp_{FRPC_VERSION}_darwin_arm64"
    if arch in ("x86_64", "amd64"):
      return f"https://github.com/fatedier/frp/releases/download/v{FRPC_VERSION}/frp_{FRPC_VERSION}_darwin_amd64.tar.gz", f"frp_{FRPC_VERSION}_darwin_amd64"
  elif system == "Linux":
    if arch in ("aarch64", "arm64"):
      return f"https://github.com/fatedier/frp/releases/download/v{FRPC_VERSION}/frp_{FRPC_VERSION}_linux_arm64.tar.gz", f"frp_{FRPC_VERSION}_linux_arm64"
    if arch in ("x86_64", "amd64"):
      return f"https://github.com/fatedier/frp/releases/download/v{FRPC_VERSION}/frp_{FRPC_VERSION}_linux_amd64.tar.gz", f"frp_{FRPC_VERSION}_linux_amd64"

  return None, None


def get_bundled_frpc_path():
  arch = platform.machine().lower()
  system = platform.system().lower()

  if system == "darwin":
    if arch in ("aarch64", "arm64"):
      return BUNDLED_FRPC_DIR / "frpc_darwin_arm64"
    if arch in ("x86_64", "amd64"):
      return BUNDLED_FRPC_DIR / "frpc_darwin_amd64"
  elif system == "linux":
    if arch in ("aarch64", "arm64"):
      return BUNDLED_FRPC_DIR / "frpc_linux_arm64"
    if arch in ("x86_64", "amd64"):
      return BUNDLED_FRPC_DIR / "frpc_linux_amd64"

  return None


def get_param_str(params, key):
  value = params.get(key)
  if isinstance(value, bytes):
    return value.decode("utf-8", errors="replace")
  return str(value or "")


def setup_frpc():
  GALAXY_DIR.mkdir(parents=True, exist_ok=True)
  frpc_bin = GALAXY_DIR / "frpc"

  if not frpc_bin.exists():
    bundled_bin = get_bundled_frpc_path()
    if bundled_bin is not None and bundled_bin.exists():
      try:
        shutil.copy2(bundled_bin, frpc_bin)
        frpc_bin.chmod(0o755)
        print(f"Galaxy: Installed bundled frpc ({bundled_bin.name}).")
        return True
      except Exception as e:
        print(f"Galaxy: Failed to install bundled frpc: {e}")

    print("Galaxy: Downloading frpc (bundled binary unavailable)...")
    url, folder_name = get_arch_url()
    if not url:
      print("Galaxy: Unsupported architecture")
      return False

    tar_path = GALAXY_DIR / "frp.tar.gz"
    try:
      urllib.request.urlretrieve(url, tar_path)
      with tarfile.open(tar_path, "r:gz") as tar:
        tar.extractall(path=GALAXY_DIR, filter='data')

      # Move binary
      extracted_bin = GALAXY_DIR / folder_name / "frpc"
      extracted_bin.rename(frpc_bin)
      frpc_bin.chmod(0o755)

      # Cleanup
      tar_path.unlink()
      shutil.rmtree(GALAXY_DIR / folder_name)
      print("Galaxy: frpc downloaded and installed.")
    except Exception as e:
      print(f"Galaxy: Failed to install frpc: {e}")
      return False

  return True


def main():
  global process
  params = Params()

  signal.signal(signal.SIGTERM, cleanup_frpc)
  signal.signal(signal.SIGINT, cleanup_frpc)

  dongle_id = get_param_str(params, "DongleId")
  while not dongle_id:
    print("Galaxy: Waiting for DongleId...")
    time.sleep(5)
    dongle_id = get_param_str(params, "DongleId")

  print(f"Galaxy: DongleId: {dongle_id}")
  AuthHandler.dongle_id = dongle_id
  print("Galaxy: Starting manager loop...")

  last_slug = None

  while True:
    glxyauth_file = GALAXY_DIR / "glxyauth"
    galaxy_password_hash = glxyauth_file.read_text().strip() if glxyauth_file.exists() else None
    slug_file = GALAXY_DIR / "glxyslug"
    slug = slug_file.read_text().strip() if slug_file.exists() else None
    is_paired = galaxy_password_hash and len(galaxy_password_hash) == 64 and slug

    if is_paired:
      if process is None or process.poll() is not None or slug != last_slug:
        cleanup_frpc()
        if process is not None and slug == last_slug:
          print(f"Galaxy: frpc exited with code {process.returncode}. Restarting...")

        print("Galaxy: Password set. Preparing frpc tunnel...")
        if not setup_frpc():
          print("Galaxy: FRPC setup failed. Retrying later...")
          time.sleep(10)
          continue

        start_auth_server()

        frpc_toml = GALAXY_DIR / "frpc.toml"
        config = f"""\
serverAddr = "galaxy.firestar.link"
serverPort = 7000

[transport]
tls.enable = true
poolCount = 2

[[proxies]]
name = "{slug}_pond"
type = "http"
localIP = "127.0.0.1"
localPort = 8082
customDomains = ["{slug}.devices.local"]
transport.useCompression = true

[[proxies]]
name = "{slug}_auth"
type = "http"
localIP = "127.0.0.1"
localPort = {AUTH_PORT}
customDomains = ["auth-{slug}.devices.local"]
"""
        frpc_toml.write_text(config)

        print(f"Galaxy: Starting frpc tunnel (slug: {slug[:4]}...)...")
        log_file = open(FRPC_LOG, 'a')
        process = subprocess.Popen(
          [str(GALAXY_DIR / "frpc"), "-c", str(frpc_toml)],
          stdout=log_file,
          stderr=log_file
        )
        last_slug = slug
    else:
      if process is not None and process.poll() is None:
        print("Galaxy: Password cleared. Stopping frpc tunnel...")
        cleanup_frpc()
        last_slug = None

    time.sleep(3)


if __name__ == "__main__":
  main()
