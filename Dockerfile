FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    qt6-base-dev \
    qt6-tools-dev \
    libgl1-mesa-dev \
    lsof \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source code
COPY . .

# Build the application
RUN cmake -S . -B build && cmake --build build

# Default command
CMD ["./build/PortMonitor"]
