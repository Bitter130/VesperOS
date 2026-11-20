#include "drivers/mouse.hpp"
#include "uefi/guids.hpp"

namespace Vesper::Drivers {

Mouse& Mouse::Instance() {
    static Mouse s_Instance;
    return s_Instance;
}

EFI_STATUS Mouse::Init(EFI_BOOT_SERVICES* bs, UINT32 screenWidth, UINT32 screenHeight) {
    if (!bs) return EFI_INVALID_PARAMETER;

    m_ScreenWidth  = screenWidth;
    m_ScreenHeight = screenHeight;

    m_State.X = (INT32)(screenWidth / 2);
    m_State.Y = (INT32)(screenHeight / 2);
    m_State.Present = false;
    m_State.Type = PointerType::None;

    m_AbsCount = 0;
    m_SimCount = 0;

    // 1. Enumerate ALL Absolute Pointer handles (USB Tablet, Touchscreens, VNC)
    UINTN handleCount = 0;
    EFI_HANDLE* handles = nullptr;

    if (bs->LocateHandleBuffer) {
        EFI_STATUS status = bs->LocateHandleBuffer(
            ByProtocol,
            &EFI_ABSOLUTE_POINTER_PROTOCOL_GUID,
            nullptr,
            &handleCount,
            &handles
        );

        if (!EFI_ERROR(status) && handles && handleCount > 0) {
            for (UINTN i = 0; i < handleCount && m_AbsCount < MAX_POINTERS; ++i) {
                EFI_ABSOLUTE_POINTER_PROTOCOL* ptr = nullptr;
                if (!EFI_ERROR(bs->HandleProtocol(handles[i], &EFI_ABSOLUTE_POINTER_PROTOCOL_GUID, (VOID**)&ptr)) && ptr) {
                    ptr->Reset(ptr, FALSE);
                    m_AbsolutePointers[m_AbsCount++] = ptr;
                    m_State.Present = true;
                    m_State.Type = PointerType::AbsoluteTablet;
                }
            }
            if (bs->FreePool) bs->FreePool(handles);
        }
    }

    // Fallback if LocateHandleBuffer wasn't available or found none
    if (m_AbsCount == 0 && bs->LocateProtocol) {
        EFI_ABSOLUTE_POINTER_PROTOCOL* ptr = nullptr;
        if (!EFI_ERROR(bs->LocateProtocol(&EFI_ABSOLUTE_POINTER_PROTOCOL_GUID, nullptr, (VOID**)&ptr)) && ptr) {
            ptr->Reset(ptr, FALSE);
            m_AbsolutePointers[m_AbsCount++] = ptr;
            m_State.Present = true;
            m_State.Type = PointerType::AbsoluteTablet;
        }
    }

    // 2. Enumerate ALL Simple Pointer handles (USB Mice, PS/2 Mice)
    handleCount = 0;
    handles = nullptr;

    if (bs->LocateHandleBuffer) {
        EFI_STATUS status = bs->LocateHandleBuffer(
            ByProtocol,
            &EFI_SIMPLE_POINTER_PROTOCOL_GUID,
            nullptr,
            &handleCount,
            &handles
        );

        if (!EFI_ERROR(status) && handles && handleCount > 0) {
            for (UINTN i = 0; i < handleCount && m_SimCount < MAX_POINTERS; ++i) {
                EFI_SIMPLE_POINTER_PROTOCOL* ptr = nullptr;
                if (!EFI_ERROR(bs->HandleProtocol(handles[i], &EFI_SIMPLE_POINTER_PROTOCOL_GUID, (VOID**)&ptr)) && ptr) {
                    ptr->Reset(ptr, FALSE);
                    m_SimplePointers[m_SimCount++] = ptr;
                    m_State.Present = true;
                    if (m_State.Type == PointerType::None) {
                        m_State.Type = PointerType::SimpleRelative;
                    }
                }
            }
            if (bs->FreePool) bs->FreePool(handles);
        }
    }

    // Fallback if LocateHandleBuffer wasn't available or found none
    if (m_SimCount == 0 && bs->LocateProtocol) {
        EFI_SIMPLE_POINTER_PROTOCOL* ptr = nullptr;
        if (!EFI_ERROR(bs->LocateProtocol(&EFI_SIMPLE_POINTER_PROTOCOL_GUID, nullptr, (VOID**)&ptr)) && ptr) {
            ptr->Reset(ptr, FALSE);
            m_SimplePointers[m_SimCount++] = ptr;
            m_State.Present = true;
            if (m_State.Type == PointerType::None) {
                m_State.Type = PointerType::SimpleRelative;
            }
        }
    }

    return m_State.Present ? EFI_SUCCESS : EFI_NOT_FOUND;
}

bool Mouse::PollMouse(MouseState& outState) {
    bool updated = false;

    // 1. Poll all Absolute Pointer devices (tablets, VNC)
    for (int i = 0; i < m_AbsCount; ++i) {
        auto* ptr = m_AbsolutePointers[i];
        if (!ptr || !ptr->Mode) continue;

        EFI_ABSOLUTE_POINTER_STATE absState;
        EFI_STATUS status = ptr->GetState(ptr, &absState);

        if (!EFI_ERROR(status) && status != EFI_NOT_READY) {
            UINT64 minX = ptr->Mode->AbsoluteMinX;
            UINT64 maxX = ptr->Mode->AbsoluteMaxX;
            UINT64 minY = ptr->Mode->AbsoluteMinY;
            UINT64 maxY = ptr->Mode->AbsoluteMaxY;

            UINT64 rangeX = (maxX > minX) ? (maxX - minX) : 1;
            UINT64 rangeY = (maxY > minY) ? (maxY - minY) : 1;

            UINT64 curX = absState.CurrentX;
            UINT64 curY = absState.CurrentY;

            if (curX < minX) curX = minX;
            if (curX > maxX) curX = maxX;
            if (curY < minY) curY = minY;
            if (curY > maxY) curY = maxY;

            INT32 newX = (INT32)(((curX - minX) * (UINT64)m_ScreenWidth) / rangeX);
            INT32 newY = (INT32)(((curY - minY) * (UINT64)m_ScreenHeight) / rangeY);

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
            break;
        }
    }

    // 2. Poll all Simple Pointer devices (physical mice, PS/2, USB relative mice)
    for (int i = 0; i < m_SimCount; ++i) {
        auto* ptr = m_SimplePointers[i];
        if (!ptr) continue;

        EFI_SIMPLE_POINTER_STATE simState;
        EFI_STATUS status = ptr->GetState(ptr, &simState);

        if (!EFI_ERROR(status) && status != EFI_NOT_READY) {
            INT32 dx = simState.RelativeMovementX;
            INT32 dy = simState.RelativeMovementY;

            // Direct 1:1 mouse movement (do NOT divide by resolution, which was truncating small deltas to 0!)
            m_State.DeltaX = dx;
            m_State.DeltaY = dy;
            m_State.DeltaZ = simState.RelativeMovementZ;

            m_State.X += dx;
            m_State.Y += dy;

            if (m_State.X < 0) m_State.X = 0;
            if (m_State.X >= (INT32)m_ScreenWidth) m_State.X = (INT32)m_ScreenWidth - 1;
            if (m_State.Y < 0) m_State.Y = 0;
            if (m_State.Y >= (INT32)m_ScreenHeight) m_State.Y = (INT32)m_ScreenHeight - 1;

            m_State.LeftButton  = (simState.LeftButton != 0);
            m_State.RightButton = (simState.RightButton != 0);
            m_State.Type = PointerType::SimpleRelative;
            updated = true;
            break;
        }
    }

    outState = m_State;
    return updated;
}

} // namespace Vesper::Drivers
