#include "frogpilot/ui/frogpilot_ui.h"

#include <cstdlib>
#include <string>

#include "system/hardware/hw.h"

static void update_state(FrogPilotUIState *fs) {
  FrogPilotUIScene &frogpilot_scene = fs->frogpilot_scene;

  SubMaster &fpsm = *(fs->sm);
  fpsm.update(0);

  const char *desktop_fake_wifi_env = std::getenv("SP_ALLOW_DESKTOP_FAKE_WIFI");
  const bool desktop_force_online = Hardware::PC() && desktop_fake_wifi_env != nullptr && std::string(desktop_fake_wifi_env) != "0";
  if (desktop_force_online) {
    frogpilot_scene.online = true;
  }

  if (fpsm.updated("deviceState")) {
    const cereal::DeviceState::Reader &deviceState = fpsm["deviceState"].getDeviceState();
    if (!desktop_force_online) {
      frogpilot_scene.online = deviceState.getNetworkType() != cereal::DeviceState::NetworkType::NONE;
    }
  }
  if (fpsm.updated("frogpilotCarState")) {
    const cereal::FrogPilotCarState::Reader &frogpilotCarState = fpsm["frogpilotCarState"].getFrogpilotCarState();
    frogpilot_scene.always_on_lateral_active = !frogpilot_scene.enabled && frogpilotCarState.getAlwaysOnLateralEnabled();
    frogpilot_scene.traffic_mode_enabled = frogpilotCarState.getTrafficModeEnabled();
  }
  if (fpsm.updated("frogpilotPlan")) {
    const cereal::FrogPilotPlan::Reader &frogpilotPlan = fpsm["frogpilotPlan"].getFrogpilotPlan();
    if (frogpilotPlan.getThemeUpdated()) {
      emit fs->themeUpdated();
    }
    capnp::Text::Reader toggles = frogpilotPlan.getFrogpilotToggles();
    QByteArray current_toggles(toggles.cStr(), toggles.size());
    static QByteArray previous_toggles;
    if (previous_toggles != current_toggles) {
      frogpilot_scene.frogpilot_toggles = QJsonDocument::fromJson(current_toggles).object();
      previous_toggles = current_toggles;
    }
  }

  // Keep force drive-state toggles authoritative from params so UI state
  // switches immediately even if frogpilotPlan is delayed.
  frogpilot_scene.frogpilot_toggles["force_offroad"] = fs->params.getBool("ForceOffroad");
  frogpilot_scene.frogpilot_toggles["force_onroad"] = fs->params.getBool("ForceOnroad");

  if (fpsm.updated("selfdriveState")) {
    const cereal::SelfdriveState::Reader &selfdriveState = fpsm["selfdriveState"].getSelfdriveState();
    frogpilot_scene.enabled = selfdriveState.getEnabled();
  }
}

FrogPilotUIState::FrogPilotUIState(QObject *parent) : QObject(parent) {
  sm = std::make_unique<SubMaster, const std::initializer_list<const char *>>({
    "carControl", "deviceState", "frogpilotCarState", "frogpilotDeviceState",
    "frogpilotPlan", "frogpilotRadarState", "frogpilotSelfdriveState", "liveDelay",
    "liveParameters", "liveTorqueParameters", "liveTracks", "mapdExtendedOut", "mapdOut", "selfdriveState"
  });

  // Provide sane local defaults until frogpilotPlan publishes real toggles.
  frogpilot_scene.frogpilot_toggles = {
    {"debug_mode", false},
    {"driver_camera_in_reverse", false},
    {"force_offroad", false},
    {"force_onroad", false},
    {"screen_brightness", 101},
    {"screen_brightness_onroad", 101},
    {"screen_timeout", 30},
    {"screen_timeout_onroad", 10},
    {"sidebar_color1", "#FFFFFFFF"},
    {"sidebar_color2", "#FFFFFFFF"},
    {"sidebar_color3", "#FFFFFFFF"},
    {"standby_mode", false},
    {"tethering_config", 0},
  };

  wifi = new WifiManager(this);

  if (params.getInt("TetheringEnabled") == 1) {
    wifi->setTetheringEnabled(true);
  }
}

FrogPilotUIState *frogpilotUIState() {
  static FrogPilotUIState frogpilot_ui_state;
  return &frogpilot_ui_state;
}

void FrogPilotUIState::update() {
  update_state(this);

  frogpilot_scene.conditional_status = frogpilot_scene.enabled ? params_memory.getInt("CEStatus") : 0;
  frogpilot_scene.driver_camera_timer = frogpilot_scene.reverse && frogpilot_scene.frogpilot_toggles.value("driver_camera_in_reverse").toBool() ? frogpilot_scene.driver_camera_timer + 1 : 0;
}
