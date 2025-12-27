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

#include "PortTableModel.h"
#include <QBrush>
#include <QColor>

PortTableModel::PortTableModel(QObject *parent) : QAbstractTableModel(parent) {}

int PortTableModel::rowCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return m_ports.size();
}

int PortTableModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid())
    return 0;
  return ColumnCount;
}

QVariant PortTableModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= m_ports.size())
    return QVariant();

  const PortInfo &info = m_ports[index.row()];

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case ProcessName:
      return info.processName;
    case PID:
      return info.pid;
    case Protocol:
      return info.protocol;
    case LocalAddress:
      return info.localAddress;
    case Port:
      return info.port;
    case State:
      return info.state;
    }
  } else if (role == Qt::ForegroundRole) {
    if (info.state == "LISTEN") {
      return QBrush(QColor("#4dc2fc")); // Light blue for listening
    } else if (info.state == "ESTABLISHED") {
      return QBrush(QColor("#81c784")); // Green for established
    }
  } else if (role == Qt::TextAlignmentRole) {
    if (index.column() == Port || index.column() == PID)
      return int(Qt::AlignRight | Qt::AlignVCenter);
    return int(Qt::AlignLeft | Qt::AlignVCenter);
  }

  return QVariant();
}

QVariant PortTableModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
  if (role != Qt::DisplayRole || orientation != Qt::Vertical) {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
      switch (section) {
      case ProcessName:
        return "Process";
      case PID:
        return "PID";
      case Protocol:
        return "Protocol";
      case LocalAddress:
        return "Address";
      case Port:
        return "Port";
      case State:
        return "State";
      }
    }
  }
  return QVariant();
}

void PortTableModel::setPorts(const QList<PortInfo> &ports) {
  beginResetModel();
  m_ports = ports;
  endResetModel();
}

void PortTableModel::clear() {
  beginResetModel();
  m_ports.clear();
  endResetModel();
}
