#ifndef PORTSNIFFER_H
#define PORTSNIFFER_H

#include "PortMonitor.h" // For PortInfo struct
#include <QMap>
#include <QObject>
#include <QProcess>
#include <QTimer>

class PortSniffer : public QObject {
  Q_OBJECT
public:
  explicit PortSniffer(QObject *parent = nullptr);
  void setTargetPort(int port);
  void start();
  void stop();
  bool isRunning() const { return m_running; }

signals:
  void connectionOpened(const PortInfo &info);
  void connectionClosed(const PortInfo &info);
  void stateChanged(const PortInfo &info, const QString &oldState);
  void errorOccurred(const QString &msg);

private slots:
  void onTimeout();

private:
  void parseOutput(const QByteArray &output);

  int m_targetPort = 0;
  bool m_running = false;
  QTimer *m_timer;
  QMap<QString, PortInfo> m_currentConnections; // Key: "RemoteIP:RemotePort"
};

#endif // PORTSNIFFER_H
