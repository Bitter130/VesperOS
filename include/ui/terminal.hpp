#pragma once

#include "ui/window.hpp"
#include "system/info.hpp"

namespace Vesper::UI {

class TerminalWindow : public Window {
public:
    TerminalWindow(int id, INT32 x, INT32 y, UINT32 w, UINT32 h, EFI_SYSTEM_TABLE* st);

    void OnRenderContent(Graphics::Framebuffer& fb) override;
    void OnKeyDown(const Drivers::KeyEvent& key) override;

    void Print(const char* text);
    void ExecuteCommand(const char* cmd);

private:
    static constexpr int MAX_LINES = 12;
    static constexpr int MAX_LINE_LEN = 60;

    char m_Lines[MAX_LINES][MAX_LINE_LEN] = {0};
    int  m_LineCount = 0;

    char m_InputBuffer[MAX_LINE_LEN] = {0};
    int  m_InputLen = 0;

    EFI_SYSTEM_TABLE* m_SystemTable = nullptr;

    void AppendLine(const char* line);
};

} // namespace Vesper::UI
