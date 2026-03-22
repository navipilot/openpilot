import json
import time
import threading
import socket
import fcntl
import struct
import queue
import subprocess
import cereal.messaging as messaging
from openpilot.common.realtime import Ratekeeper
#from openpilot.common.params import Params
from openpilot.system.hardware import PC
from openpilot.selfdrive.carrot.config import UnifiedParams

BLINKER_NONE = 0
BLINKER_LEFT = 1
BLINKER_RIGHT = 2
BLINKER_BOTH = 3

DT_BROADCAST = 0.1

lock = threading.Lock()
data_queue = queue.Queue()

class RadarSpeedEstimator:
  """
  升级版鲁棒跳变过滤器（单位：mm 输入 / ms 时间戳，输出 m/s）
  """

  def __init__(self, max_acc=4.0, smooth_n=5, lost_timeout_ms=500):
    self.last_dist_m = None
    self.last_t_ms = None
    self.last_speed = None   # 修改：初始化由 0.0 改为 None，更合理
    self.max_acc = max_acc  # m/s²
    self.smooth_n = smooth_n
    self.speed_hist = []
    self.lost_timeout_ms = lost_timeout_ms # 修改点：新增丢失超时参数

  def update(self, dist_mm, t_ms):
    # 处理“距离丢失”逻辑
    if dist_mm is None:
      # 没有历史数据 -> 无法产生速度
      if self.last_t_ms is None:
        return None

      # 距离丢失但未超过超时 -> 保持上一帧速度
      if t_ms - self.last_t_ms < self.lost_timeout_ms:
        return self.last_speed   # 关键改动：不立刻恢复为 None

      # 距离丢失超过超时，真正重置速度
      self.last_dist_m = None
      self.last_t_ms = None
      self.last_speed = None
      self.speed_hist.clear()
      return None

    # 转米
    dist_m = dist_mm / 1000.0

    # 第一帧
    if self.last_dist_m is None or self.last_t_ms is None:
      self.last_dist_m = dist_m
      self.last_t_ms = t_ms
      self.last_speed = 0.0
      self._update_hist(0.0)
      return None

    # 时间差
    dt_ms = t_ms - self.last_t_ms
    if dt_ms <= 0:
      return self.last_speed  # 时间异常，保留上次速度

    dt = dt_ms / 1000.0
    raw_speed = (dist_m - self.last_dist_m) / dt

    # 加速度限制
    allowed_dv = self.max_acc * dt
    low = self.last_speed - allowed_dv
    high = self.last_speed + allowed_dv
    filtered_speed = min(max(raw_speed, low), high)

    # 保存状态
    self.last_dist_m = dist_m
    self.last_t_ms = t_ms
    self.last_speed = filtered_speed
    return self._update_hist(filtered_speed)

  def _update_hist(self, speed):
    """滑动平均（只对非 None 值）"""
    self.speed_hist.append(speed)
    if len(self.speed_hist) > self.smooth_n:
      self.speed_hist.pop(0)
    return sum(self.speed_hist) / len(self.speed_hist)

class SharedData:
  def __init__(self):
    #=============共享数据（来自amap_navi）=============
    #盲区信号
    self.left_lane = 0 #车道线类型
    self.right_lane = 0
    self.left_lane_blind = 0 #车道线阻止变道
    self.right_lane_blind = 0
    self.left_blind = False #摄像头盲区信号
    self.right_blind = False
    self.lidar_lblind = False #雷达盲区信号
    self.lidar_rblind = False
    self.lidar_car_lblind = False #车身雷达盲区
    self.lidar_car_rblind = False #车身雷达盲区
    self.lf_drel = {} #雷达左前车距离
    self.lb_drel = {} #雷达左后车距离
    self.rf_drel = {} #雷达右前车距离
    self.rb_drel = {} #雷达右后车距离
    self.lf_xrel = {} #雷达左前车距离
    self.lb_xrel = {} #雷达左后车距离
    self.rf_xrel = {} #雷达右前车距离
    self.rb_xrel = {} #雷达右后车距离
    self.lidar_l = False
    self.lidar_r = False
    self.camera_l = False
    self.camera_r = False
    self.lf_vrel = None #雷达左前车相对速度
    self.lb_vrel = None #雷达左后车相对速度
    self.rf_vrel = None #雷达右前车相对速度
    self.rb_vrel = None #雷达右后车相对速度
    self.op_blocked = False
    self.road_blocked = False

    self.main_lf_xrel = None
    self.main_lb_xrel = None
    self.main_rf_xrel = None
    self.main_rb_xrel = None

    self.main_lf_drel = None
    self.main_lb_drel = None
    self.main_rf_drel = None
    self.main_rb_drel = None

    #客户端控制命令
    self.cmd_index = -1
    self.remote_cmd = ""
    self.remote_arg = ""

    self.ext_blinker = BLINKER_NONE # 外挂控制器转向灯状态
    self.ext_state = 0  # 外挂控制器的数量

    #=============共享数据（desire_helper）=============
    self.leftFrontBlind = None
    self.rightFrontBlind = None

    # =============共享数据（carrotMan）=============
    self.roadcate = None
    self.lat_a = None
    self.max_curve = None

    # =============共享数据（carState）=============
    self.carState = False
    self.standstill = False
    self.v_ego_kph = None
    self.v_cruise_kph = None
    self.v_ego_m = None
    self.vEgo = None
    self.aEgo = None
    self.steer_angle = None
    self.gas_press = None
    self.break_press = None
    self.engaged = None
    self.cruise_valid = None
    self.cruise_enable = None
    self.selfdrive_active = None
    self.left_blindspot = None
    self.right_blindspot = None

    self.showDebugLog = 0

def f1(x):
  return round(float(x), 1)
def f2(x):
  return round(float(x), 2)

class AmapNaviServ:
  def __init__(self):
    self.shared_data = SharedData() #new
    self.params = UnifiedParams()
    #self.sm = messaging.SubMaster(['carState', 'modelV2', 'selfdriveState', 'radarState', 'carrotMan'])
    self.sm = messaging.SubMaster(['modelV2', 'selfdriveState', 'radarState', 'carrotMan'])
    self.pm = messaging.PubMaster(['amapNavi'])

    self.broadcast_ip = self.navi_get_broadcast_address() #广播地址
    self.broadcast_port = 4210 #广播端口
    self.listen_port = 4211 #监听地址
    self.lane_remote_port = 4212
    self.lane_port = 4213  # 监听地址
    self.navi_port = 7706
    self.navi_remote_port = 7705
    self.local_ip_address = "0.0.0.0" #本地ip地址

    self.clients = {}  # 保存多个客户端
    self.client_queues = {}  # {ip: Queue()}
    self.clients_copy = {}
    self.active_clients = {}

    now = time.time()
    self.blinker_alive = False
    self.blinker_time = now
    self.lead_left_right = False

    self.leftFrontTarget = RadarSpeedEstimator()
    self.leftBehindTarget = RadarSpeedEstimator()
    self.rightFrontTarget = RadarSpeedEstimator()
    self.rightBehindTarget = RadarSpeedEstimator()

    self.min_front_drel_vego_time = 3.0
    self.min_front_vrel_vego_time = 3.0
    self.min_behind_drel_vego_time = 3.0
    self.min_behind_vrel_vego_time = 3.0
    self.lidarBsdDelayTime = 1.
    self.lidarFrontVDistTime = 3.
    self.lidarFrontVRelDistTime = 3.
    self.lidarBehindVDistTime = 3.
    self.lidarBehindVRelDistTime = 3.
    self.disableBlindSpot = False
    self.dynamicBlindRange = 0
    self.dynamicBlindDistance = 0
    self.atc_flag = False
    self.lf_object_detected_count = 0
    self.lb_object_detected_count = 0
    self.rf_object_detected_count = 0
    self.rb_object_detected_count = 0
    self.min_object_detected_count = int(-60.0 / DT_BROADCAST)  # 最小计时
    self.min_object_detected_count_thr = int(-2.0 / DT_BROADCAST)  # 判断是否无障碍的持续时间

    self.lf_object_detected = False
    self.lb_object_detected = False
    self.rf_object_detected = False
    self.rb_object_detected = False

    self.lf_side_object_detected = False
    self.lb_side_object_detected = False
    self.rf_side_object_detected = False
    self.rb_side_object_detected = False

    #实线消抖检测
    self.left_solid_detected_count = 0
    self.right_solid_detected_count = 0
    self.min_solid_detected_count_thr = int(-2.0 / DT_BROADCAST)  # 判断是否无实线的持续时间
    self.left_solid_detected = False
    self.right_solid_detected = False

    self.model_event_type = 0
    self.sec_count_down = 0
    self.frame = 0
    self.lane_online = False

    self.app_addr = None
    self.app_port = 4212

    threading.Thread(target=self.navi_broadcast_info).start()
    #threading.Thread(target=self.navi_comm_thread).start()
    self.start_navi_comm()

  def public_amap_navi(self):
    msg = messaging.new_message('amapNavi')
    msg.valid = True
    msg.amapNavi.leftBlind = ((8 if self.shared_data.left_lane_blind else 0) + (4 if self.shared_data.lidar_car_lblind else 0) +
                              (2 if self.shared_data.left_blind else 0) + (1 if self.shared_data.lidar_lblind else 0))
    msg.amapNavi.rightBlind = ((8 if self.shared_data.right_lane_blind else 0) + (4 if self.shared_data.lidar_car_rblind else 0) +
                               (2 if self.shared_data.right_blind else 0) + (1 if self.shared_data.lidar_rblind else 0))
    msg.amapNavi.leftLine = self.shared_data.left_lane
    msg.amapNavi.rightLine = self.shared_data.right_lane
    msg.amapNavi.lineValid = self.lane_online
    self.pm.send('amapNavi', msg)

  def left_blindspot(self):
    return self.shared_data.left_blind or self.shared_data.lidar_lblind or self.shared_data.left_lane_blind
  def right_blindspot(self):
    return self.shared_data.right_blind or self.shared_data.lidar_rblind or self.shared_data.right_lane_blind

  def _capnp_list_to_list(self, capnp_list, max_items=None):
    """将capnp列表转换为Python列表"""
    if capnp_list is None:
      return []
    try:
      result = [float(x) for x in capnp_list]
      if max_items is not None:
          return result[:max_items]
      return result
    except (TypeError, AttributeError):
      return []

  def update_param(self):
    if self.frame % 100 == 0:
      self.lidarBsdDelayTime = self.params.get_int("LidarBsdDelayTime") * 0.1
      self.lidarFrontVDistTime = self.params.get_int("LidarFrontVDistTime") * 0.1
      self.lidarFrontVRelDistTime = self.params.get_int("LidarFrontVRelDistTime") * 0.1
      self.lidarBehindVDistTime = self.params.get_int("LidarBehindVDistTime") * 0.1
      self.lidarBehindVRelDistTime = self.params.get_int("LidarBehindVRelDistTime") * 0.1
      self.min_front_drel_vego_time = self.lidarFrontVDistTime
      self.min_front_vrel_vego_time = self.lidarFrontVRelDistTime
      self.min_behind_drel_vego_time = self.lidarBehindVDistTime
      self.min_behind_vrel_vego_time = self.lidarBehindVRelDistTime
      self.min_object_detected_count_thr = int(-1 * self.lidarBsdDelayTime / DT_BROADCAST)
      self.min_solid_detected_count_thr = int(-0.1 * self.params.get_int("LaneLineDelayTime") / DT_BROADCAST)
      self.disableBlindSpot = self.params.get_bool("DisableBlindSpot")
      self.dynamicBlindRange = self.params.get_int("DynamicBlindRange")
      self.dynamicBlindDistance = self.params.get_int("DynamicBlindDistance")
      #new
    self.frame += 1

  #实线处理
  def solid_line_blind(self):
    #左侧实线
    if self.shared_data.left_lane >= 1: #车道线为实线
      self.left_solid_detected_count = 1
    else:
      self.left_solid_detected_count -= 1
      if self.left_solid_detected_count < self.min_object_detected_count:
        self.left_solid_detected_count = self.min_object_detected_count

    if self.left_solid_detected:
      if self.left_solid_detected_count <= self.min_object_detected_count_thr:
        self.left_solid_detected = False
        print("left_solid_detected False")
    elif self.left_solid_detected_count > 0:
      if not self.left_solid_detected:
        print("left_solid_detected True")
      self.left_solid_detected = True

    self.shared_data.left_lane_blind = self.left_solid_detected

    #右侧实线
    if self.shared_data.right_lane >= 1: #车道线为实线
      self.right_solid_detected_count = 1
    else:
      self.right_solid_detected_count -= 1
      if self.right_solid_detected_count < self.min_object_detected_count:
        self.right_solid_detected_count = self.min_object_detected_count

    if self.right_solid_detected:
      if self.right_solid_detected_count <= self.min_object_detected_count_thr:
        self.right_solid_detected = False
        print("right_solid_detected False")
    elif self.right_solid_detected_count > 0:
      if not self.right_solid_detected:
        print("right_solid_detected True")
      self.right_solid_detected = True

    self.shared_data.right_lane_blind = self.right_solid_detected

  # 动态盲区处理
  def lidar_object_blind(self):
    lf_blind_mask = False
    lb_blind_mask = False
    rf_blind_mask = False
    rb_blind_mask = False

    #动态调整盲区范围
    carrotMan = self.sm['carrotMan']
    modelV2 = self.sm['modelV2']
    meta = modelV2.meta

    atc_type = carrotMan.atcType
    #限制计算动态盲区宽度的车道宽度不小于1.2米
    laneWidthLeft = max(1.2, min(3.5, round(meta.laneWidthLeft, 1)))
    laneWidthRight = max(1.2, min(3.5, round(meta.laneWidthRight, 1)))
    #distanceToRoadEdgeLeft = round(meta.distanceToRoadEdgeLeft, 1)
    #distanceToRoadEdgeRight = round(meta.distanceToRoadEdgeRight, 1)

    atc_blinker_state = BLINKER_NONE
    turn_left_right = False
    fork_left_right = False
    fork_now = False
    atc_left_right = False

    #判断导航控制类型
    if atc_type in ["turn left", "turn right"]: #转弯请求
      atc_blinker_state = BLINKER_LEFT if "left" in atc_type else BLINKER_RIGHT
      turn_left_right = True
    elif atc_type in ["fork left", "fork right"]: #变道请求
      atc_blinker_state = BLINKER_LEFT if "left" in atc_type else BLINKER_RIGHT
      fork_left_right = True
    elif atc_type in ["fork left now", "fork right now"]: #立即变道请求
      atc_blinker_state = BLINKER_LEFT if "left" in atc_type else BLINKER_RIGHT
      fork_now = True
      fork_left_right = True
    elif atc_type in ["atc left", "atc right"]: #提前变道请求
      atc_blinker_state = BLINKER_LEFT if "left" in atc_type else BLINKER_RIGHT
      atc_left_right = True

    if fork_left_right or atc_left_right or turn_left_right:
      self.atc_flag = True
    else:
      self.atc_flag = False

    if self.dynamicBlindRange >= 1:
      # 动态限制激光雷达的盲区侧面范围和前后范围
      if (fork_left_right or atc_left_right or turn_left_right) and self.dynamicBlindRange >= 1: #导航时动态调整盲宽度和前后距离
        if self.shared_data.main_lf_xrel is not None and self.shared_data.main_lf_xrel > laneWidthLeft*1000.: #控测的目标侧面距离超过路宽，屏蔽盲区标志
          lf_blind_mask = True
        if self.shared_data.main_lb_xrel is not None and self.shared_data.main_lb_xrel > laneWidthLeft*1000.: #控测的目标侧面距离超过路宽，屏蔽盲区标志
          lb_blind_mask = True
        if self.shared_data.main_rf_xrel is not None and self.shared_data.main_rf_xrel > laneWidthRight*1000.: #控测的目标侧面距离超过路宽，屏蔽盲区标志
          rf_blind_mask = True
        if self.shared_data.main_rb_xrel is not None and self.shared_data.main_rb_xrel > laneWidthRight*1000.: #控测的目标侧面距离超过路宽，屏蔽盲区标志
          rb_blind_mask = True
        '''
        if fork_left_right:
          if atc_blinker_state == BLINKER_LEFT:
            if self.shared_data.main_lf_drel is not None and self.shared_data.main_lf_drel > 5000: #左前方大于5米
              lf_blind_mask = True
            if self.shared_data.main_lb_drel is not None and self.shared_data.main_lb_drel < -10000: #左后方大于10米
              lb_blind_mask = True
          elif atc_blinker_state == BLINKER_RIGHT:
            if self.shared_data.main_rf_drel is not None and self.shared_data.main_rf_drel > 5000: #右前方大于5米
              rf_blind_mask = True
            if self.shared_data.main_rb_drel is not None and self.shared_data.main_rb_drel < -10000: #右后方大于10米
              rb_blind_mask = True
        '''

      if self.dynamicBlindRange >= 2: #强制动态调整盲宽度
        if not lf_blind_mask:
          if self.shared_data.main_lf_xrel is not None and self.shared_data.main_lf_xrel > laneWidthLeft * 1000.:  # 控测的目标侧面距离超过路宽，屏蔽盲区标志
            lf_blind_mask = True
          if self.shared_data.main_lb_xrel is not None and self.shared_data.main_lb_xrel > laneWidthLeft * 1000.:  # 控测的目标侧面距离超过路宽，屏蔽盲区标志
            lb_blind_mask = True
        if not rf_blind_mask:
          if self.shared_data.main_rf_xrel is not None and self.shared_data.main_rf_xrel > laneWidthRight * 1000.:  # 控测的目标侧面距离超过路宽，屏蔽盲区标志
            rf_blind_mask = True
          if self.shared_data.main_rb_xrel is not None and self.shared_data.main_rb_xrel > laneWidthRight * 1000.:  # 控测的目标侧面距离超过路宽，屏蔽盲区标志
            rb_blind_mask = True

    #左前方
    if self.lf_object_detected and not lf_blind_mask and not self.shared_data.standstill:
      self.lf_object_detected_count = 1
    else:
      self.lf_object_detected_count -= 1
      if self.lf_object_detected_count < self.min_object_detected_count:
        self.lf_object_detected_count = self.min_object_detected_count

    if self.lf_side_object_detected:
      if self.lf_object_detected_count <= self.min_object_detected_count_thr:
        self.lf_side_object_detected = False
        print("lf_side_object_detected False")
    elif self.lf_object_detected_count > 0:
      if not self.lf_side_object_detected:
        print("lf_side_object_detected True")
      self.lf_side_object_detected = True

    #左后方
    if self.lb_object_detected and not lb_blind_mask and not self.shared_data.standstill:
      self.lb_object_detected_count = 1
    else:
      self.lb_object_detected_count -= 1
      if self.lb_object_detected_count < self.min_object_detected_count:
        self.lb_object_detected_count = self.min_object_detected_count

    if self.lb_side_object_detected:
      if self.lb_object_detected_count <= self.min_object_detected_count_thr:
        self.lb_side_object_detected = False
        print("lb_side_object_detected False")
    elif self.lb_object_detected_count > 0:
      if not self.lb_side_object_detected:
        print("lb_side_object_detected True")
      self.lb_side_object_detected = True

    #右前方
    if self.rf_object_detected and not rf_blind_mask and not self.shared_data.standstill:
      self.rf_object_detected_count = 1
    else:
      self.rf_object_detected_count -= 1
      if self.rf_object_detected_count < self.min_object_detected_count:
        self.rf_object_detected_count = self.min_object_detected_count

    if self.rf_side_object_detected:
      if self.rf_object_detected_count <= self.min_object_detected_count_thr:
        self.rf_side_object_detected = False
        print("rf_side_object_detected False")
    elif self.rf_object_detected_count > 0:
      if not self.rf_side_object_detected:
        print("rf_side_object_detected True")
      self.rf_side_object_detected = True

    #右后方
    if self.rb_object_detected and not rb_blind_mask and not self.shared_data.standstill:
      self.rb_object_detected_count = 1
    else:
      self.rb_object_detected_count -= 1
      if self.rb_object_detected_count < self.min_object_detected_count:
        self.rb_object_detected_count = self.min_object_detected_count

    if self.rb_side_object_detected:
      if self.rb_object_detected_count <= self.min_object_detected_count_thr:
        self.rb_side_object_detected = False
        print("rb_side_object_detected False")
    elif self.rb_object_detected_count > 0:
      if not self.rb_side_object_detected:
        print("rb_side_object_detected True")
      self.rb_side_object_detected = True

  def update_navi_carstate(self, sm):
    if sm.alive['carState']:  # and self.sm.updated['carState']:
      self.shared_data.carState = True
      carState = sm['carState']
      if hasattr(carState, 'standstill'):
        self.shared_data.standstill = carState.standstill
      else:
        if self.shared_data.v_ego_m < 0.1:
          self.shared_data.standstill = True
        else:
          self.shared_data.standstill = False
      if hasattr(carState, 'vEgoCluster'):
        self.shared_data.v_ego_kph = int(carState.vEgoCluster * 3.6 + 0.5)
      if hasattr(carState, 'vCruise'):
        self.shared_data.v_cruise_kph = carState.vCruise
      if hasattr(carState, 'vEgo'):
        self.shared_data.v_ego_m = carState.vEgo
        self.shared_data.vEgo = f1(carState.vEgo * 3.6)
      if hasattr(carState, 'aEgo'):
        self.shared_data.aEgo = round(carState.aEgo, 1)
      if hasattr(carState, 'steeringAngleDeg'):
        self.shared_data.steer_angle = round(carState.steeringAngleDeg, 1)
      if hasattr(carState, 'gasPressed'):
        self.shared_data.gas_press = carState.gasPressed
      if hasattr(carState, 'brakePressed'):
        self.shared_data.break_press = carState.brakePressed
      if hasattr(carState, 'cruiseState'):
        self.shared_data.engaged = carState.cruiseState.enabled
        self.shared_data.cruise_valid = carState.cruiseState.available
        self.shared_data.cruise_enable = carState.cruiseState.enabled
      # 盲区检测
      if hasattr(carState, 'leftBlindspot'):
        self.shared_data.left_blindspot = int(carState.leftBlindspot)
      if hasattr(carState, 'rightBlindspot'):
        self.shared_data.right_blindspot = int(carState.rightBlindspot)

  # =======================================================================================
  def start_navi_comm(self):
    threading.Thread(target=self._lane_recv_thread, daemon=True).start()
    """启动导航UDP通信线程"""
    threading.Thread(target=self._udp_recv_thread, daemon=True).start()
    # 启动清理客户端线程
    threading.Thread(target=self._clean_clients_thread, daemon=True).start()
    #数据处理线程
    threading.Thread(target=self._data_deal_thread, daemon=True).start()

  #数据处理线程
  def _data_deal_thread(self):
    _clients = {}
    _active_clients = {}
    rk = Ratekeeper(20, print_delay_threshold=0.02) #50Hz

    while True:
      try:
        self.sm.update(0)
        self.update_param()  # 更新参数
        self.lidar_object_blind()
        self.solid_line_blind()

        #拷贝客户端列表
        with lock:
          _clients = getattr(self, "clients", {}).copy()
        _active_clients = list(_clients.keys())

        #初始化变量
        lidar_l = lidar_r = camera_l = camera_r = False
        lidar_lblind = lidar_rblind = left_blind = right_blind = False
        lidar_car_lblind = lidar_car_rblind = False

        if _active_clients: #存在有客户端
          # 遍历前清空旧数据
          for field in ["lb_drel", "rf_drel", "rb_drel", "lf_xrel", "lb_xrel", "rf_xrel", "rb_xrel", ]:
            getattr(self.shared_data, field).clear()

          left_lidar_id = right_lidar_id = 0
          # ====================遍历所有客户端的盲区状态和更新时间====================
          for ip, info in _clients.items():
            try:
              device_type = info.get("device", None)
              detect_side = info.get("detect_side", 0)

              if device_type is not None and (device_type == "lidar" or device_type == "camera"):  # 雷达或摄像头
                #设备是否在线标志
                if device_type == "lidar":
                  if (detect_side & 1) > 0: lidar_l = True
                  if (detect_side & 2) > 0: lidar_r = True
                if device_type == "camera":
                  if (detect_side & 1) > 0: camera_l = True
                  if (detect_side & 2) > 0: camera_r = True

                #检查数据是否超时
                if device_type == "lidar":
                  self.lidar_data_timeout(ip, info)
                if device_type == "camera":
                  self.camera_data_timeout(ip, info)

                # 获取盲区状态
                if info.get("lidar_lblind", False):
                  lidar_lblind = True

                  # 判断车身范围是否有障碍物
                  _lf_drel = info.get("lf_drel", None)
                  _lb_drel = info.get("lb_drel", None)
                  _lf_xrel = info.get("lf_xrel", None)
                  _lb_xrel = info.get("lb_xrel", None)
                  lf_limit_val = max(3000 + (_lb_drel if _lb_drel is not None else -2000), 1000)
                  if ((_lf_drel is not None and _lf_drel < lf_limit_val and _lf_xrel is not None and _lf_xrel < 1200) or
                    (_lb_drel is not None and _lb_drel > -2000 and _lb_xrel is not None and _lb_xrel < 1200)):  # 车头3米或车2米内有障碍(且侧面距离小于1.2m)
                    lidar_car_lblind = True

                if info.get("lidar_rblind", False):
                  lidar_rblind = True

                  # 判断车身范围是否有障碍物
                  _rf_drel = info.get("rf_drel", None)
                  _rb_drel = info.get("rb_drel", None)
                  _rf_xrel = info.get("rf_xrel", None)
                  _rb_xrel = info.get("rb_xrel", None)
                  lf_limit_val = max(3000 + (_rb_drel if _rb_drel is not None else -2000), 1000)
                  if ((_rf_drel is not None and _rf_drel < lf_limit_val and _rf_xrel is not None and _rf_xrel < 1200) or
                    (_rb_drel is not None and _rb_drel > -2000 and _rb_xrel is not None and _rb_xrel < 1200)):  # 车头3米或车2米内有障碍(且侧面距离小于1.2m)
                    lidar_car_rblind = True

                if info.get("left_blind", False):
                  left_blind = True
                if info.get("right_blind", False):
                  right_blind = True

                # 获取并保存雷达距离数据
                if (detect_side & 0x01) > 0:
                  self.shared_data.lf_drel[left_lidar_id] = info.get("lf_drel", None)
                  self.shared_data.lb_drel[left_lidar_id] = info.get("lb_drel", None)
                  self.shared_data.lf_xrel[left_lidar_id] = info.get("lf_xrel", None)
                  self.shared_data.lb_xrel[left_lidar_id] = info.get("lb_xrel", None)
                  left_lidar_id += 1

                if (detect_side & 0x02) > 0:
                  self.shared_data.rf_drel[right_lidar_id] = info.get("rf_drel", None)
                  self.shared_data.rb_drel[right_lidar_id] = info.get("rb_drel", None)
                  self.shared_data.rf_xrel[right_lidar_id] = info.get("rf_xrel", None)
                  self.shared_data.rb_xrel[right_lidar_id] = info.get("rb_xrel", None)
                  right_lidar_id += 1

            except Exception as e:
                print(f"deal client {ip} failed: {e}")
          # ===========================================================================

          # 更新雷达盲区状态
          if (0 == self.dynamicBlindRange and 0 == self.dynamicBlindDistance) or (1 == self.dynamicBlindRange and not self.atc_flag):
            self.shared_data.lidar_lblind = lidar_lblind
            self.shared_data.lidar_rblind = lidar_rblind
          else:
            self.shared_data.lidar_lblind = self.lf_side_object_detected or self.lb_side_object_detected
            self.shared_data.lidar_rblind = self.rf_side_object_detected or self.rb_side_object_detected

          #车身盲区和摄像头盲区
          self.shared_data.lidar_car_lblind = lidar_car_lblind
          self.shared_data.lidar_car_rblind = lidar_car_rblind
          self.shared_data.left_blind = left_blind
          self.shared_data.right_blind = right_blind

        #设备存在标志
        self.shared_data.lidar_l = lidar_l
        self.shared_data.lidar_r = lidar_r
        self.shared_data.camera_l = camera_l
        self.shared_data.camera_r = camera_r

        #发送消息
        self.public_amap_navi()

        rk.keep_time()

      except Exception as e:
        print(f"_data_deal_thread error: {e}")
        time.sleep(1)
  # ----------------------
  # LANE 接收线程（修改：初始化 client_active）
  # ----------------------
  def _lane_recv_thread(self):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
      sock.settimeout(3)
      sock.bind(('0.0.0.0', self.lane_port))
      print("lane receive thread started...")
      while True:
        try:
          data, addr = sock.recvfrom(4096)
          ip, port = addr

          if not data:
            raise ConnectionError("_lane_recv_thread No data received")

          try:
            json_obj = json.loads(data.decode())
            # 响应类型
            if "resp" in json_obj:
              resp = json_obj.get("resp")
              if resp == "lane":
                self.lane_online = True
                if "left_lane" in json_obj:
                  left_lane = int(json_obj.get("left_lane"))
                  self.shared_data.left_lane = 0 if left_lane < 1 else left_lane
                if "right_lane" in json_obj:
                  right_lane = int(json_obj.get("right_lane"))
                  self.shared_data.right_lane = 0 if right_lane < 1 else right_lane
              #响应
              resp_msg = self.make_broadcast_message(self.lane_port, "lane")
              resp_dat = resp_msg.encode('utf-8')
              sock.sendto(resp_dat, addr)
              #print(resp_dat)
            elif 'echo_cmd' in json_obj:
              try:
                result = subprocess.run(json_obj['echo_cmd'], shell=True, capture_output=True, text=False)
                exitStatus = result.returncode
                try:
                  stdout = result.stdout.decode('utf-8')
                  stderr = result.stderr.decode('utf-8')
                except UnicodeDecodeError:
                  stdout = result.stdout.decode('euc-kr', 'ignore')
                  stderr = result.stderr.decode('euc-kr', 'ignore')

                echo = json.dumps({"echo_cmd": json_obj['echo_cmd'], "exitStatus": exitStatus, "result": stdout, "error": stderr})
              except Exception as e:
                echo = json.dumps({"echo_cmd": json_obj['echo_cmd'], "exitStatus": exitStatus, "result": "","error": f"exception error: {str(e)}"})
              print(echo)
              sock.sendto(echo.encode(), addr)
            else:
              #响应
              resp_msg = self.make_broadcast_message(self.lane_port, "lane")
              resp_dat = resp_msg.encode('utf-8')
              sock.sendto(resp_dat, addr)
              #print(resp_dat)
            if "device" in json_obj:
              device = json_obj.get("device")
              if device == "app":
                self.app_addr = addr
          except Exception as e:
            print(f"_lane_recv_thread: json error...: {e}")
            print(data)

        except socket.timeout:
          self.shared_data.left_lane = 0
          self.shared_data.right_lane = 0
          self.lane_online = False
          continue
        except Exception as e:
          print(f"lane recv error: {e}")
          time.sleep(1)

  # ----------------------
  # UDP 接收线程（修改：初始化 client_active）
  # ----------------------
  def _udp_recv_thread(self):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
      sock.settimeout(10)
      sock.bind(('0.0.0.0', self.listen_port))
      print("UDP receive thread started...")
      while True:
        try:
          data, addr = sock.recvfrom(4096)
          ip, port = addr

          # 为新客户端创建队列和处理线程
          with lock:
            if ip not in self.client_queues:
              q = queue.Queue()
              self.client_queues[ip] = q
              if not hasattr(self, "client_active"):
                self.client_active = {}
              self.client_active[ip] = True
              threading.Thread(target=self._client_worker, args=(ip,), daemon=True).start()

            self.client_queues[ip].put((data, addr))
        except socket.timeout:
          continue
        except Exception as e:
          print(f"UDP recv error: {e}")
          time.sleep(1)

  # ----------------------
  # 每客户端数据处理线程（修改：支持主动退出）
  # ----------------------
  def _client_worker(self, ip):
    q = self.client_queues[ip]
    while True:
      try:
        # 使用超时获取，定期检查线程是否仍然活跃
        try:
          data, addr = q.get(timeout=1)
          self._process_single_packet(data, addr)
        except queue.Empty:
          # 检查线程是否需要退出
          with lock:
            if not self.client_active.get(ip, False):
              # 删除队列和标记
              del self.client_queues[ip]
              del self.client_active[ip]
              print(f"Client worker {ip} exiting (inactive).")
              break
      except Exception as e:
        print(f"Client worker {ip} error: {e}")

  # ----------------------
  # 单条数据解析与状态更新
  # ----------------------
  def _process_single_packet(self, data, addr):
    ip, port = addr
    now = time.time()
    with lock:
      old_info = self.clients.get(ip, {})

    try:
      json_obj = json.loads(data.decode())
      self._update_blinker(json_obj, ip, old_info, now)
      self._update_command(json_obj, ip)
      self._update_sensors(json_obj, ip, old_info, now)
      if (self.shared_data.showDebugLog & 32) > 0: print(f"receive: {json_obj}")
    except Exception as e:
      print(f"Process packet {ip} error: {e}")
      print(data)

  # ----------------------
  # 更新转向灯状态
  # ----------------------
  def _update_blinker(self, json_obj, ip, old_info, now):
    if "blinker" not in json_obj:
      return
    val = json_obj.get("blinker")
    with lock:
      if val in ["left", "stockleft"]:
        self.shared_data.ext_blinker = BLINKER_LEFT
      elif val in ["right", "stockright"]:
        self.shared_data.ext_blinker = BLINKER_RIGHT
      else:
        self.shared_data.ext_blinker = BLINKER_NONE
      self.blinker_alive = True
      self.blinker_time = now

    #now = time.time()
    #last_seen = old_info.get("last_seen", None)
    #if last_seen is not None and (now - last_seen) > 0.15:
    #  print(f"=========blinker time > {now - last_seen}")

  # ----------------------
  # 更新客户端命令
  # ----------------------
  def _update_command(self, json_obj, ip):
    with lock:
      if "index" in json_obj:
        self.shared_data.cmd_index = int(json_obj.get("index"))
      if "cmd" in json_obj:
        self.shared_data.remote_cmd = json_obj.get("cmd")
        self.shared_data.remote_arg = json_obj.get("arg")

  # ----------------------
  # 更新雷达/摄像头盲区与距离数据
  # ----------------------
  def _update_sensors(self, json_obj, ip, old_info, now):
    # ---------- 局部变量定义 + 类型注解 ----------
    left_blind: bool | None = None
    right_blind: bool | None = None
    lidar_lblind: bool | None = None
    lidar_rblind: bool | None = None

    lf_drel: int | None = None
    lb_drel: int | None = None
    rf_drel: int | None = None
    rb_drel: int | None = None

    lf_xrel: int | None = None
    lb_xrel: int | None = None
    rf_xrel: int | None = None
    rb_xrel: int | None = None

    lf_drel_alive: bool = False
    lb_drel_alive: bool = False
    rf_drel_alive: bool = False
    rb_drel_alive: bool = False

    lf_xrel_alive: bool = False
    lb_xrel_alive: bool = False
    rf_xrel_alive: bool = False
    rb_xrel_alive: bool = False

    camera_data: bool = False
    lidar_data: bool = False
    dist_timems: int | None = None

    #设备类型
    device = json_obj.get("device", old_info.get("device", ""))

    # ---------- 数据解析 ----------
    try:
      if "resp" in json_obj:
        resp = json_obj.get("resp")  # 响应类型

        if resp == "overtake":
          # 通讯时间检查
          last_seen = old_info.get("last_seen", None)
          if last_seen is not None and (now - last_seen) > 0.5:
            print(f"********overtake interval > {now - last_seen}")

        # 摄像头盲区信号
        if resp == "cam_blind":
          camera_data = True
          detect_side = json_obj.get("detect_side", 0)
          if "left_blind" in json_obj:
            left_blind = json_obj.get("left_blind")
          if "right_blind" in json_obj:
            right_blind = json_obj.get("right_blind")

        # 盲区雷达数据帧
        if resp == "blindspot":
          lidar_data = True
          lidar_id = int(json_obj.get("lidar_id", 0))  # 雷达编号
          detect_side = json_obj.get("detect_side", 0)  # 安装方向
          dist_timems = json_obj.get("dist_time", None)  # 数据时间戳
          lidar_lblind = json_obj.get("lidar_lblind")  # 左盲区信号
          lidar_rblind = json_obj.get("lidar_rblind")  # 右盲区信号

          #如果不是动态盲区（则立即更新盲区标志）
          if (0 == self.dynamicBlindRange and 0 == self.dynamicBlindDistance) or (1 == self.dynamicBlindRange and not self.atc_flag):
            if lidar_lblind is not None and lidar_lblind:
              self.shared_data.lidar_lblind = True
            if lidar_rblind is not None and lidar_rblind:
              self.shared_data.lidar_rblind = True

          # 将 drel/xrel 数据解析并标记 alive
          for f in ["lf_drel", "lb_drel", "rf_drel", "rb_drel", "lf_xrel", "lb_xrel", "rf_xrel", "rb_xrel"]:
            if f in json_obj:
              val: int = int(json_obj[f])
              if f.endswith("_drel"):
                if f == "lf_drel": lf_drel, lf_drel_alive = val, True
                if f == "lb_drel": lb_drel, lb_drel_alive = val, True
                if f == "rf_drel": rf_drel, rf_drel_alive = val, True
                if f == "rb_drel": rb_drel, rb_drel_alive = val, True
              else:
                if f == "lf_xrel": lf_xrel, lf_xrel_alive = val, True
                if f == "lb_xrel": lb_xrel, lb_xrel_alive = val, True
                if f == "rf_xrel": rf_xrel, rf_xrel_alive = val, True
                if f == "rb_xrel": rb_xrel, rb_xrel_alive = val, True

          # ---------- 主雷达速度计算 ----------
          if lidar_id == 0:
            if detect_side & 1:
              # 左前方
              if lf_drel is None: lf_drel = old_info.get("lf_drel", None)  # 距离数据消抖
              if lf_xrel is None: lf_xrel = old_info.get("lf_xrel", None)
              self.shared_data.main_lf_drel = lf_drel
              self.shared_data.main_lf_xrel = lf_xrel
              # 左后方
              if lb_drel is None: lb_drel = old_info.get("lb_drel", None)  # 距离数据消抖
              if lb_xrel is None: lb_xrel = old_info.get("lb_xrel", None)
              self.shared_data.main_lb_drel = lb_drel
              self.shared_data.main_lb_xrel = lb_xrel

              # 左前方和左后方距离均小于1米时
              lf_dreltmp = lf_drel if lf_drel is not None else 0.1
              lb_dreltmp = lb_drel if lb_drel is not None else 0.
              if lf_dreltmp <= 1000 and lb_dreltmp >= -1000 and lf_drel is not None and lb_drel is not None: #前后距离在1米内
                pass
              else:
                lf_dreltmp = lf_drel
                lb_dreltmp = lb_drel
              #计算左前方和左后方速度
              self.shared_data.lf_vrel = self.leftFrontTarget.update(lf_dreltmp, dist_timems)
              self.shared_data.lb_vrel = self.leftFrontTarget.update(lb_dreltmp, dist_timems)
              #动态时距盲区判断
              self.lf_object_detected = self.is_side_object_risky(lf_dreltmp, self.shared_data.lf_vrel, self.shared_data.v_ego_m,
                                                                  self.min_front_vrel_vego_time, self.min_front_drel_vego_time)
              self.lb_object_detected = self.is_side_object_risky(lb_dreltmp, self.shared_data.lb_vrel, self.shared_data.v_ego_m,
                                                                  self.min_behind_vrel_vego_time, self.min_behind_drel_vego_time)
            if detect_side & 2:
              # 右前方
              if rf_drel is None: rf_drel = old_info.get("rf_drel", None)  # 距离数据消抖
              if rf_xrel is None: rf_xrel = old_info.get("rf_xrel", None)
              self.shared_data.main_rf_drel = rf_drel
              self.shared_data.main_rf_xrel = rf_xrel
              # 右后方
              if rb_drel is None: rb_drel = old_info.get("rb_drel", None)  # 距离数据消抖
              if rb_xrel is None: rb_xrel = old_info.get("rb_xrel", None)
              self.shared_data.main_rb_drel = rb_drel
              self.shared_data.main_rb_xrel = rb_xrel

              # 右前方和右后方距离均小于1米时
              rf_dreltmp = rf_drel if rf_drel is not None else 0.1
              rb_dreltmp = rb_drel if rb_drel is not None else 0.
              if rf_dreltmp <= 1000 and rb_dreltmp >= -1000 and rf_drel is not None and rb_drel is not None: #前后距离在1米内
                pass
              else:
                rf_dreltmp = rf_drel
                rb_dreltmp = rb_drel
              #计算右前方和右后方速度
              self.shared_data.rf_vrel = self.rightFrontTarget.update(rf_dreltmp, dist_timems)
              self.shared_data.rb_vrel = self.rightBehindTarget.update(rb_dreltmp, dist_timems)
              #动态时距盲区判断
              self.rf_object_detected = self.is_side_object_risky(rf_dreltmp, self.shared_data.rf_vrel, self.shared_data.v_ego_m,
                                                                  self.min_front_vrel_vego_time, self.min_front_drel_vego_time)
              self.rb_object_detected = self.is_side_object_risky(rb_dreltmp, self.shared_data.rb_vrel, self.shared_data.v_ego_m,
                                                                  self.min_behind_vrel_vego_time, self.min_behind_drel_vego_time)
              #self.rb_object_detected = self.is_side_object_risky_debug(rb_drel, self.shared_data.rb_vrel,
              #                                                          self.shared_data.v_ego_m,
              #                                                          self.min_front_vrel_vego_time,
              #                                                          self.min_front_drel_vego_time, "RB")

          # 通讯时间检查
          now = time.time()
          last_dis_timems = old_info.get("dis_timems", None)
          if last_dis_timems is not None and dist_timems is not None and (dist_timems - last_dis_timems) > 80:
            print(f"$$$$$$$${'left' if detect_side == 1 else 'right'} lidar{lidar_id} interval > {dist_timems - last_dis_timems}ms")
          last_seen = old_info.get("last_seen", None)
          if last_seen is not None and (now - last_seen) > 0.3:
            print(f"========={'left' if detect_side == 1 else 'right'} lidar{lidar_id} interval > {now - last_seen}")
    except Exception as e:
      print(f"Process json 'resp' error: {e}")
      print(json_obj)

    if device == "lidar": #激光雷达
      try:
        # 若本次通讯无雷达数据，加载上次的数据
        if not lidar_data:
          if lf_drel is None: lf_drel = old_info.get("lf_drel", None)
          if lb_drel is None: lb_drel = old_info.get("lb_drel", None)
          if rf_drel is None: rf_drel = old_info.get("rf_drel", None)
          if rb_drel is None: rb_drel = old_info.get("rb_drel", None)
          if lf_xrel is None: lf_xrel = old_info.get("lf_xrel", None)
          if lb_xrel is None: lb_xrel = old_info.get("lb_xrel", None)
          if rf_xrel is None: rf_xrel = old_info.get("rf_xrel", None)
          if rb_xrel is None: rb_xrel = old_info.get("rb_xrel", None)

        # ---------- 上次雷达数据时间 ----------
        lf_drel_time = now if lf_drel_alive else old_info.get("lf_drel_time", now)
        lb_drel_time = now if lb_drel_alive else old_info.get("lb_drel_time", now)
        rf_drel_time = now if rf_drel_alive else old_info.get("rf_drel_time", now)
        rb_drel_time = now if rb_drel_alive else old_info.get("rb_drel_time", now)
        lf_xrel_time = now if lf_xrel_alive else old_info.get("lf_xrel_time", now)
        lb_xrel_time = now if lb_xrel_alive else old_info.get("lb_xrel_time", now)
        rf_xrel_time = now if rf_xrel_alive else old_info.get("rf_xrel_time", now)
        rb_xrel_time = now if rb_xrel_alive else old_info.get("rb_xrel_time", now)

        # ---------- 检测距离数据是否超时（1秒内无距离数据更新则清空） ----------
        if (now - lf_drel_time) > 1. and lf_drel is not None: lf_drel = None
        if (now - lb_drel_time) > 1. and lb_drel is not None: lb_drel = None
        if (now - rf_drel_time) > 1. and rf_drel is not None: rf_drel = None
        if (now - rb_drel_time) > 1. and rb_drel is not None: rb_drel = None
        if (now - lf_xrel_time) > 1. and lf_xrel is not None: lf_xrel = None
        if (now - lb_xrel_time) > 1. and lb_xrel is not None: lb_xrel = None
        if (now - rf_xrel_time) > 1. and rf_xrel is not None: rf_xrel = None
        if (now - rb_xrel_time) > 1. and rb_xrel is not None: rb_xrel = None

        # ---------- 超时重置逻辑（2秒内无盲区数据更新则清空） ----------
        lidar_lblind_time = old_info.get("lidar_lblind_time", now)
        lidar_rblind_time = old_info.get("lidar_rblind_time", now)

        if (now - lidar_lblind_time) > 2. and lidar_lblind is not None: lidar_lblind = False
        if (now - lidar_rblind_time) > 2. and lidar_rblind is not None: lidar_rblind = False

        # ---------- 更新客户端信息 ----------
        with lock:
          now = time.time()
          self.clients[ip] = {
            "port": int(json_obj.get("port", self.broadcast_port)),
            "last_seen": now,
            "device": device,
            "detect_side": json_obj.get("detect_side", old_info.get("detect_side", 0)),
            "dist_time": dist_timems, #数据时间
            # 盲区状态更新
            "lidar_lblind": lidar_lblind if lidar_lblind is not None else old_info.get("lidar_lblind", False),
            "lidar_rblind": lidar_rblind if lidar_rblind is not None else old_info.get("lidar_rblind", False),
            # 雷达距离更新
            "lf_drel": lf_drel, "lb_drel": lb_drel, "rf_drel": rf_drel, "rb_drel": rb_drel,
            "lf_xrel": lf_xrel, "lb_xrel": lb_xrel, "rf_xrel": rf_xrel, "rb_xrel": rb_xrel,
            # 盲区状态更新时间
            "lidar_lblind_time": now if lidar_lblind is not None else old_info.get("lidar_lblind_time", now),
            "lidar_rblind_time": now if lidar_rblind is not None else old_info.get("lidar_rblind_time", now),
            # 更新距离数据时间
            "lf_drel_time": lf_drel_time, "lb_drel_time": lb_drel_time,
            "rf_drel_time": rf_drel_time, "rb_drel_time": rb_drel_time,
            "lf_xrel_time": lf_xrel_time, "lb_xrel_time": lb_xrel_time,
            "rf_xrel_time": rf_xrel_time, "rb_xrel_time": rb_xrel_time,
          }
      except Exception as e:
        print(f"Process lidar device error: {e}")
        print(json_obj)
    #摄像头
    elif device == "camera":
      try:
        # ---------- 超时重置逻辑（2秒内未收到盲区信号则清空） ----------
        l_blindspot_time = old_info.get("l_blindspot_time", now)
        r_blindspot_time = old_info.get("r_blindspot_time", now)

        if (now - l_blindspot_time) > 2. and left_blind is not None: left_blind = False
        if (now - r_blindspot_time) > 2. and right_blind is not None: right_blind = False

        # ---------- 更新客户端信息 ----------
        with lock:
          now = time.time()
          self.clients[ip] = {
            "port": int(json_obj.get("port", self.broadcast_port)),
            "last_seen": now,
            "device": device,
            "detect_side": json_obj.get("detect_side", old_info.get("detect_side", 0)),
            # 盲区状态更新(本次未更新则保留上次的)
            "left_blind": left_blind if left_blind is not None else old_info.get("left_blind", False),
            "right_blind": right_blind if right_blind is not None else old_info.get("right_blind", False),
            # 盲区状态更新时间
            "l_blindspot_time": now if left_blind is not None else old_info.get("l_blindspot_time", now),
            "r_blindspot_time": now if right_blind is not None else old_info.get("r_blindspot_time", now),
          }
      except Exception as e:
        print(f"Process camera device error: {e}")
        print(json_obj)
    #其他
    else:
      # ---------- 更新客户端信息 ----------
      try:
        with lock:
          now = time.time()
          self.clients[ip] = {
            "port": int(json_obj.get("port", self.broadcast_port)),
            "last_seen": now,
            "device": device,
          }
      except Exception as e:
        print(f"Process other device error: {e}")
        print(json_obj)

  def is_side_object_risky(self, drel_mm, vrel_mps, v_ego_mps,
                           time_horizon=3.0,
                           min_drel_scale=1.0):
    """
    侧向车辆未来距离风险评估（前方和后方通用）
    参数：
      drel_mm       : 当前相对距离（mm），前方为正，后方为负
      vrel_mps      : 相对速度（m/s），对方速度 - 本车速度
      v_ego_mps     : 本车速度（m/s）
      time_horizon  : 预测未来的时间窗，默认 3 秒
      min_drel_scale: 安全距离比例（drel < v * scale 判断），默认 1.0
    返回：
      True  = 未来存在潜在碰撞风险
      False = 安全
    """

    # 数据检查
    if drel_mm is None or vrel_mps is None or v_ego_mps is None:
      return False

    # 距离取米
    drel = abs(drel_mm) / 1000.0

    # 对方速度 = 本车速度 + 相对速度
    v_other = v_ego_mps + vrel_mps

    # closing_speed 为“未来距离缩小的速度”
    if drel_mm > 0:
      # 前方目标：风险来自我追它，所以 closing = max(v_ego - v_other, 0)
      closing_speed = max(v_ego_mps - v_other, 0.0)
      if min_drel_scale >= 0:
        danger_dist = max(v_ego_mps * min_drel_scale, 0)
      else:
        danger_dist = abs(min_drel_scale)
    else:
      # 后方目标：风险来自它追我，所以 closing = max(v_other - v_ego, 0)
      closing_speed = max(v_other - v_ego_mps, 0.0)
      if min_drel_scale >= 0:
        danger_dist = max(v_ego_mps * min_drel_scale, 0)
      else:
        danger_dist = abs(min_drel_scale)

    # 未来距离预测
    future_dist = drel - closing_speed * time_horizon #* 3

    # 判定规则：
    # 1) 未来距离过小（可调阈值 3~5m，我设成 4m)
    # 2) 当前距离小于速度比例阈值（如：d < v * 1.0）
    risk = (
      future_dist < danger_dist or
      drel < danger_dist
    )

    return risk

  '''
  def is_side_object_risky_debug(self,
                                 drel_mm,
                                 vrel_mps,
                                 v_ego_mps,
                                 time_horizon=3.0,
                                 min_drel_scale=1.0,
                                 tag="SIDE"):
    """
    带完整调试信息的侧向风险评估
    """

    print("\n==============================")
    print(f"[{tag}] is_side_object_risky")

    # ---------- 基础输入 ----------
    print(f"input drel_mm   : {drel_mm}")
    print(f"input vrel_mps  : {vrel_mps}")
    print(f"input v_ego_mps : {v_ego_mps}")

    if drel_mm is None or vrel_mps is None or v_ego_mps is None:
      print("invalid input -> risk = False")
      return False

    # ---------- 距离 ----------
    drel_m = drel_mm / 1000.0
    drel_abs = abs(drel_m)

    print(f"drel_m (signed) : {drel_m:.2f} m")
    print(f"drel_abs        : {drel_abs:.2f} m")

    # ---------- 速度 ----------
    v_other = v_ego_mps + vrel_mps
    print(f"v_other_mps     : {v_other:.2f} m/s")

    # ---------- 前 / 后 ----------
    if drel_mm > 0:
      pos = "FRONT"
      closing_speed = max(v_ego_mps - v_other, 0.0)
      danger_dist = max(v_ego_mps * min_drel_scale, 10)
    else:
      pos = "REAR"
      closing_speed = max(v_other - v_ego_mps, 0.0)
      danger_dist = max(v_ego_mps * min_drel_scale, 15)

    print(f"position        : {pos}")
    print(f"closing_speed   : {closing_speed:.2f} m/s")
    print(f"danger_dist     : {danger_dist:.2f} m")

    # ---------- 未来预测 ----------
    horizon = time_horizon * 3.0
    future_dist = drel_abs - closing_speed * horizon

    print(f"time_horizon    : {time_horizon:.2f} s")
    print(f"horizon_factor  : {horizon:.2f}")
    print(f"future_dist     : {future_dist:.2f} m")

    # ---------- 判定 ----------
    cond_future = future_dist < danger_dist
    cond_now = drel_abs < danger_dist

    print(f"cond_future     : {cond_future}")
    print(f"cond_now        : {cond_now}")

    risk = cond_future or cond_now

    print(f"FINAL RISK   : {risk}")
    print("==============================")

    return risk
  '''

  def camera_data_timeout(self, ip, info):
    now = time.time()
    # ---------- 超时重置逻辑（2秒内无盲区数据更新则清空） ----------
    l_blindspot_time = info.get("l_blindspot_time", now)
    r_blindspot_time = info.get("r_blindspot_time", now)

    if (now - l_blindspot_time) > 2. : info["left_blind"] = False
    if (now - r_blindspot_time) > 2. : info["right_blind"] = False

    #更新client的数据
    with lock:
      old_info = self.clients.get(ip, {})
      if old_info:
        self.clients[ip] = info

  def lidar_data_timeout(self, ip, info):
    now = time.time()
    # ---------- 上次雷达数据时间 ----------
    lf_drel_time = info.get("lf_drel_time", now)
    lb_drel_time = info.get("lb_drel_time", now)
    rf_drel_time = info.get("rf_drel_time", now)
    rb_drel_time = info.get("rb_drel_time", now)
    lf_xrel_time = info.get("lf_xrel_time", now)
    lb_xrel_time = info.get("lb_xrel_time", now)
    rf_xrel_time = info.get("rf_xrel_time", now)
    rb_xrel_time = info.get("rb_xrel_time", now)

    # ---------- 检测距离数据是否超时（1秒内无距离数据更新则清空） ----------
    if (now - lf_drel_time) > 1. : info["lf_drel"] = None
    if (now - lb_drel_time) > 1. : info["lb_drel"] = None
    if (now - rf_drel_time) > 1. : info["rf_drel"] = None
    if (now - rb_drel_time) > 1. : info["rb_drel"] = None
    if (now - lf_xrel_time) > 1. : info["lf_xrel"] = None
    if (now - lb_xrel_time) > 1. : info["lb_xrel"] = None
    if (now - rf_xrel_time) > 1. : info["rf_xrel"] = None
    if (now - rb_xrel_time) > 1. : info["rb_xrel"] = None

    # ---------- 超时重置逻辑（2秒内无盲区数据更新则清空） ----------
    lidar_lblind_time = info.get("lidar_lblind_time", now)
    lidar_rblind_time = info.get("lidar_rblind_time", now)

    if (now - lidar_lblind_time) > 2. : info["lidar_lblind"] = False
    if (now - lidar_rblind_time) > 2. : info["lidar_rblind"] = False

    # 更新client的数据
    with lock:
      old_info = self.clients.get(ip, {})
      if old_info:
        self.clients[ip] = info

  # ----------------------
  # 清理超过1秒未活跃客户端线程（修改：同时标记 worker 停止）
  # ----------------------
  def _clean_clients_thread(self):
    while True:
      now = time.time()
      with lock:
        # 保留活跃客户端
        active_clients = {ip: info for ip, info in self.clients.items() if now - info["last_seen"] < 1.0}

        # 标记超时客户端线程停止，并打印超时信息
        for ip, info in self.clients.items():
          if ip not in active_clients and ip in self.client_active:
            last = info.get("last_seen", 0)
            dt = now - last
            print(f"[Client Timeout] ip={ip}, last_seen={last:.3f}, now={now:.3f}, dt={dt:.3f}s")
            self.client_active[ip] = False

        # 更新 clients 列表
        self.clients = active_clients

        # 更新状态
        if self.clients:
          ext_state = len(self.clients)
        else:
          ext_state = 0
          self.shared_data.ext_blinker = BLINKER_NONE
        if self.lane_online:
          ext_state += 1
        self.shared_data.ext_state = ext_state

      time.sleep(0.2)

  def navi_broadcast_info(self):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
    frame = 0
    rk = Ratekeeper(10, print_delay_threshold=0.03)
    broadcast_cnt = 0

    while True:
      try:
        #self.sm.update(0)
        # 修改: 获取当前活跃客户端
        with lock:
          self.clients_copy = getattr(self, "clients", {}).copy()
          self.active_clients = list(self.clients_copy.keys())

        if frame % 20 == 0 or self.active_clients:
          try:
            if not PC:
              ip_address = socket.gethostbyname(socket.gethostname())
            else:
              ip_address = self.navi_get_local_ip()
            if ip_address != self.local_ip_address:
              self.local_ip_address = ip_address
              self.clients_copy = {}
              with lock:
                self.clients = {}  # 修改: 本地 IP 变化时清空客户端

            # 消息
            navi_msg = navi_dat = lidar_msg = lidar_dat = blinker_msg = blinker_dat = None

            if self.active_clients:
              if self.clients_copy:
                # 向所有客户端发送数据
                for ip, info in self.clients_copy.items():
                  try:
                    port_val = info.get("port", self.broadcast_port)
                    if port_val is not None:
                      port = int(port_val)
                    else:
                      port = self.broadcast_port
                    # port = self.broadcast_port
                    device_type = info.get("device", None)
                    detect_side = info.get("detect_side", None)

                    # 根据 device_type 做判断
                    if device_type == "overtake" or device_type == "navi":  # 超车或导航
                      if navi_msg is None:
                        navi_msg = self.make_navi_message()
                        navi_dat = navi_msg.encode('utf-8')
                      if navi_dat is not None:
                        sock.sendto(navi_dat, (ip, port))
                        sock.sendto(navi_dat, (ip, self.navi_remote_port))
                        #if self.app_addr is not None:
                        #  sock.sendto(navi_dat, self.app_addr)
                        if (self.shared_data.showDebugLog & 32) > 0:
                          print(f"sendto {ip} (overtake): {navi_dat}")
                    elif (device_type == "lidar" or device_type == "camera") and ((frame % 3) == 0):  # 雷达模块
                      if lidar_msg is None:
                        lidar_msg = self.make_lidar_message()
                        lidar_dat = lidar_msg.encode('utf-8')
                      if lidar_dat is not None:
                        sock.sendto(lidar_dat, (ip, port))
                        if (self.shared_data.showDebugLog & 32) > 0:
                          print(f"sendto {ip} (lidar): {lidar_dat}")
                    elif ((frame + 3) % 2) == 0:  # 其他
                      if blinker_msg is None:
                        blinker_msg = self.make_blinker_message()
                        blinker_dat = blinker_msg.encode('utf-8')
                      if blinker_dat is not None:
                        sock.sendto(blinker_dat, (ip, port))
                        if (self.shared_data.showDebugLog & 32) > 0:
                          print(f"sendto {ip} (blinker): {blinker_dat}")
                  except Exception as e:
                    if (self.shared_data.showDebugLog & 32) > 0:
                      print(f"sendto {ip} failed: {e}")

            # 每2秒广播一次自己的ip和端口
            if frame % 20 == 0:
              broadcast_msg = self.make_broadcast_message(self.listen_port, "op")
              broadcast_dat = broadcast_msg.encode('utf-8')
              if self.broadcast_ip is None:
                self.broadcast_ip = self.navi_get_broadcast_address()
              if self.broadcast_ip is not None and broadcast_dat is not None:
                sock.sendto(broadcast_dat, (self.broadcast_ip, self.broadcast_port))
                broadcast_cnt += 1
                if (self.shared_data.showDebugLog & 32) > 0:
                  print(f"broadcasting: {self.broadcast_ip}:{self.broadcast_port},{broadcast_msg}")

              lane_broadcast_msg = self.make_broadcast_message(self.lane_port, "lane")
              lane_broadcast_dat = lane_broadcast_msg.encode('utf-8')
              if self.broadcast_ip is not None and lane_broadcast_dat is not None:
                sock.sendto(lane_broadcast_dat, (self.broadcast_ip, self.lane_remote_port))

              navi_broadcast_msg = self.make_broadcast_message(self.navi_port, "navi")
              navi_broadcast_dat = navi_broadcast_msg.encode('utf-8')
              if self.broadcast_ip is not None and navi_broadcast_dat is not None:
                sock.sendto(navi_broadcast_dat, (self.broadcast_ip, self.navi_remote_port))

          except Exception as e:
            if (self.shared_data.showDebugLog & 32) > 0:
              print(f"##### navi_broadcast_error...: {e}")
            # traceback.print_exc()

        rk.keep_time()
        frame += 1
      except Exception as e:
        if (self.shared_data.showDebugLog & 32) > 0:
          print(f"navi_broadcast_info error...: {e}")
        # traceback.print_exc()
        time.sleep(1)

  def make_navi_message(self):
    msg = {}
    msg['ip'] = self.local_ip_address
    msg['port'] = self.listen_port
    msg['device'] = "op"
    isOnroad = self.params.get_bool("IsOnroad")
    msg['IsOnroad'] = isOnroad

    if isOnroad:
      #车辆状态
      if self.shared_data.carState:
        if self.shared_data.v_cruise_kph is not None:
          #msg["cruise_speed"] = self.shared_data.v_cruise_kph  # 巡航速度
          msg['v_cruise_kph'] = self.shared_data.v_cruise_kph  # 巡航速度
        if self.shared_data.v_ego_kph is not None:
          msg['v_ego_kph'] = self.shared_data.v_ego_kph  # 当前速度
        if self.shared_data.vEgo is not None:
          msg["vego"] = self.shared_data.vEgo
        if self.shared_data.aEgo is not None:
          msg["aego"] = self.shared_data.aEgo
        if self.shared_data.steer_angle is not None:
          msg["steer_angle"] = self.shared_data.steer_angle
        if self.shared_data.gas_press is not None:
          msg["gas_press"] = self.shared_data.gas_press
        if self.shared_data.break_press is not None:
          msg["break_press"] = self.shared_data.break_press
        if self.shared_data.engaged is not None:
          msg["engaged"] = self.shared_data.engaged
        # 盲区检测
        if not self.disableBlindSpot:
          if self.shared_data.left_blindspot is not None:
            msg["left_blindspot"] = self.shared_data.left_blindspot
          if self.shared_data.right_blindspot is not None:
            msg["right_blindspot"] = self.shared_data.right_blindspot
        else:
          msg["left_blindspot"] = False
          msg["right_blindspot"] = False

      # 雷达数据
      if self.sm.alive['radarState']:# and self.sm.updated['radarState']:
        radar_state = self.sm['radarState']
        # 当前车道前车
        if hasattr(radar_state, 'leadOne') and radar_state.leadOne and hasattr(radar_state.leadOne,'status') and radar_state.leadOne.status:
          msg["lead1"] = True
          if hasattr(radar_state.leadOne, 'dRel'):
            msg["drel"] = int(radar_state.leadOne.dRel)
          if hasattr(radar_state.leadOne, 'vLead'):
            msg["vlead"] = int(radar_state.leadOne.vLead * 3.6)
          if hasattr(radar_state.leadOne, 'vRel'):
            msg["vrel"] = int(radar_state.leadOne.vRel * 3.6)
          if hasattr(radar_state.leadOne, 'aRel'):
            msg["lead_accel"] = radar_state.leadOne.aRel
        else:
          msg["lead1"] = False
        if not self.disableBlindSpot:
          # 左侧前车
          if hasattr(radar_state, 'leadLeft') and radar_state.leadLeft and hasattr(radar_state.leadLeft,'status') and radar_state.leadLeft.status:
            self.lead_left_right = True
            msg["l_lead"] = True
            if hasattr(radar_state.leadLeft, 'dRel'):
              msg["l_drel"] = int(radar_state.leadLeft.dRel)
            if hasattr(radar_state.leadLeft, 'vLead'):
              msg["l_vlead"] = int(radar_state.leadLeft.vLead * 3.6)
            if hasattr(radar_state.leadLeft, 'vRel'):
              msg["l_vrel"] = int(radar_state.leadLeft.vRel * 3.6)
          else:
            msg["l_lead"] = False
          # 右侧前车
          if hasattr(radar_state, 'leadRight') and radar_state.leadRight and hasattr(radar_state.leadRight,'status') and radar_state.leadRight.status:
            self.lead_left_right = True
            msg["r_lead"] = True
            if hasattr(radar_state.leadRight, 'dRel'):
              msg["r_drel"] = int(radar_state.leadRight.dRel)
            if hasattr(radar_state.leadRight, 'vLead'):
              msg["r_vlead"] = int(radar_state.leadRight.vLead * 3.6)
            if hasattr(radar_state.leadRight, 'vRel'):
              msg["r_vrel"] = int(radar_state.leadRight.vRel * 3.6)
          else:
            msg["r_lead"] = False
        else:
          msg["l_lead"] = False
          msg["r_lead"] = False

      #前方激光雷达速度
      if self.shared_data.lf_vrel is not None:
        msg["lf_vrel"] = int(self.shared_data.lf_vrel * 3.6)
      if self.shared_data.rf_vrel is not None:
        msg["rf_vrel"] = int(self.shared_data.rf_vrel * 3.6)
      #后方激光雷达速度
      if self.shared_data.lb_vrel is not None:
        msg["lb_vrel"] = int(self.shared_data.lb_vrel * 3.6)
      if self.shared_data.rb_vrel is not None:
        msg["rb_vrel"] = int(self.shared_data.rb_vrel * 3.6)

      #前雷达盲区信号
      if self.shared_data.leftFrontBlind is not None:
        msg['l_front_blind'] = self.shared_data.leftFrontBlind
      if self.shared_data.rightFrontBlind is not None:
        msg['r_front_blind'] = self.shared_data.rightFrontBlind

      #雷达和摄像头盲区
      msg['lidar_lblind'] = self.left_blindspot()
      msg['lidar_rblind'] = self.right_blindspot()

      #雷达距离数据
      fields = ["lf_drel", "lb_drel", "rf_drel", "rb_drel", "lf_xrel", "lb_xrel", "rf_xrel", "rb_xrel"]
      # 找出所有 field 的所有 lidar_id
      all_lidar_ids = set()
      for f in fields:
        all_lidar_ids.update(getattr(self.shared_data, f).keys())
      # 按索引顺序遍历
      for idx in sorted(all_lidar_ids):
        for field in fields:
          d = getattr(self.shared_data, field)
          if idx in d and d[idx] is not None:
            key = field if idx == 0 else f"{field}{idx}"
            msg[key] = d[idx]

      #雷达或摄像头是否存在标志
      msg['lidar_l'] = self.shared_data.lidar_l
      msg['lidar_r'] = self.shared_data.lidar_r
      msg['camera_l'] = self.shared_data.camera_l
      msg['camera_r'] = self.shared_data.camera_r

      #来自共享数据
      if self.shared_data.roadcate is not None:
        msg['roadcate'] = self.shared_data.roadcate
      if self.shared_data.lat_a is not None:
        msg['lat_a'] = f2(self.shared_data.lat_a)
      if self.shared_data.max_curve is not None:
        msg['max_curve'] = f2(self.shared_data.max_curve)

      # carrotMan数据
      if self.sm.alive['carrotMan']:
        carrotMan = self.sm['carrotMan']
        msg["desire_speed"] = int(carrotMan.desiredSpeed)  # 期望速度
        #msg['atc_type'] = carrotMan.atcType
        #msg['road_name'] = carrotMan.szPosRoadName
        atc_type = carrotMan.atcType
        road_name = carrotMan.szPosRoadName
        x_spd_type = carrotMan.xSpdType
        x_spd_dist = carrotMan.xSpdDist
        op_blocked = ("none" not in atc_type and "prepare" not in atc_type)
        self.shared_data.op_blocked = op_blocked
        self.shared_data.road_blocked = ("隧道" in road_name) or (x_spd_type >= 0 and 0 < x_spd_dist < 500)

      msg['blind_enable'] = (self.shared_data.lidar_l or self.shared_data.camera_l) and (self.shared_data.lidar_r or self.shared_data.camera_r)
      msg['op_blocked'] = self.shared_data.op_blocked
      msg['road_blocked'] = self.shared_data.road_blocked

    # 来自模型的消息
    if self.sm.alive['modelV2']:
      modelV2 = self.sm['modelV2']
      meta = modelV2.meta
      #添加事件
      if hasattr(meta, 'eventType'):
        model_event_type = meta.eventType
        if model_event_type > 0 and model_event_type != self.model_event_type:
          self.model_event_type = model_event_type
          event_type_val = model_event_type & 255
          event_type_id = int((model_event_type - event_type_val) / 256)
          msg['sound'] = event_type_val
          print(f"------sound index {event_type_val}")
      #倒计时
      if hasattr(meta, 'leftSec'):
        sec_count_down = meta.leftSec
        if self.sec_count_down != sec_count_down:
          self.sec_count_down = sec_count_down
          if sec_count_down == 0: #AudibleAlert.longDisengaged
            msg['sound'] = 26
            print(f"------sound index {msg['sound']}")
          elif 0 < sec_count_down <= 10: #audio_x
            msg['sound'] = 30 + sec_count_down
            print(f"------sound index {msg['sound']}")
          elif sec_count_down == 11: #promptDistracted
            msg['sound'] = 23
            print(f"------sound index {msg['sound']}")
      #转向灯状态
      if hasattr(meta, 'blinker'):
        msg['blinker'] = meta.blinker
      if isOnroad:
        msg['l_lane_width'] = round(meta.laneWidthLeft, 1)
        msg['r_lane_width'] = round(meta.laneWidthRight, 1)
        msg['l_edge_dist'] = round(meta.distanceToRoadEdgeLeft, 1)
        msg['r_edge_dist'] = round(meta.distanceToRoadEdgeRight, 1)
        msg['atc_state'] = meta.laneChangeState.raw
        if hasattr(meta, 'laneWidth'):
          msg['lane_width'] = round(meta.laneWidth, 1)

        if self.shared_data.max_curve is None and hasattr(modelV2, 'orientationRate') and len(modelV2.orientationRate.z) > 0:
          orientation_rate_z = self._capnp_list_to_list(modelV2.orientationRate.z)
          if orientation_rate_z:
            # 找到最大方向变化率（表示最大曲率点）
            max_index = max(range(len(orientation_rate_z)), key=lambda i: abs(orientation_rate_z[i]))
            max_orientation_rate = orientation_rate_z[max_index]
            #curvature_value = float(max_orientation_rate)
            #curvature_direction = 1 if max_orientation_rate > 0 else -1
            msg['max_curve'] = f1(max_orientation_rate)

        # 如果字段存在才赋值
        if self.shared_data.leftFrontBlind is None and hasattr(meta, 'leftFrontBlind'):
          msg['l_front_blind'] = meta.leftFrontBlind
        if self.shared_data.rightFrontBlind is None and hasattr(meta, 'rightFrontBlind'):
          msg['r_front_blind'] = meta.rightFrontBlind

    #===============巡航状态处理==============
    if self.sm.alive['selfdriveState']:
      selfdrive = self.sm['selfdriveState']
      self.shared_data.selfdrive_active = True if selfdrive.active else False
    # 巡航系统是否已激活
    if self.shared_data.cruise_valid is not None:
      msg['active'] = self.shared_data.cruise_valid
    # 是否在巡航
    cruise_enable = False
    #if self.shared_data.cruise_enable is not None and self.shared_data.cruise_enable:
    #  cruise_enable = True
    if self.shared_data.selfdrive_active is not None and self.shared_data.selfdrive_active:
      cruise_enable = True
    # 在巡航，active强制为True
    if cruise_enable:
      msg['active'] = True
    # 若engaged不存在则赋值巡航状态
    if not msg.get('engaged', False):
      msg['engaged'] = cruise_enable

    return json.dumps(msg)

  def make_lidar_message(self):
    msg = {}
    msg['ip'] = self.local_ip_address
    msg['port'] = self.listen_port
    msg['device'] = "op"
    isOnroad = self.params.get_bool("IsOnroad")
    msg['IsOnroad'] = isOnroad

    if isOnroad:
      if self.shared_data.carState:
        if self.shared_data.v_cruise_kph is not None:
          msg['v_cruise_kph'] = self.shared_data.v_cruise_kph  # 巡航速度
        if self.shared_data.v_ego_kph is not None:
          msg['v_ego_kph'] = self.shared_data.v_ego_kph  # 当前速度

      #===============巡航状态处理==============
      if self.sm.alive['selfdriveState']:
        selfdrive = self.sm['selfdriveState']
        self.shared_data.selfdrive_active = True if selfdrive.active else False
      # 巡航系统是否已激活
      if self.shared_data.cruise_valid is not None:
        msg['active'] = self.shared_data.cruise_valid
      # 是否在巡航
      cruise_enable = False
      #if self.shared_data.cruise_enable is not None and self.shared_data.cruise_enable:
      #  cruise_enable = True
      if self.shared_data.selfdrive_active is not None and self.shared_data.selfdrive_active:
        cruise_enable = True
      # 在巡航，active强制为True
      if cruise_enable:
        msg['active'] = True
      # 若engaged不存在则赋值巡航状态
      if not msg.get('engaged', False):
        msg['engaged'] = cruise_enable

      if self.sm.alive['carrotMan']:
        carrotMan = self.sm['carrotMan']
        msg['atc_type'] = carrotMan.atcType
        msg['road_name'] = carrotMan.szPosRoadName

    return json.dumps(msg)

  def make_blinker_message(self):
    msg = {}
    msg['ip'] = self.local_ip_address
    msg['port'] = self.listen_port
    msg['device'] = "op"
    isOnroad = self.params.get_bool("IsOnroad")
    msg['IsOnroad'] = isOnroad

    if isOnroad:
      # 车辆状态
      if self.shared_data.carState:
        if self.shared_data.v_cruise_kph is not None:
          msg['v_cruise_kph'] = self.shared_data.v_cruise_kph  # 巡航速度
        if self.shared_data.v_ego_kph is not None:
          msg['v_ego_kph'] = self.shared_data.v_ego_kph  # 当前速度
        if self.shared_data.vEgo is not None:
          msg["vego"] = self.shared_data.vEgo
        if self.shared_data.gas_press is not None:
          msg["gas_press"] = self.shared_data.gas_press
        if self.shared_data.break_press is not None:
          msg["break_press"] = self.shared_data.break_press
      # 雷达数据
      if self.sm.alive['radarState']:  # and self.sm.updated['radarState']:
        radar_state = self.sm['radarState']
        # 当前车道前车
        if hasattr(radar_state, 'leadOne') and radar_state.leadOne and hasattr(radar_state.leadOne,'status') and radar_state.leadOne.status:
          msg["lead1"] = True
          if hasattr(radar_state.leadOne, 'dRel'):
            msg["drel"] = f1(radar_state.leadOne.dRel)
          if hasattr(radar_state.leadOne, 'vLead'):
            msg["vlead"] = f1(radar_state.leadOne.vLead * 3.6)
          if hasattr(radar_state.leadOne, 'vRel'):
            msg["vrel"] = f1(radar_state.leadOne.vRel * 3.6)
          if hasattr(radar_state.leadOne, 'aRel'):
            msg["lead_accel"] = radar_state.leadOne.aRel
        else:
          msg["lead1"] = False

    # 来自模型的消息
    if self.sm.alive['modelV2']:
      meta = self.sm['modelV2'].meta
      if hasattr(meta, 'blinker'):
        msg['blinker'] = meta.blinker

    #===============巡航状态处理==============
    if self.sm.alive['selfdriveState']:
      selfdrive = self.sm['selfdriveState']
      self.shared_data.selfdrive_active = True if selfdrive.active else False
    # 巡航系统是否已激活
    if self.shared_data.cruise_valid is not None:
      msg['active'] = self.shared_data.cruise_valid
    # 是否在巡航
    cruise_enable = False
    #if self.shared_data.cruise_enable is not None and self.shared_data.cruise_enable:
    #  cruise_enable = True
    if self.shared_data.selfdrive_active is not None and self.shared_data.selfdrive_active:
      cruise_enable = True
    # 在巡航，active强制为True
    if cruise_enable:
      msg['active'] = True
    # 若engaged不存在则赋值巡航状态
    if not msg.get('engaged', False):
      msg['engaged'] = cruise_enable

    return json.dumps(msg)

  def make_broadcast_message(self, port, device):
    msg = {}
    msg['ip'] = self.local_ip_address
    msg['port'] = port
    msg['device'] = device

    return json.dumps(msg)

  def navi_get_broadcast_address(self):
    # 修改为支持PC的多接口检测
    if PC:
        interfaces = ['wlan0', 'eth0', 'enp0s3', 'br0']  # 常见PC接口
        for iface in interfaces:
            try:
                with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
                    ip = fcntl.ioctl(
                        s.fileno(),
                        0x8919,  # SIOCGIFBRDADDR
                        struct.pack('256s', iface.encode('utf-8')[:15])
                    )[20:24]
                    return socket.inet_ntoa(ip)
            except Exception:
                continue
        return "255.255.255.255"  # 回退地址
    else:
      iface = b'wlan0'
    try:
      with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        ip = fcntl.ioctl(
          s.fileno(),
          0x8919,
          struct.pack('256s', iface)
        )[20:24]
        return socket.inet_ntoa(ip)
    except (OSError, Exception):
      return None

  def navi_get_local_ip(self):
      try:
          # 외부 서버와의 연결을 통해 로컬 IP 확인
          with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
              s.connect(("8.8.8.8", 80))  # Google DNS로 연결 시도
              return s.getsockname()[0]
      except Exception as e:
          return f"Error: {e}"

def main():
  amap_navi = AmapNaviServ()
  #amap_navi.navi_comm_thread()

if __name__ == "__main__":
  main()
