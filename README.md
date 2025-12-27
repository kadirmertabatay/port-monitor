# Port Monitor

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Qt](https://img.shields.io/badge/Qt-6.2+-41CD52.svg?logo=qt&logoColor=white)](https://www.qt.io/)
[![C++](https://img.shields.io/badge/C++-17-00599C.svg?logo=c%2B%2B&logoColor=white)](https://isocpp.org/)

**Port Monitor** is a professional desktop utility built with C++ and Qt to provide real-time visibility into active network ports and connections on macOS and Linux systems.

---

## Key Features

- **Real-time Monitoring**: Instant detection of active IPv4 and IPv6 ports using optimized `lsof` calls.
- **Process Tracing**: Map every active port to its parent process name and PID.
- **Advanced Filtering**: Lightning-fast search across process names, PIDs, protocols, and port numbers.
- **Smart Refresh**: Configurable auto-refresh (5s default) or manual tactical scanning.
- **Premium Dark Theme**: High-contrast, glassmorphism-inspired UI with color-coded connection states (Listening, Established).

---

## Quick Start

### Prerequisites

Ensure you have the following installed:

- **Qt 6.2+** (Gui, Widgets, Network modules)
- **CMake 3.16+**
- **C++17 Compiler** (Clang/GCC)
- **macOS** or **Linux**

### Native Build (Recommended)

```bash
# Clone the repository
git clone <repository-url>
cd port-monitor

# Configure and Build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Launch Application
./build/PortMonitor
```

---

## Docker Deployment

For sandboxed builds or Linux server environments:

### Build Image

```bash
docker build -t port-monitor .
```

### Run Container (macOS with XQuartz)

1. **Allow X11 Access**: `xhost +localhost`
2. **Run Image**:

```bash
docker run -it --rm \
    -e DISPLAY=host.docker.internal:0 \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --network host \
    port-monitor
```

### Simplified Launch (macOS)

We provide a helper script that automates XQuartz startup and X11 permissions:

```bash
./run_docker.sh
```

> [!IMPORTANT]  
> Due to Docker Desktop's VM layer on macOS, `--network host` may limit port scanning to the container's namespace. Native execution is recommended for full host system visibility.

---

## Project Architecture

```text
port-monitor/
├── src/
│   ├── main.cpp            # App entry & theme initialization
│   ├── MainWindow.cpp/h    # Controller & Main UI logic
│   ├── PortMonitor.cpp/h   # Backend scanner (lsof bridge)
│   └── PortTableModel.cpp/h # High-performance table model
├── resources/
│   ├── styles.qss          # Modern dark-mode stylesheet
│   └── resources.qrc       # Qt Resource System
├── CMakeLists.txt          # Modern CMake build system
└── Dockerfile              # Containerized build environment
```

---

## Troubleshooting

- **No Ports Detected**: Verify system permissions. Some high-level ports may require elevated privileges. Run with `sudo ./build/PortMonitor` for full visibility.
- **Display Error (Docker)**: Ensure XQuartz is running on macOS and "Allow connections from network clients" is checked in Preferences.

---

## License

Distributed under the **Apache License 2.0**. See `LICENSE` for more information.

Copyright © 2025 **Kadir Mert Abatay**. All rights reserved.
