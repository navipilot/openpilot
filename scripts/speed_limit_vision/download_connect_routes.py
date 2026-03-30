#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import re

from dataclasses import dataclass
from email.utils import parsedate_to_datetime
from pathlib import Path
from urllib.parse import urlparse

import requests

from scripts.speed_limit_vision import common


API_HOST = os.getenv("COMMA_API_HOST", "https://api.commadotai.com")
DEFAULT_FILES_MANIFEST = common.preferred_files_manifest_path()
STREAM_FILE_NAMES = {
  "fcamera": {"fcamera.hevc"},
  "qlog": {"qlog.zst", "qlog.bz2", "qlog"},
  "rlog": {"rlog.zst", "rlog.bz2", "rlog"},
  "qcamera": {"qcamera.ts"},
  "dcamera": {"dcamera.hevc"},
  "ecamera": {"ecamera.hevc"},
}


@dataclass(frozen=True)
class RouteRequest:
  dongle_id: str
  log_id: str
  segment_filter: set[int] | None

  @property
  def canonical_name(self) -> str:
    return f"{self.dongle_id}|{self.log_id}"


def parse_args() -> argparse.Namespace:
  parser = argparse.ArgumentParser(description="Download route files directly from comma connect into the speed-limit review layout.")
  parser.add_argument("routes", nargs="*", help="Route ids like 'dongle/logid' or segment ids like 'dongle/logid--9'.")
  parser.add_argument("--routes-file", type=Path, help="Optional file containing one route id per line.")
  parser.add_argument("--clip-root", type=Path, default=common.preferred_clip_root(), help="Destination root for downloaded segment directories.")
  parser.add_argument("--qlog-mtimes", type=Path, default=common.preferred_qlog_mtimes_path(), help="Path to qlog mtime manifest used by bookmark replay.")
  parser.add_argument("--files-manifest", type=Path, default=DEFAULT_FILES_MANIFEST, help="Path to downloaded-files manifest.")
  parser.add_argument("--streams", default="fcamera,qlog", help="Comma-separated stream set: fcamera,qlog,rlog,qcamera,dcamera,ecamera.")
  parser.add_argument("--segments", help="Optional segment filter, e.g. '0,2,5-8'.")
  parser.add_argument("--overwrite", action="store_true", help="Redownload files even if they already exist locally.")
  parser.add_argument("--timeout", type=float, default=60.0, help="HTTP timeout in seconds.")
  return parser.parse_args()


def read_token() -> str:
  token = os.getenv("COMMA_JWT", "").strip()
  if token:
    return token

  auth_path = Path.home() / ".comma" / "auth.json"
  if not auth_path.is_file():
    raise FileNotFoundError(f"Missing auth token at {auth_path}. Run python3 tools/lib/auth.py first or set COMMA_JWT.")
  auth = json.loads(auth_path.read_text(encoding="utf-8"))
  token = auth.get("access_token", "").strip()
  if not token:
    raise ValueError(f"No access_token in {auth_path}")
  return token


def parse_segment_spec(spec: str | None) -> set[int] | None:
  if not spec:
    return None

  selected: set[int] = set()
  for part in spec.split(","):
    part = part.strip()
    if not part:
      continue
    if "-" in part:
      start_text, end_text = part.split("-", 1)
      start = int(start_text)
      end = int(end_text)
      selected.update(range(min(start, end), max(start, end) + 1))
    else:
      selected.add(int(part))
  return selected


def load_route_inputs(args: argparse.Namespace) -> list[str]:
  raw_routes = list(args.routes)
  if args.routes_file:
    raw_routes.extend(line.strip() for line in args.routes_file.expanduser().resolve().read_text(encoding="utf-8").splitlines())
  if not raw_routes:
    raise ValueError("No routes provided.")
  return [route for route in raw_routes if route and not route.lstrip().startswith("#")]


def parse_route_request(raw: str, default_segments: set[int] | None) -> RouteRequest:
  text = raw.strip().strip("'\"")
  text = text.replace("|", "/")
  match = re.fullmatch(r"([0-9a-f]{16})/([^/]+)", text)
  if not match:
    raise ValueError(f"Unrecognized route id: {raw}")

  dongle_id = match.group(1)
  tail = match.group(2)
  segment_filter = set(default_segments) if default_segments else None

  parts = tail.split("--")
  if len(parts) == 3 and parts[-1].isdigit():
    log_id = "--".join(parts[:2])
    segment = int(parts[-1])
    if segment_filter is None:
      segment_filter = {segment}
    else:
      segment_filter.add(segment)
  else:
    log_id = tail

  if len(log_id) != 20:
    raise ValueError(f"Invalid log id in route: {raw}")
  return RouteRequest(dongle_id=dongle_id, log_id=log_id, segment_filter=segment_filter)


def merge_route_requests(raw_routes: list[str], default_segments: set[int] | None) -> list[RouteRequest]:
  merged: dict[tuple[str, str], set[int] | None] = {}
  for raw in raw_routes:
    request = parse_route_request(raw, default_segments)
    key = (request.dongle_id, request.log_id)
    if key not in merged:
      merged[key] = None if request.segment_filter is None else set(request.segment_filter)
      continue
    if merged[key] is None or request.segment_filter is None:
      merged[key] = None
    else:
      merged[key].update(request.segment_filter)
  return [RouteRequest(dongle_id=dongle_id, log_id=log_id, segment_filter=segments) for (dongle_id, log_id), segments in sorted(merged.items())]


def selected_file_names(streams_csv: str) -> set[str]:
  selected: set[str] = set()
  for stream in streams_csv.split(","):
    stream = stream.strip()
    if not stream:
      continue
    if stream not in STREAM_FILE_NAMES:
      raise ValueError(f"Unknown stream '{stream}'. Valid streams: {', '.join(sorted(STREAM_FILE_NAMES))}")
    selected.update(STREAM_FILE_NAMES[stream])
  return selected


def api_get_json(session: requests.Session, endpoint: str, timeout: float):
  response = session.get(f"{API_HOST}/{endpoint.lstrip('/')}", timeout=timeout)
  response.raise_for_status()
  return response.json()


def iter_route_file_urls(files_payload: dict, file_names: set[str]):
  for value in files_payload.values():
    if not isinstance(value, list):
      continue
    for url in value:
      parsed = urlparse(url)
      parts = parsed.path.strip("/").split("/")
      if len(parts) < 4:
        continue
      segment_text = parts[-2]
      file_name = parts[-1]
      if not segment_text.isdigit() or file_name not in file_names:
        continue
      yield int(segment_text), file_name, url


def download_to_path(session: requests.Session, url: str, dest_path: Path, overwrite: bool, timeout: float) -> int | None:
  dest_path.parent.mkdir(parents=True, exist_ok=True)
  if dest_path.exists() and not overwrite:
    return int(dest_path.stat().st_mtime)

  response = session.get(url, stream=True, timeout=timeout)
  response.raise_for_status()
  temp_path = dest_path.with_suffix(dest_path.suffix + ".part")
  with temp_path.open("wb") as handle:
    for chunk in response.iter_content(chunk_size=1 << 20):
      if chunk:
        handle.write(chunk)
  temp_path.replace(dest_path)

  last_modified = response.headers.get("Last-Modified")
  if last_modified:
    epoch = int(parsedate_to_datetime(last_modified).timestamp())
    os.utime(dest_path, (epoch, epoch))
    return epoch
  return int(dest_path.stat().st_mtime)


def load_manifest_lines(path: Path) -> dict[str, str]:
  if not path.is_file():
    return {}
  lines: dict[str, str] = {}
  for raw_line in path.read_text(encoding="utf-8").splitlines():
    line = raw_line.strip()
    if not line:
      continue
    key = line.split(" ", 1)[0]
    lines[key] = line
  return lines


def write_manifest(path: Path, lines: dict[str, str]) -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  with path.open("w", encoding="utf-8") as handle:
    for key in sorted(lines):
      handle.write(lines[key] + "\n")


def main() -> int:
  args = parse_args()
  token = read_token()
  route_requests = merge_route_requests(load_route_inputs(args), parse_segment_spec(args.segments))
  file_names = selected_file_names(args.streams)
  clip_root = args.clip_root.expanduser().resolve()
  qlog_mtimes_path = args.qlog_mtimes.expanduser().resolve()
  files_manifest_path = args.files_manifest.expanduser().resolve()

  api_session = requests.Session()
  api_session.headers.update({
    "Authorization": f"JWT {token}",
    "User-Agent": "OpenpilotTools",
  })
  download_session = requests.Session()
  download_session.headers.update({
    "User-Agent": "OpenpilotTools",
  })

  qlog_lines = load_manifest_lines(qlog_mtimes_path)
  file_lines = load_manifest_lines(files_manifest_path)

  for request in route_requests:
    route_meta = api_get_json(api_session, f"v1/route/{request.canonical_name}", timeout=args.timeout)
    files_payload = api_get_json(api_session, f"v1/route/{request.canonical_name}/files", timeout=args.timeout)
    log_id = request.log_id
    start_time = route_meta.get("start_time", "")
    print(f"{request.canonical_name}: downloading streams={sorted(file_names)} segments={sorted(request.segment_filter) if request.segment_filter else 'all'}")

    for segment, file_name, url in iter_route_file_urls(files_payload, file_names):
      if request.segment_filter is not None and segment not in request.segment_filter:
        continue

      segment_name = f"{log_id}--{segment}"
      segment_dir = clip_root / segment_name
      dest_path = segment_dir / file_name
      epoch = download_to_path(download_session, url, dest_path, args.overwrite, args.timeout)
      print(f"  {segment_name}/{file_name} <- {urlparse(url).netloc}")
      file_lines[f"{segment_name} {dest_path}"] = f"{segment_name} {dest_path}"
      if file_name.startswith("qlog") and epoch is not None:
        qlog_lines[str(dest_path)] = f"{dest_path} {epoch}"

    if start_time:
      print(f"  route start_time={start_time}")

  write_manifest(qlog_mtimes_path, qlog_lines)
  write_manifest(files_manifest_path, file_lines)
  print(f"Updated {qlog_mtimes_path}")
  print(f"Updated {files_manifest_path}")
  return 0


if __name__ == "__main__":
  raise SystemExit(main())
