#include "starpilot/ui/qt/widgets/developer_sidebar.h"

#include <QRegularExpression>

void DeveloperSidebar::drawMetric(QPainter &p, const QPair<QString, QString> &label, QColor c, int y) {
  const QRect rect = {12, y, 275, 126};

  p.setPen(Qt::NoPen);
  p.setBrush(QBrush(c));
  p.setClipRect(rect.x() + rect.width() - 4 - 18, rect.y(), 18, rect.height(), Qt::ClipOperation::ReplaceClip);
  p.drawRoundedRect(QRect(rect.x() + rect.width() - 4 - 100, rect.y() + 4, 100, 118), 18, 18);
  p.setClipping(false);

  QPen pen = QPen(QColor(0xff, 0xff, 0xff, 0x55));
  pen.setWidth(2);
  p.setPen(pen);
  p.setBrush(Qt::NoBrush);
  p.drawRoundedRect(rect, 20, 20);

  p.setPen(QColor(0xff, 0xff, 0xff));
  p.setFont(InterFont(35, QFont::DemiBold));
  if (label.second.isEmpty()) {
    p.drawText(rect.adjusted(8, 8, -22, -8), Qt::AlignCenter | Qt::TextWordWrap, label.first);
  } else {
    p.drawText(rect.adjusted(8, 8, -22, -8), Qt::AlignCenter, label.first + "\n" + label.second);
  }
}

DeveloperSidebar::DeveloperSidebar(QWidget *parent) : QFrame(parent) {
  setAttribute(Qt::WA_OpaquePaintEvent);
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  setFixedWidth(300);

  QObject::connect(starpilotUIState(), &StarPilotUIState::themeUpdated, this, &DeveloperSidebar::updateToggles);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &DeveloperSidebar::resetVariables);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &DeveloperSidebar::updateState);
}

void DeveloperSidebar::showEvent(QShowEvent *event) {
  updateToggles();
}

void DeveloperSidebar::updateToggles() {
  StarPilotUIState &fs = *starpilotUIState();
  StarPilotUIScene &starpilot_scene = fs.starpilot_scene;
  QJsonObject &starpilot_toggles = starpilot_scene.starpilot_toggles;

  metricAssignments.clear();
  for (int i = 1; i <= 7; ++i) {
    QString key = QString("developer_sidebar_metric%1").arg(i);
    int metricId = starpilot_toggles.value(key).toInt();
    metricAssignments.push_back(metricId);
  }

  metricColor = QColor(starpilot_toggles.value("sidebar_color1").toString());
}

void DeveloperSidebar::resetVariables() {
  lateralEngagementTime = 0;
  longitudinalEngagementTime = 0;
  maxAcceleration = 0;
  totalEngagementTime = 0;
}

void DeveloperSidebar::updateState(const UIState &s, const StarPilotUIState &fs) {
  if (!isVisible()) {
    return;
  }

  const SubMaster &sm = *(s.sm);

  const StarPilotUIScene &starpilot_scene = fs.starpilot_scene;
  const SubMaster &fpsm = *(fs.sm);

  const cereal::CarControl::Reader &carControl = fpsm["carControl"].getCarControl();
  const cereal::CarState::Reader &carState = sm["carState"].getCarState();
  const cereal::StarPilotPlan::Reader &starpilotPlan = fpsm["starpilotPlan"].getStarpilotPlan();
  const cereal::LiveDelayData::Reader &liveDelay = fpsm["liveDelay"].getLiveDelay();
  const cereal::LiveParametersData::Reader &liveParameters = fpsm["liveParameters"].getLiveParameters();
  const cereal::LiveTorqueParametersData::Reader &liveTorqueParameters = fpsm["liveTorqueParameters"].getLiveTorqueParameters();
  const bool forceAutoTuneOff = starpilot_scene.starpilot_toggles.value("force_auto_tune_off").toBool();
  const bool usingLiveTorqueTune = !forceAutoTuneOff && (liveTorqueParameters.getUseParams() || starpilot_scene.starpilot_toggles.value("force_auto_tune").toBool());

  const bool is_metric = s.scene.is_metric;
  const bool use_si = starpilot_scene.starpilot_toggles.value("use_si_metrics").toBool();

  const QString accelerationUnit = (is_metric || use_si) ? tr(" m/s²") : tr(" ft/s²");
  const float accelerationConversion = (is_metric || use_si) ? 1.0f : METER_TO_FOOT;

  double acceleration = carState.getAEgo() * accelerationConversion;
  if (!carState.getGasPressed()) {
    maxAcceleration = std::max(maxAcceleration, acceleration);
  }

  lateralEngagementTime += carControl.getLatActive() && !starpilot_scene.reverse && !starpilot_scene.standstill ? 1 : 0;
  longitudinalEngagementTime += carControl.getLongActive() && !starpilot_scene.reverse && !starpilot_scene.standstill ? 1 : 0;
  totalEngagementTime += !(starpilot_scene.reverse || starpilot_scene.standstill) || totalEngagementTime == 0 ? 1 : 0;

  static int maxSteerAngle = 0;
  int currentSteerAngle = fabs(carState.getSteeringAngleDeg());

  static int maxTorque = 0;
  int currentTorque = fabs(carControl.getActuators().getTorque() * 100);

  static QElapsedTimer torqueTimer;

  if (currentTorque >= 50) {
    maxSteerAngle = std::max(maxSteerAngle, currentSteerAngle);
    maxTorque = std::max(maxTorque, currentTorque);

    torqueTimer.start();
  } else if (torqueTimer.elapsed() >= 10000) {
    maxTorque = 0;
    maxSteerAngle = 0;

    torqueTimer.invalidate();
  }

  QString steerLabel = QString::number(currentSteerAngle) + "°";
  QString torqueLabel = QString::number(currentTorque) + "%";

  if (currentTorque >= 50 || torqueTimer.isValid()) {
    steerLabel += QString(" - (%1°)").arg(maxSteerAngle);
    torqueLabel += QString(" - (%1%)").arg(maxTorque);
  }

  float displayedFriction = usingLiveTorqueTune ? liveTorqueParameters.getFrictionCoefficientFiltered() : params.getFloat("SteerFrictionStock");
  if (displayedFriction == 0.0f) {
    displayedFriction = forceAutoTuneOff ? params.getFloat("SteerFriction") : liveTorqueParameters.getFrictionCoefficientFiltered();
  }

  float displayedLatAccel = usingLiveTorqueTune ? liveTorqueParameters.getLatAccelFactorFiltered() : params.getFloat("SteerLatAccelStock");
  if (displayedLatAccel == 0.0f) {
    displayedLatAccel = forceAutoTuneOff ? params.getFloat("SteerLatAccel") : liveTorqueParameters.getLatAccelFactorFiltered();
  }

  accelerationStatus = ItemStatus(QPair<QString, QString>(tr("ACCEL"), QString::number(acceleration, 'f', 2) + accelerationUnit), metricColor);
  accelerationJerkStatus = ItemStatus(QPair<QString, QString>(tr("ACCEL JERK"), QString::number(starpilotPlan.getAccelerationJerk())), metricColor);
  actuatorAccelerationStatus = ItemStatus(QPair<QString, QString>(tr("ACT ACCEL"), QString::number(carControl.getActuators().getAccel() * accelerationConversion, 'f', 2) + accelerationUnit), metricColor);
  dangerFactorStatus = ItemStatus(QPair<QString, QString>(tr("DANGER %"), QString::number(starpilotPlan.getDangerFactor() * 100.0f, 'f', 2) + "%"), metricColor);
  dangerJerkStatus = ItemStatus(QPair<QString, QString>(tr("DANGER JERK"), QString::number(starpilotPlan.getDangerJerk())), metricColor);
  delayStatus = ItemStatus(QPair<QString, QString>(tr("STEER DELAY"), QString::number(liveDelay.getLateralDelay(), 'f', 5)), metricColor);
  frictionStatus = ItemStatus(QPair<QString, QString>(tr("FRICTION"), QString::number(displayedFriction, 'f', 5)), metricColor);
  latAccelStatus = ItemStatus(QPair<QString, QString>(tr("LAT ACCEL"), QString::number(displayedLatAccel, 'f', 5)), metricColor);
  lateralEngagementStatus = ItemStatus(QPair<QString, QString>(tr("LATERAL %"), QString::number((lateralEngagementTime / totalEngagementTime) * 100.0f, 'f', 2) + "%"), metricColor);
  longitudinalEngagementStatus = ItemStatus(QPair<QString, QString>(tr("LONG %"), QString::number((longitudinalEngagementTime / totalEngagementTime) * 100.0f, 'f', 2) + "%"), metricColor);
  maxAccelerationStatus = ItemStatus(QPair<QString, QString>(tr("MAX ACCEL"), QString::number(maxAcceleration, 'f', 2) + accelerationUnit), metricColor);
  speedJerkStatus = ItemStatus(QPair<QString, QString>(tr("SPEED JERK"), QString::number(starpilotPlan.getSpeedJerk())), metricColor);
  steerAngleStatus = ItemStatus(QPair<QString, QString>(tr("STEER ANGLE"), steerLabel), metricColor);
  steerRatioStatus = ItemStatus(QPair<QString, QString>(tr("STEER RATIO"), QString::number(liveParameters.getSteerRatio(), 'f', 5)), metricColor);
  stiffnessFactorStatus = ItemStatus(QPair<QString, QString>(tr("STEER STIFF"), QString::number(liveParameters.getStiffnessFactor(), 'f', 5)), metricColor);
  torqueStatus = ItemStatus(QPair<QString, QString>(tr("TORQUE %"), torqueLabel), metricColor);

  QString modelName = starpilot_scene.starpilot_toggles.value("model_name").toString();
  modelName.remove(QRegularExpression("\\(.*\\)"));
  modelName.remove(QRegularExpression("[^a-zA-Z0-9 \\-\\.:]"));
  modelNameStatus = ItemStatus(QPair<QString, QString>(modelName.trimmed(), ""), metricColor);

  update();
}

void DeveloperSidebar::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setPen(Qt::NoPen);
  p.setRenderHint(QPainter::Antialiasing);

  p.fillRect(rect(), QColor(57, 57, 57));

  QMap<int, ItemStatus*> metricMap;
  metricMap.insert(1, &accelerationStatus);
  metricMap.insert(2, &maxAccelerationStatus);
  metricMap.insert(3, &delayStatus);
  metricMap.insert(4, &frictionStatus);
  metricMap.insert(5, &latAccelStatus);
  metricMap.insert(6, &steerRatioStatus);
  metricMap.insert(7, &stiffnessFactorStatus);
  metricMap.insert(8, &lateralEngagementStatus);
  metricMap.insert(9, &longitudinalEngagementStatus);
  metricMap.insert(10, &steerAngleStatus);
  metricMap.insert(11, &torqueStatus);
  metricMap.insert(12, &actuatorAccelerationStatus);
  metricMap.insert(13, &dangerFactorStatus);
  metricMap.insert(14, &accelerationJerkStatus);
  metricMap.insert(15, &dangerJerkStatus);
  metricMap.insert(16, &speedJerkStatus);
  metricMap.insert(17, &modelNameStatus);

  int count = 0;
  for (size_t i = 0; i < metricAssignments.size(); ++i) {
    if (metricAssignments[i] > 0 && metricMap.contains(metricAssignments[i])) {
      count++;
    }
  }
  if (count == 0) {
    return;
  }

  int metricHeight = 126;
  int spacing = (height() - (count * metricHeight)) / (count + 1);
  int y = spacing;

  for (size_t i = 0; i < metricAssignments.size(); ++i) {
    int metricId = metricAssignments[i];

    if (metricId == 0) {
      continue;
    }

    if (!metricMap.contains(metricId)) {
      continue;
    }

    ItemStatus *status = metricMap[metricId];
    drawMetric(p, status->first, status->second, y);
    y += metricHeight + spacing;
  }
}
