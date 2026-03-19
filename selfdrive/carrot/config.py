#!/usr/bin/env python3
"""
统一的参数管理模块
提供统一的接口访问系统参数和自定义参数，支持fallback机制
"""

import json
import os
from common.params import Params
from common.params_pyx import UnknownKeyName

class UnifiedParams:
    """统一的参数管理类，同时处理系统参数和自定义参数"""

    _instance = None
    _initialized = False

    def __new__(cls, nav_json_file=None):
        if cls._instance is None:
            cls._instance = super(UnifiedParams, cls).__new__(cls)
            cls._instance._initialized = False
        return cls._instance

    def __init__(self, nav_json_file=None):
        """初始化统一的参数管理器"""
        if self._initialized:
            return

        self.system_params = Params()

        if nav_json_file is None:
            current_dir = os.path.dirname(os.path.abspath(__file__))
            nav_json_file = os.path.join(current_dir, "nav_params.json")

        self.nav_json_file = os.path.realpath(nav_json_file)

        self.nav_data = {}
        self._load_nav_params()

        self._initialized = True

    def _match_system_param(self):
      # -----------------------------
      # 检查 system_params是否已有参数，有则使用系统参数
      # -----------------------------
      for key in list(self.nav_data.keys()):
        try:
          # 尝试从 system_params 读取，依类型猜测
          sys_val = None
          # 按 JSON 类型尝试读取
          if isinstance(self.nav_data[key], int):
            sys_val = self.system_params.get_int(key)
          elif isinstance(self.nav_data[key], float):
            sys_val = self.system_params.get_float(key)
          elif self.nav_data[key] in (0, 1):  # 布尔型（int表示）
            sys_val = self.system_params.get_bool(key)

          if sys_val is not None:
            self.nav_data[key] = sys_val  # 使用系统值覆盖 JSON 值
        except Exception:
          pass

    def _save_system_param(self):
      """
      尝试将 nav_data 中的参数写入 system_params。
      如果 system_params 中存在对应的 key，则覆盖系统参数。
      如果 system_params 中不存在该 key，则忽略（说明它是自定义参数）。
      """
      for key, value in list(self.nav_data.items()):
        try:
          # 根据 nav_data 的类型选择正确的 put 方法
          if isinstance(value, bool) or value in (0, 1):
            # bool（在 JSON 中通常表现为 0/1）
            self.system_params.put_bool(key, bool(value))
          elif isinstance(value, int):
            self.system_params.put_int(key, int(value))
          elif isinstance(value, float):
            self.system_params.put_float(key, float(value))
          else:
            # 原始值（字符串等）
            self.system_params.put(key, str(value))

        except (KeyError, AttributeError, UnknownKeyName):
          # system_params 不认识这个 key → 忽略
          pass
        except Exception:
          # 其他未知异常也忽略，避免影响主流程
          pass

    def _load_nav_params(self):
        """加载自定义参数数据"""
        try:
            if os.path.exists(self.nav_json_file):
                with open(self.nav_json_file, 'r', encoding='utf-8') as f:
                    self.nav_data = json.load(f)
                    self._match_system_param()
            else:
                self.nav_data = self._get_default_nav_data()
                self._match_system_param()
                self._save_nav_data()
        except json.JSONDecodeError as e:
            print(f"⚠️ 自定义配置文件格式错误，使用默认配置并重新创建: {e}")
            self.nav_data = self._get_default_nav_data()
            self._match_system_param()
            self._save_nav_data()
        except (OSError, IOError) as e:
            print(f"⚠️ 加载自定义参数{self.nav_json_file}失败: {e}")
            self.nav_data = self._get_default_nav_data()
            self._match_system_param()

    def _save_nav_data(self):
        """保存自定义参数到文件"""
        try:
            os.makedirs(os.path.dirname(self.nav_json_file), exist_ok=True)
            with open(self.nav_json_file, 'w', encoding='utf-8') as f:
                json.dump(self.nav_data, f, indent=2, ensure_ascii=False)
        except (OSError, IOError) as e:
            print(f"❌ 保存自定义参数失败: {e}")

    def _get_default_nav_data(self):
        """获取默认的自定义参数数据"""
        return {
            "AutoTurnDistOffset": 0,
            "AutoForkDistOffset": 30,
            "AutoDoForkBlinkerDist": 15,
            "AutoDoForkNavDist": 15,
            "AutoForkDistOffsetH": 1000,
            "AutoDoForkDecalDistH": 50,
            "AutoDoForkDecalDist": 20,
            "AutoDoForkBlinkerDistH": 30,
            "AutoDoForkNavDistH": 50,
            "AutoUpRoadLimit": 0,
            "AutoUpRoadLimit40KMH": 15,
            "AutoUpHighwayRoadLimit": 0,
            "AutoUpHighwayRoadLimit40KMH": 15,
            "RoadType": -1,
            "AutoForkDecalRateH": 80,
            "AutoForkSpeedMinH": 60,
            "AutoKeepForkSpeedH": 5,
            "AutoForkDecalRate": 80,
            "AutoForkSpeedMin": 45,
            "AutoKeepForkSpeed": 5,
            "ShowDebugLog": 0,
            "AutoCurveSpeedFactorH": 100,
            "AutoCurveSpeedAggressivenessH": 100,
            "SameSpiCamFilter": 1,
            "StockBlinkerCtrl": 0,
            "ExtBlinkerCtrlTest": 0,
            "BlinkerMode": 1,
            "LaneStabTime": 50,
            "DynamicBlindRange": 0,
            "DynamicBlindDistance":0,
            "DisableBlindSpot": 0,
            "BsdDelayTime": 20,
            "SideBsdDelayTime": 20,
            "SideRelDistTime": 10,
            "SidevRelDistTime": 10,
            "SideRadarMinDist": 0,

            "LidarBsdDelayTime": 10,
            "LidarFrontVDistTime": 10,
            "LidarFrontVRelDistTime": 30,
            "LidarBehindVDistTime": 10,
            "LidarBehindVRelDistTime": 30,
            "LaneLineDelayTime": 10,

            "AutoTurnInNotRoadEdge": 1,
            "ContinuousLaneChange": 1,
            "ContinuousLaneChangeCnt": 4,
            "ContinuousLaneChangeInterval": 2,
            "AutoTurnLeft": 1,
            "AutoEnTurnNewLaneTimeH": 0,
            "AutoEnTurnNewLaneTime": 0,
            "NewLaneWidthDiff": 8,
            "DynamicExperimentalSpeed": -5,
            "DynamicExperimentalLatA": 0,
        }

    def get_bool(self, key, default=False):
        """获取布尔值参数 - 统一接口"""
        # 优先尝试系统参数
        try:
            value = self.system_params.get_bool(key)
            if value is not None:
                return value
        except (KeyError, AttributeError, UnknownKeyName) as e:
            pass

        # 系统参数不存在，尝试自定义参数
        if key in self.nav_data:
            value = self.nav_data[key]
            try:
                return bool(int(value))
            except (ValueError, TypeError):
                return default

        return default

    def get_int(self, key, default=0):
        """获取整数值参数 - 统一接口"""
        try:
            value = self.system_params.get_int(key)
            if value is not None:
                return value
        except (KeyError, AttributeError, UnknownKeyName) as e:
            pass

        if key in self.nav_data:
            value = self.nav_data[key]
            try:
                return int(value)
            except (ValueError, TypeError):
                return default

        return default

    def get_float(self, key, default=0.0):
        """获取浮点数值参数 - 统一接口"""
        try:
            value = self.system_params.get_float(key)
            if value is not None:
                return value
        except (KeyError, AttributeError, UnknownKeyName) as e:
            pass

        if key in self.nav_data:
            value = self.nav_data[key]
            try:
                return float(value)
            except (ValueError, TypeError):
                return default

        return default

    def put_bool(self, key, value):
      """设置布尔值参数 - 统一接口"""
      bool_value = bool(value)
      int_value = int(bool_value)

      json_need_save = False

      # 先尝试保存系统参数
      try:
        self.system_params.put_bool(key, bool_value)
      except (KeyError, AttributeError, UnknownKeyName):
        # 系统参数没有 → 写入 nav_data
        self.nav_data[key] = int_value
        json_need_save = True
      else:
        # 系统参数写成功，但如果 key 存在于 nav_data，也要同步更新
        if key in self.nav_data:
          self.nav_data[key] = int_value
          json_need_save = True

      if json_need_save:
        try:
          self._save_nav_data()
        except (OSError, IOError):
          pass

    def put_int(self, key, value):
      """设置整数值参数 - 统一接口"""
      int_value = int(value)

      json_need_save = False

      try:
        self.system_params.put_int(key, int_value)
      except (KeyError, AttributeError, UnknownKeyName):
        self.nav_data[key] = int_value
        json_need_save = True
      else:
        if key in self.nav_data:
          self.nav_data[key] = int_value
          json_need_save = True

      if json_need_save:
        try:
          self._save_nav_data()
        except (OSError, IOError):
          pass

    def put_float(self, key, value):
      """设置浮点数值参数 - 统一接口"""
      float_value = float(value)

      json_need_save = False

      try:
        self.system_params.put_float(key, float_value)
      except (KeyError, AttributeError, UnknownKeyName):
        self.nav_data[key] = float_value
        json_need_save = True
      else:
        if key in self.nav_data:
          self.nav_data[key] = float_value
          json_need_save = True

      if json_need_save:
        try:
          self._save_nav_data()
        except (OSError, IOError):
          pass

    def put(self, key, dat):
      """设置原始数据 - 系统参数专用"""
      json_need_save = False

      try:
        self.system_params.put(key, dat)
      except (KeyError, AttributeError, UnknownKeyName):
        self.nav_data[key] = dat
        json_need_save = True
      else:
        if key in self.nav_data:
          self.nav_data[key] = dat
          json_need_save = True

      if json_need_save:
        try:
          self._save_nav_data()
        except (OSError, IOError):
          pass

    # 为了保持兼容性，也提供原始的 get/put 方法
    def get(self, key, encoding='utf-8'):
        """获取原始数据 - 系统参数专用"""
        return self.system_params.get(key, encoding=encoding)

# 全局实例，便于导入使用
unified_params = UnifiedParams()
