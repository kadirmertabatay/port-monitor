#ifndef PORTSNIFFERWIDGET_H
#define PORTSNIFFERWIDGET_H

#include "PortSniffer.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

class PortSnifferWidget : public QWidget {
  Q_OBJECT
public:
  explicit PortSnifferWidget(QWidget *parent = nullptr);

private slots:
  void onToggleSniffing();
  void onSnifferError(const QString &msg);
  void onConnectionOpened(const PortInfo &info);
  void onConnectionClosed(const PortInfo &info);
  void onStateChanged(const PortInfo &info, const QString &oldState);
  void clearLogs();

private:
  void addLog(const QString &msg, const QString &color);

  QLineEdit *m_portInput;
  QPushButton *m_toggleBtn;
  QPushButton *m_clearBtn;
  QTableWidget *m_logTable;
  PortSniffer *m_sniffer;
  bool m_isSniffing = false;
};

#endif // PORTSNIFFERWIDGET_H
