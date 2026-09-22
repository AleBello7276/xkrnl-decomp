#pragma once

#include "cdrom.h"
#include "krnl.h"
#include "types.h"

// TODO: sort
extern ULONG_PTR MmGetPhysicalAddress(PVOID);

/* */
void SataCdRomHvInit();

/* */
void HvxDvdAuthBuildNVPage(uint32_t unk);

/* */
NTSTATUS HvxDvdAuthVerifyNVPage(ULONG_PTR Address, QWORD Argument);

/* */
NTSTATUS HvxDvdAuthRecordAuthenticationPage(ULONG_PTR Address);

/* */
NTSTATUS HvxDvdAuthRecordXControl(ULONG_PTR Start, ULONG_PTR End);

/* */
NTSTATUS HvxDvdAuthGetAuthPage(ULONG_PTR Address);

/* */
NTSTATUS HvxDvdAuthVerifyAuthPage(ULONG_PTR Start, QWORD Argument, ULONG_PTR End, QWORD Argument2);

/* */
NTSTATUS HvxDvdAuthGetNextLBAIndex(ULONG_PTR Address);

/* */
NTSTATUS HvxDvdAuthVerifyLBA(ULONG_PTR, ULONG_PTR, ULONG_PTR);

/* */
PVOID HvxDvdAuthClearDiscAuthInfo(QWORD Argument);

/* */
PVOID HvxDvdAuthTestMode();

/* */
NTSTATUS HvxDvdAuthGetAuthResults(ULONG_PTR Address, QWORD Argument);

/* */
PVOID HvxDvdAuthSetDriveAuthResult();

/* */
PVOID HvxDvdAuthSetDiscAuthResult(ULONG_PTR Address);

/* */
NTSTATUS HvxDvdAuthFwcr(QWORD Argument, ULONG_PTR Idk, ULONG_PTR Address);

/* */
NTSTATUS HvxDvdAuthEx(QWORD Function, ULONG PhysicalAddress, ULONG BufferLength, QWORD Argument,
                      QWORD Reserved);

/* */
void SataCdRomHvReportMarshalFailure(CDROM_HV_FUNCTION Function, NTSTATUS Status);

/* */
NTSTATUS SataCdRomHvMarshal(CDROM_HV_FUNCTION Function, ULONGLONG Argument, PVOID InputBuffer,
                            ULONG InputLength, PVOID OutputBuffer, ULONG OutputLength, QWORD VerifyArgument);

/* */
NTSTATUS SataCdRomHvBuildNVPage(PVOID Buff);

/* */
NTSTATUS SataCdRomHvVerifyNVPage(PVOID InputBuff, QWORD Argument);

/* */
NTSTATUS SataCdRomHvRecordAuthenticationPage(PVOID InputBuff);

/* */
NTSTATUS SataCdRomHvRecordXControl(PVOID InputBuff, PVOID OutBuff);

/* */
NTSTATUS SataCdRomHvGetAuthPage(PVOID OutBuff);

/* */
NTSTATUS SataCdRomHvVerifyAuthPage(PVOID InputBuff, QWORD InArgument, PVOID OutBuff, QWORD VerifyArg);

/* */
NTSTATUS SataCdRomHvClearDiscAuthInfo(QWORD Arg);

/* */
NTSTATUS SataCdRomHvGetNextLBAIndex(PVOID OutBuff);

/* */
NTSTATUS SataCdRomHvAuthExMarshal(QWORD Function, PVOID Buffer, ULONG BufferLength, QWORD Argument);

/* */
NTSTATUS SataCdRomHvVerifyLBA(PVOID UnkA, ULONG_PTR UnkB, BOOL* BoolOut);

/* */
NTSTATUS SataCdRomHvGetXControlResults(PVOID buff, size_t size);

/* */
PVOID SataCdRomHvXGD2TestMode();

/* */
NTSTATUS SataCdRomHvAuthExInitialize();

/* */
NTSTATUS SataCdRomHvAuthExNext(PVOID);

/* */
NTSTATUS SataCdRomHvAuthExVerify(PVOID unk1, s32 unk2);
