#include "system/power.hpp"
#include "uefi/guids.hpp"

namespace Vesper::System {

Power& Power::Instance() {
    static Power s_Instance;
    return s_Instance;
}

void Power::Init(EFI_SYSTEM_TABLE* systemTable) {
    m_SystemTable = systemTable;
}

void Power::Shutdown() {
    if (m_SystemTable && m_SystemTable->RuntimeServices) {
        auto* rt = m_SystemTable->RuntimeServices;
        if (rt->ResetSystem) {
            rt->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, nullptr);
        }
    }

    // Halt fallback if system hasn't powered off
    while (true) {
        asm volatile("cli; hlt");
    }
}

void Power::Reboot() {
    if (m_SystemTable && m_SystemTable->RuntimeServices) {
        auto* rt = m_SystemTable->RuntimeServices;
        if (rt->ResetSystem) {
            rt->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, nullptr);
        }
    }

    // Halt fallback
    while (true) {
        asm volatile("cli; hlt");
    }
}

void Power::RebootToFirmware() {
    if (m_SystemTable && m_SystemTable->RuntimeServices) {
        auto* rt = m_SystemTable->RuntimeServices;

        // Set OsIndications to boot directly into UEFI Firmware Setup (bit 0 = EFI_OS_INDICATIONS_BOOT_TO_FW_UI)
        if (rt->SetVariable) {
            CHAR16 varName[] = {'O', 's', 'I', 'n', 'd', 'i', 'c', 'a', 't', 'i', 'o', 'n', 's', 0};
            UINT64 indications = 0x0000000000000001ULL;
            UINT32 attributes = 0x00000001 | 0x00000002 | 0x00000004; // NON_VOLATILE | BOOTSERVICE | RUNTIME

            if (rt->GetVariable) {
                UINTN dataSize = sizeof(UINT64);
                UINT64 existing = 0;
                UINT32 attr = 0;
                if (!EFI_ERROR(rt->GetVariable(varName, &EFI_GLOBAL_VARIABLE_GUID, &attr, &dataSize, &existing))) {
                    indications |= existing;
                }
            }

            rt->SetVariable(varName, &EFI_GLOBAL_VARIABLE_GUID, attributes, sizeof(indications), &indications);
        }

        // Reboot into UEFI setup
        if (rt->ResetSystem) {
            rt->ResetSystem(EfiResetCold, EFI_SUCCESS, 0, nullptr);
        }
    }

    // Halt fallback
    while (true) {
        asm volatile("cli; hlt");
    }
}

} // namespace Vesper::System
