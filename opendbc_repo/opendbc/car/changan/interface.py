#!/usr/bin/env python3
from opendbc.car.changan.values import DBC, CarControllerParams, EPS_SCALE
from opendbc.car import structs, get_safety_config
from opendbc.car.interfaces import CarInterfaceBase
from opendbc.car.changan.carcontroller import CarController
from opendbc.car.changan.carstate import CarState
from opendbc.car.changan.radar_interface import RadarInterface
from openpilot.system.hardware import HARDWARE
from opendbc.car.changan.values import CAR, ChanganSafetyFlags
from openpilot.common.params import Params
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.asymmetric import padding
import base64

# ── 以下为原版授权验证相关 import，已注释 ─────────────────────────
# from openpilot.system.hardware import HARDWARE
# from opendbc.car.changan.values import ChanganSafetyFlags
# from openpilot.common.params import Params
# from cryptography.hazmat.primitives import serialization, hashes
# from cryptography.hazmat.primitives.asymmetric import padding
# import base64
# ─────────────────────────────────────────────────────────────────

# ── 以下为原版内置授权私钥，已注释 ───────────────────────────────
# _AUTH_PRIVATE_KEY = (
#     b'-----BEGIN PRIVATE KEY-----\n'
#     b'MIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDeEZ6kt97ddbft\n'
#     b'1Bl4WL2o2lWExBy8KImpwxEc8Di7oFiOJjicbLWwjzkq6Nrcz3xWlMHkVG0RYAyx\n'
#     b'yUOW+Sn8qgxf9jfmbcb+vn6lvAOVnMuyc/Zxn0dzXBVnmDXdKzVFlXjK3rdf4Mer\n'
#     b'EuYTSY00602csRatl5il/jwcHZWSrjEI+IPqX2xZsj8d2fAnqUL12b43Hi4X0mSc\n'
#     b'j7zQuAE8qyfn/ev+H5t1CykBRKm/2BQabogI6k2d8arkZLWVQH3tdDBQAONQznYc\n'
#     b'e6nwfkIMxO8AaBbI4ouFkpZyOj8EPVnCvaeGJ+mERYbsHyKsj7XJwffD5yNs21pA\n'
#     b'F21Sd7GNAgMBAAECggEAJ3HnxQhoDNxRHGyVTZwtrCWEFT4kgIOO2Fmu234MBb/Y\n'
#     b'mv9NBPQGXeNC201bOuecBWENA7q/WNu/xYCjgdimTZwC8LSyfzq2ox9BLb00wcmF\n'
#     b'ISCaANqy/Vm475rIzo1TPBXty9M9rUknDksgFQL2kxTMx21pQtJvqeR71t6mZ9Q2\n'
#     b'PrYnFznp4mPx4WqikSzWcGlD9I5a4MhTxCcXjj758AS7+iqJjvKl6Ktb9zx8uhjo\n'
#     b'XDptmf24H/JGcaDm0rfnkFBu8wE++Au2zaDuRH5i75HHivvkrmYStm+5sc6kaSUE\n'
#     b'Ef5aQOPAdIWOD4t8Fa4gAl3bXCbsVaE0OMH9hZVsIQKBgQD+OKprQ57RBGmTTmx1\n'
#     b'ymeymWHBQ8/NrxzdtKiyJm/n1ER6qFldKopehw1b1js5ps6YQvGpUH1kHBbRi8iO\n'
#     b'MyrS0+7b2/xByWgIo6jpRVfx5kKUA7vnH5r2Q5YjfWfo18QTsoda9EHBbbdd0DR4\n'
#     b'MedlkijWJtfKd/SkqW20dyMI5QKBgQDfn12mALJJvSyatxJvQ0wsE2BW5gRBryoc\n'
#     b'yZ+qHb71Ndm2h/XuektDO/HHvsOYMmxHw3VaTUyPe4M6Mpn9H9S+eBruNM+cVSKW\n'
#     b'haZmCIg1aPkVaXQjS7ZZt12gHVgVINpcOvQqgArDsSzsrMhvCH8Qiq8bR6PHE5xy\n'
#     b'vGrSLZRDiQKBgQCgEav0aBpqv11+fAW80RokEYHEv97rQpRpYKRLr/oMPdJ0bOX/\n'
#     b'zJZdWKnnpmeaMFaq9pPVKTIaw7j2bg50wvD22KzwUH5wKdHQhsQHRTtPu802B3fb\n'
#     b'2kGnR/0UEZVIvfre8C1b7qoskdnAPyWafMmXYcVGuiRrfsZSm7/QvqcXpQKBgQCL\n'
#     b'XpqSFEsJtBIxlrmxiGLX8XeQkSjD+zSm3m5ZKtLRpq86mMAI0kVH8Ps+P0FOANcF\n'
#     b'Xucr0ZNOtW7S/aiFXqpLzzURku51g4XcBA3x941UpEjhNGkCenR6s9JxMPitql0N\n'
#     b'MFzuhACxeCIEm6cLtGLk31pFCUAkma9evtlghrGp6QKBgQCG1YGLaYBT513AxjMj\n'
#     b'vMMnTIz/PEooRCorUBRN3BUvPkXFgmRES99pqsqRKtOZ2xz9R8BdjThsnTaD4kl2\n'
#     b'BsbcQ1+ciY0XnnW0SHXPL2VabRwhNaWlrBNv5SayyQZnXWfUoI0arguoBNBe+S3A\n'
#     b'79aO4TwD9QRJXIHC9wEpUiis1g==\n'
#     b'-----END PRIVATE KEY-----'
# )
# ─────────────────────────────────────────────────────────────────


class CarInterface(CarInterfaceBase):
  CarState = CarState
  CarController = CarController
  RadarInterface = RadarInterface

  @staticmethod
  def get_pid_accel_limits(CP, current_speed, cruise_speed):
    return CarControllerParams.ACCEL_MIN, CarControllerParams.ACCEL_MAX

  @staticmethod
  def _get_params(ret: structs.CarParams, candidate, fingerprint, car_fw,
                  experimental_longh, is_release, docs) -> structs.CarParams:

    ret.brand = 'changan'
    ret.safetyConfigs = [get_safety_config(structs.CarParams.SafetyModel.changan)]

    # 车型 safetyParam 标志（对齐 values.py 中实际定义的 ChanganFlags）
    # 原版逆向含 ChanganSafetyFlags.QIYUAN_A05_FLAG / CHANGAN_Z6_IDD_FLAG / CHANGAN_Z6_FLAG，
    # 当前 values.py 只定义了 ChanganFlags.CHANGAN_Z6_IDD，其余车型不设额外 flag：
    if candidate == CAR.CHANGAN_Z6:
      ret.safetyConfigs[0].safetyParam |= ChanganSafetyFlags.CHANGAN_Z6_FLAG.value
    elif candidate == CAR.CHANGAN_Z6_IDD:
      ret.safetyConfigs[0].safetyParam |= ChanganSafetyFlags.CHANGAN_Z6_IDD_FLAG.value
    # if candidate == CAR.QIYUAN_A05:
    #   ret.safetyConfigs[0].safetyParam |= ChanganSafetyFlags.QIYUAN_A05_FLAG.value
    # elif candidate == CAR.CHANGAN_Z6_IDD:
    #   ret.safetyConfigs[0].safetyParam |= ChanganSafetyFlags.CHANGAN_Z6_IDD_FLAG.value
    # else:
    #   ret.safetyConfigs[0].safetyParam |= ChanganSafetyFlags.CHANGAN_Z6_FLAG.value

    # 横向控制参数（逆向字节码确认）
    ret.steerActuatorDelay = 0.12
    ret.steerLimitTimer = 1
    ret.steerControlType = structs.CarParams.SteerControlType.angle
    ret.centerToFront = ret.wheelbase * 0.44
    ret.enableBsm = True
    ret.radarUnavailable = True
    ret.minSteerSpeed = 0

    # 纵向控制参数（逆向字节码确认）
    ret.openpilotLongitudinalControl = True
    ret.autoResumeSng = ret.openpilotLongitudinalControl
    ret.minEnableSpeed = -1
    ret.vEgoStopping = 0.25
    ret.vEgoStarting = 0.25
    ret.stoppingDecelRate = 0.3
    ret.startingState = True
    ret.startAccel = 0.5
    ret.stopAccel = -0.5
    ret.longitudinalActuatorDelay = 0.3

    # ── 以下为原版授权验证逻辑，已注释 ───────────────────────────
    # 原版逻辑：
    #   1. params = Params(); auth_key_raw = params.get('OpAuthKey')
    #   2. 若 auth_key_raw is None => ret.dashcamOnly = True; return ret
    #   3. 否则 base64 解码 => RSA-OAEP 用 _AUTH_PRIVATE_KEY 解密
    #   4. 与 HARDWARE.get_serial() 比对，不匹配 => dashcamOnly=True，匹配 => False
    #   5. 任何 Exception => dashcamOnly=True
    #   6. candidate == CAR.QIYUAN_A07 => 强制 dashcamOnly=False
    #
    # params = Params()
    # auth_key_raw = params.get('OpAuthKey')
    # if auth_key_raw is None:
    #   ret.dashcamOnly = True
    #   return ret
    # try:
    #   auth_key_b64 = auth_key_raw.replace('\n', '').strip()
    #   auth_key_bytes = base64.b64decode(auth_key_b64)
    #   private_key = serialization.load_pem_private_key(_AUTH_PRIVATE_KEY, password=None)
    #   decrypted = private_key.decrypt(
    #     auth_key_bytes,
    #     padding.OAEP(
    #       mgf=padding.MGF1(algorithm=hashes.SHA1()),
    #       algorithm=hashes.SHA1(),
    #       label=None,
    #     )
    #   )
    #   serial = HARDWARE.get_serial()
    #   if serial != decrypted.decode():
    #     ret.dashcamOnly = True
    #   else:
    #     ret.dashcamOnly = False
    # except Exception:
    #   ret.dashcamOnly = True
    # if candidate == CAR.QIYUAN_A07:
    #   ret.dashcamOnly = False
    # ─────────────────────────────────────────────────────────────

    return ret
