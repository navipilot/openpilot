from types import SimpleNamespace

from opendbc.car import structs

from openpilot.selfdrive.car.car_specific import CarSpecificEvents, EventName


def create_events(brand):
  events = CarSpecificEvents(SimpleNamespace(
    brand=brand,
    openpilotLongitudinalControl=True,
    pcmCruise=True,
  ))
  writes = []
  events.params = SimpleNamespace(put_bool=lambda key, value: writes.append((key, value)))
  return events, writes


def cancel_in_park(events):
  state = structs.CarState(gearShifter=structs.CarState.GearShifter.park)
  state.cruiseState.available = True
  state.buttonEvents = [structs.CarState.ButtonEvent(
    pressed=True,
    type=structs.CarState.ButtonEvent.Type.cancel,
  )]
  return events.create_common_events(state, structs.CarState(), allow_button_cancel=True)


def test_tesla_cancel_in_park_does_not_request_shutdown():
  events, writes = create_events("tesla")
  result = cancel_in_park(events)
  assert EventName.buttonCancel in result.names
  assert writes == []


def test_non_tesla_cancel_in_park_retains_shutdown_behavior():
  events, writes = create_events("hyundai")
  result = cancel_in_park(events)
  assert EventName.buttonCancel in result.names
  assert writes == [("DoShutdown", True)]
