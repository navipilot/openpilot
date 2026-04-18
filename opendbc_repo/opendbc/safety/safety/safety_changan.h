#pragma once

#include "safety_declarations.h"

// ── CAN 消息地址 ──────────────────────────────────────────────────
#define CHANGAN_STEER_ANGLE      0x180  // GW_180: 转向角 (bus0, 8byte)
#define CHANGAN_STEER_COMMAND    0x1BA  // GW_1BA: 转向控制 (bus0, 32byte CANFD, openpilot发送)
#define CHANGAN_STEER_TORQUE     0x17E  // GW_17E: EPS状态  (bus2, 8byte,   openpilot发送)
#define CHANGAN_WHEEL_SPEEDS     0x187  // GW_187: 车速 Petrol (bus0, 8byte)
#define CHANGAN_IDD_WHEEL_SPEEDS 0x17A  // GW_17A: 车速 IDD    (bus0, 64byte CANFD)
#define CHANGAN_PEDAL_DATA       0x196  // GW_196: 油门/制动 Petrol (bus0, 8byte)
#define CHANGAN_IDD_PEDAL_DATA   0x1A6  // GW_1A6: 油门/制动 IDD    (bus0, 8byte)
#define CHANGAN_ACC_COMMAND      0x244  // GW_244: 纵向ACC控制 (bus2 rx / bus0 tx, 32byte CANFD)
#define CHANGAN_CRUISE_BUTTONS   0x28C  // GW_28C: MFS巡航按钮 (bus0, 8byte)
#define CHANGAN_ACC_SET_SPEED    0x307  // GW_307: 巡航速度HUD (bus0, 64byte CANFD, openpilot发送)
#define CHANGAN_ADAS_INFO        0x31A  // GW_31A: ADAS状态HUD (bus0, 64byte CANFD, openpilot发送)
#define CHANGAN_GEAR_INFO        0x331  // GW_331: 挡位信息 Z6  (bus0, 8byte, 可选)

// ── GW_28C MFS 按钮位定义 (大端, 每信号2bit) ─────────────────────
// GW_MFS_Crusie_switch_signal   : 1|2@0+  => byte0 bits[1:0]
// GW_MFS_Cancle_switch_signal   : 3|2@0+  => byte0 bits[3:2]
// GW_MFS_RESPlus_switch_signal  : 5|2@0+  => byte0 bits[5:4]
// GW_MFS_SETReduce_switch_signal: 7|2@0+  => byte0 bits[7:6]
#define CHANGAN_BTN_MAIN_MASK    0x03U
#define CHANGAN_BTN_CANCEL_MASK  0x0CU
#define CHANGAN_BTN_RESUME_MASK  0x30U
#define CHANGAN_BTN_SET_MASK     0xC0U

// ── 转向角限制 ────────────────────────────────────────────────────
const AngleSteeringLimits CHANGAN_STEER_LIMITS = {
  .max_angle = 1800,        // 180.0 deg × 10
  .angle_deg_to_can = 10.,
  .angle_rate_up_lookup   = { .x = {0., 10., 20.}, .y = {5., 1.4, 0.15} },
  .angle_rate_down_lookup = { .x = {0., 10., 20.}, .y = {5., 1.4, 0.26} },
  .enforce_angle_error = false,
  .inactive_angle_is_zero = false,
};

// ── CRC8 查找表 ───────────────────────────────────────────────────
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

// ── Checksum / Counter（仅用于 rx_checks 框架校验，实际 ignore） ──
static uint32_t changan_get_checksum(const CANPacket_t *to_push) {
  // 所有 rx 消息均设 ignore_checksum=true，此函数不影响实际行为
  (void)to_push;
  return 0U;
}

static uint32_t changan_compute_checksum(const CANPacket_t *to_push) {
  (void)to_push;
  return 0U;
}

static uint8_t changan_get_counter(const CANPacket_t *to_push) {
  // GW_28C: GW_MFS_RollingCounter_28C : 51|4@0+ => byte6 bits[7:4]
  if (GET_ADDR(to_push) == CHANGAN_CRUISE_BUTTONS) {
    return (GET_BYTE(to_push, 6) >> 4) & 0xFU;
  }
  return 0U;
}

// ── RX Hook ───────────────────────────────────────────────────────
static void changan_rx_hook(const CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  int bus  = GET_BUS(to_push);

  // controls_allowed 同步：从 GW_244 (bus2) 读取 ACC_ACCMode
  // DBC: ACC_ACCMode : 54|3@0+ (big-endian 3bit)
  //   bit54 在 byte6=[55,54,53,52,51,50,49,48]: bit54 = byte6 bit6
  //   3bits (54,53,52) => byte6 bits[6:4] => (GET_BYTE(6) >> 4) & 0x7
  //   ACC_ACCMode == 3 => Active => controls_allowed = true
  if (bus == 2 && addr == CHANGAN_ACC_COMMAND) {
    uint8_t acc_mode = (GET_BYTE(to_push, 6) >> 4) & 0x7U;
    controls_allowed = (acc_mode == 3U);
  }

  // Cancel 按钮：立即撤销控制权
  // GW_28C: GW_MFS_Cancle_switch_signal : 3|2@0+ => byte0 bits[3:2]
  if (bus == 0 && addr == CHANGAN_CRUISE_BUTTONS) {
    uint8_t btn = GET_BYTE(to_push, 0);
    if ((btn & CHANGAN_BTN_CANCEL_MASK) != 0U) {
      controls_allowed = false;
    }
    changan_cruise_button_prev = btn;
  }

  // 车速：GW_187 / GW_17A
  // ESP_VehicleSpeed : 36|13@0+ (0.05625,0) km/h
  // bit36 大端: byte4=[39,38,37,36,...], 5bits in byte4[4:0]; byte5 全部8bits
  // raw = ((byte4 & 0x1F) << 8) | byte5
  if (bus == 0 && addr == CHANGAN_WHEEL_SPEEDS) {
    int speed_raw = (((int)(GET_BYTE(to_push, 4) & 0x1FU)) << 8) | GET_BYTE(to_push, 5);
    UPDATE_VEHICLE_SPEED(speed_raw * 0.05625 / 3.6);
  }
  if (bus == 0 && addr == CHANGAN_IDD_WHEEL_SPEEDS) {
    int speed_raw = (((int)(GET_BYTE(to_push, 4) & 0x1FU)) << 8) | GET_BYTE(to_push, 5);
    UPDATE_VEHICLE_SPEED(speed_raw * 0.05625 / 3.6);
  }

  // 转向角：GW_180 SAS_SteeringAngle : 7|16@0- (0.1,0) 有符号
  // big-endian 16bit: byte0 高8bit + byte1 低8bit
  if (bus == 0 && addr == CHANGAN_STEER_ANGLE) {
    int angle_raw = (GET_BYTE(to_push, 0) << 8) | GET_BYTE(to_push, 1);
    update_sample(&angle_meas, to_signed(angle_raw, 16));
  }

  // 制动 / 油门（Petrol）：GW_196
  // EMS_BrakePedalStatus : 55|2@0+ => byte6 bits[7:6]
  // EMS_RealAccPedal     :  7|8@0+ => byte0 全部
  if (bus == 0 && addr == CHANGAN_PEDAL_DATA) {
    brake_pressed = ((GET_BYTE(to_push, 6) >> 6) & 0x3U) != 0U;
    gas_pressed   = GET_BYTE(to_push, 0) != 0U;
  }

  // 制动 / 油门（IDD）：GW_1A6
  // EMS_BrakePedalStatus : 35|2@0+ => byte4 bits[3:2]
  // EMS_AccPedal         : 47|8@0+ => byte5 全部
  if (bus == 0 && addr == CHANGAN_IDD_PEDAL_DATA) {
    brake_pressed = ((GET_BYTE(to_push, 4) >> 2) & 0x3U) != 0U;
    gas_pressed   = GET_BYTE(to_push, 5) != 0U;
  }

  // 原车 ECU 转向消息检测（bus2 上有 GW_1BA 说明原车 ECU 在发）
  bool stock_ecu_detected = (bus == 2) && (addr == CHANGAN_STEER_COMMAND);
  generic_rx_checks(stock_ecu_detected);
}

// ── TX Hook ───────────────────────────────────────────────────────
static bool changan_tx_hook(const CANPacket_t *to_send) {
  int addr = GET_ADDR(to_send);
  bool violation = false;

  // GW_1BA (0x1BA): 转向控制
  // DBC: EPS_AngleCmd    : 17|14@0+ (0.1,-720)
  //   raw = ((byte2 & 0x03) << 12) | (byte1 << 4) | ((byte0 >> 4) & 0x0F)
  //   physical = raw * 0.1 - 720  => safety单位(×10°): desired = raw - 7200
  // DBC: EPS_LatCtrlActive : 35|1@0+ => byte4 bit3
  if (addr == CHANGAN_STEER_COMMAND) {
    int raw_angle = (((int)(GET_BYTE(to_send, 2) & 0x03U)) << 12) |
                    ((int)GET_BYTE(to_send, 1) << 4)               |
                    (((int)GET_BYTE(to_send, 0) >> 4) & 0x0FU);
    int desired_angle = raw_angle - 7200;  // 单位: 0.1°，max_angle=1800 对应 180°

    bool steer_req = ((GET_BYTE(to_send, 4) >> 3) & 0x1U) != 0U;

    if (steer_req && !controls_allowed) {
      violation = true;
    }
    if (steer_angle_cmd_checks(desired_angle, steer_req, CHANGAN_STEER_LIMITS)) {
      violation = true;
    }
  }

  // GW_244 (0x244): 纵向 ACC 控制
  // controls_allowed=false 时禁止发送，防止未授权时下发加速度指令
  if (addr == CHANGAN_ACC_COMMAND) {
    if (!controls_allowed) {
      violation = true;
    }
  }

  // GW_17E (0x17E): EPS 心跳，始终允许发送（无直接控制量）
  // GW_307 (0x307) / GW_31A (0x31A): HUD 状态，无需 controls_allowed 检查

  return !violation;
}

// ── FWD Hook ──────────────────────────────────────────────────────
static int changan_fwd_hook(int bus, int addr) {
  if (bus == 0) {
    return 2;  // bus0 所有消息透传至 bus2
  }
  if (bus == 2) {
    // 屏蔽 openpilot 在 bus0 重发的消息，防止双发冲突
    bool block = (addr == CHANGAN_STEER_COMMAND) ||  // 0x1BA 转向
                 (addr == CHANGAN_STEER_TORQUE)  ||  // 0x17E EPS心跳
                 (addr == CHANGAN_ACC_COMMAND)   ||  // 0x244 纵向ACC
                 (addr == CHANGAN_ACC_SET_SPEED) ||  // 0x307 巡航速度HUD
                 (addr == CHANGAN_ADAS_INFO);         // 0x31A ADAS HUD
    if (!block) {
      return 0;
    }
  }
  return -1;
}

// ── Init ──────────────────────────────────────────────────────────
static safety_config changan_init(uint16_t param) {
  controls_allowed = false;
  heartbeat_engaged = false;
  heartbeat_engaged_mismatches = 0U;
  changan_cruise_button_prev = 0x00U;

  // TX 消息列表（与 carcontroller.py 严格对应）
  static const CanMsg CHANGAN_TX_MSGS[] = {
    {CHANGAN_STEER_COMMAND, 0, 32},  // GW_1BA: 转向控制
    {CHANGAN_STEER_TORQUE,  2, 8},   // GW_17E: EPS心跳（cam bus）
    {CHANGAN_ACC_COMMAND,   0, 32},  // GW_244: 纵向ACC
    {CHANGAN_ACC_SET_SPEED, 0, 64},  // GW_307: 巡航速度HUD
    {CHANGAN_ADAS_INFO,     0, 64},  // GW_31A: ADAS HUD
  };

  // RX 检查列表
  static RxCheck changan_rx_checks[] = {
    // GW_180: 转向角 (bus0, 8byte, 100Hz)
    {.msg = {{CHANGAN_STEER_ANGLE, 0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}, {0}}},

    // GW_196 (Petrol) / GW_1A6 (IDD): 踏板 (bus0, 8byte, 100Hz)
    {.msg = {{CHANGAN_PEDAL_DATA,     0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {CHANGAN_IDD_PEDAL_DATA, 0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}}},

    // GW_187 (Petrol,8byte) / GW_17A (IDD,64byte): 车速 (bus0, 100Hz)
    {.msg = {{CHANGAN_WHEEL_SPEEDS,     0, 8,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U},
             {CHANGAN_IDD_WHEEL_SPEEDS, 0, 64,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 100U}, {0}}},

    // GW_244: ACC 状态 (bus2, 32byte CANFD, 50Hz) — 用于同步 controls_allowed
    {.msg = {{CHANGAN_ACC_COMMAND, 2, 32,
              .ignore_checksum = true, .ignore_counter = true, .frequency = 50U}, {0}, {0}}},

    // GW_28C: 巡航按钮 (bus0, 8byte, 50Hz)
    // DBC 有 rolling counter，保持检查以防报文重放
    {.msg = {{CHANGAN_CRUISE_BUTTONS, 0, 8,
              .ignore_checksum = true, .ignore_counter = false, .frequency = 50U}, {0}, {0}}},
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
