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
#include <QElapsedTimer>
#include <QFrame>
#include <QHBoxLayout>
#include <QProcess>
#include <QPushButton>
#include <QTcpSocket>

ProcessDetailsDialog::ProcessDetailsDialog(const PortInfo &info,
                                           QWidget *parent)
    : QDialog(parent), m_info(info) {
  setWindowTitle("Process Details");
  setMinimumWidth(500);
  setupUi();
}

void ProcessDetailsDialog::setupUi() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setSpacing(15);
  mainLayout->setContentsMargins(20, 20, 20, 20);

  // Header Section
  QLabel *titleLabel = new QLabel(m_info.processName, this);
  titleLabel->setStyleSheet(
      "font-size: 20px; font-weight: bold; color: #4dc2fc;");
  titleLabel->setAlignment(Qt::AlignCenter);
  mainLayout->addWidget(titleLabel);

  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setStyleSheet("background-color: #555;");
  mainLayout->addWidget(line);

  // Details Section (Table)
  QFrame *detailsFrame = new QFrame(this);
  detailsFrame->setStyleSheet("background-color: #2b2b2b; border-radius: 8px;");
  QVBoxLayout *detailsLayout = new QVBoxLayout(detailsFrame);

  detailsLayout->addWidget(createDetailRow("PID", m_info.pid));
  detailsLayout->addWidget(createDetailRow("User", m_info.user));
  detailsLayout->addWidget(createDetailRow("State", m_info.state));
  detailsLayout->addWidget(createDetailRow("Protocol", m_info.protocol));
  detailsLayout->addWidget(
      createDetailRow("Local Address", m_info.localAddress));
  detailsLayout->addWidget(
      createDetailRow("Port", QString::number(m_info.port)));

  mainLayout->addWidget(detailsFrame);

  // --- Command Line Section ---
  QLabel *cmdLabel = new QLabel("Command Line:", this);
  cmdLabel->setStyleSheet("font-weight: bold; color: #ddd; margin-top: 10px;");
  mainLayout->addWidget(cmdLabel);

  m_cmdArgsText = new QTextEdit(this);
  m_cmdArgsText->setReadOnly(true);
  m_cmdArgsText->setFixedHeight(60);
  m_cmdArgsText->setStyleSheet(
      "background-color: #1e1e1e; color: #aaa; border: 1px solid #444; "
      "border-radius: 4px; padding: 5px;");
  m_cmdArgsText->setText(getCommandLine(m_info.pid));
  mainLayout->addWidget(m_cmdArgsText);

  // --- Connection Test Section ---
  QFrame *testFrame = new QFrame(this);
  testFrame->setStyleSheet(
      "background-color: #2b2b2b; border-radius: 8px; padding: 5px;");
  QHBoxLayout *testLayout = new QHBoxLayout(testFrame);
  testLayout->setContentsMargins(10, 5, 10, 5);

  m_testConnBtn = new QPushButton("Test Connection", this);
  m_testConnBtn->setCursor(Qt::PointingHandCursor);
  connect(m_testConnBtn, &QPushButton::clicked, this,
          &ProcessDetailsDialog::onTestConnectionClicked);
  testLayout->addWidget(m_testConnBtn);

  m_connectionStatusLabel = new QLabel("Ready to test", this);
  m_connectionStatusLabel->setStyleSheet("color: #888;");
  testLayout->addWidget(m_connectionStatusLabel);
  testLayout->addStretch();

  mainLayout->addWidget(testFrame);

  // Buttons Section (Bottom)
  QHBoxLayout *btnLayout = new QHBoxLayout();

  QPushButton *copyBtn = new QPushButton("Copy Info", this);
  copyBtn->setCursor(Qt::PointingHandCursor);
  connect(copyBtn, &QPushButton::clicked, this, [this]() {
    QClipboard *clipboard = QApplication::clipboard();
    QString text = QString("Process: %1\nPID: %2\nUser: %3\nPort: %4\nCmd: %5")
                       .arg(m_info.processName)
                       .arg(m_info.pid)
                       .arg(m_info.user)
                       .arg(m_info.port)
                       .arg(m_cmdArgsText->toPlainText());
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

QString ProcessDetailsDialog::getCommandLine(const QString &pid) {
  QProcess p;
  // 'ps -p PID -o args=' gets the arguments without header
  p.start("ps", QStringList() << "-p" << pid << "-o" << "args=");
  if (p.waitForFinished(1000)) {
    QString output = p.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
      return output;
    }
  }
  return "Unavailable";
}

void ProcessDetailsDialog::onTestConnectionClicked() {
  m_connectionStatusLabel->setText("Connecting...");
  m_connectionStatusLabel->setStyleSheet("color: #e6e600;"); // Yellow
  m_testConnBtn->setEnabled(false);

  QTcpSocket *socket = new QTcpSocket(this);
  QElapsedTimer *timer = new QElapsedTimer();
  timer->start();

  // Handle potential wildcards or empty addresses by defaulting to localhost
  QString targetAddr = m_info.localAddress;
  if (targetAddr == "*" || targetAddr == "0.0.0.0" || targetAddr == "[::]" ||
      targetAddr.isEmpty()) {
    targetAddr = "127.0.0.1";
  }

  // Connect signals properly
  connect(socket, &QTcpSocket::connected, this, [this, socket, timer]() {
    qint64 elapsed = timer->elapsed();
    m_connectionStatusLabel->setText(
        QString("Success! Latency: %1 ms").arg(elapsed));
    m_connectionStatusLabel->setStyleSheet(
        "color: #81c784; font-weight: bold;"); // Green
    socket->disconnectFromHost();
    m_testConnBtn->setEnabled(true);
    socket->deleteLater();
    delete timer;
  });

  connect(socket, &QTcpSocket::errorOccurred, this,
          [this, socket, timer](QAbstractSocket::SocketError) {
            m_connectionStatusLabel->setText("Connection Failed");
            m_connectionStatusLabel->setStyleSheet(
                "color: #e74c3c; font-weight: bold;"); // Red
            m_testConnBtn->setEnabled(true);
            socket->deleteLater();
            delete timer;
          });

  socket->connectToHost(targetAddr, m_info.port);
}
