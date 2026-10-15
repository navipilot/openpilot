"""Independent traffic-control observation and longitudinal constraint support."""
from openpilot.sunnypilot.selfdrive.traffic_control.controller import TrafficControlMode


TRAFFIC_SIGNAL_CONTROL_PARAM = "TeslaTrafficSignalControlEnabled"

__all__ = (
  "TRAFFIC_SIGNAL_CONTROL_PARAM", "TrafficControlMode",
)
