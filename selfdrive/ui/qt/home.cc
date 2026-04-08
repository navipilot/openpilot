#include "selfdrive/ui/qt/home.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "selfdrive/ui/qt/offroad/experimental_mode.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/widgets/prime.h"

#include "starpilot/ui/qt/widgets/drive_stats.h"
#include "starpilot/ui/qt/widgets/drive_summary.h"

// HomeWindow: the container for the offroad and onroad UIs

HomeWindow::HomeWindow(QWidget* parent) : QWidget(parent) {
  QHBoxLayout *main_layout = new QHBoxLayout(this);
  main_layout->setMargin(0);
  main_layout->setSpacing(0);

  sidebar = new Sidebar(this);
  main_layout->addWidget(sidebar);
  QObject::connect(sidebar, &Sidebar::openSettings, this, &HomeWindow::openSettings);

  slayout = new QStackedLayout();
  main_layout->addLayout(slayout);

  home = new OffroadHome(this);
  QObject::connect(home, &OffroadHome::openSettings, this, &HomeWindow::openSettings);
  slayout->addWidget(home);

  onroad = new OnroadWindow(this);
  slayout->addWidget(onroad);

  body = new BodyWindow(this);
  slayout->addWidget(body);

  driver_view = new DriverViewWindow(this);
  connect(driver_view, &DriverViewWindow::done, [=] {
    showDriverView(false);
  });
  slayout->addWidget(driver_view);
  setAttribute(Qt::WA_NoSystemBackground);
  QObject::connect(uiState(), &UIState::uiUpdate, this, &HomeWindow::updateState);
  QObject::connect(uiState(), &UIState::offroadTransition, this, &HomeWindow::offroadTransition);
  QObject::connect(uiState(), &UIState::offroadTransition, sidebar, &Sidebar::offroadTransition);

  developer_sidebar = new DeveloperSidebar(this);
  main_layout->addWidget(developer_sidebar);
  developer_sidebar->setVisible(false);
}

void HomeWindow::showSidebar(bool show) {
  sidebar->setVisible(show);
}

void HomeWindow::updateState(const UIState &s, const StarPilotUIState &fs) {
  const SubMaster &sm = *(s.sm);

  // switch to the generic robot UI
  if (onroad->isVisible() && !body->isEnabled() && sm["carParams"].getCarParams().getNotCar()) {
    body->setEnabled(true);
    slayout->setCurrentWidget(body);
  }

  const StarPilotUIScene &starpilot_scene = fs.starpilot_scene;
  const QJsonObject &starpilot_toggles = starpilot_scene.starpilot_toggles;

  if (s.scene.started) {
    if (starpilot_scene.driver_camera_timer >= UI_FREQ / 2) {
      showDriverView(true, true);
    } else {
      if (driver_view->isVisible()) {
        sidebar->setVisible(params.getBool("Sidebar") || starpilot_toggles.value("debug_mode").toBool());
        slayout->setCurrentWidget(onroad);
      }

      developer_sidebar->setVisible(starpilot_toggles.value("developer_sidebar").toBool());

      starpilotUIState()->starpilot_scene.sidebars_open = developer_sidebar->isVisible() && sidebar->isVisible();
    }
  }
}

void HomeWindow::offroadTransition(bool offroad) {
  StarPilotUIState &fs = *starpilotUIState();
  StarPilotUIScene &starpilot_scene = fs.starpilot_scene;
  QJsonObject &starpilot_toggles = starpilot_scene.starpilot_toggles;

  body->setEnabled(false);
  sidebar->setVisible(offroad || params.getBool("SidebarOpen") || starpilot_toggles.value("debug_mode").toBool());
  if (offroad) {
    slayout->setCurrentWidget(home);

    developer_sidebar->setVisible(false);
  } else {
    slayout->setCurrentWidget(onroad);
  }
}

void HomeWindow::showDriverView(bool show, bool started) {
  if (show) {
    if (!started) {
      emit closeSettings();
    }
    slayout->setCurrentWidget(driver_view);
  } else {
    slayout->setCurrentWidget(home);
  }
  sidebar->setVisible(show == false);

  developer_sidebar->setVisible(false);
}

void HomeWindow::mousePressEvent(QMouseEvent* e) {
  // Handle sidebar collapsing
  if ((onroad->isVisible() || body->isVisible()) && (!sidebar->isVisible() || e->x() > sidebar->width())) {
    sidebar->setVisible(!sidebar->isVisible());

    params.putBool("SidebarOpen", sidebar->isVisible());
  }
}

void HomeWindow::mouseDoubleClickEvent(QMouseEvent* e) {
  HomeWindow::mousePressEvent(e);
  const SubMaster &sm = *(uiState()->sm);
  if (sm["carParams"].getCarParams().getNotCar()) {
    if (onroad->isVisible()) {
      slayout->setCurrentWidget(body);
    } else if (body->isVisible()) {
      slayout->setCurrentWidget(onroad);
    }
    showSidebar(false);
  }
}

// OffroadHome: the offroad home page

OffroadHome::OffroadHome(QWidget* parent) : QFrame(parent) {
  QVBoxLayout* main_layout = new QVBoxLayout(this);
  main_layout->setContentsMargins(40, 40, 40, 40);

  // top header
  QHBoxLayout* header_layout = new QHBoxLayout();
  header_layout->setContentsMargins(0, 0, 0, 0);
  header_layout->setSpacing(16);

  date = new ElidedLabel();
  header_layout->addWidget(date, 0, Qt::AlignHCenter | Qt::AlignLeft);

  update_notif = new QPushButton(tr("UPDATE"));
  update_notif->setVisible(false);
  update_notif->setStyleSheet("background-color: #364DEF;");
  QObject::connect(update_notif, &QPushButton::clicked, [=]() { center_layout->setCurrentIndex(1); });
  header_layout->addWidget(update_notif, 0, Qt::AlignHCenter | Qt::AlignLeft);

  alert_notif = new QPushButton();
  alert_notif->setVisible(false);
  alert_notif->setStyleSheet("background-color: #E22C2C;");
  QObject::connect(alert_notif, &QPushButton::clicked, [=] { center_layout->setCurrentIndex(2); });
  header_layout->addWidget(alert_notif, 0, Qt::AlignHCenter | Qt::AlignLeft);

  version = new ElidedLabel();
  header_layout->addWidget(version, 0, Qt::AlignHCenter | Qt::AlignRight);

  main_layout->addLayout(header_layout);

  // main content
  main_layout->addSpacing(25);
  center_layout = new QStackedLayout();

  QWidget *home_widget = new QWidget(this);
  {
    QHBoxLayout *home_layout = new QHBoxLayout(home_widget);
    home_layout->setContentsMargins(0, 0, 0, 0);
    home_layout->setSpacing(30);

    // left: stock prime card in simple mode, StarPilot drive stats otherwise
    left_widget = new QStackedWidget(this);

    stock_left_widget = new QStackedWidget(this);
    QVBoxLayout *left_prime_layout = new QVBoxLayout();
    left_prime_layout->setContentsMargins(0, 0, 0, 0);
    QWidget *prime_user = new PrimeUserWidget();
    prime_user->setStyleSheet(R"(
    border-radius: 10px;
    background-color: #333333;
    )");
    left_prime_layout->addWidget(prime_user);
    left_prime_layout->addStretch();
    stock_left_widget->addWidget(new LayoutWidget(left_prime_layout));
    stock_left_widget->addWidget(new PrimeAdWidget);
    stock_left_widget->setStyleSheet("border-radius: 10px;");
    QObject::connect(uiState()->prime_state, &PrimeState::changed, this, [this]() {
      stock_left_widget->setCurrentIndex(uiState()->prime_state->isSubscribed() ? 0 : 1);
    });
    left_widget->addWidget(stock_left_widget);

    QWidget *custom_left_widget = new QWidget(this);
    custom_left_stack = new QStackedLayout(custom_left_widget);
    custom_left_stack->setContentsMargins(0, 0, 0, 0);
    custom_left_stack->addWidget(new DriveStats());
    StarPilotDriveSummary *drive_summary = new StarPilotDriveSummary(this);
    custom_left_stack->addWidget(drive_summary);
    left_widget->addWidget(custom_left_widget);
    left_widget->setCurrentIndex(params.getBool("SimpleMode") ? 0 : 1);

    QObject::connect(drive_summary, &StarPilotDriveSummary::panelClosed, [this]() {
      custom_left_stack->setCurrentIndex(0);
    });
    QObject::connect(uiState(), &UIState::offroadTransition, [this](bool offroad) {
      static bool previouslyOnroad = false;
      if (offroad && previouslyOnroad && !params.getBool("SimpleMode")) {
        custom_left_stack->setCurrentIndex(1);
      }
      previouslyOnroad = !offroad;
    });

    home_layout->addWidget(left_widget, 1);

    // right: ExperimentalModeButton, SetupWidget, Random Events Summary
    right_widget = new QStackedWidget(this);
    right_widget->setFixedWidth(750);

    QWidget *default_right = new QWidget(this);
    QVBoxLayout *right_column = new QVBoxLayout(default_right);
    right_column->setContentsMargins(0, 0, 0, 0);
    right_column->setSpacing(30);

    ExperimentalModeButton *experimental_mode = new ExperimentalModeButton(this);
    QObject::connect(experimental_mode, &ExperimentalModeButton::openSettings, this, &OffroadHome::openSettings);
    right_column->addWidget(experimental_mode, 1);

    SetupWidget *setup_widget = new SetupWidget;
    QObject::connect(setup_widget, &SetupWidget::openSettings, this, &OffroadHome::openSettings);
    right_column->addWidget(setup_widget, 1);

    right_widget->addWidget(default_right);

    StarPilotDriveSummary *random_events_summary = new StarPilotDriveSummary(this, true);
    right_widget->addWidget(random_events_summary);
    right_widget->setCurrentIndex(0);

    QObject::connect(random_events_summary, &StarPilotDriveSummary::panelClosed, [=]() {
      right_widget->setCurrentIndex(0);
    });
    QObject::connect(uiState(), &UIState::offroadTransition, [this](bool offroad) {
      static bool previouslyOnroad = false;
      if (offroad && previouslyOnroad && !params.getBool("SimpleMode") &&
          starpilotUIState()->starpilot_scene.starpilot_toggles.value("random_events").toBool()) {
        right_widget->setCurrentIndex(1);
      }
      previouslyOnroad = !offroad;
    });

    home_layout->addWidget(right_widget, 0);
  }
  center_layout->addWidget(home_widget);

  // add update & alerts widgets
  update_widget = new UpdateAlert();
  QObject::connect(update_widget, &UpdateAlert::dismiss, [=]() { center_layout->setCurrentIndex(0); });
  center_layout->addWidget(update_widget);
  alerts_widget = new OffroadAlert();
  QObject::connect(alerts_widget, &OffroadAlert::dismiss, [=]() { center_layout->setCurrentIndex(0); });
  center_layout->addWidget(alerts_widget);

  main_layout->addLayout(center_layout, 1);

  // set up refresh timer
  timer = new QTimer(this);
  timer->callOnTimeout(this, &OffroadHome::refresh);

  setStyleSheet(R"(
    * {
      color: white;
    }
    OffroadHome {
      background-color: black;
    }
    OffroadHome > QPushButton {
      padding: 15px 30px;
      border-radius: 5px;
      font-size: 40px;
      font-weight: 500;
    }
    OffroadHome > QLabel {
      font-size: 55px;
    }
  )");
}

void OffroadHome::showEvent(QShowEvent *event) {
  refresh();
  timer->start(10 * 1000);
}

void OffroadHome::hideEvent(QHideEvent *event) {
  timer->stop();
}

void OffroadHome::refresh() {
  bool updateAvailable = update_widget->refresh();
  int alerts = alerts_widget->refresh();

  // pop-up new notification
  int idx = center_layout->currentIndex();
  if (!updateAvailable && !alerts) {
    idx = 0;
  } else if (updateAvailable && (!update_notif->isVisible() || (!alerts && idx == 2))) {
    idx = 1;
  } else if (alerts && (!alert_notif->isVisible() || (!updateAvailable && idx == 1))) {
    idx = 2;
  }
  center_layout->setCurrentIndex(idx);

  update_notif->setVisible(updateAvailable);
  alert_notif->setVisible(alerts);
  if (alerts) {
    alert_notif->setText(QString::number(alerts) + (alerts > 1 ? tr(" ALERTS") : tr(" ALERT")));
  }

  StarPilotUIState &fs = *starpilotUIState();
  StarPilotUIScene &starpilot_scene = fs.starpilot_scene;
  QJsonObject &starpilot_toggles = starpilot_scene.starpilot_toggles;
  const bool simple_mode = params.getBool("SimpleMode");

  stock_left_widget->setCurrentIndex(uiState()->prime_state->isSubscribed() ? 0 : 1);
  left_widget->setCurrentIndex(simple_mode ? 0 : 1);
  if (simple_mode) {
    custom_left_stack->setCurrentIndex(0);
    right_widget->setCurrentIndex(0);
  }

  date->setText(QLocale(uiState()->language.mid(5)).toString(QDateTime::currentDateTime(), "dddd, MMMM d"));
  date->setVisible(util::system_time_valid() && !simple_mode);

  if (simple_mode) {
    version->setText(getBrand() + " " + QString::fromStdString(params.get("UpdaterCurrentDescription")));
  } else {
    QString versionText = getVersion().left(14).trimmed();
    if (!versionText.startsWith("v", Qt::CaseInsensitive)) {
      versionText.prepend("v");
    }
    version->setText(getBrand() + " - " + versionText + " - " + cleanModelName(starpilot_toggles.value("model_name").toString()));
  }
}
