#!/usr/bin/env bash
set -e

echo "=== Cleaning up any existing instances ==="
pkill -f qemu-system-x86_64 2>/dev/null || true
pkill -f websockify 2>/dev/null || true
rm -f /tmp/vnc.sock

echo "=== Ensuring Packages ==="
sudo apt-get update -y
sudo apt-get install -y qemu-system-x86 ovmf novnc websockify clang lld llvm mtools dosfstools curl

# Ensure lld-link symlink exists
if ! command -v lld-link &>/dev/null; then
    if command -v lld &>/dev/null; then
        sudo ln -sf "$(which lld)" /usr/local/bin/lld-link
    fi
fi

# Ensure index.html exists for noVNC
if [ -d "/usr/share/novnc" ]; then
    sudo ln -sf /usr/share/novnc/vnc.html /usr/share/novnc/index.html 2>/dev/null || true
fi

# Ensure bin directory exists
mkdir -p bin

echo "=== Compiling VesperOS UEFI Binary ==="
clang++ \
    --target=x86_64-unknown-windows \
    -nostdlib -ffreestanding -fshort-wchar -fno-stack-protector \
    -fno-exceptions -fno-rtti -fno-threadsafe-statics -mno-stack-arg-probe \
    -mno-red-zone -std=c++20 -O2 -Iinclude -fuse-ld=lld \
    -Wl,-subsystem:efi_application -Wl,-entry:efi_main -Wl,-nodefaultlib \
    src/main.cpp src/graphics/font.cpp src/graphics/framebuffer.cpp \
    src/drivers/display.cpp src/drivers/keyboard.cpp src/drivers/mouse.cpp \
    src/system/info.cpp src/system/runtime.cpp \
    src/ui/window.cpp src/ui/window_manager.cpp \
    src/ui/terminal.cpp src/ui/sysinfo_window.cpp \
    -o bin/BOOTX64.EFI

echo "=== Packaging Bootable FAT32 Disk Image ==="
dd if=/dev/zero of=bin/vesper-os.img bs=1M count=64 status=none
mkfs.vfat -F 32 bin/vesper-os.img >/dev/null
mmd -i bin/vesper-os.img ::/EFI
mmd -i bin/vesper-os.img ::/EFI/BOOT
mcopy -i bin/vesper-os.img bin/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI

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

echo "=== Starting QEMU with UEFI, USB Tablet & Keyboard ==="
qemu-system-x86_64 \
    -bios "$OVMF_PATH" \
    -m 512M \
    -device usb-ehci \
    -device usb-tablet \
    -device usb-kbd \
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
echo "  Web interface is listening on port 6080."
echo "=========================================================================="
echo ""

NOVNC_DIR="/usr/share/novnc"
websockify --web "$NOVNC_DIR" --unix-target=/tmp/vnc.sock 6080
