#include "ui/terminal.hpp"
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
        AppendLine("Commands: help, info, mem, clear, echo <text>, reboot");
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
        if (m_SystemTable && m_SystemTable->RuntimeServices) {
            // Cold reboot via UEFI RuntimeServices
            typedef void (EFIAPI *EFI_RESET_SYSTEM)(UINT32, UINTN, UINTN, VOID*);
            struct MiniRT {
                EFI_TABLE_HEADER Hdr;
                void* GetTime; void* SetTime; void* GetWakeupTime; void* SetWakeupTime;
                void* SetVirtualAddressMap; void* ConvertPointer; void* GetVariable;
                void* GetNextVariableName; void* SetVariable; void* GetNextHighMonotonicCount;
                EFI_RESET_SYSTEM ResetSystem;
            };
            MiniRT* rt = (MiniRT*)m_SystemTable->RuntimeServices;
            if (rt && rt->ResetSystem) {
                rt->ResetSystem(0, 0, 0, nullptr);
            }
        }
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
