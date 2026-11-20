#include "ui/window.hpp"

namespace Vesper::UI {

Window::Window(int id, const char* title, INT32 x, INT32 y, UINT32 width, UINT32 height)
    : m_Id(id), m_X(x), m_Y(y), m_Width(width), m_Height(height) {
    int i = 0;
    if (title) {
        while (title[i] && i < 31) {
            m_Title[i] = title[i];
            i++;
        }
    }
    m_Title[i] = '\0';
}

bool Window::ContainsPoint(INT32 px, INT32 py) const {
    if (!m_IsVisible || m_IsMinimized) return false;
    return (px >= m_X && px < m_X + (INT32)m_Width &&
            py >= m_Y && py < m_Y + (INT32)m_Height);
}

bool Window::IsOnTitleBar(INT32 px, INT32 py) const {
    if (!m_IsVisible || m_IsMinimized) return false;
    return (px >= m_X && px < m_X + (INT32)m_Width &&
            py >= m_Y && py < m_Y + (INT32)TITLEBAR_HEIGHT);
}

bool Window::IsOnCloseButton(INT32 px, INT32 py) const {
    if (!m_IsVisible || m_IsMinimized) return false;
    INT32 btnX = m_X + (INT32)m_Width - 22;
    INT32 btnY = m_Y + 4;
    return (px >= btnX && px < btnX + 18 && py >= btnY && py < btnY + 18);
}

void Window::Render(Graphics::Framebuffer& fb) {
    if (!m_IsVisible || m_IsMinimized) return;

    using namespace Graphics;

    // Window shadow / subtle outer elevation
    fb.DrawRect(m_X - 1, m_Y - 1, m_Width + 2, m_Height + 2, m_IsFocused ? Color::BorderActive() : Color::Border());

    // Titlebar
    Color titleColor = m_IsFocused ? Color::TitleActive() : Color::TitleInactive();
    fb.FillRect(m_X, m_Y, m_Width, TITLEBAR_HEIGHT, titleColor);
    fb.DrawHLine(m_X, m_Y + TITLEBAR_HEIGHT - 1, m_Width, Color::Border());

    // Title Text
    fb.DrawString(m_X + 10, m_Y + 5, m_Title, m_IsFocused ? Color::TextWhite() : Color::TextMuted(), titleColor, false);

    // Close Button [X]
    INT32 btnX = m_X + (INT32)m_Width - 22;
    INT32 btnY = m_Y + 4;
    fb.FillRect(btnX, btnY, 18, 18, Color::RGB(34, 38, 50));
    fb.DrawRect(btnX, btnY, 18, 18, Color::Border());
    fb.DrawString(btnX + 5, btnY + 1, "x", Color::TextMuted(), Color::RGB(34, 38, 50), false);

    // Window Client Area Background
    fb.FillRect(m_X, m_Y + TITLEBAR_HEIGHT, m_Width, m_Height - TITLEBAR_HEIGHT, Color::WindowBg());

    // Render App-specific content
    OnRenderContent(fb);
}

} // namespace Vesper::UI
