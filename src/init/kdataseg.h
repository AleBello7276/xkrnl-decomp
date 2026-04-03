//
// word arounds to allocate stuff in custom sections
//

#pragma once
#include <types.h>

#pragma section("PROTDATA", read, write)
extern ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackLimit[0x1000];
extern ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackBase[0x3000];
