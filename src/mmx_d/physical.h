#pragma once
#include <krnl.h>
#include <types.h>

#define PAGE_4KB 0x1000

// Must Be __Linker__*
static KSPIN_LOCK MmGlobalLock;

void HvxFlushDcacheRange(uint32_t startAddr, uint32_t size);

uint32_t MmQueryAllocationSize(void* address);

void* MmAllocatePhysicalMemoryEx(uint32_t unk0, uint32_t unk1, uint32_t unk2, uint32_t unk3, uint32_t unk4,
                                 uint32_t unk5);

void* MmAllocatePhysicalMemory(uint32_t unk0, uint32_t unk1, uint32_t unk2);
