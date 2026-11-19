#include "uefi/uefi.hpp"
#include "graphics/framebuffer.hpp"
#include "drivers/display.hpp"
#include "drivers/keyboard.hpp"
#include "drivers/mouse.hpp"
#include "system/info.hpp"
#include "utils/str.hpp"

using namespace Vesper;

static void RenderHeader(const Drivers::DisplayInfo& disp) {
    auto& fb = Graphics::Framebuffer::Instance();

    // Top Header Bar
    fb.FillRect(0, 0, disp.Width, 42, Graphics::Color::Panel());
    fb.DrawHLine(0, 42, disp.Width, Graphics::Color::Border());

    fb.DrawString(20, 13, "VESPER MICROKERNEL", Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawString(180, 13, "//", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    fb.DrawString(205, 13, "RELEASE v0.1.0", Graphics::Color::Accent(), Graphics::Color::Panel());
    fb.DrawString(330, 13, "//", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    fb.DrawString(355, 13, "ARCH: x86_64", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    fb.DrawString(465, 13, "//", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    fb.DrawString(490, 13, "STATUS: FREESTANDING ACTIVE", Graphics::Color::Success(), Graphics::Color::Panel());
}

static void RenderFooter(const Drivers::DisplayInfo& disp) {
    auto& fb = Graphics::Framebuffer::Instance();

    // Bottom Status Line
    UINT32 footerY = disp.Height - 30;
    fb.FillRect(0, footerY, disp.Width, 30, Graphics::Color::Panel());
    fb.DrawHLine(0, footerY, disp.Width, Graphics::Color::Border());

    fb.DrawString(20, footerY + 7, "VESPER READY  |  Move pointer or press keys to test live bus telemetry", Graphics::Color::TextMuted(), Graphics::Color::Panel());
}

static void RenderPanels(const Drivers::DisplayInfo& disp, const System::HardwareInfo& sys) {
    auto& fb = Graphics::Framebuffer::Instance();

    // Clean background
    fb.Clear(Graphics::Color::Bg());

    RenderHeader(disp);
    RenderFooter(disp);

    UINT32 panelW = (disp.Width - 60) / 2;
    if (panelW < 360) panelW = 360;
    UINT32 panelH = 340;
    UINT32 panel1X = 20;
    UINT32 panel1Y = 62;

    // Left Panel: Architecture & System
    fb.FillRect(panel1X, panel1Y, panelW, panelH, Graphics::Color::Panel());
    fb.DrawRect(panel1X, panel1Y, panelW, panelH, Graphics::Color::Border());

    fb.DrawString(panel1X + 16, panel1Y + 16, "SYSTEM & HARDWARE TELEMETRY", Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawHLine(panel1X + 16, panel1Y + 38, panelW - 32, Graphics::Color::Border());

    char buf[64];

    // Screen Resolution
    fb.DrawString(panel1X + 16, panel1Y + 54, "Display Resolution:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    Utils::UIntToString(disp.Width, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 54, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 215, panel1Y + 54, "x", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    Utils::UIntToString(disp.Height, buf, sizeof(buf));
    fb.DrawString(panel1X + 230, panel1Y + 54, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());

    // Scanline pitch
    fb.DrawString(panel1X + 16, panel1Y + 78, "Scanline Stride:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    Utils::UIntToString(disp.PixelsPerScanLine, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 78, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 225, panel1Y + 78, "px", Graphics::Color::TextMuted(), Graphics::Color::Panel());

    // Refresh rate
    fb.DrawString(panel1X + 16, panel1Y + 102, "Vertical Refresh:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    Utils::UIntToString(disp.RefreshRateHz, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 102, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 205, panel1Y + 102, disp.EdidFound ? "Hz (EDID Timing)" : "Hz (GOP Nominal)", Graphics::Color::TextMuted(), Graphics::Color::Panel());

    // Display Monitor Name
    fb.DrawString(panel1X + 16, panel1Y + 126, "Monitor Identification:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 175, panel1Y + 126, disp.MonitorName[0] ? disp.MonitorName : "Generic UEFI Framebuffer", Graphics::Color::TextWhite(), Graphics::Color::Panel());

    // Framebuffer Address
    fb.DrawString(panel1X + 16, panel1Y + 154, "Framebuffer Base:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    Utils::HexToString(disp.FrameBufferBase, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 154, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());

    // Firmware Vendor
    fb.DrawString(panel1X + 16, panel1Y + 182, "UEFI Firmware:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 175, panel1Y + 182, sys.FirmwareVendor, Graphics::Color::TextWhite(), Graphics::Color::Panel());

    // Firmware Revision
    fb.DrawString(panel1X + 16, panel1Y + 206, "Firmware Revision:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    Utils::HexToString(sys.FirmwareRevision, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 206, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());

    // Total RAM
    fb.DrawString(panel1X + 16, panel1Y + 234, "Physical Memory:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    uint64_t ramMB = sys.TotalRamBytes / (1024 * 1024);
    Utils::UIntToString(ramMB, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 234, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 225, panel1Y + 234, "MB Total", Graphics::Color::TextMuted(), Graphics::Color::Panel());

    // Usable RAM
    fb.DrawString(panel1X + 16, panel1Y + 258, "Conventional Memory:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
    uint64_t usableMB = sys.UsableRamBytes / (1024 * 1024);
    Utils::UIntToString(usableMB, buf, sizeof(buf));
    fb.DrawString(panel1X + 175, panel1Y + 258, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawString(panel1X + 225, panel1Y + 258, "MB Available", Graphics::Color::TextMuted(), Graphics::Color::Panel());

    // Right Panel: Hardware Peripherals
    UINT32 panel2X = panel1X + panelW + 20;
    UINT32 panel2Y = panel1Y;

    fb.FillRect(panel2X, panel2Y, panelW, panelH, Graphics::Color::Panel());
    fb.DrawRect(panel2X, panel2Y, panelW, panelH, Graphics::Color::Border());

    fb.DrawString(panel2X + 16, panel2Y + 16, "INPUT SUBSYSTEM & PERIPHERALS", Graphics::Color::TextWhite(), Graphics::Color::Panel());
    fb.DrawHLine(panel2X + 16, panel2Y + 38, panelW - 32, Graphics::Color::Border());
}

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE imageHandle, EFI_SYSTEM_TABLE* systemTable) {
    (void)imageHandle;

    if (!systemTable || !systemTable->BootServices) {
        return EFI_INVALID_PARAMETER;
    }

    // 1. Initialize Display & Framebuffer
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

    // 4. Render Base UI
    RenderPanels(dispInfo, sysInfo);

    auto& fb = Graphics::Framebuffer::Instance();
    auto& kbd = Drivers::Keyboard::Instance();
    auto& mouse = Drivers::Mouse::Instance();

    UINT32 panelW = (dispInfo.Width - 60) / 2;
    if (panelW < 360) panelW = 360;
    UINT32 panel2X = 20 + panelW + 20;
    UINT32 panel2Y = 62;

    char buf[64];
    uint64_t keyPressCount = 0;
    Drivers::KeyEvent lastKey;
    Drivers::MouseState lastMouse = mouse.GetState();

    // Draw initial cursor with clean backing store
    fb.MoveCursor(lastMouse.X, lastMouse.Y);

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
            // Move cursor cleanly without leaving background smudges
            fb.MoveCursor(currentMouse.X, currentMouse.Y);
            lastMouse = currentMouse;
            needsUpdate = true;
        }

        // Update telemetry panel
        if (needsUpdate) {
            // Clear telemetry area inside Right Panel
            fb.FillRect(panel2X + 16, panel2Y + 46, panelW - 32, 275, Graphics::Color::Panel());

            // Keyboard Section
            fb.DrawString(panel2X + 16, panel2Y + 48, "Keyboard Protocol:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            fb.DrawString(panel2X + 175, panel2Y + 48, "[ACTIVE] EFI_TEXT_IN", Graphics::Color::Success(), Graphics::Color::Panel());

            fb.DrawString(panel2X + 16, panel2Y + 72, "Total Keystrokes:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            Utils::UIntToString(keyPressCount, buf, sizeof(buf));
            fb.DrawString(panel2X + 175, panel2Y + 72, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());

            fb.DrawString(panel2X + 16, panel2Y + 96, "Last Key Detected:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            if (lastKey.HasKey) {
                fb.DrawString(panel2X + 175, panel2Y + 96, lastKey.Description, Graphics::Color::Accent(), Graphics::Color::Panel());

                fb.DrawString(panel2X + 16, panel2Y + 120, "ScanCode / UTF-16:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
                Utils::HexToString(lastKey.ScanCode, buf, sizeof(buf));
                fb.DrawString(panel2X + 175, panel2Y + 120, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
                fb.DrawString(panel2X + 235, panel2Y + 120, "/", Graphics::Color::TextMuted(), Graphics::Color::Panel());
                Utils::HexToString(lastKey.UnicodeChar, buf, sizeof(buf));
                fb.DrawString(panel2X + 250, panel2Y + 120, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
            } else {
                fb.DrawString(panel2X + 175, panel2Y + 96, "Awaiting input...", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            }

            fb.DrawHLine(panel2X + 16, panel2Y + 146, panelW - 32, Graphics::Color::Border());

            // Pointer Section
            fb.DrawString(panel2X + 16, panel2Y + 158, "Pointer Interface:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            if (lastMouse.Present) {
                if (lastMouse.Type == Drivers::PointerType::AbsoluteTablet) {
                    fb.DrawString(panel2X + 175, panel2Y + 158, "[ACTIVE] AbsoluteTablet", Graphics::Color::Success(), Graphics::Color::Panel());
                } else {
                    fb.DrawString(panel2X + 175, panel2Y + 158, "[ACTIVE] SimpleRelative", Graphics::Color::Success(), Graphics::Color::Panel());
                }
            } else {
                fb.DrawString(panel2X + 175, panel2Y + 158, "[INACTIVE] Not Found", Graphics::Color::Warning(), Graphics::Color::Panel());
            }

            fb.DrawString(panel2X + 16, panel2Y + 182, "Pointer Position:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            fb.DrawString(panel2X + 175, panel2Y + 182, "X:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            Utils::IntToString(lastMouse.X, buf, sizeof(buf));
            fb.DrawString(panel2X + 195, panel2Y + 182, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
            fb.DrawString(panel2X + 245, panel2Y + 182, "Y:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            Utils::IntToString(lastMouse.Y, buf, sizeof(buf));
            fb.DrawString(panel2X + 265, panel2Y + 182, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());

            fb.DrawString(panel2X + 16, panel2Y + 206, "Relative Delta:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            Utils::IntToString(lastMouse.DeltaX, buf, sizeof(buf));
            fb.DrawString(panel2X + 175, panel2Y + 206, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());
            fb.DrawString(panel2X + 225, panel2Y + 206, ",", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            Utils::IntToString(lastMouse.DeltaY, buf, sizeof(buf));
            fb.DrawString(panel2X + 240, panel2Y + 206, buf, Graphics::Color::TextWhite(), Graphics::Color::Panel());

            fb.DrawString(panel2X + 16, panel2Y + 230, "Button State:", Graphics::Color::TextMuted(), Graphics::Color::Panel());
            fb.DrawString(panel2X + 175, panel2Y + 230,
                lastMouse.LeftButton ? "[LEFT: DOWN]" : "[LEFT: UP]",
                lastMouse.LeftButton ? Graphics::Color::Success() : Graphics::Color::TextMuted(),
                Graphics::Color::Panel()
            );
            fb.DrawString(panel2X + 290, panel2Y + 230,
                lastMouse.RightButton ? "[RIGHT: DOWN]" : "[RIGHT: UP]",
                lastMouse.RightButton ? Graphics::Color::Success() : Graphics::Color::TextMuted(),
                Graphics::Color::Panel()
            );
        }

        // Delay ~5ms
        systemTable->BootServices->Stall(5000);
    }

    return EFI_SUCCESS;
}
