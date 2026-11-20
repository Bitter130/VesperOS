#include "uefi/uefi.hpp"
#include "graphics/framebuffer.hpp"
#include "drivers/display.hpp"
#include "drivers/keyboard.hpp"
#include "drivers/mouse.hpp"
#include "system/info.hpp"
#include "ui/window_manager.hpp"
#include "ui/terminal.hpp"
#include "ui/sysinfo_window.hpp"

using namespace Vesper;

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    (void)imageHandle;

    if (!systemTable || !systemTable->BootServices) {
        return EFI_INVALID_PARAMETER;
    }

    // 1. Initialize Display & Framebuffer with Double Buffering in RAM
    EFI_STATUS status = Drivers::Display::Instance().Init(systemTable->BootServices);
    if (EFI_ERROR(status)) {
        return status;
    }

    const auto& dispInfo = Drivers::Display::Instance().GetInfo();

    // 2. Query System & Hardware Memory Information
    System::SystemInfo::Instance().Query(systemTable);

    // 3. Initialize Keyboard and Mouse Drivers
    Drivers::Keyboard::Instance().Init(systemTable);
    Drivers::Mouse::Instance().Init(systemTable->BootServices, dispInfo.Width, dispInfo.Height);

    auto& fb = Graphics::Framebuffer::Instance();
    auto& kbd = Drivers::Keyboard::Instance();
    auto& mouse = Drivers::Mouse::Instance();
    auto& wm = UI::WindowManager::Instance();

    // 4. Initialize Window Manager
    wm.Init(dispInfo.Width, dispInfo.Height);

    // 5. Create Default Windows (Terminal + System Monitor)
    UI::TerminalWindow terminal(1, 40, 50, 500, 310, systemTable);
    
    INT32 sysInfoX = (INT32)dispInfo.Width - 440;
    if (sysInfoX < 400) sysInfoX = 400;
    UI::SysInfoWindow sysInfoWin(2, sysInfoX, 50, 400, 260);

    wm.AddWindow(&sysInfoWin);
    wm.AddWindow(&terminal); // Terminal on top by default

    uint64_t tickCounter = 0;
    uint64_t uptimeSeconds = 0;

    // 6. 60 FPS Desktop Compositor Main Loop
    while (true) {
        // Poll Keyboard Input
        Drivers::KeyEvent currentKey;
        if (kbd.PollKey(currentKey)) {
            wm.HandleKey(currentKey);
        }

        // Poll Mouse Input (Handles both AbsoluteTablet & SimpleRelative)
        Drivers::MouseState currentMouse;
        mouse.PollMouse(currentMouse);
        wm.HandleMouse(currentMouse);

        // Track approximate uptime seconds (every ~200 iterations of 5ms = 1 sec)
        tickCounter++;
        if (tickCounter >= 200) {
            uptimeSeconds++;
            tickCounter = 0;
        }

        // 1. Render all windows and taskbar into the offscreen RAM backbuffer
        wm.Render(fb, uptimeSeconds);

        // 2. Draw the mouse cursor on top of the backbuffer
        fb.DrawCursor(currentMouse.X, currentMouse.Y);

        // 3. Blit the entire frame to the physical display in one pass (flicker-free!)
        fb.SwapBuffers();

        // 5ms delay (~60 FPS target without burning 100% CPU bus)
        systemTable->BootServices->Stall(5000);
    }

    return EFI_SUCCESS;
}
