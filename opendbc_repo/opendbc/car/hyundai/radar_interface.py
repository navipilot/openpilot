import math

from opendbc.can import CANParser
from opendbc.car import Bus, structs
from opendbc.car.interfaces import RadarInterfaceBase
from opendbc.car.hyundai.values import CAR, DBC
from openpilot.common.swaglog import cloudlog

RADAR_START_ADDR = 0x500
RADAR_MSG_COUNT = 32

# POC for parsing corner radars: https://github.com/commaai/openpilot/pull/24221/


def get_radar_can_parser(CP):
  if Bus.radar not in DBC[CP.carFingerprint]:
    return None

  messages = [(f"RADAR_TRACK_{addr:x}", 50) for addr in range(RADAR_START_ADDR, RADAR_START_ADDR + RADAR_MSG_COUNT)]
  return CANParser(DBC[CP.carFingerprint][Bus.radar], messages, 1)


class RadarInterface(RadarInterfaceBase):
  def __init__(self, CP):
    super().__init__(CP)
    self.updated_messages = set()
    self.trigger_msg = RADAR_START_ADDR + RADAR_MSG_COUNT - 1
    self.track_id = 0

    self.radar_off_can = CP.radarUnavailable
    # Probe whether radar tracks still exist on the Ioniq 6 while OP long is active,
    # without changing planner behavior yet.
    self.ioniq_6_radar_probe = CP.carFingerprint == CAR.HYUNDAI_IONIQ_6 and CP.openpilotLongitudinalControl and self.radar_off_can
    self.ioniq_6_radar_probe_logged = False
    self.ioniq_6_radar_probe_updates = 0
    self.rcp = get_radar_can_parser(CP)

  def update(self, can_strings):
    if self.ioniq_6_radar_probe and self.rcp is not None and not self.ioniq_6_radar_probe_logged:
      vls = self.rcp.update(can_strings)
      self.updated_messages.update(vls)
      self.ioniq_6_radar_probe_updates += 1

      if self.trigger_msg in self.updated_messages:
        rr = self._update(self.updated_messages)
        cloudlog.warning(f"Ioniq 6 radar probe: saw {len(rr.points)} radar tracks with radarUnavailable forced on")
        self.updated_messages.clear()
        self.ioniq_6_radar_probe_logged = True
      elif self.ioniq_6_radar_probe_updates >= 500:
        cloudlog.warning("Ioniq 6 radar probe: no radar track frames observed after startup")
        self.ioniq_6_radar_probe_logged = True
        self.updated_messages.clear()

    if self.radar_off_can or (self.rcp is None):
      return super().update(None)

    vls = self.rcp.update(can_strings)
    self.updated_messages.update(vls)

    if self.trigger_msg not in self.updated_messages:
      return None

    rr = self._update(self.updated_messages)
    self.updated_messages.clear()

    return rr

  def _update(self, updated_messages):
    ret = structs.RadarData()
    if self.rcp is None:
      return ret

    if not self.rcp.can_valid:
      ret.errors.canError = True

    for addr in range(RADAR_START_ADDR, RADAR_START_ADDR + RADAR_MSG_COUNT):
      msg = self.rcp.vl[f"RADAR_TRACK_{addr:x}"]

      if addr not in self.pts:
        self.pts[addr] = structs.RadarData.RadarPoint()
        self.pts[addr].trackId = self.track_id
        self.track_id += 1

      valid = msg['STATE'] in (3, 4)
      if valid:
        azimuth = math.radians(msg['AZIMUTH'])
        self.pts[addr].measured = True
        self.pts[addr].dRel = math.cos(azimuth) * msg['LONG_DIST']
        self.pts[addr].yRel = 0.5 * -math.sin(azimuth) * msg['LONG_DIST']
        self.pts[addr].vRel = msg['REL_SPEED']
        self.pts[addr].aRel = msg['REL_ACCEL']
        self.pts[addr].yvRel = float('nan')

      else:
        del self.pts[addr]

    ret.points = list(self.pts.values())
    return ret
