from opendbc.can import CANParser
from opendbc.car import Bus, structs
from opendbc.car.carlog import carlog
from opendbc.car.interfaces import RadarInterfaceBase
from opendbc.car.tesla.values import DBC

RADAR_START_ADDR = 0x410
RADAR_MSG_COUNT = 80  # 40 points * 2 messages each


def get_radar_can_parser(CP):
  if Bus.radar not in DBC[CP.carFingerprint]:
    return None

  messages = [('RadarStatus', 16)]
  for i in range(RADAR_MSG_COUNT // 2):
    messages.extend([
      (f'RadarPoint{i}_A', 16),
      (f'RadarPoint{i}_B', 16),
    ])

  return CANParser(DBC[CP.carFingerprint][Bus.radar], messages, 1)


class RadarInterface(RadarInterfaceBase):
  def __init__(self, CP):
    super().__init__(CP)
    self.updated_messages = set()
    self.trigger_msg = RADAR_START_ADDR + RADAR_MSG_COUNT - 1
    self.track_id = 0

    self.radar_off_can = CP.radarUnavailable
    self.rcp = get_radar_can_parser(CP)
    self.runtime_radar_seen = False
    self.missing_trigger_count = 0
    self.logged_missing_trigger = False

  def update(self, can_strings):
    if self.rcp is None:
      return super().update(None)

    runtime_radar_seen = any(can.src == 1 and can.address == RADAR_START_ADDR for packet in can_strings for can in packet)
    if runtime_radar_seen and self.radar_off_can:
      self.radar_off_can = False
      carlog.warning("Tesla radar detected at runtime on bus 1 despite CP.radarUnavailable=True")

    if self.radar_off_can:
      return super().update(None)

    vls = self.rcp.update(can_strings)
    self.updated_messages.update(vls)

    if self.trigger_msg not in self.updated_messages:
      if any(msg >= RADAR_START_ADDR and msg < self.trigger_msg for msg in self.updated_messages):
        self.missing_trigger_count += 1
        if self.missing_trigger_count > 50 and not self.logged_missing_trigger:
          max_msg = max(self.updated_messages)
          carlog.warning(f"Tesla radar waiting for trigger 0x{self.trigger_msg:X}, highest seen 0x{max_msg:X}")
          self.logged_missing_trigger = True
      return None

    self.missing_trigger_count = 0
    self.logged_missing_trigger = False
    rr = self._update(self.updated_messages)
    self.updated_messages.clear()

    return rr

  def _update(self, updated_messages):
    ret = structs.RadarData()
    if self.rcp is None:
      return ret

    if not self.rcp.can_valid:
      ret.errors.canError = True

    radar_status = self.rcp.vl['RadarStatus']
    if radar_status['shortTermUnavailable']:
      ret.errors.radarUnavailableTemporary = True
    if radar_status['sensorBlocked'] or radar_status['vehDynamicsError']:
      ret.errors.radarFault = True

    for i in range(RADAR_MSG_COUNT // 2):
      msg_a = self.rcp.vl[f'RadarPoint{i}_A']
      msg_b = self.rcp.vl[f'RadarPoint{i}_B']

      # Make sure msg A and B are together
      if msg_a['Index'] != msg_b['Index2']:
        continue

      if not msg_a['Tracked']:
        if i in self.pts:
          del self.pts[i]
        continue

      if i not in self.pts:
        self.pts[i] = structs.RadarData.RadarPoint()
        self.pts[i].trackId = self.track_id
        self.track_id += 1

      self.pts[i].dRel = msg_a['LongDist']
      self.pts[i].yRel = msg_a['LatDist']
      self.pts[i].vRel = msg_a['LongSpeed']
      self.pts[i].aRel = msg_a['LongAccel']
      self.pts[i].yvRel = msg_b['LatSpeed']
      self.pts[i].measured = bool(msg_a['Meas'])

    ret.points = list(self.pts.values())
    return ret
