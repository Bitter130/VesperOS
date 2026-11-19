#include "uefi/uefi.hpp"
#include "graphics/framebuffer.hpp"
#include "drivers/display.hpp"
#include "drivers/keyboard.hpp"
#include "drivers/mouse.hpp"
#include "system/info.hpp"
#include "utils/str.hpp"

using namespace Vesper;

static void RenderStaticDashboard(const Drivers::DisplayInfo& disp, const System::HardwareInfo& sys) {
    auto& fb = Graphics::Framebuffer::Instance();

    // Dark sleek background
    fb.Clear(Graphics::Color::DarkSlate());

    // Top Header Banner
    fb.FillRect(0, 0, disp.Width, 50, Graphics::Color::PanelBg());
    fb.DrawRect(0, 0, disp.Width, 50, Graphics::Color::Border());

    fb.DrawString(24, 16, "VESPER OS", Graphics::Color::Cyan(), Graphics::Color::PanelBg());
    fb.DrawString(112, 16, "|  Bare-Metal x86_64 UEFI Microkernel", Graphics::Color::Gray(), Graphics::Color::PanelBg());

    // Left Panel: System & Hardware Telemetry
    UINT32 panelW = (disp.Width - 72) / 2;
    if (panelW < 360) panelW = 360;
    UINT32 panelH = 340;
    UINT32 panel1X = 24;
    UINT32 panel1Y = 70;

    fb.FillRect(panel1X, panel1Y, panelW, panelH, Graphics::Color::PanelBg());
    fb.DrawRect(panel1X, panel1Y, panelW, panelH, Graphics::Color::Border());

    fb.DrawString(panel1X + 16, panel1Y + 16, "[ SYSTEM & DISPLAY ]", Graphics::Color::Cyan(), Graphics::Color::PanelBg());

    char buf[64];

    // Screen Resolution
    fb.DrawString(panel1X + 16, panel1Y + 48, "Resolution:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    Utils::UIntToString(disp.Width, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 48, buf, Graphics::Color::White(), Graphics::Color::PanelBg());
    fb.DrawString(panel1X + 190, panel1Y + 48, "x", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    Utils::UIntToString(disp.Height, buf, sizeof(buf));
    fb.DrawString(panel1X + 205, panel1Y + 48, buf, Graphics::Color::White(), Graphics::Color::PanelBg());

    // Pixels per scanline
    fb.DrawString(panel1X + 16, panel1Y + 72, "Scanline Pitch:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    Utils::UIntToString(disp.PixelsPerScanLine, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 72, buf, Graphics::Color::White(), Graphics::Color::PanelBg());

    // Refresh Rate
    fb.DrawString(panel1X + 16, panel1Y + 96, "Refresh Rate:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    Utils::UIntToString(disp.RefreshRateHz, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 96, buf, Graphics::Color::Emerald(), Graphics::Color::PanelBg());
    fb.DrawString(panel1X + 180, panel1Y + 96, disp.EdidFound ? "Hz (EDID Native)" : "Hz (GOP Standard)", Graphics::Color::Gray(), Graphics::Color::PanelBg());

    // Monitor Name
    fb.DrawString(panel1X + 16, panel1Y + 120, "Display Device:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    fb.DrawString(panel1X + 150, panel1Y + 120, disp.MonitorName[0] ? disp.MonitorName : "Generic UEFI Display", Graphics::Color::White(), Graphics::Color::PanelBg());

    // Framebuffer Address
    fb.DrawString(panel1X + 16, panel1Y + 150, "Framebuffer Base:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    Utils::HexToString(disp.FrameBufferBase, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 150, buf, Graphics::Color::Amber(), Graphics::Color::PanelBg());

    // Firmware Vendor & Revision
    fb.DrawString(panel1X + 16, panel1Y + 180, "UEFI Firmware:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    fb.DrawString(panel1X + 150, panel1Y + 180, sys.FirmwareVendor, Graphics::Color::White(), Graphics::Color::PanelBg());

    fb.DrawString(panel1X + 16, panel1Y + 204, "Firmware Rev:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    Utils::HexToString(sys.FirmwareRevision, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 204, buf, Graphics::Color::White(), Graphics::Color::PanelBg());

    // Total RAM
    fb.DrawString(panel1X + 16, panel1Y + 234, "Physical RAM:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    uint64_t ramMB = sys.TotalRamBytes / (1024 * 1024);
    Utils::UIntToString(ramMB, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 234, buf, Graphics::Color::Emerald(), Graphics::Color::PanelBg());
    fb.DrawString(panel1X + 200, panel1Y + 234, "MB", Graphics::Color::Gray(), Graphics::Color::PanelBg());

    // Usable Conventional RAM
    fb.DrawString(panel1X + 16, panel1Y + 258, "Usable RAM:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
    uint64_t usableMB = sys.UsableRamBytes / (1024 * 1024);
    Utils::UIntToString(usableMB, buf, sizeof(buf));
    fb.DrawString(panel1X + 150, panel1Y + 258, buf, Graphics::Color::Emerald(), Graphics::Color::PanelBg());
    fb.DrawString(panel1X + 200, panel1Y + 258, "MB", Graphics::Color::Gray(), Graphics::Color::PanelBg());

    // Right Panel: Live Peripherals
    UINT32 panel2X = panel1X + panelW + 24;
    UINT32 panel2Y = panel1Y;

    fb.FillRect(panel2X, panel2Y, panelW, panelH, Graphics::Color::PanelBg());
    fb.DrawRect(panel2X, panel2Y, panelW, panelH, Graphics::Color::Border());

    fb.DrawString(panel2X + 16, panel2Y + 16, "[ LIVE PERIPHERALS & INPUT ]", Graphics::Color::Cyan(), Graphics::Color::PanelBg());

    // Footer info
    fb.FillRect(0, disp.Height - 32, disp.Width, 32, Graphics::Color::PanelBg());
    fb.DrawRect(0, disp.Height - 32, disp.Width, 32, Graphics::Color::Border());
    fb.DrawString(24, disp.Height - 24, "Booted natively from USB via \\EFI\\BOOT\\BOOTX64.EFI  --  Move mouse or press any key to test input", Graphics::Color::Gray(), Graphics::Color::PanelBg());
}

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    (void)imageHandle;

    if (!systemTable || !systemTable->BootServices) {
        return EFI_INVALID_PARAMETER;
    }

    // 1. Initialize Display & Framebuffer (GOP + EDID)
    EFI_STATUS status = Drivers::Display::Instance().Init(systemTable->BootServices);
    if (EFI_ERROR(status)) {
        return status;
    }

    const auto& dispInfo = Drivers::Display::Instance().GetInfo();

    // 2. Query System & Hardware Memory Information
    System::SystemInfo::Instance().Query(systemTable);
    const auto& sysInfo = System::SystemInfo::Instance().GetInfo();

    // 3. Initialize Keyboard and Mouse Drivers
    Drivers::Keyboard::Instance().Init(systemTable);
    Drivers::Mouse::Instance().Init(systemTable->BootServices, dispInfo.Width, dispInfo.Height);

    // 4. Render Static Dashboard
    RenderStaticDashboard(dispInfo, sysInfo);

    auto& fb = Graphics::Framebuffer::Instance();
    auto& kbd = Drivers::Keyboard::Instance();
    auto& mouse = Drivers::Mouse::Instance();

    UINT32 panelW = (dispInfo.Width - 72) / 2;
    if (panelW < 360) panelW = 360;
    UINT32 panel2X = 24 + panelW + 24;
    UINT32 panel2Y = 70;

    char buf[64];
    uint64_t keyPressCount = 0;
    Drivers::KeyEvent lastKey;
    Drivers::MouseState lastMouse = mouse.GetState();

    INT32 prevCursorX = lastMouse.X;
    INT32 prevCursorY = lastMouse.Y;

    // Initial cursor draw
    fb.DrawCursor((UINT32)lastMouse.X, (UINT32)lastMouse.Y);

    // Live Main Loop
    while (true) {
        bool needsUpdate = false;

        // Poll Keyboard
        Drivers::KeyEvent currentKey;
        if (kbd.PollKey(currentKey)) {
            lastKey = currentKey;
            keyPressCount++;
            needsUpdate = true;
        }

        // Poll Mouse
        Drivers::MouseState currentMouse;
        if (mouse.PollMouse(currentMouse)) {
            // Restore previous cursor area by redrawing or clearing cursor bounds
            // For simple, robust rendering, redraw cursor when position changed
            if (currentMouse.X != prevCursorX || currentMouse.Y != prevCursorY) {
                // Clear old cursor footprint (14x20)
                fb.FillRect((UINT32)prevCursorX, (UINT32)prevCursorY, 14, 20, Graphics::Color::DarkSlate());
                // Draw new cursor
                fb.DrawCursor((UINT32)currentMouse.X, (UINT32)currentMouse.Y);
                prevCursorX = currentMouse.X;
                prevCursorY = currentMouse.Y;
            }
            lastMouse = currentMouse;
            needsUpdate = true;
        }

        // Render dynamic peripheral data
        if (needsUpdate) {
            // Clear telemetry area inside Right Panel
            fb.FillRect(panel2X + 16, panel2Y + 48, panelW - 32, 270, Graphics::Color::PanelBg());

            // Keyboard Telemetry
            fb.DrawString(panel2X + 16, panel2Y + 48, "Keyboard Status:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            fb.DrawString(panel2X + 160, panel2Y + 48, "ACTIVE (SimpleTextInput)", Graphics::Color::Emerald(), Graphics::Color::PanelBg());

            fb.DrawString(panel2X + 16, panel2Y + 72, "Total Keystrokes:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            Utils::UIntToString(keyPressCount, buf, sizeof(buf));
            fb.DrawString(panel2X + 160, panel2Y + 72, buf, Graphics::Color::White(), Graphics::Color::PanelBg());

            fb.DrawString(panel2X + 16, panel2Y + 96, "Last Key Pressed:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            if (lastKey.HasKey) {
                fb.DrawString(panel2X + 160, panel2Y + 96, lastKey.Description, Graphics::Color::Cyan(), Graphics::Color::PanelBg());

                fb.DrawString(panel2X + 16, panel2Y + 120, "ScanCode / UTF16:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
                Utils::HexToString(lastKey.ScanCode, buf, sizeof(buf));
                fb.DrawString(panel2X + 160, panel2Y + 120, buf, Graphics::Color::Amber(), Graphics::Color::PanelBg());
                fb.DrawString(panel2X + 220, panel2Y + 120, "/", Graphics::Color::Gray(), Graphics::Color::PanelBg());
                Utils::HexToString(lastKey.UnicodeChar, buf, sizeof(buf));
                fb.DrawString(panel2X + 235, panel2Y + 120, buf, Graphics::Color::Amber(), Graphics::Color::PanelBg());
            } else {
                fb.DrawString(panel2X + 160, panel2Y + 96, "Waiting for key...", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            }

            // Mouse Telemetry
            fb.DrawString(panel2X + 16, panel2Y + 156, "Pointer Protocol:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            if (lastMouse.Present) {
                fb.DrawString(panel2X + 160, panel2Y + 156, "CONNECTED", Graphics::Color::Emerald(), Graphics::Color::PanelBg());
            } else {
                fb.DrawString(panel2X + 160, panel2Y + 156, "NOT DETECTED", Graphics::Color::Amber(), Graphics::Color::PanelBg());
            }

            fb.DrawString(panel2X + 16, panel2Y + 180, "Cursor Position:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            fb.DrawString(panel2X + 160, panel2Y + 180, "X:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            Utils::IntToString(lastMouse.X, buf, sizeof(buf));
            fb.DrawString(panel2X + 180, panel2Y + 180, buf, Graphics::Color::White(), Graphics::Color::PanelBg());
            fb.DrawString(panel2X + 230, panel2Y + 180, "Y:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            Utils::IntToString(lastMouse.Y, buf, sizeof(buf));
            fb.DrawString(panel2X + 250, panel2Y + 180, buf, Graphics::Color::White(), Graphics::Color::PanelBg());

            fb.DrawString(panel2X + 16, panel2Y + 204, "Delta Movement:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            Utils::IntToString(lastMouse.DeltaX, buf, sizeof(buf));
            fb.DrawString(panel2X + 160, panel2Y + 204, buf, Graphics::Color::White(), Graphics::Color::PanelBg());
            fb.DrawString(panel2X + 210, panel2Y + 204, ",", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            Utils::IntToString(lastMouse.DeltaY, buf, sizeof(buf));
            fb.DrawString(panel2X + 225, panel2Y + 204, buf, Graphics::Color::White(), Graphics::Color::PanelBg());

            fb.DrawString(panel2X + 16, panel2Y + 228, "Mouse Buttons:", Graphics::Color::Gray(), Graphics::Color::PanelBg());
            fb.DrawString(panel2X + 160, panel2Y + 228,
                lastMouse.LeftButton ? "[LEFT: CLICK]" : "[LEFT: UP]",
                lastMouse.LeftButton ? Graphics::Color::Emerald() : Graphics::Color::Gray(),
                Graphics::Color::PanelBg()
            );
            fb.DrawString(panel2X + 280, panel2Y + 228,
                lastMouse.RightButton ? "[RIGHT: CLICK]" : "[RIGHT: UP]",
                lastMouse.RightButton ? Graphics::Color::Emerald() : Graphics::Color::Gray(),
                Graphics::Color::PanelBg()
            );
        }

        // Delay 5ms (prevents burning 100% CPU in VM/bare-metal)
        systemTable->BootServices->Stall(5000);
    }

    return EFI_SUCCESS;
}
