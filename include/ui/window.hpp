#pragma once

#include "graphics/framebuffer.hpp"
#include "drivers/keyboard.hpp"

namespace Vesper::UI {

class Window {
public:
    Window(int id, const char* title, INT32 x, INT32 y, UINT32 width, UINT32 height);
    virtual ~Window() = default;

    int GetId() const { return m_Id; }
    const char* GetTitle() const { return m_Title; }

    INT32 GetX() const { return m_X; }
    INT32 GetY() const { return m_Y; }
    UINT32 GetWidth() const { return m_Width; }
    UINT32 GetHeight() const { return m_Height; }

    void SetPosition(INT32 x, INT32 y) { m_X = x; m_Y = y; }
    void SetSize(UINT32 w, UINT32 h)   { m_Width = w; m_Height = h; }

    bool IsVisible() const   { return m_IsVisible; }
    void SetVisible(bool v)  { m_IsVisible = v; }

    bool IsFocused() const   { return m_IsFocused; }
    void SetFocused(bool f)  { m_IsFocused = f; }

    bool IsMinimized() const { return m_IsMinimized; }
    void SetMinimized(bool m){ m_IsMinimized = m; }

    bool IsDragging() const  { return m_IsDragging; }
    void SetDragging(bool d) { m_IsDragging = d; }

    INT32 GetDragOffsetX() const { return m_DragOffsetX; }
    INT32 GetDragOffsetY() const { return m_DragOffsetY; }
    void SetDragOffset(INT32 ox, INT32 oy) { m_DragOffsetX = ox; m_DragOffsetY = oy; }

    // Hit testing
    bool ContainsPoint(INT32 px, INT32 py) const;
    bool IsOnTitleBar(INT32 px, INT32 py) const;
    bool IsOnCloseButton(INT32 px, INT32 py) const;

    // Render Window frame, titlebar, and content
    void Render(Graphics::Framebuffer& fb);

    // Event hooks
    virtual void OnRenderContent(Graphics::Framebuffer& fb) = 0;
    virtual void OnKeyDown(const Drivers::KeyEvent& key) { (void)key; }
    virtual void OnClientClick(INT32 clientX, INT32 clientY, bool left) { (void)clientX; (void)clientY; (void)left; }

protected:
    int    m_Id = 0;
    char   m_Title[32] = {0};
    INT32  m_X = 50;
    INT32  m_Y = 50;
    UINT32 m_Width = 400;
    UINT32 m_Height = 250;

    bool   m_IsVisible = true;
    bool   m_IsFocused = false;
    bool   m_IsMinimized = false;
    bool   m_IsDragging = false;

    INT32  m_DragOffsetX = 0;
    INT32  m_DragOffsetY = 0;

    static constexpr UINT32 TITLEBAR_HEIGHT = 26;
};

} // namespace Vesper::UI
