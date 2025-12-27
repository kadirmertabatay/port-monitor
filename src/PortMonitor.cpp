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

void PortMonitor::parseLsofOutput(const QByteArray &output) {
  QList<PortInfo> ports;
  QString data = QString::fromUtf8(output);
  QStringList lines = data.split('\n', Qt::SkipEmptyParts);

  // Skip header line if present (COMMAND PID USER FD TYPE DEVICE SIZE/OFF NODE
  // NAME)
  if (!lines.isEmpty() && lines.first().startsWith("COMMAND")) {
    lines.removeFirst();
  }

  for (const QString &line : lines) {
    // Simplified parsing logic
    QStringList parts =
        line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 9)
      continue;

    PortInfo info;
    info.processName = parts[0];
    info.pid = parts[1];
    info.protocol = parts[4];

    // NAME column contains the address and port.
    QString nameField = parts.last();
    // Check for state in parens at end
    QRegularExpression stateRegex("\\(([^)]+)\\)$"); // Matches (LISTEN)
    QRegularExpressionMatch match = stateRegex.match(nameField);
    if (match.hasMatch()) {
      info.state = match.captured(1);
      nameField.replace(stateRegex, "");
      nameField = nameField.trimmed();
    } else {
      info.state = "ESTABLISHED/NONE";
    }

    // nameField is now like "*:52970" or "127.0.0.1:80->..."
    int lastColon = nameField.lastIndexOf(':');
    if (lastColon != -1) {
      info.localAddress = nameField.left(lastColon);
      QString portStr = nameField.mid(lastColon + 1);
      int arrow = portStr.indexOf("->");
      if (arrow != -1) {
        portStr = portStr.left(arrow);
        info.state = "ESTABLISHED";
      }
      info.port = portStr.toInt();
    }

    ports.append(info);
  }

  emit portsUpdated(ports);
}
