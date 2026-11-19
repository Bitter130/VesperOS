#pragma once

#include "uefi/uefi.hpp"

namespace Vesper::System {

struct HardwareInfo {
    char   FirmwareVendor[64] = {0};
    UINT32 FirmwareRevision = 0;
    UINT64 TotalRamBytes = 0;
    UINT64 UsableRamBytes = 0;
    UINT64 ReservedRamBytes = 0;
    UINT32 MemoryDescriptorCount = 0;
};

class SystemInfo {
public:
    static SystemInfo& Instance();

    EFI_STATUS Query(EFI_SYSTEM_TABLE* st);

    const HardwareInfo& GetInfo() const { return m_Info; }

private:
    SystemInfo() = default;

    HardwareInfo m_Info;
};

} // namespace Vesper::System
