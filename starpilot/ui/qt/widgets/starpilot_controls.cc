#include "selfdrive/ui/ui.h"

#include "starpilot/ui/starpilot_ui.h"

bool StarPilotConfirmationDialog::toggleReboot(QWidget *parent) {
  ConfirmationDialog d(tr("Reboot required to take effect."), tr("Reboot Now"), tr("Reboot Later"), false, parent);
  return d.exec();
}

bool StarPilotConfirmationDialog::yesorno(const QString &prompt_text, QWidget *parent) {
  ConfirmationDialog d(prompt_text, tr("Yes"), tr("No"), false, parent);
  return d.exec();
}

bool isFrogsGoMoo() {
  static bool is_FrogsGoMoo = QFile::exists("/persist/frogsgomoo.py");
  return is_FrogsGoMoo;
}

bool useKonikServer() {
  static bool use_konik = QFile::exists("/cache/use_konik");
  return use_konik;
}

void clearMovie(QSharedPointer<QMovie> &movie, QWidget *parent) {
  if (!movie) {
    return;
  }

  QObject::disconnect(movie.data(), nullptr, parent, nullptr);
  movie->stop();
  movie.reset();
}

void loadGif(const QString &gifPath, QSharedPointer<QMovie> &movie, const QSize &size, QWidget *parent) {
  if (!parent || gifPath.isEmpty()) {
    return;
  }

  if (movie && movie->fileName() == gifPath && movie->state() == QMovie::Running) {
    if (movie->scaledSize() != size) {
      movie->setScaledSize(size);
    }
    return;
  }

  if (!QFileInfo::exists(gifPath)) {
    clearMovie(movie, parent);
    return;
  }

  clearMovie(movie, parent);

  movie = QSharedPointer<QMovie>::create(gifPath);
  movie->setCacheMode(QMovie::CacheAll);
  movie->setScaledSize(size);

  QPointer<QWidget> safeParent(parent);
  QObject::connect(movie.data(), &QMovie::frameChanged, parent, [safeParent]() {
    if (safeParent && safeParent->isVisible()) {
      safeParent->update();
    }
  }, Qt::UniqueConnection);

  movie->start();
}

static QString resolveImagePath(const QString &basePath, const QStringList &extensions) {
  for (const QString &extension : extensions) {
    const QString candidate = basePath + "." + extension;
    if (QFileInfo::exists(candidate)) {
      return candidate;
    }
  }

  return {};
}

void loadImage(const QString &basePath, QPixmap &pixmap, QSharedPointer<QMovie> &movie, const QSize &size, QWidget *parent) {
  if (!parent || basePath.isEmpty()) {
    return;
  }

  static QHash<QString, QPixmap> pixmapCache;
  const QString gifPath = resolveImagePath(basePath, {"gif"});
  if (!gifPath.isEmpty()) {
    loadGif(gifPath, movie, size, parent);
    if (!pixmap.isNull()) {
      pixmap = QPixmap();
    }
    return;
  }

  clearMovie(movie, parent);

  const QString imagePath = resolveImagePath(basePath, {"png", "webp", "jpg", "jpeg"});
  if (imagePath.isEmpty()) {
    if (!pixmap.isNull()) {
      pixmap = QPixmap();
      parent->update();
    }
    return;
  }

  const QFileInfo imageInfo(imagePath);
  const QString cacheKey = imagePath + QString("_%1x%2_%3").arg(size.width()).arg(size.height()).arg(imageInfo.lastModified().toMSecsSinceEpoch());

  if (pixmapCache.contains(cacheKey)) {
    QPixmap &cached = pixmapCache[cacheKey];
    if (pixmap.cacheKey() != cached.cacheKey()) {
      pixmap = cached;
      parent->update();
    }
    return;
  }

  QPixmap loadedPixmap(imagePath);
  if (!loadedPixmap.isNull()) {
    pixmap = loadedPixmap.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    pixmapCache.insert(cacheKey, pixmap);
  } else {
    pixmap = QPixmap();
  }

  parent->update();
}

void openDescriptions(bool forceOpenDescriptions, std::map<QString, AbstractControl*> toggles) {
  if (forceOpenDescriptions) {
    for (auto &[key, toggle] : toggles) {
      if (key != "CESpeed") {
        toggle->showDescription();
      }
    }
  }
}

void updateStarPilotToggles() {
  static Params params_memory{"", true};
  params_memory.putBool("StarPilotTogglesUpdated", true);
}

QString cleanModelName(QString modelName) {
  return modelName.remove("_default").remove("(Default)");
}
