#pragma once

#include "uefi/uefi.hpp"

namespace Vesper::System {

class Power {
public:
    static Power& Instance();

    void Init(EFI_SYSTEM_TABLE* systemTable);

    // Power off the PC (ACPI S5 / EfiResetShutdown)
    void Shutdown();

    // Standard cold reboot
    void Reboot();

    // Reboot directly into UEFI Firmware Setup / BIOS screen
    void RebootToFirmware();

private:
    Power() = default;
    EFI_SYSTEM_TABLE* m_SystemTable = nullptr;
};

} // namespace Vesper::System
