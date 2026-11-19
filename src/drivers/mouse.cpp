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

    // Center cursor initially
    m_State.X = (INT32)(screenWidth / 2);
    m_State.Y = (INT32)(screenHeight / 2);
    m_State.Present = false;
    m_State.Type = PointerType::None;

    // 1. Try Absolute Pointer Protocol first (for Virtual Machines, VNC, USB Tablet)
    EFI_STATUS status = bs->LocateProtocol(&EFI_ABSOLUTE_POINTER_PROTOCOL_GUID, nullptr, (VOID**)&m_AbsolutePointer);
    if (!EFI_ERROR(status) && m_AbsolutePointer) {
        m_AbsolutePointer->Reset(m_AbsolutePointer, FALSE);
        m_State.Present = true;
        m_State.Type = PointerType::AbsoluteTablet;
    }

    // 2. Also try Simple Pointer Protocol (for physical USB/PS2 mouse)
    status = bs->LocateProtocol(&EFI_SIMPLE_POINTER_PROTOCOL_GUID, nullptr, (VOID**)&m_SimplePointer);
    if (!EFI_ERROR(status) && m_SimplePointer) {
        m_SimplePointer->Reset(m_SimplePointer, FALSE);
        m_State.Present = true;
        if (m_State.Type == PointerType::None) {
            m_State.Type = PointerType::SimpleRelative;
        }
    }

    return m_State.Present ? EFI_SUCCESS : EFI_NOT_FOUND;
}

bool Mouse::PollMouse(MouseState& outState) {
    bool updated = false;

    // Check Absolute Pointer first
    if (m_AbsolutePointer && m_AbsolutePointer->Mode) {
        EFI_ABSOLUTE_POINTER_STATE absState;
        EFI_STATUS status = m_AbsolutePointer->GetState(m_AbsolutePointer, &absState);

        if (!EFI_ERROR(status) && status != EFI_NOT_READY) {
            UINT64 minX = m_AbsolutePointer->Mode->AbsoluteMinX;
            UINT64 maxX = m_AbsolutePointer->Mode->AbsoluteMaxX;
            UINT64 minY = m_AbsolutePointer->Mode->AbsoluteMinY;
            UINT64 maxY = m_AbsolutePointer->Mode->AbsoluteMaxY;

            UINT64 rangeX = (maxX > minX) ? (maxX - minX) : 1;
            UINT64 rangeY = (maxY > minY) ? (maxY - minY) : 1;

            INT32 newX = (INT32)(((absState.CurrentX - minX) * m_ScreenWidth) / rangeX);
            INT32 newY = (INT32)(((absState.CurrentY - minY) * m_ScreenHeight) / rangeY);

            // Clamp
            if (newX < 0) newX = 0;
            if (newX >= (INT32)m_ScreenWidth) newX = (INT32)m_ScreenWidth - 1;
            if (newY < 0) newY = 0;
            if (newY >= (INT32)m_ScreenHeight) newY = (INT32)m_ScreenHeight - 1;

            m_State.DeltaX = newX - m_State.X;
            m_State.DeltaY = newY - m_State.Y;
            m_State.X = newX;
            m_State.Y = newY;
            m_State.DeltaZ = (INT32)absState.CurrentZ;

            m_State.LeftButton  = (absState.ActiveButtons & 0x01) != 0;
            m_State.RightButton = (absState.ActiveButtons & 0x02) != 0;
            m_State.Type = PointerType::AbsoluteTablet;
            updated = true;
        }
    }

    // If absolute didn't update or isn't present, check Simple Pointer
    if (!updated && m_SimplePointer) {
        EFI_SIMPLE_POINTER_STATE simState;
        EFI_STATUS status = m_SimplePointer->GetState(m_SimplePointer, &simState);

        if (!EFI_ERROR(status) && status != EFI_NOT_READY) {
            INT32 dx = simState.RelativeMovementX;
            INT32 dy = simState.RelativeMovementY;

            if (m_SimplePointer->Mode && m_SimplePointer->Mode->ResolutionX > 0 && m_SimplePointer->Mode->ResolutionY > 0) {
                dx = (INT32)(((INT64)dx * 50) / (INT64)m_SimplePointer->Mode->ResolutionX);
                dy = (INT32)(((INT64)dy * 50) / (INT64)m_SimplePointer->Mode->ResolutionY);
            } else {
                dx /= 2;
                dy /= 2;
            }

            m_State.DeltaX = dx;
            m_State.DeltaY = dy;
            m_State.DeltaZ = simState.RelativeMovementZ;

            m_State.X += dx;
            m_State.Y += dy;

            if (m_State.X < 0) m_State.X = 0;
            if (m_State.X >= (INT32)m_ScreenWidth) m_State.X = (INT32)m_ScreenWidth - 1;
            if (m_State.Y < 0) m_State.Y = 0;
            if (m_State.Y >= (INT32)m_ScreenHeight) m_State.Y = (INT32)m_ScreenHeight - 1;

            m_State.LeftButton = (simState.LeftButton != 0);
            m_State.RightButton = (simState.RightButton != 0);
            m_State.Type = PointerType::SimpleRelative;
            updated = true;
        }
    }

    outState = m_State;
    return updated;
}

} // namespace Vesper::Drivers
