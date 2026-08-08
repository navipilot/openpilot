# CP搭子 (CPlink) — Comma3 导航数据桥梁

<div align="center">

[![GitHub Release](https://img.shields.io/github/v/release/jixiexiaoge/openpilot)](https://github.com/jixiexiaoge/openpilot/releases)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

**安装地址：** `binge.fit/cp` · `navipilot/Carrotpilot` · `i.carrotpilot.app/c3-wip`

</div>

---

## 简介

**CP搭子** 是专为 Comma3 + CarrotPilot 打造的 Android 导航数据桥接应用，由 [JixieXiaoGe](https://github.com/jixiexiaoge) 开发。

核心定位：将高德车机版、腾讯导航、高德手机版、谷歌地图的数据稳定分发给 CarrotPilot，服务 NOO 场景。

---

## 主要功能

- 🗺️ **多导航源支持**：高德车机版 / 腾讯导航 / 高德手机版 / 谷歌地图
- 🔁 **NOO 数据分发**：导航路线、转向、状态稳定分发至 CarrotPilot
- 🧪 **自动实验模式切换**：根据路况自动切换，减少人工干预
- 🔀 **模型选择切换器**：快速切换适合当前车型与风格的模型
- 🌐 **7000 Web 控制台**：丰富的无屏化设备管理功能
- 💡 **端侧增强**：LED 屏幕、自动超车、驾驶报告持续完善

---

## 设备要求

| 项目 | 要求 |
|------|------|
| 手机系统 | Android 8.0+ |
| 车机设备 | Comma3 + CarrotPilot |
| 网络环境 | 手机与设备同一 WiFi |

---

## 安装方式

**方式一：Comma3 浏览器访问**
```
binge.fit/cp
```

**方式二：[GitHub Releases](https://github.com/jixiexiaoge/openpilot/releases) 下载 APK**

**安装步骤：**
1. 手机安装高德地图 + CP搭子 APK
2. 授予位置与网络权限
3. 确保手机与 Comma3 在同一 WiFi 网络
4. 打开应用，等待自动连接

---

## 常见问题

**Q：一直显示"搜索设备中..."？**
- 确认 Comma3 已开机并运行 CarrotPilot
- 确认手机与设备在同一 WiFi
- 检查防火墙是否放行端口 7000 / 7705 / 7706

**Q：收不到高德地图数据？**
- 确保高德地图正在导航状态
- 检查手机是否允许后台运行
- 授予所有必要权限

**Q：应用在后台自动停止？**
- 授予后台运行与后台位置权限
- 将应用加入省电白名单，关闭电池优化

---

## ☕ 赞助支持

如果 CP搭子 对您有帮助，欢迎赞助支持开发：

<div align="center">

**微信赞助**

![微信赞助](/wechat.jpg "微信赞助与入群")

**PayPal 赞助**

![PayPal 赞助](/paypal.png "Donate via PayPal")

</div>

加入用户社群：微信号 `carrotpilot-JX` 或 `wbk18820232680`

---

## 分支说明

| 分支 | 说明 |
|------|------|
| master | 主分支 |
| fishop | 现代、起亚国内特调版（飞扬开发） |
| Dragonpilot (DP) | 支持 C3 + C4，稳定版 |
| release | IQ Pilot 最新分支，特斯拉/大众/本田特调 |
| Carrotpilot (CP) | CP 最新分支，供 C3 使用 |
| carrotZ6 | 长安 Z6 专用 |

---

## 致谢

感谢 CarrotPilot 团队、高德地图，以及所有赞助者和用户的支持与反馈！
