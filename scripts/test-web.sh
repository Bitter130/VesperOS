#!/usr/bin/env bash
set -e

echo "=== Installing QEMU, OVMF, noVNC & LLVM/LLD ==="
sudo apt-get update -y
sudo apt-get install -y qemu-system-x86 ovmf novnc websockify clang lld llvm mtools dosfstools

# Ensure lld-link symlink exists
if ! command -v lld-link &>/dev/null; then
    if command -v lld &>/dev/null; then
        sudo ln -sf "$(which lld)" /usr/local/bin/lld-link
    fi
fi

# Ensure bin directory exists
mkdir -p bin

# Check if image needs to be downloaded or compiled
if [ ! -f "bin/vesper-os.img" ]; then
    echo "=== Fetching latest prebuilt disk image ==="
    if curl -fSL --connect-timeout 10 -o bin/vesper-os.img https://github.com/Bitter130/VesperOS/releases/download/v0.1.0/vesper-os.img; then
        echo "Downloaded vesper-os.img from release v0.1.0 successfully!"
    else
        echo "Building vesper-os.img from source..."
        clang++ \
            --target=x86_64-unknown-windows \
            -nostdlib -ffreestanding -fshort-wchar -fno-stack-protector \
            -fno-exceptions -fno-rtti -fno-threadsafe-statics -mno-stack-arg-probe \
            -mno-red-zone -std=c++20 -O2 -Iinclude -fuse-ld=lld \
            -Wl,-subsystem:efi_application -Wl,-entry:efi_main -Wl,-nodefaultlib \
            src/main.cpp src/graphics/font.cpp src/graphics/framebuffer.cpp \
            src/drivers/display.cpp src/drivers/keyboard.cpp src/drivers/mouse.cpp \
            src/system/info.cpp src/system/runtime.cpp \
            -o bin/BOOTX64.EFI

        dd if=/dev/zero of=bin/vesper-os.img bs=1M count=64
        mkfs.vfat -F 32 bin/vesper-os.img
        mmd -i bin/vesper-os.img ::/EFI
        mmd -i bin/vesper-os.img ::/EFI/BOOT
        mcopy -i bin/vesper-os.img bin/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI
    fi
fi

# Find OVMF firmware path
OVMF_PATH=""
for p in /usr/share/OVMF/OVMF_CODE.fd /usr/share/ovmf/OVMF.fd /usr/share/edk2/ovmf/OVMF_CODE.fd; do
    if [ -f "$p" ]; then
        OVMF_PATH="$p"
        break
    fi
done

if [ -z "$OVMF_PATH" ]; then
    echo "[ERROR] Could not find OVMF UEFI firmware file."
    exit 1
fi

echo "=== Starting QEMU with UEFI (${OVMF_PATH}) ==="
qemu-system-x86_64 \
    -bios "$OVMF_PATH" \
    -m 512M \
    -drive file=bin/vesper-os.img,format=raw \
    -vnc :0 &

sleep 2

echo ""
echo "======================================================================"
echo ">>> In Codespaces, open the PORTS tab or click 'Open in Browser' <<<"
echo ">>> Forwarded Port: 6080                                           <<<"
echo "======================================================================"
echo ""

websockify --web /usr/share/novnc 6080 localhost:5900
