#!/usr/bin/env python3
from opendbc.car.interfaces import RadarInterfaceBase
from opendbc.car import Bus, structs
from opendbc.can.parser import CANParser

class RadarInterface(RadarInterfaceBase):
  def __init__(self, CP):
    super().__init__(CP)

    if CP.radarUnavailable:
      self.rcp = None
    else:
      messages = [('GW_31A', 10)]
      self.rcp = CANParser(DBC[CP.carFingerprint][Bus.pt], messages, 2)
      self.trigger_msg = 0x31A

    self.updated_messages = set()

  def update(self, can_strings):
    if self.rcp is None:
      return super().update(None)

    values = self.rcp.update_strings(can_strings)
    self.updated_messages.update(values)

    if self.trigger_msg not in self.updated_messages:
      return None

    ret = structs.RadarData()

    #if not self.rcp.can_valid:
    #  ret.errors.canError = True

    msg_mrr = self.rcp.vl['GW_31A']

    for target_key in ('ACC_Target7', 'ACC_Target6'):
      target_id = msg_mrr.get(f'{target_key}ID', 0)
      target_det = msg_mrr.get(f'{target_key}Detection', 0)
      target_long = msg_mrr.get(f'{target_key}LngRange', 0.0)
      target_lat = msg_mrr.get(f'{target_key}LatRange', 0.0)
      if target_det and target_id > 0:
        if target_id not in self.pts:
          self.pts[target_id] = structs.RadarData.RadarPoint()
          self.pts[target_id].trackId = target_id
        self.pts[target_id].dRel = float(target_long)
        self.pts[target_id].yRel = float(target_lat)
        self.pts[target_id].vRel = float('nan')
        self.pts[target_id].aRel = float('nan')
        self.pts[target_id].yvRel = float('nan')
        self.pts[target_id].measured = True

    ret.points = list(self.pts.values())
    self.updated_messages.clear()
    return ret
