#pragma once

#include "uefi/uefi.hpp"

namespace Vesper::Drivers {

enum class PointerType {
    None,
    SimpleRelative,
    AbsoluteTablet
};

struct MouseState {
    INT32       X = 0;
    INT32       Y = 0;
    INT32       DeltaX = 0;
    INT32       DeltaY = 0;
    INT32       DeltaZ = 0;
    bool        LeftButton = false;
    bool        RightButton = false;
    bool        Present = false;
    PointerType Type = PointerType::None;
};

class Mouse {
public:
    static Mouse& Instance();

    EFI_STATUS Init(EFI_BOOT_SERVICES* bs, UINT32 screenWidth, UINT32 screenHeight);

    void SetScreenSize(UINT32 screenWidth, UINT32 screenHeight) {
        m_ScreenWidth = screenWidth;
        m_ScreenHeight = screenHeight;
        if (m_State.X >= (INT32)m_ScreenWidth) m_State.X = (INT32)m_ScreenWidth - 1;
        if (m_State.Y >= (INT32)m_ScreenHeight) m_State.Y = (INT32)m_ScreenHeight - 1;
    }

    bool PollMouse(MouseState& outState);

    const MouseState& GetState() const { return m_State; }

private:
    Mouse() = default;

    EFI_SIMPLE_POINTER_PROTOCOL*   m_SimplePointer = nullptr;
    EFI_ABSOLUTE_POINTER_PROTOCOL* m_AbsolutePointer = nullptr;
    MouseState m_State;
    UINT32     m_ScreenWidth = 800;
    UINT32     m_ScreenHeight = 600;
};

} // namespace Vesper::Drivers
