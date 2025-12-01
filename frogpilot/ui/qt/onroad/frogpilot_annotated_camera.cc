#include "frogpilot/ui/qt/onroad/frogpilot_annotated_camera.h"

FrogPilotAnnotatedCameraWidget::FrogPilotAnnotatedCameraWidget(QWidget *parent) : QWidget(parent) {
  QSize iconSize(img_size / 4, img_size / 4);
}

void FrogPilotAnnotatedCameraWidget::showEvent(QShowEvent *event) {
}

void FrogPilotAnnotatedCameraWidget::updateState(const UIState &s, const FrogPilotUIState &fs) {
  const UIScene &scene = s.scene;

  const SubMaster &sm = *(s.sm);
  const SubMaster &fpsm = *(fs.sm);

  const cereal::CarState::Reader &carState = sm["carState"].getCarState();
  const cereal::FrogPilotCarState::Reader &frogpilotCarState = fpsm["frogpilotCarState"].getFrogpilotCarState();
  const cereal::FrogPilotPlan::Reader &frogpilotPlan = fpsm["frogpilotPlan"].getFrogpilotPlan();
  const cereal::FrogPilotSelfdriveState::Reader &frogpilotSelfdriveState = fpsm["frogpilotSelfdriveState"].getFrogpilotSelfdriveState();
  const cereal::ModelDataV2::Reader &modelV2 = sm["modelV2"].getModelV2();
  const cereal::SelfdriveState::Reader &selfdriveState = sm["selfdriveState"].getSelfdriveState();

  if (scene.is_metric || frogpilot_toggles.value("use_si_metrics").toBool()) {
    leadDistanceUnit = tr(" meters");
    leadSpeedUnit = frogpilot_toggles.value("use_si_metrics").toBool() ? tr(" m/s") : tr(" km/h");
    speedUnit = scene.is_metric ? tr("km/h") : tr("mph");

    distanceConversion = 1.0f;
    speedConversion = scene.is_metric ? MS_TO_KPH : MS_TO_MPH;
    speedConversionMetrics = frogpilot_toggles.value("use_si_metrics").toBool() ? 1.0f : MS_TO_KPH;
  } else {
    leadDistanceUnit = tr(" feet");
    leadSpeedUnit = tr(" mph");
    speedUnit = tr("mph");

    distanceConversion = METER_TO_FOOT;
    speedConversion = MS_TO_MPH;
    speedConversionMetrics = MS_TO_MPH;
  }

  blindspotLeft = carState.getLeftBlindspot();
  blindspotRight = carState.getRightBlindspot();

  hideBottomIcons = selfdriveState.getAlertSize() != cereal::SelfdriveState::AlertSize::NONE;
  hideBottomIcons |= frogpilotSelfdriveState.getAlertSize() != cereal::FrogPilotSelfdriveState::AlertSize::NONE;
}

void FrogPilotAnnotatedCameraWidget::mousePressEvent(QMouseEvent *mouseEvent) {
  mouseEvent->ignore();
}

void FrogPilotAnnotatedCameraWidget::paintFrogPilotWidgets(QPainter &p, UIState &s) {
  if (!hideBottomIcons && frogpilot_toggles.value("compass").toBool()) {
    paintCompass(p);
  } else {
    compassPosition.setX(0);
    compassPosition.setY(0);
  }
}

void FrogPilotAnnotatedCameraWidget::paintBlindSpotPath(QPainter &p) {
  p.save();

  QLinearGradient bs(0, height(), 0, 0);
  bs.setColorAt(0.0f, QColor::fromHslF(0 / 360.0f, 0.75f, 0.5f, 0.4f));
  bs.setColorAt(0.5f, QColor::fromHslF(0 / 360.0f, 0.75f, 0.5f, 0.35f));
  bs.setColorAt(1.0f, QColor::fromHslF(0 / 360.0f, 0.75f, 0.5f, 0.0f));
  p.setBrush(bs);

  if (track_adjacent_vertices[0].boundingRect().width() > 0 && blindspotLeft) {
    p.drawPolygon(track_adjacent_vertices[0]);
  }
  if (track_adjacent_vertices[1].boundingRect().width() > 0 && blindspotRight) {
    p.drawPolygon(track_adjacent_vertices[1]);
  }

  p.restore();
}

void FrogPilotAnnotatedCameraWidget::paintCompass(QPainter &p) {
  if (dmIconPosition == QPoint(0, 0)) {
    return;
  }

  p.save();

  constexpr double PIXELS_PER_DEGREE = 2.5;

  constexpr int BASE_RIBBON_WIDTH = static_cast<int>(360 * PIXELS_PER_DEGREE);
  constexpr int BORDER_WIDTH = 10;
  constexpr int MARGIN = 5;
  constexpr int TRIANGLE_SIZE = 40;

  static QPixmap compassRibbon = [&]() {
    QPixmap ribbon(BASE_RIBBON_WIDTH * 2, widget_size);
    ribbon.fill(Qt::transparent);

    QPainter ribbonPainter(&ribbon);
    ribbonPainter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    QFont font = InterFont(65, QFont::Bold);
    ribbonPainter.setFont(font);
    QFontMetrics fm(font);

    QMap<int, QString> directionLabels = {{0, "N"}, {45, "NE"}, {90, "E"}, {135, "SE"}, {180, "S"}, {225, "SW"}, {270, "W"}, {315, "NW"}, {360, "N"}};

    for (int cycle = 0; cycle < 2; ++cycle) {
      int xOffset = cycle * 360;

      for (int degree = 0; degree < 360; ++degree) {
        int x = qRound((xOffset + degree) * PIXELS_PER_DEGREE);

        if (directionLabels.contains(degree)) {
          QString label = directionLabels[degree];
          ribbonPainter.setPen(whiteColor());
          ribbonPainter.drawText(x - fm.horizontalAdvance(label) / 2, fm.ascent(), label);
        }

        int notchHeight = (degree % 45 == 0) ? 35 : (degree % 15 == 0) ? 25 : 15;
        int notchWidth = (degree % 45 == 0) ? 5 : (degree % 15 == 0) ? 4 : 3;

        ribbonPainter.setPen(QPen(whiteColor(), notchWidth));
        ribbonPainter.drawLine(x, widget_size - notchHeight - MARGIN, x, widget_size);
      }
    }

    return ribbon;
  }();

  compassPosition.rx() = rightHandDM ? UI_BORDER_SIZE + widget_size / 2 : width() - UI_BORDER_SIZE - btn_size;
  compassPosition.ry() = dmIconPosition.y() - widget_size / 2;

  QRect compassWidget(compassPosition, QSize(widget_size, widget_size));

  p.setBrush(blackColor(166));
  p.setPen(QPen(blackColor(), BORDER_WIDTH));
  p.drawRoundedRect(compassWidget, 24, 24);

  QPainterPath clipPath;
  clipPath.addRoundedRect(compassWidget.adjusted(MARGIN, MARGIN, -MARGIN, -MARGIN), 24, 24);
  p.setClipPath(clipPath);

  double rawBearing = QJsonDocument::fromJson(QByteArray::fromStdString(params_memory.get("LastGPSPosition"))).object().value("bearing").toDouble(0.0);
  int bearing = qRound(fmod(rawBearing + 360.0, 360.0));
  int offset = qRound(bearing * PIXELS_PER_DEGREE) % BASE_RIBBON_WIDTH;
  int drawX = compassWidget.center().x() - offset;

  p.drawPixmap(drawX - BASE_RIBBON_WIDTH, compassWidget.top() + MARGIN, compassRibbon);
  p.drawPixmap(drawX, compassWidget.top() + MARGIN, compassRibbon);

  int triangleX = compassWidget.center().x();
  int triangleY = compassWidget.bottom() - TRIANGLE_SIZE;
  QPolygon triangle({
    QPoint(triangleX, triangleY - TRIANGLE_SIZE),
    QPoint(triangleX - TRIANGLE_SIZE / 1.5, triangleY),
    QPoint(triangleX + TRIANGLE_SIZE / 1.5, triangleY)
  });

  p.setBrush(whiteColor());
  p.setPen(Qt::NoPen);
  p.drawPolygon(triangle);

  p.restore();
}
