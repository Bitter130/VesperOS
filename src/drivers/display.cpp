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

    if (edid[0] != 0x00 || edid[1] != 0xFF || edid[2] != 0xFF || edid[3] != 0xFF ||
        edid[4] != 0xFF || edid[5] != 0xFF || edid[6] != 0xFF || edid[7] != 0x00) {
        return;
    }

    m_Info.EdidFound = true;

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

    for (int block = 0; block < 4; ++block) {
        int offset = 54 + (block * 18);
        if (offset + 18 > 128) break;

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

void Display::UpdateDisplayMetrics() {
    if (!m_GOP || !m_GOP->Mode || !m_GOP->Mode->Info) return;

    m_Info.Width             = m_GOP->Mode->Info->HorizontalResolution;
    m_Info.Height            = m_GOP->Mode->Info->VerticalResolution;
    m_Info.PixelsPerScanLine = m_GOP->Mode->Info->PixelsPerScanLine;
    m_Info.FrameBufferBase   = m_GOP->Mode->FrameBufferBase;
    m_Info.FrameBufferSize   = m_GOP->Mode->FrameBufferSize;

    Graphics::Framebuffer::Instance().Resize(
        (UINT32*)m_Info.FrameBufferBase,
        m_Info.Width,
        m_Info.Height,
        m_Info.PixelsPerScanLine,
        m_BootServices
    );

    if (m_ResizeCallback) {
        m_ResizeCallback(m_Info.Width, m_Info.Height);
    }
}

EFI_STATUS Display::Init(EFI_BOOT_SERVICES* bs) {
    if (!bs) return EFI_INVALID_PARAMETER;
    m_BootServices = bs;

    EFI_STATUS status = bs->LocateProtocol(&EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID, nullptr, (VOID**)&m_GOP);
    if (EFI_ERROR(status) || !m_GOP || !m_GOP->Mode) {
        return status;
    }

    // Select highest native resolution
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

    if (bestMode != m_GOP->Mode->Mode) {
        m_GOP->SetMode(m_GOP, bestMode);
    }

    m_Info.Width             = m_GOP->Mode->Info->HorizontalResolution;
    m_Info.Height            = m_GOP->Mode->Info->VerticalResolution;
    m_Info.PixelsPerScanLine = m_GOP->Mode->Info->PixelsPerScanLine;
    m_Info.FrameBufferBase   = m_GOP->Mode->FrameBufferBase;
    m_Info.FrameBufferSize   = m_GOP->Mode->FrameBufferSize;
    m_Info.RefreshRateHz     = 60;

    EFI_EDID_ACTIVE_PROTOCOL* edidActive = nullptr;
    status = bs->LocateProtocol(&EFI_EDID_ACTIVE_PROTOCOL_GUID, nullptr, (VOID**)&edidActive);
    if (!EFI_ERROR(status) && edidActive && edidActive->EdidDataBlock) {
        ParseEDID(edidActive->EdidDataBlock, edidActive->SizeOfEdid);
    } else {
        EFI_EDID_ACTIVE_PROTOCOL* edidDiscovered = nullptr;
        status = bs->LocateProtocol(&EFI_EDID_DISCOVERED_PROTOCOL_GUID, nullptr, (VOID**)&edidDiscovered);
        if (!EFI_ERROR(status) && edidDiscovered && edidDiscovered->EdidDataBlock) {
            ParseEDID(edidDiscovered->EdidDataBlock, edidDiscovered->SizeOfEdid);
        }
    }

    Graphics::Framebuffer::Instance().Init(
        (UINT32*)m_Info.FrameBufferBase,
        m_Info.Width,
        m_Info.Height,
        m_Info.PixelsPerScanLine,
        m_GOP->Mode->Info->PixelFormat,
        bs
    );

    return EFI_SUCCESS;
}

bool Display::CheckResolutionChange() {
    if (!m_GOP || !m_GOP->Mode || !m_GOP->Mode->Info) return false;

    UINT32 currentW = m_GOP->Mode->Info->HorizontalResolution;
    UINT32 currentH = m_GOP->Mode->Info->VerticalResolution;

    if (currentW != m_Info.Width || currentH != m_Info.Height) {
        UpdateDisplayMetrics();
        return true;
    }
    return false;
}

bool Display::SetMode(UINT32 modeIndex) {
    if (!m_GOP || modeIndex >= m_GOP->Mode->MaxMode) return false;

    EFI_STATUS status = m_GOP->SetMode(m_GOP, modeIndex);
    if (EFI_ERROR(status)) return false;

    UpdateDisplayMetrics();
    return true;
}

bool Display::SetResolution(UINT32 width, UINT32 height) {
    if (!m_GOP) return false;

    for (UINT32 i = 0; i < m_GOP->Mode->MaxMode; ++i) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo = nullptr;
        UINTN sizeOfInfo = 0;

        if (!EFI_ERROR(m_GOP->QueryMode(m_GOP, i, &sizeOfInfo, &modeInfo)) && modeInfo) {
            if (modeInfo->HorizontalResolution == width && modeInfo->VerticalResolution == height) {
                return SetMode(i);
            }
        }
    }
    return false;
}

UINT32 Display::GetModeCount() const {
    return m_GOP ? m_GOP->Mode->MaxMode : 0;
}

bool Display::QueryMode(UINT32 modeIndex, UINT32& outW, UINT32& outH) {
    if (!m_GOP || modeIndex >= m_GOP->Mode->MaxMode) return false;

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* modeInfo = nullptr;
    UINTN sizeOfInfo = 0;

    EFI_STATUS status = m_GOP->QueryMode(m_GOP, modeIndex, &sizeOfInfo, &modeInfo);
    if (!EFI_ERROR(status) && modeInfo) {
        outW = modeInfo->HorizontalResolution;
        outH = modeInfo->VerticalResolution;
        return true;
    }
    return false;
}

} // namespace Vesper::Drivers
