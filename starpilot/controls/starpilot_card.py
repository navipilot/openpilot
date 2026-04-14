#!/usr/bin/env python3
from opendbc.safety import ALTERNATIVE_EXPERIENCE
from openpilot.common.params import Params
from openpilot.selfdrive.car.cruise import CRUISE_LONG_PRESS, ButtonType
from openpilot.selfdrive.selfdrived.events import ET

from openpilot.starpilot.common.starpilot_utilities import is_FrogsGoMoo
from openpilot.starpilot.common.starpilot_variables import ERROR_LOGS_PATH, GearShifter, NON_DRIVING_GEARS
from openpilot.starpilot.controls.lib.conditional_experimental_mode import CEStatus

class StarPilotCard:
  def __init__(self, CP, FPCP):
    self.CP = CP

    self.params = Params(return_defaults=True)
    self.params_memory = Params(memory=True)

    self.accel_pressed = False
    self.always_on_lateral_allowed = False
    self.prev_active = False
    self.decel_pressed = False
    self.distancePressed_previously = False
    self.force_coast = False
    self.pause_lateral = False
    self.pause_longitudinal = False
    self.switchback_mode_enabled = self.params_memory.get_bool("SwitchbackModeEnabled")
    self.traffic_mode_enabled = False

    self.gap_counter = 0

    self.always_on_lateral_set = bool(FPCP.alternativeExperience & ALTERNATIVE_EXPERIENCE.ALWAYS_ON_LATERAL)
    self.frogs_go_moo = is_FrogsGoMoo()

    self.long_press_threshold = CRUISE_LONG_PRESS * (1.5 if self.CP.brand == "gm" else 1)
    self.very_long_press_threshold = CRUISE_LONG_PRESS * 5

    self.error_log = ERROR_LOGS_PATH / "error.txt"

  def handle_button_event(self, key, sm, starpilot_toggles):
    if sm["carControl"].longActive and getattr(starpilot_toggles, f"experimental_mode_via_{key}"):
      self.handle_experimental_mode(sm, starpilot_toggles)
    elif getattr(starpilot_toggles, f"force_coast_via_{key}"):
      self.force_coast = not self.force_coast
    elif getattr(starpilot_toggles, f"pause_lateral_via_{key}"):
      self.pause_lateral = not self.pause_lateral
    elif getattr(starpilot_toggles, f"pause_longitudinal_via_{key}"):
      self.pause_longitudinal = not self.pause_longitudinal
    elif getattr(starpilot_toggles, f"switchback_mode_via_{key}"):
      self.switchback_mode_enabled = not self.switchback_mode_enabled
      self.params_memory.put_bool("SwitchbackModeEnabled", self.switchback_mode_enabled)
    elif sm["carControl"].longActive and getattr(starpilot_toggles, f"traffic_mode_via_{key}"):
      self.traffic_mode_enabled = not self.traffic_mode_enabled

  def handle_experimental_mode(self, sm, starpilot_toggles):
    if getattr(starpilot_toggles, "safe_mode", False):
      return

    if starpilot_toggles.conditional_experimental_mode:
      if self.params_memory.get_int("CEStatus") in (CEStatus["USER_DISABLED"], CEStatus["USER_OVERRIDDEN"]):
        override_value = CEStatus["OFF"]
      elif sm["selfdriveState"].experimentalMode:
        override_value = CEStatus["USER_DISABLED"]
      else:
        override_value = CEStatus["USER_OVERRIDDEN"]

      self.params_memory.put_int("CEStatus", override_value)
    else:
      self.params.put_bool_nonblocking("ExperimentalMode", not sm["selfdriveState"].experimentalMode)

  def update(self, carState, starpilotCarState, sm, starpilot_toggles):
    self.switchback_mode_enabled = self.params_memory.get_bool("SwitchbackModeEnabled")

    if self.CP.brand == "hyundai":
      for be in carState.buttonEvents:
        if be.type == ButtonType.lkas and be.pressed and starpilot_toggles.always_on_lateral_lkas:
          self.always_on_lateral_allowed = not self.always_on_lateral_allowed
        elif be.type == ButtonType.mainCruise and be.pressed and starpilot_toggles.always_on_lateral_main:
          self.always_on_lateral_allowed = not self.always_on_lateral_allowed
    elif starpilot_toggles.always_on_lateral_main:
      self.always_on_lateral_allowed = carState.cruiseState.available

    # On rising edge of engagement (SET press enabling lat+long), auto-enable AOL
    # so that lateral persists when braking disengages longitudinal
    if sm["selfdriveState"].active and not self.prev_active and self.always_on_lateral_set and starpilot_toggles.always_on_lateral_lkas:
      self.always_on_lateral_allowed = True

    self.prev_active = sm["selfdriveState"].active

    self.always_on_lateral_enabled = self.always_on_lateral_allowed and self.always_on_lateral_set
    self.always_on_lateral_enabled &= carState.gearShifter not in NON_DRIVING_GEARS
    self.always_on_lateral_enabled &= sm["starpilotPlan"].lateralCheck
    self.always_on_lateral_enabled &= sm["liveCalibration"].calPerc >= 1
    self.always_on_lateral_enabled &= (ET.IMMEDIATE_DISABLE not in sm["selfdriveState"].alertType + sm["starpilotSelfdriveState"].alertType) or self.frogs_go_moo
    self.always_on_lateral_enabled &= not (carState.brakePressed and carState.vEgo < starpilot_toggles.always_on_lateral_pause_speed) or carState.standstill
    self.always_on_lateral_enabled &= not self.error_log.is_file() or self.frogs_go_moo

    if sm.updated["starpilotPlan"] or any(be.type in (ButtonType.accelCruise, ButtonType.resumeCruise) for be in carState.buttonEvents):
      self.accel_pressed = any(be.type in (ButtonType.accelCruise, ButtonType.resumeCruise) for be in carState.buttonEvents)

    if sm.updated["starpilotPlan"] or any(be.type == ButtonType.decelCruise for be in carState.buttonEvents):
      self.decel_pressed = any(be.type == ButtonType.decelCruise for be in carState.buttonEvents)

    starpilotCarState.distancePressed |= self.params_memory.get_bool("OnroadDistanceButtonPressed")

    if starpilotCarState.distancePressed:
      self.gap_counter += 1
    elif not self.distancePressed_previously:
      self.gap_counter = 0

    self.distancePressed_previously = starpilotCarState.distancePressed

    if not starpilotCarState.distancePressed and 1 <= self.gap_counter < self.long_press_threshold:
      self.handle_button_event("distance", sm, starpilot_toggles)
    elif self.gap_counter == self.long_press_threshold:
      self.handle_button_event("distance_long", sm, starpilot_toggles)
    elif self.gap_counter == self.very_long_press_threshold:
      self.handle_button_event("distance_long", sm, starpilot_toggles)
      self.handle_button_event("distance_very_long", sm, starpilot_toggles)

    if any(be.pressed and be.type == ButtonType.lkas for be in carState.buttonEvents):
      self.handle_button_event("lkas", sm, starpilot_toggles)

    self.force_coast &= not (carState.brakePressed or carState.gasPressed)

    starpilotCarState.accelPressed = self.accel_pressed
    starpilotCarState.alwaysOnLateralEnabled = self.always_on_lateral_enabled
    starpilotCarState.decelPressed = self.decel_pressed
    starpilotCarState.distanceLongPressed = self.very_long_press_threshold > self.gap_counter >= self.long_press_threshold
    starpilotCarState.distanceVeryLongPressed = self.gap_counter >= self.very_long_press_threshold
    starpilotCarState.forceCoast = self.force_coast
    starpilotCarState.isParked = carState.gearShifter == GearShifter.park
    starpilotCarState.pauseLateral = self.pause_lateral
    starpilotCarState.pauseLongitudinal = self.pause_longitudinal
    starpilotCarState.trafficModeEnabled = self.traffic_mode_enabled

    return starpilotCarState
