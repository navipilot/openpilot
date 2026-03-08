#!/usr/bin/env python3
"""
OpenPilot 彩色视频流服务器
正式版本 - 优化性能与颜色校正
"""

import time
import threading
import numpy as np
#from flask import Flask, Response, render_template_string
#import cv2

import sys

FLASK_AVAILABLE = True
OPENCV_AVAILABLE = True

try:
    from flask import Flask, Response, render_template_string
except ImportError as e:
    print("[FATAL] Flask 未安装，无法启动服务")
    print("请安装: pip install flask")
    FLASK_AVAILABLE = False

try:
    import cv2
except ImportError as e:
    print("[FATAL] OpenCV(cv2) 未安装，无法启动服务")
    print("请安装: pip install opencv-python")
    OPENCV_AVAILABLE = False

# 全局变量
latest_jpeg = None
latest_gray = None
vipc_width = None
vipc_height = None
vipc_stride = None
target_width = 416
target_height = 416
JPEG_QUALITY = 50
gray_img = False

# 请求统计
req_lock = threading.Lock()
req_count = 0
req_window_start = time.time()
REQ_WINDOW = 2.0   # 统计 2 秒内的请求数
latest_req_count = 0
req_frame_time = 0.05

frame_lock = threading.Lock()
last_snapshot_time = 0
status_text = "waiting app connect..."
status_lock = threading.Lock()

HTML = """
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Lane Detect</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body {
            margin: 0;
            background: black;
            color: #0f0;
            font-family: monospace;
            font-size: 18px;
            display: flex;
            align-items: center;
            justify-content: center;
            height: 100vh;
        }
        #text {
            white-space: pre;
        }
    </style>
</head>
<body>
<div id="text">waiting app connect...</div>
<script>
    const el = document.getElementById("text");
    function update() {
        fetch("/status")
            .then(r => r.text())
            .then(t => el.textContent = t)
            .catch(() => {});
    }
    update();
    setInterval(update, 1000);   //每秒刷新一次
</script>
</body>
</html>
"""

def y_to_jpeg(buf, w, h, stride, target_w, target_h, quality=70):
  """
  快速灰度 JPEG（Y plane）
  - 整数下采样，低 CPU
  - 中心裁剪到 target_w × target_h
  - 避免 cv2.resize
  """
  try:
    # 取 Y plane
    data = np.frombuffer(buf, dtype=np.uint8)
    y_plane = data[:h * stride].reshape(h, stride)[:, :w]

    # 计算缩放比例（覆盖目标尺寸）
    scale_x = target_w / w
    scale_y = target_h / h
    scale = max(scale_x, scale_y)

    # 计算整数下采样步长
    step_x = max(1, int(1 / scale))
    step_y = max(1, int(1 / scale))

    # 下采样
    y_ds = y_plane[0:h:step_y, 0:w:step_x]
    ds_h, ds_w = y_ds.shape

    # 中心裁剪
    start_x = max(0, (ds_w - target_w) // 2)
    start_y = max(0, (ds_h - target_h) // 2)
    y_crop = y_ds[start_y:start_y + target_h, start_x:start_x + target_w]

    # 确保偶数尺寸（Skia 安全）
    y_crop = y_crop[:target_h & ~1, :target_w & ~1]

    # JPEG 编码
    ok, jpg = cv2.imencode(
      ".jpg",
      y_crop,
      [cv2.IMWRITE_JPEG_QUALITY, quality]
    )
    return jpg.tobytes() if ok else None

  except Exception as e:
    print("Y->JPEG fast error:", e)
    return None

def convert_yuv_to_bgr(yuv_data, width, height, stride):
    """YUV NV12 转 BGR (OpenCV默认顺序)"""
    try:
        y_size = height * stride
        y_plane = yuv_data[:y_size].reshape(height, stride)
        uv_plane = yuv_data[y_size:y_size + (height//2) * stride].reshape(height//2, stride)

        if stride > width:
            y_plane = y_plane[:, :width]
            uv_plane = uv_plane[:, :width]

        yuv_nv12 = np.vstack([y_plane, uv_plane])
        bgr_img = cv2.cvtColor(yuv_nv12, cv2.COLOR_YUV2BGR_NV12)
        return bgr_img
    except:
        return None

def yuv_nv12_to_small_bgr(yuv_data, width, height, stride, target_w, target_h):
  """
  用整数下采样先缩小 NV12，再转 BGR
  """
  y_size = height * stride
  y_plane = yuv_data[:y_size].reshape(height, stride)[:, :width]
  uv_plane = yuv_data[y_size:y_size + (height // 2) * stride].reshape(height // 2, stride)[:, :width]

  # 计算下采样倍数，确保大于 target
  scale_h = height // target_h
  scale_w = width // target_w
  scale = max(1, min(scale_h, scale_w))  # 整数倍采样

  # Y plane 下采样
  y_small = y_plane[::scale, ::scale]

  # UV plane 下采样（每两行、两列采样）
  uv_small = uv_plane[::scale, ::scale]

  # 合并回 NV12 小尺寸
  nv12_small = np.vstack([y_small, uv_small])

  # 转成 BGR
  bgr_img = cv2.cvtColor(nv12_small, cv2.COLOR_YUV2BGR_NV12)

  return bgr_img

def encode_to_jpg(image, target_w, target_h, quality):
  """
  固定尺寸 + 低延迟 JPEG
  """
  # 固定尺寸 resize（不等比例）
  if image.shape[1] != target_w or image.shape[0] != target_h:
    image = cv2.resize(
      image,
      (target_w, target_h),
      interpolation=cv2.INTER_AREA  #更快
    )
  # JPEG 编码（去掉 OPTIMIZE）
  success, jpeg = cv2.imencode(
    ".jpg",
    image,
    [cv2.IMWRITE_JPEG_QUALITY, quality]
  )
  return jpeg.tobytes() if success else None

def camera_thread():
  global latest_jpeg, latest_gray, status_text, gray_img
  global vipc_width, vipc_height, vipc_stride

  from msgq.visionipc.visionipc_pyx import VisionIpcClient, VisionStreamType

  vipc_client = VisionIpcClient(
    "camerad",
    VisionStreamType.VISION_STREAM_ROAD,
    False  # 非实时，更省 CPU
  )

  for _ in range(3):
    if vipc_client.connect(False):
      break
    time.sleep(1)
  else:
    return

  vipc_width = vipc_client.width
  vipc_height = vipc_client.height
  vipc_stride = vipc_client.stride

  last_print = time.time()
  frame_times = []

  while True:
    # 没客户端，不干活
    if time.time() - last_snapshot_time > 2.0:
      time.sleep(0.1)
      continue

    yuv_buf = vipc_client.recv()
    if not yuv_buf:
      time.sleep(0.05)
      continue

    start_time = time.time()

    bgr = None
    if gray_img: #只处理灰度图像
      jpeg = y_to_jpeg(yuv_buf.data, vipc_width, vipc_height, vipc_stride, target_width, target_height, JPEG_QUALITY)
      if jpeg is None:
        time.sleep(0.05)
        continue
      with frame_lock:
        latest_gray = jpeg
    else:
      bgr = convert_yuv_to_bgr(yuv_buf.data,vipc_width,vipc_height,vipc_stride) # YUV → BGR（临时）
      #bgr = yuv_nv12_to_small_bgr(yuv_buf.data, vipc_width, vipc_height, vipc_stride, target_width, target_height)
      if bgr is None:
        time.sleep(0.05)
        continue
      # BGR → JPEG（唯一输出）
      jpeg = encode_to_jpg(bgr, target_width, target_height, JPEG_QUALITY)
      if jpeg is None:
        time.sleep(0.05)
        continue
      with frame_lock:
        latest_jpeg = jpeg

    # 统计
    t = time.time() - start_time
    frame_times.append(t * 1000)

    if time.time() - last_print > 2.0:
      if frame_times:
        text = (
          f"JPEG {target_width}x{target_height} | "
          f"avg {np.mean(frame_times):.1f} ms | "
          f"FPS {len(frame_times) / 2:.1f}"
        )
        #print(text)
        with status_lock:
          status_text = text
      frame_times.clear()
      last_print = time.time()

    # 显式释放（帮助 GC）
    if bgr is not None:
      del bgr

    #是否需要延时
    with req_lock:
      _req_frame_time = req_frame_time
    sleep_time = _req_frame_time - t if  _req_frame_time > t else 0
    if sleep_time > 0:
      time.sleep(sleep_time)


def ensure_package(max_retry=10, retry_delay=3):
  """
  确保 Python 包可用：
  - import 失败则自动 pip install
  - 每 10 秒重试一次
  - 最多重试 10 次
  - 最终失败返回 None
  """
  import subprocess
  import_name = "flask"
  pip_name = import_name
  import_success = False

  for attempt in range(1, max_retry + 1):
    try:
      from flask import Flask, Response, render_template_string
      import_success = True
      break
    except ImportError:
      print(f"[WARN] {import_name} 未安装，尝试安装 {pip_name} ({attempt}/{max_retry})")
      try:
        subprocess.run("pip install flask", shell=True, capture_output=True, text=False)
      except Exception as e:
        print(f"[ERROR] pip install {pip_name} 失败: {e}")

      if attempt < max_retry:
        try:
          from flask import Flask, Response, render_template_string
          import_success = True
          break
        except ImportError:
          print(f"[INFO] {retry_delay}s 后重试...")
          time.sleep(retry_delay)

  if not import_success:
    print(f"[FATAL] {import_name} 安装失败，已重试 {max_retry} 次")
    return import_success

  import_success = False
  import_name = "opencv-python"
  pip_name = import_name

  for attempt in range(1, max_retry + 1):
    try:
      import cv2
      import_success = True
    except ImportError:
      print(f"[WARN] {import_name} 未安装，尝试安装 {pip_name} ({attempt}/{max_retry})")
      try:
        subprocess.run("pip install opencv-python", shell=True, capture_output=True, text=False)
      except Exception as e:
        print(f"[ERROR] pip install {pip_name} 失败: {e}")

      if attempt < max_retry:
        try:
          import cv2
          import_success = True
          break
        except ImportError:
          print(f"[INFO] {retry_delay}s 后重试...")
          time.sleep(retry_delay)

  if not import_success:
    print(f"[FATAL] {import_name} 安装失败，已重试 {max_retry} 次")
  return import_success

def main():
    while not (FLASK_AVAILABLE and OPENCV_AVAILABLE):
      print("=" * 60)
      print("Lane 视频流服务未启动（缺少依赖）")
      print("=" * 60)
      #result = ensure_package()
      #if result:
      #  break
      while True:
        try:
          from flask import Flask, Response, render_template_string
          break
        except ImportError:
          print(f"[ERROR] flask未安装，请手动执行命令pip install flask进行安装")
        time.sleep(30)
      while True:
        try:
          import cv2
          break
        except ImportError:
          print(f"[ERROR] opencv未安装，请手动执行命令pip install opencv-python进行安装")
        time.sleep(30)

    import logging
    logging.getLogger('werkzeug').setLevel(logging.ERROR)

    print("=" * 60)
    print("车道线服务程序")
    print("访问: http://0.0.0.0:8888")
    print("=" * 60)

    cam_thread = threading.Thread(target=camera_thread, daemon=True)
    cam_thread.start()

    time.sleep(1)

    from flask import Flask, Response, render_template_string
    import cv2
    app = Flask(__name__)

    @app.route('/')
    def index():
      return render_template_string(HTML)

    @app.route("/status")
    def status():
      with status_lock:
        return status_text

    @app.route("/roadrgb.jpg")
    def roadrgb():
      """返回最新一帧 JPEG，用于 Android 单帧抓取"""
      global latest_jpeg, last_snapshot_time, gray_img
      global req_count, latest_req_count, req_frame_time, req_window_start
      gray_img = False
      last_snapshot_time = time.time()

      with req_lock:
        now = time.time()
        if now - req_window_start > REQ_WINDOW:
          req_window_start = now
          latest_req_count = req_count
          if latest_req_count >= 1:
            req_frame_time = 2.0 / latest_req_count
            # print(f"request {latest_req_count}, interval {req_frame_time:.2f} s")
          req_count = 0
        req_count += 1

      with frame_lock:
        jpeg = latest_jpeg
      # 如果没有帧，返回一张黑色占位图
      if jpeg is None:
        import numpy as np
        import cv2
        placeholder = np.zeros((target_height, target_width, 3), dtype=np.uint8)
        _, jpeg = cv2.imencode('.jpg', placeholder)
        jpeg = jpeg.tobytes()

      return Response(jpeg, mimetype="image/jpeg")

    @app.route("/roadgray.jpg")
    def roadgray():
      """返回最新一帧灰度 JPEG，用于 Android 单帧抓取"""
      global latest_gray, last_snapshot_time, gray_img
      global req_count, latest_req_count, req_frame_time, req_window_start
      gray_img = True
      last_snapshot_time = time.time()

      with req_lock:
        now = time.time()
        if now - req_window_start > REQ_WINDOW:
          req_window_start = now
          latest_req_count = req_count
          if latest_req_count >= 1:
            req_frame_time = 2.0 / latest_req_count
            # print(f"request {latest_req_count}, interval {req_frame_time:.2f} s")
          req_count = 0
        req_count += 1

      with frame_lock:
        jpeg = latest_gray

      if jpeg is None:
        placeholder = np.zeros((target_height, target_width), dtype=np.uint8)
        ok, jpeg = cv2.imencode(
          ".jpg",
          placeholder,
          [cv2.IMWRITE_JPEG_QUALITY, JPEG_QUALITY]
        )
        jpeg = jpeg.tobytes() if ok else b""

      return Response(jpeg, mimetype="image/jpeg")

    from werkzeug.serving import run_simple
    run_simple('0.0.0.0', 8888, app, threaded=True, processes=1, use_reloader=False)

if __name__ == "__main__":
    main()
