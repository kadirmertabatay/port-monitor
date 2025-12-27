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

#include "MainWindow.h"
#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QSplashScreen>
#include <QTimer>

int main(int argc, char *argv[]) {
  // Ensure we can see the tray icon on some systems
  QApplication::setQuitOnLastWindowClosed(false);

  QApplication app(argc, argv);
  app.setApplicationName("Port Monitor");
  app.setOrganizationName("KadirMertAbatay");
  app.setWindowIcon(QIcon(":/icon.png"));

  // Load Stylesheet
  QFile styleFile(":/styles.qss");
  if (styleFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    app.setStyleSheet(styleFile.readAll());
  }

  // Create Splash Screen
  QPixmap splashPix(450, 280);
  splashPix.fill(Qt::transparent);

  QPainter painter(&splashPix);
  painter.setRenderHint(QPainter::Antialiasing);

  // Background
  QLinearGradient gradient(0, 0, 450, 280);
  gradient.setColorAt(0, QColor(45, 45, 45));
  gradient.setColorAt(1, QColor(25, 25, 25));

  QPainterPath path;
  path.addRoundedRect(0, 0, 450, 280, 15, 15);
  painter.fillPath(path, gradient);

  // App Title
  painter.setPen(QColor(61, 174, 233));
  painter.setFont(QFont("Arial", 28, QFont::Bold));
  painter.drawText(QRect(0, 70, 450, 50), Qt::AlignCenter, "Port Monitor");

  // Subtitle
  painter.setPen(QColor(200, 200, 200));
  painter.setFont(QFont("Arial", 14));
  painter.drawText(QRect(0, 130, 450, 30), Qt::AlignCenter,
                   "Advanced Network Tool");

  // Loading...
  painter.setPen(QColor(120, 120, 120));
  painter.setFont(QFont("Arial", 10));
  painter.drawText(QRect(0, 230, 450, 30), Qt::AlignCenter,
                   "Initializing System...");

  QSplashScreen splash(splashPix);
  splash.show();
  app.processEvents();

  MainWindow window;
  window.resize(1000, 700);

  // Delay showing main window to let splash be seen
  QTimer::singleShot(2000, [&]() {
    splash.finish(&window);
    window.show();
  });

  return app.exec();
}
