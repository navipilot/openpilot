#pragma once

#include "safety_declarations.h"

// CAN msgs we care about
#define CHANGAN_STEER_ANGLE      0x180 // SAS_SteeringAngle
#define CHANGAN_STEER_COMMAND    0x1BA // GW_1BA
#define CHANGAN_STEER_TORQUE     0x17E // GW_17E
#define CHANGAN_WHEEL_SPEEDS     0x187 // GW_187 (Petrol)
#define CHANGAN_IDD_WHEEL_SPEEDS 0x17A // SPEED (IDD)
#define CHANGAN_PEDAL_DATA       0x196 // GW_196 (Petrol Brake/Gas)
#define CHANGAN_IDD_PEDAL_DATA   0x1A6 // GW_1A6 (IDD Brake/Gas)
#define CHANGAN_ACC_COMMAND      0x244 // GW_244
#define CHANGAN_CRUISE_BUTTONS   0x28C // GW_28C
#define CHANGAN_ADAS_INFO        0x31A // GW_31A (ACC State Info)
#define CHANGAN_ACC_HUD          0x382 // GW_382 (CANFD 64byte)

// GW_28C MFS巡航按钮信号定义（大端，每个信号2bit，非零即按下）
// DBC: GW_MFS_Crusie_switch_signal   : 1|2@0+  => byte0 bits[1:0]
// DBC: GW_MFS_Cancle_switch_signal   : 3|2@0+  => byte0 bits[3:2]
// DBC: GW_MFS_RESPlus_switch_signal  : 5|2@0+  => byte0 bits[5:4]
// DBC: GW_MFS_SETReduce_switch_signal: 7|2@0+  => byte0 bits[7:6]
#define CHANGAN_BTN_MAIN_MASK    0x03U  // byte0 bits[1:0]
#define CHANGAN_BTN_CANCEL_MASK  0x0CU  // byte0 bits[3:2]
#define CHANGAN_BTN_RESUME_MASK  0x30U  // byte0 bits[5:4]
#define CHANGAN_BTN_SET_MASK     0xC0U  // byte0 bits[7:6]

const AngleSteeringLimits CHANGAN_STEER_LIMITS = {
  .max_angle = 1800,          // 180.0 degrees * 10
  .angle_deg_to_can = 10.,
  .angle_rate_up_lookup = {
    .x = {0., 10., 20.},
    .y = {5., 1.4, 0.15},
  },
  .angle_rate_down_lookup = {
    .x = {0., 10., 20.},
    .y = {5., 1.4, 0.26},
  },
  .enforce_angle_error = false,
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

static uint8_t changan_cruise_button_prev = 0x00U;

static uint32_t changan_get_checksum(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  if (addr == 0x180 || addr == 0x17E || addr == 0x187 || addr == 0x17A ||
      addr == 0x196 || addr == 0x1A6 || addr == 0x244 || addr == 0x28C ||
      addr == 0x307 || addr == 0x31A || addr == 0x382) {
    return GET_BYTE(to_push, 7);
  }
  return 0;
}

static uint32_t changan_compute_checksum(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  if (addr == 0x180 || addr == 0x17E || addr == 0x187 || addr == 0x17A ||
      addr == 0x196 || addr == 0x1A6 || addr == 0x244 || addr == 0x28C ||
      addr == 0x307 || addr == 0x31A || addr == 0x382) {
    uint8_t checksum = 0;
    for (int i = 0; i < 7; i++) {
      checksum = changan_crc8_tab[checksum ^ GET_BYTE(to_push, i)];
    }
    return checksum;
  }
  return 0;
}

static uint8_t changan_get_counter(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  if (addr == 0x180 || addr == 0x17E || addr == 0x187 || addr == 0x17A ||
      addr == 0x196 || addr == 0x1A6 || addr == 0x244 || addr == 0x28C ||
      addr == 0x307 || addr == 0x31A || addr == 0x382) {
    return (GET_BYTE(to_push, 6) >> 4) & 0xF;
  }
  return 0;
}

static void changan_rx_hook(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  int bus = GET_BUS(to_push);

  // ── controls_allowed 同步 ──────────────────────────────────────
  // 从 GW_244 (bus2) 读取 ACC_ACCMode，与 carstate.py 逻辑保持一致：
  //   ACC_ACCMode==3 => Active => controls_allowed=true
  //   其他值 (2=Standby, 7=Fault...) => controls_allowed=false
  //
  // DBC: ACC_ACCMode : 54|3@0+ in GW_244 (32byte CANFD)
  //   start_bit=54, 3bits big-endian
  //   byte6=[55,54,53,52,51,50,49,48]: bit54=byte6 bit6, 3bits => byte6 bits[6:4]
  //   读取: (GET_BYTE(6) >> 4) & 0x7
  if (bus == 2 && addr == CHANGAN_ACC_COMMAND) {
    uint8_t acc_mode = (GET_BYTE(to_push, 6) >> 4) & 0x7U;
    if (acc_mode == 3U) {
      controls_allowed = true;
    } else {
      controls_allowed = false;
    }
  }

  // ── 巡航按钮：Cancel 立即撤销控制权 ───────────────────────────
  // DBC GW_28C: GW_MFS_Cancle_switch_signal : 3|2@0+ => byte0 bits[3:2]
  // 按下时(非零)立即清除 controls_allowed
  if ((bus == 0 || bus == 2) && addr == CHANGAN_CRUISE_BUTTONS) {
    uint8_t current_button = GET_BYTE(to_push, 0);
    bool btn_cancel = (current_button & CHANGAN_BTN_CANCEL_MASK) != 0U;
    if (btn_cancel) {
      controls_allowed = false;
    }
    changan_cruise_button_prev = current_button;
  }

  // ── 车速更新 ────────────────────────────────────────────────────
  if (bus == 0) {
    // GW_187 Petrol: ESP_VehicleSpeed : 36|13@0+
    //   start_bit=36, 13bit big-endian
    //   byte4=[39,38,37,36,35,34,33,32]: bit36=byte4 bit4, 5bits in byte4 bits[4:0]
    //   byte5=[47..40]: 全部8bits (LSB部分)
    //   读取: ((byte4 & 0x1F) << 8) | byte5, factor=0.05625 km/h
    if (addr == CHANGAN_WHEEL_SPEEDS) {
      int speed = (((int)(GET_BYTE(to_push, 4) & 0x1FU)) << 8) | GET_BYTE(to_push, 5);
      UPDATE_VEHICLE_SPEED(speed * 0.05625 / 3.6);
    }
    // 转向角度: GW_180 SAS_SteeringAngle : 7|16@0+ (大端16bit => byte0高8 + byte1低8)
    if (addr == CHANGAN_STEER_ANGLE) {
      int angle_meas_new = (GET_BYTE(to_push, 0) << 8) | GET_BYTE(to_push, 1);
      update_sample(&angle_meas, to_signed(angle_meas_new, 16));
    }
    // GW_196 Petrol Brake/Gas (修复：按照DBC正确bit位提取)
    // EMS_BrakePedalStatus : 55|2@0+ => byte6 bits[7:6]
    // EMS_RealAccPedal     :  7|8@0+ => byte0 全部
    if (addr == CHANGAN_PEDAL_DATA) {
      brake_pressed = ((GET_BYTE(to_push, 6) >> 6) & 0x3U) != 0U;
      gas_pressed   = GET_BYTE(to_push, 0) != 0U;
    }
  }

  if (bus == 2) {
    // GW_17A IDD: ESP_VehicleSpeed : 36|13@0+ (同GW_187结构相同)
    if (addr == CHANGAN_IDD_WHEEL_SPEEDS) {
      int speed = (((int)(GET_BYTE(to_push, 4) & 0x1FU)) << 8) | GET_BYTE(to_push, 5);
      UPDATE_VEHICLE_SPEED(speed * 0.05625 / 3.6);
    }
    // GW_1A6 IDD Brake/Gas (修复：按照DBC正确bit位提取)
    // EMS_BrakePedalStatus : 35|2@0+ => byte4 bits[3:2]
    // EMS_AccPedal         : 47|8@0+ => byte5 全部
    if (addr == CHANGAN_IDD_PEDAL_DATA) {
      brake_pressed = ((GET_BYTE(to_push, 4) >> 2) & 0x3U) != 0U;
      gas_pressed   = GET_BYTE(to_push, 5) != 0U;
    }
  }

  // 检测原车ECU转向消息（stock ECU detected on bus2）
  bool stock_ecu_detected = (bus == 2) && (addr == CHANGAN_STEER_COMMAND);
  generic_rx_checks(stock_ecu_detected);
}

static bool changan_tx_hook(const CANPacket_t *to_send) {
  int addr = GET_ADDR(to_send);
  bool tx = true;
  bool violation = false;

  // ── 转向控制消息 GW_1BA (0x1BA) ────────────────────────────────
  // DBC: EPS_AngleCmd    : 17|14@0+ (0.1,-720) => Motorola 14bit
  //   MSB at bit17: byte2=[23,22,21,20,19,18,17,16], bit17=byte2 bit1
  //   Bit sequence (MSB->LSB): bit17(B2b1), bit16(B2b0),
  //                            bit15..8(B1全部8bit),
  //                            bit7..4(B0 bits[7:4])
  //   Raw = ((byte2 & 0x03) << 12) | (byte1 << 4) | ((byte0 >> 4) & 0x0F)
  //   Physical = raw * 0.1 - 720  =>  raw = (angle_deg + 720) / 0.1
  //
  // DBC: EPS_LatCtrlActive : 35|1@0+ => byte4=[39,38,37,36,35,34,33,32], bit35=byte4 bit3
  if (addr == CHANGAN_STEER_COMMAND) {
    // 提取 EPS_AngleCmd raw值 (14bit无符号，物理值=raw*0.1-720)
    int raw_angle = (((int)(GET_BYTE(to_send, 2) & 0x03U)) << 12) |
                    ((int)GET_BYTE(to_send, 1) << 4) |
                    (((int)GET_BYTE(to_send, 0) >> 4) & 0x0FU);
    // 转换为物理角度值（单位：degrees * 10，对应 max_angle=1800）
    // DBC物理值 = raw * 0.1 - 720, 安全层内部以0.1度为单位 => deg_x10 = raw - 7200
    int desired_angle = raw_angle - 7200;  // 单位: 0.1度, range: [-7200, 9000+]

    // 提取 EPS_LatCtrlActive (1bit)
    bool steer_req = ((GET_BYTE(to_send, 4) >> 3) & 0x1U) != 0U;

    // 安全检查1: 控制权限
    if (steer_req && !controls_allowed) {
      violation = true;
    }

    // 安全检查2: 角度超限（steer_angle_cmd_checks内部会与max_angle=1800比较）
    if (steer_angle_cmd_checks(desired_angle, steer_req, CHANGAN_STEER_LIMITS)) {
      violation = true;
    }
  }

  // ── 纵向控制消息 GW_244 (0x244) 和 GW_382 (0x382) ─────────────
  // 这些消息在 controls_allowed=false 时禁止发送
  if (addr == CHANGAN_ACC_COMMAND || addr == CHANGAN_ACC_HUD) {
    if (!controls_allowed) {
      violation = true;
    }
  }

  // ── HUD 消息 GW_307 (0x307) 和 GW_31A (0x31A) ─────────────────
  // HUD状态显示消息，不涉及直接控制，无需 controls_allowed 检查

  if (violation) {
    tx = false;
  }
  return tx;
}

static int changan_fwd_hook(int bus, int addr) {
  int bus_fwd = -1;
  if (bus == 0) {
    bus_fwd = 2;
  }
  if (bus == 2) {
    // 屏蔽由openpilot接管的控制消息，防止原车ECU和openpilot同时发送冲突
    bool block = (addr == CHANGAN_STEER_COMMAND) ||  // 0x1BA 转向
                 (addr == CHANGAN_ACC_COMMAND)   ||  // 0x244 纵向ACC
                 (addr == 0x307)                 ||  // GW_307 巡航速度HUD
                 (addr == 0x31A)                 ||  // GW_31A ADAS状态HUD
                 (addr == CHANGAN_ACC_HUD);           // 0x382 GW_382
    if (!block) {
      bus_fwd = 0;
    }
  }
  return bus_fwd;
}

static safety_config changan_init(uint16_t param) {
  controls_allowed = false;
  heartbeat_engaged = false;
  heartbeat_engaged_mismatches = 0U;
  changan_cruise_button_prev = 0x00U;

  // TX消息列表
  // 注意: GW_307(0x307)和GW_31A(0x31A)是64字节CANFD消息
  //       GW_382(0x382)是64字节CANFD消息
  //       GW_1BA(0x1BA)和GW_244(0x244)是32字节CANFD消息
  // 如果panda固件不支持CANFD，需要将len改为8或使用CANFD模式
  static const CanMsg CHANGAN_TX_MSGS[] = {
    {CHANGAN_STEER_COMMAND, 0, 32},  // GW_1BA: 转向控制 (32byte CANFD)
    {CHANGAN_ACC_COMMAND,   0, 32},  // GW_244: 纵向ACC控制 (32byte CANFD)
    {0x307,                 0, 64},  // GW_307: 巡航速度HUD (64byte CANFD)
    {0x31A,                 0, 64},  // GW_31A: ADAS状态HUD (64byte CANFD)
    {CHANGAN_ACC_HUD,       2, 64},  // GW_382: ACC辅助信息 (64byte CANFD, bus2)
  };

  static RxCheck changan_rx_checks[] = {
    // 转向角度 GW_180 (bus0, 8byte, 100Hz)
    {.msg = {{CHANGAN_STEER_ANGLE, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},
    // 踏板数据: GW_196(Petrol, bus0) 或 GW_1A6(IDD, bus0) 均为100Hz
    {.msg = {{CHANGAN_PEDAL_DATA,     0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {CHANGAN_IDD_PEDAL_DATA, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}}},
    // ACC状态 GW_244 (bus2, 32byte CANFD, 50Hz) — 用于同步 controls_allowed
    {.msg = {{CHANGAN_ACC_COMMAND, 2, 32, .ignore_checksum = true, .ignore_counter = true, .frequency = 50U}, {0}, {0}}},
    // 巡航按钮 GW_28C (bus0 或 bus2, 8byte, 50Hz)
    {.msg = {{CHANGAN_CRUISE_BUTTONS, 0, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 50U},
             {CHANGAN_CRUISE_BUTTONS, 2, 8, .ignore_checksum = true, .ignore_counter = true, .frequency = 50U}, {0}}},
    // 车速: GW_187(Petrol, bus0) 或 GW_17A(IDD, bus2) 均为100Hz，GW_17A是64byte CANFD
    {.msg = {{CHANGAN_WHEEL_SPEEDS,     0, 8,  .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {CHANGAN_IDD_WHEEL_SPEEDS, 2, 64, .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}}},
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
