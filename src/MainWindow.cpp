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
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupUi();

  m_model = new PortTableModel(this);
  m_portTable->setModel(m_model);

  m_portMonitor = new PortMonitor(this);
  connect(m_portMonitor, &PortMonitor::portsUpdated, this,
          &MainWindow::onPortsUpdated);

  // Auto-refresh every 5 seconds
  m_refreshTimer = new QTimer(this);
  connect(m_refreshTimer, &QTimer::timeout, this,
          &MainWindow::onRefreshClicked);
  m_refreshTimer->start(5000);

  // Initial refresh
  onRefreshClicked();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUi() {
  QWidget *centralWidget = new QWidget(this);
  setCentralWidget(centralWidget);

  QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(10, 10, 10, 10);
  mainLayout->setSpacing(10);

  // Top Bar
  QHBoxLayout *topLayout = new QHBoxLayout();

  m_searchBox = new QLineEdit(this);
  m_searchBox->setPlaceholderText("Filter ports, processes...");
  connect(m_searchBox, &QLineEdit::textChanged, this,
          &MainWindow::onFilterTextChanged);

  m_refreshBtn = new QPushButton("Refresh", this);
  m_refreshBtn->setCursor(Qt::PointingHandCursor);
  connect(m_refreshBtn, &QPushButton::clicked, this,
          &MainWindow::onRefreshClicked);

  topLayout->addWidget(m_searchBox, 1);
  topLayout->addWidget(m_refreshBtn);

  mainLayout->addLayout(topLayout);

  // Table
  m_portTable = new QTableView(this);
  m_portTable->setAlternatingRowColors(true);
  m_portTable->horizontalHeader()->setStretchLastSection(true);
  m_portTable->verticalHeader()->setVisible(false);
  m_portTable->setShowGrid(false);
  m_portTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_portTable->setSelectionMode(QAbstractItemView::SingleSelection);

  mainLayout->addWidget(m_portTable);

  // Status Bar
  statusBar()->showMessage("Ready");
}

void MainWindow::onRefreshClicked() {
  statusBar()->showMessage("Refreshing...");
  m_portMonitor->refresh();
}

void MainWindow::onPortsUpdated(const QList<PortInfo> &ports) {
  m_allPorts = ports;

  // Apply functionality of filtering if text exists
  onFilterTextChanged(m_searchBox->text());

  statusBar()->showMessage(
      QString("Found %1 active connections").arg(ports.size()));
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
