#include "PortSniffer.h"
#include <QDebug>
#include <QRegularExpression>

PortSniffer::PortSniffer(QObject *parent) : QObject(parent) {
  m_timer = new QTimer(this);
  connect(m_timer, &QTimer::timeout, this, &PortSniffer::onTimeout);
}

void PortSniffer::setTargetPort(int port) { m_targetPort = port; }

void PortSniffer::start() {
  if (m_targetPort <= 0) {
    emit errorOccurred("Invalid port number");
    return;
  }
  m_currentConnections.clear();
  m_running = true;
  m_timer->start(1000); // Poll every 1 second
  onTimeout();          // Immediate run
}

void PortSniffer::stop() {
  m_running = false;
  m_timer->stop();
}

void PortSniffer::onTimeout() {
  if (!m_running)
    return;

  QProcess process;
  QString program = "lsof";
  QStringList arguments;
  // -i :<port> -> select by port
  // -n -> no host names
  // -P -> no port names
  arguments << "-i" << QString(":%1").arg(m_targetPort) << "-n" << "-P";

  process.start(program, arguments);
  if (!process.waitForFinished(2000)) {
    emit errorOccurred("lsof command timed out");
    return;
  }

  if (process.exitStatus() == QProcess::CrashExit) {
    emit errorOccurred("lsof crashed");
    return;
  }

  // lsof returns 1 if no files are found (empty output), which is valid for us
  parseOutput(process.readAllStandardOutput());
}

void PortSniffer::parseOutput(const QByteArray &output) {
  QMap<QString, PortInfo> scanResults;
  QString data = QString::fromUtf8(output);
  QStringList lines = data.split('\n', Qt::SkipEmptyParts);

  if (!lines.isEmpty() && lines.first().startsWith("COMMAND")) {
    lines.removeFirst();
  }

  for (const QString &line : lines) {
    QStringList parts =
        line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    if (parts.size() < 9)
      continue;

    PortInfo info;
    info.processName = parts[0];
    info.pid = parts[1];
    info.user = parts[2];
    QString protocol = parts[7].contains("TCP")
                           ? "TCP"
                           : (parts[7].contains("UDP") ? "UDP" : parts[4]);
    info.protocol = protocol;

    // NAME field logic
    QString nameField;
    for (int i = 8; i < parts.size(); ++i) {
      nameField += parts[i] + " ";
    }
    nameField = nameField.trimmed();

    QRegularExpression stateRegex("\\(([^)]+)\\)$");
    QRegularExpressionMatch match = stateRegex.match(nameField);
    if (match.hasMatch()) {
      info.state = match.captured(1);
      nameField.remove(match.capturedStart(0), match.capturedLength(0));
      nameField = nameField.trimmed();
    } else {
      info.state = (info.protocol == "UDP") ? "NONE" : "ESTABLISHED";
    }

    // Identify Remote Address for Unique Key
    // Standard format: Local->Remote
    QString remoteAddr = "Listener";
    QString localAddr = nameField;

    if (nameField.contains("->")) {
      QStringList addrParts = nameField.split("->");
      localAddr = addrParts[0];
      if (addrParts.size() > 1) {
        remoteAddr = addrParts[1];
      }
    }

    // If it's the listener itself (LISTEN), we might treat it differently or
    // generally But for "Sniffer", we care about connections *to* this port. If
    // state is LISTEN, it's the server socket.

    // Key strategy: Use PID + RemoteAddress as unique key for a connection
    // For LISTEN socket: Remote is usually "*" or empty in some contexts, here
    // we use "Listener"
    QString key = QString("%1|%2").arg(info.pid).arg(remoteAddr);

    // Store exact addresses in PortInfo for display
    info.localAddress = localAddr;
    info.port = m_targetPort; // We know the port
    // We can hijack 'localAddress' storage or add a 'remoteAddress' field to
    // PortInfo? For now, let's put "Remote: <addr>" in localAddress or just use
    // it as description. Actually, PortInfo struct is simple. Let's just use
    // what we have. The UI will display what we send.

    if (remoteAddr != "Listener") {
      info.localAddress = QString("%1 -> %2").arg(localAddr).arg(remoteAddr);
    } else {
      info.localAddress = localAddr; // Just the bind address
    }

    scanResults.insert(key, info);

    if (!m_currentConnections.contains(key)) {
      emit connectionOpened(info);
    } else {
      // Check for state change
      PortInfo oldInfo = m_currentConnections.value(key);
      if (oldInfo.state != info.state) {
        emit stateChanged(info, oldInfo.state);
      }
    }
  }

  // Check for closed connections
  for (const QString &key : m_currentConnections.keys()) {
    if (!scanResults.contains(key)) {
      emit connectionClosed(m_currentConnections.value(key));
    }
  }

  m_currentConnections = scanResults;
}
