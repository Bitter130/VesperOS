#pragma once

#include "uefi/uefi.hpp"

namespace Vesper::Graphics {

struct Color {
    UINT8 B;
    UINT8 G;
    UINT8 R;
    UINT8 A;

    static constexpr Color RGB(UINT8 r, UINT8 g, UINT8 b) {
        return Color{b, g, r, 0xFF};
    }

    static constexpr Color Bg()          { return RGB(11, 12, 16); }     // Deep black-slate desktop
    static constexpr Color Taskbar()     { return RGB(16, 18, 24); }     // Taskbar background
    static constexpr Color WindowBg()    { return RGB(18, 20, 27); }     // Window client area
    static constexpr Color TitleActive() { return RGB(28, 32, 44); }     // Active window titlebar
    static constexpr Color TitleInactive(){return RGB(20, 22, 30); }     // Inactive window titlebar
    static constexpr Color Border()      { return RGB(42, 46, 60); }     // Subtle structural border
    static constexpr Color BorderActive(){ return RGB(70, 80, 105); }    // Active border highlight
    static constexpr Color TextWhite()   { return RGB(240, 243, 246); }  // Primary text
    static constexpr Color TextMuted()   { return RGB(120, 128, 142); }  // Subdued labels
    static constexpr Color Accent()      { return RGB(96, 165, 250); }   // Ice steel blue
    static constexpr Color Success()     { return RGB(52, 211, 153); }   // Terminal mint green
    static constexpr Color CloseBtn()    { return RGB(239, 68, 68); }    // Red [X] button
};

class Framebuffer {
public:
    static Framebuffer& Instance();

    void Init(UINT32* base, UINT32 width, UINT32 height, UINT32 pitch, EFI_GRAPHICS_PIXEL_FORMAT format, EFI_BOOT_SERVICES* bs);
    void Resize(UINT32* base, UINT32 width, UINT32 height, UINT32 pitch, EFI_BOOT_SERVICES* bs);

    UINT32 GetWidth() const  { return m_Width; }
    UINT32 GetHeight() const { return m_Height; }
    UINT32 GetPitch() const  { return m_Pitch; }

    void Clear(Color color);
    void PutPixel(UINT32 x, UINT32 y, Color color);
    void DrawHLine(UINT32 x, UINT32 y, UINT32 w, Color color);
    void DrawVLine(UINT32 x, UINT32 y, UINT32 h, Color color);
    void DrawRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color);
    void FillRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color);

    void DrawChar(UINT32 x, UINT32 y, char c, Color fg, Color bg = Color::Bg(), bool transparentBg = false);
    void DrawString(UINT32 x, UINT32 y, const char* str, Color fg, Color bg = Color::Bg(), bool transparentBg = false);

    void DrawCursor(INT32 x, INT32 y);

    // Double buffer swap: Copies offscreen backbuffer to the physical video framebuffer
    void SwapBuffers();

private:
    Framebuffer() = default;

    UINT32* m_FrontBuffer = nullptr;
    UINT32* m_BackBuffer  = nullptr;
    UINTN   m_AllocatedPages = 0;
    UINT32  m_Width = 0;
    UINT32  m_Height = 0;
    UINT32  m_Pitch = 0;
    EFI_GRAPHICS_PIXEL_FORMAT m_Format = PixelBlueGreenRedReserved8BitPerColor;
    bool    m_HasDoubleBuffer = false;
};

} // namespace Vesper::Graphics
