#pragma once
#include <types.h>

typedef struct HardwareInfo {
    LONG Flags;
    uint8_t NumberOfProcessors;
    uint8_t PCIBridgeRevisionID;
    uint8_t Reserved[6];
    uint16_t BldrMagic;
    uint16_t BldrFlags;
} HardwareInfo;

#define HARDWAREINFO_FLAGS_0x80 0x80

#define HARDWAREINFO_BLDR_0x100 0x100

extern HardwareInfo XboxHardwareInfo;
