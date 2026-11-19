#include "drivers/keyboard.hpp"

namespace Vesper::Drivers {

Keyboard& Keyboard::Instance() {
    static Keyboard s_Instance;
    return s_Instance;
}

EFI_STATUS Keyboard::Init(EFI_SYSTEM_TABLE* st) {
    if (!st || !st->ConIn) return EFI_INVALID_PARAMETER;
    m_ConIn = st->ConIn;
    // Reset keyboard buffer
    m_ConIn->Reset(m_ConIn, FALSE);
    return EFI_SUCCESS;
}

static void StringCopy(char* dest, const char* src, int maxLen) {
    int i = 0;
    while (src[i] && i < maxLen - 1) {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

bool Keyboard::PollKey(KeyEvent& outEvent) {
    if (!m_ConIn) return false;

    EFI_INPUT_KEY key;
    EFI_STATUS status = m_ConIn->ReadKeyStroke(m_ConIn, &key);

    if (EFI_ERROR(status) || status == EFI_NOT_READY) {
        outEvent.HasKey = false;
        return false;
    }

    outEvent.ScanCode = key.ScanCode;
    outEvent.UnicodeChar = key.UnicodeChar;
    outEvent.HasKey = true;

    // Decode scan codes or unicode
    if (key.ScanCode != 0) {
        switch (key.ScanCode) {
            case 0x01: StringCopy(outEvent.Description, "Up Arrow", sizeof(outEvent.Description)); break;
            case 0x02: StringCopy(outEvent.Description, "Down Arrow", sizeof(outEvent.Description)); break;
            case 0x03: StringCopy(outEvent.Description, "Right Arrow", sizeof(outEvent.Description)); break;
            case 0x04: StringCopy(outEvent.Description, "Left Arrow", sizeof(outEvent.Description)); break;
            case 0x05: StringCopy(outEvent.Description, "Home", sizeof(outEvent.Description)); break;
            case 0x06: StringCopy(outEvent.Description, "End", sizeof(outEvent.Description)); break;
            case 0x07: StringCopy(outEvent.Description, "Insert", sizeof(outEvent.Description)); break;
            case 0x08: StringCopy(outEvent.Description, "Delete", sizeof(outEvent.Description)); break;
            case 0x09: StringCopy(outEvent.Description, "Page Up", sizeof(outEvent.Description)); break;
            case 0x0A: StringCopy(outEvent.Description, "Page Down", sizeof(outEvent.Description)); break;
            case 0x0B: StringCopy(outEvent.Description, "F1", sizeof(outEvent.Description)); break;
            case 0x0C: StringCopy(outEvent.Description, "F2", sizeof(outEvent.Description)); break;
            case 0x0D: StringCopy(outEvent.Description, "F3", sizeof(outEvent.Description)); break;
            case 0x0E: StringCopy(outEvent.Description, "F4", sizeof(outEvent.Description)); break;
            case 0x0F: StringCopy(outEvent.Description, "F5", sizeof(outEvent.Description)); break;
            case 0x10: StringCopy(outEvent.Description, "F6", sizeof(outEvent.Description)); break;
            case 0x11: StringCopy(outEvent.Description, "F7", sizeof(outEvent.Description)); break;
            case 0x12: StringCopy(outEvent.Description, "F8", sizeof(outEvent.Description)); break;
            case 0x13: StringCopy(outEvent.Description, "F9", sizeof(outEvent.Description)); break;
            case 0x14: StringCopy(outEvent.Description, "F10", sizeof(outEvent.Description)); break;
            case 0x15: StringCopy(outEvent.Description, "F11", sizeof(outEvent.Description)); break;
            case 0x16: StringCopy(outEvent.Description, "F12", sizeof(outEvent.Description)); break;
            case 0x17: StringCopy(outEvent.Description, "Escape", sizeof(outEvent.Description)); break;
            default:   StringCopy(outEvent.Description, "Special Key", sizeof(outEvent.Description)); break;
        }
    } else if (key.UnicodeChar != 0) {
        if (key.UnicodeChar == '\r' || key.UnicodeChar == '\n') {
            StringCopy(outEvent.Description, "Enter [Return]", sizeof(outEvent.Description));
        } else if (key.UnicodeChar == '\b') {
            StringCopy(outEvent.Description, "Backspace", sizeof(outEvent.Description));
        } else if (key.UnicodeChar == '\t') {
            StringCopy(outEvent.Description, "Tab", sizeof(outEvent.Description));
        } else if (key.UnicodeChar == ' ') {
            StringCopy(outEvent.Description, "Spacebar", sizeof(outEvent.Description));
        } else if (key.UnicodeChar >= 32 && key.UnicodeChar <= 126) {
            outEvent.Description[0] = '\'';
            outEvent.Description[1] = (char)key.UnicodeChar;
            outEvent.Description[2] = '\'';
            outEvent.Description[3] = '\0';
        } else {
            StringCopy(outEvent.Description, "Unicode Char", sizeof(outEvent.Description));
        }
    }

    return true;
}

} // namespace Vesper::Drivers
