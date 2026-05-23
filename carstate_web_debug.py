#!/usr/bin/env python3
"""
特斯拉 CarState 实时调试工具
==============================
- 订阅 carState ZMQ 消息
- Web UI (aiohttp, port 9000) 展示所有车辆数据
- SSE 实时推送更新
- 浏览器打开 http://localhost:9000 查看

用法:
  cd carrotpilot && python tools/carstate_web_debug.py
"""
import asyncio
import json
import time
from collections import OrderedDict

from aiohttp import web

from cereal import messaging

# ── 按功能分组的 CarState 字段定义 ─────────────────────────────────
FIELD_GROUPS = OrderedDict([
  ("车速", [
    ("vEgo", "车速 (滤波)", "m/s", ".2f"),
    ("aEgo", "加速度 (滤波)", "m/s²", ".3f"),
    ("vEgoRaw", "车速 (原始)", "m/s", ".2f"),
    ("vEgoCluster", "仪表盘车速", "m/s", ".2f"),
    ("vCluRatio", "车速比值", "", ".4f"),
    ("yawRate", "横摆角速度", "rad/s", ".3f"),
    ("standstill", "静止", "", "bool"),
  ]),
  ("巡航", [
    ("vCruise", "设定速度 (框架)", "m/s", ".2f"),
    ("vCruiseCluster", "仪表设定速度", "m/s", ".2f"),
    ("cruiseState.enabled", "巡航启用", "", "bool"),
    ("cruiseState.available", "巡航可用", "", "bool"),
    ("cruiseState.speed", "巡航速度", "m/s", ".2f"),
    ("cruiseState.speedCluster", "仪表巡航速度", "m/s", ".2f"),
    ("cruiseState.standstill", "巡航静止", "", "bool"),
    ("cruiseState.nonAdaptive", "非自适应", "", "bool"),
    ("activateCruise", "Carrot 激活巡航", "", "int"),
    ("pcmCruiseGap", "跟车距离", "档位", "int"),
    ("carrotCruise", "Carrot 巡航", "", "int"),
  ]),
  ("踏板 & 制动", [
    ("gas", "油门踏板", "%", ".1f"),
    ("gasPressed", "油门踩下", "", "bool"),
    ("brake", "制动踏板", "%", ".1f"),
    ("brakePressed", "制动踩下", "", "bool"),
    ("brakeLights", "制动灯", "", "bool"),
    ("regenBraking", "动能回收", "", "bool"),
    ("engineRpm", "发动机转速", "rpm", ".0f"),
  ]),
  ("转向", [
    ("steeringAngleDeg", "方向盘角度", "deg", ".1f"),
    ("steeringAngleOffsetDeg", "角度偏移", "deg", ".1f"),
    ("steeringRateDeg", "转向速度", "deg/s", ".1f"),
    ("steeringTorque", "方向盘扭矩", "Nm", ".2f"),
    ("steeringTorqueEps", "EPS 扭矩", "Nm", ".2f"),
    ("steeringPressed", "驾驶人操控", "", "bool"),
    ("steerFaultTemporary", "EPS 临时故障", "", "bool"),
    ("steerFaultPermanent", "EPS 永久故障", "", "bool"),
    ("invalidLkasSetting", "LKA 无效设置", "", "bool"),
    ("lowSpeedAlert", "低速告警", "", "bool"),
  ]),
  ("ESP & 车辆动态", [
    ("espDisabled", "ESP 关闭", "", "bool"),
    ("espActive", "ESP 激活", "", "bool"),
    ("vehicleSensorsInvalid", "传感器无效", "", "bool"),
    ("carFaultedNonCritical", "非关键故障", "", "bool"),
    ("accFaulted", "ACC 故障", "", "bool"),
    ("stockAeb", "原厂 AEB", "", "bool"),
    ("stockFcw", "原厂 FCW", "", "bool"),
  ]),
  ("档位 & 驻车", [
    ("gearShifter", "档位", "", "str"),
    ("gearStep", "档位步进", "", "int"),
    ("parkingBrake", "驻车制动", "", "bool"),
    ("brakeHoldActive", "Auto Hold", "", "bool"),
    ("softHoldActive", "Soft Hold", "", "int"),
  ]),
  ("灯光 & 信号", [
    ("leftBlinker", "左转向灯", "", "bool"),
    ("rightBlinker", "右转向灯", "", "bool"),
    ("genericToggle", "远光灯", "", "bool"),
    ("leftBlindspot", "左侧盲区", "", "bool"),
    ("rightBlindspot", "右侧盲区", "", "bool"),
  ]),
  ("车门 & 安全带", [
    ("doorOpen", "车门打开", "", "bool"),
    ("seatbeltUnlatched", "安全带未系", "", "bool"),
  ]),
  ("电池 & 胎压", [
    ("fuelGauge", "电量", "%", ".1f"),
    ("charging", "充电中", "", "bool"),
    ("tpms_fl", "左前胎压", "bar", ".0f"),
    ("tpms_fr", "右前胎压", "bar", ".0f"),
    ("tpms_rl", "左后胎压", "bar", ".0f"),
    ("tpms_rr", "右后胎压", "bar", ".0f"),
  ]),
  ("限速 & 导航", [
    ("speedLimit", "道路限速", "m/s", ".2f"),
    ("speedLimitDistance", "限速距离", "m", ".0f"),
    ("useLaneLineSpeed", "车道线限速", "", "bool"),
  ]),
  ("车道线", [
    ("leftLatDist", "左线横向距离", "m", ".2f"),
    ("rightLatDist", "右线横向距离", "m", ".2f"),
    ("leftLongDist", "左线纵向距离", "m", ".0f"),
    ("rightLongDist", "右线纵向距离", "m", ".0f"),
    ("leftLaneLine", "左线类型", "", "lane"),
    ("rightLaneLine", "右线类型", "", "lane"),
  ]),
  ("CAN & 系统", [
    ("canValid", "CAN 有效", "", "bool"),
    ("canTimeout", "CAN 超时", "", "bool"),
    ("canErrorCounter", "CAN 错误计数", "", "int"),
    ("cumLagMs", "累积延迟", "ms", ".1f"),
    ("buttonEnable", "启用按钮", "", "bool"),
    ("latEnabled", "横向启用", "", "bool"),
    ("datetime", "时间戳 (ns)", "", "int"),
  ]),
])

LANE_TYPE_MAP = {-1: "无车道", 0: "虚线", 1: "实线", 10: "白虚线", 11: "白实线",
                  20: "黄虚线", 21: "黄实线"}


def carstate_to_dict(cs) -> dict:
  """将 CarState capnp 结构体转为扁平 dict"""
  d = {}

  scalar_fields = [
    "canValid", "canTimeout", "canErrorCounter",
    "vEgo", "aEgo", "vEgoRaw", "vEgoCluster",
    "vCruise", "vCruiseCluster",
    "yawRate", "standstill",
    "gas", "gasPressed", "engineRpm",
    "brake", "brakePressed", "brakeLights", "regenBraking",
    "parkingBrake", "brakeHoldActive",
    "steeringAngleDeg", "steeringAngleOffsetDeg", "steeringRateDeg",
    "steeringTorque", "steeringTorqueEps", "steeringPressed",
    "steerFaultTemporary", "steerFaultPermanent",
    "invalidLkasSetting", "stockAeb", "stockFcw",
    "espDisabled", "accFaulted", "carFaultedNonCritical",
    "espActive", "vehicleSensorsInvalid", "lowSpeedAlert",
    "buttonEnable",
    "leftBlinker", "rightBlinker", "genericToggle",
    "doorOpen", "seatbeltUnlatched",
    "leftBlindspot", "rightBlindspot",
    "fuelGauge", "charging",
    "cumLagMs", "vCluRatio",
    "softHoldActive", "activateCruise", "latEnabled",
    "pcmCruiseGap",
    "speedLimit", "speedLimitDistance",
    "gearStep",
    "useLaneLineSpeed",
    "leftLatDist", "rightLatDist",
    "leftLongDist", "rightLongDist",
    "carrotCruise",
    "leftLaneLine", "rightLaneLine",
    "datetime",
  ]
  for name in scalar_fields:
    d[name] = getattr(cs, name, None)

  # gearShifter 枚举
  try:
    d["gearShifter"] = str(cs.gearShifter).rsplit(".", 1)[-1]
  except Exception:
    d["gearShifter"] = "unknown"

  # cruiseState 子结构
  csv = cs.cruiseState
  d["cruiseState.enabled"] = csv.enabled
  d["cruiseState.available"] = csv.available
  d["cruiseState.speed"] = csv.speed
  d["cruiseState.speedCluster"] = csv.speedCluster
  d["cruiseState.standstill"] = csv.standstill
  d["cruiseState.nonAdaptive"] = csv.nonAdaptive

  # tpms
  tp = cs.tpms
  d["tpms_fl"] = tp.fl
  d["tpms_fr"] = tp.fr
  d["tpms_rl"] = tp.rl
  d["tpms_rr"] = tp.rr

  return d


# ── 单一 HTML 页面 ────────────────────────────────────────────────
HTML_TPL = """<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="utf-8">
<title>Tesla CarState 实时调试</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
body{font-family:-apple-system,"Segoe UI",monospace;background:#0d1117;color:#c9d1d9;padding:20px}
h1{font-size:20px;margin-bottom:16px;display:flex;align-items:center;gap:12px}
h1 small{font-size:13px;color:#8b949e;font-weight:400}
.bar{display:flex;gap:20px;margin-bottom:20px;font-size:13px}
.bar .dot{display:inline-block;width:10px;height:10px;border-radius:50%;margin-right:6px}
.bar .dot.green{background:#3fb950}
.bar .dot.red{background:#f85149}
.bar .dot.gray{background:#484f58}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(380px,1fr));gap:12px}
.card{background:#161b22;border:1px solid #30363d;border-radius:8px;overflow:hidden}
.card h2{font-size:13px;padding:8px 14px;background:#1c2128;color:#8b949e;text-transform:uppercase;letter-spacing:.5px;border-bottom:1px solid #30363d}
.card table{width:100%;border-collapse:collapse;font-size:13px}
.card td{padding:5px 14px;border-bottom:1px solid #21262d}
.card tr:last-child td{border-bottom:none}
.card .lbl{color:#8b949e;width:55%}
.card .val{text-align:right;font-weight:600;font-variant-numeric:tabular-nums}
.card .val.on{color:#3fb950}
.card .val.off{color:#484f58}
.val .unit{font-weight:400;color:#8b949e;font-size:11px;margin-left:3px}
.flash{animation:fade 1.2s ease}
@keyframes fade{0%{background:rgba(63,185,80,.15)}100%{background:transparent}}
#err{display:none;background:#f85149;color:#fff;padding:8px 16px;border-radius:6px;margin-bottom:12px;font-size:13px}
</style>
</head>
<body>
<h1>Tesla CarState <small>实时调试面板</small></h1>
<div class="bar">
  <span><span class="dot gray" id="cDot"></span><span id="cTxt">等待连接...</span></span>
  <span>更新: <span id="fps">0</span> Hz</span>
  <span>延迟: <span id="lag">—</span></span>
</div>
<div id="err"></div>
<div class="grid" id="grid"></div>
<script>
const GROUPS = """ + json.dumps(list(FIELD_GROUPS.items()), ensure_ascii=False) + """;
const LANE = """ + json.dumps(LANE_TYPE_MAP, ensure_ascii=False) + """;
let last={}, lastT=0;
function mk(t,c){let e=document.createElement(t);if(c)e.className=c;return e}
function build(){
  let grid=document.getElementById('grid');
  GROUPS.forEach(function(grp){
    let card=mk('div','card'), h2=mk('h2'); h2.textContent=grp[0];
    let tbl=mk('table'), tbd=mk('tbody'); tbl.appendChild(tbd);
    grp[1].forEach(function(f){
      let tr=mk('tr'); tr.dataset.k=f[0];
      let l=mk('td','lbl'); l.textContent=f[1];
      let v=mk('td','val');
      if(f[2]){let u=mk('span','unit');u.textContent=f[2];v.appendChild(u)}
      tr.appendChild(l);tr.appendChild(v);tbd.appendChild(tr);
    });
    card.appendChild(h2);card.appendChild(tbl);grid.appendChild(card);
  });
}
function fix(v,f){
  if(v==null||v===undefined)return '—';
  if(f==='bool')return v?'ON':'OFF';
  if(f==='str')return String(v);
  if(f==='lane')return LANE[v]||String(v);
  if(f==='int')return String(Math.round(v));
  return v.toFixed(parseInt(f.split('.')[1])||2);
}
function cls(v,f){
  if(f!=='bool')return'';
  return v?'on':'off';
}
function upd(d){
  let n=Date.now(), dt=n-lastT; lastT=n;
  document.getElementById('fps').textContent=dt>0?(1000/dt).toFixed(1):'—';
  document.getElementById('lag').textContent=d.datetime?((n-d.datetime/1e6).toFixed(0)+' ms'):'—';
  GROUPS.forEach(function(grp){
    grp[1].forEach(function(f){
      let td=document.querySelector('tr[data-k="'+f[0]+'"] td.val');
      if(!td)return;
      let v=d[f[0]], o=last[f[0]];
      td.childNodes[0]&&td.childNodes[0].nodeType===3?td.childNodes[0].nodeValue=fix(v,f[3]):td.textContent=fix(v,f[3]);
      // remove then re-add .on/.off for bools
      if(f[3]==='bool'){td.classList.remove('on','off');td.classList.add(cls(v,f[3]))}
      if(o!==v){td.parentElement.classList.remove('flash');void td.parentElement.offsetWidth;td.parentElement.classList.add('flash')}
    });
  });
  Object.assign(last,d);
}
build();
var es=new EventSource('/events');
es.onmessage=function(e){
  try{var d=JSON.parse(e.data);
  document.getElementById('cDot').className='dot green';
  document.getElementById('cTxt').textContent='已连接';
  document.getElementById('err').style.display='none';
  upd(d);
  }catch(err){document.getElementById('err').textContent='解析错误: '+err.message;document.getElementById('err').style.display='block'}
};
es.onerror=function(){
  document.getElementById('cDot').className='dot red';
  document.getElementById('cTxt').textContent='连接断开';
  document.getElementById('err').textContent='连接断开，正在重连...';
  document.getElementById('err').style.display='block'
};
</script>
</body>
</html>"""

# ── aiohttp 服务 ──────────────────────────────────────────────────
async def sse_handler(request):
  """SSE 端点 — 推送 CarState 实时数据"""
  resp = web.StreamResponse(
    status=200, reason='OK',
    headers={
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive',
      'Access-Control-Allow-Origin': '*',
    },
  )
  await resp.prepare(request)

  sock = messaging.sub_sock('carState', conflate=True)
  loop = asyncio.get_event_loop()

  try:
    while True:
      msg = await loop.run_in_executor(None, messaging.recv_one_or_none, sock)
      if msg is not None:
        data = carstate_to_dict(msg.carState)
        payload = json.dumps(data, default=str)
        await resp.write(f"data: {payload}\n\n".encode())
      else:
        await resp.write(": heartbeat\n\n".encode())
      await asyncio.sleep(0.01)
  except (ConnectionResetError, ConnectionAbortedError):
    pass
  finally:
    sock.close()
  return resp


async def index_handler(request):
  return web.Response(text=HTML_TPL, content_type='text/html', charset='utf-8')


def main():
  app = web.Application()
  app.router.add_get('/', index_handler)
  app.router.add_get('/events', sse_handler)

  print("=" * 50)
  print("  Tesla CarState 实时调试面板")
  print(f"  启动时间: {time.strftime('%Y-%m-%d %H:%M:%S')}")
  print("  打开浏览器: http://localhost:9000")
  print("=" * 50)
  web.run_app(app, port=9000, access_log=None)


if __name__ == '__main__':
  main()
