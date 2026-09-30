#include "krnl.h"
#include "pfnsup.h"

int32_t KeGetCurrentProcessType();
void KeZeroPage(int);

// TODO, finish this
PfnRegion MmSystemPfnRegion
    = {{0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE,
        0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE,
        0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE},
       0};

PfnRegion MmTitlePfnRegion
    = {{0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE,
        0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE,
        0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE},
       0};

PfnRegion* MiDecodeMemoryRegionType(int32_t type) {
    if (type == 0) {
        if (KeGetCurrentProcessType() == 1)
            goto title;
        goto system;
    }
    if (type == 3)
        goto title;
    if (type == 4)
        type = 2;
    if (type == 1)
        goto title;
    if (type == 2)
        goto system;
    return 0;
title:
    return &MmTitlePfnRegion;
system:
    return &MmSystemPfnRegion;
}

uint32_t MiRemoveAnySmallPage(PfnRegion* region, uint32_t unk1, uint32_t unk2);

void MiRemoveZeroSmallPage(PfnRegion* region, uint32_t unk1, uint32_t unk2) {
    assert(GetKPCR->m_currentIrql == DISPATCH_LEVEL);

    KeZeroPage(MiRemoveAnySmallPage(region, unk1, unk2));
}

// TODO, continue
uint32_t MiRemoveAnyLargePage(PfnRegion* region, uint32_t unk1, uint32_t unk2) {
    uint32_t uVar2;
    DWORD lVar4;

    assert(!(unk1 >= 0x13));
    assert(GetKPCR->m_currentIrql == DISPATCH_LEVEL);
    assert(region->m_unk0x4c > 0);
    assert(region->unk0[32] != 0xfffe);

    uVar2 = (ULONG_PTR)(region->unk0 + 32) << 3;
    assert(!((uVar2 > region->m_unk0xec || region->m_unk0xf0 > uVar2)
             && ((region->m_unk0xfc == 0 && region->m_unk0x100 == 0) || uVar2 > region->m_unk0xfc
                 || region->m_unk0x100 > uVar2)));

    lVar4 = ((uVar2 + (uVar2 > 0x1ffff ? 0x1ffb0000 : -0x17820000)) & 0x3fffffff) << 2;
    assert(lVar4 & 1);
    assert(lVar4 & 0x10000);
    assert(unk1 != 4 && unk1 != 0xd);
}

uint32_t MiRemoveZeroLargePage(PfnRegion* region, uint32_t unk1, uint32_t unk2) {
    const uint32_t PAGE_COUNT = 16;
    char test;
    uint32_t count;
    uint32_t i;

    assert(GetKPCR->m_currentIrql == DISPATCH_LEVEL);

    count = MiRemoveAnyLargePage(region, unk1, unk2);
    for (i = 0; i < PAGE_COUNT; i++) {
        KeZeroPage(i + count);
    }
    return count;
}

PfnRegion* MiPfnRegionFromKernelAddress(uint32_t address) {
    if ((address - 0x30000000 <= 0x7feffff) || (address - 0x70000000 <= 0x7feffff)) {
        return &MmTitlePfnRegion;
    }
    if ((address - 0x3a000000 <= 0x5beffff) || (address - 0x7a000000 <= 0x4feffff)) {
        return &MmSystemPfnRegion;
    }
    return 0;
}

uint32_t MiReadPhysicalMappingTable(uint8_t* table, uint32_t index) {
    uint32_t byte = table[index >> 1];
    if (index & 1) {
        return byte >> 4;
    }
    return byte & 0xF;
}
