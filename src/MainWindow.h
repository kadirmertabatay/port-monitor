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

#pragma once

#include "FlowLayout.h"
#include "PortMonitor.h"
#include "PortTableModel.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>
#include <QSpinBox>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QTableView>
#include <QTableWidget>
#include <QTimer>

struct PortDef {
  int port;
  QString name;
  QString desc;
};

struct PortStatus {
  int port;
  QString name;
  QString description;
  QLabel *label;
  QWidget *container;
  QPushButton *openButton;
  QPushButton *deleteButton;
  bool isCustom;
};

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void onRefreshClicked();
  void onPortsUpdated(const QList<PortInfo> &ports);
  void onFilterTextChanged(const QString &text);
  void onCustomContextMenuRequested(const QPoint &pos);
  void onKillProcessRequested();
  void showProcessDetails();
  void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
  void addLogEntry(const QString &event, const PortInfo &info);

  // Settings Slots
  void saveSettings();
  void loadSettings();
  void saveCustomPorts();
  void loadCustomPorts();

  // Log Slots
  void filterActivityLog();

  // Custom Port Slot
  void onAddPortClicked();

private:
  void setupUi();
  void setupSettingsTab(QWidget *parent);
  void setupDashboard();
  void createTrayIcon();
  void updateTrayMenu();
  void updateDashboard(const QList<PortInfo> &ports);
  bool isDarkTheme();

  QTabWidget *m_tabWidget;
  QTableWidget *m_logTable;

  // Log Filter Widgets
  QLineEdit *m_logSearchBox;
  QComboBox *m_logFilterCombo;

  QTableView *m_portTable;
  QLineEdit *m_searchBox;
  QPushButton *m_refreshBtn;
  QSystemTrayIcon *m_trayIcon;
  QMenu *m_trayMenu;
  FlowLayout *m_dashboardLayout;
  QList<PortStatus> m_trackedPorts;

  PortMonitor *m_portMonitor;
  PortTableModel *m_model;
  QTimer *m_refreshTimer;
  QList<PortInfo> m_allPorts;
  QList<PortDef> m_customPorts;

  // Settings Widgets
  QCheckBox *m_notificationsCheck;
  QComboBox *m_themeCombo;
  QCheckBox *m_autoStartCheck;
};
