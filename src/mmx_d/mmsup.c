#include "mmsup.h"

#include "krnl.h"
#include "pfnsup.h"

uint32_t MiGetPteAddress(uint32_t address) {
    if (address - 0x40000000U <= 0x3FFFFFFF) {
        uint32_t index = (address >> 16) & 0x3FFF;
        return (index + 0xE87DC000) << 2;
    }
    return ((address >> 10) & 0x3FFFFC) + 0x3FC00000;
}

uint32_t MiGetMappedPteAddress(uint32_t address) {
    if (address - 0x40000000U <= 0x3FFFFFFF) {
        uint32_t index = (address >> 16) & 0x3FFF;
        return (index + 0xE87DC000) << 2;
    }
    {
        uint32_t entry = MmPdeDatabase[address >> 22];
        if (entry & 1) {
            return ((address >> 10) & 0x3FFFFC) + 0x3FC00000;
        }
        return 0;
    }
}

typedef struct _FLUSH_VA_LIST {
    uint32_t m_entries;
    void* m_data;
} FLUSH_VA_LIST;

void MiFlushFlushVaList(FLUSH_VA_LIST* param_1) {
    uint32_t count = param_1->m_entries;
    // uint32_t* lsist = param_1;
    param_1->m_entries = 0;

    if (count == 0)
        return;

    KeFlushMultipleTb(count, &param_1->m_data);
}

uint32_t MiDecodePteProtectionMask(uint32_t pte) {
    uint32_t protect = MmPteProtectionMaskToProtect[(pte >> 1) & 7];
    if (!(pte & 1)) {
        protect |= 0x100;
        if (!(pte & 0x400)) {
            protect = 1;
        }
    }
    if (!(pte & 0x40))
        return protect;
    if (pte & 0x10) {
        protect |= 0x200;
    } else {
        protect |= 0x400;
    }
    return protect;
}

int MiMakePhysicalProtectionMask(uint32_t protect, uint32_t* output) {
    uint32_t has200;
    uint32_t has2;
    uint32_t mask;

    if ((protect & ~0x6) & ~0x600)
        return 0;

    has200 = protect & 0x200;
    if (has200) {
        if (protect & 0x400)
            return 0;
    }

    has2 = protect & 0x2;
    if (has2) {
        if (protect & 0x4)
            return 0;
    }

    if (!(protect & 0x6))
        return 0;

    mask = has2 ? 3 : 2;

    if (protect & 0x400) {
        mask |= 0x4;
    } else if (has200) {
        mask |= 0xC;
    }

    {
        int ret = 1;
        *output = mask;
        return ret;
    }
}

bool MmIsDevkitMemoryPresent() {
    return MmDevkitMemoryPresent;
}

bool MmIsAddressValid(uint32_t address) {
    uint32_t pte;
    uint8_t* r3;
    uint32_t r4;
    uint32_t result;

    if (address <= 0x7FFFFFFF) {
        uint32_t pteAddr = MiGetMappedPteAddress(address);
        if (pteAddr == 0)
            goto invalid;
        pte = *(uint32_t*)pteAddr;
        if (pte & 1)
            return 1;
        goto invalid;
    }

    if (address + 0x80000000U <= 0x1FFFFFFF) {
        return (HvxGetImagePageTableEntry(address) >> 3) & 1;
    }

    if (address + 0x20000000U <= 0x1FFFEFFF) {
        uint32_t pageNum = (address >> 12) & 0x1FFFF;
        uint32_t pteAddr;
        if (pageNum + 1 < 0x20000) {
            pteAddr = pageNum - 0x17820000;
        } else {
            pteAddr = pageNum + 0x1FFB0000;
        }
        pte = *(uint32_t*)((pteAddr + 1) << 2);
        if (!(pte & 1))
            goto invalid;
        if (pte & 2)
            return 1;
        goto invalid;
    }

    if (address + 0x60000000U <= 0x1FFFFFFF) {
        r3 = MmPhysical64KBMappingTable;
        r4 = (address + 0x60000000U) >> 16;
        goto call_table;
    }

    {
        uint32_t shifted = address + 0x40000000U;
        if (shifted > 0x1FFFFFFF)
            goto invalid;
        r3 = (uint8_t*)MmPdeDatabase + 0xC00;
        r4 = shifted >> 24;
    }
call_table:
    result = MiReadPhysicalMappingTable(r3, r4);
    goto normalize;

invalid:
    result = 0;
normalize:
    return result ? 1 : 0;
}

uint32_t MmQueryMemoryRegionType(uint32_t address) {
    if (address + 0x6E000000U <= 0xDFEFFFF)
        goto ret1;
    if (address - 0x10000U <= 0x2FFDFFFF)
        goto ret1;
    if (address - 0x30000000U <= 0x7FEFFFF)
        goto ret1;
    if (address - 0x70000000U > 0x7FEFFFF)
        return 2;
ret1:
    return 1;
}

uint32_t MmGetPoolPagesType(uint32_t address) {
    if (address - 0x3A000000U <= 0x5BEFFFF) {
        return 2;
    }
    assert(address - 0x30000000U <= 0x7FEFFFF);
    return 1;
}

void MmUnmapMemory(uint32_t address, uint32_t size) {
    KSPIN_LOCK* lock = &MmGlobalLock;
    KIRQL irql = KfAcquireSpinLock(lock);
    MiUnmapPages(address, size);
    KfReleaseSpinLock(lock, irql);
}

void MmDoubleMapMemory(uint32_t srcAddr, uint32_t dstAddr, uint32_t numPages, uint32_t flags) {
    uint32_t pteBase;
    uint32_t endPte;
    KSPIN_LOCK* lock;
    KIRQL irql;

    assert(GetKPCR->m_currentIrql < 2);
    assert(srcAddr != 0 && (srcAddr & 0xFFF) == 0 && srcAddr <= 0x3FFFFFFF);
    assert(dstAddr != 0 && (dstAddr & 0xFFF) == 0 && dstAddr <= 0x7FFFFFFF);
    assert(numPages != 0);

    pteBase = ((srcAddr >> 10) & 0x3FFFFC) + 0x3FC00000;
    lock = &MmGlobalLock;
    endPte = pteBase + numPages * 4 - 4;

    irql = KfAcquireSpinLock(lock);
    while (pteBase <= endPte) {
        MiDoubleMapPage(pteBase, dstAddr, flags);
        pteBase += 4;
        dstAddr += 0x1000;
    }
    KfReleaseSpinLock(lock, irql);
}

uint32_t MmResetLowestAvailablePages(uint32_t* titleSmallOld, uint32_t* systemSmallOld,
                                     uint32_t* titleLargeOld, uint32_t* systemLargeOld) {
    KIRQL irql = KfAcquireSpinLock(&MmGlobalLock);

    if (titleSmallOld) {
        *titleSmallOld = MmTitlePfnRegion.m_unk0xf4;
        MmTitlePfnRegion.m_unk0xf4 = (uint32_t)-1;
        MmTitlePfnRegion.m_unk0x10c = (uint32_t)-1;
        MmTitlePfnRegion.m_unk0x110 = (uint32_t)-1;
    }

    if (titleLargeOld) {
        *titleLargeOld = MmTitlePfnRegion.m_unk0xf8;
        MmTitlePfnRegion.m_unk0xf8 = 0;
    }

    if (systemSmallOld) {
        *systemSmallOld = MmSystemPfnRegion.m_unk0xf4;
        MmSystemPfnRegion.m_unk0xf4 = (uint32_t)-1;
        MmSystemPfnRegion.m_unk0x10c = (uint32_t)-1;
        MmSystemPfnRegion.m_unk0x110 = (uint32_t)-1;
    }

    if (systemLargeOld) {
        *systemLargeOld = MmSystemPfnRegion.m_unk0xf8;
        MmSystemPfnRegion.m_unk0xf8 = 0;
    }

    KfReleaseSpinLock(&MmGlobalLock, irql);
    return 0;
}

void MiQueryStatistics(uint32_t* out) {
    uint32_t structSize;

    assert(GetKPCR->m_currentIrql == 2);

    structSize = out[0];

    out[1] = MmNumberOfPhysicalPages;
    out[2] = MmAllocatedPagesByUsage[0];
    out[3] = MmTitlePfnRegion.m_unk0x48;
    out[4] = 0x2FFE0000;
    out[5] = MmTitlePfnRegion.m_unk0xb4 + MmTitlePfnRegion.m_unk0x98 + MmTitlePfnRegion.m_unk0x7c;
    out[6] = MmAllocatedPagesByUsage[1];
    out[7] = MmAllocatedPagesByUsage[9];
    out[8] = MmAllocatedPagesByUsage[8];
    out[9] = MmAllocatedPagesByUsage[6];
    out[10] = MmAllocatedPagesByUsage[7];
    out[11] = MmAllocatedPagesByUsage[5];
    out[12] = MmAllocatedPagesByUsage[3] + MmAllocatedPagesByUsage[4];
    out[13] = MmAllocatedPagesByUsage[2];

    out[14] = MmSystemPfnRegion.m_unk0x48;
    out[15] = 0x01FF0000;
    out[16] = MmSystemPfnRegion.m_unk0xb4 + MmSystemPfnRegion.m_unk0x98 + MmSystemPfnRegion.m_unk0x7c;
    out[17] = MmAllocatedPagesByUsage[10];
    out[18] = MmAllocatedPagesByUsage[18];
    out[19] = MmAllocatedPagesByUsage[17];
    out[20] = MmAllocatedPagesByUsage[15];
    out[21] = MmAllocatedPagesByUsage[16];
    out[22] = MmAllocatedPagesByUsage[14];
    out[23] = MmAllocatedPagesByUsage[13] + MmAllocatedPagesByUsage[12];
    out[24] = MmAllocatedPagesByUsage[11];

    out[25] = MmHighestPhysicalPage;

    if (structSize < 0x70)
        return;

    out[26] = MmTitlePfnRegion.m_unk0xf4;
    out[27] = MmSystemPfnRegion.m_unk0xf4;
}

uint32_t MmQueryStatistics(uint32_t* buf) {
    if (buf[0] < 0x68)
        return 0xC000000D;

    {
        KSPIN_LOCK* lock = &MmGlobalLock;
        KIRQL irql = KfAcquireSpinLock(lock);
        MiQueryStatistics(buf);
        KfReleaseSpinLock(lock, irql);
    }

    if (KeDebugMonitorData) {
        typedef void (*DebugCallback)(uint32_t, uint32_t*);
        DebugCallback callback = (DebugCallback)(((uint32_t*)KeDebugMonitorData)[6]);
        callback(0x86, buf);
    }

    return 0;
}

uint32_t MmQueryDevkitStatistics(uint32_t* buf) {
    uint32_t* out = buf;

    if (out[0] < 0x98)
        goto error;

    {
        uint32_t* field4 = out + 1;
        if (*field4 != 0x70)
            goto error;

        {
            KIRQL irql = KfAcquireSpinLock(&MmGlobalLock);
            MiQueryStatistics(field4);
            KfReleaseSpinLock(&MmGlobalLock, irql);
        }

        out[0x74 / 4] = MmTitlePfnRegion.m_unk0x104;
        out[0x78 / 4] = MmNumberOfTitleDebugPages;
        out[0x7c / 4] = MmSystemPfnRegion.m_unk0x104;
        out[0x80 / 4] = MmNumberOfSystemDebugPages;

        out[0x88 / 4] = MmTitlePfnRegion.m_unk0xf0 - MmTitlePfnRegion.m_unk0xec + 1;

        if (MmTitlePfnRegion.m_unk0xfc != 0 && MmTitlePfnRegion.m_unk0x100 != 0) {
            out[0x8c / 4] = MmTitlePfnRegion.m_unk0x100 - MmTitlePfnRegion.m_unk0xfc + 1;
        } else {
            out[0x8c / 4] = 0;
        }

        out[0x90 / 4] = MmSystemPfnRegion.m_unk0xf0 - MmSystemPfnRegion.m_unk0xec + 1;

        if (MmSystemPfnRegion.m_unk0xfc != 0 && MmSystemPfnRegion.m_unk0x100 != 0) {
            out[0x94 / 4] = MmSystemPfnRegion.m_unk0x100 - MmSystemPfnRegion.m_unk0xfc + 1;
        } else {
            out[0x94 / 4] = 0;
        }

        out[0x84 / 4] = 0x20000;
        out[0] = 0x98;

        return 0;
    }

error:
    return 0xC000000D;
}

void MmSetFsCacheElementIndex(uint32_t address, uint32_t index) {
    KSPIN_LOCK* lock = &MmGlobalLock;
    KIRQL irql = KfAcquireSpinLock(lock);

    {
        uint32_t pteOffset = (address >> 10) & 0x3FFFFC;
        uint32_t pte = *(uint32_t*)(pteOffset + 0x3FC00000);
        uint32_t pageNum = pte >> 12;
        uint32_t pfnIndex;
        uint32_t* pfnPtr;
        uint32_t pfn;

        if (pageNum < 0x20000) {
            pfnIndex = pageNum - 0x17820000;
        } else {
            pfnIndex = pageNum + 0x1FFB0000;
        }
        pfnPtr = (uint32_t*)(pfnIndex << 2);
        pfn = *pfnPtr;

        assert(pfn & 0x10000);
        {
            uint32_t usage = pfn >> 27;
            assert(usage == 2 || usage == 0xb);
        }
        assert(pfn & 0xFFFC);

        pfn = *pfnPtr;
        pfn = (pfn & ~0x07FE0000) | ((index << 17) & 0x07FE0000);
        pfn = (pfn & ~0xFFFC) | ((((pfn >> 2) - 1) << 2) & 0xFFFC);
        *pfnPtr = pfn;
    }

    KfReleaseSpinLock(lock, irql);
}

void MiZeroAndFlushPtes(uint32_t pteBase, uint32_t count) {
    // struct {
    //     uint32_t count;
    //     uint32_t entries[32];
    // } flush;
    // uint32_t endPte;
    // uint32_t currentPte;
    //
    // assert(pteBase <= 0x3FFFFFFF);
    //
    // flush.count = 0;
    // endPte = pteBase + count * 4;
    // currentPte = pteBase;
    //
    // while (currentPte < endPte) {
    //     *(uint32_t*)currentPte = 0;
    //     flush.entries[flush.count] = currentPte << 10;
    //     flush.count++;
    //     if (flush.count == 32) {
    //         flush.count = 0;
    //         KeFlushMultipleTb((void*)32, flush.entries);
    //     }
    //     currentPte += 4;
    // }
    //
    // if (flush.count != 0) {
    //     KeFlushMultipleTb((void*)flush.count, flush.entries);
    // }
}
