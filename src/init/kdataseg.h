//
// word arounds to allocate stuff in custom sections
//

#pragma once
#include <types.h>

#pragma section("PROTDATA", read, write)
extern ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackLimit[0x1000];
extern ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackBase[0x3000];

// https://github.com/xenia-canary/xenia-canary/blob/canary_experimental/src/xenia/kernel/kernel_state.h#L98
typedef struct _TIMESTAMP_BUNDLE {
    LARGE_INTEGER InterruptTime;
    LARGE_INTEGER SystemTime;
    DWORD TickCount;
    LARGE_INTEGER pad;
} TIMESTAMP_BUNDLE;

extern ALLOC_SECT("PROTDATA") TIMESTAMP_BUNDLE KeTimeStampBundle;

#pragma section("CLRDATAA", read, write)

#define STATIC_TRANSFER_BUFFER_SIZE 32
#define DVD_AUTH_BUFFER_SIZE 0x1000

extern ALLOC_SECT("CLRDATAA") BYTE SataCdRomStaticTransferBuffer[STATIC_TRANSFER_BUFFER_SIZE];
extern ALLOC_SECT("CLRDATAA") BYTE SataCdRomDvdAuthBuffer[DVD_AUTH_BUFFER_SIZE];
