# Tesla 自动转向灯移植与测试报告

日期：2026-10-23

版本说明：第 1–7 节记录初始移植时的本机验证及边界，随后该版本已发布为 `73fb3a6b5`。第 8 节记录实车不亮灯反馈后的复现；第 9 节记录 2026-10-24 的本地灯控修复，第 10 节记录历史三项安全测试失败的解释及测试适配。后两项尚未完成目标车辆验收，提交/发布状态见下一段。

发布说明（2026-10-24）：按用户要求，将第 9、10 节修复一起提交并发布至 `navipilot/openpilot:try`，以前版 `73fb3a6b5` 为基线；各节“未提交/未推送”为对应测试时点的状态。实际发布 SHA 及是否成功以远端核验和交付消息为准。本次不包含原有 Hyundai DBC、字体改动，不新增固件保护，也不代表已部署、刷写或完成实车验收。

## 1. 结论与交付边界

已在 carrot 实现：使用既有导航/ATC 转弯、变道意图，向受支持的 Tesla 请求左右转向灯，保持至既有动作结束，再取消自身请求。包含自动灯来源隔离、真实拨杆优先、反馈确认、独立失联看门狗、有界取消重试和 Panda 安全约束。

**本次完成的是代码移植、本机合成/回归测试及固件构建，不是实车认证。功能默认关闭，不能据此宣称所有 Tesla 年款均可正常使用。目标车辆必须先完成封闭场地验收。**

| 项目 | 结果 |
|---|---|
| 主机端灯控、意图、拨杆、车型、启动门禁 | 221 passed，另有 23 subtests passed |
| Tesla 安全层完整回归 | 107 passed，3 failed，38 skipped；另有 105 subtests passed |
| 上述 3 项失败的原版对照 | 使用未修改的 HEAD Tesla 安全代码，复现同样 3 项失败 |
| 排除已有失败后的安全回归 | 107 passed，37 skipped，4 deselected |
| 新代码及聚焦修改的 Ruff | 通过；card/modeld 原有 25 条诊断无新增 |
| 原生 Params、Cap'n Proto C++ | 构建成功；新参数默认关闭及隔离目录读写通过 |
| F4/H7 普通及 Tesla-wake 固件 | 四种均成功构建、开发签名 |
| 原有固件选择测试及实际文件签名 | 4 passed；四个本地构建的选择与 128 字节签名读取均通过 |
| 本地 Wiki | 三语说明补丁应用、重新生成；595 个 Markdown 文件验证通过 |
| 用户文档映射、diff 空白检查 | 通过 |
| 实车、部署、刷写、Git 提交/推送、远端 Wiki 发布 | 均未进行 |

完整安全回归**不是全绿**。已有失败未被改成跳过、删除断言或通过修改转向安全策略来掩盖。

## 2. 基线、范围与支持条件

- 参考仓库：`/home/jeff/xpilot`，分支 `dev20261018-slgame-jetlink`，HEAD `67936d9e0555b8187a6df405f5a50de69924626a`。
- 目标仓库：`/home/jeff/carrot`，分支 `Carrot`，基线 `214e7c24f7635201e19e970b465413a2a6ea10b2`。
- 保留 carrot 原有手机导航入口、CarrotMan、ATC、变道状态机、转弯判断、BSD/扭矩/车道线门禁、启动准备及红绿灯起步限制。
- 未移植 xpilot 的 NavAssist 私有导航协议、Jetlink、模型、SP 参数体系、氛围灯或诊断 UI。手机仍通过 carrot 已有方式提供导航；不是让 xpilot 专用 App 协议直接兼容 carrot。
- 未修改 Hyundai 行为。任务开始前已有的两份 Hyundai DBC 和四份字体文件修改保持原样，未清理其他已有生成文件。

启用必须同时满足：

1. Model 3/Y；Model X 仍不支持本功能，未解除原有 dashcam-only 限制。
2. 检测到额外 vehicle CAN 总线：bus 1 的 `0x3DF`。
3. 用户启用 `TeslaAutoTurnSignals`，并重启以重新生成 CarParams。
4. 存在有效、足够新鲜的 `SCCM_leftStalk` 真实拨杆信号。没有拨杆或使用其他信号布局的年款不能仅靠开关获得支持。
5. 有真实、校验正确的 idle `DAS_bodyControls` 模板，UI 和物理灯反馈有效、初始双侧关闭。
6. 最终横向控制处于有效状态，车辆 CAN 有效、D 挡，modelV2/carControl 新鲜且有效，驾驶员未操作拨杆、未开双闪。

缺少这些条件时不会通过伪造反馈、猜测驾驶员状态或放宽起步/转向权限来强行开启。

## 3. 实现清单

| 文件 | 改动 |
|---|---|
| `openpilot/selfdrive/car/tesla_turn_signals.py` | 独立灯控执行器：会话、原始 CAN、模板、发送回显、取消、超时和诊断 |
| `openpilot/selfdrive/controls/lib/desire_helper.py` | 输出灯光方向/请求编号；剥离自动灯的驾驶员归因；完成消费与防重触发 |
| `openpilot/selfdrive/modeld/modeld.py` | 仅按 Tesla 配置启用 producer，发布既有 modelV2 的追加字段 |
| `openpilot/selfdrive/car/card.py` | 接收真实反馈；独立取消检查；在最终启动/横向门禁后追加灯光 CAN |
| `opendbc_repo/opendbc/car/tesla/carstate.py` | 可选真实拨杆接收、新鲜度、左右按压计数；同批次按下/释放不丢失 |
| `opendbc_repo/opendbc/car/tesla/interface.py`、`values.py` | Model 3/Y、vehicle bus、参数开关门禁；独立 CP flag 32 / safety flag 8 |
| `opendbc_repo/opendbc/car/car.capnp` | 追加 autoTurnSignalDirection/Status、turnSignalDriverValid/Direction |
| `openpilot/cereal/log.capnp` | 追加 model meta 的方向及 UInt64 请求编号；不更改旧字段编号 |
| `opendbc_repo/opendbc/safety/safety/safety_tesla_turn_signals.h` | 真实模板约束、可修改位、计数/校验、会话和取消重试限制 |
| `opendbc_repo/opendbc/safety/safety/safety_tesla.h` | 独立权限接入；保留 Summon、转向、纵向、AEB 等原规则 |
| `openpilot/common/params_keys.h`、`openpilot/selfdrive/carrot_settings.json` | 注册默认关闭的持久参数；接入启动设置菜单和中英韩说明 |
| `docs/user/{en,ko}/{settings,tesla}.md` | 默认值、生效方式、能力要求、诊断和安全边界 |
| `tools/docs/wiki_settings/patches/tesla-auto-turn-signals.patch`、`README.md` | 三语 Wiki MANUAL 区域补丁及本地应用方式；不发布远端 |

新增测试文件：

- `openpilot/selfdrive/car/tests/test_tesla_turn_signals.py`
- `openpilot/selfdrive/car/tests/test_tesla_signal_card.py`
- `openpilot/selfdrive/controls/tests/test_tesla_auto_signal_intent.py`
- `opendbc_repo/opendbc/car/tesla/tests/test_auto_turn_signals.py`
- `opendbc_repo/opendbc/safety/tests/test_tesla_turn_signals.py`

为运行实际 C 安全回归，修正了 `tests/libsafety/safety.c` 的主机夹具兼容性：补足生产驱动已有的 `safety_tx_buffered_for_fwd` 符号，并将 Python 测试的旧 bus/address 转发 ABI 转接到现有 packet hook。没有修改生产转发规则。原夹具直接构建会出现缺失符号，随后旧转发 ABI 会导致段错误；修正后转发测试正常执行。

## 4. 行为与保护

### 意图与来源隔离

转弯使用 carrot 已有 turn desire；变道使用既有 pre/starting/finishing 状态，finishing 期间不提前取消。准备转弯可在既有导航方向和距离窗口内预亮灯，但不因此授予转向权限。

每段新方向请求有独立编号，同一持续请求保持编号不变；完成后的等待状态不重复开灯，导航意图变化或真正开始下一次既有变道后才重新考虑。模型进程新实例使用单调时间初始化编号。

`leftBlinker/rightBlinker` 保留真实车辆灯反馈。新增所有权字段仅用于 DesireHelper 的驾驶员输入归因：自动灯及取消尾巴不能把左侧 ATC 的等待状态错误推进为变道，也不能在转弯结束后触发第二次变道。真实拨杆方向和按压计数仍可确认驾驶员操作。

### CAN 执行与确认

- bus 1，`0x3E9`，8 字节；左请求 1、右请求 2、取消 3。
- 仅从校验正确的原厂 idle 帧生成命令，保留非灯控字段；仅改变 request、reason、counter、checksum。
- 每个接收模板仅消费一次；计数器取原始值加一并按 4 bit 回绕。
- 匹配 bus 1 的发送回显 `0x81`；`0xC1` 拒绝反馈不是成功。
- 真实 UI：bus 0，`0x311`，校验 checksum；物理灯：bus 1，`0x3F5`。
- 关闭确认需要 UI 与物理灯双侧均为 OFF，持续 0.5 秒且反馈保持新鲜。闪烁的灭相位、FAULT、SNA、UI 无效值均不能假装关闭成功。
- 真实拨杆、同批次快速按压或双闪出现后取消自动请求并释放自动所有权；手动接管状态不是“已确认物理熄灯”。

### 超时与安全层

| 限制 | 当前值 |
|---|---|
| 原始模板最长年龄 | 1.5 秒 |
| 控制/意图上下文最长年龄 | 0.2 秒 |
| UI/物理灯反馈最长年龄 | 0.5 秒 |
| 单次发送等待回显 | 1.2 秒 |
| 亮灯确认等待 | 2.5 秒 |
| 单次自动会话 | 60 秒 |
| 取消确认等待 | 5 秒 |
| 取消尝试 | 最多 3 次 |
| Panda 活跃帧数量 | 每会话最多 1200 帧 |

上下文看门狗在 CAN 接收侧独立推进，controlsd/modeld 停止更新后也可请求取消，不依赖新的有效控制消息到来。Panda 取消重试允许在已接收首次取消、但主机丢失回显时进行，仍受 3 次/5 秒边界约束。

Panda 必须拥有独立 feature bit、正确 bus/长度/标准帧、真实新鲜 idle 模板、匹配非所属位、counter 和 checksum 才允许发送。反方向必须先取消；取消在 `controls_allowed` 为 false 时仍可执行，但不能绕过 Summon 或继电器故障限制。旧 feature flag 组合仍拒绝新增灯控输出。

取消无法确认或被拒绝时记录 fault，停止后续自动输出至下一次创建 onroad 控制实例；所有权保留至真实 OFF 稳定，避免尾巴误触发。**这些逻辑不能在设备断电、card 停止运行或 CAN 断线后凭空发送关闭命令，也不能保证车辆物理灯必然关闭。**

## 5. 测试环境与复现命令

所有本机验证均先进入 carrot，通过用户指定的 `op venv`：

```bash
cd /home/jeff/carrot
op venv
python -c 'import sys; print(sys.executable)'
# /home/jeff/carrot/.venv/bin/python
```

自动化 shell 使用交互 bash 加载 `op` alias，再在该 venv 的子 shell 执行以下命令；没有绕过此要求直接使用系统 Python。缺失的测试依赖 `parameterized==0.9.0` 已安装至此 venv；局域网代理当时不可达，改用直接连接安装成功。未更改依赖清单。

### 主机端与原有门禁

```bash
PYTHONDONTWRITEBYTECODE=1 PYTEST_DISABLE_PLUGIN_AUTOLOAD=1 \
python -m pytest -q -o addopts= -p no:cacheprovider \
  --confcutdir=openpilot/selfdrive \
  openpilot/selfdrive/car/tests/test_tesla_turn_signals.py \
  openpilot/selfdrive/car/tests/test_tesla_signal_card.py \
  openpilot/selfdrive/controls/tests/test_tesla_auto_signal_intent.py \
  openpilot/selfdrive/controls/tests/test_desire_helper_stalk.py \
  opendbc_repo/opendbc/car/tesla/tests/test_auto_turn_signals.py \
  opendbc_repo/opendbc/car/tesla/tests/test_tesla.py \
  openpilot/selfdrive/controls/tests/test_lateral_readiness.py \
  openpilot/selfdrive/controls/tests/test_lateral_startup_control.py
```

结果：**221 passed，23 subtests passed**。覆盖左右完整会话、稳定熄灯、超时/无回显/拒绝、驾驶员/双闪接管、失联独立取消、故障锁存、所有权尾巴、schema 实际 reader 与 sendcan 字节序列化、真实 DBC 拨杆解析、Model 3/Y/X 能力组合、finishing 保持及完成消费。

card 测试执行生产 `controls_update` 方法的完整 AST，替换外部 I/O，不启动设备进程；验证启动未就绪、ImpactDashcamReboot、过期 model/control 均不能发送开灯命令。它不是实车或完整多进程回放。

### C 安全回归

必须先重建库，不能使用旧 `libsafety.so`。本次使用现有 `tests/libsafety/SConscript`，通过会话目录中的最小 SConstruct 调用；等价的可复现编译为：

```bash
gcc -c -fPIC -Wall -Werror -nostdlib -fno-builtin -std=gnu11 \
  -Wfatal-errors -Wno-pointer-to-int-cast \
  -Iopendbc_repo/opendbc/safety/tests/libsafety \
  -Iopendbc_repo/opendbc/safety/board \
  -Iopendbc_repo/opendbc/safety \
  opendbc_repo/opendbc/safety/tests/libsafety/safety.c \
  -o opendbc_repo/opendbc/safety/tests/libsafety/safety.os
gcc -shared opendbc_repo/opendbc/safety/tests/libsafety/safety.os \
  -o opendbc_repo/opendbc/safety/tests/libsafety/libsafety.so

PYTHONDONTWRITEBYTECODE=1 PYTEST_DISABLE_PLUGIN_AUTOLOAD=1 \
python -m pytest -q -o addopts= -p no:cacheprovider \
  --confcutdir=opendbc_repo/opendbc/safety/tests \
  opendbc_repo/opendbc/safety/tests/test_tesla_turn_signals.py \
  opendbc_repo/opendbc/safety/tests/test_tesla.py
```

新增 10 项安全测试全部通过，包括默认拒绝、所有 flag 组合、wrong bus/长度/修改位/计数/校验、模板过期和消费、换向、时间/数量上限、取消重试、extended/returned/rejected 模板、Summon 和继电器故障。

全量结果：**107 passed，3 failed，38 skipped，105 subtests passed**。失败项均为既有 `test_angle_cmd_when_disabled`，分别位于 `TestTeslaStockSafety`、`TestTeslaLongitudinalSafety`、`TestTeslaAutoSpeedLimitSafety`。

为区分回归，另建隔离 C 库，将 Tesla header 替换为 `git show HEAD:opendbc_repo/opendbc/safety/safety/safety_tesla.h` 的原文，其余生产规则及同一主机夹具不变，重新编译并运行这三项：**同样 3 failed**。因此未修改本任务无关的转向安全行为。使用 `-k 'not angle_cmd_when_disabled'` 得到 **107 passed，37 skipped，4 deselected**；第四个 deselected 是基类收集项，不是额外失败。

pytest 的三条 unknown-config warning 来自禁用非必要插件后仍存在的 asyncio/cpp 配置选项，不计为逻辑通过或失败。

### 原生构建、参数与静态检查

```bash
scons -Q -j2 openpilot/common/params_pyx.so openpilot/cereal/libcereal.a \
  panda/board/obj/panda.bin.signed \
  panda/board/obj/panda_h7.bin.signed \
  panda/board/obj/panda_tesla_wake.bin.signed \
  panda/board/obj/panda_h7_tesla_wake.bin.signed
git diff --check
```

四种固件均按现有 `-Wall -Wextra -Werror` 构建成功。`pandad.get_firmware_path()` 的原有选择逻辑不变：TeslaWakeOnCAN 决定普通或 wake 变体，MCU 决定 F4/H7。新增安全规则包含在四种构建中，不能只部署 Python 而继续使用不匹配固件。

另运行 `openpilot/selfdrive/pandad/tests/test_tesla_wake_firmware.py`：4 passed。使用仅提供 MCU 信息的假对象、隔离 FakeParams 和四个实际本地固件文件验证路径及签名读取，四种组合均通过，没有实例化 Panda 或访问车辆。临时脚本重复序列化同一 CarParams builder 时产生一条 pycapnp write-flag 提醒，不影响断言结果。

新 Params 已用隔离临时目录验证：key 注册、默认 False、True/False 写入及读回均成功；未修改本机实际驾驶参数。

新增 Python 和聚焦 Tesla/Desire 文件 Ruff 通过。对 card.py/modeld.py 另与 HEAD 原文逐项比较诊断：分别存在 22/3 条旧诊断，本次无新增；不宣称整个旧文件 lint 全绿。未运行全仓测试或设备 GPU 模型构建。

本地最终固件 SHA-256：

| 文件（`panda/board/obj/`） | 字节数 | SHA-256 |
|---|---:|---|
| `panda.bin.signed` | 58984 | `64292266a38fa3294251da0158b5d89faefc03a23b942fa6fff99a51a1955803` |
| `panda_h7.bin.signed` | 88372 | `b5affaec91d40f29ba5afc217ecfea4bf4c3a68be503fd769d9c09b7916db100` |
| `panda_tesla_wake.bin.signed` | 58996 | `003f2f82cf37bd265db3ab9b877cd21dc73d4ad33f199d9d9c864acb714a5e47` |
| `panda_h7_tesla_wake.bin.signed` | 88384 | `0089f4aff3a891523b171be15ab7bd0d49863ed1515b97590b9dd2742e06e326` |

## 6. 设置、文档和诊断

开关路径：Driving / Start & Auto / Engage 中的 Tesla Automatic Turn Signals，对应 `TeslaAutoTurnSignals`，默认 `0`。开关不立即改变已经运行的 CarParams，修改后需重启。

诊断字段：

- `carState.autoTurnSignalDirection`：自动会话/取消尾巴仍拥有的方向。
- `carState.autoTurnSignalStatus`：idle、active、waiting、cancelling、cancelled、driver_takeover 或 fault 原因。
- `modelV2.meta.autoTurnSignalDirection/autoTurnSignalRequestId`：上游请求。
- `tesla_auto_turn_signal` cloudlog 事件：状态变化记录。

KO/EN 用户指南及三语设置说明已同步，参数总数更新为 190。Wiki 仅在会话目录生成：先生成三语页面，应用本仓库三语 MANUAL 补丁，再以该目录作为 `--wiki-dir` 重新生成；对 Markdown 页面验证为 **595 file(s) passed**。`Settings-Catalog.json` 是索引而不是单条内容文档，未错误地交给单条内容 schema 验证。Wiki review 标记仍未认证，未推送远端。

原始本机测试输出保存在本会话 `files/` 下的 `tesla-host-result.log`、`tesla-safety-result.log`、`tesla-safety-full-result.log`、`tesla-baseline-result.log`、`tesla-build-result.log`。这是当前工作树的验证记录，不是已发布版本的证明。

## 7. 实车验收前置与剩余风险

默认保持关闭。在目标车的封闭测试场地、具备随时人工接管条件时，至少确认：

1. 实际 bus 布局、idle 模板、拨杆枚举、UI 与物理灯编码和刷新率与支持条件一致；不一致则停止启用，而非放宽门禁。
2. 导航左/右转、左/右变道、准备阶段、finishing、完成、取消的真实灯光和回读一致；左侧原有人工确认规则未被自动灯绕过。
3. 同侧/反侧拨杆、快速按压、双闪、连续路口及下一次变道能按驾驶员意图接管，没有反复重亮或尾巴触发。
4. 未就绪、停车挡、退出横向、CAN/模型/控制消息中断、无回显、灯故障时不发新的自动开启请求；取消失败显示诊断并停止重试。
5. 实际安装的普通/wake、F4/H7 固件与该代码一致；操作前按既有维护流程处理，不把本机开发构建当作已刷写。

尚未验证具体车辆响应、固件与 Tesla 版本兼容、无拨杆年款、真实手机 App 端到端路线、离线日志的不同单调时钟及断电行为。拨杆新鲜度采用运行设备单调时钟，离线回放若不对齐 CAN 时间会保守禁用，不能把本机夹具测试称作实车回放通过。

**必须由实车验收补齐“目标车可正常使用”的证据；本报告没有保证断线/断电时车辆一定熄灯，也没有保证安全回归的原有失败已经解决。**

## 8. 实车不亮灯反馈后的排查

用户反馈：开关开启，7706 的 `carrotCmd=LANECHANGE` 左右变道与导航左右转弯均能执行，但转向灯不亮，Comma 3 和车辆无报错。本节针对已发布源码 `73fb3a6b533bde2ebb198c1f83973f3795fd782a` 做只读分析；没有取得该次实车 rlog、运行时 CarParams 或 Panda 状态，不能据此认定目标车根因已经确认或修复。

### 本机已复现的阻断点与实现缺陷

所有执行均先进入 `carrot`，使用 `op venv`。复现使用真实 DBC、当前参数生成逻辑和已有 DesireHelper/执行器夹具，没有连接车辆或发送真实 CAN。

| 检查项 | 实际复现结果 | 结论边界 |
|---|---|---|
| 参数开启，但 fingerprint 的 bus1 缺少 `0x3DF` | `CP.flags=4`，自动灯功能位关闭，`safetyParam=0`；相同夹具加入该消息后为 `flags=44`、`safetyParam=8` | 开关开启不代表运行时准入成功。这些数值来自合成 fingerprint，不是用户车辆读数 |
| 实体拨杆消息来源 | 同一 DBC 的 idle `0x249` 在 bus0 时 `turnSignalDriverValid=True`；只在 bus1 时为 False，party 时间戳为 0 | 当前解析固定读取 bus0。两个 DBC 都有该消息，不证明用户车辆实际上在哪条总线上发送 |
| 拨杆无效时的灯请求 | 即使模拟有效的导航方向、控制和灯反馈，执行器发送 0 帧，状态仍为 `idle` | 有静默阻断缺陷；无界面报错不能证明灯控链路正常 |
| 真实 App 短命令的完整起始阶段 | LEFT/RIGHT 均在第 0–3 帧有对应灯方向；第 4 帧起方向为 0，但原状态机仍为 `laneChangeStarting` | App 请求约 0.2 秒到期会错误地结束灯意图。不同于原状态机仍继续执行的变道生命周期 |
| CAN 时间域偏移 | 将反馈时间设置为执行器时间之后 30 秒时，发送 0 帧、状态为 `waiting_for_vehicle_feedback` | 新鲜度检查依赖时间域一致。此为合成风险检查，不代表用户设备存在该偏移 |

主要代码位置：

- `opendbc_repo/opendbc/car/tesla/interface.py`：`has_vehicle_bus` 与 CP/safety 功能位。
- `opendbc_repo/opendbc/car/tesla/carstate.py`：仅从 `cp_party` 读取 `SCCM_leftStalk`，以及拨杆有效性和计数。
- `openpilot/selfdrive/controls/lib/desire_helper.py`：App 的 0.2 秒计数，以及 `_update_auto_signal` 对当前 `atc_enabled` 的持续依赖。
- `openpilot/selfdrive/car/tesla_turn_signals.py`：`update` 准入；尚未创建会话时 `_cancel` 直接返回，未记录具体阻断原因。
- `openpilot/selfdrive/car/card.py`：灯状态进入 carState/cloudlog，但没有相应的用户界面告警。

重新运行现有意图、Tesla 参数/拨杆、执行器和 card 四个测试模块，结果为 **76 passed，3 条既有 pytest 配置警告**。这些原测试仍能通过，同时上述缺陷能够复现，说明之前覆盖不足：意图测试以持续导航意图为主，没有覆盖真实 App 短命令的完整生命周期；拨杆测试只注入 bus0，执行器夹具默认拨杆有效。不能用这些通过结果宣称目标车可正常亮灯。

### 应先用现有实车日志区分的公共阻断点

变道/转弯由 carrot 原有链路驱动，自动灯另走独立准入和 CAN 链路，所以“车辆完成了动作”不能证明自动灯已发出请求。两个场景都完全不亮，优先检查公共链路；App 的短命令问题不能单独解释导航也不亮。

1. 核对实际安装版本、开关修改后是否重启，以及本次运行的 `CarParams.flags & 32`、`safetyConfigs[].safetyParam & 8`。再比对实际 `pandaStates` 的安全模式和参数；本机构建成功不等于车端已刷入匹配固件。
2. 核对原始接收 CAN 中 bus1 的 `0x3DF`、`0x3E9`、`0x3F5`，以及 bus0 的 `0x311`。原有转向控制可用不代表 vehicle CAN 已接入或映射正确；缺少必需总线时不能靠删门禁解决。
3. 同时核对 `0x249` 的实际来源、刷新率和枚举，以及 `carState.turnSignalDriverValid/Direction`。没有实体拨杆的年款不能假定该信号存在，需要明确对应的驾驶员输入协议。
4. 对齐 `modelV2.meta.autoTurnSignalDirection/autoTurnSignalRequestId`、`carControl.latActive`、`carState.canValid/gearShifter` 和 `autoTurnSignalStatus`；检查 `sendcan` 是否真正含有 bus1 `0x3E9`。
5. 已有发送时，区分同报文的 bus1 TX echo（src `0x81`）、拒绝回报（`0xC1`），以及真实 UI/物理灯变化。TX echo 只证明报文发出，不证明车辆接受或灯已亮。

取证优先使用本次已有日志；不要为了补日志再次在公共道路试验，不要绕过安全门禁或用原始 CAN 命令试点灯。

### 最小修复方案与当前状态

1. 将 App 灯请求绑定到原状态机已接受的变道会话，锁存来源、方向和请求 ID，按既有状态机的完成/取消条件结束，而不是随 0.2 秒命令消失。保留原有盲区、驾驶员确认、横向退出等条件，不能用自动灯反过来批准变道。
2. 依据实际车辆拓扑修正真实拨杆解析。若证实信号在 vehicle bus，则显式注册和解析该来源，保留校验、新鲜度、按压计数及驾驶员优先；不能直接令 `turnSignalDriverValid=True` 或拿自动灯回读代替驾驶员输入。
3. 将未启用、拨杆缺失/过期、模板缺失、UI/前灯反馈缺失、控制/模型过期、Panda 拒绝等原因分别诊断。避免在没有会话时一律留在 `idle`，也不能把“开关已开”显示成“功能已就绪”。
4. 增加左右 App 短命令贯穿 starting/finishing/完成/取消的回归，以及实际来源拨杆、消息缺失/陈旧、固件不匹配和逐层阻断的回归；复核时间域。之后才进行匹配固件的台架和封闭场地验收。

**本轮仅更新排查报告，没有修改灯控实现、刷写设备或推送分支。软件缺陷已复现，但该车的公共阻断点仍待实际日志确认，不能宣称已修复。**

## 9. CAN1 拓展线链路修复（2026-10-24）

### 依据与检测原则

用户补充车辆已连接 CAN1 拓展线，现有自动模拟巡航滚轮功能可以使用。代码核对确认，两项功能**原本就共用** `interface.py` 中 `has_vehicle_bus = 0x3DF in fingerprint[CANBUS.vehicle]` 的检测，生成同一个 `TeslaFlags.HAS_VEHICLE_BUS`。不是自动灯遗漏了拓展线判断，也不能仅凭旧的 bus0 拨杆解析就推断没有接线。

自动滚轮发的是 CAN1 `0x3C2`，自动灯发的是 CAN1 `0x3E9`；两者只共享总线可用性判断，不共享报文内容或执行权限。保持现有识别标准，不将“任何 bus 上有消息”视为已接入拓展线。灯控还需独立的真实 idle 模板、UI/前灯反馈和 feature 权限；滚轮工作并不等于这些灯控条件全部满足。

### 已实施的最小修复

| 修改 | 结果与保护 |
|---|---|
| CAN1 真实拨杆来源 | 仅在灯功能启用时，为 vehicle parser 预注册可选 `SCCM_leftStalk`，不改变原有 UI/TPMS 必需消息。CAN1 一旦收到该信号即优先使用，未曾收到时兼容原 CAN0 来源；CAN1 过期后不偷偷切回另一总线的 idle 值 |
| 来源冲突与按压计数 | 新鲜来源方向冲突、无效枚举或另一来源出现无法对应的快速按压时，拒绝自动输出；半按到全按不重复计数，镜像输入及短暂无效恢复也不将同一次按压当作新请求。不伪造有效拨杆，不用灯反馈替代驾驶员输入 |
| App 变道生命周期 | 灯方向锁存到原状态机已经接受的 pre/starting/finishing 会话，而不是 0.2 秒命令脉冲。完成、取消、驾驶员操作或原有准入失效时释放；同一已取消会话不恢复重亮，下一条真正接受的 App 命令可获得新 ID |
| 导航准备到转弯 | 左/右 `turn … prepare` 切到同方向 `turn …` 时，原 ATC 会抑制动作切换一帧，旧灯意图因此错误归零。现在仅桥接已存在的同方向灯请求，保持 ID；原有转弯动作的一帧抑制仍然保留，不增加转向权限 |
| CAN 时间域 | C++ CAN 事件使用 `CLOCK_BOOTTIME`；仅灯控接收/发送时钟和真实拨杆新鲜度改用相同时钟（不支持的平台使用 `CLOCK_MONOTONIC`）。模型/控制的接收新鲜度仍使用 SubMaster 原有 monotonic 时间，不修改巡航滚轮或普通控制器时钟。REPLAY 的灯执行器仍用日志时间，未宣称完整车辆离线回放已通过 |
| 明确阻断原因 | 配置时区分开关关闭、车型不支持、passive、未识别拓展线、需要重启、CP 安全配置缺失。运行时区分拨杆不可用、控制/CAN/D 挡不满足、模板与 UI/前灯反馈缺失等。首次 Tesla 状态也记录 cloudlog，不再因没有会话而始终留在 `idle` |

CP 灯功能位仍为 32，Panda 独立权限仍为 8。自动灯不依赖 alpha longitudinal 或自动滚轮是否开启，避免把灯控错误绑定到纵向功能。现有滚轮发送逻辑、Panda 生产规则、报文格式、模板消费、取消与熄灯确认均未放宽；此次没有新增 schema、参数或固件修改。

### 当前诊断含义

状态从 `carState.autoTurnSignalStatus` 和 `tesla_auto_turn_signal` 事件读取，**不是新增屏幕告警**。

| 状态 | 应检查的环节 |
|---|---|
| `blocked:vehicle_bus_unavailable` | 本次启动是否在 CAN1 检测到 `0x3DF`，而非只看设置开关 |
| `blocked:restart_required` | 设置已开，但本次 CarParams 没有灯功能位，需按原流程重启重新识别 |
| `blocked:safety_configuration` | CP 请求的 Tesla 安全配置中缺少灯控权限；不代表已经验证设备固件 |
| `blocked:driver_input_unavailable` | `0x249` 来源、刷新率、枚举、双来源冲突；无拨杆或不同协议布局仍未自动支持 |
| `blocked:context_stale` / `blocked:lateral_inactive` / `blocked:can_invalid` / `blocked:not_in_drive` | 原有控制和新鲜度准入 |
| `waiting:ui_feedback` / `waiting:front_feedback` | 缺少新鲜 bus0 `0x311` / bus1 `0x3F5` |
| `waiting:body_template` | 缺少新鲜、正确校验的 bus1 idle `0x3E9` |
| `waiting:lamps_off` / `waiting:invalid_lamp_feedback` | 当前不具备双侧关闭且有效的起始灯状态 |
| `cancelling:panda_rejected` / `fault:panda_rejected_cancel` | 收到拒绝回报；需核对实际安全模式、固件及原始收发报文，不能视为车辆已执行 |

### 验证结果

均先进入 `carrot` 并执行 `op venv`。主机回归：

```bash
PYTHONDONTWRITEBYTECODE=1 PYTEST_DISABLE_PLUGIN_AUTOLOAD=1 python -m pytest -q \
  -o addopts= -p no:cacheprovider --confcutdir=/home/jeff/carrot \
  openpilot/selfdrive/controls/tests/test_tesla_auto_signal_intent.py \
  openpilot/selfdrive/controls/tests/test_desire_helper.py \
  openpilot/selfdrive/controls/tests/test_desire_helper_stalk.py \
  openpilot/selfdrive/controls/tests/test_lever_forced_turn.py \
  openpilot/selfdrive/controls/tests/test_lane_change_side_obstacles.py \
  openpilot/selfdrive/controls/tests/test_lateral_readiness.py \
  openpilot/selfdrive/controls/tests/test_lateral_startup_control.py \
  opendbc_repo/opendbc/car/tesla/tests \
  openpilot/selfdrive/car/tests/test_tesla_turn_signals.py \
  openpilot/selfdrive/car/tests/test_tesla_signal_card.py
```

结果：**351 passed，23 subtests passed**。覆盖左右 App 短命令贯穿整个状态机、350ms 后才有灯模板/反馈仍可请求、完成与驾驶员/控制/CAN/拖车取消后不重亮、下一条命令、导航准备到左右转弯、CAN1/兼容 CAN0 拨杆、同批次按压释放、来源冲突与镜像计数、无拓展线/错误 bus 的标记、拨杆缺失、配置/反馈阻断诊断，以及现有 Tesla 自动滚轮、转向与启动门禁。模拟暂停导致的两个时钟相差 5 秒时，card 仍使用 CAN 对应时钟；未把时间域差当作接收陈旧。

独立运行 `opendbc_repo/opendbc/safety/tests/test_tesla_turn_signals.py`：**10 passed**，沿用现有 C 测试库，包含 feature 位组合、真实模板、可修改位、总线/长度/校验、反向取消和有界重试。没有重新运行全量安全套件或重建固件；第 5 节原有 3 项转向安全失败没有在此次处理，不能将这 10 项通过说成全量安全全绿。

新增/修改的聚焦 Python Ruff 通过；`card.py` 对照当前 HEAD 的 22 条旧诊断无新增。pytest 仍有 3 条已有插件配置警告。EN/KO 用户指南及三语 Wiki 补丁同步；本地重新生成、应用补丁、再次生成后 **595 个 Markdown 文件验证通过**，用户文档映射检查通过。Wiki 未发布。

**当前交付是本地软件修复及上述本机证据，未提交、未推送、未部署或刷写。仍未取得目标车原始日志，不能证明它此前一定是 CAN1 拨杆问题，也不能保证此次修改后该车已能正常亮灯。设备仍须具备匹配的 Panda 灯控规则，并先完成台架/封闭场地验证，不应直接继续公共道路试验。**

## 10. 历史三项转向安全测试失败的核对与适配

### 同一个预期冲突，不是三个独立故障

三项均为继承自 `safety/tests/common.py` 的 `test_angle_cmd_when_disabled`：

- `TestTeslaStockSafety`：原厂纵向。
- `TestTeslaLongitudinalSafety`：openpilot 纵向。
- `TestTeslaAutoSpeedLimitSafety`：openpilot 纵向加自动巡航滚轮。

先在 `carrot` 中执行 `op venv`，运行既有 SCons C 库目标确认源码状态，再运行原三项测试，复现 **3 failed**。进一步完整枚举每种模式的 `controls_allowed`、转向请求位、19 个测量角度和 19 个请求角度，即每种模式 1444 个组合：三种模式均只有 **361 个 `controls_allowed=False` 且转向请求位为开**的组合与通用测试预期不同。请求位为 NONE 时的角度约束没有出现此类差异。

原因在本分支既有 `opendbc_repo/opendbc/safety/safety.h` 的 `steer_angle_cmd_checks`：局部 `aol_allowed` 固定为 true，允许横向独立于原厂巡航，仍执行角速率检查。通用测试却沿用“`controls_allowed=False` 时所有主动转向都应拒绝”的预期。这个测试把原厂巡航授权关闭与转向报文的 NONE 位混在了一起，与当前分支的独立横向策略不一致。主机 `controlsd.py` 也明确允许 `(selfdrive_active or always_lateral)` 后再检查横向开关、故障、车速、启动准备等条件。

**不能为了通过该通用测试就直接将 `aol_allowed` 改为 false，或在 Tesla 中无条件添加 `!controls_allowed` 禁止转向；这会改变本分支独立横向/全时横向的行为。**

### 本次修改及验证边界

此次只修改 `opendbc_repo/opendbc/safety/tests/test_tesla.py`：

1. 在 Tesla 测试基类中提供相同测试名的分支专属预期。保留完整的授权/请求位/测量角度/请求角度组合；主动请求在预置相同上一请求、满足角速率等条件时可以独立于巡航，NONE 请求仍必须贴合测量角度。逐次确认 TX 不改变巡航授权。
2. 扩展原有 inactive 检查，在巡航授权开/关两种状态下均验证测量角度限幅，以及正、负两个方向的错误关闭角度必须被拒绝。
3. 新增未开巡航时的角速率正反例，覆盖 0、1、5、10、25、50m/s，以及负角、零角、正角；每次单独设置上一请求，确保超速变化仍被拒绝，不能通过连续请求的副作用蒙混过关。
4. 新增横向/纵向隔离检查：独立横向报文被接受后，不得打开 `controls_allowed`，也不得因此允许非零纵向加速。

没有修改共享通用测试、生产安全代码、任何限值或权限位，也没有增加 skip/xfail 标记。**这是测试与既有分支策略的适配，不是新增 Panda 独立横向授权机制或固件安全加固。** 固件中的 `aol_allowed=true` 仍然存在，Panda 并没有因此开始独立读取主机的 `AlwaysLateral` 开关；主机横向门禁仍是现有架构的一部分。测试全绿不能被解释为这方面的架构风险已经消失。

### 最新验证结果

```bash
PYTHONDONTWRITEBYTECODE=1 PYTEST_DISABLE_PLUGIN_AUTOLOAD=1 python -m pytest -q \
  -o addopts= -p no:cacheprovider --confcutdir=/home/jeff/carrot \
  opendbc_repo/opendbc/safety/tests/test_tesla.py \
  opendbc_repo/opendbc/safety/tests/test_tesla_turn_signals.py
```

结果：**116 passed，40 skipped，339 subtests passed，0 failed**。没有用 `-k` 排除历史失败。40 个跳过包含既有抽象基类及不适用测试；相较原 38 个增加的 2 个来自基类新增的两种测试方法被收集，不是将失败改成跳过。仍有 3 条既有 pytest 插件配置警告。

为验证适配后不会放过真正的约束破坏，另在会话目录生成并编译两份隔离 C 库，未覆盖正常库或生产源码：

| 隔离故障注入 | 测试确实拒绝的例子 |
|---|---|
| 去掉 NONE 请求的测量角度检查 | 修正后的同名测试在测量 -90°、请求 -80°、转向请求位为关时断言失败 |
| 未开巡航时跳过主动转向角速率检查 | 新增角速率测试在 0m/s、上一请求 -90°、单次变化 +6.1° 时断言失败 |

这两个预期失败证明测试并非无条件接受当前返回值；它们不是正常代码的回归失败。隔离产物及结果保存在本会话 `files/tesla-disabled-mutants/`，正常 C 库未替换。

同时运行实际控制器反馈、协同转向、主机 `AlwaysLateral`/转向门禁及启动准备测试：**103 passed**。修改文件 Ruff 与 diff 空白检查通过。生产安全规则无变化，未为此次测试适配重新构建或刷写四种固件；第 9 节灯控仍要求设备具备匹配固件。

**截至本节，原三项测试失败已通过明确的分支策略适配消除；没有据此宣称车辆问题已全部解决、固件增加了新保护，或完成实车安全认证。本次仍未提交或推送。**
