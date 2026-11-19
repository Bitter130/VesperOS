#pragma once

#include <stdint.h>

namespace Vesper::Utils {

inline void IntToString(int64_t value, char* buffer, int maxLen) {
    if (maxLen <= 0 || !buffer) return;
    if (value == 0) {
        if (maxLen > 1) {
            buffer[0] = '0';
            buffer[1] = '\0';
        }
        return;
    }

    bool negative = (value < 0);
    uint64_t uval = negative ? (uint64_t)(-value) : (uint64_t)value;

    char temp[32];
    int count = 0;
    while (uval > 0 && count < 30) {
        temp[count++] = '0' + (uval % 10);
        uval /= 10;
    }

    int idx = 0;
    if (negative && idx < maxLen - 1) {
        buffer[idx++] = '-';
    }

    while (count > 0 && idx < maxLen - 1) {
        buffer[idx++] = temp[--count];
    }
    buffer[idx] = '\0';
}

inline void UIntToString(uint64_t value, char* buffer, int maxLen) {
    if (maxLen <= 0 || !buffer) return;
    if (value == 0) {
        if (maxLen > 1) {
            buffer[0] = '0';
            buffer[1] = '\0';
        }
        return;
    }

    char temp[32];
    int count = 0;
    while (value > 0 && count < 30) {
        temp[count++] = '0' + (value % 10);
        value /= 10;
    }

    int idx = 0;
    while (count > 0 && idx < maxLen - 1) {
        buffer[idx++] = temp[--count];
    }
    buffer[idx] = '\0';
}

inline void HexToString(uint64_t value, char* buffer, int maxLen, bool prefix = true) {
    if (maxLen <= 0 || !buffer) return;
    const char hexChars[] = "0123456789ABCDEF";

    char temp[32];
    int count = 0;

    if (value == 0) {
        temp[count++] = '0';
    } else {
        while (value > 0 && count < 30) {
            temp[count++] = hexChars[value & 0xF];
            value >>= 4;
        }
    }

    int idx = 0;
    if (prefix && idx + 2 < maxLen) {
        buffer[idx++] = '0';
        buffer[idx++] = 'x';
    }

    while (count > 0 && idx < maxLen - 1) {
        buffer[idx++] = temp[--count];
    }
    buffer[idx] = '\0';
}

} // namespace Vesper::Utils
