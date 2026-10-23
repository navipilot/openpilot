# Tesla 自动转向灯移植与测试报告

日期：2026-10-23

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
