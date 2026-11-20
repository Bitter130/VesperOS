#include "graphics/framebuffer.hpp"
#include "graphics/font.hpp"

extern "C" void* memcpy(void* dest, const void* src, size_t count);

namespace Vesper::Graphics {

Framebuffer& Framebuffer::Instance() {
    static Framebuffer s_Instance;
    return s_Instance;
}

void Framebuffer::Init(UINT32* base, UINT32 width, UINT32 height, UINT32 pitch, EFI_GRAPHICS_PIXEL_FORMAT format, EFI_BOOT_SERVICES* bs) {
    m_FrontBuffer = base;
    m_Width = width;
    m_Height = height;
    m_Pitch = pitch;
    m_Format = format;
    m_BackBuffer = nullptr;
    m_HasDoubleBuffer = false;
    m_AllocatedPages = 0;

    // Allocate offscreen backbuffer in RAM for flicker-free 60fps rendering
    if (bs && bs->AllocatePages) {
        UINTN bufferBytes = (UINTN)m_Pitch * m_Height * sizeof(UINT32);
        UINTN pagesNeeded = (bufferBytes + 4095) / 4096;
        UINT64 allocatedAddress = 0;

        EFI_STATUS status = bs->AllocatePages(AllocateAnyPages, EfiLoaderData, pagesNeeded, &allocatedAddress);
        if (!EFI_ERROR(status) && allocatedAddress != 0) {
            m_BackBuffer = (UINT32*)allocatedAddress;
            m_AllocatedPages = pagesNeeded;
            m_HasDoubleBuffer = true;
        }
    }

    if (!m_BackBuffer) {
        m_BackBuffer = m_FrontBuffer;
        m_HasDoubleBuffer = false;
    }
}

void Framebuffer::Resize(UINT32* base, UINT32 width, UINT32 height, UINT32 pitch, EFI_BOOT_SERVICES* bs) {
    m_FrontBuffer = base;
    m_Width = width;
    m_Height = height;
    m_Pitch = pitch;

    UINTN bufferBytes = (UINTN)m_Pitch * m_Height * sizeof(UINT32);
    UINTN pagesNeeded = (bufferBytes + 4095) / 4096;

    // Reallocate backbuffer if larger than current allocation
    if (pagesNeeded > m_AllocatedPages && bs && bs->AllocatePages) {
        if (m_BackBuffer && m_HasDoubleBuffer && bs->FreePages) {
            bs->FreePages((UINT64)m_BackBuffer, m_AllocatedPages);
        }

        UINT64 allocatedAddress = 0;
        EFI_STATUS status = bs->AllocatePages(AllocateAnyPages, EfiLoaderData, pagesNeeded, &allocatedAddress);
        if (!EFI_ERROR(status) && allocatedAddress != 0) {
            m_BackBuffer = (UINT32*)allocatedAddress;
            m_AllocatedPages = pagesNeeded;
            m_HasDoubleBuffer = true;
        } else {
            m_BackBuffer = m_FrontBuffer;
            m_HasDoubleBuffer = false;
        }
    }

    if (!m_BackBuffer) {
        m_BackBuffer = m_FrontBuffer;
        m_HasDoubleBuffer = false;
    }
}

static inline UINT32 PackColor(Color c) {
    return (c.A << 24) | (c.R << 16) | (c.G << 8) | c.B;
}

void Framebuffer::Clear(Color color) {
    if (!m_BackBuffer) return;
    UINT32 c = PackColor(color);
    for (UINT32 y = 0; y < m_Height; ++y) {
        UINT32* row = m_BackBuffer + (y * m_Pitch);
        for (UINT32 x = 0; x < m_Width; ++x) {
            row[x] = c;
        }
    }
}

void Framebuffer::PutPixel(UINT32 x, UINT32 y, Color color) {
    if (!m_BackBuffer || x >= m_Width || y >= m_Height) return;
    m_BackBuffer[y * m_Pitch + x] = PackColor(color);
}

void Framebuffer::DrawHLine(UINT32 x, UINT32 y, UINT32 w, Color color) {
    if (!m_BackBuffer || y >= m_Height) return;
    UINT32 endX = (x + w < m_Width) ? (x + w) : m_Width;
    UINT32 c = PackColor(color);
    UINT32* row = m_BackBuffer + (y * m_Pitch);
    for (UINT32 px = x; px < endX; ++px) {
        row[px] = c;
    }
}

void Framebuffer::DrawVLine(UINT32 x, UINT32 y, UINT32 h, Color color) {
    if (!m_BackBuffer || x >= m_Width) return;
    UINT32 endY = (y + h < m_Height) ? (y + h) : m_Height;
    UINT32 c = PackColor(color);
    for (UINT32 py = y; py < endY; ++py) {
        m_BackBuffer[py * m_Pitch + x] = c;
    }
}

void Framebuffer::DrawRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color) {
    DrawHLine(x, y, w, color);
    DrawHLine(x, y + h - 1, w, color);
    DrawVLine(x, y, h, color);
    DrawVLine(x + w - 1, y, h, color);
}

void Framebuffer::FillRect(UINT32 x, UINT32 y, UINT32 w, UINT32 h, Color color) {
    if (!m_BackBuffer) return;
    UINT32 c = PackColor(color);
    UINT32 endX = (x + w < m_Width) ? (x + w) : m_Width;
    UINT32 endY = (y + h < m_Height) ? (y + h) : m_Height;

    for (UINT32 py = y; py < endY; ++py) {
        UINT32* row = m_BackBuffer + (py * m_Pitch);
        for (UINT32 px = x; px < endX; ++px) {
            row[px] = c;
        }
    }
}

void Framebuffer::DrawChar(UINT32 x, UINT32 y, char c, Color fg, Color bg, bool transparentBg) {
    if (!m_BackBuffer) return;
    if (c < 32 || c > 126) c = '?';

    const uint8_t* glyph = Font8x16[c - 32];
    UINT32 fgColor = PackColor(fg);
    UINT32 bgColor = PackColor(bg);

    for (UINT32 row = 0; row < 16; ++row) {
        UINT32 py = y + row;
        if (py >= m_Height) break;
        uint8_t line = glyph[row];
        UINT32* rowPtr = m_BackBuffer + (py * m_Pitch);

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

void Framebuffer::DrawCursor(INT32 x, INT32 y) {
    if (!m_BackBuffer) return;

    if (x < 0) x = 0;
    if (x >= (INT32)m_Width) x = (INT32)m_Width - 1;
    if (y < 0) y = 0;
    if (y >= (INT32)m_Height) y = (INT32)m_Height - 1;

    UINT32 cx = (UINT32)x;
    UINT32 cy = (UINT32)y;
    UINT32 w = 12;
    UINT32 h = 18;

    if (cx + w > m_Width)  w = m_Width - cx;
    if (cy + h > m_Height) h = m_Height - cy;

    static const uint16_t CursorOutline[18] = {
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

    static const uint16_t CursorInside[18] = {
        0b000000000000,
        0b010000000000,
        0b011000000000,
        0b011100000000,
        0b011110000000,
        0b011111000000,
        0b011111100000,
        0b011111110000,
        0b011111100000,
        0b011110000000,
        0b011011000000,
        0b010001100000,
        0b000000110000,
        0b000000011000,
        0b000000001000,
        0b000000000000,
        0b000000000000,
        0b000000000000
    };

    UINT32 white = 0xFFFFFFFF;
    UINT32 black = 0xFF000000;

    for (UINT32 r = 0; r < h; ++r) {
        uint16_t outline = CursorOutline[r];
        uint16_t inside  = CursorInside[r];
        UINT32* rowPtr = m_BackBuffer + ((cy + r) * m_Pitch);

        for (UINT32 c = 0; c < w; ++c) {
            uint16_t mask = (1 << (11 - c));
            if (inside & mask) {
                rowPtr[cx + c] = white;
            } else if (outline & mask) {
                rowPtr[cx + c] = black;
            }
        }
    }
}

void Framebuffer::SwapBuffers() {
    if (m_HasDoubleBuffer && m_FrontBuffer && m_BackBuffer && m_FrontBuffer != m_BackBuffer) {
        UINTN bytes = (UINTN)m_Pitch * m_Height * sizeof(UINT32);
        memcpy(m_FrontBuffer, m_BackBuffer, bytes);
    }
}

} // namespace Vesper::Graphics
