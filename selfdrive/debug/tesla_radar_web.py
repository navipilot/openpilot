#!/usr/bin/env python3
import argparse
import json
import threading
import time
from http import HTTPStatus
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

import cereal.messaging as messaging

from opendbc.can import CANParser
from openpilot.selfdrive.pandad import can_capnp_to_list


RADAR_BUS = 1
RADAR_START_ADDR = 0x410
RADAR_STATUS_ADDR = 0x401
RADAR_MSG_COUNT = 80
RADAR_TRIGGER_ADDR = RADAR_START_ADDR + RADAR_MSG_COUNT - 1
SIGNAL_TIMEOUT = 1.0
FRAME_TIMEOUT = 1.0

HTML = """<!doctype html>
<html lang="zh-CN">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Tesla Radar Debug</title>
  <style>
    :root {
      --bg: #0e1116;
      --panel: #171b22;
      --panel-2: #1f2530;
      --text: #e8edf5;
      --muted: #99a6b5;
      --good: #20c997;
      --warn: #ffb020;
      --bad: #ff5d5d;
      --line: #2d3644;
      --accent: #4db6ff;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      background: radial-gradient(circle at top, #162130, var(--bg) 50%);
      color: var(--text);
      font-family: Consolas, "Courier New", monospace;
    }
    .wrap {
      max-width: 1400px;
      margin: 0 auto;
      padding: 20px;
    }
    .top {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 12px;
      margin-bottom: 16px;
    }
    .card {
      background: linear-gradient(180deg, var(--panel), var(--panel-2));
      border: 1px solid var(--line);
      border-radius: 14px;
      padding: 14px 16px;
      box-shadow: 0 10px 30px rgba(0, 0, 0, 0.25);
    }
    .label {
      color: var(--muted);
      font-size: 12px;
      margin-bottom: 8px;
    }
    .value {
      font-size: 24px;
      font-weight: 700;
    }
    .good { color: var(--good); }
    .warn { color: var(--warn); }
    .bad { color: var(--bad); }
    .meta {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));
      gap: 12px;
      margin-bottom: 16px;
    }
    pre {
      margin: 0;
      white-space: pre-wrap;
      word-break: break-word;
      color: var(--muted);
      font-size: 12px;
      line-height: 1.5;
    }
    table {
      width: 100%;
      border-collapse: collapse;
      overflow: hidden;
      border-radius: 14px;
      background: linear-gradient(180deg, var(--panel), var(--panel-2));
      border: 1px solid var(--line);
    }
    th, td {
      padding: 10px 12px;
      border-bottom: 1px solid var(--line);
      text-align: right;
      font-size: 13px;
    }
    th:first-child, td:first-child { text-align: left; }
    th {
      position: sticky;
      top: 0;
      background: #121821;
      color: var(--accent);
    }
    tbody tr:hover { background: rgba(255, 255, 255, 0.03); }
    .empty {
      margin-top: 12px;
      padding: 18px;
      background: linear-gradient(180deg, var(--panel), var(--panel-2));
      border: 1px solid var(--line);
      border-radius: 14px;
      color: var(--muted);
    }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="top">
      <div class="card"><div class="label">雷达信号</div><div class="value" id="signal">--</div></div>
      <div class="card"><div class="label">完整帧</div><div class="value" id="frame">--</div></div>
      <div class="card"><div class="label">最近雷达包</div><div class="value" id="lastSeen">--</div></div>
      <div class="card"><div class="label">已解析目标数</div><div class="value" id="pointCount">0</div></div>
      <div class="card"><div class="label">最高报文地址</div><div class="value" id="highestAddr">--</div></div>
      <div class="card"><div class="label">缺失触发计数</div><div class="value" id="missingTrigger">0</div></div>
    </div>

    <div class="meta">
      <div class="card"><div class="label">状态</div><pre id="status"></pre></div>
      <div class="card"><div class="label">说明</div><pre id="explain"></pre></div>
    </div>

    <table>
      <thead>
        <tr>
          <th>Point</th>
          <th>dRel</th>
          <th>yRel</th>
          <th>vRel</th>
          <th>aRel</th>
          <th>yvRel</th>
          <th>Tracked</th>
          <th>Meas</th>
          <th>AddrA</th>
          <th>AddrB</th>
        </tr>
      </thead>
      <tbody id="points"></tbody>
    </table>
    <div class="empty" id="empty" style="display:none;">当前没有解析到有效雷达目标。</div>
  </div>

  <script>
    function cls(ok, warn) {
      return ok ? 'good' : (warn ? 'warn' : 'bad');
    }

    function fmtMs(sec) {
      if (sec === null) return '--';
      return (sec * 1000).toFixed(0) + ' ms';
    }

    function fmtNum(v) {
      return typeof v === 'number' ? v.toFixed(2) : '--';
    }

    async function refresh() {
      const resp = await fetch('/api/state', { cache: 'no-store' });
      const data = await resp.json();

      const signal = document.getElementById('signal');
      signal.textContent = data.signal_ok ? '有信号' : '无信号';
      signal.className = 'value ' + cls(data.signal_ok, false);

      const frame = document.getElementById('frame');
      frame.textContent = data.frame_ok ? '完整' : '未收齐';
      frame.className = 'value ' + cls(data.frame_ok, data.signal_ok && !data.frame_ok);

      document.getElementById('lastSeen').textContent = fmtMs(data.last_radar_seen_age_sec);
      document.getElementById('pointCount').textContent = String(data.points.length);
      document.getElementById('highestAddr').textContent = data.highest_addr_hex || '--';
      document.getElementById('missingTrigger').textContent = String(data.missing_trigger_count);

      document.getElementById('status').textContent = JSON.stringify(data.status, null, 2);
      document.getElementById('explain').textContent = data.explain;

      const tbody = document.getElementById('points');
      const empty = document.getElementById('empty');
      tbody.innerHTML = '';

      if (data.points.length === 0) {
        empty.style.display = 'block';
      } else {
        empty.style.display = 'none';
        for (const p of data.points) {
          const row = document.createElement('tr');
          row.innerHTML = `
            <td>${p.name}</td>
            <td>${fmtNum(p.dRel)}</td>
            <td>${fmtNum(p.yRel)}</td>
            <td>${fmtNum(p.vRel)}</td>
            <td>${fmtNum(p.aRel)}</td>
            <td>${fmtNum(p.yvRel)}</td>
            <td>${p.tracked}</td>
            <td>${p.measured}</td>
            <td>${p.addr_a}</td>
            <td>${p.addr_b}</td>
          `;
          tbody.appendChild(row);
        }
      }
    }

    refresh();
    setInterval(refresh, 500);
  </script>
</body>
</html>
"""


class TeslaRadarDebugState:
  def __init__(self):
    self.lock = threading.Lock()
    self.last_radar_seen_monotonic = 0.0
    self.last_frame_monotonic = 0.0
    self.highest_addr_seen = None
    self.missing_trigger_count = 0
    self.status = {
      "shortTermUnavailable": False,
      "sensorBlocked": False,
      "vehDynamicsError": False,
      "canValid": True,
    }
    self.points = []

  def snapshot(self):
    now = time.monotonic()
    with self.lock:
      signal_ok = (now - self.last_radar_seen_monotonic) < SIGNAL_TIMEOUT if self.last_radar_seen_monotonic > 0 else False
      frame_ok = (now - self.last_frame_monotonic) < FRAME_TIMEOUT if self.last_frame_monotonic > 0 else False
      highest_addr_hex = f"0x{self.highest_addr_seen:X}" if self.highest_addr_seen is not None else None
      return {
        "signal_ok": signal_ok,
        "frame_ok": frame_ok,
        "last_radar_seen_age_sec": None if self.last_radar_seen_monotonic == 0 else now - self.last_radar_seen_monotonic,
        "highest_addr_hex": highest_addr_hex,
        "missing_trigger_count": self.missing_trigger_count,
        "status": dict(self.status),
        "points": list(self.points),
        "explain": (
          "有信号: 最近 1 秒内至少看到一条 Tesla 雷达报文; 完整帧: 最近 1 秒内收到了触发地址 0x45F。"
          " 如果一直有信号但完整帧始终未收齐，通常说明雷达报文序列和代码假设不一致。"
        ),
      }


def build_messages():
  messages = [("RadarStatus", 16)]
  for i in range(RADAR_MSG_COUNT // 2):
    messages.append((f"RadarPoint{i}_A", 16))
    messages.append((f"RadarPoint{i}_B", 16))
  return messages


def collect_points(parser):
  points = []
  for i in range(RADAR_MSG_COUNT // 2):
    name_a = f"RadarPoint{i}_A"
    name_b = f"RadarPoint{i}_B"
    msg_a = parser.vl[name_a]
    msg_b = parser.vl[name_b]

    if msg_a["Index"] != msg_b["Index2"]:
      continue
    if not msg_a["Tracked"]:
      continue

    points.append({
      "name": f"P{i}",
      "dRel": float(msg_a["LongDist"]),
      "yRel": float(msg_a["LatDist"]),
      "vRel": float(msg_a["LongSpeed"]),
      "aRel": float(msg_a["LongAccel"]),
      "yvRel": float(msg_b["LatSpeed"]),
      "tracked": bool(msg_a["Tracked"]),
      "measured": bool(msg_a["Meas"]),
      "addr_a": f"0x{RADAR_START_ADDR + i * 2:X}",
      "addr_b": f"0x{RADAR_START_ADDR + i * 2 + 1:X}",
    })

  points.sort(key=lambda x: x["dRel"])
  return points


def radar_reader(state: TeslaRadarDebugState, dbc_name: str, addr: str):
  sock = messaging.sub_sock('can', addr=addr, timeout=100)
  parser = CANParser(dbc_name, build_messages(), RADAR_BUS)

  while True:
    can_strings = messaging.drain_sock_raw(sock, wait_for_one=True)
    can_list = can_capnp_to_list(can_strings)
    updated = parser.update(can_list)
    now = time.monotonic()

    radar_packets = [can for packet in can_list for can in packet if can.src == RADAR_BUS and RADAR_STATUS_ADDR <= can.address <= RADAR_TRIGGER_ADDR]
    if not radar_packets:
      continue

    highest_addr = max(can.address for can in radar_packets)
    full_frame = RADAR_TRIGGER_ADDR in updated
    radar_status = parser.vl['RadarStatus']
    points = collect_points(parser)

    with state.lock:
      state.last_radar_seen_monotonic = now
      state.highest_addr_seen = highest_addr
      state.status = {
        "shortTermUnavailable": bool(radar_status["shortTermUnavailable"]),
        "sensorBlocked": bool(radar_status["sensorBlocked"]),
        "vehDynamicsError": bool(radar_status["vehDynamicsError"]),
        "canValid": bool(parser.can_valid),
      }
      state.points = points
      if full_frame:
        state.last_frame_monotonic = now
        state.missing_trigger_count = 0
      else:
        state.missing_trigger_count += 1


class Handler(BaseHTTPRequestHandler):
  state = None

  def do_GET(self):
    if self.path in ('/', '/index.html'):
      body = HTML.encode('utf-8')
      self.send_response(HTTPStatus.OK)
      self.send_header('Content-Type', 'text/html; charset=utf-8')
      self.send_header('Content-Length', str(len(body)))
      self.end_headers()
      self.wfile.write(body)
      return

    if self.path == '/api/state':
      body = json.dumps(self.state.snapshot()).encode('utf-8')
      self.send_response(HTTPStatus.OK)
      self.send_header('Content-Type', 'application/json; charset=utf-8')
      self.send_header('Cache-Control', 'no-store')
      self.send_header('Content-Length', str(len(body)))
      self.end_headers()
      self.wfile.write(body)
      return

    self.send_error(HTTPStatus.NOT_FOUND)

  def log_message(self, format, *args):
    return


def main():
  parser = argparse.ArgumentParser(description='Tesla radar live web debugger')
  parser.add_argument('--port', type=int, default=7777, help='Web server port')
  parser.add_argument('--addr', default='127.0.0.1', help='messaging address')
  parser.add_argument('--dbc', default='tesla_radar_continental_generated', help='DBC name')
  args = parser.parse_args()

  state = TeslaRadarDebugState()
  Handler.state = state

  reader = threading.Thread(target=radar_reader, args=(state, args.dbc, args.addr), daemon=True)
  reader.start()

  server = ThreadingHTTPServer(('0.0.0.0', args.port), Handler)
  print(f'Tesla radar debug server listening on http://127.0.0.1:{args.port}')
  server.serve_forever()


if __name__ == '__main__':
  main()