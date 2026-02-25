# Navipilot / CP搭子 (CPlink) - 智能化 Comma3 导航数据桥梁

一款由 [JixieXiaoGe](https://github.com/jixiexiaoge) 开发的专为 Comma3 设备与 CarrotPilot 深度打造的 Android “导航数据桥接”应用。

**CP搭子 (CPlink)** 是现阶段唯一完美支持从高德地图读取实时导航数据、交通灯倒计时及电子眼信息，并无缝同步至 Comma3 设备的专业级方案。

---

## 🚀 核心能力

- **全维度导航数据**: 实时同步导航路线、道路名称、转弯 (TBT) 指示与距离。
- **电子眼与限速**: 完美支持道路限速、测速相机、闯红灯拍照、区间测速等全方位 SDI 提醒。
- **智能交通灯**: 独家支持交通灯状态监测及高精度绿灯倒计时显示。
- **高频 GPS 同步**: 毫秒级发送手机 GPS 位置及行驶方向，确保地图定位永不漂移。
- **三模式自动切换**: OSM (开源地图) / AMAP (高德模式) / Tencent (腾讯 SDK) 三模智能互斥，确保数据源唯一且稳定。

---

## 🛠️ 设备要求

- **移动端**: Android 8.0+ 手机，安装高德地图 APP 与 CP搭子。
- **设备端**: Comma3 硬件，已安装 CarrotPilot 软件。
- **网络**: 手机与设备必须处于同一本地 WiFi 网络环境下。

---

## 📊 功能对比一览

对比 carrotman（韩国原版）、CPlink（CP搭子）与 CarrotAmap 的主要能力：

| 功能 | carrotman (韩国原版) | **CPlink (CP搭子)** | CarrotAmap |
| :--- | :---: | :---: | :---: |
| **地图导航数据控车** | TMAP / Waze | **高德地图 / 腾讯地图** | 高德地图车机版 |
| **实时交通灯/红灯发送** | ✅ | ✅ | ❌ |
| **加减速/变道控制** | 🟡 语音 | ✅ **深度集成** | ❌ |
| **辅助变道提醒** | ❌ | ✅ | ❌ |
| **一键限速设定** | ❌ | ✅ | ❌ |
| **道路转弯信息 (TBT)** | ✅ 完整 | ✅ **高精度** | 🟡 部分 |
| **摄像头/电子眼信息** | ✅ 完整 | ✅ **全类型** | 🟡 部分 |
| **C3 管理器/设置修改** | ✅ 完整 | ✅ | ❌ |
| **自动连接 C3** | ✅ | ✅ | ✅ |
| **一键导航 (回家/公司)** | ❌ | ✅ | ✅ |
| **后台运行稳定性** | ✅ | ✅ **极高** | 🟡 存在闪退 |
| **修改 C3 设置** | ✅ 完整 | ✅ | ❌ |
| **发送手机 GPS 坐标** | ✅ | ✅ **高频** | ✅ |

> *图例：✅ 支持｜🟡 部分/进行中｜❌ 不支持*

---

## 🌟 进阶专业特性

### 1. 驾驶行为评分系统 (Driving Score)
基于五维建模的实时驾驶评分：平稳指数、预判指数、接管依赖、节能效率及 NOO 稳定度。系统会自动分析生成驾驶风格标签（如：平稳型、激进型），并提供暗色 Apple 风格的驾驶报告。

### 2. 条件实验模式 (Conditional Experiment)
实时监控行驶路况（如弯道、前车、低速），通过 REST API (Port 7000) 自动在 Chill 与 Experimental 模式间平滑切换。

### 3. 道路限速自学习 (Speed Limit Learner)
内置 AI 学习算法，通过行驶数据自动修正道路限速，解决开源地图限速数据缺失的痛点。

---

了解更多与完整功能清单：参见《[CP搭子 - 应用功能详细说明](./app功能说明.md)》

如果您愿意支持本项目，并想加入 CarrotPilot 爱好者群，欢迎添加微信管理员：`carrotpilot-JX` 或 `wbk18820232680`，我们会尽快邀请您入群。感谢您的支持与关注。

![](/wechat.jpg "微信赞助与入群")

---

# Navipilot / CPlink - Professional Comma3 Navigation Bridge

A professional-grade Android "Navigation Data Bridge" application specifically crafted for Comma3 hardware and CarrotPilot software. 

**CPlink (CP搭子)** stands as the most robust solution for relaying real-time AMAP (AutoNavi) navigation streams, traffic light countdowns, and safety enforcement data directly to your Comma3 device.

---

## 🛡️ Core Capabilities

- **Unified Navigation Data**: Real-time sync of routes, road names, and high-precision Turn-By-Turn (TBT) guidance.
- **Safety Enforcement (SDI)**: Comprehensive alerts for speed limits, fixed cameras, red-light cameras, and point-to-point (P2P) enforcement.
- **V2I Traffic Signal**: Direct integration with real-time traffic signal status and precision green-light countdowns.
- **High-Frequency GPS**: Millisecond-level location syncing ensures zero latency in high-speed maneuvering.
- **Tri-Mode Mutex Engine**: Seamlessly toggles between OSM, AMAP, and Tencent SDK modes without data conflicts.

---

## 📉 Advanced Pro Features

### 1. Driving Behavior Engine
Real-time 5D analytics (Smoothness, Anticipation, Reliance, Efficiency, NOO stability) with Apple-inspired visualization.

### 2. Context-Aware Experimental Mode
Automated toggling between "Chill" and "Experimental" modes based on real-time road geometry and traffic density via HTTP REST API.

### 3. Crowd-Sourced Speed Mapping
Intelligent speed limit learning system that fills metadata gaps in OSRM/Overpass data using the P85 speed percentile.

---

## 📥 Download & Install

- **Official Release**: [GitHub Releases](https://github.com/jixiexiaoge/openpilot/releases)
- **C3 URL Install**: `binge.fit/cp`

---

## ☕ Support the Project

Direct sponsorship fuels the development of new features and better stability. Scan the WeChat code above or use PayPal below to contribute.

![](/paypal.png "Donate via PayPal")

---

**Version**: 2.3 | **Last Updated**: 2026-02-25


