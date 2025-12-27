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

#include "ProcessDetailsDialog.h"
#include <QApplication>
#include <QClipboard>
#include <QFrame>
#include <QHBoxLayout>
#include <QPushButton>

ProcessDetailsDialog::ProcessDetailsDialog(const PortInfo &info,
                                           QWidget *parent)
    : QDialog(parent) {
  setWindowTitle("Process Details");
  setMinimumWidth(400);
  setupUi(info);
}

void ProcessDetailsDialog::setupUi(const PortInfo &info) {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(15);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  // Header Section
  QLabel *titleLabel = new QLabel(info.processName, this);
  titleLabel->setStyleSheet(
      "font-size: 20px; font-weight: bold; color: #4dc2fc;");
  titleLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(titleLabel);

  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setStyleSheet("background-color: #555;");
  mainLayout->addWidget(line);

  // Details Section
  QFrame *detailsFrame = new QFrame(this);
  detailsFrame->setStyleSheet("background-color: #2b2b2b; border-radius: 8px;");
  QVBoxLayout *detailsLayout = new QVBoxLayout(detailsFrame);

  detailsLayout->addWidget(createDetailRow("PID", info.pid));
  detailsLayout->addWidget(createDetailRow("User", info.user));
  detailsLayout->addWidget(createDetailRow("State", info.state));
  detailsLayout->addWidget(createDetailRow("Protocol", info.protocol));
  detailsLayout->addWidget(createDetailRow("Local Address", info.localAddress));
  detailsLayout->addWidget(createDetailRow("Port", QString::number(info.port)));

  mainLayout->addWidget(detailsFrame);

  // Buttons Section
  QHBoxLayout *btnLayout = new QHBoxLayout();

  QPushButton *copyBtn = new QPushButton("Copy Info", this);
  copyBtn->setCursor(Qt::PointingHandCursor);
  connect(copyBtn, &QPushButton::clicked, this, [info]() {
    QClipboard *clipboard = QApplication::clipboard();
    QString text = QString("Process: %1\nPID: %2\nUser: %3\nPort: %4")
                       .arg(info.processName)
                       .arg(info.pid)
                       .arg(info.user)
                       .arg(info.port);
    clipboard->setText(text);
  });

  QPushButton *closeBtn = new QPushButton("Close", this);
  closeBtn->setCursor(Qt::PointingHandCursor);
  connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

  btnLayout->addStretch();
  btnLayout->addWidget(copyBtn);
  btnLayout->addWidget(closeBtn);

  mainLayout->addLayout(btnLayout);
}

QWidget *ProcessDetailsDialog::createDetailRow(const QString &label,
                                               const QString &value) {
  QWidget *rowWidget = new QWidget(this);
  QHBoxLayout *layout = new QHBoxLayout(rowWidget);
  layout->setContentsMargins(10, 5, 10, 5);

  QLabel *lbl = new QLabel(label + ":", rowWidget);
  lbl->setStyleSheet("color: #888; font-weight: bold;");
  lbl->setFixedWidth(100);

  QLabel *val = new QLabel(value, rowWidget);
  val->setStyleSheet("color: #ddd;");
  val->setWordWrap(true);
  val->setTextInteractionFlags(Qt::TextSelectableByMouse);

  layout->addWidget(lbl);
  layout->addWidget(val);

  return rowWidget;
}
