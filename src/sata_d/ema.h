#pragma once

#include <krnl.h>
#include <types.h>

#include "sata.h"

#define EMA_MAGIC 0x454d4120 /* "EMA " in ascii*/

// TODO: TBD
typedef struct _SATA_EMA_BLOCK {
    USHORT Size;                // +0x00  = 0x24
    UCHAR Reserved_0x02[5];     // +0x02
    UCHAR Flags;                // +0x07  = 1
    ULONG TransferLength;       // +0x08  = 0x800
    UCHAR Reserved_0x0C[4];     // +0x0C
    PVOID Buffer;               // +0x10  = &SataCdRomAP21ScratchBuffer
    UCHAR Reserved_0x14[0x10];  // +0x14 .. +0x24
} SATA_EMA_BLOCK;               // sizeof == 0x24

NTSTATUS EmaExecuteSingle(PVOID Param1);

NTSTATUS EmaExecute(PVOID Param1);
