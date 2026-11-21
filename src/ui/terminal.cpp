#include "ui/terminal.hpp"
#include "drivers/display.hpp"
#include "system/power.hpp"
#include "utils/str.hpp"

namespace Vesper::UI {

static bool StrEquals(const char* a, const char* b) {
    while (*a && *b) {
        if (*a != *b) return false;
        a++;
        b++;
    }
    return *a == *b;
}

static bool StrStartsWith(const char* str, const char* prefix) {
    while (*prefix) {
        if (*str != *prefix) return false;
        str++;
        prefix++;
    }
    return true;
}

TerminalWindow::TerminalWindow(int id, INT32 x, INT32 y, UINT32 w, UINT32 h, EFI_SYSTEM_TABLE* st)
    : Window(id, "Terminal // bash", x, y, w, h), m_SystemTable(st) {
    AppendLine("VesperOS Terminal [Version 0.2.0]");
    AppendLine("Type 'help' for available microkernel commands.");
    AppendLine("");
}

void TerminalWindow::AppendLine(const char* line) {
    if (m_LineCount < MAX_LINES) {
        int i = 0;
        while (line[i] && i < MAX_LINE_LEN - 1) {
            m_Lines[m_LineCount][i] = line[i];
            i++;
        }
        m_Lines[m_LineCount][i] = '\0';
        m_LineCount++;
    } else {
        // Shift lines up
        for (int l = 0; l < MAX_LINES - 1; ++l) {
            for (int c = 0; c < MAX_LINE_LEN; ++c) {
                m_Lines[l][c] = m_Lines[l + 1][c];
            }
        }
        int i = 0;
        while (line[i] && i < MAX_LINE_LEN - 1) {
            m_Lines[MAX_LINES - 1][i] = line[i];
            i++;
        }
        m_Lines[MAX_LINES - 1][i] = '\0';
    }
}

void TerminalWindow::Print(const char* text) {
    AppendLine(text);
}

void TerminalWindow::ExecuteCommand(const char* cmd) {
    // Print user's command
    char echoBuf[MAX_LINE_LEN];
    echoBuf[0] = '>';
    echoBuf[1] = ' ';
    int idx = 2;
    for (int i = 0; cmd[i] && idx < MAX_LINE_LEN - 1; ++i) {
        echoBuf[idx++] = cmd[i];
    }
    echoBuf[idx] = '\0';
    AppendLine(echoBuf);

    if (StrEquals(cmd, "help")) {
        AppendLine("Commands: help, info, mem, res, clear, echo <text>, reboot, shutdown, firmware");
    } else if (StrEquals(cmd, "res")) {
        auto& disp = Drivers::Display::Instance();
        const auto& info = disp.GetInfo();
        char buf[MAX_LINE_LEN];

        // Current resolution
        char wStr[16], hStr[16];
        Utils::UIntToString(info.Width, wStr, sizeof(wStr));
        Utils::UIntToString(info.Height, hStr, sizeof(hStr));
        int p = 0;
        const char* pfx = "Active: ";
        while (*pfx) buf[p++] = *pfx++;
        for (int i = 0; wStr[i]; ++i) buf[p++] = wStr[i];
        buf[p++] = 'x';
        for (int i = 0; hStr[i]; ++i) buf[p++] = hStr[i];
        buf[p] = '\0';
        AppendLine(buf);

        // List modes
        UINT32 count = disp.GetModeCount();
        if (count > 6) count = 6; // Limit display lines
        for (UINT32 m = 0; m < count; ++m) {
            UINT32 mw = 0, mh = 0;
            if (disp.QueryMode(m, mw, mh)) {
                char line[MAX_LINE_LEN] = "  [";
                char mNum[8];
                Utils::UIntToString(m, mNum, sizeof(mNum));
                int lp = 3;
                for (int i = 0; mNum[i]; ++i) line[lp++] = mNum[i];
                line[lp++] = ']'; line[lp++] = ' ';
                Utils::UIntToString(mw, wStr, sizeof(wStr));
                Utils::UIntToString(mh, hStr, sizeof(hStr));
                for (int i = 0; wStr[i]; ++i) line[lp++] = wStr[i];
                line[lp++] = 'x';
                for (int i = 0; hStr[i]; ++i) line[lp++] = hStr[i];
                line[lp] = '\0';
                AppendLine(line);
            }
        }
        AppendLine("Usage: res <mode_idx> or res <w> <h>");
    } else if (StrStartsWith(cmd, "res ")) {
        const char* args = cmd + 4;
        while (*args == ' ') args++;

        auto& disp = Drivers::Display::Instance();

        // Check if user passed two numbers (width height) or single number (mode index)
        uint64_t val1 = 0;
        while (*args >= '0' && *args <= '9') {
            val1 = val1 * 10 + (*args - '0');
            args++;
        }
        while (*args == ' ') args++;

        if (*args >= '0' && *args <= '9') {
            uint64_t val2 = 0;
            while (*args >= '0' && *args <= '9') {
                val2 = val2 * 10 + (*args - '0');
                args++;
            }
            if (disp.SetResolution((UINT32)val1, (UINT32)val2)) {
                AppendLine("Resolution changed successfully.");
            } else {
                AppendLine("Unsupported resolution.");
            }
        } else {
            // Single value: mode index
            if (disp.SetMode((UINT32)val1)) {
                AppendLine("Mode switched successfully.");
            } else {
                AppendLine("Invalid mode index.");
            }
        }
    } else if (StrEquals(cmd, "clear")) {
        m_LineCount = 0;
    } else if (StrEquals(cmd, "info")) {
        AppendLine("VesperOS x86_64 Freestanding UEFI Microkernel");
        AppendLine("UI Compositor: Double-buffered 60 FPS Window Manager");
    } else if (StrEquals(cmd, "mem")) {
        const auto& sys = System::SystemInfo::Instance().GetInfo();
        char buf[MAX_LINE_LEN];
        uint64_t totalMB = sys.TotalRamBytes / (1024 * 1024);
        uint64_t usableMB = sys.UsableRamBytes / (1024 * 1024);

        int pos = 0;
        const char* label1 = "Total RAM: ";
        while (*label1) buf[pos++] = *label1++;
        char num[32];
        Utils::UIntToString(totalMB, num, sizeof(num));
        for (int i = 0; num[i]; ++i) buf[pos++] = num[i];
        buf[pos++] = ' '; buf[pos++] = 'M'; buf[pos++] = 'B';
        buf[pos] = '\0';
        AppendLine(buf);

        pos = 0;
        const char* label2 = "Usable RAM: ";
        while (*label2) buf[pos++] = *label2++;
        Utils::UIntToString(usableMB, num, sizeof(num));
        for (int i = 0; num[i]; ++i) buf[pos++] = num[i];
        buf[pos++] = ' '; buf[pos++] = 'M'; buf[pos++] = 'B';
        buf[pos] = '\0';
        AppendLine(buf);
    } else if (StrStartsWith(cmd, "echo ")) {
        AppendLine(cmd + 5);
    } else if (StrEquals(cmd, "reboot")) {
        AppendLine("Rebooting system...");
        System::Power::Instance().Reboot();
    } else if (StrEquals(cmd, "shutdown") || StrEquals(cmd, "poweroff")) {
        AppendLine("Shutting down PC...");
        System::Power::Instance().Shutdown();
    } else if (StrEquals(cmd, "firmware") || StrEquals(cmd, "bios")) {
        AppendLine("Rebooting to UEFI Firmware Setup...");
        System::Power::Instance().RebootToFirmware();
    } else if (cmd[0] != '\0') {
        char err[MAX_LINE_LEN] = "Unknown command: ";
        int p = 17;
        for (int i = 0; cmd[i] && p < MAX_LINE_LEN - 1; ++i) {
            err[p++] = cmd[i];
        }
        err[p] = '\0';
        AppendLine(err);
    }
}

void TerminalWindow::OnKeyDown(const Drivers::KeyEvent& key) {
    if (!key.HasKey) return;

    if (key.UnicodeChar == '\r' || key.UnicodeChar == '\n') {
        ExecuteCommand(m_InputBuffer);
        m_InputLen = 0;
        m_InputBuffer[0] = '\0';
    } else if (key.UnicodeChar == '\b') {
        if (m_InputLen > 0) {
            m_InputLen--;
            m_InputBuffer[m_InputLen] = '\0';
        }
    } else if (key.UnicodeChar >= 32 && key.UnicodeChar <= 126) {
        if (m_InputLen < MAX_LINE_LEN - 1) {
            m_InputBuffer[m_InputLen++] = (char)key.UnicodeChar;
            m_InputBuffer[m_InputLen] = '\0';
        }
    }
}

void TerminalWindow::OnRenderContent(Graphics::Framebuffer& fb) {
    using namespace Graphics;

    UINT32 startX = m_X + 12;
    UINT32 startY = m_Y + TITLEBAR_HEIGHT + 10;

    // Draw output history
    for (int i = 0; i < m_LineCount; ++i) {
        Color fg = (m_Lines[i][0] == '>') ? Color::Accent() : Color::TextWhite();
        fb.DrawString(startX, startY + (i * 18), m_Lines[i], fg, Color::WindowBg(), false);
    }

    // Draw active prompt
    UINT32 promptY = startY + (m_LineCount * 18);
    fb.DrawString(startX, promptY, "vesper:~$ ", Color::Success(), Color::WindowBg(), false);

    // Draw active input buffer
    fb.DrawString(startX + 80, promptY, m_InputBuffer, Color::TextWhite(), Color::WindowBg(), false);

    // Draw cursor
    UINT32 cursorX = startX + 80 + (m_InputLen * 8);
    fb.DrawChar(cursorX, promptY, '_', Color::Accent(), Color::WindowBg(), false);
}

} // namespace Vesper::UI
