#include "PortSnifferWidget.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QVBoxLayout>

PortSnifferWidget::PortSnifferWidget(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *layout = new QVBoxLayout(this);

  // Control Panel
  QHBoxLayout *controlsLog = new QHBoxLayout();

  QLabel *label = new QLabel("Monitor Port:", this);
  label->setStyleSheet("font-weight: bold; font-size: 14px;");

  m_portInput = new QLineEdit(this);
  m_portInput->setPlaceholderText("e.g. 3000");
  m_portInput->setValidator(new QIntValidator(1, 65535, this));
  m_portInput->setFixedWidth(100);

  m_toggleBtn = new QPushButton("Start Monitoring", this);
  m_toggleBtn->setCheckable(true);
  m_toggleBtn->setStyleSheet("background-color: #28a745; color: white; "
                             "font-weight: bold; padding: 6px;");
  connect(m_toggleBtn, &QPushButton::clicked, this,
          &PortSnifferWidget::onToggleSniffing);

  m_clearBtn = new QPushButton("Clear Logs", this);
  connect(m_clearBtn, &QPushButton::clicked, this,
          &PortSnifferWidget::clearLogs);

  controlsLog->addWidget(label);
  controlsLog->addWidget(m_portInput);
  controlsLog->addWidget(m_toggleBtn);
  controlsLog->addStretch();
  controlsLog->addWidget(m_clearBtn);

  // Log Table
  m_logTable = new QTableWidget(this);
  m_logTable->setColumnCount(3);
  m_logTable->setHorizontalHeaderLabels({"Time", "Event", "Details"});
  m_logTable->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents);
  m_logTable->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::ResizeToContents);
  m_logTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
  m_logTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_logTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_logTable->setStyleSheet("QTableWidget { background-color: #1e1e1e; color: "
                            "#dcdcdc; gridline-color: #333; }"
                            "QHeaderView::section { background-color: #2d2d2d; "
                            "color: white; padding: 4px; border: none; }");

  layout->addLayout(controlsLog);
  layout->addWidget(m_logTable);

  // Backend
  m_sniffer = new PortSniffer(this);
  connect(m_sniffer, &PortSniffer::errorOccurred, this,
          &PortSnifferWidget::onSnifferError);
  connect(m_sniffer, &PortSniffer::connectionOpened, this,
          &PortSnifferWidget::onConnectionOpened);
  connect(m_sniffer, &PortSniffer::connectionClosed, this,
          &PortSnifferWidget::onConnectionClosed);
  connect(m_sniffer, &PortSniffer::stateChanged, this,
          &PortSnifferWidget::onStateChanged);
}

void PortSnifferWidget::onToggleSniffing() {
  if (m_toggleBtn->isChecked()) {
    QString portStr = m_portInput->text();
    if (portStr.isEmpty()) {
      QMessageBox::warning(this, "Input Error", "Please enter a port number.");
      m_toggleBtn->setChecked(false);
      return;
    }

    int port = portStr.toInt();
    m_sniffer->setTargetPort(port);
    m_sniffer->start();

    m_isSniffing = true;
    m_toggleBtn->setText("Stop Monitoring");
    m_toggleBtn->setStyleSheet("background-color: #dc3545; color: white; "
                               "font-weight: bold; padding: 6px;");
    m_portInput->setEnabled(false);
    addLog("Monitoring Started", "#4dc2fc");
  } else {
    m_sniffer->stop();
    m_isSniffing = false;
    m_toggleBtn->setText("Start Monitoring");
    m_toggleBtn->setStyleSheet("background-color: #28a745; color: white; "
                               "font-weight: bold; padding: 6px;");
    m_portInput->setEnabled(true);
    addLog("Monitoring Stopped", "#ffa500");
  }
}

void PortSnifferWidget::onSnifferError(const QString &msg) {
  addLog("Error: " + msg, "#ff4444");
  // Stop if error is severe? For now just log.
}

void PortSnifferWidget::onConnectionOpened(const PortInfo &info) {
  QString details = QString("%1 (%2) - %3")
                        .arg(info.processName)
                        .arg(info.pid)
                        .arg(info.localAddress);
  addLog("New Connection", "#00ff00");

  // Update last row details
  int row = 0;
  m_logTable->item(row, 2)->setText(details);
}

void PortSnifferWidget::onConnectionClosed(const PortInfo &info) {
  QString details =
      QString("Process %1 (%2) closed").arg(info.processName).arg(info.pid);
  addLog("Connection Closed", "#ff4444");
  int row = 0;
  m_logTable->item(row, 2)->setText(details);
}

void PortSnifferWidget::onStateChanged(const PortInfo &info,
                                       const QString &oldState) {
  QString details = QString("%1 -> %2").arg(oldState).arg(info.state);
  addLog("State Changed", "#ffff00");
  int row = 0;
  m_logTable->item(row, 2)->setText(details);
}

void PortSnifferWidget::addLog(const QString &msg, const QString &color) {
  int row = 0;
  m_logTable->insertRow(row);

  QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");

  QTableWidgetItem *timeItem = new QTableWidgetItem(timestamp);
  QTableWidgetItem *msgItem = new QTableWidgetItem(msg);
  QTableWidgetItem *detailsItem = new QTableWidgetItem("");

  msgItem->setForeground(QBrush(QColor(color)));

  m_logTable->setItem(row, 0, timeItem);
  m_logTable->setItem(row, 1, msgItem);
  m_logTable->setItem(row, 2, detailsItem);

  m_logTable->scrollToTop();
}

void PortSnifferWidget::clearLogs() { m_logTable->setRowCount(0); }
