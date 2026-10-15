import openpilot.cereal.messaging as messaging
from types import SimpleNamespace

import pytest

from openpilot.cereal.services import SERVICE_LIST
from openpilot.sunnypilot.selfdrive.traffic_control import trafficcontrold
from openpilot.sunnypilot.selfdrive.traffic_control.controller import TrafficControlMode, TrafficControlPhase
from openpilot.sunnypilot.selfdrive.traffic_control.radar_state import TrafficRadarGoPolicy


class FakeParams:
  def __init__(self, enabled, reference_dm="60", max_speed_kph="60"):
    self.enabled = enabled
    self.reference_dm = reference_dm
    self.max_speed_kph = max_speed_kph

  def get(self, key, return_default=False):
    del return_default
    if key == "TeslaTrafficStopReference":
      return self.reference_dm
    if key == "TeslaTrafficControlMaxSpeed":
      return self.max_speed_kph
    return None

  def get_bool(self, key):
    if key == "TeslaTrafficSignalControlEnabled":
      return self.enabled
    return False


def test_independent_traffic_radar_service_uses_the_existing_twenty_hz_slot():
  message = messaging.new_message("trafficRadarState")
  target = message.trafficRadarState
  target.targetPresent = True
  target.controlAllowed = True
  target.distanceToStopPoint = 42.0

  assert message.which() == "trafficRadarState"
  assert target.targetPresent and target.controlAllowed
  assert target.distanceToStopPoint == 42.0
  assert SERVICE_LIST["trafficRadarState"].should_log
  assert SERVICE_LIST["trafficRadarState"].frequency == 20.0
  assert SERVICE_LIST["trafficRadarState"].decimation == 5


def test_user_switch_maps_off_to_disabled_and_on_to_stop_go():
  disabled = trafficcontrold.build_source(FakeParams(False))
  active = trafficcontrold.build_source(FakeParams(True))

  assert disabled.controller.config.mode == TrafficControlMode.off
  assert disabled.go_policy == TrafficRadarGoPolicy.passive
  assert active.controller.config.mode == TrafficControlMode.stopGo
  assert active.go_policy == TrafficRadarGoPolicy.active


def test_disabled_source_does_not_compute_stop_candidates_but_keeps_raw_distance():
  from openpilot.sunnypilot.selfdrive.traffic_control.tests.test_radar_state import red_light_sm
  source = trafficcontrold.build_source(FakeParams(False))
  sm = red_light_sm()
  for now_ns in range(100_000_000, 1_100_000_000, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState
    assert target.mode == int(TrafficControlMode.off)
    assert target.phase == int(TrafficControlPhase.off)
    assert not target.targetPresent
    assert not target.controlAllowed
    assert not target.plannerStartRequested
    assert target.stopSessionId == 0
    assert target.rawDistance == sm["carStateSP"].teslaTrafficControl.distance


def test_disabling_clears_session_and_reenabling_requires_new_confirmation():
  from openpilot.sunnypilot.selfdrive.traffic_control.tests.test_radar_state import red_light_sm
  params = FakeParams(True)
  source = trafficcontrold.build_source(params)
  sm = red_light_sm()
  for now_ns in range(100_000_000, 1_100_000_000, 100_000_000):
    sm["carStateSP"].teslaTrafficControl.frameMonoTime = now_ns
    target = source.update(sm, now_ns).trafficRadarState
  assert target.stopSessionId > 0 and target.controlAllowed

  params.enabled = False
  trafficcontrold.refresh_source_config(source, params)
  assert source.controller.stop_session_id == 0
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_100_000_000
  target = source.update(sm, 1_100_000_000).trafficRadarState
  assert target.mode == 0 and target.stopSessionId == 0 and not target.controlAllowed

  params.enabled = True
  trafficcontrold.refresh_source_config(source, params)
  sm["carStateSP"].teslaTrafficControl.frameMonoTime = 1_200_000_000
  target = source.update(sm, 1_200_000_000).trafficRadarState
  assert target.phase == int(TrafficControlPhase.redCandidate)
  assert not target.controlAllowed


def test_manual_stop_reference_uses_fixed_cp_style_geometry():
  source = trafficcontrold.build_source(FakeParams(True, reference_dm="20"))

  assert source.controller.config.default_stop_reference == 2.0


def test_control_speed_setting_is_read_as_kph_and_applies_live():
  params = FakeParams(True, max_speed_kph="60")
  source = trafficcontrold.build_source(params)
  assert source.controller.config.max_control_speed == pytest.approx(60.0 / 3.6)

  params.max_speed_kph = "45"
  trafficcontrold.refresh_source_config(source, params)
  assert source.controller.config.max_control_speed == pytest.approx(45.0 / 3.6)


def test_runtime_stop_reference_refresh_applies_when_idle_without_moving_an_active_event():
  params = FakeParams(True)
  source = trafficcontrold.build_source(params)
  assert source.controller.stop_reference == 6.0

  params.reference_dm = "50"
  trafficcontrold.refresh_source_config(source, params)
  assert source.controller.config.default_stop_reference == 5.0
  assert source.controller.stop_reference == 5.0

  source.controller.phase = TrafficControlPhase.braking
  source.controller.stop_reference = 5.0
  params.reference_dm = "40"
  trafficcontrold.refresh_source_config(source, params)
  assert source.controller.config.default_stop_reference == 4.0
  assert source.controller.stop_reference == 5.0

  source.controller.reset()
  assert source.controller.stop_reference == 4.0


def test_trafficcontrold_publishes_only_the_independent_traffic_radar_service(monkeypatch):
  published = {}

  class FakeSubMaster:
    updated = {"modelV2": True}
    logMonoTime = {"modelV2": 123}

    def __init__(self, services, **kwargs):
      published["subscriptions"] = tuple(services)
      published["submaster_options"] = kwargs
      self.update_count = 0

    def update(self):
      if self.update_count:
        raise StopIteration
      self.update_count += 1

  class FakePubMaster:
    def __init__(self, services):
      published["services"] = tuple(services)

    def send(self, service, message):
      published["sent"] = (service, message)

  message = object()
  source = SimpleNamespace(update=lambda sm, now_ns: message)
  monkeypatch.setattr(trafficcontrold, "config_realtime_process", lambda *args: None)
  monkeypatch.setattr(trafficcontrold, "Params", lambda: object())
  monkeypatch.setattr(trafficcontrold, "build_source", lambda params: source)
  monkeypatch.setattr(trafficcontrold.messaging, "SubMaster", FakeSubMaster)
  monkeypatch.setattr(trafficcontrold.messaging, "PubMaster", FakePubMaster)

  with pytest.raises(StopIteration):
    trafficcontrold.main()

  assert published["services"] == ("trafficRadarState",)
  assert published["sent"] == ("trafficRadarState", message)
  assert "modelV2" in published["subscriptions"]
  assert "radarState" not in published["subscriptions"]
  assert not ({"radarState", "modelV2", "can", "sendcan"} & set(published["services"]))


def test_trafficcontrold_refreshes_runtime_settings_without_restarting(monkeypatch):
  refreshed = []

  class FakeSubMaster:
    updated = {"modelV2": True}
    logMonoTime = {"modelV2": 123}

    def __init__(self, _services, **_kwargs):
      self.update_count = 0

    def update(self):
      if self.update_count >= 20:
        raise StopIteration
      self.update_count += 1

  class FakePubMaster:
    def __init__(self, _services):
      pass

    def send(self, _service, _message):
      pass

  params = FakeParams(True)
  source = SimpleNamespace(update=lambda _sm, _now_ns: object())
  monkeypatch.setattr(trafficcontrold, "config_realtime_process", lambda *args: None)
  monkeypatch.setattr(trafficcontrold, "Params", lambda: params)
  monkeypatch.setattr(trafficcontrold, "build_source", lambda _params: source)
  monkeypatch.setattr(trafficcontrold, "refresh_source_config", lambda _source, _params: refreshed.append(True))
  monkeypatch.setattr(trafficcontrold.messaging, "SubMaster", FakeSubMaster)
  monkeypatch.setattr(trafficcontrold.messaging, "PubMaster", FakePubMaster)

  with pytest.raises(StopIteration):
    trafficcontrold.main()

  assert refreshed == [True]
