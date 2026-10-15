from itertools import permutations

import pytest

import openpilot.cereal.messaging as messaging
from openpilot.sunnypilot.selfdrive.traffic_control.tesla_observer import (
  TeslaTrafficControlObservation,
  TeslaTrafficControlObserver,
  publish_tesla_traffic_control,
)


def _frame(values, bus=2):
  fields = {
    "APP_tcFeatureState": (0, 2),
    "APP_tcStateMachine": (2, 3),
    "APP_tcControlSource": (9, 2),
    "APP_tcControlType": (11, 5),
    "APP_tcControlDistance": (16, 8),
    "APP_tcControlLightState": (24, 3),
    "APP_tcContinuationReason": (27, 4),
    "APP_tcConfirmationType": (31, 2),
    "APP_tcWarningSuppressionReason": (33, 5),
    "APP_tcUnavailableReason": (38, 4),
    "APP_tcVisionLight": (42, 1),
    "APP_tcVisionSign": (43, 1),
    "APP_tcVisionRoadMarking": (44, 1),
    "APP_tcVisionLine": (45, 1),
  }
  raw = 0
  for name, value in values.items():
    start, size = fields[name]
    raw |= (int(value) & ((1 << size) - 1)) << start
  return 0x25D, raw.to_bytes(8, "little"), bus


def test_observer_accepts_party_traffic_light_and_rejects_veh_collision():
  observer = TeslaTrafficControlObserver()
  values = {
    "APP_tcFeatureState": 3,
    "APP_tcStateMachine": 4,
    "APP_tcControlSource": 3,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 80,
    "APP_tcControlLightState": 1,
    "APP_tcVisionLight": 1,
  }

  address, data, _ = _frame(values)
  observer.update([(1_000_000_000, [(address, data, 1)])], 1_000_000_000)
  assert not observer.snapshot(1_000_000_000).available

  observer.update([(1_050_000_000, [(address, data, 2)])], 1_050_000_000)
  observation = observer.snapshot(1_050_000_000)
  assert observation.available
  assert observation.valid_for_control
  assert observation.source_bus == 2
  assert observation.distance == 80
  assert observation.light_state == 1
  assert observation.quality == 2
  assert observation.raw_address == address
  assert observation.raw_payload == data


def test_raw_frame_diagnostics_survive_car_state_sp_serialization():
  address, data, _ = _frame({
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 42,
    "APP_tcControlLightState": 2,
  })
  observer = TeslaTrafficControlObserver()
  observer.update([(1_234_567_890, [(address, data[:6], 2)])], 1_234_567_890)
  message = messaging.new_message("carStateSP")
  publish_tesla_traffic_control(message.carStateSP, observer.snapshot(1_234_567_890))
  decoded = messaging.log_from_bytes(message.to_bytes()).carStateSP.teslaTrafficControl
  restored = TeslaTrafficControlObservation.from_message(decoded)
  assert restored.raw_address == 0x25D
  assert restored.raw_payload == data[:6]
  assert restored.frame_mono_time == 1_234_567_890
  assert (restored.light_state, restored.control_type, restored.distance) == (2, 3, 42)


def test_observer_treats_255_as_no_target_and_supports_shortened_dlc():
  observer = TeslaTrafficControlObserver()
  address, data, _ = _frame({
    "APP_tcFeatureState": 0,
    "APP_tcStateMachine": 6,
    "APP_tcControlSource": 3,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 255,
    "APP_tcControlLightState": 1,
  })
  observer.update([(2_000_000_000, [(address, data[:6], 2)])], 2_000_000_000)
  observation = observer.snapshot(2_000_000_000)
  assert observation.available
  assert observation.dlc == 6
  assert observation.raw_payload == data[:6]
  assert observation.distance == 255
  assert not observation.valid_for_control


def test_observer_accepts_logged_red_aware_state_when_feature_flag_is_disabled():
  observer = TeslaTrafficControlObserver()
  address, data, _ = _frame({
    "APP_tcFeatureState": 0,
    "APP_tcStateMachine": 2,
    "APP_tcControlSource": 3,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 19,
    "APP_tcControlLightState": 1,
    "APP_tcUnavailableReason": 1,
    "APP_tcVisionLight": 1,
  })
  observer.update([(3_000_000_000, [(address, data, 2)])], 3_000_000_000)
  observation = observer.snapshot(3_000_000_000)
  assert observation.valid_for_control
  assert observation.feature_state == 0
  assert observation.control_type == 3
  assert observation.light_state == 1
  assert observation.distance == 19
  assert observation.state_machine == 2
  assert observation.quality == 2


def test_observer_accepts_logged_feature_zero_yellow_transition():
  observer = TeslaTrafficControlObserver()
  address, data, _ = _frame({
    "APP_tcFeatureState": 0,
    "APP_tcStateMachine": 6,
    "APP_tcControlSource": 3,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 49,
    "APP_tcControlLightState": 3,
    "APP_tcContinuationReason": 5,
    "APP_tcUnavailableReason": 1,
    "APP_tcVisionLight": 1,
  })

  observer.update([(3_050_000_000, [(address, data, 2)])], 3_050_000_000)
  observation = observer.snapshot(3_050_000_000)

  assert observation.valid_for_control
  assert observation.feature_state == 0
  assert observation.state_machine == 6
  assert observation.control_source == 3
  assert observation.light_state == 3
  assert observation.continuation_reason == 5
  assert observation.distance == 49


def test_observer_ignores_internal_availability_fields_for_color_distance_control():
  observer = TeslaTrafficControlObserver()
  address, data, _ = _frame({
    "APP_tcFeatureState": 0,
    "APP_tcStateMachine": 6,
    "APP_tcControlSource": 3,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 19,
    "APP_tcControlLightState": 1,
    "APP_tcUnavailableReason": 1,
    "APP_tcVisionLight": 1,
  })
  observer.update([(3_100_000_000, [(address, data, 2)])], 3_100_000_000)
  assert observer.snapshot(3_100_000_000).valid_for_control


def test_observer_uses_only_ap_party_and_never_falls_back_to_party():
  observer = TeslaTrafficControlObserver()
  values = {
    "APP_tcFeatureState": 3,
    "APP_tcStateMachine": 4,
    "APP_tcControlSource": 3,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 60,
    "APP_tcControlLightState": 1,
    "APP_tcVisionLight": 1,
  }
  address, data, _ = _frame(values)
  observer.update([(5_000_000_000, [(address, data, 2)])], 5_000_000_000)

  party_values = dict(values, APP_tcControlDistance=20, APP_tcControlLightState=2)
  _, party_data, _ = _frame(party_values, bus=0)
  observer.update([(5_050_000_000, [(address, party_data, 0)])], 5_050_000_000)

  observation = observer.snapshot(5_050_000_000)
  assert observation.source_bus == 2
  assert observation.light_state == 1
  assert observation.distance == 60

  expired = observer.snapshot(5_800_000_001)
  assert expired.source_bus == 2
  assert not expired.available
  assert expired.distance == 60


def test_observer_accepts_two_hz_frames_but_expires_after_750ms():
  observer = TeslaTrafficControlObserver()
  address, data, _ = _frame({
    "APP_tcFeatureState": 3,
    "APP_tcControlSource": 2,
    "APP_tcControlType": 3,
    "APP_tcControlDistance": 50,
    "APP_tcControlLightState": 1,
  })
  observer.update([(4_000_000_000, [(address, data, 2)])], 4_000_000_000)
  assert observer.snapshot(4_200_000_000).available
  assert observer.snapshot(4_700_000_000).available
  assert not observer.snapshot(4_750_000_001).available


def test_observer_accepts_200m_boundary_and_rejects_201m_for_control():
  observer = TeslaTrafficControlObserver()
  for timestamp, distance in ((6_000_000_000, 200), (6_100_000_000, 201)):
    address, data, _ = _frame({
      "APP_tcControlType": 3,
      "APP_tcControlDistance": distance,
      "APP_tcControlLightState": 1,
    })
    observer.update([(timestamp, [(address, data, 2)])], timestamp)
    assert observer.snapshot(timestamp).valid_for_control == (distance == 200)


def test_observer_does_not_mix_valid_frame_metadata_with_short_tail_values():
  observer = TeslaTrafficControlObserver()
  red = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 80, "APP_tcControlLightState": 1})
  address, data, bus = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 20, "APP_tcControlLightState": 2})
  observer.update([(2_000_000_000, [red, (address, data[:4], bus)])], 2_000_000_000)

  observation = observer.snapshot(2_000_000_000)
  assert (observation.light_state, observation.distance, observation.dlc, observation.frame_mono_time) == (
    1, 80, 8, 2_000_000_000,
  )
  assert observation.valid_for_control


def test_observer_uses_latest_qualified_frame_regardless_of_batch_order():
  red_address, red_data, red_bus = _frame({
    "APP_tcControlType": 3, "APP_tcControlDistance": 80, "APP_tcControlLightState": 1,
  })
  green = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 20, "APP_tcControlLightState": 2})
  yellow = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 40, "APP_tcControlLightState": 3})
  packets = [
    (3_000_000_000, [(red_address, red_data[:6], red_bus)]),
    (2_800_000_000, [green]),
    (2_900_000_000, [yellow]),
    (3_100_000_000, [(green[0], green[1][:4], green[2])]),
    (3_200_000_000, [(green[0], green[1], 1)]),
  ]
  for batch in permutations(packets):
    observer = TeslaTrafficControlObserver()
    observer.update(list(batch), 3_200_000_000)
    observation = observer.snapshot(3_200_000_000)
    assert (observation.light_state, observation.distance, observation.dlc, observation.frame_mono_time) == (
      1, 80, 6, 3_000_000_000,
    )
    assert observation.valid_for_control


def test_observer_same_timestamp_uses_last_qualified_frame_as_one_tuple():
  observer = TeslaTrafficControlObserver()
  red = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 80, "APP_tcControlLightState": 1})
  address, data, bus = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 20, "APP_tcControlLightState": 2})
  observer.update([(2_000_000_000, [red, (address, data[:6], bus), (red[0], red[1][:4], red[2])])], 2_000_000_000)
  observation = observer.snapshot(2_000_000_000)
  assert (observation.light_state, observation.distance, observation.dlc) == (2, 20, 6)


def test_rejected_or_older_frames_cannot_refresh_the_observation():
  observer = TeslaTrafficControlObserver()
  red = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 80, "APP_tcControlLightState": 1})
  green = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 20, "APP_tcControlLightState": 2})
  observer.update([(2_000_000_000, [red])], 2_000_000_000)
  original = observer.snapshot(2_000_000_000)
  observer.update([(1_900_000_000, [green])], 2_100_000_000)
  assert observer.snapshot(2_100_000_000) == original

  # The real parser rejects payloads over 64 bytes; it must not pair its
  # previous decoded values with the new rejected frame's timestamp/DLC.
  observer.update([(3_000_000_000, [(green[0], green[1] + bytes(57), green[2])])], 3_000_000_000)
  expired = observer.snapshot(3_000_000_000)
  assert not expired.available
  assert not expired.valid_for_control
  assert (expired.distance, expired.frame_mono_time, expired.dlc) == (80, 2_000_000_000, 8)


@pytest.mark.parametrize(("rejected_time", "rejected_first"), [
  (2_000_000_000, False), (2_100_000_000, False), (2_100_000_000, True),
])
def test_rejected_latest_decode_does_not_mask_a_valid_frame_in_the_same_batch(rejected_time, rejected_first):
  observer = TeslaTrafficControlObserver()
  red = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 80, "APP_tcControlLightState": 1})
  green = _frame({"APP_tcControlType": 3, "APP_tcControlDistance": 20, "APP_tcControlLightState": 2})
  packets = [
    (2_000_000_000, [red]),
    (rejected_time, [(green[0], green[1] + bytes(57), green[2])]),
  ]
  observer.update(packets[::-1] if rejected_first else packets, 2_100_000_000)
  result = observer.snapshot(2_100_000_000)
  assert result.available and result.valid_for_control
  assert (result.light_state, result.distance, result.frame_mono_time) == (1, 80, 2_000_000_000)
