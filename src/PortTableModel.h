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
#include <QAbstractTableModel>

class PortTableModel : public QAbstractTableModel {
  Q_OBJECT

public:
  explicit PortTableModel(QObject *parent = nullptr);

  enum Column {
    ProcessName = 0,
    PID,
    User,
    Protocol,
    LocalAddress,
    Port,
    State,
    Action,
    ColumnCount
  };

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  void setPorts(const QList<PortInfo> &ports);
  void clear();

private:
  QList<PortInfo> m_ports;
};
