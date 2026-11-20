#include "ui/sysinfo_window.hpp"
#include "utils/str.hpp"

namespace Vesper::UI {

SysInfoWindow::SysInfoWindow(int id, INT32 x, INT32 y, UINT32 w, UINT32 h)
    : Window(id, "System Monitor", x, y, w, h) {
}

void SysInfoWindow::OnRenderContent(Graphics::Framebuffer& fb) {
    using namespace Graphics;

    const auto& disp = Drivers::Display::Instance().GetInfo();
    const auto& sys = System::SystemInfo::Instance().GetInfo();

    UINT32 startX = m_X + 16;
    UINT32 startY = m_Y + TITLEBAR_HEIGHT + 14;

    char buf[64];

    // Hardware Architecture
    fb.DrawString(startX, startY, "Architecture:", Color::TextMuted(), Color::WindowBg(), false);
    fb.DrawString(startX + 130, startY, "x86_64 Freestanding UEFI", Color::TextWhite(), Color::WindowBg(), false);

    // Firmware
    fb.DrawString(startX, startY + 22, "Firmware:", Color::TextMuted(), Color::WindowBg(), false);
    fb.DrawString(startX + 130, startY + 22, sys.FirmwareVendor, Color::TextWhite(), Color::WindowBg(), false);

    // Display
    fb.DrawString(startX, startY + 44, "Display Mode:", Color::TextMuted(), Color::WindowBg(), false);
    Utils::UIntToString(disp.Width, buf, sizeof(buf));
    fb.DrawString(startX + 130, startY + 44, buf, Color::TextWhite(), Color::WindowBg(), false);
    fb.DrawString(startX + 170, startY + 44, "x", Color::TextMuted(), Color::WindowBg(), false);
    Utils::UIntToString(disp.Height, buf, sizeof(buf));
    fb.DrawString(startX + 185, startY + 44, buf, Color::TextWhite(), Color::WindowBg(), false);
    fb.DrawString(startX + 225, startY + 44, "@", Color::TextMuted(), Color::WindowBg(), false);
    Utils::UIntToString(disp.RefreshRateHz, buf, sizeof(buf));
    fb.DrawString(startX + 240, startY + 44, buf, Color::Accent(), Color::WindowBg(), false);
    fb.DrawString(startX + 265, startY + 44, "Hz", Color::TextMuted(), Color::WindowBg(), false);

    // Framebuffer
    fb.DrawString(startX, startY + 66, "VRAM Physical:", Color::TextMuted(), Color::WindowBg(), false);
    Utils::HexToString(disp.FrameBufferBase, buf, sizeof(buf));
    fb.DrawString(startX + 130, startY + 66, buf, Color::TextWhite(), Color::WindowBg(), false);

    fb.DrawHLine(startX, startY + 92, m_Width - 32, Color::Border());

    // RAM stats
    uint64_t totalMB = sys.TotalRamBytes / (1024 * 1024);
    uint64_t usableMB = sys.UsableRamBytes / (1024 * 1024);
    uint64_t usedMB = (totalMB > usableMB) ? (totalMB - usableMB) : 0;

    fb.DrawString(startX, startY + 104, "Total Memory:", Color::TextMuted(), Color::WindowBg(), false);
    Utils::UIntToString(totalMB, buf, sizeof(buf));
    fb.DrawString(startX + 130, startY + 104, buf, Color::TextWhite(), Color::WindowBg(), false);
    fb.DrawString(startX + 175, startY + 104, "MB", Color::TextMuted(), Color::WindowBg(), false);

    fb.DrawString(startX, startY + 126, "Available RAM:", Color::TextMuted(), Color::WindowBg(), false);
    Utils::UIntToString(usableMB, buf, sizeof(buf));
    fb.DrawString(startX + 130, startY + 126, buf, Color::Success(), Color::WindowBg(), false);
    fb.DrawString(startX + 175, startY + 126, "MB", Color::TextMuted(), Color::WindowBg(), false);

    // Graphical RAM usage bar
    UINT32 barX = startX;
    UINT32 barY = startY + 154;
    UINT32 barW = m_Width - 32;
    UINT32 barH = 14;

    fb.DrawRect(barX, barY, barW, barH, Color::Border());
    fb.FillRect(barX + 1, barY + 1, barW - 2, barH - 2, Color::RGB(25, 28, 38));

    if (totalMB > 0) {
        UINT32 filledW = (UINT32)((usedMB * (barW - 2)) / totalMB);
        if (filledW > barW - 2) filledW = barW - 2;
        if (filledW > 0) {
            fb.FillRect(barX + 1, barY + 1, filledW, barH - 2, Color::Accent());
        }
    }
}

} // namespace Vesper::UI
