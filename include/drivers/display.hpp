#pragma once

#include "uefi/uefi.hpp"

namespace Vesper::Drivers {

struct DisplayInfo {
    UINT32 Width = 0;
    UINT32 Height = 0;
    UINT32 PixelsPerScanLine = 0;
    UINT64 FrameBufferBase = 0;
    UINTN  FrameBufferSize = 0;
    UINT32 RefreshRateHz = 0;
    char   MonitorName[32] = {0};
    bool   EdidFound = false;
};

typedef void (*ScreenResizeCallback)(UINT32 newWidth, UINT32 newHeight);

class Display {
public:
    static Display& Instance();

    EFI_STATUS Init(EFI_BOOT_SERVICES* bs);

    const DisplayInfo& GetInfo() const { return m_Info; }

    // Dynamic resolution controls
    bool CheckResolutionChange();
    bool SetMode(UINT32 modeIndex);
    bool SetResolution(UINT32 width, UINT32 height);

    UINT32 GetModeCount() const;
    bool QueryMode(UINT32 modeIndex, UINT32& outW, UINT32& outH);

    void SetResizeCallback(ScreenResizeCallback cb) { m_ResizeCallback = cb; }

private:
    Display() = default;

    void ParseEDID(const UINT8* edid, UINT32 size);
    void UpdateDisplayMetrics();

    EFI_GRAPHICS_OUTPUT_PROTOCOL* m_GOP = nullptr;
    EFI_BOOT_SERVICES*            m_BootServices = nullptr;
    DisplayInfo                   m_Info;
    ScreenResizeCallback          m_ResizeCallback = nullptr;
};

} // namespace Vesper::Drivers
