#include "libyuv.h"

#include "selfdrive/ui/qt/util.h"

#include "frogpilot/ui/screenrecorder/screenrecorder.h"

constexpr int BITRATE = 8 * 1024 * 1024;
constexpr int MAX_DURATION = 1000 * 60 * 5;
constexpr int SCREEN_WIDTH = 2160;
constexpr int SCREEN_HEIGHT = 1080;

const QDir RECORDINGS_FOLDER("/data/media/screen_recordings");

ScreenRecorder::ScreenRecorder(QWidget *parent) : QPushButton(parent) {
  setFixedSize(btn_size, btn_size);

  rootWidget = topWidget(this);

  QObject::connect(this, &QPushButton::clicked, this, &ScreenRecorder::toggleRecording);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &ScreenRecorder::stopRecording);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &ScreenRecorder::updateState);
}

void ScreenRecorder::updateState() {
  if (!recording) {
    return;
  }

  if (QDateTime::currentMSecsSinceEpoch() - startedTime > MAX_DURATION) {
    stopRecording();
    startRecording();
    return;
  }

  if (captureBuffer.size() != QSize(SCREEN_WIDTH, SCREEN_HEIGHT)) {
    captureBuffer = QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_RGBA8888);
  }

  QPainter p(&captureBuffer);
  rootWidget->render(&p);
  p.end();

  imageQueue.push(QImage(captureBuffer));
}

void ScreenRecorder::toggleRecording() {
  recording ? stopRecording() : startRecording();
}

void ScreenRecorder::startRecording() {
  encoder = std::make_unique<OmxEncoder>(
    RECORDINGS_FOLDER.path().toStdString().c_str(),
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    UI_FREQ,
    BITRATE
  );
  encoder->encoder_open((QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss").toStdString() + ".mp4").c_str());

  if (!encoder->is_open) {
    encoder.reset();
    return;
  }

  recording = true;

  startedTime = QDateTime::currentMSecsSinceEpoch();

  imageQueue.clear();

  encodingThread = std::thread(&ScreenRecorder::encodeImage, this);

  update();
}

void ScreenRecorder::stopRecording() {
  if (!recording) {
    return;
  }

  recording = false;

  if (encodingThread.joinable()) {
    encodingThread.join();
  }

  if (encoder) {
    encoder->encoder_close();
    encoder.reset();
  }

  update();
}

void ScreenRecorder::encodeImage() {
  while (recording) {
    QImage image;
    if (imageQueue.pop_wait_for(image, std::chrono::milliseconds(1000))) {
      uint64_t currentTimestamp = nanos_since_boot();

      if (image.format() != QImage::Format_RGBA8888) {
        image = image.convertToFormat(QImage::Format_RGBA8888);
      }

      const uint8_t *bits = image.constBits();
      if (bits) {
        encoder->encode_frame_rgba(bits, SCREEN_WIDTH, SCREEN_HEIGHT, currentTimestamp);
      }
    }
  }
}

void ScreenRecorder::paintEvent(QPaintEvent *event) {
  QPainter p(this);
  p.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

  if (recording) {
    qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - startedTime;

    qreal phase = (elapsed % 2000) / 2000.0 * 2 * M_PI;
    qreal alphaFactor = 0.5 + 0.5 * sin(phase);

    QColor glowColor = redColor();
    glowColor.setAlphaF(0.3 + 0.7 * alphaFactor);

    int glowWidth = 8 + static_cast<int>(2 * alphaFactor);

    p.setBrush(blackColor(166));
    p.setFont(InterFont(25, QFont::Bold));
    p.setPen(QPen(glowColor, glowWidth));
  } else {
    p.setBrush(blackColor(166));
    p.setFont(InterFont(25, QFont::DemiBold));
    p.setPen(QPen(redColor(), 8));
  }

  int centeringOffset = 10;

  QRect buttonRect(centeringOffset, btn_size / 3, btn_size - centeringOffset * 2, btn_size / 3);
  p.drawRoundedRect(buttonRect, 24, 24);

  QRect textRect = buttonRect.adjusted(centeringOffset, 0, -centeringOffset, 0);
  p.setPen(QPen(whiteColor(), 6));
  p.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, recording ? tr("RECORDING") : tr("RECORD"));

  if (!recording) {
    p.setBrush(redColor(166));
    p.setPen(Qt::NoPen);
    p.drawEllipse(QPoint(buttonRect.right() - btn_size / 10 - centeringOffset, buttonRect.center().y()), btn_size / 10, btn_size / 10);
  }
}
