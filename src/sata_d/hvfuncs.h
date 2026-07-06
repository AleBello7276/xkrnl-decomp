#pragma once
#include "krnl.h"
#include "sata.h"
#include "types.h"

extern PVOID MmGetPhysicalAddress(PVOID);

void HvxDvdAuthBuildNVPage(uint32_t unk);
void* HvxDvdAuthVerifyNVPage(void* unk, void* unk2);
void* HvxDvdAuthRecordAuthenticationPage(void* unk);
void* HvxDvdAuthRecordXControl(void* unk, void* unk2);
void* HvxDvdAuthGetAuthPage();
void* HvxDvdAuthVerifyAuthPage();
void* HvxDvdAuthGetNextLBAIndex();
void* HvxDvdAuthVerifyLBA(PVOID, PVOID, PVOID);
void* HvxDvdAuthClearDiscAuthInfo();
void* HvxDvdAuthTestMode();
void* HvxDvdAuthGetAuthResults();
void* HvxDvdAuthSetDriveAuthResult();
void* HvxDvdAuthSetDiscAuthResult();
void* HvxDvdAuthFwcr();
void* HvxDvdAuthEx();

NTSTATUS SataCdRomHvMarshal(u32, u32, u32, u32, PVOID, size_t, u32);
u32 SataCdRomHvAuthExMarshal(u32, u32, u32, s64);

NTSTATUS SataCdRomHvBuildNVPage(u32 unk1);
NTSTATUS SataCdRomHvVerifyNVPage(u32 unk1, u32 unk2);

NTSTATUS SataCdRomHvRecordAuthenticationPage(u32 unk1);
NTSTATUS SataCdRomHvRecordXControl(u32 unk1, u32 unk2);
NTSTATUS SataCdRomHvGetAuthPage(u32 unk1);
NTSTATUS SataCdRomHvVerifyAuthPage(u32 unk1, u32 unk2, u32 unk3, u32 unk4);

NTSTATUS SataCdRomHvClearDiscAuthInfo(u32 unk1);
NTSTATUS SataCdRomHvGetNextLBAIndex(u32 unk1);
u32 SataCdRomHvVerifyLBA(PVOID unk1, PVOID unk2, uint32_t* unk3);
NTSTATUS SataCdRomHvGetXControlResults(PVOID buff, size_t size);
void* SataCdRomHvXGD2TestMode();

NTSTATUS SataCdRomHvAuthExInitialize();
NTSTATUS SataCdRomHvAuthExNext(u32 unk1);
