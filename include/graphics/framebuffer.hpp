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

    static constexpr Color Bg()         { return RGB(11, 12, 16); }     // Obsidian black
    static constexpr Color Panel()      { return RGB(18, 20, 26); }     // Elevated slate
    static constexpr Color Border()     { return RGB(38, 42, 54); }     // Precision border
    static constexpr Color TextWhite()  { return RGB(240, 243, 246); }  // Crisp white
    static constexpr Color TextMuted()  { return RGB(120, 128, 142); }  // Clean muted gray
    static constexpr Color Accent()     { return RGB(96, 165, 250); }   // Steel blue
    static constexpr Color Success()    { return RGB(52, 211, 153); }   // Mint terminal green
    static constexpr Color Warning()    { return RGB(251, 191, 36); }   // Amber
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
    void DrawHLine(UINT32 x, UINT32 y, UINT32 w, Color color);
    void DrawVLine(UINT32 x, UINT32 y, UINT32 h, Color color);
    void DrawRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color);
    void FillRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color);

    void DrawChar(UINT32 x, UINT32 y, char c, Color fg, Color bg = Color::Bg(), bool transparentBg = false);
    void DrawString(UINT32 x, UINT32 y, const char* str, Color fg, Color bg = Color::Bg(), bool transparentBg = false);

    // Cursor with backing store (zero smearing / clean overlay)
    void MoveCursor(INT32 x, INT32 y);

private:
    Framebuffer() = default;

    UINT32* m_Base = nullptr;
    UINT32  m_Width = 0;
    UINT32  m_Height = 0;
    UINT32  m_Pitch = 0;
    EFI_GRAPHICS_PIXEL_FORMAT m_Format = PixelBlueGreenRedReserved8BitPerColor;

    // Backing store for cursor (16x20)
    UINT32  m_CursorBacking[16 * 20] = {0};
    INT32   m_CursorBackingX = 0;
    INT32   m_CursorBackingY = 0;
    UINT32  m_CursorBackingW = 0;
    UINT32  m_CursorBackingH = 0;
    bool    m_HasCursorBacking = false;
};

} // namespace Vesper::Graphics
