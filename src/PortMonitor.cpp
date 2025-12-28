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

#include "PortMonitor.h"
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

PortMonitor::PortMonitor(QObject *parent) : QObject(parent) {}

void PortMonitor::refresh() {
  QProcess *process = new QProcess(this);
  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
          [this, process](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitStatus == QProcess::NormalExit && exitCode == 0) {
              parseLsofOutput(process->readAllStandardOutput());
            } else {
              emit errorOccurred(process->readAllStandardError());
            }
            process->deleteLater();
          });

  // Command to list Internet files, no host names, no service names
  process->start("lsof", QStringList() << "-i" << "-P" << "-n");
}

void PortMonitor::killProcess(qint64 pid) {
  qDebug() << "Attempting to kill process:" << pid;
  QString program = "kill";
  QStringList arguments;
  arguments << "-9" << QString::number(pid);

  QProcess *process = new QProcess(this);
  connect(
      process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
      [this, pid, process](int exitCode, QProcess::ExitStatus exitStatus) {
        bool success = (exitStatus == QProcess::NormalExit && exitCode == 0);
        QString message =
            success ? "Process killed successfully"
                    : QString::fromUtf8(process->readAllStandardError());
        emit processKilled(pid, success, message);
        process->deleteLater();
        // Trigger a refresh after killing
        if (success) {
          refresh();
        }
      });
  process->start(program, arguments);
}

void PortMonitor::parseLsofOutput(const QByteArray &output) {
  QList<PortInfo> ports;
  QMap<QString, PortInfo> currentPorts;

  QString data = QString::fromUtf8(output);
  QStringList lines = data.split('\n', Qt::SkipEmptyParts);

  if (!lines.isEmpty() && lines.first().startsWith("COMMAND")) {
    lines.removeFirst();
  }

  for (const QString &line : lines) {
    QStringList parts =
        line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 9) {
      continue;
    }

    PortInfo info;
    info.processName = parts[0];
    info.pid = parts[1];
    info.user = parts[2];

    // Protocol is usually in the NODE column (index 7) or TYPE column
    QString type = parts[4];
    QString node = parts[7];
    info.protocol =
        node.contains("TCP") ? "TCP" : (node.contains("UDP") ? "UDP" : type);

    // NAME field can be multiple parts if state is present (e.g. "address:port
    // (LISTEN)")
    QString nameField;
    for (int i = 8; i < parts.size(); ++i) {
      nameField += parts[i] + " ";
    }
    nameField = nameField.trimmed();

    // 1. Extract State
    QRegularExpression stateRegex("\\(([^)]+)\\)$");
    QRegularExpressionMatch match = stateRegex.match(nameField);
    if (match.hasMatch()) {
      info.state = match.captured(1);
      nameField.remove(match.capturedStart(0), match.capturedLength(0));
      nameField = nameField.trimmed();
    } else {
      info.state = (info.protocol == "UDP") ? "NONE" : "ESTABLISHED";
    }

    // 2. Handle Connection Arrows (e.g. 127.0.0.1:3000->127.0.0.1:54321)
    QString localSegment = nameField;
    if (nameField.contains("->")) {
      localSegment = nameField.split("->").first();
      if (info.state == "NONE")
        info.state = "ESTABLISHED";
    }

    // 3. Extract Port and Address from Local Segment
    int lastColon = localSegment.lastIndexOf(':');
    info.port = (lastColon != -1) ? localSegment.mid(lastColon + 1).toInt() : 0;
    info.localAddress =
        (lastColon != -1) ? localSegment.left(lastColon) : localSegment;

    ports.append(info);

    // Track unique listeners
    if (info.state == "LISTEN") {
      QString key = QString("%1:%2").arg(info.protocol).arg(info.port);
      currentPorts.insert(key, info);

      // Check if new
      if (!m_knownPorts.contains(key)) {
        emit newPortDetected(info);
      }
    }
  }

  // Check for closed ports (in known but not in current)
  for (const QString &key : m_knownPorts.keys()) {
    if (!currentPorts.contains(key)) {
      emit portClosed(m_knownPorts.value(key));
    }
  }

  m_knownPorts = currentPorts;
  emit portsUpdated(ports);
}
