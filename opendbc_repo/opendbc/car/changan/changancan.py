"""
changancan.py — 从 Pyarmor 反编译字节码完整还原
逐函数按字节码逻辑 1:1 还原，未做任何额外改动
"""

# ── CRC 查找表（从字节码常量直接提取）────────────────────────────
crc8_tab = [
    0, 29, 58, 39, 116, 105, 78, 83, 232, 245, 210, 207, 156, 129, 166, 187,
    205, 208, 247, 234, 185, 164, 131, 158, 37, 56, 31, 2, 81, 76, 107, 118,
    135, 154, 189, 160, 243, 238, 201, 212, 111, 114, 85, 72, 27, 6, 33, 60,
    74, 87, 112, 109, 62, 35, 4, 25, 162, 191, 152, 133, 214, 203, 236, 241,
    19, 14, 41, 52, 103, 122, 93, 64, 251, 230, 193, 220, 143, 146, 181, 168,
    222, 195, 228, 249, 170, 183, 144, 141, 54, 43, 12, 17, 66, 95, 120, 101,
    148, 137, 174, 179, 224, 253, 218, 199, 124, 97, 70, 91, 8, 21, 50, 47,
    89, 68, 99, 126, 45, 48, 23, 10, 177, 172, 139, 150, 197, 216, 255, 226,
    38, 59, 28, 1, 82, 79, 104, 117, 206, 211, 244, 233, 186, 167, 128, 157,
    235, 246, 209, 204, 159, 130, 165, 184, 3, 30, 57, 36, 119, 106, 77, 80,
    161, 188, 155, 134, 213, 200, 239, 242, 73, 84, 115, 110, 61, 32, 7, 26,
    108, 113, 86, 75, 24, 5, 34, 63, 132, 153, 190, 163, 240, 237, 202, 215,
    53, 40, 15, 18, 65, 92, 123, 102, 221, 192, 231, 250, 169, 180, 147, 142,
    248, 229, 194, 223, 140, 145, 182, 171, 16, 13, 42, 55, 100, 121, 94, 67,
    178, 175, 136, 149, 198, 219, 252, 225, 90, 71, 96, 125, 46, 51, 20, 9,
    127, 98, 69, 88, 11, 22, 49, 44, 151, 138, 173, 176, 227, 254, 217, 196,
]


def crc_calculate_crc8(data: bytes) -> int:
    """CRC8，初始值 0xFF，最终异或 0xFF（字节码确认）"""
    crc = 0xFF
    for byte in data:
        crc = crc8_tab[crc ^ byte]
    return crc ^ 0xFF


def crc16_ccitt_false(data: bytes, poly: int = 4129, init_val: int = 65535) -> int:
    """CRC16-CCITT-FALSE（字节码确认 poly=4129, init=65535）"""
    crc = init_val
    for byte in data:
        crc ^= byte << 8
        for _ in range(8):
            if crc & 0x8000:
                crc = (crc << 1) ^ poly
            else:
                crc <<= 1
            crc &= 0xFFFF
    return crc


def create_244_command(packer, msg: dict, accel, counter, longActive, accTrq, vEgoRaw):
    """GW_244 — CHANGAN_Z6 (Petrol) 通用版本
    字节码确认：从 msg.copy() 出发，覆盖关键字段
    CRC: bytes[0:7] => ACC_CRCCheck_24E, bytes[8:15] => ACC_CRCCheck_25E
    """
    values = msg.copy()
    values.update({
        'ACC_ACCTargetAcceleration': accel,
        'ACC_CDDActive':             1 if (longActive and accel < 0) else 0,
        'ACC_RollingCounter_24E':    counter,
        'ACC_RollingCounter_25E':    counter,
        'ACC_ACCMode':               3 if longActive else 2,
        'ACC_AccTrqReq':             accTrq,
        'ACC_AccTrqReqActive':       1 if (longActive and accel >= 0) else 0,
    })
    dat = packer.make_can_msg('GW_244', 0, values)[1]
    values['ACC_CRCCheck_24E'] = crc_calculate_crc8(dat[:7])
    values['ACC_CRCCheck_25E'] = crc_calculate_crc8(dat[8:15])
    return packer.make_can_msg('GW_244', 0, values)


def create_244_command_idd(packer, msg: dict, accel, counter, longActive, accTrq, vEgoRaw):
    """GW_244 — CHANGAN_Z6_IDD 专用版本
    字节码确认额外字段：ACC_DecToStop, ACC_AWBActive, ACC_AEBCtrlType,
    ACC_TextInfoForDriver, ACC_Driveoff_Request, ACC_FCWPreWarning,
    ACC_FCWLatentWarning, ACC_LatTakeoverReq, ACC_LngTakeOverReq, ACC_HandsOnReq
    ACC_DecToStop: longActive and accel < 0 and vEgoRaw == 0 => 1 else 0
    """
    values = msg.copy()
    values.update({
        'ACC_ACCTargetAcceleration': accel,
        'ACC_CDDActive':             1 if (longActive and accel < 0) else 0,
        'ACC_RollingCounter_24E':    counter,
        'ACC_RollingCounter_25E':    counter,
        'ACC_ACCMode':               3 if longActive else 2,
        'ACC_AccTrqReq':             accTrq,
        'ACC_AccTrqReqActive':       1 if (longActive and accel >= 0) else 0,
        'ACC_DecToStop':             1 if (longActive and accel < 0 and vEgoRaw == 0) else 0,
        'ACC_AWBActive':             0,
        'ACC_AEBCtrlType':           0,
        'ACC_TextInfoForDriver':     0,
        'ACC_Driveoff_Request':      0,
        'ACC_FCWPreWarning':         0,
        'ACC_FCWLatentWarning':      0,
        'ACC_LatTakeoverReq':        0,
        'ACC_LngTakeOverReq':        0,
        'ACC_HandsOnReq':            0,
    })
    dat = packer.make_can_msg('GW_244', 0, values)[1]
    values['ACC_CRCCheck_24E'] = crc_calculate_crc8(dat[:7])
    values['ACC_CRCCheck_25E'] = crc_calculate_crc8(dat[8:15])
    return packer.make_can_msg('GW_244', 0, values)


def create_1BA_command(packer, msg: dict, angle, latCtrlActive, counter):
    """GW_1BA — 转向控制（bus0, 32byte）
    字节码确认：从 msg 中提取固定字段列表构建基础 dict，再 update 关键字段
    透传字段：ACC_CRCCheck_1BA, ACC_RollingCounter_1BA, EPS_LatCtrlActive,
              EPS_AngleCmd, ACC_MotorTorqueMinLimitRequest, ACC_MotorTorqueMaxLimitRequest
    CRC: bytes[0:7] => ACC_CRCCheck_1BA
    """
    keys = ('ACC_CRCCheck_1BA', 'ACC_RollingCounter_1BA', 'EPS_LatCtrlActive',
            'EPS_AngleCmd', 'ACC_MotorTorqueMinLimitRequest', 'ACC_MotorTorqueMaxLimitRequest')
    values = {k: msg[k] for k in keys if k in msg}
    values.update({
        'EPS_AngleCmd':           angle,
        'EPS_LatCtrlActive':      latCtrlActive,
        'ACC_RollingCounter_1BA': counter,
    })
    dat = packer.make_can_msg('GW_1BA', 0, values)[1]
    values['ACC_CRCCheck_1BA'] = crc_calculate_crc8(dat[:7])
    return packer.make_can_msg('GW_1BA', 0, values)


def create_17E_command(packer, msg: dict, longActive, counter):
    """GW_17E — EPS 状态心跳（bus0 计算 CRC，返回 bus2 消息）
    字节码确认：
      - 透传字段：EPS_CRCCheck_17E, EPS_RollingCounter_17E,
                  EPS_LatCtrlAvailabilityStatus, EPS_LatCtrlActive,
                  EPS_Handwheel_Relang_Valid, EPS_MeasuredTorsionBarTorqValid,
                  EPS_Handwheel_Relang, EPS_Pinionang, EPS_Pinionang_Valid,
                  EPS_ADS_Abortfeedback, EPS_MeasuredTorsionBarTorque
      - EPS_MeasuredTorsionBarTorque: longActive => msg[...]+1, else msg[...]
      - make_can_msg('GW_17E', 0, ...) 取 dat[0:7] 计算 CRC
      - 返回 make_can_msg('GW_17E', 2, ...)  ← bus2
    调用方需同时调用本函数（bus2）以及手动发一条 bus0 的 GW_17E
    """
    keys = ('EPS_CRCCheck_17E', 'EPS_RollingCounter_17E', 'EPS_LatCtrlAvailabilityStatus',
            'EPS_LatCtrlActive', 'EPS_Handwheel_Relang_Valid', 'EPS_MeasuredTorsionBarTorqValid',
            'EPS_Handwheel_Relang', 'EPS_Pinionang', 'EPS_Pinionang_Valid',
            'EPS_ADS_Abortfeedback', 'EPS_MeasuredTorsionBarTorque')
    values = {k: msg[k] for k in keys if k in msg}

    # EPS_MeasuredTorsionBarTorque：激活时 +1（逆向确认）
    torque_val = msg.get('EPS_MeasuredTorsionBarTorque', 0)
    values.update({
        'EPS_MeasuredTorsionBarTorque': torque_val + 1 if longActive else torque_val,
        'EPS_RollingCounter_17E':       counter,
    })
    # CRC 基于 bus0 的帧计算
    dat = packer.make_can_msg('GW_17E', 0, values)[1]
    values['EPS_CRCCheck_17E'] = crc_calculate_crc8(dat[:7])
    # 返回 bus2 消息（逆向确认最后 make_can_msg 用 bus=2）
    return packer.make_can_msg('GW_17E', 2, values)


def create_307_command(packer, msg: dict, counter, cruiseSpeed):
    """GW_307 — 巡航速度 HUD（bus0, 64byte）
    字节码确认透传字段（从 msg 中选取，含大量目标信息字段）：
      ACC_RLaneDistanceFus, ACC_LLaneDistanceFus, ACC_RRLaneDis, ACC_LLLaneDis,
      ACC_Target7~6 系列, ACC_CRCCheck_35F/35E/322/344/35F,
      ACC_RollingCounter_35F/35E/322/344,
      ACC_CSLAEnableStatus, ACC_IACCProhibitionTime, ACC_CSLSetReq,
      ACC_VehicleStartRemindSts, ACC_ACCTargetRelSpd, ACC_FRadarCalibrationStatus,
      ACC_AEBEnable, ACC_FCWSettingStatus, ACC_TimeGapSet, ACC_DistanceLevel,
      ACC_ObjValid, ACC_SetSpeed
    覆盖字段：ACC_SetSpeed, ACC_RollingCounter_35E/322/344/35F（均设为 counter）
    CRC：bytes[0:7]=>35E, [8:15]=>322, [16:23]=>344, [24:31]=>35F
    """
    keep_keys = (
        'ACC_RLaneDistanceFus', 'ACC_LLaneDistanceFus', 'ACC_RRLaneDis', 'ACC_LLLaneDis',
        'ACC_Target7ZoneID', 'ACC_Target7HeadingAngle', 'ACC_Target7LatRange',
        'ACC_Target7LngRange', 'ACC_Target7Direction', 'ACC_Target7Type',
        'ACC_Target7ID', 'ACC_Target7Detection',
        'ACC_Target6ZoneID', 'ACC_Target6HeadingAngle', 'ACC_Target6LatRange',
        'ACC_Target6LngRange', 'ACC_Target6Direction', 'ACC_Target6Type',
        'ACC_Target6ID', 'ACC_Target6Detection',
        'ACC_CRCCheck_35F', 'ACC_RollingCounter_35F',
        'ACC_CSLAEnableStatus', 'ACC_IACCProhibitionTime', 'ACC_CSLSetReq',
        'ACC_VehicleStartRemindSts',
        'ACC_CRCCheck_344', 'ACC_RollingCounter_344',
        'ACC_CRCCheck_322', 'ACC_RollingCounter_322',
        'ACC_ACCTargetRelSpd', 'ACC_FRadarCalibrationStatus',
        'ACC_CRCCheck_35E', 'ACC_RollingCounter_35E',
        'ACC_AEBEnable', 'ACC_FCWSettingStatus', 'ACC_TimeGapSet',
        'ACC_DistanceLevel', 'ACC_ObjValid', 'ACC_SetSpeed',
    )
    values = {k: msg[k] for k in keep_keys if k in msg}
    values.update({
        'ACC_SetSpeed':            cruiseSpeed,
        'ACC_RollingCounter_35E':  counter,
        'ACC_RollingCounter_322':  counter,
        'ACC_RollingCounter_344':  counter,
        'ACC_RollingCounter_35F':  counter,
    })
    dat = packer.make_can_msg('GW_307', 0, values)[1]
    values['ACC_CRCCheck_35E'] = crc_calculate_crc8(dat[:7])
    values['ACC_CRCCheck_322'] = crc_calculate_crc8(dat[8:15])
    values['ACC_CRCCheck_344'] = crc_calculate_crc8(dat[16:23])
    values['ACC_CRCCheck_35F'] = crc_calculate_crc8(dat[24:31])
    return packer.make_can_msg('GW_307', 0, values)


def create_31A_command(packer, msg: dict, counter, longActive, steeringPressed):
    """GW_31A — ADAS 状态 HUD（bus0, 64byte）
    字节码确认：
      透传字段（从 msg 选取）：
        ACC_CRCCheck_367, ACC_RollingCounter_367,
        ACC_LatPathHeadingAngle, ACC_LatPathDY,
        ACC_ELKEnableStatus, ACC_ELKInterventionMode, ACC_ELKMode,
        ACC_CRCCheck_30D, ACC_RollingCounter_30D, ACC_HighBeamControl,
        ACC_RRLaneDetection, ACC_LLLaneDetection,
        ACC_TargetBasedLateralControl, ACC_DriverHandsOffStatus,
        ACC_IACCHWATextInfoForDriver, ACC_IACCHWAMode,
        ACC_CRCCheck_30A, ACC_RollingCounter_30A,
        ACC_LaneChangeStatus, ACC_RoadCurvatureFar, ACC_RoadCurvatureNear,
        ACC_RoadCurvature, ACC_LLaneMarkerType, ACC_HostLaneLeftStatus,
        ACC_HostLaneRightStatus, ACC_IACCHWAEnable, ACC_RLaneMarkerType,
        ACC_CRCCheck_36D, ACC_RollingCounter_36D,
        ACC_FRadarFailureStatus, ACC_Voiceinfo, ACC_AEBTargetmode,
        ACC_AEBTextInfo, ACC_AEBStatus, ACC_ELKAlert,
        ACC_AEBTargetLatRange, ACC_AEBTargetRelSpeed, ACC_AEBTargetLngRange

    ACC_DriverHandsOffStatus 逻辑（字节码确认）：
      longActive=True, steeringPressed=True  => 4
      longActive=True, steeringPressed=False => 3
      longActive=False                       => 1 (default)

    ACC_IACCHWAMode（字节码确认）：
      longActive=True, steeringPressed=False => 2
      otherwise                              => 0

    覆盖字段：
      ACC_IACCHWAMode, ACC_TargetBasedLateralControl=0, ACC_AEBTextInfo=0,
      ACC_IACCHWATextInfoForDriver=0, ACC_ELKAlert=0,
      ACC_RollingCounter_36D/30A/30D/367 = counter

    CRC：bytes[0:7]=>36D, [8:15]=>30A, [16:23]=>30D, [24:31]=>367
    """
    keep_keys = (
        'ACC_CRCCheck_367', 'ACC_RollingCounter_367',
        'ACC_LatPathHeadingAngle', 'ACC_LatPathDY',
        'ACC_ELKEnableStatus', 'ACC_ELKInterventionMode', 'ACC_ELKMode',
        'ACC_CRCCheck_30D', 'ACC_RollingCounter_30D', 'ACC_HighBeamControl',
        'ACC_RRLaneDetection', 'ACC_LLLaneDetection',
        'ACC_TargetBasedLateralControl', 'ACC_DriverHandsOffStatus',
        'ACC_IACCHWATextInfoForDriver', 'ACC_IACCHWAMode',
        'ACC_CRCCheck_30A', 'ACC_RollingCounter_30A',
        'ACC_LaneChangeStatus', 'ACC_RoadCurvatureFar', 'ACC_RoadCurvatureNear',
        'ACC_RoadCurvature', 'ACC_LLaneMarkerType', 'ACC_HostLaneLeftStatus',
        'ACC_HostLaneRightStatus', 'ACC_IACCHWAEnable', 'ACC_RLaneMarkerType',
        'ACC_CRCCheck_36D', 'ACC_RollingCounter_36D',
        'ACC_FRadarFailureStatus', 'ACC_Voiceinfo', 'ACC_AEBTargetmode',
        'ACC_AEBTextInfo', 'ACC_AEBStatus', 'ACC_ELKAlert',
        'ACC_AEBTargetLatRange', 'ACC_AEBTargetRelSpeed', 'ACC_AEBTargetLngRange',
    )
    values = {k: msg[k] for k in keep_keys if k in msg}

    # ACC_DriverHandsOffStatus
    if longActive:
        hands_off = 4 if steeringPressed else 3
    else:
        hands_off = 1

    # ACC_IACCHWAMode
    iacc_mode = 2 if (longActive and not steeringPressed) else 0

    values.update({
        'ACC_IACCHWAMode':               iacc_mode,
        'ACC_TargetBasedLateralControl': 0,
        'ACC_AEBTextInfo':               0,
        'ACC_IACCHWATextInfoForDriver':  0,
        'ACC_ELKAlert':                  0,
        'ACC_DriverHandsOffStatus':      hands_off,
        'ACC_RollingCounter_36D':        counter,
        'ACC_RollingCounter_30A':        counter,
        'ACC_RollingCounter_30D':        counter,
        'ACC_RollingCounter_367':        counter,
    })
    dat = packer.make_can_msg('GW_31A', 0, values)[1]
    values['ACC_CRCCheck_36D'] = crc_calculate_crc8(dat[:7])
    values['ACC_CRCCheck_30A'] = crc_calculate_crc8(dat[8:15])
    values['ACC_CRCCheck_30D'] = crc_calculate_crc8(dat[16:23])
    values['ACC_CRCCheck_367'] = crc_calculate_crc8(dat[24:31])
    return packer.make_can_msg('GW_31A', 0, values)
