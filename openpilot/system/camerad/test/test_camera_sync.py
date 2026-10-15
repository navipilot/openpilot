import unittest

import numpy as np

from openpilot.system.camerad.test import test_camerad


class TestCameraSync(unittest.TestCase):
  @staticmethod
  def camera_logs(sensor, cabin_offset_ns):
    frames = np.arange(30)
    timestamps = 1000000000 + frames * 50000000
    return {
      camera: {
        "t": timestamps / 1e9,
        "frameId": frames.copy(),
        "timestampSof": timestamps + (cabin_offset_ns if camera == "cabinCameraState" else 0),
        "imageSensor": np.full(len(frames), sensor),
      }
      for camera in test_camerad.CAMERAS
    }

  @staticmethod
  def check_logs(logs):
    checker = test_camerad.TestCamerad("test_frame_sync")
    checker.logs = logs
    checker.test_frame_sync()

  def test_sensor_specific_phase(self):
    for sensor, offset in (("ar0231", 0), ("ox03c10", 25000000), ("os04c10", 25000000)):
      with self.subTest(sensor=sensor):
        self.check_logs(self.camera_logs(sensor, offset))

  def test_wrong_sensor_phase_is_rejected(self):
    for sensor, offset in (("ar0231", 25000000), ("ox03c10", 0), ("os04c10", 0)):
      with self.subTest(sensor=sensor), self.assertRaises(AssertionError):
        self.check_logs(self.camera_logs(sensor, offset))

  def test_frame_ids_must_remain_aligned(self):
    logs = self.camera_logs("ar0231", 0)
    logs["cabinCameraState"]["frameId"] += 1
    with self.assertRaisesRegex(AssertionError, "frame IDs not aligned"):
      self.check_logs(logs)

  def test_road_phase_tolerance_is_preserved(self):
    logs = self.camera_logs("ar0231", 0)
    logs["wideRoadCameraState"]["timestampSof"] += 1100001
    with self.assertRaisesRegex(AssertionError, "Frames not synced properly"):
      self.check_logs(logs)


if __name__ == "__main__":
  unittest.main()
