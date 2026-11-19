#!/usr/bin/env bash
set -e

echo "=== Cleaning up any existing instances ==="
pkill -f qemu-system-x86_64 2>/dev/null || true
pkill -f websockify 2>/dev/null || true
rm -f /tmp/vnc.sock

echo "=== Installing Packages ==="
sudo apt-get update -y
sudo apt-get install -y qemu-system-x86 ovmf novnc websockify curl

# Make sure index.html exists in novnc directory so root URL / loads immediately
if [ -d "/usr/share/novnc" ]; then
    if [ -f "/usr/share/novnc/vnc.html" ]; then
        sudo cp -f /usr/share/novnc/vnc.html /usr/share/novnc/index.html
    fi
fi

# Ensure bin directory exists
mkdir -p bin

# Fetch prebuilt disk image from GitHub release v0.1.0
if [ ! -f "bin/vesper-os.img" ]; then
    echo "=== Fetching prebuilt v0.1.0 disk image ==="
    curl -fSL --connect-timeout 10 -o bin/vesper-os.img https://github.com/Bitter130/VesperOS/releases/download/v0.1.0/vesper-os.img
fi

# Locate OVMF firmware
OVMF_PATH=""
for p in /usr/share/OVMF/OVMF_CODE.fd /usr/share/ovmf/OVMF.fd /usr/share/edk2/ovmf/OVMF_CODE.fd; do
    if [ -f "$p" ]; then
        OVMF_PATH="$p"
        break
    fi
done

if [ -z "$OVMF_PATH" ]; then
    echo "[ERROR] Could not find OVMF firmware."
    exit 1
fi

echo "=== Starting QEMU with UEFI (Unix Socket) ==="
# Using Unix socket completely prevents VS Code from opening the broken 5900 port!
qemu-system-x86_64 \
    -bios "$OVMF_PATH" \
    -m 512M \
    -drive file=bin/vesper-os.img,format=raw \
    -vnc unix:/tmp/vnc.sock &

# Wait for socket to be created
for i in {1..10}; do
    if [ -S /tmp/vnc.sock ]; then
        break
    fi
    sleep 0.5
done

echo ""
echo "=========================================================================="
echo "  VesperOS is RUNNING! "
echo "  Opening Web Interface on Port 6080..."
echo "=========================================================================="
echo ""

NOVNC_DIR="/usr/share/novnc"
websockify --web "$NOVNC_DIR" --unix-target=/tmp/vnc.sock 6080
