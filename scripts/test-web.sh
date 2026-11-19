#!/usr/bin/env bash
set -e

echo "=== Installing QEMU, OVMF & noVNC ==="
sudo apt-get update -y
sudo apt-get install -y qemu-system-x86 ovmf novnc websockify

# Build disk image if missing
if [ ! -f "bin/vesper-os.img" ]; then
    echo "=== Building disk image ==="
    mkdir -p bin
    if [ ! -f "bin/BOOTX64.EFI" ]; then
        echo "Compiling BOOTX64.EFI..."
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
    fi
    sudo apt-get install -y mtools dosfstools
    dd if=/dev/zero of=bin/vesper-os.img bs=1M count=64
    mkfs.vfat -F 32 bin/vesper-os.img
    mmd -i bin/vesper-os.img ::/EFI
    mmd -i bin/vesper-os.img ::/EFI/BOOT
    mcopy -i bin/vesper-os.img bin/BOOTX64.EFI ::/EFI/BOOT/BOOTX64.EFI
fi

echo "=== Starting QEMU with UEFI (VNC on :0) ==="
qemu-system-x86_64 \
    -bios /usr/share/OVMF/OVMF_CODE.fd \
    -m 512M \
    -drive file=bin/vesper-os.img,format=raw \
    -vnc :0 &

sleep 2

echo "=== Starting Web VNC (noVNC on port 6080) ==="
echo ""
echo ">>> In VS Code / Codespaces, look at the Ports tab or click 'Open in Browser' for port 6080! <<<"
echo ""

websockify --web /usr/share/novnc 6080 localhost:5900
