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

#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>

struct PortInfo {
  QString protocol;
  QString localAddress;
  QString state;
  QString pid;
  QString processName;
  QString user;
  int port;
};

class PortMonitor : public QObject {
  Q_OBJECT

public:
  explicit PortMonitor(QObject *parent = nullptr);
  void refresh();
  Q_INVOKABLE void killProcess(qint64 pid);

signals:
  void portsUpdated(const QList<PortInfo> &ports);
  void newPortDetected(const PortInfo &port);
  void portClosed(const PortInfo &port);
  void errorOccurred(const QString &error);
  void processKilled(qint64 pid, bool success, const QString &message);

private:
  void parseLsofOutput(const QByteArray &output);
  QMap<QString, PortInfo> m_knownPorts;
};
