#pragma once

#include "uefi/uefi.hpp"

namespace Vesper::Drivers {

struct KeyEvent {
    UINT16 ScanCode = 0;
    CHAR16 UnicodeChar = 0;
    bool   HasKey = false;
    char   Description[32] = {0};
};

class Keyboard {
public:
    static Keyboard& Instance();

    EFI_STATUS Init(EFI_SYSTEM_TABLE* st);

    bool PollKey(KeyEvent& outEvent);

private:
    Keyboard() = default;

    EFI_SIMPLE_TEXT_INPUT_PROTOCOL* m_ConIn = nullptr;
};

} // namespace Vesper::Drivers
