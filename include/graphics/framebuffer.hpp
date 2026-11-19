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
    static constexpr Color Black()       { return RGB(0, 0, 0); }
    static constexpr Color White()       { return RGB(255, 255, 255); }
    static constexpr Color DarkSlate()   { return RGB(15, 18, 25); }
    static constexpr Color PanelBg()     { return RGB(24, 28, 38); }
    static constexpr Color Border()      { return RGB(45, 55, 75); }
    static constexpr Color Cyan()        { return RGB(0, 220, 255); }
    static constexpr Color Emerald()     { return RGB(0, 230, 140); }
    static constexpr Color Amber()       { return RGB(255, 180, 50); }
    static constexpr Color Gray()        { return RGB(160, 175, 195); }
};

class Framebuffer {
public:
    static Framebuffer& Instance();

    void Init(UINT32* base, UINT32 width, UINT32 height, UINT32 pitch, EFI_GRAPHICS_PIXEL_FORMAT format);

    UINT32 GetWidth() const  { return m_Width; }
    UINT32 GetHeight() const { return m_Height; }
    UINT32 GetPitch() const  { return m_Pitch; }
    UINT64 GetBase() const   { return (UINT64)m_Base; }

    void Clear(Color color);
    void PutPixel(UINT32 x, UINT32 y, Color color);
    void DrawRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color);
    void FillRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color);

    void DrawChar(UINT32 x, UINT32 y, char c, Color fg, Color bg = Color::Black(), bool transparentBg = false);
    void DrawString(UINT32 x, UINT32 y, const char* str, Color fg, Color bg = Color::Black(), bool transparentBg = false);

    void DrawCursor(UINT32 x, UINT32 y);

private:
    Framebuffer() = default;

    UINT32* m_Base = nullptr;
    UINT32  m_Width = 0;
    UINT32  m_Height = 0;
    UINT32  m_Pitch = 0;
    EFI_GRAPHICS_PIXEL_FORMAT m_Format = PixelBlueGreenRedReserved8BitPerColor;
};

} // namespace Vesper::Graphics
