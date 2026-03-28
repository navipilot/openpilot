#include "starpilot/ui/qt/offroad/sounds_settings.h"

StarPilotSoundsPanel::StarPilotSoundsPanel(StarPilotSettingsWindow *parent, bool forceOpen) : StarPilotListWidget(parent), parent(parent) {
  forceOpenDescriptions = forceOpen;

  QStackedLayout *soundsLayout = new QStackedLayout();
  addItem(soundsLayout);

  StarPilotListWidget *soundsList = new StarPilotListWidget(this);

  ScrollView *soundsPanel = new ScrollView(soundsList, this);

  soundsLayout->addWidget(soundsPanel);

  StarPilotListWidget *alertVolumeControlList = new StarPilotListWidget(this);
  StarPilotListWidget *customAlertsList = new StarPilotListWidget(this);

  ScrollView *alertVolumeControlPanel = new ScrollView(alertVolumeControlList, this);
  ScrollView *customAlertsPanel = new ScrollView(customAlertsList, this);

  soundsLayout->addWidget(alertVolumeControlPanel);
  soundsLayout->addWidget(customAlertsPanel);

  const std::vector<std::tuple<QString, QString, QString, QString>> soundsToggles {
    {"AlertVolumeControl", tr("Alert Volume Controller"), tr("<b>Set how loud each type of openpilot alert is</b> to keep routine prompts from becoming distracting."), "../../starpilot/assets/toggle_icons/icon_mute.png"},
    {"SwitchbackModeCooldown", tr("Switchback Mode Cooldown"), tr("<b>Set the minimum time between repeated steering-limit and minimum-steer-speed alerts while \"Switchback Mode\" is active.</b><br><br>Useful on winding roads where \"Turn Exceeds Steering Limit\" and \"Steer Unavailable Under\" can repeat frequently. Set to <b>Off</b> to disable the cooldown even when the mode is on."), ""},
    {"BelowSteerSpeedVolume", tr("Min Steer Speed Alert Volume"), tr("<b>Set the volume for the \"Steer Unavailable Under\" alert shown below the car's minimum steering speed.</b><br><br>Set to <b>Muted</b> to silence only this alert."), ""},
    {"DisengageVolume", tr("Disengage Volume"), tr("<b>Set the volume for alerts when openpilot disengages.</b><br><br>Examples include: \"Cruise Fault: Restart the Car\", \"Parking Brake Engaged\", \"Pedal Pressed\"."), ""},
    {"EngageVolume", tr("Engage Volume"), tr("<b>Set the volume for the chime when openpilot engages</b>, such as after pressing the \"RESUME\" or \"SET\" steering wheel buttons."), ""},
    {"PromptVolume", tr("Prompt Volume"), tr("<b>Set the volume for prompts that need attention.</b><br><br>Examples include: \"Car Detected in Blindspot\", \"Steering Temporarily Unavailable\", \"Turn Exceeds Steering Limit\"."), ""},
    {"PromptDistractedVolume", tr("Prompt Distracted Volume"), tr("<b>Set the volume for prompts when openpilot detects driver distraction or unresponsiveness.</b><br><br>Examples include: \"Pay Attention\", \"Touch Steering Wheel\"."), ""},
    {"RefuseVolume", tr("Refuse Volume"), tr("<b>Set the volume for alerts when openpilot refuses to engage.</b><br><br>Examples include: \"Brake Hold Active\", \"Door Open\", \"Seatbelt Unlatched\"."), ""},
    {"WarningSoftVolume", tr("Warning Soft Volume"), tr("<b>Set the volume for softer warnings about potential risks.</b><br><br>Examples include: \"BRAKE! Risk of Collision\", \"Steering Temporarily Unavailable\"."), ""},
    {"WarningImmediateVolume", tr("Warning Immediate Volume"), tr("<b>Set the volume for the loudest warnings that require urgent attention.</b><br><br>Examples include: \"DISENGAGE IMMEDIATELY — Driver Distracted\", \"DISENGAGE IMMEDIATELY — Driver Unresponsive\"."), ""},

    {"CustomAlerts", tr("StarPilot Alerts"), tr("<b>Optional StarPilot alerts</b> that highlight driving events in a more noticeable way."), "../../starpilot/assets/toggle_icons/icon_green_light.png"},
    {"GoatScream", tr("Goat Scream"), tr("<b>Play the infamous \"Goat Scream\" when the steering controller reaches its limit.</b> Based on the \"Turn Exceeds Steering Limit\" event."), ""},
    {"GreenLightAlert", tr("Green Light Alert"), tr("<b>Play an alert when the model predicts a red light has turned green.</b><br><br><i><b>Disclaimer</b>: openpilot does not explicitly detect traffic lights. This alert is based on end-to-end model predictions from camera input and may trigger even when the light has not changed.</i>"), ""},
    {"LeadDepartingAlert", tr("Lead Departing Alert"), tr("<b>Play an alert when the lead vehicle departs from a stop.</b>"), ""},
    {"LoudBlindspotAlert", tr("Loud \"Car Detected in Blindspot\" Alert"), tr("<b>Play a louder alert if a vehicle is in the blind spot when attempting to change lanes.</b> Based on the \"Car Detected in Blindspot\" event."), ""},
    {"SpeedLimitChangedAlert", tr("Speed Limit Changed Alert"), tr("<b>Play an alert when the posted speed limit changes.</b>"), ""}
  };

  for (const auto &[param, title, desc, icon] : soundsToggles) {
    AbstractControl *soundsToggle;

    if (param == "AlertVolumeControl") {
      StarPilotManageControl *alertVolumeControlToggle = new StarPilotManageControl(param, title, desc, icon);
      QObject::connect(alertVolumeControlToggle, &StarPilotManageControl::manageButtonClicked, [soundsLayout, alertVolumeControlPanel]() {
        soundsLayout->setCurrentWidget(alertVolumeControlPanel);
      });
      soundsToggle = alertVolumeControlToggle;
    } else if (alertCooldownKeys.contains(param)) {
      std::map<float, QString> cooldownLabels;
      for (int i = 0; i <= 30; ++i) {
        cooldownLabels[i] = i == 0 ? tr("Off") : i == 1 ? tr("1 minute") : QString::number(i) + tr(" minutes");
      }
      soundsToggle = new StarPilotParamValueControl(param, title, desc, icon, 0, 30, QString(), cooldownLabels, 1);
    } else if (alertVolumeControlKeys.contains(param)) {
      std::map<float, QString> volumeLabels;
      for (int i = 0; i <= 101; ++i) {
        volumeLabels[i] = i == 0 ? tr("Muted") : i == 101 ? tr("Auto") : QString::number(i) + "%";
      }
      std::vector<QString> alertButton{tr("Test")};
      if (param == "WarningImmediateVolume" || param == "WarningSoftVolume") {
        soundsToggle = new StarPilotParamValueButtonControl(param, title, desc, icon, 25, 101, QString(), volumeLabels, 1, true, {}, alertButton, false, false);
      } else {
        soundsToggle = new StarPilotParamValueButtonControl(param, title, desc, icon, 0, 101, QString(), volumeLabels, 1, true, {}, alertButton, false, false);
      }

    } else if (param == "CustomAlerts") {
      StarPilotManageControl *customAlertsToggle = new StarPilotManageControl(param, title, desc, icon);
      QObject::connect(customAlertsToggle, &StarPilotManageControl::manageButtonClicked, [soundsLayout, customAlertsPanel]() {
        soundsLayout->setCurrentWidget(customAlertsPanel);
      });
      soundsToggle = customAlertsToggle;

    } else {
      soundsToggle = new ParamControl(param, title, desc, icon);
    }

    toggles[param] = soundsToggle;

    if (alertCooldownKeys.contains(param) || alertVolumeControlKeys.contains(param)) {
      alertVolumeControlList->addItem(soundsToggle);
    } else if (customAlertsKeys.contains(param)) {
      customAlertsList->addItem(soundsToggle);
    } else {
      soundsList->addItem(soundsToggle);

      parentKeys.insert(param);
    }

    if (StarPilotManageControl *frogPilotManageToggle = qobject_cast<StarPilotManageControl*>(soundsToggle)) {
      QObject::connect(frogPilotManageToggle, &StarPilotManageControl::manageButtonClicked, [this]() {
        emit openSubPanel();
        openDescriptions(forceOpenDescriptions, toggles);
      });
    }

    QObject::connect(soundsToggle, &AbstractControl::hideDescriptionEvent, [this]() {
      update();
    });
    QObject::connect(soundsToggle, &AbstractControl::showDescriptionEvent, [this]() {
      update();
    });
  }

  for (const QString &key : alertVolumeControlKeys) {
    StarPilotParamValueButtonControl *toggle = static_cast<StarPilotParamValueButtonControl*>(toggles[key]);
    QObject::connect(toggle, &StarPilotParamValueButtonControl::buttonClicked, [key, toggle, this]() {
      toggle->updateParam();
      testSound(key);
    });
  }

  QObject::connect(parent, &StarPilotSettingsWindow::closeSubPanel, [soundsLayout, soundsPanel, this] {
    openDescriptions(forceOpenDescriptions, toggles);
    soundsLayout->setCurrentWidget(soundsPanel);
  });
  QObject::connect(uiState(), &UIState::uiUpdate, this, &StarPilotSoundsPanel::updateState);

  for (auto &[key, toggle] : toggles) {
    if (alertCooldownKeys.contains(key) || alertVolumeControlKeys.contains(key)) {
      toggle->setVisible(true);
    }
  }

  initializeSoundPlayer();

  updateToggles();
}

void StarPilotSoundsPanel::showEvent(QShowEvent *event) {
  updateToggles();
}

void StarPilotSoundsPanel::initializeSoundPlayer() {
  QString program = R"(
import numpy as np
import sounddevice as sd
import sys
import wave

while True:
  try:
    line = sys.stdin.readline()
    if not line:
      break
    path, volume = line.strip().split('|')

    sound_file = wave.open(path, 'rb')
    audio = np.frombuffer(sound_file.readframes(sound_file.getnframes()), dtype=np.int16).astype(np.float32) / 32768.0

    sd.play(audio * float(volume), sound_file.getframerate())
    sd.wait()
  except Exception:
    pass
)";

  soundPlayerProcess = new QProcess(this);
  soundPlayerProcess->start("python3", QStringList{"-u", "-c", program});
}

void StarPilotSoundsPanel::updateState(const UIState &s) {
  if (!isVisible()) {
    return;
  }

  started = s.scene.started;
}

void StarPilotSoundsPanel::updateToggles() {
  const bool showAllToggles = parent->showAllTogglesEnabled();

  for (auto &[key, toggle] : toggles) {
    if (parentKeys.contains(key)) {
      toggle->setVisible(showAllToggles);
    }
  }

  for (auto &[key, toggle] : toggles) {
    if (parentKeys.contains(key)) {
      continue;
    }

    bool setVisible = showAllToggles || parent->tuningLevel >= parent->starpilotToggleLevels[key].toDouble();

    if (!showAllToggles) {
      if (key == "LoudBlindspotAlert") {
        setVisible &= parent->hasBSM;
      }

      else if (key == "SpeedLimitChangedAlert") {
        setVisible &= params.getBool("ShowSpeedLimits") || (parent->hasOpenpilotLongitudinal && params.getBool("SpeedLimitController"));
      }
    }

    toggle->setVisible(setVisible);

    if (setVisible) {
      if (alertCooldownKeys.contains(key) || alertVolumeControlKeys.contains(key)) {
        toggles["AlertVolumeControl"]->setVisible(true);
      } else if (customAlertsKeys.contains(key)) {
        toggles["CustomAlerts"]->setVisible(true);
      }
    }
  }

  openDescriptions(forceOpenDescriptions, toggles);

  update();
}

void StarPilotSoundsPanel::testSound(const QString &key) {
  QString baseName = QString(key).remove("Volume");

  if (started) {
    updateStarPilotToggles();

    util::sleep_for(UI_FREQ);

    QString camelCaseAlert = baseName == "BelowSteerSpeed" ? "belowSteerSpeed" : QString(baseName).replace(0, 1, baseName[0].toLower());
    params_memory.put("TestAlert", camelCaseAlert.toStdString());
  } else {
    QString previewBaseName = baseName == "BelowSteerSpeed" ? "Prompt" : baseName;
    QString snakeCaseAlert = QString(previewBaseName).replace(QRegularExpression("([A-Z])"), "_\\1").toLower().mid(1);
    QString stockPath = "../../selfdrive/assets/sounds/" + snakeCaseAlert + ".wav";
    QString themePath = "../../starpilot/assets/active_theme/sounds/" + snakeCaseAlert + ".wav";

    float volume = params.getFloat(key.toStdString()) / 100.0f;

    soundPlayerProcess->write(((QFile::exists(themePath) ? themePath : stockPath) + "|" + QString::number(volume) + "\n").toUtf8());
  }
}
