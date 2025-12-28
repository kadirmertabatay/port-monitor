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

#include "PortMonitor.h"
#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QVBoxLayout>

class ProcessDetailsDialog : public QDialog {
  Q_OBJECT

public:
  explicit ProcessDetailsDialog(const PortInfo &info,
                                QWidget *parent = nullptr);

private slots:
  void onTestConnectionClicked();

private:
  void setupUi();
  QWidget *createDetailRow(const QString &label, const QString &value);
  QString getCommandLine(const QString &pid);

  PortInfo m_info;
  QTextEdit *m_cmdArgsText;
  QLabel *m_connectionStatusLabel;
  QPushButton *m_testConnBtn;
};
