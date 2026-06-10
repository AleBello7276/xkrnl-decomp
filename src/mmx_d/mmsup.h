#pragma once

#include <types.h>

extern uint16_t MmProtectToPteProtectionMask[16];
extern uint16_t MmPteProtectionMaskToProtect[8];
extern uint8_t MmDevkitMemoryPresent;
extern uint32_t MmPdeDatabase[1024];
extern uint8_t MmPhysical64KBMappingTable[];
extern KSPIN_LOCK MmGlobalLock;
extern uint32_t MmNumberOfPhysicalPages;
extern uint32_t MmAllocatedPagesByUsage[20];
extern uint32_t MmHighestPhysicalPage;
extern uint32_t MmNumberOfTitleDebugPages;
extern uint32_t MmNumberOfSystemDebugPages;
extern void* KeDebugMonitorData;

void KeFlushMultipleTb(uint32_t count_or_ptr, uint32_t* addresses);
uint32_t HvxGetImagePageTableEntry(uint32_t address);
uint32_t MiReadPhysicalMappingTable(uint8_t* table, uint32_t index);
uint8_t KfAcquireSpinLock(KSPIN_LOCK* lock);
void KfReleaseSpinLock(KSPIN_LOCK* lock, uint8_t irql);
void MiUnmapPages(uint32_t address, uint32_t size);
void MiDoubleMapPage(uint32_t pteAddr, uint32_t physAddr, uint32_t flags);
void MiFreeMappedMemory(void* pfnRegion, uint32_t addr, uint32_t size, uint32_t flags);
void MiQueryStatistics(uint32_t* out);
void* MiAllocateMappedMemory(void* pfnRegion, uint32_t usage, uint32_t align, uint32_t size, uint32_t unk4,
                             uint32_t flags);
void RtlFillMemoryUlong(uint32_t base, uint32_t size, uint32_t fill);
int32_t KeGetCurrentProcessType(void);
