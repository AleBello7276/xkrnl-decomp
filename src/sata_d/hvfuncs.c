#include "hvfuncs.h"

#include "init/kdataseg.h"
#include "ke_d/ke.h"

ALLOC_SECT("CLRDATAA") BOOL SataCdRomHvVerifyComplete;

void SataCdRomHvInit() {
    SataCdRomDvdAuthBufferLock = NULL;
    SataCdRomDvdAuthBufferPhysical = MmGetPhysicalAddress(&SataCdRomDvdAuthBuffer);
}

void HvxDvdAuthBuildNVPage(uint32_t unk) {
    _asm {
        li r0, 0x25
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthVerifyNVPage(ULONG_PTR Address, QWORD Argument) {
    _asm {
        li r0, 0x26
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthRecordAuthenticationPage(ULONG_PTR Address) {
    _asm {
        li r0, 0x27
        sc
        blr
    }
}
NTSTATUS HvxDvdAuthRecordXControl(ULONG_PTR Start, ULONG_PTR End) {
    _asm {
        li r0, 0x28
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthGetAuthPage(ULONG_PTR Address) {
    _asm { 
        li r0, 0x29
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthVerifyAuthPage(ULONG_PTR Start, QWORD Argument, ULONG_PTR End, QWORD Argument2) {
    _asm { 
        li r0, 0x2a
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthGetNextLBAIndex(ULONG_PTR Address) {
    _asm { 
        li r0, 0x2b
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthVerifyLBA(ULONG_PTR unk1, ULONG_PTR unk2, ULONG_PTR unk3) {
    _asm { 
        li r0, 0x2c
        sc
        blr
    }
}

PVOID HvxDvdAuthClearDiscAuthInfo(QWORD Argument) {
    _asm { 
        li r0, 0x2d
        sc
        blr
    }
}

void* HvxDvdAuthTestMode() {
    _asm { 
        li r0, 0x41
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthGetAuthResults(ULONG_PTR Address, QWORD Argument) {
    _asm { 
        li r0, 0x5c
        sc
        blr
    }
}

void* HvxDvdAuthSetDriveAuthResult() {
    _asm { 
        li r0, 0x5d
        sc
        blr
    }
}

PVOID HvxDvdAuthSetDiscAuthResult(ULONG_PTR Address) {
    _asm { 
        li r0, 0x5e
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthFwcr(QWORD Argument, ULONG_PTR Idk, ULONG_PTR Address) {
    _asm { 
        li r0, 0x72
        sc
        blr
    }
}

NTSTATUS HvxDvdAuthEx(QWORD Function, ULONG PhysicalAddress, ULONG BufferLength, QWORD Argument,
                      QWORD Reserved) {
    _asm { 
        li r0, 0x74
        sc
        blr
    }
}

void SataCdRomHvReportMarshalFailure(CDROM_HV_FUNCTION Function, NTSTATUS Status) {
    DWORD Unk[10];

    if ((Function >= SataCdRomHvRecordAuthenticationPage_e)
        && (Function <= SataCdRomHvGetNextLBAIndex_e || Function == Unknown_e)) {
        Unk[0] = 1;
        Unk[1] = Function;
        Unk[2] = Status;
        SataCdRomHvMarshal(SataCdRomHvSetDiscAuthResult_e, NULL, Unk, sizeof(Unk), NULL, NULL, NULL);
    }
}

NTSTATUS SataCdRomHvMarshal(CDROM_HV_FUNCTION Function, QWORD Argument, PVOID InputBuffer, ULONG InputLength,
                            PVOID OutputBuffer, ULONG OutputLength, QWORD VerifyArgument) {
    const size_t PAGE_4K_SIZE = 0x1000;
    NTSTATUS Status;
    ULONG PhysicalAddress;
    ULONG OutputOffset = InputLength;
    KIRQL OldIrql;

    if ((InputLength + OutputLength) > PAGE_4K_SIZE) {
        SataCdRomHvReportMarshalFailure(Function, STATUS_INVALID_PARAMETER);

        return STATUS_INVALID_PARAMETER;
    }

    OldIrql = KfAcquireSpinLock(&SataCdRomDvdAuthBufferLock);

    PhysicalAddress = (ULONG)SataCdRomDvdAuthBufferPhysical;

    if (InputLength != 0) {
        memcpy(SataCdRomDvdAuthBuffer, InputBuffer, InputLength);
    }

    switch (Function) {
    case SataCdRomHvBuildNVPage_e: {
        HvxDvdAuthBuildNVPage(PhysicalAddress + InputLength);
        Status = STATUS_SUCCESS;
        break;
    }

    case SataCdRomHvVerifyNVPage_e: {
        Status = HvxDvdAuthVerifyNVPage(PhysicalAddress, Argument);
        break;
    }

    case SataCdRomHvRecordAuthenticationPage_e: {
        Status = HvxDvdAuthRecordAuthenticationPage(PhysicalAddress);
        break;
    }

    case SataCdRomHvRecordXControl_e: {
        Status = HvxDvdAuthRecordXControl(PhysicalAddress, PhysicalAddress + InputLength);
        break;
    }

    case SataCdRomHvGetAuthPage_e: {
        Status = HvxDvdAuthGetAuthPage(PhysicalAddress + InputLength);
        break;
    }

    case SataCdRomHvVerifyAuthPage_e: {
        Status = HvxDvdAuthVerifyAuthPage(PhysicalAddress, Argument, PhysicalAddress + InputLength,
                                          VerifyArgument);
        break;
    }

    case SataCdRomHvGetNextLBAIndex_e: {
        Status = HvxDvdAuthGetNextLBAIndex(PhysicalAddress + InputLength);
        break;
    }

    case SataCdRomHvClearDiscAuthInfo_e: {
        HvxDvdAuthClearDiscAuthInfo(Argument);
        Status = STATUS_SUCCESS;
        break;
    }

    case SataCdRomHvGetAuthResults_e: {
        Status = HvxDvdAuthGetAuthResults(PhysicalAddress + InputLength, Argument);
        break;
    }

    case SataCdRomHvSetDiscAuthResult_e: {
        HvxDvdAuthSetDiscAuthResult(PhysicalAddress);
        Status = STATUS_SUCCESS;
        break;
    }

    case SataCdRomHvFwcr_e: {
        OutputOffset = 0;
        Status = HvxDvdAuthFwcr(Argument, GetKPCR->unk_0x100, PhysicalAddress);
        break;
    }

    default: {
        Status = STATUS_INVALID_PARAMETER;
        SataCdRomHvReportMarshalFailure(Unknown_e, STATUS_INVALID_PARAMETER);
        goto Exit;
    }
    }

    if (OutputLength != 0) {
        memcpy(OutputBuffer, SataCdRomDvdAuthBuffer + OutputOffset, OutputLength);
    }

    if (Status != STATUS_SUCCESS) {
        DbgPrint("SATA: HV returned = 0x%x\n", Status);

        Status = STATUS_TOO_MANY_SECRETS;
    }

Exit:

    KfReleaseSpinLock(&SataCdRomDvdAuthBufferLock, OldIrql);

    return Status;
}

NTSTATUS SataCdRomHvAuthExMarshal(QWORD Function, PVOID Buffer, ULONG BufferLength, QWORD Argument) {
    const size_t PAGE_4K_SIZE = 0x1000;
    NTSTATUS Status;
    ULONG PhysicalAddress;
    KIRQL OldIrql;

    if (BufferLength > PAGE_4K_SIZE) {
        SataCdRomHvReportMarshalFailure(Function, STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    OldIrql = KfAcquireSpinLock(&SataCdRomDvdAuthBufferLock);

    if (BufferLength == 0) {
        PhysicalAddress = 0;
    } else {
        memcpy(SataCdRomDvdAuthBuffer, Buffer, BufferLength);
        PhysicalAddress = (ULONG)SataCdRomDvdAuthBufferPhysical;
    }

    Status = HvxDvdAuthEx(Function, PhysicalAddress, BufferLength, Argument, 0);

    if (BufferLength != 0) {
        memcpy(Buffer, SataCdRomDvdAuthBuffer, BufferLength);
    }

    KfReleaseSpinLock(&SataCdRomDvdAuthBufferLock, OldIrql);

    return Status;
}

NTSTATUS SataCdRomHvBuildNVPage(PVOID Buff) {
    const size_t BUFF_SIZE = 0x4a;
    return SataCdRomHvMarshal(SataCdRomHvBuildNVPage_e, NULL, NULL, NULL, Buff, BUFF_SIZE, NULL);
}

NTSTATUS SataCdRomHvVerifyNVPage(PVOID InputBuff, QWORD Argument) {
    const size_t BUFF_SIZE = 0x4a;
    return SataCdRomHvMarshal(SataCdRomHvVerifyNVPage_e, Argument, InputBuff, BUFF_SIZE, NULL, NULL, NULL);
}

NTSTATUS SataCdRomHvRecordAuthenticationPage(PVOID InputBuff) {
    const size_t BUFF_SIZE = 0x2a;
    return SataCdRomHvMarshal(SataCdRomHvRecordAuthenticationPage_e, NULL, InputBuff, BUFF_SIZE, NULL, NULL,
                              NULL);
}

NTSTATUS SataCdRomHvRecordXControl(PVOID InputBuff, PVOID OutBuff) {
    const size_t INPUT_BUFF_SIZE = 0x804;
    const size_t OUT_BUFF_SIZE = 4;
    return SataCdRomHvMarshal(SataCdRomHvRecordXControl_e, NULL, InputBuff, INPUT_BUFF_SIZE, OutBuff,
                              OUT_BUFF_SIZE, NULL);
}

NTSTATUS SataCdRomHvGetAuthPage(PVOID OutBuff) {
    const size_t OUT_BUFF_SIZE = 0x22;
    return SataCdRomHvMarshal(SataCdRomHvGetAuthPage_e, NULL, NULL, NULL, OutBuff, OUT_BUFF_SIZE, NULL);
}

NTSTATUS SataCdRomHvVerifyAuthPage(PVOID InputBuff, QWORD InArgument, PVOID OutBuff, QWORD VerifyArg) {
    const size_t INPUT_BUFF_SIZE = 0x22;
    const size_t OUT_BUFF_SIZE = 4;
    return SataCdRomHvMarshal(SataCdRomHvVerifyAuthPage_e, InArgument, InputBuff, INPUT_BUFF_SIZE, OutBuff,
                              OUT_BUFF_SIZE, VerifyArg);
}

NTSTATUS SataCdRomHvClearDiscAuthInfo(QWORD Arg) {
    return SataCdRomHvMarshal(SataCdRomHvClearDiscAuthInfo_e, Arg, NULL, NULL, NULL, NULL, NULL);
}

NTSTATUS SataCdRomHvGetNextLBAIndex(PVOID OutBuff) {
    const size_t OUT_BUFF_SIZE = 4;
    return SataCdRomHvMarshal(SataCdRomHvGetNextLBAIndex_e, NULL, NULL, NULL, OutBuff, OUT_BUFF_SIZE, NULL);
}

NTSTATUS SataCdRomHvVerifyLBA(PVOID UnkA, ULONG_PTR UnkB, BOOL* BoolOut) {
    ULONG_PTR AddrA;
    ULONG_PTR AddrB;
    NTSTATUS Status;

    SataCdRomHvVerifyComplete = *BoolOut;
    AddrA = MmGetPhysicalAddress(&SataCdRomHvVerifyComplete);
    AddrB = MmGetPhysicalAddress(UnkA);

    Status = HvxDvdAuthVerifyLBA(AddrB, UnkB, AddrA);

    *BoolOut = SataCdRomHvVerifyComplete;
    return Status;
}

NTSTATUS SataCdRomHvGetXControlResults(PVOID OutBuff, size_t Size) {
    const size_t SOME_SIZE = 0x804;

    if ((OutBuff == nullptr) || Size < SOME_SIZE)
        return STATUS_INVALID_PARAMETER;

    memset(OutBuff, 0, Size);
    return SataCdRomHvMarshal(SataCdRomHvGetAuthResults_e, TRUE, NULL, NULL, OutBuff, SOME_SIZE, NULL);
}

void* SataCdRomHvXGD2TestMode() {
    return HvxDvdAuthTestMode();
}

// TODO: figure these out
NTSTATUS SataCdRomHvAuthExInitialize() {
    NTSTATUS Status = SataCdRomHvAuthExMarshal(4, 0, 0, 0);

    if (Status == (NTSTATUS)0xc8000097)
        return STATUS_ACCESS_DENIED;
    if (Status == (NTSTATUS)0xc8000098)
        return STATUS_NOT_FOUND;
    if (Status != 0)
        return STATUS_TOO_MANY_SECRETS;

    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomHvAuthExNext(PVOID unk1) {
    NTSTATUS Status = SataCdRomHvAuthExMarshal(5, unk1, 0x22, 0);

    if (Status == (NTSTATUS)0xc800009a)
        return STATUS_NO_MORE_ENTRIES;
    if (Status != 0)
        return STATUS_TOO_MANY_SECRETS;

    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomHvAuthExVerify(PVOID unk1, s32 unk2) {
    s32 res = SataCdRomHvAuthExMarshal(6, unk1, 0x22, unk2);

    if (res != 0)
        return STATUS_TOO_MANY_SECRETS;

    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomHvAuthExTerminate(u32* unk1) {
    u32 value;
    s32 res = SataCdRomHvAuthExMarshal(7, 0, 0, 0);
    switch (res) {
    case ((s32)0xc800009e):
        value = 2;
        break;
    case ((s32)0xc800009d):
        value = 3;
        break;

    default:
        value = 4;
        break;
    }

    *unk1 = value;
    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomHvXGD2GetAuthResults(PVOID Buff, DWORD Size) {
    const size_t MAX_SIZE = 0x30;

    if (Buff != NULL && MAX_SIZE <= Size)
        return SataCdRomHvMarshal(SataCdRomHvGetAuthResults_e, NULL, NULL, NULL, Buff, MAX_SIZE, NULL);

    return STATUS_INVALID_PARAMETER;
}

void SataCdRomHvSetDriveAuthResult() {
    HvxDvdAuthSetDriveAuthResult();
}
void SataCdRomHvSetDiscAuthResult(PVOID InBuff) {
    const size_t IN_SIZE = 0x28;
    SataCdRomHvMarshal(SataCdRomHvSetDiscAuthResult_e, NULL, InBuff, IN_SIZE, NULL, NULL, NULL);
}

NTSTATUS SataCdRomHvFwcr(QWORD Argument, PVOID BuffA, PVOID BuffB) {
    const size_t SOME_SIZE = 50;
    const size_t SOME_SIZE_2 = 48;
    ULONG_PTR PhysA;
    ULONG_PTR PhysB;
    KIRQL OldIrql;
    NTSTATUS Status;

    OldIrql = KfAcquireSpinLock(&SataCdRomDvdAuthBufferLock);

    PhysA = MmGetPhysicalAddress(SataCdRomDvdAuthBuffer);
    PhysB = MmGetPhysicalAddress(SataCdRomDvdAuthBuffer + 50);
    memcpy(SataCdRomDvdAuthBuffer, BuffA, 50);
    memcpy(SataCdRomDvdAuthBuffer + 50, BuffB, SOME_SIZE_2);

    Status = HvxDvdAuthFwcr(Argument, PhysA, PhysB);

    memcpy(BuffA, SataCdRomDvdAuthBuffer, 50);
    memcpy(BuffB, SataCdRomDvdAuthBuffer + 50, SOME_SIZE_2);

    KfReleaseSpinLock(&SataCdRomDvdAuthBufferLock, OldIrql);

    return Status;
}
