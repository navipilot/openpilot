# Navipilot / CP搭子 (CPlink) - 智能化 Comma3 导航数据桥梁

<div align="center">

[![GitHub Release](https://img.shields.io/github/v/release/jixiexiaoge/openpilot)](https://github.com/jixiexiaoge/openpilot/releases)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Android%208.0%2B-green.svg)](https://developer.android.com)

**快速安装地址：** `binge.fit/cp`

</div>

---

## 📖 目录 (Table of Contents)

- [项目简介 (Introduction)](#项目简介)
- [核心能力 (Core Capabilities)](#-核心能力)
- [设备要求 (Requirements)](#️-设备要求)
- [功能对比 (Feature Comparison)](#-功能对比一览)
- [进阶专业特性 (Advanced Features)](#-进阶专业特性)
- [下载与安装 (Download & Install)](#-下载与安装)
- [使用指南 (User Guide)](#-使用指南)
- [常见问题 (FAQ)](#-常见问题)
- [技术规格 (Technical Specs)](#-技术规格)
- [支持项目 (Support)](#-支持项目)
- [English Documentation](#english-documentation)

---

## 项目简介

一款由 [JixieXiaoGe](https://github.com/jixiexiaoge) 开发的专为 Comma3 设备与 CarrotPilot 深度打造的 Android "导航数据桥接"应用。

**CP搭子 (CPlink)** 是现阶段唯一完美支持从高德地图读取实时导航数据、交通灯倒计时及电子眼信息，并无缝同步至 Comma3 设备的专业级方案。

### ✨ 核心优势

- 🎯 **唯一性**：国内唯一支持高德地图的 Comma3 导航桥接应用
- 🚀 **实时性**：毫秒级数据同步，零延迟响应
- 📡 **完整性**：支持 65+ 种电子眼类型，覆盖所有交通场景
- 🔧 **易用性**：自动设备发现，无需手动配置
- 💪 **稳定性**：长期实际路测验证，后台运行稳定可靠

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

## 📥 下载与安装

### 方式一：Comma3 设备直接安装
在 Comma3 设备浏览器中访问：
```
binge.fit/cp
```

### 方式二：GitHub 下载
访问 [GitHub Releases](https://github.com/jixiexiaoge/openpilot/releases) 页面下载最新版本 APK 文件。

### 安装步骤

1. **手机端准备**
   - 确保 Android 版本 ≥ 8.0
   - 安装最新版本高德地图
   - 下载并安装 CP搭子 APK

2. **首次启动配置**
   - 打开 CP搭子应用
   - 授予必要权限（位置、网络）
   - 等待自动连接 Comma3 设备

3. **连接验证**
   - 确保手机与 Comma3 在同一 WiFi 网络
   - 应用显示"已连接"状态即可使用

---

## 📱 使用指南

### 快速开始

1. **启动导航**
   - 打开高德地图，设置目的地并开始导航
   - 打开 CP搭子应用

2. **自动连接**
   - 应用会自动搜索并连接 Comma3 设备
   - 连接成功后，导航数据自动同步

3. **开始驾驶**
   - Comma3 自动接收导航数据
   - 实时显示转弯提示、限速信息、电子眼警告等

### 主要功能操作

- **🏠 一键回家**：点击主界面"回家"按钮（需先在高德地图设置家庭地址）
- **🏢 一键去公司**：点击"公司"按钮（需先在高德地图设置公司地址）
- **⚙️ 高阶功能**：点击"高阶"按钮访问速度调整、变道控制等功能（需赞助者权限）

### 详细功能说明

了解更多与完整功能清单，请参见：
- 📖 [CP搭子 - 应用功能详细说明](./app功能说明.md)

---

## ❓ 常见问题

### Q1: 应用显示"搜索设备中..."，一直连接不上？
**解决方案：**
- ✅ 确认 Comma3 设备已开机并运行 CarrotPilot
- ✅ 检查手机和 Comma3 是否在同一 WiFi 网络
- ✅ 确认防火墙未阻止端口 7705 和 7706
- ✅ 尝试重启应用或重新连接 WiFi

### Q2: 为什么收不到高德地图的数据？
**解决方案：**
- ✅ 确保高德地图正在导航（必须开始导航才会发送数据）
- ✅ 使用最新版本高德地图
- ✅ 检查手机是否允许应用后台运行
- ✅ 授予应用所有必要权限

### Q3: 区间限速不显示？
**解决方案：**
- ✅ 确认导航路线中确实有区间测速路段
- ✅ 使用高德地图完整版（非简化版）
- ✅ 检查应用与 Comma3 的连接状态

### Q4: GPS 坐标显示为 0？
**解决方案：**
- ✅ 检查是否授予了位置权限
- ✅ 确认手机 GPS 功能已开启
- ✅ 在户外空旷地带测试（室内 GPS 信号弱）

### Q5: 应用在后台会自动停止？
**解决方案：**
- ✅ 授予应用后台运行权限
- ✅ 在手机设置中将应用加入白名单
- ✅ Android 10+ 需要授予后台位置权限
- ✅ 关闭电池优化功能

更多问题请查看详细文档或加入用户群咨询。

---

## 🔧 技术规格

### 系统要求

| 项目 | 要求 |
|------|------|
| **Android 版本** | 8.0 (API 26) 或更高 |
| **高德地图版本** | 支持标准广播接口的版本 |
| **Comma3 系统** | CarrotPilot 兼容系统 |
| **网络要求** | WiFi（手机与设备同一网络） |

### 数据传输规格

| 参数 | 规格 |
|------|------|
| **传输协议** | UDP（用户数据报协议） |
| **数据格式** | JSON（CarrotMan 协议） |
| **发送频率** | 200ms（约每秒 5 次） |
| **数据包大小** | 通常 < 2KB |
| **监听端口** | 7705（设备发现）、7706（数据传输） |

### 支持的数据类型

- ✅ **GPS 位置信息**：实时经纬度、方向、速度、精度
- ✅ **道路限速**：当前道路限速值自动识别
- ✅ **电子眼信息**：65+ 种摄像头类型（测速、闯红灯、区间测速等）
- ✅ **转弯导航**：完整 TBT（Turn-By-Turn）数据
- ✅ **目的地信息**：名称、地址、坐标、剩余距离/时间
- ✅ **交通灯状态**：红绿灯状态、倒计时、方向信息

---

## ☕ 支持项目

如果您觉得 CP搭子 对您有帮助，欢迎通过以下方式支持项目开发：

### 加入用户社群

添加微信管理员加入 CarrotPilot 爱好者群：
- 微信号：`carrotpilot-JX`
- 微信号：`wbk18820232680`

### 赞助支持

您的支持将帮助我们：
- 🚀 开发更多新功能
- 🐛 修复 Bug 和优化性能
- 📚 完善文档和教程
- 💬 提供更好的技术支持

<div align="center">

**微信赞助**

![微信赞助](/wechat.jpg "微信赞助与入群")

**PayPal 赞助**

![PayPal 赞助](/paypal.png "Donate via PayPal")

</div>

---

## 📄 版本信息

- **当前版本**：2.3
- **最后更新**：2026-02-25
- **开发者**：[JixieXiaoGe](https://github.com/jixiexiaoge)

---

## 📜 许可证

本项目采用开源许可证发布。使用前请仔细阅读相关条款。

---

## 🙏 致谢

感谢所有支持和使用 CP搭子 的用户，您的反馈和建议是我们不断改进的动力！

特别感谢：
- CarrotPilot 团队提供的优秀系统平台
- 高德地图提供的导航数据支持
- 所有赞助者和贡献者的支持

---

<div id="english-documentation"></div>

# 🌐 Navipilot / CPlink - Professional Comma3 Navigation Bridge

<div align="center">

[![GitHub Release](https://img.shields.io/github/v/release/jixiexiaoge/openpilot)](https://github.com/jixiexiaoge/openpilot/releases)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Android%208.0%2B-green.svg)](https://developer.android.com)

**Quick Installation:** `binge.fit/cp`

</div>

## 📋 Table of Contents

- [Introduction](#introduction)
- [Core Capabilities](#-core-capabilities)
- [System Requirements](#-system-requirements)
- [Feature Comparison](#-feature-comparison)
- [Advanced Features](#-advanced-professional-features)
- [Download & Install](#-download--installation)
- [Quick Start Guide](#-quick-start-guide)
- [Frequently Asked Questions](#-frequently-asked-questions)
- [Technical Specifications](#-technical-specifications)
- [Support the Project](#-support-the-project)

---

## Introduction

A professional-grade Android "Navigation Data Bridge" application specifically crafted for Comma3 hardware and CarrotPilot software, developed by [JixieXiaoGe](https://github.com/jixiexiaoge).

**CPlink (CP搭子)** stands as the most robust solution for relaying real-time AMAP (AutoNavi) navigation streams, traffic light countdowns, and safety enforcement data directly to your Comma3 device.

### ✨ Key Advantages

- 🎯 **Unique Solution**: The only Comma3 navigation bridge supporting AMAP (AutoNavi) in China
- 🚀 **Real-time Performance**: Millisecond-level data synchronization with zero latency
- 📡 **Comprehensive Coverage**: Support for 65+ camera types covering all traffic scenarios
- 🔧 **User-Friendly**: Automatic device discovery without manual configuration
- 💪 **Highly Stable**: Extensively road-tested with reliable background operation

---

## 🛡️ Core Capabilities

- **Unified Navigation Data**: Real-time synchronization of routes, road names, and high-precision Turn-By-Turn (TBT) guidance
- **Safety Enforcement (SDI)**: Comprehensive alerts for speed limits, fixed cameras, red-light cameras, and point-to-point (P2P) enforcement
- **V2I Traffic Signal**: Direct integration with real-time traffic signal status and precision green-light countdowns
- **High-Frequency GPS**: Millisecond-level location syncing ensures zero latency in high-speed maneuvering
- **Tri-Mode Mutex Engine**: Seamlessly toggles between OSM, AMAP, and Tencent SDK modes without data conflicts

---

## 📊 System Requirements

### Hardware Requirements
- **Mobile Device**: Android 8.0+ smartphone with AMAP app installed
- **Comma3 Device**: Comma3 hardware with CarrotPilot software
- **Network**: Both devices must be on the same local WiFi network

### Software Requirements
- AMAP (AutoNavi) navigation app
- CPlink (CP搭子) Android application
- Location and network permissions granted

---

## 📊 Feature Comparison

Comparison of capabilities between carrotman (Korean original), **CPlink (CP搭子)**, and CarrotAmap:

| Feature | carrotman (Korean) | **CPlink (CP搭子)** | CarrotAmap |
| :--- | :---: | :---: | :---: |
| **Navigation Data Source** | TMAP / Waze | **AMAP / Tencent Maps** | AMAP Car Version |
| **Real-time Traffic Lights** | ✅ | ✅ | ❌ |
| **Speed/Lane Control** | 🟡 Voice | ✅ **Deep Integration** | ❌ |
| **Lane Change Assistance** | ❌ | ✅ | ❌ |
| **One-Touch Speed Limit** | ❌ | ✅ | ❌ |
| **Turn-by-Turn (TBT) Info** | ✅ Complete | ✅ **High Precision** | 🟡 Partial |
| **Camera/Speed Trap Info** | ✅ Complete | ✅ **All Types** | 🟡 Partial |
| **C3 Manager/Settings** | ✅ Complete | ✅ | ❌ |
| **Auto-Connect to C3** | ✅ | ✅ | ✅ |
| **One-Touch Navigation** | ❌ | ✅ | ✅ |
| **Background Stability** | ✅ | ✅ **Excellent** | 🟡 Occasional Crashes |
| **Modify C3 Settings** | ✅ Complete | ✅ | ❌ |
| **Send Phone GPS** | ✅ | ✅ **High Frequency** | ✅ |

> *Legend: ✅ Supported | 🟡 Partial/In Progress | ❌ Not Supported*

---

## 🌟 Advanced Professional Features

### 1. Driving Behavior Engine
Real-time 5D analytics measuring Smoothness, Anticipation, Reliance, Efficiency, and NOO stability. The system automatically generates driving style labels (e.g., Smooth, Aggressive) and provides dark-themed Apple-style driving reports.

### 2. Context-Aware Experimental Mode
Real-time monitoring of driving conditions (curves, vehicles ahead, low speed) with automatic switching between Chill and Experimental modes via REST API (Port 7000).

### 3. Crowd-Sourced Speed Mapping
Built-in AI learning algorithm that automatically corrects road speed limits using driving data, solving the pain point of missing speed limit data in open-source maps.

---

## 📥 Download & Installation

### Method 1: Direct Installation on Comma3
Visit in Comma3 device browser:
```
binge.fit/cp
```

### Method 2: GitHub Download
Download the latest APK from [GitHub Releases](https://github.com/jixiexiaoge/openpilot/releases)

### Installation Steps

1. **Mobile Device Preparation**
   - Ensure Android version ≥ 8.0
   - Install latest version of AMAP
   - Download and install CPlink APK

2. **First Launch Configuration**
   - Open CPlink application
   - Grant necessary permissions (location, network)
   - Wait for automatic connection to Comma3 device

3. **Connection Verification**
   - Ensure phone and Comma3 are on same WiFi network
   - App displays "Connected" status when ready

---

## 🚀 Quick Start Guide

### Getting Started

1. **Start Navigation**
   - Open AMAP, set destination, and begin navigation
   - Launch CPlink application

2. **Automatic Connection**
   - App automatically searches for and connects to Comma3 device
   - Once connected, navigation data syncs automatically

3. **Start Driving**
   - Comma3 automatically receives navigation data
   - Real-time display of turn prompts, speed limits, camera warnings, etc.

### Main Features

- **🏠 One-Touch Home**: Tap "Home" button on main screen (requires home address set in AMAP)
- **🏢 One-Touch Work**: Tap "Work" button (requires work address set in AMAP)
- **⚙️ Advanced Functions**: Tap "Advanced" button for speed adjustment, lane change control, etc. (requires sponsor permissions)

### Detailed Documentation

For complete feature list and detailed instructions:
- 📖 [CPlink - Detailed Feature Guide](./app功能说明.md) (Chinese)

---

## ❓ Frequently Asked Questions

### Q1: App shows "Searching for device..." but won't connect?
**Solution:**
- ✅ Confirm Comma3 device is powered on and running CarrotPilot
- ✅ Check that phone and Comma3 are on same WiFi network
- ✅ Ensure firewall is not blocking ports 7705 and 7706
- ✅ Try restarting app or reconnecting WiFi

### Q2: Not receiving AMAP data?
**Solution:**
- ✅ Ensure AMAP is actively navigating (data only sent during navigation)
- ✅ Use latest version of AMAP
- ✅ Check that phone allows background app operation
- ✅ Grant all necessary permissions

### Q3: Interval speed limit not showing?
**Solution:**
- ✅ Confirm route actually has interval speed cameras
- ✅ Use full version of AMAP (not lite version)
- ✅ Check connection status with Comma3

### Q4: GPS coordinates showing as 0?
**Solution:**
- ✅ Verify location permissions are granted
- ✅ Confirm phone GPS is enabled
- ✅ Test in outdoor open area (indoor GPS signal is weak)

### Q5: App stops in background?
**Solution:**
- ✅ Grant background operation permissions
- ✅ Add app to phone whitelist
- ✅ Android 10+ requires background location permission
- ✅ Disable battery optimization

For more questions, check detailed documentation or join user community.

---

## 🔧 Technical Specifications

### System Requirements

| Item | Requirement |
|------|-------------|
| **Android Version** | 8.0 (API 26) or higher |
| **AMAP Version** | Version supporting standard broadcast interface |
| **Comma3 System** | CarrotPilot compatible system |
| **Network** | WiFi (phone and device on same network) |

### Data Transmission Specs

| Parameter | Specification |
|-----------|---------------|
| **Protocol** | UDP (User Datagram Protocol) |
| **Data Format** | JSON (CarrotMan protocol) |
| **Send Frequency** | 200ms (approximately 5 times per second) |
| **Packet Size** | Typically < 2KB |
| **Ports** | 7705 (device discovery), 7706 (data transmission) |

### Supported Data Types

- ✅ **GPS Position**: Real-time latitude/longitude, direction, speed, accuracy
- ✅ **Road Speed Limits**: Automatic recognition of current road speed limits
- ✅ **Camera Information**: 65+ camera types (speed cameras, red-light cameras, interval speed monitoring, etc.)
- ✅ **Turn Navigation**: Complete TBT (Turn-By-Turn) data
- ✅ **Destination Info**: Name, address, coordinates, remaining distance/time
- ✅ **Traffic Signals**: Traffic light status, countdown, direction information

---

## ☕ Support the Project

If you find CPlink helpful, please support the project development:

### Join User Community

Add WeChat administrator to join CarrotPilot enthusiast group:
- WeChat: `carrotpilot-JX`
- WeChat: `wbk18820232680`

### Sponsor Support

Your support helps us:
- 🚀 Develop new features
- 🐛 Fix bugs and optimize performance
- 📚 Improve documentation and tutorials
- 💬 Provide better technical support

<div align="center">

**WeChat Sponsorship**

![WeChat Sponsorship](/wechat.jpg "WeChat Sponsorship & Community")

**PayPal Sponsorship**

![PayPal Sponsorship](/paypal.png "Donate via PayPal")

</div>

---

## 📄 Version Information

- **Current Version**: 2.3
- **Last Updated**: 2026-02-25
- **Developer**: [JixieXiaoGe](https://github.com/jixiexiaoge)

---

## 📜 License

This project is released under an open-source license. Please read the terms carefully before use.

---

## 🙏 Acknowledgments

Thanks to all users who support and use CPlink. Your feedback and suggestions are our driving force for continuous improvement!

Special thanks to:
- CarrotPilot team for the excellent system platform
- AMAP (AutoNavi) for navigation data support
- All sponsors and contributors for their support

---