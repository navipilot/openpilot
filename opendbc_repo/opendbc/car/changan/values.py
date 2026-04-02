from collections import defaultdict
from dataclasses import dataclass, field
from enum import IntFlag

from opendbc.car import Bus, DbcDict, PlatformConfig, Platforms, CarSpecs, AngleSteeringLimits, structs
from opendbc.car.common.conversions import Conversions as CV
from opendbc.car.docs_definitions import CarDocs, CarParts, CarHarness

Ecu = structs.CarParams.Ecu

MIN_ACC_SPEED = 19. * CV.MPH_TO_MS
PEDAL_TRANSITION = 10. * CV.MPH_TO_MS


class ChanganFlags(IntFlag):
  CHANGAN_Z6 = 1
  CHANGAN_Z6_IDD = 2
  QIYUAN_A05 = 4


class CarControllerParams:
  ACCEL_MAX = 2.0
  ACCEL_MIN = -3.5
  STEER_STEP = 1
  STEER_MAX = 900  # Fixed from DAS reference
  STEER_ERROR_MAX = 650
  MAX_STEERING_ANGLE = 180.0  # Fixed from DAS reference
  STEERING_SMOOTHING_FACTOR = 0.3
  ANGLE_LIMITS: AngleSteeringLimits = AngleSteeringLimits(
    180,  # Max angle: 180 degrees (from DAS reference)
    ([10, 20], [1.4, 0.15]),  # Angle rate up (from DAS reference)
    ([10, 20], [1.4, 0.26]),  # Angle rate down (from DAS reference)
  )

  def __init__(self, CP):
    if CP.lateralTuning.which == "torque":
      self.STEER_DELTA_UP = 15
      self.STEER_DELTA_DOWN = 25
    else:
      self.STEER_DELTA_UP = 10
      self.STEER_DELTA_DOWN = 25


@dataclass
class ChangAnCarDocs(CarDocs):
  package: str = "All"
  car_parts: CarParts = field(default_factory=CarParts.common([CarHarness.custom]))


class CAR(Platforms):
  CHANGAN_Z6 = PlatformConfig(
    [ChangAnCarDocs("Changan Z6")],
    CarSpecs(mass=2205, wheelbase=2.80, steerRatio=15.0, centerToFrontRatio=0.44, tireStiffnessFactor=0.444),
    DbcDict({Bus.pt: "changan", Bus.cam: "changan"}),
  )
  CHANGAN_Z6_IDD = PlatformConfig(
    [ChangAnCarDocs("Changan Z6 iDD")],
    CHANGAN_Z6.specs,
    CHANGAN_Z6.dbc_dict,
    flags=ChanganFlags.CHANGAN_Z6_IDD,
  )
  QIYUAN_A05 = PlatformConfig(
    [ChangAnCarDocs("Qiyuan A05")],
    CarSpecs(mass=1965, wheelbase=2.76, steerRatio=13.9, centerToFrontRatio=0.44, tireStiffnessFactor=0.444),
    DbcDict({Bus.pt: "changan_can", Bus.cam: "changan_can"}),
    flags=ChanganFlags.QIYUAN_A05,
  )
  QIYUAN_A07 = PlatformConfig(
    [ChangAnCarDocs("Qiyuan A07")],
    CarSpecs(mass=2190, wheelbase=2.76, steerRatio=13.9, centerToFrontRatio=0.44, tireStiffnessFactor=0.444),
    DbcDict({Bus.pt: "changan_can", Bus.cam: "changan_can"}),
  )


STEER_THRESHOLD = 15  # Matches reference
EPS_SCALE = defaultdict(lambda: 73)
DBC = CAR.create_dbc_map()


if __name__ == "__main__":
  cars = []
  for platform in CAR:
    for doc in platform.config.car_docs:
      cars.append(doc.name)
  cars.sort()
  for c in cars:
    print(c)
