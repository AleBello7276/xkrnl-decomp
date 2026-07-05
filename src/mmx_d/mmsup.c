#include "mmsup.h"

#include "intrinsics.h"
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
/*
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
    */

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
