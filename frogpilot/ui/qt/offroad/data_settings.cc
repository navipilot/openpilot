#include "frogpilot/ui/qt/offroad/data_settings.h"

FrogPilotDataPanel::FrogPilotDataPanel(FrogPilotSettingsWindow *parent, bool forceOpen) : FrogPilotListWidget(parent), parent(parent) {
  forceOpenDescriptions = forceOpen;

  QStackedLayout *dataLayout = new QStackedLayout();
  addItem(dataLayout);

  FrogPilotListWidget *dataMainList = new FrogPilotListWidget(this);
  ScrollView *dataMainPanel = new ScrollView(dataMainList, this);
  dataLayout->addWidget(dataMainPanel);

  FrogPilotListWidget *statsLabelsList = new FrogPilotListWidget(this);
  ScrollView *statsLabelsPanel = new ScrollView(statsLabelsList, this);
  dataLayout->addWidget(statsLabelsPanel);

  FrogPilotButtonsControl *viewStatsButton = new FrogPilotButtonsControl(tr("FrogPilot Stats"), tr("<b>View your collected FrogPilot stats.</b>"), "", {tr("RESET"), tr("VIEW")});
  QObject::connect(viewStatsButton, &FrogPilotButtonsControl::buttonClicked, [dataLayout, statsLabelsPanel, this](int id) {
    if (id == 0) {
      if (ConfirmationDialog::confirm(tr("Are you sure you want to reset all of your FrogPilot stats?"), tr("Reset"), this)) {
        params.remove("FrogPilotStats");
      }
    } else if (id == 1) {
      emit openSubPanel();
      dataLayout->setCurrentWidget(statsLabelsPanel);
    }
  });
  if (forceOpenDescriptions) {
    viewStatsButton->showDescription();
  }
  dataMainList->addItem(viewStatsButton);

  QObject::connect(parent, &FrogPilotSettingsWindow::closeSubPanel, [dataLayout, dataMainPanel] {
    dataLayout->setCurrentWidget(dataMainPanel);
  });
  QObject::connect(parent, &FrogPilotSettingsWindow::updateMetric, [this](bool metric){isMetric = metric;});
  QObject::connect(uiState(), &UIState::offroadTransition, [statsLabelsList, this](bool offroad) {
    if (offroad) {
      updateStatsLabels(statsLabelsList);
    }
  });

  updateStatsLabels(statsLabelsList);
}

void FrogPilotDataPanel::updateStatsLabels(FrogPilotListWidget *labelsList) {
  labelsList->clear();

  QJsonObject stats = QJsonDocument::fromJson(QByteArray::fromStdString(params.get("FrogPilotStats"))).object();

  static QMap<QString, QPair<QString, QString>> keyMap = {
    {"AEBEvents", {tr("Total Emergency Brake Alerts"), "count"}},
    {"AOLTime", {tr("Time Using \"Always On Lateral\""), "timePercent"}},
    {"CruiseSpeedTimes", {tr("Favorite Set Speed"), "speed"}},
    {"CurrentMonthsMeters", {tr("Distance Driven This Month"), "distance"}},
    {"DayTime", {tr("Time Driving (Daytime)"), "timePercent"}},
    {"Disengages", {tr("Total Disengagements"), "count"}},
    {"Engages", {tr("Total Engagements"), "count"}},
    {"ExperimentalModeTime", {tr("Time Using \"Experimental Mode\""), "timePercent"}},
    {"FrogChirps", {tr("Total Frog Chirps"), "count"}},
    {"FrogHops", {tr("Total Frog Hops"), "count"}},
    {"FrogPilotDrives", {tr("Total Drives"), "count"}},
    {"FrogPilotMeters", {tr("Total Distance Driven"), "distance"}},
    {"FrogPilotSeconds", {tr("Total Driving Time"), "time"}},
    {"FrogSqueaks", {tr("Total Frog Squeaks"), "count"}},
    {"GoatScreams", {tr("Total Goat Screams"), "count"}},
    {"HighestAcceleration", {tr("Highest Acceleration Rate"), "accel"}},
    {"LateralTime", {tr("Time Using Lateral Control"), "timePercent"}},
    {"LongestDistanceWithoutOverride", {tr("Longest Distance Without an Override"), "distance"}},
    {"LongitudinalTime", {tr("Time Using Longitudinal Control"), "timePercent"}},
    {"ModelTimes", {tr("Driving Models:"), "parent"}},
    {"Month", {tr("Month"), "other"}},
    {"NightTime", {tr("Time Driving (Nighttime)"), "timePercent"}},
    {"Overrides", {tr("Total Overrides"), "count"}},
    {"OverrideTime", {tr("Time Overriding openpilot"), "timePercent"}},
    {"PersonalityTimes", {tr("Driving Personalities:"), "parent"}},
    {"RandomEvents", {tr("Random Events:"), "parent"}},
    {"StandstillTime", {tr("Time Stopped"), "timePercent"}},
    {"StopLightTime", {tr("Time Spent at Stoplights"), "timePercent"}},
    {"TrackedTime", {tr("Total Time Tracked"), "time"}},
    {"WeatherTimes", {tr("Time Driven (Weather):"), "parent"}}
  };

  static QMap<QString, QString> randomEventsMap = {
    {"accel30", tr("UwUs")},
    {"accel35", tr("Loch Ness Encounters")},
    {"accel40", tr("Visits to 1955")},
    {"dejaVuCurve", tr("Deja Vu Moments")},
    {"firefoxSteerSaturated", tr("Internet Explorer Weeeeeeees")},
    {"hal9000", tr("HAL 9000 Denials")},
    {"openpilotCrashedRandomEvent", tr("openpilot Crashes")},
    {"thisIsFineSteerSaturated", tr("This Is Fine Moments")},
    {"toBeContinued", tr("To Be Continued Moments")},
    {"vCruise69", tr("Noices")},
    {"yourFrogTriedToKillMe", tr("Attempted Frog Murders")},
    {"youveGotMail", tr("Total Mail Received")}
  };

  static QSet<QString> ignoredKeys = {
    "Month"
  };

  QStringList keys = keyMap.keys();
  std::sort(keys.begin(), keys.end(), [&](const QString &a, const QString &b) {
    return keyMap.value(a).first.toLower() < keyMap.value(b).first.toLower();
  });

  std::function<QString(double)> format_number = [&](double number) {
    return QLocale().toString(number);
  };

  std::function<QString(double)> format_distance = [&](double meters) {
    double value;
    QString unit;
    if (isMetric) {
      value = meters / 1000.0;
      unit = (value == 1.0) ? tr(" kilometer") : tr(" kilometers");
    } else {
      value = meters * METER_TO_MILE;
      unit = (value == 1.0) ? tr(" mile") : tr(" miles");
    }
    return format_number(qRound(value)) + unit;
  };

  std::function<QString(int)> format_time = [&](int seconds) {
    static int secondsInDay = 60 * 60 * 24;
    static int secondsInHour = 60 * 60;

    int days = seconds / secondsInDay;
    int hours = (seconds % secondsInDay) / secondsInHour;
    int minutes = (seconds % secondsInHour) / 60;

    QString result;
    if (days > 0) {
      result += format_number(days) + (days == 1 ? tr(" day ") : tr(" days "));
    }
    if (hours > 0 || days > 0) {
      result += format_number(hours) + (hours == 1 ? tr(" hour ") : tr(" hours "));
    }
    result += format_number(minutes) + (minutes == 1 ? tr(" minute") : tr(" minutes"));
    return result.trimmed();
  };

  double trackedTime = stats.contains("TrackedTime") ? stats.value("TrackedTime").toDouble() : 0.0;

  for (const QString &key : keys) {
    if (ignoredKeys.contains(key)) {
      continue;
    }

    QJsonValue value = stats.contains(key) ? stats.value(key) : QJsonValue(0);
    QString labelText = keyMap.value(key).first;
    QString type = keyMap.value(key).second;

    if (key == "AEBEvents") {
      QJsonObject totalEvents = stats.value("TotalEvents").toObject();

      QString trimmedLabel = labelText;
      QString prefix = tr("Total ");
      if (trimmedLabel.startsWith(prefix)) {
        trimmedLabel = trimmedLabel.mid(prefix.length());
      }
      QString displayValue = format_number(totalEvents.value("stockAeb").toInt(0) + totalEvents.value("fcw").toInt(0)) + " " + trimmedLabel;

      labelsList->addItem(new LabelControl(labelText, displayValue, "", this));

    } else if (key == "CruiseSpeedTimes" && value.isObject()) {
      QJsonObject speeds = value.toObject();

      double maxTime = -1.0;
      QString bestSpeed;
      for (const QString &speedKey : speeds.keys()) {
        double time = speeds.value(speedKey).toDouble();
        if (time > maxTime) {
          bestSpeed = speedKey;
          maxTime = time;
        }
      }

      QString displaySpeed;
      if (isMetric) {
        displaySpeed = QString::number(qRound(bestSpeed.toDouble() * MS_TO_KPH)) + " " + tr("km/h");
      } else {
        displaySpeed = QString::number(qRound(bestSpeed.toDouble() * MS_TO_MPH)) + " " + tr("mph");
      }

      labelsList->addItem(new LabelControl(labelText, displaySpeed + " (" + format_time(maxTime) + ")", "", this));
    } else if (type == "parent" && value.isObject()) {
      labelsList->addItem(new LabelControl(labelText, "", "", this));

      QJsonObject subObject = value.toObject();
      QStringList subKeys;

      if (key == "RandomEvents") {
        subKeys = randomEventsMap.keys();
      } else {
        subKeys = subObject.keys();
      }

      std::sort(subKeys.begin(), subKeys.end(), [&](const QString &a, const QString &b) {
        QString displayA, displayB;
        if (key == "RandomEvents") {
          displayA = randomEventsMap.value(a, a);
          displayB = randomEventsMap.value(b, b);
        } else {
          displayA = a;
          displayB = b;
        }
        return displayA.toLower() < displayB.toLower();
      });

      for (const QString &subkey : subKeys) {
        if (subkey == "Unknown") {
          continue;
        }

        QString displaySubKey;
        if (key == "ModelTimes") {
          displaySubKey = cleanModelName(subkey);
        } else if (key == "RandomEvents") {
          displaySubKey = randomEventsMap.value(subkey, subkey);
        } else if (key == "WeatherTimes") {
          displaySubKey = subkey.left(1).toUpper() + subkey.mid(1);
        } else {
          displaySubKey = subkey;
        }

        QString subvalue;
        if (key.endsWith("Times")) {
          subvalue = format_time(subObject.value(subkey).toDouble());
        } else {
          subvalue = format_number(subObject.value(subkey).toInt(0));
        }

        labelsList->addItem(new LabelControl("     " + displaySubKey, subvalue, "", this));
      }
    } else {
      QString displayValue;
      if (type == "accel") {
        displayValue = QString::number(value.toDouble(), 'f', 2) + " " + tr("m/s²");
      } else if (type == "count") {
        QString trimmedLabel = labelText;
        QString prefix = tr("Total ");
        if (trimmedLabel.startsWith(prefix)) {
          trimmedLabel = trimmedLabel.mid(prefix.length());
        }
        displayValue = format_number(value.toInt()) + " " + trimmedLabel;
      } else if (type == "distance") {
        displayValue = format_distance(value.toDouble());
      } else if (type == "time" || type == "timePercent") {
        displayValue = format_time(value.toDouble());
      } else {
        QString stringValue = value.toVariant().toString();
        displayValue = stringValue.isEmpty() ? "0" : stringValue;
      }

      labelsList->addItem(new LabelControl(labelText, displayValue, "", this));

      if (type == "timePercent") {
        int percent = 0;
        if (trackedTime > 0.0) {
          percent = (value.toDouble() * 100.0) / trackedTime;
        }

        labelsList->addItem(new LabelControl(tr("% of ") + labelText, format_number(percent) + "%", "", this));
      }
    }
  }
}
