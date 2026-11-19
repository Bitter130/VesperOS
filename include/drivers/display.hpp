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

class Display {
public:
    static Display& Instance();

    EFI_STATUS Init(EFI_BOOT_SERVICES* bs);

    const DisplayInfo& GetInfo() const { return m_Info; }

private:
    Display() = default;

    void ParseEDID(const UINT8* edid, UINT32 size);

    EFI_GRAPHICS_OUTPUT_PROTOCOL* m_GOP = nullptr;
    DisplayInfo m_Info;
};

} // namespace Vesper::Drivers
