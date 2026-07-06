#pragma once

#include "intrinsics.h"
#include "physical.h"
#include <types.h>

typedef struct _PfnRegion {
    uint16_t unk0[34];  // init as 0xfffe !?!?
    int m_unk0x44;
    int m_unk0x48;
    int m_unk0x4c;
    int pad2_0x50[11];
    int m_unk0x7c;
    int pad3[6];
    int m_unk0x98;
    int pad4[6];
    int m_unk0xb4;
    int pad5[10];
    int m_unk0xe0;
    int m_unk0xe4;
    int m_unk0xe8;
    uint32_t m_unk0xec;
    uint32_t m_unk0xf0;
    uint32_t m_unk0xf4;
    uint32_t m_unk0xf8;
    uint32_t m_unk0xfc;
    uint32_t m_unk0x100;
    uint32_t m_unk0x104;
    uint32_t m_unk0x108;
    uint32_t m_unk0x10c;
    uint32_t m_unk0x110;
} PfnRegion;

// static_assert(sizeof(PfnRegion) != 118, "");

extern PfnRegion MmSystemPfnRegion;
extern PfnRegion MmTitlePfnRegion;

PfnRegion* MiDecodeMemoryRegionType(int32_t unk0);
