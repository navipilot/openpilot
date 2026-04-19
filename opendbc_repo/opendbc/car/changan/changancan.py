from opendbc.can.packer import CANPacker
from typing import List

_CRC8_TABLE: List[int] = [
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
  0x7F, 0x62, 0x45, 0x58, 0x0B, 0x16, 0x31, 0x2C, 0x97, 0x8A, 0xAD, 0xB0, 0xE3, 0xFE, 0xD9, 0xC4,
]


def _crc8(data: bytes) -> int:
  crc = 0
  for byte in data:
    crc = _CRC8_TABLE[crc ^ byte]
  return crc


def _crc_and_pack(packer: CANPacker, bus: int, name: str,
                  values: dict, crc_key: str, crc_byte_slice: slice) -> tuple:
  dat = packer.make_can_msg(name, bus, values)[1]
  values[crc_key] = _crc8(bytes(dat[crc_byte_slice]))
  return packer.make_can_msg(name, bus, values)


# ── GW_1BA: Steering angle command (32-byte CAN-FD, bus 0) ──────────────────

def create_1BA_command(packer: CANPacker, sigs: dict,
                       angle: float, lat_active: bool, counter: int):
  """GW_1BA (0x1BA): EPS steering angle command.
  Preserves all original signals from rx (MotorTorque limits, etc).
  CRC-8 covers bytes[0:7].
  """
  _1BA_PRESERVE = (
    'ACC_CRCCheck_1BA', 'ACC_RollingCounter_1BA', 'EPS_LatCtrlActive',
    'EPS_AngleCmd', 'ACC_MotorTorqueMinLimitRequest', 'ACC_MotorTorqueMaxLimitRequest',
  )
  values = {k: sigs[k] for k in _1BA_PRESERVE if k in sigs}
  values.update({
    'EPS_AngleCmd':           angle,
    'EPS_LatCtrlActive':      1 if lat_active else 0,
    'ACC_RollingCounter_1BA': counter,
  })
  return _crc_and_pack(packer, 0, 'GW_1BA', values, 'ACC_CRCCheck_1BA', slice(0, 7))


# ── GW_17E: EPS heartbeat / reverse confirmation (8-byte, bus 0+2) ────────

def create_17E_command(packer: CANPacker, sigs: dict,
                       lat_active: bool, counter: int):
  """GW_17E (0x17E): EPS heartbeat — sent on BOTH bus 0 and bus 2.
  Preserves all 11 original signals from rx.
  CRC-8 covers bytes[0:7].
  Returns the bus-2 message tuple.
  """
  _17E_PRESERVE = (
    'EPS_CRCCheck_17E', 'EPS_RollingCounter_17E',
    'EPS_LatCtrlAvailabilityStatus', 'EPS_LatCtrlActive',
    'EPS_Handwheel_Relang_Valid', 'EPS_MeasuredTorsionBarTorqValid',
    'EPS_Handwheel_Relang', 'EPS_Pinionang', 'EPS_Pinionang_Valid',
    'EPS_ADS_Abortfeedback', 'EPS_MeasuredTorsionBarTorque',
  )
  values = {k: sigs[k] for k in _17E_PRESERVE if k in sigs}
  values.update({
    'EPS_LatCtrlAvailabilityStatus': 1 if lat_active else 0,
    'EPS_RollingCounter_17E':       counter,
  })
  if lat_active and 'EPS_MeasuredTorsionBarTorque' in sigs:
    values['EPS_MeasuredTorsionBarTorque'] = sigs['EPS_MeasuredTorsionBarTorque'] + 1
  msg0 = _crc_and_pack(packer, 0, 'GW_17E', values, 'EPS_CRCCheck_17E', slice(0, 7))
  msg2 = _crc_and_pack(packer, 2, 'GW_17E', values, 'EPS_CRCCheck_17E', slice(0, 7))
  return [msg0, msg2]


# ── GW_307: ACC HUD speed display (64-byte CAN-FD, bus 0) ──────────────────

def create_307_command(packer: CANPacker, sigs: dict, counter: int, cruise_speed: float):
  """GW_307 (0x307): HUD cruise set-speed + radar objects.
  Preserves all 42 original signals from rx (Target6/7, RollingCounters, etc).
  4 CRC-8 checksums over 4 byte slices.
  """
  _307_PRESERVE = (
    'ACC_RLaneDistanceFus', 'ACC_LLaneDistanceFus', 'ACC_RRLaneDis', 'ACC_LLLaneDis',
    'ACC_Target7ZoneID', 'ACC_Target7HeadingAngle', 'ACC_Target7LatRange', 'ACC_Target7LngRange',
    'ACC_Target7Direction', 'ACC_Target7Type', 'ACC_Target7ID', 'ACC_Target7Detection',
    'ACC_Target6ZoneID', 'ACC_Target6HeadingAngle', 'ACC_Target6LatRange', 'ACC_Target6LngRange',
    'ACC_Target6Direction', 'ACC_Target6Type', 'ACC_Target6ID', 'ACC_Target6Detection',
    'ACC_CRCCheck_35F', 'ACC_RollingCounter_35F',
    'ACC_CSLAEnableStatus', 'ACC_IACCProhibitionTime', 'ACC_CSLSetReq', 'ACC_VehicleStartRemindSts',
    'ACC_CRCCheck_344', 'ACC_RollingCounter_344',
    'ACC_CRCCheck_322', 'ACC_RollingCounter_322',
    'ACC_ACCTargetRelSpd', 'ACC_FRadarCalibrationStatus',
    'ACC_CRCCheck_35E', 'ACC_RollingCounter_35E',
    'ACC_AEBEnable', 'ACC_FCWSettingStatus', 'ACC_TimeGapSet', 'ACC_DistanceLevel',
    'ACC_ObjValid', 'ACC_SetSpeed',
  )
  values = {k: sigs[k] for k in _307_PRESERVE if k in sigs}
  values.update({
    'ACC_SetSpeed':             cruise_speed,
    'ACC_RollingCounter_35E':   counter,
    'ACC_RollingCounter_322':   counter,
    'ACC_RollingCounter_344':   counter,
    'ACC_RollingCounter_35F':   counter,
  })
  dat = packer.make_can_msg('GW_307', 0, values)[1]
  values['ACC_CRCCheck_35E'] = _crc8(bytes(dat[0:7]))
  values['ACC_CRCCheck_322'] = _crc8(bytes(dat[7:15]))
  values['ACC_CRCCheck_344'] = _crc8(bytes(dat[15:23]))
  values['ACC_CRCCheck_35F'] = _crc8(bytes(dat[23:31]))
  return packer.make_can_msg('GW_307', 0, values)


# ── GW_31A: ADAS HUD status (64-byte CAN-FD, bus 0) ──────────────────────

def create_31A_command(packer: CANPacker, sigs: dict, counter: int,
                       lat_active: bool, steering_pressed: bool):
  """GW_31A (0x31A): ADAS HUD status + lane/radar info.
  Preserves all 40 original signals from rx.
  DriverHandsOffStatus logic:
    longActive + pressed  -> 4 (hands-on, active)
    longActive + !pressed -> 2 (hands-free, active)
    !longActive + pressed -> 3 (hands-on, inactive)
    !longActive + !pressed-> 0 (hands-free, inactive)
  4 CRC-8 checksums over 4 byte slices.
  """
  _31A_PRESERVE = (
    'ACC_CRCCheck_367', 'ACC_RollingCounter_367',
    'ACC_LatPathHeadingAngle', 'ACC_LatPathDY',
    'ACC_ELKEnableStatus', 'ACC_ELKInterventionMode', 'ACC_ELKMode',
    'ACC_CRCCheck_30D', 'ACC_RollingCounter_30D',
    'ACC_HighBeamControl', 'ACC_RRLaneDetection', 'ACC_LLLaneDetection',
    'ACC_TargetBasedLateralControl', 'ACC_DriverHandsOffStatus',
    'ACC_IACCHWATextInfoForDriver', 'ACC_IACCHWAMode',
    'ACC_CRCCheck_30A', 'ACC_RollingCounter_30A',
    'ACC_LaneChangeStatus',
    'ACC_RoadCurvatureFar', 'ACC_RoadCurvatureNear', 'ACC_RoadCurvature',
    'ACC_LLaneMarkerType', 'ACC_HostLaneLeftStatus', 'ACC_HostLaneRightStatus',
    'ACC_IACCHWAEnable', 'ACC_RLaneMarkerType',
    'ACC_CRCCheck_36D', 'ACC_RollingCounter_36D',
    'ACC_FRadarFailureStatus', 'ACC_Voiceinfo',
    'ACC_AEBTargetmode', 'ACC_AEBTextInfo', 'ACC_AEBStatus', 'ACC_ELKAlert',
    'ACC_AEBTargetLatRange', 'ACC_AEBTargetRelSpeed', 'ACC_AEBTargetLngRange',
  )
  values = {k: sigs[k] for k in _31A_PRESERVE if k in sigs}

  if lat_active:
    driver_hands_off = 4 if steering_pressed else 2
  else:
    driver_hands_off = 3 if steering_pressed else 0

  values.update({
    'ACC_IACCHWAMode':        3 if lat_active else 0,
    'ACC_IACCHWAEnable':      1 if lat_active else 0,
    'ACC_TargetBasedLateralControl': 1 if lat_active else 0,
    'ACC_DriverHandsOffStatus': driver_hands_off,
    'ACC_RollingCounter_36D': counter,
    'ACC_RollingCounter_30A': counter,
    'ACC_RollingCounter_30D': counter,
    'ACC_RollingCounter_367': counter,
  })
  dat = packer.make_can_msg('GW_31A', 0, values)[1]
  values['ACC_CRCCheck_36D'] = _crc8(bytes(dat[0:7]))
  values['ACC_CRCCheck_30A'] = _crc8(bytes(dat[7:15]))
  values['ACC_CRCCheck_30D'] = _crc8(bytes(dat[15:23]))
  values['ACC_CRCCheck_367'] = _crc8(bytes(dat[23:31]))
  return packer.make_can_msg('GW_31A', 0, values)


# ── Public API aliases (matches carcontroller.py expectations) ─────────────────

create_steering_control = create_1BA_command
create_1BA_command = create_1BA_command
create_307_command = create_307_command
create_31A_command = create_31A_command
create_eps_control = create_17E_command


def create_17E_commands(packer: CANPacker, sigs: dict,
                        lat_active: bool, counter: int) -> List[tuple]:
  """Wrapper: forwards to create_17E_command which returns [bus0, bus2]."""
  return create_17E_command(packer, sigs, lat_active, counter)


# ── GW_244: Longitudinal ACC control (8-byte, bus 0) ──────────────────────

def create_244_command_idd(packer: CANPacker, sigs: dict,
                           accel: float, counter: int,
                           long_active: bool, acc_trq: float,
                           v_ego_raw: float):
  """GW_244 (0x244): Longitudinal ACC command for Z6_IDD.
  ACC_ACCMode: 3=active, 2=standby
  ACC_CDDActive: 1 when braking (accel < 0 and longActive)
  ACC_AccTrqReqActive: 1 when accelerating (accel >= 0 and longActive)
  ACC_DecToStop: 1 when longActive, braking, and vEgoRaw == 0
  CRC-8 covers bytes[0:7] and bytes[8:15].
  """
  values = {k: sigs[k] for k in sigs}
  values.update({
    'ACC_ACCTargetAcceleration': accel,
    'ACC_CDDActive':            1 if (long_active and accel < 0) else 0,
    'ACC_RollingCounter_24E':  counter,
    'ACC_RollingCounter_25E':   counter,
    'ACC_ACCMode':              3 if long_active else 2,
    'ACC_AccTrqReq':            acc_trq,
    'ACC_AccTrqReqActive':      1 if (long_active and accel >= 0) else 0,
    'ACC_DecToStop':            1 if (long_active and accel < 0 and v_ego_raw == 0) else 0,
    'ACC_AWBActive':            0,
    'ACC_AEBCtrlType':          0,
    'ACC_TextInfoForDriver':    0,
    'ACC_Driveoff_Request':      0,
    'ACC_FCWPreWarning':        0,
    'ACC_FCWLatentWarning':     0,
    'ACC_LatTakeoverReq':       0,
    'ACC_LngTakeOverReq':       0,
    'ACC_HandsOnReq':           0,
  })
  dat = packer.make_can_msg('GW_244', 0, values)[1]
  values['ACC_CRCCheck_24E'] = _crc8(bytes(dat[0:7]))
  values['ACC_CRCCheck_25E'] = _crc8(bytes(dat[8:15]))
  return packer.make_can_msg('GW_244', 0, values)
