#include "drivers/mouse.hpp"
#include "uefi/guids.hpp"

namespace Vesper::Drivers {

Mouse& Mouse::Instance() {
    static Mouse s_Instance;
    return s_Instance;
}

EFI_STATUS Mouse::Init(EFI_BOOT_SERVICES* bs, UINT32 screenWidth, UINT32 screenHeight) {
    if (!bs) return EFI_INVALID_PARAMETER;

    m_ScreenWidth = screenWidth;
    m_ScreenHeight = screenHeight;

    // Start cursor in the center of the screen
    m_State.X = (INT32)(screenWidth / 2);
    m_State.Y = (INT32)(screenHeight / 2);
    m_State.Present = false;

    // Locate Simple Pointer Protocol
    EFI_STATUS status = bs->LocateProtocol(&EFI_SIMPLE_POINTER_PROTOCOL_GUID, nullptr, (VOID**)&m_Pointer);
    if (!EFI_ERROR(status) && m_Pointer) {
        m_Pointer->Reset(m_Pointer, FALSE);
        m_State.Present = true;
    }

    return status;
}

bool Mouse::PollMouse(MouseState& outState) {
    if (!m_Pointer) {
        outState = m_State;
        return false;
    }

    EFI_SIMPLE_POINTER_STATE state;
    EFI_STATUS status = m_Pointer->GetState(m_Pointer, &state);

    if (EFI_ERROR(status) || status == EFI_NOT_READY) {
        outState = m_State;
        return false;
    }

    // Scale mouse movement according to resolution
    INT32 dx = state.RelativeMovementX;
    INT32 dy = state.RelativeMovementY;

    if (m_Pointer->Mode && m_Pointer->Mode->ResolutionX > 0 && m_Pointer->Mode->ResolutionY > 0) {
        dx = (INT32)(((INT64)dx * 50) / (INT64)m_Pointer->Mode->ResolutionX);
        dy = (INT32)(((INT64)dy * 50) / (INT64)m_Pointer->Mode->ResolutionY);
    } else {
        dx /= 2;
        dy /= 2;
    }

    m_State.DeltaX = dx;
    m_State.DeltaY = dy;
    m_State.DeltaZ = state.RelativeMovementZ;

    m_State.X += dx;
    m_State.Y += dy;

    // Clamp coordinates to screen dimensions
    if (m_State.X < 0) m_State.X = 0;
    if (m_State.X >= (INT32)m_ScreenWidth) m_State.X = (INT32)m_ScreenWidth - 1;

    if (m_State.Y < 0) m_State.Y = 0;
    if (m_State.Y >= (INT32)m_ScreenHeight) m_State.Y = (INT32)m_ScreenHeight - 1;

    m_State.LeftButton = (state.LeftButton != 0);
    m_State.RightButton = (state.RightButton != 0);

    outState = m_State;
    return true;
}

} // namespace Vesper::Drivers
