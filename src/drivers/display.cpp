#include "drivers/display.hpp"
#include "uefi/guids.hpp"
#include "graphics/framebuffer.hpp"

namespace Vesper::Drivers {

Display& Display::Instance() {
    static Display s_Instance;
    return s_Instance;
}

void Display::ParseEDID(const UINT8* edid, UINT32 size) {
    if (!edid || size < 128) return;

    // Check EDID Header: 00 FF FF FF FF FF FF 00
    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[2] != 0xFF || edid[3] != 0xFF ||
        edid[4] != 0xFF || edid[5] != 0xFF || edid[6] != 0xFF || edid[7] != 0x00) {
        return;
    }

    m_Info.EdidFound = true;

    // Detailed Timing Descriptor 1 starts at byte 54
    UINT32 pixelClock10KHz = (UINT32)edid[54] | ((UINT32)edid[55] << 8);
    if (pixelClock10KHz != 0) {
        UINT32 pixelClockHz = pixelClock10KHz * 10000;

        UINT32 hActive = (UINT32)edid[56] | (((UINT32)edid[58] & 0xF0) << 4);
        UINT32 hBlank  = (UINT32)edid[57] | (((UINT32)edid[58] & 0x0F) << 8);
        UINT32 vActive = (UINT32)edid[59] | (((UINT32)edid[61] & 0xF0) << 4);
        UINT32 vBlank  = (UINT32)edid[60] | (((UINT32)edid[61] & 0x0F) << 8);

        UINT32 hTotal = hActive + hBlank;
        UINT32 vTotal = vActive + vBlank;

        if (hTotal > 0 && vTotal > 0) {
            m_Info.RefreshRateHz = (pixelClockHz + (hTotal * vTotal / 2)) / (hTotal * vTotal);
        }
    }

    // Search Descriptor Blocks (bytes 72, 90, 108) for Monitor Name (tag 0xFC)
    for (int block = 0; block < 4; ++block) {
        int offset = 54 + (block * 18);
        if (offset + 18 > 128) break;

        // If bytes 0..2 are zero, this is a display descriptor block
        if (edid[offset] == 0 && edid[offset + 1] == 0 && edid[offset + 2] == 0 && edid[offset + 3] == 0xFC) {
            int nameIdx = 0;
            for (int i = 5; i < 18; ++i) {
                char c = (char)edid[offset + i];
                if (c == 0x0A || c == 0x00) break;
                if (nameIdx < 31) {
                    m_Info.MonitorName[nameIdx++] = c;
                }
            }
            m_Info.MonitorName[nameIdx] = '\0';
            break;
        }
    }
}

EFI_STATUS Display::Init(EFI_BOOT_SERVICES* bs) {
    if (!bs) return EFI_INVALID_PARAMETER;

    // 1. Locate Graphics Output Protocol
    EFI_STATUS status = bs->LocateProtocol(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (VOID**)&m_GOP);
    if (EFI_ERROR(status) || !m_GOP || !m_GOP->Mode) {
        return status;
    }

    // 2. Query available modes and select the highest resolution mode
    UINT32 bestMode = m_GOP->Mode->Mode;
    UINT64 maxPixels = 0;

    for (UINT32 modeIndex = 0; modeIndex < m_GOP->Mode->MaxMode; ++modeIndex) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo = nullptr;
        UINTN sizeOfInfo = 0;

        if (!EFI_ERROR(m_GOP->QueryMode(m_GOP, modeIndex, &sizeOfInfo, &modeInfo)) && modeInfo) {
            UINT64 pixels = (UINT64)modeInfo->HorizontalResolution * modeInfo->VerticalResolution;
            if (pixels > maxPixels) {
                maxPixels = pixels;
                bestMode = modeIndex;
            }
        }
    }

    // Switch to best mode if different
    if (bestMode != m_GOP->Mode->Mode) {
        m_GOP->SetMode(m_GOP, bestMode);
    }

    // 3. Populate Display Info
    m_Info.Width             = m_GOP->Mode->Info->HorizontalResolution;
    m_Info.Height            = m_GOP->Mode->Info->VerticalResolution;
    m_Info.PixelsPerScanLine = m_GOP->Mode->Info->PixelsPerScanLine;
    m_Info.FrameBufferBase   = m_GOP->Mode->FrameBufferBase;
    m_Info.FrameBufferSize   = m_GOP->Mode->FrameBufferSize;
    m_Info.RefreshRateHz     = 60; // Default fallback

    // 4. Try locating EDID Active Protocol to detect hardware refresh rate & monitor model
    EFI_EDID_ACTIVE_PROTOCOL* edidActive = nullptr;
    status = bs->LocateProtocol(&EFI_EDID_ACTIVE_PROTOCOL_GUID, nullptr, (VOID**)&edidActive);
    if (!EFI_ERROR(status) && edidActive && edidActive->EdidDataBlock) {
        ParseEDID(edidActive->EdidDataBlock, edidActive->SizeOfEdid);
    } else {
        // Fallback to Discovered EDID Protocol
        EFI_EDID_ACTIVE_PROTOCOL* edidDiscovered = nullptr;
        status = bs->LocateProtocol(&EFI_EDID_DISCOVERED_PROTOCOL_GUID, nullptr, (VOID**)&edidDiscovered);
        if (!EFI_ERROR(status) && edidDiscovered && edidDiscovered->EdidDataBlock) {
            ParseEDID(edidDiscovered->EdidDataBlock, edidDiscovered->SizeOfEdid);
        }
    }

    // 5. Initialize Framebuffer
    Graphics::Framebuffer::Instance().Init(
        (UINT32*)m_Info.FrameBufferBase,
        m_Info.Width,
        m_Info.Height,
        m_Info.PixelsPerScanLine,
        m_GOP->Mode->Info->PixelFormat
    );

    return EFI_SUCCESS;
}

} // namespace Vesper::Drivers
