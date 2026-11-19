#include "graphics/framebuffer.hpp"
#include "graphics/font.hpp"

namespace Vesper::Graphics {

Framebuffer& Framebuffer::Instance() {
    static Framebuffer s_Instance;
    return s_Instance;
}

void Framebuffer::Init(UINT32* base, UINT32 width, UINT32 height, UINT32 pitch, EFI_GRAPHICS_PIXEL_FORMAT format) {
    m_Base = base;
    m_Width = width;
    m_Height = height;
    m_Pitch = pitch;
    m_Format = format;
}

void Framebuffer::Clear(Color color) {
    if (!m_Base) return;
    UINT32 c = (color.A << 24) | (color.R << 16) | (color.G << 8) | color.B;
    for (UINT32 y = 0; y < m_Height; ++y) {
        UINT32* row = m_Base + (y * m_Pitch);
        for (UINT32 x = 0; x < m_Width; ++x) {
            row[x] = c;
        }
    }
}

void Framebuffer::PutPixel(UINT32 x, UINT32 y, Color color) {
    if (!m_Base || x >= m_Width || y >= m_Height) return;
    UINT32 c = (color.A << 24) | (color.R << 16) | (color.G << 8) | color.B;
    m_Base[y * m_Pitch + x] = c;
}

void Framebuffer::DrawRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color) {
    if (!m_Base) return;
    for (UINT32 i = 0; i < w; ++i) {
        PutPixel(x + i, y, color);
        PutPixel(x + i, y + h - 1, color);
    }
    for (UINT32 j = 0; j < h; ++j) {
        PutPixel(x, y + j, color);
        PutPixel(x + w - 1, y + j, color);
    }
}

void Framebuffer::FillRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color) {
    if (!m_Base) return;
    UINT32 c = (color.A << 24) | (color.R << 16) | (color.G << 8) | color.B;
    UINT32 endX = (x + w < m_Width) ? (x + w) : m_Width;
    UINT32 endY = (y + h < m_Height) ? (y + h) : m_Height;

    for (UINT32 py = y; py < endY; ++py) {
        UINT32* row = m_Base + (py * m_Pitch);
        for (UINT32 px = x; px < endX; ++px) {
            row[px] = c;
        }
    }
}

void Framebuffer::DrawChar(UINT32 x, UINT32 y, char c, Color fg, Color bg, bool transparentBg) {
    if (!m_Base) return;
    if (c < 32 || c > 126) c = '?';

    const uint8_t* glyph = Font8x16[c - 32];
    UINT32 fgColor = (fg.A << 24) | (fg.R << 16) | (fg.G << 8) | fg.B;
    UINT32 bgColor = (bg.A << 24) | (bg.R << 16) | (bg.G << 8) | bg.B;

    for (UINT32 row = 0; row < 16; ++row) {
        UINT32 py = y + row;
        if (py >= m_Height) break;
        uint8_t line = glyph[row];
        UINT32* rowPtr = m_Base + (py * m_Pitch);

        for (UINT32 col = 0; col < 8; ++col) {
            UINT32 px = x + col;
            if (px >= m_Width) break;

            if (line & (0x80 >> col)) {
                rowPtr[px] = fgColor;
            } else if (!transparentBg) {
                rowPtr[px] = bgColor;
            }
        }
    }
}

void Framebuffer::DrawString(UINT32 x, UINT32 y, const char* str, Color fg, Color bg, bool transparentBg) {
    if (!str) return;
    UINT32 cursorX = x;
    UINT32 cursorY = y;

    while (*str) {
        if (*str == '\n') {
            cursorX = x;
            cursorY += 18;
        } else if (*str == '\r') {
            cursorX = x;
        } else if (*str == '\t') {
            cursorX += 8 * 4;
        } else {
            DrawChar(cursorX, cursorY, *str, fg, bg, transparentBg);
            cursorX += 8;
        }
        str++;
    }
}

void Framebuffer::DrawCursor(UINT32 x, UINT32 y) {
    // 12x18 Arrow mouse cursor bitmap
    static const uint16_t CursorBitmap[18] = {
        0b100000000000,
        0b110000000000,
        0b111000000000,
        0b111100000000,
        0b111110000000,
        0b111111000000,
        0b111111100000,
        0b111111110000,
        0b111111111000,
        0b111111111100,
        0b111111000000,
        0b110111100000,
        0b100011110000,
        0b000001111000,
        0b000000111100,
        0b000000011100,
        0b000000001000,
        0b000000000000
    };

    Color white = Color::White();
    Color black = Color::Black();

    for (UINT32 row = 0; row < 18; ++row) {
        UINT32 py = y + row;
        if (py >= m_Height) break;
        uint16_t line = CursorBitmap[row];

        for (UINT32 col = 0; col < 12; ++col) {
            UINT32 px = x + col;
            if (px >= m_Width) break;

            if (line & (1 << (11 - col))) {
                PutPixel(px, py, (col == 0 || row == 0 || (line & (1 << (12 - col))) == 0) ? black : white);
            }
        }
    }
}

} // namespace Vesper::Graphics
