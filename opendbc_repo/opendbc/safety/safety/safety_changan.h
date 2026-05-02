#pragma once

#include "safety_declarations.h"

// Must match opendbc.car.changan.values.ChanganSafetyFlags
#define CHANGAN_PARAM_Z6_IDD_FLAG (1U << 2)

static void changan_rx_hook(const CANPacket_t *to_push) {
  const int bus = GET_BUS(to_push);
  const int addr = GET_ADDR(to_push);
  const bool idd = GET_FLAG(current_safety_param, CHANGAN_PARAM_Z6_IDD_FLAG);

  if (bus == 0) {
    if (addr == 0x180U) {
      int angle_raw = ((int)GET_BYTE(to_push, 0) << 8) | (int)GET_BYTE(to_push, 1);
      angle_raw = to_signed(angle_raw, 16);
      update_sample(&angle_meas, angle_raw * 10);
    }

    if ((!idd && (addr == 0x187U)) || (idd && (addr == 0x17AU))) {
      const uint16_t spd_raw = (uint16_t)(((uint32_t)GET_BYTE(to_push, 4) << 8) | (uint32_t)GET_BYTE(to_push, 5));
      const uint16_t spd13 = (uint16_t)((spd_raw >> 5) & 0x1FFFU);
      vehicle_moving = spd13 > 0U;
      UPDATE_VEHICLE_SPEED((float)spd13 * 0.05625f / 3.6f);
    }

    if (addr == 0x17EU) {
      const uint8_t lat_avail = (uint8_t)((GET_BYTE(to_push, 6) >> 5) & 3U);
      pcm_cruise_check(lat_avail == 1U);
    }

    if (!idd) {
      if (addr == 0x196U) {
        gas_pressed = GET_BYTE(to_push, 0) > 0U;
        brake_pressed = GET_BIT(to_push, 55U);
      }
    } else {
      if (addr == 0x1A6U) {
        brake_pressed = GET_BIT(to_push, 35U) || GET_BIT(to_push, 36U);
      }
      if (addr == 0x1C6U) {
        gas_pressed = GET_BYTE(to_push, 0) > 0U;
      }
    }
  }

  generic_rx_checks(false);
}

static bool changan_tx_hook(const CANPacket_t *to_send) {
  const int bus = GET_BUS(to_send);
  const int addr = GET_ADDR(to_send);
  const int len = GET_LEN(to_send);

  const bool adas_pt = ((addr == 0x244U) || (addr == 0x1BAU) || (addr == 0x307U) || (addr == 0x31AU)) && (len == 8) && (bus == 0);
  const bool eps_fwd = (addr == 0x17EU) && (len == 8) && ((bus == 0U) || (bus == 2U));

  if (!controls_allowed && (adas_pt || eps_fwd)) {
    return false;
  }
  return true;
}

static safety_config changan_init(uint16_t param) {
  static RxCheck changan_rx_z6[] = {
    {.msg = {{0x180, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x187, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x17E, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x196, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x244, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 50U}, {0}, {0}}},
    {.msg = {{0x1BA, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x307, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 10U}, {0}, {0}}},
    {.msg = {{0x31A, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 10U}, {0}, {0}}},
  };

  static RxCheck changan_rx_idd[] = {
    {.msg = {{0x180, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x17A, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x17E, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x1A6, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x1C6, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x244, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 50U}, {0}, {0}}},
    {.msg = {{0x1BA, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    {.msg = {{0x307, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 10U}, {0}, {0}}},
    {.msg = {{0x31A, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 10U}, {0}, {0}}},
  };

  static const CanMsg CHANGAN_TX_MSGS[] = {
    {0x244, 0, 8}, {0x1BA, 0, 8}, {0x307, 0, 8}, {0x31A, 0, 8},
    {0x17E, 0, 8}, {0x17E, 2, 8},
  };

  if (GET_FLAG(param, CHANGAN_PARAM_Z6_IDD_FLAG)) {
    return BUILD_SAFETY_CFG(changan_rx_idd, CHANGAN_TX_MSGS);
  }
  return BUILD_SAFETY_CFG(changan_rx_z6, CHANGAN_TX_MSGS);
}

const safety_hooks changan_hooks = {
  .init = changan_init,
  .rx = changan_rx_hook,
  .tx = changan_tx_hook,
  .fwd = default_fwd_hook,
};
