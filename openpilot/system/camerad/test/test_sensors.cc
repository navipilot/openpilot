#include <cmath>

#include "common/hardware/comma/hardware.h"
#include "common/tests/native_test.h"
#include "system/camerad/cameras/spectra.h"
#include "system/camerad/sensors/sensor.h"

struct SpectraCameraTest {
  static void check_sync(cereal::FrameData::ImageSensor sensor, uint64_t cabin_offset, bool expected) {
    SpectraCamera::camera_sync_data.clear();
    SpectraCamera::first_frame_synced = false;
    for (const auto &config : ALL_CAMERA_CONFIGS) {
      const uint64_t offset = config.camera_num == CABIN_CAMERA_CONFIG.camera_num ? cabin_offset : 0;
      CHECK(!SpectraCamera::syncFirstFrame(config.camera_num, 1, 1, 1000000000ULL + offset,
                                          config.uses_staggered_sof(sensor)));
    }
    CHECK(SpectraCamera::first_frame_synced == expected);
    if (expected) {
      CHECK(SpectraCamera::syncFirstFrame(0, 2, 2, 1050000000ULL, false));
      for (const auto &[camera, data] : SpectraCamera::camera_sync_data) {
        CHECK(data.frame_id_offset == 2);
      }
    }
  }
};

void test_sensors() {
  using ImageSensor = cereal::FrameData::ImageSensor;
  using DeviceType = cereal::InitData::DeviceType;
  CHECK(HardwareComma::get_device_type("tici") == DeviceType::TICI);
  CHECK(HardwareComma::get_device_type("tizi") == DeviceType::TIZI);
  CHECK(HardwareComma::get_device_type("mici") == DeviceType::MICI);

  AR0231 ar;
  OX03C10 ox;
  OS04C10 os;
  CHECK(ar.image_sensor == ImageSensor::AR0231);
  CHECK(ar.frame_width == 1928 && ar.frame_height == 1208);
  CHECK(ar.frame_stride == 2896);
  CHECK(ar.extra_height == 12 && ar.frame_offset == 2);
  CHECK(ar.registers_offset == 0 && ar.stats_offset == 1210);
  CHECK(ar.probe_reg_addr == 0x3000 && ar.probe_expected_data == 0x354);
  CHECK(ar.data_word && ar.bits_per_pixel == 12);
  CHECK(ar.mclk_frequency == 19200000 && ar.readout_time_ns == 22850000);
  CHECK(ar.getSlaveAddress(0) == 0x20);
  CHECK(ar.getSlaveAddress(1) == 0x30);
  CHECK(ar.getSlaveAddress(2) == 0x20);
  CHECK(ar.start_reg_array.size() == 1);
  CHECK(ar.start_reg_array[0].reg_addr == 0x301A && ar.start_reg_array[0].reg_data == 0x91C);
  CHECK(ar.gamma_lut_rgb.size() == 64 && ar.vignetting_lut.size() == 221);
  CHECK(ar.linearization_lut.size() == 36 && ar.color_correct_matrix.size() == 9);

  for (bool dc : {false, true}) {
    for (int gain = ar.analog_gain_min_idx; gain <= ar.analog_gain_max_idx; ++gain) {
      for (int exposure : {ar.exposure_time_min, ar.exposure_time_max}) {
        const auto regs = ar.getExposureRegisters(exposure, gain, dc);
        CHECK(regs.size() == 3);
        CHECK(regs[0].reg_addr == 0x3366 && regs[0].reg_data == uint32_t(0xFF00 | gain << 4 | gain));
        CHECK(regs[1].reg_addr == 0x3362 && regs[1].reg_data == uint32_t(dc));
        CHECK(regs[2].reg_addr == 0x3012 && regs[2].reg_data == uint32_t(exposure));
        const float ev = exposure * ar.sensor_analog_gains[gain];
        CHECK(std::isfinite(ar.getExposureScore(ev, exposure, gain, ar.sensor_analog_gains[gain], gain)));
      }
    }
  }

  CHECK(ar.bps_blob_index() == 2);
  CHECK(ox.bps_blob_index() == ox.num() && ox.bps_blob_index() == 2);
  CHECK(os.bps_blob_index() == os.num() && os.bps_blob_index() == 3);
  CHECK(ar.frame_width == ox.frame_width && ar.frame_height == ox.frame_height);
  CHECK(ar.frame_stride == ox.frame_stride);

  CHECK(!CABIN_CAMERA_CONFIG.uses_staggered_sof(ImageSensor::AR0231));
  CHECK(CABIN_CAMERA_CONFIG.uses_staggered_sof(ImageSensor::OX03C10));
  CHECK(CABIN_CAMERA_CONFIG.uses_staggered_sof(ImageSensor::OS04C10));
  for (ImageSensor sensor : {ImageSensor::AR0231, ImageSensor::OX03C10, ImageSensor::OS04C10}) {
    CHECK(!NARROW_ROAD_CAMERA_CONFIG.uses_staggered_sof(sensor));
    CHECK(!WIDE_ROAD_CAMERA_CONFIG.uses_staggered_sof(sensor));
    const uint64_t expected_offset = sensor == ImageSensor::AR0231 ? 0 : 25000000ULL;
    SpectraCameraTest::check_sync(sensor, expected_offset, true);
    SpectraCameraTest::check_sync(sensor, expected_offset + 200000ULL, true);
    SpectraCameraTest::check_sync(sensor, expected_offset + 200001ULL, false);
    SpectraCameraTest::check_sync(sensor, sensor == ImageSensor::AR0231 ? 25000000ULL : 0, false);
  }
}

int main() {
  return run_native_test(test_sensors);
}
