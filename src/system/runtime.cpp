#include <stddef.h>
#include <stdint.h>

extern "C" {

void* memset(void* dest, int val, size_t count) {
    unsigned char* p = (unsigned char*)dest;
    while (count--) {
        *p++ = (unsigned char)val;
    }
    return dest;
}

void* memcpy(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    if (d < s) {
        while (count--) {
            *d++ = *s++;
        }
    } else {
        d += count;
        s += count;
        while (count--) {
            *--d = *--s;
        }
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = (const unsigned char*)s1;
    const unsigned char* p2 = (const unsigned char*)s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

// Stack probe helper for Windows ABI
void __chkstk() {}

// C++ ABI Stubs for freestanding builds
int __cxa_guard_acquire(uint64_t* guard) {
    return !*(uint8_t*)guard;
}

void __cxa_guard_release(uint64_t* guard) {
    *(uint8_t*)guard = 1;
}

void __cxa_guard_abort(uint64_t*) {}

void __cxa_pure_virtual() {
    while (1) {}
}

void _purecall() {
    while (1) {}
}

} // extern "C"

// C++ delete stubs for virtual destructors
void operator delete(void*) noexcept {}
void operator delete(void*, size_t) noexcept {}
void operator delete[](void*) noexcept {}
void operator delete[](void*, size_t) noexcept {}
