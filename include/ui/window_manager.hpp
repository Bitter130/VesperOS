#pragma once

#include "ui/window.hpp"
#include "drivers/mouse.hpp"

namespace Vesper::UI {

class WindowManager {
public:
    static WindowManager& Instance();

    void Init(UINT32 screenWidth, UINT32 screenHeight);

    bool AddWindow(Window* win);
    void RemoveWindow(Window* win);

    void FocusWindow(Window* win);
    Window* GetFocusedWindow() const { return m_FocusedWindow; }

    void OnScreenResize(UINT32 newWidth, UINT32 newHeight);

    void HandleMouse(const Drivers::MouseState& mouse);
    void HandleKey(const Drivers::KeyEvent& key);

    void Render(Graphics::Framebuffer& fb, uint64_t uptimeSeconds);

    bool IsStartMenuOpen() const { return m_StartMenuOpen; }
    void ToggleStartMenu() { m_StartMenuOpen = !m_StartMenuOpen; }

private:
    WindowManager() = default;

    static constexpr int MAX_WINDOWS = 8;
    Window* m_Windows[MAX_WINDOWS] = {nullptr};
    int     m_WindowCount = 0;

    Window* m_FocusedWindow  = nullptr;
    Window* m_DraggingWindow = nullptr;

    UINT32  m_ScreenWidth  = 1024;
    UINT32  m_ScreenHeight = 768;

    bool    m_PrevLeftButton = false;
    bool    m_StartMenuOpen  = false;
    INT32   m_MouseX         = 0;
    INT32   m_MouseY         = 0;

    static constexpr UINT32 TASKBAR_HEIGHT = 34;
};

} // namespace Vesper::UI
