#include "system/info.hpp"

namespace Vesper::System {

SystemInfo& SystemInfo::Instance() {
    static SystemInfo s_Instance;
    return s_Instance;
}

EFI_STATUS SystemInfo::Query(EFI_SYSTEM_TABLE* st) {
    if (!st || !st->BootServices) return EFI_INVALID_PARAMETER;

    // 1. Convert FirmwareVendor UTF-16 to ASCII
    if (st->FirmwareVendor) {
        int i = 0;
        while (st->FirmwareVendor[i] && i < 63) {
            m_Info.FirmwareVendor[i] = (char)(st->FirmwareVendor[i] & 0x7F);
            i++;
        }
        m_Info.FirmwareVendor[i] = '\0';
    } else {
        m_Info.FirmwareVendor[0] = 'U';
        m_Info.FirmwareVendor[1] = 'n';
        m_Info.FirmwareVendor[2] = 'k';
        m_Info.FirmwareVendor[3] = 'n';
        m_Info.FirmwareVendor[4] = 'o';
        m_Info.FirmwareVendor[5] = 'w';
        m_Info.FirmwareVendor[6] = 'n';
        m_Info.FirmwareVendor[7] = '\0';
    }

    m_Info.FirmwareRevision = st->FirmwareRevision;

    // 2. Query Memory Map to calculate total and usable RAM
    static UINT8 memoryBuffer[16384];
    UINTN mapSize = sizeof(memoryBuffer);
    UINTN mapKey = 0;
    UINTN descSize = 0;
    UINT32 descVer = 0;

    EFI_STATUS status = st->BootServices->GetMemoryMap(
        &mapSize,
        (EFI_MEMORY_DESCRIPTOR*)memoryBuffer,
        &mapKey,
        &descSize,
        &descVer
    );

    if (!EFI_ERROR(status) && descSize > 0) {
        m_Info.TotalRamBytes = 0;
        m_Info.UsableRamBytes = 0;
        m_Info.ReservedRamBytes = 0;
        m_Info.MemoryDescriptorCount = (UINT32)(mapSize / descSize);

        UINT8* ptr = memoryBuffer;
        for (UINTN i = 0; i < mapSize; i += descSize) {
            EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)ptr;
            UINT64 bytes = desc->NumberOfPages * 4096;
            m_Info.TotalRamBytes += bytes;

            if (desc->Type == EfiConventionalMemory ||
                desc->Type == EfiBootServicesCode ||
                desc->Type == EfiBootServicesData) {
                m_Info.UsableRamBytes += bytes;
            } else {
                m_Info.ReservedRamBytes += bytes;
            }

            ptr += descSize;
        }
    }

    return EFI_SUCCESS;
}

} // namespace Vesper::System
