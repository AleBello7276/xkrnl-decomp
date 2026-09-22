#include "init/kdataseg.h"

// PROTDATA
ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackLimit[0x1000];
ALLOC_SECT("PROTDATA") uint8_t KiInitializeKernelStackBase[0x3000];

ALLOC_SECT("PROTDATA") TIMESTAMP_BUNDLE KeTimeStampBundle = {0, 0, 0};

ALLOC_SECT("CLRDATAA") BYTE SataCdRomStaticTransferBuffer[STATIC_TRANSFER_BUFFER_SIZE];
