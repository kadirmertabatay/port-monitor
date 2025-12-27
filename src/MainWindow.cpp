/*
 * Copyright 2025 Kadir Mert Abatay
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MainWindow.h"
#include "ProcessDetailsDialog.h"
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowIcon(QIcon(":/icon.png"));
  setupUi();

  m_model = new PortTableModel(this);
  m_portTable->setModel(m_model);
  connect(
      m_portTable, &QTableView::clicked, this,
      [this](const QModelIndex &index) {
        if (index.column() == PortTableModel::Action) {
          int port =
              m_model->data(m_model->index(index.row(), PortTableModel::Port))
                  .toInt();
          QString state =
              m_model->data(m_model->index(index.row(), PortTableModel::State))
                  .toString();
          if (state == "LISTEN" && port > 0) {
            QDesktopServices::openUrl(
                QUrl(QString("http://localhost:%1").arg(port)));
          }
        }
      });

  m_portMonitor = new PortMonitor(this);
  connect(m_portMonitor, &PortMonitor::portsUpdated, this,
          &MainWindow::onPortsUpdated);
  connect(m_portMonitor, &PortMonitor::newPortDetected, this,
          [this](const PortInfo &info) {
            if (m_trayIcon && m_trayIcon->isVisible()) {
              QString msg = QString("Process: %1\nPort: %2 (%3)")
                                .arg(info.processName)
                                .arg(info.port)
                                .arg(info.protocol);
              m_trayIcon->showMessage("New Listener Detected", msg,
                                      QSystemTrayIcon::Information, 3000);
            }
          });

  m_refreshTimer = new QTimer(this);
  connect(m_refreshTimer, &QTimer::timeout, this,
          &MainWindow::onRefreshClicked);
  m_refreshTimer->start(5000);

  m_refreshTimer->start(5000);

  createTrayIcon();

  createTrayIcon();

  onRefreshClicked();
}

MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (m_trayIcon->isVisible()) {
    QMessageBox::information(this, "Port Monitor",
                             "The application will keep running in the system "
                             "tray. To terminate the program, choose 'Quit' in "
                             "the context menu of the system tray entry.");
    hide();
    event->ignore();
  }
}

void MainWindow::createTrayIcon() {
  m_trayMenu = new QMenu(this);
  QAction *restoreAction =
      m_trayMenu->addAction("Restore", this, &QWidget::showNormal);
  QAction *quitAction =
      m_trayMenu->addAction("Quit", qApp, &QApplication::quit);

  m_trayIcon = new QSystemTrayIcon(this);
  m_trayIcon->setContextMenu(m_trayMenu);

  // Use the app icon if available, or a standard one
  QIcon icon = QIcon(":/icon.png");
  if (icon.isNull()) {
    icon = QIcon::fromTheme("network-wired"); // Fallback
  }
  m_trayIcon->setIcon(icon);

  connect(m_trayIcon, &QSystemTrayIcon::activated, this,
          &MainWindow::onTrayIconActivated);
  m_trayIcon->show();
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
  if (reason == QSystemTrayIcon::Trigger ||
      reason == QSystemTrayIcon::DoubleClick) {
    if (isVisible()) {
      hide();
    } else {
      showNormal();
      activateWindow();
    }
  }
}

void MainWindow::setupUi() {
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(15, 15, 15, 15);
  mainLayout->setSpacing(15);

  // --- Dashboard Section ---
  QLabel *dashTitle = new QLabel("Developer Port Dashboard", this);
  dashTitle->setStyleSheet(
      "font-size: 18px; font-weight: bold; color: #3daee9;");
  mainLayout->addWidget(dashTitle);

  QFrame *dashFrame = new QFrame(this);
  dashFrame->setObjectName("dashFrame");
  dashFrame->setStyleSheet("#dashFrame { background-color: #333333; "
                           "border-radius: 8px; padding: 10px; }");
  m_dashboardLayout = new QGridLayout(dashFrame);
  m_dashboardLayout->setSpacing(10);

  setupDashboard();
  mainLayout->addWidget(dashFrame);

  // --- Control Section ---
  QHBoxLayout *topLayout = new QHBoxLayout();
  m_searchBox = new QLineEdit(this);
  m_searchBox->setPlaceholderText("Search processes or ports...");
  connect(m_searchBox, &QLineEdit::textChanged, this,
          &MainWindow::onFilterTextChanged);

  m_refreshBtn = new QPushButton("Refresh Now", this);
  m_refreshBtn->setCursor(Qt::PointingHandCursor);
  connect(m_refreshBtn, &QPushButton::clicked, this,
          &MainWindow::onRefreshClicked);

  topLayout->addWidget(m_searchBox, 1);
  topLayout->addWidget(m_refreshBtn);
  mainLayout->addLayout(topLayout);

  // --- Table Section ---
  m_portTable = new QTableView(this);
  m_portTable->setAlternatingRowColors(true);
  m_portTable->horizontalHeader()->setStretchLastSection(true);
  m_portTable->verticalHeader()->setVisible(false);
  m_portTable->setShowGrid(false);
  m_portTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_portTable->setSelectionMode(QAbstractItemView::SingleSelection);
  m_portTable->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_portTable, &QTableView::customContextMenuRequested, this,
          &MainWindow::onCustomContextMenuRequested);
  connect(m_portTable, &QTableView::doubleClicked, this,
          &MainWindow::showProcessDetails);

  // Set column widths
  m_portTable->horizontalHeader()->setSectionResizeMode(
      PortTableModel::ProcessName, QHeaderView::Stretch);
  m_portTable->horizontalHeader()->setSectionResizeMode(
      PortTableModel::Port, QHeaderView::ResizeToContents);
  m_portTable->horizontalHeader()->setSectionResizeMode(PortTableModel::Action,
                                                        QHeaderView::Fixed);
  m_portTable->horizontalHeader()->resizeSection(PortTableModel::Action, 100);

  mainLayout->addWidget(m_portTable);

  statusBar()->showMessage("System Ready");
}

void MainWindow::setupDashboard() {
  // Define tracked ports
  struct PortDef {
    int port;
    QString name;
    QString desc;
  };
  QList<PortDef> defs = {{3000, "React / Node", "Default for web dev"},
                         {5000, "Flask / AirPlay", "Python or Control Center"},
                         {5432, "PostgreSQL", "Database Service"},
                         {6379, "Redis", "Memory Cache"},
                         {8000, "Django / Dev", "Common Dev Port"},
                         {8080, "HTTP / Java", "Alternative Web Port"},
                         {27017, "MongoDB", "NoSQL Database"},
                         {9000, "PHP / API", "FastCGI / Port 9000"}};

  int col = 0;
  int row = 0;
  for (const auto &def : defs) {
    QWidget *container = new QWidget(this);
    container->setProperty("class", "dashboardCard");
    QVBoxLayout *layout = new QVBoxLayout(container);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(4);

    QLabel *nameLabel =
        new QLabel(QString("%1 (%2)").arg(def.name).arg(def.port), this);
    nameLabel->setObjectName("dashPortName");

    QLabel *statusLabel = new QLabel("OFFLINE", this);
    statusLabel->setObjectName("dashStatusLabel");
    statusLabel->setStyleSheet("color: #888888;");
    statusLabel->setToolTip(def.desc);

    QPushButton *openBtn = new QPushButton("Launch", container);
    openBtn->setObjectName("dashOpenBtn");
    openBtn->setCursor(Qt::PointingHandCursor);
    openBtn->setVisible(false);
    connect(openBtn, &QPushButton::clicked, this, [this, def]() {
      QDesktopServices::openUrl(
          QUrl(QString("http://localhost:%1").arg(def.port)));
    });

    layout->addWidget(nameLabel);
    layout->addWidget(statusLabel);
    layout->addStretch();
    layout->addWidget(openBtn);

    m_dashboardLayout->addWidget(container, row, col);

    m_trackedPorts.append(
        {def.port, def.name, def.desc, statusLabel, container, openBtn});

    col++;
    if (col > 3) {
      col = 0;
      row++;
    }
  }
}

void MainWindow::updateDashboard(const QList<PortInfo> &ports) {
  qDebug() << "Updating dashboard with" << ports.size() << "ports";
  for (auto &tracked : m_trackedPorts) {
    bool found = false;
    QString process;
    for (const auto &p : ports) {
      if (p.port == tracked.port) {
        qDebug() << "Matched port" << tracked.port << "with process"
                 << p.processName;
        found = true;
        process = p.processName;
        break;
      }
    }

    if (found) {
      tracked.label->setText(QString("● ONLINE (%1)").arg(process));
      tracked.label->setStyleSheet("color: #81c784; font-weight: bold;");
      tracked.openButton->setVisible(true);
      tracked.container->setProperty("online", true);
    } else {
      tracked.label->setText("○ OFFLINE");
      tracked.label->setStyleSheet("color: #666666;");
      tracked.openButton->setVisible(false);
      tracked.container->setProperty("online", false);
    }
    // Refresh style
    tracked.container->style()->unpolish(tracked.container);
    tracked.container->style()->polish(tracked.container);
  }
}

void MainWindow::onRefreshClicked() {
  statusBar()->showMessage("Scanning ports...");
  m_portMonitor->refresh();
}

void MainWindow::onPortsUpdated(const QList<PortInfo> &ports) {
  m_allPorts = ports;
  updateDashboard(ports);
  onFilterTextChanged(m_searchBox->text());
  statusBar()->showMessage(QString("Active connections: %1").arg(ports.size()));
}

void MainWindow::onFilterTextChanged(const QString &text) {
  if (text.isEmpty()) {
    m_model->setPorts(m_allPorts);
    return;
  }
  QList<PortInfo> filtered;
  for (const PortInfo &info : m_allPorts) {
    if (info.processName.contains(text, Qt::CaseInsensitive) ||
        info.pid.contains(text, Qt::CaseInsensitive) ||
        QString::number(info.port).contains(text) ||
        info.protocol.contains(text, Qt::CaseInsensitive)) {
      filtered.append(info);
    }
  }
  m_model->setPorts(filtered);
}

void MainWindow::onCustomContextMenuRequested(const QPoint &pos) {
  QModelIndex index = m_portTable->indexAt(pos);
  if (!index.isValid())
    return;

  QMenu contextMenu(this);
  contextMenu.addAction(QIcon::fromTheme("application-exit"), "End Process",
                        this, &MainWindow::onKillProcessRequested);
  contextMenu.addAction(
      QIcon::fromTheme("edit-copy"), "Copy PID", this, [this, index]() {
        int row = index.row();
        QString pid =
            m_model->data(m_model->index(row, PortTableModel::PID)).toString();
        QApplication::clipboard()->setText(pid);
      });
  contextMenu.addSeparator();
  contextMenu.addAction(QIcon::fromTheme("dialog-information"), "View Details",
                        this, &MainWindow::showProcessDetails);

  contextMenu.exec(m_portTable->viewport()->mapToGlobal(pos));
}

void MainWindow::onKillProcessRequested() {
  QModelIndexList selection = m_portTable->selectionModel()->selectedRows();
  if (selection.isEmpty())
    return;

  int row = selection.first().row();
  QString pidStr =
      m_model->data(m_model->index(row, PortTableModel::PID)).toString();
  QString name = m_model->data(m_model->index(row, PortTableModel::ProcessName))
                     .toString();
  bool ok;
  qint64 pid = pidStr.toLongLong(&ok);

  if (ok) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
        this, "End Process",
        QString("Are you sure you want to end process '%1' (PID: %2)?")
            .arg(name)
            .arg(pid),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      connect(m_portMonitor, &PortMonitor::processKilled, this,
              [this](qint64, bool success, const QString &msg) {
                if (!success) {
                  QMessageBox::critical(this, "Error",
                                        "Failed to kill process: " + msg);
                } else {
                  statusBar()->showMessage("Process killed successfully.");
                  onRefreshClicked();
                }
                // Disconnect to avoid multiple slots overlapping on subsequent
                // calls
                disconnect(m_portMonitor, &PortMonitor::processKilled, this,
                           nullptr);
              });
      m_portMonitor->killProcess(pid);
    }
  }
}

void MainWindow::showProcessDetails() {
  QModelIndexList selection = m_portTable->selectionModel()->selectedRows();
  if (selection.isEmpty())
    return;

  int row = selection.first().row();

  PortInfo info;
  info.processName =
      m_model->data(m_model->index(row, PortTableModel::ProcessName))
          .toString();
  info.pid = m_model->data(m_model->index(row, PortTableModel::PID)).toString();
  info.user =
      m_model->data(m_model->index(row, PortTableModel::User)).toString();
  info.protocol =
      m_model->data(m_model->index(row, PortTableModel::Protocol)).toString();
  info.localAddress =
      m_model->data(m_model->index(row, PortTableModel::LocalAddress))
          .toString();
  info.port = m_model->data(m_model->index(row, PortTableModel::Port)).toInt();
  info.state =
      m_model->data(m_model->index(row, PortTableModel::State)).toString();

  ProcessDetailsDialog dialog(info, this);
  dialog.exec();
}
