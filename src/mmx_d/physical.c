#include "physical.h"

#include "krnl.h"

void HvxFlushDcacheRange(uint32_t startAddr, uint32_t size) {
    __asm {
        li r0, 0xc
        sc
        blr
    }
}

KIRQL KfAcquireSpinLock(KSPIN_LOCK* lock);
void KfReleaseSpinLock(KSPIN_LOCK* lock, KIRQL irql);

uint32_t MmQueryAllocationSize(void* address) {
    uint64_t addr = (uint64_t)address;
    uint32_t pageSize = PAGE_4KB;
    KIRQL irql;

    assert(GetKPCR->m_currentIrql <= 2);

    irql = KfAcquireSpinLock(&MmGlobalLock);

    if (addr < 0x80000000) {
    }

    KfReleaseSpinLock(&MmGlobalLock, irql);
}

void* MmAllocatePhysicalMemory(uint32_t unk0, uint32_t unk1, uint32_t unk2) {
    return MmAllocatePhysicalMemoryEx(unk0, unk1, unk2, 0, -1, 0);
}
