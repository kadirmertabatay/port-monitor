#!/bin/bash

# Port Monitor Docker Runner for macOS
# This script handles XQuartz setup and X11 permissions automatically.

echo "🚀 Preparing Port Monitor Docker environment..."

# 1. Check if XQuartz is installed
if [ ! -d "/Applications/Utilities/XQuartz.app" ] && [ ! -d "/Applications/XQuartz.app" ]; then
    echo "❌ Error: XQuartz is not installed. Please run: brew install --cask xquartz"
    exit 1
fi

# 2. Configure XQuartz settings (Network & GLX)
UPDATED_SETTINGS=0
if [ "$(defaults read org.xquartz.X11 nolisten_tcp 2>/dev/null)" != "0" ]; then
    defaults write org.xquartz.X11 nolisten_tcp 0
    UPDATED_SETTINGS=1
fi
if [ "$(defaults read org.xquartz.X11 enable_iglx 2>/dev/null)" != "1" ]; then
    defaults write org.xquartz.X11 enable_iglx -bool true
    UPDATED_SETTINGS=1
fi

if [ $UPDATED_SETTINGS -eq 1 ]; then
    echo "🔧 XQuartz settings optimized. If GUI fails, please RESTART XQuartz manually."
fi

# 3. Start XQuartz if not running
if ! pgrep -x "XQuartz" > /dev/null; then
    echo "⌚ Starting XQuartz..."
    open -a XQuartz
    sleep 3
fi

# 4. Detect Local IP for DISPLAY
IP_ADDR=$(ifconfig en0 | grep 'inet ' | awk '{print $2}')
if [ -z "$IP_ADDR" ]; then
    IP_ADDR=$(ifconfig en1 | grep 'inet ' | awk '{print $2}')
fi

# 5. Configure xhost permissions
export DISPLAY=:0
XHOST_BIN="/opt/X11/bin/xhost"
if [ -f "$XHOST_BIN" ]; then
    "$XHOST_BIN" +localhost > /dev/null 2>&1
    if [ ! -z "$IP_ADDR" ]; then
        "$XHOST_BIN" +$IP_ADDR > /dev/null 2>&1
    fi
    "$XHOST_BIN" +host.docker.internal > /dev/null 2>&1
else
    echo "❌ Error: xhost not found. Check XQuartz installation."
    exit 1
fi

# 6. Rebuild if needed (since we updated Dockerfile)
echo "🏗️  Ensuring Docker image is up to date..."
docker build -t port-monitor .

# 7. Run the container
echo "🚢 Launching Port Monitor..."
echo "💡 Hint: Search XQuartz Preferences -> Security -> 'Allow connections' if it still fails."

# Use IP_ADDR if found, otherwise fallback to host.docker.internal
TARGET_DISPLAY="host.docker.internal:0"
if [ ! -z "$IP_ADDR" ]; then
    TARGET_DISPLAY="$IP_ADDR:0"
fi

docker run -it --rm \
    -e DISPLAY="$TARGET_DISPLAY" \
    -e LIBGL_ALWAYS_SOFTWARE=1 \
    -e QT_X11_NO_MITSHM=1 \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    --network host \
    port-monitor
