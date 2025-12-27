cmake_minimum_required(VERSION 3.16)

project(PortMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PROJECT_SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/PortMonitor.cpp
    src/PortMonitor.h
    src/PortTableModel.cpp
    src/PortTableModel.h
    resources/resources.qrc
)

add_executable(PortMonitor ${PROJECT_SOURCES})

target_link_libraries(PortMonitor PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

# Set icon if we had one, for now skip.
