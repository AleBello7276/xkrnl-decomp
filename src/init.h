#pragma once
#include <types.h>

typedef struct HardwareInfo {
    uint32_t Flags;
    uint8_t NumberOfProcessors;
    uint8_t PCIBridgeRevisionID;
    uint8_t Reserved[6];
    uint16_t BldrMagic;
    uint16_t BldrFlags;
} HardwareInfo;

extern HardwareInfo XboxHardwareInfo;
