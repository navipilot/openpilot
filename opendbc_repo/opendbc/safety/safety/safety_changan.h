#pragma once

#include "safety_declarations.h"

// ── CAN 消息地址 ──────────────────────────────────────────────────
#define CHANGAN_STEER_ANGLE      0x180  // GW_180 (bus0, 8byte)
#define CHANGAN_STEER_COMMAND    0x1BA  // GW_1BA (bus0, 32byte, openpilot TX)
#define CHANGAN_STEER_TORQUE     0x17E  // GW_17E (bus0+bus2, 8byte, openpilot TX ×2)
#define CHANGAN_WHEEL_SPEEDS     0x187  // GW_187 Petrol (bus0, 8byte)
#define CHANGAN_IDD_WHEEL_SPEEDS 0x17A  // GW_17A IDD    (bus0, 64byte)
#define CHANGAN_PEDAL_DATA       0x196  // GW_196 Petrol (bus0, 8byte)
#define CHANGAN_IDD_PEDAL_DATA   0x1A6  // GW_1A6 IDD    (bus0, 8byte)
#define CHANGAN_ACC_COMMAND      0x244  // GW_244 (bus2 RX, 32byte)
#define CHANGAN_CRUISE_BUTTONS   0x28C  // GW_28C (bus0, 8byte)
#define CHANGAN_ACC_SET_SPEED    0x307  // GW_307 (bus0, 64byte, openpilot TX)
#define CHANGAN_ADAS_INFO        0x31A  // GW_31A (bus0, 64byte, openpilot TX)

#define CHANGAN_BTN_CANCEL_MASK  0x0CU  // GW_28C byte0 bits[3:2]

const AngleSteeringLimits CHANGAN_STEER_LIMITS = {
  .max_angle = 1800,
  .angle_deg_to_can = 10.,
  .angle_rate_up_lookup   = { .x = {0., 10., 20.}, .y = {5., 1.4, 0.15} },
  .angle_rate_down_lookup = { .x = {0., 10., 20.}, .y = {5., 1.4, 0.26} },
  .enforce_angle_error    = false,
  .inactive_angle_is_zero = false,
};

static const uint8_t changan_crc8_tab[256] = {
  0x00, 0x1D, 0x3A, 0x27, 0x74, 0x69, 0x4E, 0x53, 0xE8, 0xF5, 0xD2, 0xCF, 0x9C, 0x81, 0xA6, 0xBB,
  0xCD, 0xD0, 0xF7, 0xEA, 0xB9, 0xA4, 0x83, 0x9E, 0x25, 0x38, 0x1F, 0x02, 0x51, 0x4C, 0x6B, 0x76,
  0x87, 0x9A, 0xBD, 0xA0, 0xF3, 0xEE, 0xC9, 0xD4, 0x6F, 0x72, 0x55, 0x48, 0x1B, 0x06, 0x21, 0x3C,
  0x4A, 0x57, 0x70, 0x6D, 0x3E, 0x23, 0x04, 0x19, 0xA2, 0xBF, 0x98, 0x85, 0xD6, 0xCB, 0xEC, 0xF1,
  0x13, 0x0E, 0x29, 0x34, 0x67, 0x7A, 0x5D, 0x40, 0xFB, 0xE6, 0xC1, 0xDC, 0x8F, 0x92, 0xB5, 0xA8,
  0xDE, 0xC3, 0xE4, 0xF9, 0xAA, 0xB7, 0x90, 0x8D, 0x36, 0x2B, 0x0C, 0x11, 0x42, 0x5F, 0x78, 0x65,
  0x94, 0x89, 0xAE, 0xB3, 0xE0, 0xFD, 0xDA, 0xC7, 0x7C, 0x61, 0x46, 0x5B, 0x08, 0x15, 0x32, 0x2F,
  0x59, 0x44, 0x63, 0x7E, 0x2D, 0x30, 0x17, 0x0A, 0xB1, 0xAC, 0x8B, 0x96, 0xC5, 0xD8, 0xFF, 0xE2,
  0x26, 0x3B, 0x1C, 0x01, 0x52, 0x4F, 0x68, 0x75, 0xCE, 0xD3, 0xF4, 0xE9, 0xBA, 0xA7, 0x80, 0x9D,
  0xEB, 0xF6, 0xD1, 0xCC, 0x9F, 0x82, 0xA5, 0xB8, 0x03, 0x1E, 0x39, 0x24, 0x77, 0x6A, 0x4D, 0x50,
  0xA1, 0xBC, 0x9B, 0x86, 0xD5, 0xC8, 0xEF, 0xF2, 0x49, 0x54, 0x73, 0x6E, 0x3D, 0x20, 0x07, 0x1A,
  0x6C, 0x71, 0x56, 0x4B, 0x18, 0x05, 0x22, 0x3F, 0x84, 0x99, 0xBE, 0xA3, 0xF0, 0xED, 0xCA, 0xD7,
  0x35, 0x28, 0x0F, 0x12, 0x41, 0x5C, 0x7B, 0x66, 0xDD, 0xC0, 0xE7, 0xFA, 0xA9, 0xB4, 0x93, 0x8E,
  0xF8, 0xE5, 0xC2, 0xDF, 0x8C, 0x91, 0xB6, 0xAB, 0x10, 0x0D, 0x2A, 0x37, 0x64, 0x79, 0x5E, 0x43,
  0xB2, 0xAF, 0x88, 0x95, 0xC6, 0xDB, 0xFC, 0xE1, 0x5A, 0x47, 0x60, 0x7D, 0x2E, 0x33, 0x14, 0x09,
  0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C, 0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4
};

static uint32_t changan_get_checksum(const CANPacket_t *to_push) {
  (void)to_push; return 0U;
}
static uint32_t changan_compute_checksum(const CANPacket_t *to_push) {
  (void)to_push; return 0U;
}
static uint8_t changan_get_counter(const CANPacket_t *to_push) {
  (void)to_push; return 0U;
}

static void changan_rx_hook(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  int bus  = GET_BUS(to_push);

// 1. 同时检查 Bus 0 和 Bus 2 的 ACC 状态（防止接线差异导致无法同步控制权）
  if (addr == CHANGAN_ACC_COMMAND) {
    uint8_t acc_mode = (GET_BYTE(to_push, 6) >> 4) & 0x7U; // 36|13@0+
    controls_allowed = (acc_mode == 3U);
  }

  // 2. 增加刹车强制撤销控制权（安全冗余）
  if (addr == CHANGAN_PEDAL_DATA || addr == CHANGAN_IDD_PEDAL_DATA) {
     bool brake = (addr == CHANGAN_PEDAL_DATA) ? (((GET_BYTE(to_push, 6) >> 6) & 0x3U) != 0U) : (((GET_BYTE(to_push, 4) >> 2) & 0x3U) != 0U);
     if (brake) {
       controls_allowed = false;
     }
  }

  // 车速: GW_187/GW_17A  ESP_VehicleSpeed : 36|13@0+ (0.05625,0)
  if (bus == 0 && (addr == CHANGAN_WHEEL_SPEEDS || addr == CHANGAN_IDD_WHEEL_SPEEDS)) {
    int spd = (((int)(GET_BYTE(to_push, 4) & 0x1FU)) << 8) | GET_BYTE(to_push, 5);
    UPDATE_VEHICLE_SPEED(spd * 0.05625 / 3.6);
  }

  // 转向角: GW_180  SAS_SteeringAngle : 7|16@0-
  if (bus == 0 && addr == CHANGAN_STEER_ANGLE) {
    int raw = (GET_BYTE(to_push, 0) << 8) | GET_BYTE(to_push, 1);
    update_sample(&angle_meas, to_signed(raw, 16));
  }

  // 踏板 Petrol: GW_196
  if (bus == 0 && addr == CHANGAN_PEDAL_DATA) {
    brake_pressed = ((GET_BYTE(to_push, 6) >> 6) & 0x3U) != 0U;
    gas_pressed   = GET_BYTE(to_push, 0) != 0U;
  }

  // 踏板 IDD: GW_1A6
  if (bus == 0 && addr == CHANGAN_IDD_PEDAL_DATA) {
    brake_pressed = ((GET_BYTE(to_push, 4) >> 2) & 0x3U) != 0U;
    gas_pressed   = GET_BYTE(to_push, 5) != 0U;
  }

  // GW_1BA 在 bus2 是原车正常广播，不是继电器故障
  generic_rx_checks(false);
}

static bool changan_tx_hook(const CANPacket_t *to_send) {
  int addr = GET_ADDR(to_send);
  bool violation = false;

  // HUD 和心跳消息：始终放行（含 bus0 和 bus2 的 GW_17E）
  if (addr == CHANGAN_STEER_TORQUE ||
      addr == CHANGAN_ACC_SET_SPEED ||
      addr == CHANGAN_ADAS_INFO) {
    return true;
  }

  // GW_1BA：转向控制
  // EPS_AngleCmd : 17|14@0+ (0.1,-720)
  //   raw = ((byte2&0x03)<<12)|(byte1<<4)|((byte0>>4)&0x0F)
  //   desired(0.1°) = raw - 7200
  // EPS_LatCtrlActive : 35|1@0+ => byte4 bit3
  if (addr == CHANGAN_STEER_COMMAND) {
    int raw_angle = (((int)(GET_BYTE(to_send, 2) & 0x03U)) << 12) |
                    ((int)GET_BYTE(to_send, 1) << 4) |
                    (((int)GET_BYTE(to_send, 0) >> 4) & 0x0FU);
    int desired_angle = raw_angle - 7200;
    bool steer_req = ((GET_BYTE(to_send, 4) >> 3) & 0x1U) != 0U;

    if (steer_req && !controls_allowed) {
      violation = true;
    }
    if (steer_angle_cmd_checks(desired_angle, steer_req, CHANGAN_STEER_LIMITS)) {
      violation = true;
    }
  }

  return !violation;
}

static int changan_fwd_hook(int bus, int addr) {
  if (bus == 0) {
    return 2;
  }
  if (bus == 2) {
    // 屏蔽 openpilot 在 bus0 重发的消息
    bool block = (addr == CHANGAN_STEER_COMMAND) ||  // 0x1BA
                 (addr == CHANGAN_STEER_TORQUE)  ||  // 0x17E (bus0版)
                 (addr == CHANGAN_ACC_SET_SPEED) ||  // 0x307
                 (addr == CHANGAN_ADAS_INFO);         // 0x31A
    return block ? -1 : 0;
  }
  return -1;
}

static safety_config changan_init(uint16_t param) {
  controls_allowed = false;
  heartbeat_engaged = false;
  heartbeat_engaged_mismatches = 0U;

  // TX 列表：与 carcontroller.py 完全对应
  // GW_17E 发两次：bus0(0) + bus2(2)，逆向确认
  static const CanMsg CHANGAN_TX_MSGS[] = {
    {CHANGAN_STEER_COMMAND, 0, 32},  // GW_1BA: 转向控制 bus0
    {CHANGAN_STEER_TORQUE,  0,  8},  // GW_17E: EPS心跳  bus0
    {CHANGAN_STEER_TORQUE,  2,  8},  // GW_17E: EPS心跳  bus2（逆向确认）
    {CHANGAN_ACC_SET_SPEED, 0, 64},  // GW_307: 巡航HUD  bus0
    {CHANGAN_ADAS_INFO,     0, 64},  // GW_31A: ADAS HUD bus0
  };

  static RxCheck changan_rx_checks[] = {
    {.msg = {{CHANGAN_STEER_ANGLE, 0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {0}, {0}}},
    {.msg = {{CHANGAN_PEDAL_DATA,     0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {CHANGAN_IDD_PEDAL_DATA, 0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {0}}},
    {.msg = {{CHANGAN_WHEEL_SPEEDS,     0,  8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {CHANGAN_IDD_WHEEL_SPEEDS, 0, 64,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {0}}},
    {.msg = {{CHANGAN_ACC_COMMAND, 2, 32,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 50U},
             {0}, {0}}},
    {.msg = {{CHANGAN_CRUISE_BUTTONS, 0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 50U},
             {0}, {0}}},
  };

  UNUSED(param);
  return BUILD_SAFETY_CFG(changan_rx_checks, CHANGAN_TX_MSGS);
}

const safety_hooks changan_hooks = {
  .init             = changan_init,
  .rx               = changan_rx_hook,
  .tx               = changan_tx_hook,
  .fwd              = changan_fwd_hook,
  .get_checksum     = changan_get_checksum,
  .compute_checksum = changan_compute_checksum,
  .get_counter      = changan_get_counter,
};