#pragma once

#include <stdint.h>
#include <stddef.h>

#define EFIAPI __attribute__((ms_abi))

typedef uint64_t UINTN;
typedef int64_t  INTN;
typedef uint8_t  UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef int8_t   INT8;
typedef int16_t  INT16;
typedef int32_t  INT32;
typedef int64_t  INT64;
typedef uint16_t CHAR16;
typedef uint8_t  BOOLEAN;
typedef void     VOID;

#define TRUE  1
#define FALSE 0

typedef void* EFI_HANDLE;
typedef void* EFI_EVENT;
typedef UINTN EFI_STATUS;

// Standard UEFI Status Codes
#define EFI_SUCCESS               0ULL
#define EFI_LOAD_ERROR            (1ULL  | (1ULL << 63))
#define EFI_INVALID_PARAMETER     (2ULL  | (1ULL << 63))
#define EFI_UNSUPPORTED           (3ULL  | (1ULL << 63))
#define EFI_BAD_BUFFER_SIZE       (4ULL  | (1ULL << 63))
#define EFI_BUFFER_TOO_SMALL      (5ULL  | (1ULL << 63))
#define EFI_NOT_READY             (6ULL  | (1ULL << 63))
#define EFI_DEVICE_ERROR          (7ULL  | (1ULL << 63))
#define EFI_WRITE_PROTECTED       (8ULL  | (1ULL << 63))
#define EFI_OUT_OF_RESOURCES      (9ULL  | (1ULL << 63))
#define EFI_NOT_FOUND             (14ULL | (1ULL << 63))

#define EFI_ERROR(status) (((INTN)(status)) < 0)

// 128-bit GUID
typedef struct {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
} EFI_GUID;

// Table Header
typedef struct {
    UINT64 Signature;
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 CRC32;
    UINT32 Reserved;
} EFI_TABLE_HEADER;

// Simple Text Output Protocol
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    BOOLEAN ExtendedVerification
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    const CHAR16 *String
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This
);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE)(
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
    UINTN Attribute
);

typedef struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    EFI_TEXT_RESET             Reset;
    EFI_TEXT_STRING            OutputString;
    void*                      TestString;
    void*                      QueryMode;
    void*                      SetMode;
    EFI_TEXT_SET_ATTRIBUTE     SetAttribute;
    EFI_TEXT_CLEAR_SCREEN      ClearScreen;
    void*                      SetCursorPosition;
    void*                      EnableCursor;
    void*                      Mode;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

// Simple Text Input Protocol
typedef struct {
    UINT16 ScanCode;
    CHAR16 UnicodeChar;
} EFI_INPUT_KEY;

struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET)(
    struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    BOOLEAN ExtendedVerification
);

typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)(
    struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL *This,
    EFI_INPUT_KEY *Key
);

typedef struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
    EFI_INPUT_RESET     Reset;
    EFI_INPUT_READ_KEY  ReadKeyStroke;
    EFI_EVENT           WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

// Pointer / Mouse Protocol
typedef struct {
    INT32   RelativeMovementX;
    INT32   RelativeMovementY;
    INT32   RelativeMovementZ;
    BOOLEAN LeftButton;
    BOOLEAN RightButton;
} EFI_SIMPLE_POINTER_STATE;

typedef struct {
    UINT64 ResolutionX;
    UINT64 ResolutionY;
    UINT64 ResolutionZ;
    BOOLEAN LeftButton;
    BOOLEAN RightButton;
} EFI_SIMPLE_POINTER_MODE;

struct EFI_SIMPLE_POINTER_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_POINTER_RESET)(
    struct EFI_SIMPLE_POINTER_PROTOCOL *This,
    BOOLEAN ExtendedVerification
);

typedef EFI_STATUS (EFIAPI *EFI_SIMPLE_POINTER_GET_STATE)(
    struct EFI_SIMPLE_POINTER_PROTOCOL *This,
    EFI_SIMPLE_POINTER_STATE *State
);

typedef struct EFI_SIMPLE_POINTER_PROTOCOL {
    EFI_SIMPLE_POINTER_RESET     Reset;
    EFI_SIMPLE_POINTER_GET_STATE GetState;
    EFI_EVENT                    WaitForInput;
    EFI_SIMPLE_POINTER_MODE     *Mode;
} EFI_SIMPLE_POINTER_PROTOCOL;

// Graphics Output Protocol (GOP)
typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32 RedMask;
    UINT32 GreenMask;
    UINT32 BlueMask;
    UINT32 ReservedMask;
} EFI_PIXEL_BITMASK;

typedef struct {
    UINT32                    Version;
    UINT32                    HorizontalResolution;
    UINT32                    VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT PixelFormat;
    EFI_PIXEL_BITMASK         PixelInformation;
    UINT32                    PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32                               MaxMode;
    UINT32                               Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
    UINTN                                SizeOfInfo;
    UINT64                               FrameBufferBase;
    UINTN                                FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

struct EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE)(
    struct EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
    UINT32                                       ModeNumber,
    UINTN                                        *SizeOfInfo,
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION         **Info
);

typedef EFI_STATUS (EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE)(
    struct EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    UINT32                              ModeNumber
);

typedef struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE QueryMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE   SetMode;
    void*                                   Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE       *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

// EDID (Extended Display Identification Data) Protocol - Refresh rate & monitor metadata
typedef struct {
    UINT32 SizeOfEdid;
    UINT8* EdidDataBlock;
} EFI_EDID_ACTIVE_PROTOCOL;

// Memory Descriptor
typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiPersistentMemory,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef struct {
    UINT32          Type;
    UINT32          Pad;
    UINT64          PhysicalStart;
    UINT64          VirtualStart;
    UINT64          NumberOfPages;
    UINT64          Attribute;
} EFI_MEMORY_DESCRIPTOR;

// Boot Services
typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

typedef struct EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr;

    void* RaiseTPL;
    void* RestoreTPL;

    EFI_STATUS (EFIAPI *AllocatePages)(
        EFI_ALLOCATE_TYPE Type,
        EFI_MEMORY_TYPE   MemoryType,
        UINTN             Pages,
        UINT64            *Memory
    );
    EFI_STATUS (EFIAPI *FreePages)(
        UINT64 Memory,
        UINTN  Pages
    );
    EFI_STATUS (EFIAPI *GetMemoryMap)(
        UINTN                 *MemoryMapSize,
        EFI_MEMORY_DESCRIPTOR *MemoryMap,
        UINTN                 *MapKey,
        UINTN                 *DescriptorSize,
        UINT32                *DescriptorVersion
    );
    EFI_STATUS (EFIAPI *AllocatePool)(
        EFI_MEMORY_TYPE PoolType,
        UINTN           Size,
        VOID            **Buffer
    );
    EFI_STATUS (EFIAPI *FreePool)(
        VOID *Buffer
    );

    void* CreateEvent;
    void* SetTimer;
    EFI_STATUS (EFIAPI *WaitForEvent)(
        UINTN     NumberOfEvents,
        EFI_EVENT *Event,
        UINTN     *Index
    );
    void* SignalEvent;
    void* CloseEvent;
    void* CheckEvent;

    void* InstallProtocolInterface;
    void* ReinstallProtocolInterface;
    void* UninstallProtocolInterface;
    EFI_STATUS (EFIAPI *HandleProtocol)(
        EFI_HANDLE ImageHandle,
        EFI_GUID   *Protocol,
        VOID       **Interface
    );
    void* PpcHandleProtocol;
    void* RegisterProtocolNotify;
    EFI_STATUS (EFIAPI *LocateHandle)(
        EFI_LOCATE_SEARCH_TYPE SearchType,
        EFI_GUID               *Protocol,
        VOID                   *SearchKey,
        UINTN                  *BufferSize,
        EFI_HANDLE             *Buffer
    );
    void* LocateDevicePath;
    void* InstallConfigurationTable;

    void* LoadImage;
    void* StartImage;
    void* Exit;
    void* UnloadImage;
    EFI_STATUS (EFIAPI *ExitBootServices)(
        EFI_HANDLE ImageHandle,
        UINTN      MapKey
    );

    void* GetNextMonotonicCount;
    EFI_STATUS (EFIAPI *Stall)(
        UINTN Microseconds
    );
    void* SetWatchdogTimer;

    void* ConnectController;
    void* DisconnectController;

    void* OpenProtocol;
    void* CloseProtocol;
    void* OpenProtocolInformation;

    void* ProtocolsPerHandle;
    EFI_STATUS (EFIAPI *LocateHandleBuffer)(
        EFI_LOCATE_SEARCH_TYPE SearchType,
        EFI_GUID               *Protocol,
        VOID                   *SearchKey,
        UINTN                  *NoHandles,
        EFI_HANDLE             **Buffer
    );
    EFI_STATUS (EFIAPI *LocateProtocol)(
        EFI_GUID *Protocol,
        VOID     *Registration,
        VOID     **Interface
    );
} EFI_BOOT_SERVICES;

// System Table
typedef struct {
    EFI_TABLE_HEADER                Hdr;
    CHAR16                          *FirmwareVendor;
    UINT32                          FirmwareRevision;
    EFI_HANDLE                      ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *ConIn;
    EFI_HANDLE                      ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE                      StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *StdErr;
    void*                           RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;
    UINTN                           NumberOfTableEntries;
    void*                           ConfigurationTable;
} EFI_SYSTEM_TABLE;
