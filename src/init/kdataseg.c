#include "init/kdataseg.h"

// PROTDATA
ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackLimit[0x1000];
ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackBase[0x3000];
