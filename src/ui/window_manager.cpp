#include "ui/window_manager.hpp"
#include "utils/str.hpp"

namespace Vesper::UI {

WindowManager& WindowManager::Instance() {
    static WindowManager s_Instance;
    return s_Instance;
}

void WindowManager::Init(UINT32 screenWidth, UINT32 screenHeight) {
    m_ScreenWidth  = screenWidth;
    m_ScreenHeight = screenHeight;
    m_WindowCount  = 0;
    m_FocusedWindow = nullptr;
    m_DraggingWindow = nullptr;
    m_PrevLeftButton = false;
    m_StartMenuOpen = false;
}

bool WindowManager::AddWindow(Window* win) {
    if (!win || m_WindowCount >= MAX_WINDOWS) return false;
    m_Windows[m_WindowCount++] = win;
    FocusWindow(win);
    return true;
}

void WindowManager::RemoveWindow(Window* win) {
    if (!win) return;
    int foundIdx = -1;
    for (int i = 0; i < m_WindowCount; ++i) {
        if (m_Windows[i] == win) {
            foundIdx = i;
            break;
        }
    }
    if (foundIdx != -1) {
        for (int i = foundIdx; i < m_WindowCount - 1; ++i) {
            m_Windows[i] = m_Windows[i + 1];
        }
        m_WindowCount--;
        m_Windows[m_WindowCount] = nullptr;

        if (m_FocusedWindow == win) {
            m_FocusedWindow = (m_WindowCount > 0) ? m_Windows[m_WindowCount - 1] : nullptr;
            if (m_FocusedWindow) m_FocusedWindow->SetFocused(true);
        }
        if (m_DraggingWindow == win) {
            m_DraggingWindow = nullptr;
        }
    }
}

void WindowManager::FocusWindow(Window* win) {
    if (!win) return;

    // Unfocus all
    for (int i = 0; i < m_WindowCount; ++i) {
        if (m_Windows[i]) m_Windows[i]->SetFocused(false);
    }

    // Bring to top of Z-order (move to end of array)
    int foundIdx = -1;
    for (int i = 0; i < m_WindowCount; ++i) {
        if (m_Windows[i] == win) {
            foundIdx = i;
            break;
        }
    }

    if (foundIdx != -1) {
        for (int i = foundIdx; i < m_WindowCount - 1; ++i) {
            m_Windows[i] = m_Windows[i + 1];
        }
        m_Windows[m_WindowCount - 1] = win;
    }

    m_FocusedWindow = win;
    win->SetFocused(true);
    win->SetVisible(true);
    win->SetMinimized(false);
}

void WindowManager::HandleMouse(const Drivers::MouseState& mouse) {
    bool clickStarted = mouse.LeftButton && !m_PrevLeftButton;
    bool clickReleased = !mouse.LeftButton && m_PrevLeftButton;
    m_PrevLeftButton = mouse.LeftButton;

    UINT32 taskbarY = m_ScreenHeight - TASKBAR_HEIGHT;

    // Handle Dragging
    if (m_DraggingWindow) {
        if (mouse.LeftButton) {
            INT32 newX = mouse.X - m_DraggingWindow->GetDragOffsetX();
            INT32 newY = mouse.Y - m_DraggingWindow->GetDragOffsetY();

            // Clamp so title bar stays on screen
            if (newY < 0) newY = 0;
            if (newY > (INT32)taskbarY - 26) newY = (INT32)taskbarY - 26;

            m_DraggingWindow->SetPosition(newX, newY);
            return;
        } else {
            m_DraggingWindow->SetDragging(false);
            m_DraggingWindow = nullptr;
        }
    }

    if (clickStarted) {
        // 1. Check Start Menu if open
        if (m_StartMenuOpen) {
            INT32 menuW = 200;
            INT32 menuH = 110;
            INT32 menuX = 10;
            INT32 menuY = (INT32)taskbarY - menuH - 6;

            if (mouse.X >= menuX && mouse.X < menuX + menuW &&
                mouse.Y >= menuY && mouse.Y < menuY + menuH) {
                // Item 1: Terminal (menuY + 8 to menuY + 36)
                if (mouse.Y >= menuY + 8 && mouse.Y < menuY + 36) {
                    if (m_WindowCount > 0 && m_Windows[0]) {
                        FocusWindow(m_Windows[0]);
                    }
                }
                // Item 2: System Monitor (menuY + 40 to menuY + 68)
                else if (mouse.Y >= menuY + 40 && mouse.Y < menuY + 68) {
                    if (m_WindowCount > 1 && m_Windows[1]) {
                        FocusWindow(m_Windows[1]);
                    }
                }
                m_StartMenuOpen = false;
                return;
            } else {
                m_StartMenuOpen = false;
            }
        }

        // 2. Check Taskbar Click
        if (mouse.Y >= (INT32)taskbarY) {
            // Check Start Button [ ❖ Vesper ] (x: 10 to 110)
            if (mouse.X >= 10 && mouse.X <= 110) {
                m_StartMenuOpen = !m_StartMenuOpen;
                return;
            }

            // Check Taskbar Window Chips
            INT32 chipX = 120;
            for (int i = 0; i < m_WindowCount; ++i) {
                Window* win = m_Windows[i];
                if (!win) continue;
                if (mouse.X >= chipX && mouse.X < chipX + 130) {
                    if (m_FocusedWindow == win && !win->IsMinimized()) {
                        win->SetMinimized(true);
                        win->SetFocused(false);
                        m_FocusedWindow = nullptr;
                    } else {
                        FocusWindow(win);
                    }
                    return;
                }
                chipX += 135;
            }
            return;
        }

        // 3. Check Windows (top to bottom of Z-order)
        for (int i = m_WindowCount - 1; i >= 0; --i) {
            Window* win = m_Windows[i];
            if (!win || !win->IsVisible() || win->IsMinimized()) continue;

            // Check Close button
            if (win->IsOnCloseButton(mouse.X, mouse.Y)) {
                win->SetVisible(false);
                if (m_FocusedWindow == win) {
                    m_FocusedWindow = nullptr;
                }
                return;
            }

            // Check Titlebar (Dragging)
            if (win->IsOnTitleBar(mouse.X, mouse.Y)) {
                FocusWindow(win);
                m_DraggingWindow = win;
                win->SetDragging(true);
                win->SetDragOffset(mouse.X - win->GetX(), mouse.Y - win->GetY());
                return;
            }

            // Check Client area
            if (win->ContainsPoint(mouse.X, mouse.Y)) {
                FocusWindow(win);
                win->OnClientClick(mouse.X - win->GetX(), mouse.Y - win->GetY(), true);
                return;
            }
        }
    }
}

void WindowManager::HandleKey(const Drivers::KeyEvent& key) {
    if (m_FocusedWindow && key.HasKey) {
        m_FocusedWindow->OnKeyDown(key);
    }
}

void WindowManager::Render(Graphics::Framebuffer& fb, uint64_t uptimeSeconds) {
    using namespace Graphics;

    // 1. Desktop Wallpaper / Grid Background
    fb.Clear(Color::Bg());

    // Subtle dark grid pattern across the desktop
    for (UINT32 y = 20; y < m_ScreenHeight - TASKBAR_HEIGHT; y += 40) {
        for (UINT32 x = 20; x < m_ScreenWidth; x += 40) {
            fb.PutPixel(x, y, Color::RGB(22, 24, 32));
        }
    }

    // 2. Render Windows in Z-order
    for (int i = 0; i < m_WindowCount; ++i) {
        if (m_Windows[i]) {
            m_Windows[i]->Render(fb);
        }
    }

    // 3. Render Taskbar
    UINT32 taskbarY = m_ScreenHeight - TASKBAR_HEIGHT;
    fb.FillRect(0, taskbarY, m_ScreenWidth, TASKBAR_HEIGHT, Color::Taskbar());
    fb.DrawHLine(0, taskbarY, m_ScreenWidth, Color::Border());

    // Start Button [ ❖ Vesper ]
    Color startBg = m_StartMenuOpen ? Color::Accent() : Color::RGB(26, 30, 42);
    Color startFg = m_StartMenuOpen ? Color::RGB(10, 12, 16) : Color::TextWhite();
    fb.FillRect(10, taskbarY + 4, 100, TASKBAR_HEIGHT - 8, startBg);
    fb.DrawRect(10, taskbarY + 4, 100, TASKBAR_HEIGHT - 8, Color::Border());
    fb.DrawString(22, taskbarY + 9, ":: VESPER", startFg, startBg, false);

    // Active Window Chips
    INT32 chipX = 120;
    for (int i = 0; i < m_WindowCount; ++i) {
        Window* win = m_Windows[i];
        if (!win || !win->IsVisible()) continue;

        bool active = (m_FocusedWindow == win && !win->IsMinimized());
        Color chipBg = active ? Color::RGB(32, 36, 50) : Color::RGB(20, 22, 30);
        Color chipFg = active ? Color::TextWhite() : Color::TextMuted();

        fb.FillRect(chipX, taskbarY + 4, 130, TASKBAR_HEIGHT - 8, chipBg);
        fb.DrawRect(chipX, taskbarY + 4, 130, TASKBAR_HEIGHT - 8, active ? Color::Accent() : Color::Border());

        // Truncate title for chip
        char chipTitle[15];
        const char* fullTitle = win->GetTitle();
        int c = 0;
        while (fullTitle[c] && c < 14) {
            chipTitle[c] = fullTitle[c];
            c++;
        }
        chipTitle[c] = '\0';

        fb.DrawString(chipX + 10, taskbarY + 9, chipTitle, chipFg, chipBg, false);
        chipX += 135;
    }

    // Taskbar Right Side: Architecture & Uptime Clock
    char clockBuf[32];
    uint64_t hrs  = (uptimeSeconds / 3600);
    uint64_t mins = (uptimeSeconds % 3600) / 60;
    uint64_t secs = (uptimeSeconds % 60);

    char hStr[8], mStr[8], sStr[8];
    Utils::UIntToString(hrs, hStr, sizeof(hStr));
    Utils::UIntToString(mins, mStr, sizeof(mStr));
    Utils::UIntToString(secs, sStr, sizeof(sStr));

    clockBuf[0] = (hrs < 10) ? '0' : hStr[0];
    clockBuf[1] = (hrs < 10) ? hStr[0] : hStr[1];
    clockBuf[2] = ':';
    clockBuf[3] = (mins < 10) ? '0' : mStr[0];
    clockBuf[4] = (mins < 10) ? mStr[0] : mStr[1];
    clockBuf[5] = ':';
    clockBuf[6] = (secs < 10) ? '0' : sStr[0];
    clockBuf[7] = (secs < 10) ? sStr[0] : sStr[1];
    clockBuf[8] = '\0';

    fb.DrawString(m_ScreenWidth - 180, taskbarY + 9, "x86_64 //", Color::TextMuted(), Color::Taskbar(), false);
    fb.DrawString(m_ScreenWidth - 95, taskbarY + 9, clockBuf, Color::TextWhite(), Color::Taskbar(), false);

    // 4. Start Menu Popup (if open)
    if (m_StartMenuOpen) {
        INT32 menuW = 200;
        INT32 menuH = 110;
        INT32 menuX = 10;
        INT32 menuY = (INT32)taskbarY - menuH - 6;

        fb.FillRect(menuX, menuY, menuW, menuH, Color::RGB(22, 26, 36));
        fb.DrawRect(menuX, menuY, menuW, menuH, Color::BorderActive());

        // Header
        fb.FillRect(menuX, menuY, menuW, 26, Color::RGB(28, 32, 46));
        fb.DrawString(menuX + 12, menuY + 5, "Applications", Color::Accent(), Color::RGB(28, 32, 46), false);
        fb.DrawHLine(menuX, menuY + 26, menuW, Color::Border());

        // Menu items
        fb.DrawString(menuX + 16, menuY + 38, "> Terminal Shell", Color::TextWhite(), Color::RGB(22, 26, 36), false);
        fb.DrawString(menuX + 16, menuY + 68, "> System Monitor", Color::TextWhite(), Color::RGB(22, 26, 36), false);
    }
}

} // namespace Vesper::UI
