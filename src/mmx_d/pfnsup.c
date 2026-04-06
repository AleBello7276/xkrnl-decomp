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

// NON_MATCHING
PfnRegion* MiDecodeMemoryRegionType(int32_t unk0) {
    if (unk0 == 0) {
        if (KeGetCurrentProcessType() == 1) {
            return &MmTitlePfnRegion;
        }
        unk0 = 2;
    }
    if (unk0 == 1) {
        return &MmTitlePfnRegion;
    }
    if (unk0 != 2) {
        return nullptr;
    }
    return &MmSystemPfnRegion;
}

uint32_t MiRemoveAnySmallPage(PfnRegion* region, uint32_t unk1, uint32_t unk2);

void MiRemoveZeroSmallPage(PfnRegion* region, uint32_t unk1, uint32_t unk2) {
    assert(GetKPCR->m_unk_0x18 != 2);

    KeZeroPage(MiRemoveAnySmallPage(region, unk1, unk2));
}

// TODO, continue
uint32_t MiRemoveAnyLargePage(PfnRegion* region, uint32_t unk1, uint32_t unk2) {
    uint32_t uVar2;

    assert(unk1 > 0x12);
    assert(GetKPCR->m_unk_0x18 != 2);
    assert(region->m_unk0x4c == 0);
    assert(region->unk0[32] == 0xfffe);

    uVar2 = region->unk0[32] * 8;
    if (uVar2 < region->m_unk0xec || region->m_unk0xf0 < uVar2) {
        if ((region->m_unk0xfc == 0 && region->m_unk0x100 == 0) || uVar2 < region->m_unk0xfc
            || region->m_unk0x100 < uVar2) {
            assert(1);
        }
    }
}

uint32_t MiRemoveZeroLargePage(PfnRegion* region, uint32_t unk1, uint32_t unk2) {
    const uint32_t PAGE_COUNT = 16;
    char test;
    uint32_t count;
    uint32_t i;

    assert(GetKPCR->m_unk_0x18 != 2);

    count = MiRemoveAnyLargePage(region, unk1, unk2);
    for (i = 0; i < PAGE_COUNT; i++) {
        KeZeroPage(i + count);
    }
    return count;
}
