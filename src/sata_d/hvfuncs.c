#include "hvfuncs.h"

ALLOC_SECT("CLRDATAA") uint32_t SataCdRomHvVerifyComplete;

void SataCdRomHvInit() {}

void HvxDvdAuthBuildNVPage(uint32_t unk) {
    _asm {
        li r0, 0x25
        sc
        blr
    }
}

void* HvxDvdAuthVerifyNVPage(void* unk, void* unk2) {
    _asm {
        li r0, 0x26
        sc
        blr
    }
}

void* HvxDvdAuthRecordAuthenticationPage(void* unk) {
    _asm {
        li r0, 0x27
        sc
        blr
    }
}
void* HvxDvdAuthRecordXControl(void* unk, void* unk2) {
    _asm {
        li r0, 0x28
        sc
        blr
    }
}

void* HvxDvdAuthGetAuthPage() {
    _asm { li r0, 0x29
    sc
blr}
}
void* HvxDvdAuthVerifyAuthPage() {
    _asm { 
        li r0, 0x2a
        sc
        blr
    }
}

void* HvxDvdAuthGetNextLBAIndex() {
    _asm { 
        li r0, 0x2b
        sc
        blr
    }
}

void* HvxDvdAuthVerifyLBA(PVOID unk1, PVOID unk2, PVOID unk3) {
    _asm { 
        li r0, 0x2c
        sc
        blr
    }
}

void* HvxDvdAuthClearDiscAuthInfo() {
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

void* HvxDvdAuthGetAuthResults() {
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

void* HvxDvdAuthSetDiscAuthResult() {
    _asm { 
        li r0, 0x5e
        sc
        blr
    }
}

void* HvxDvdAuthFwcr() {
    _asm { 
        li r0, 0x72
        sc
        blr
    }
}

void* HvxDvdAuthEx() {
    _asm { 
        li r0, 0x74
        sc
        blr
    }
}

NTSTATUS SataCdRomHvBuildNVPage(u32 unk1) {
    return SataCdRomHvMarshal(0x25, 0, 0, 0, unk1, 0x4a, 0);
}

NTSTATUS SataCdRomHvVerifyNVPage(u32 unk1, u32 unk2) {
    return SataCdRomHvMarshal(0x26, unk2, unk1, 0x4a, 0, 0, 0);
}

NTSTATUS SataCdRomHvRecordAuthenticationPage(u32 unk1) {
    return SataCdRomHvMarshal(0x27, 0, unk1, 0x2a, 0, 0, 0);
}

NTSTATUS SataCdRomHvRecordXControl(u32 unk1, u32 unk2) {
    return SataCdRomHvMarshal(0x28, 0, unk1, 0x804, unk2, 4, 0);
}

NTSTATUS SataCdRomHvGetAuthPage(u32 unk1) {
    return SataCdRomHvMarshal(0x29, 0, 0, 0, unk1, 0x22, 0);
}

NTSTATUS SataCdRomHvVerifyAuthPage(u32 unk1, u32 unk2, u32 unk3, u32 unk4) {
    return SataCdRomHvMarshal(0x2a, unk2, unk1, 0x22, unk3, 4, unk4);
}

NTSTATUS SataCdRomHvClearDiscAuthInfo(u32 unk1) {
    return SataCdRomHvMarshal(0x2d, unk1, 0, 0, 0, 0, 0);
}

NTSTATUS SataCdRomHvGetNextLBAIndex(u32 unk1) {
    return SataCdRomHvMarshal(0x2b, 0, 0, 0, unk1, 4, 0);
}

u32 SataCdRomHvVerifyLBA(PVOID unk1, PVOID unk2, uint32_t* unk3) {
    PVOID addr1;
    PVOID addr2;
    void* idk;
    SataCdRomHvVerifyComplete = *unk3;
    addr1 = MmGetPhysicalAddress(&SataCdRomHvVerifyComplete);
    addr2 = MmGetPhysicalAddress(unk1);
    idk = HvxDvdAuthVerifyLBA(addr2, unk2, addr1);
    *unk3 = SataCdRomHvVerifyComplete;
    return (u32)idk;
}

NTSTATUS SataCdRomHvGetXControlResults(PVOID buff, size_t size) {
    const u32 SOME_CONSTOF = 0x804;

    if ((buff == nullptr) || size < SOME_CONSTOF)
        return STATUS_INVALID_PARAMETER;

    memset(buff, 0, size);
    return SataCdRomHvMarshal(0x5c, 1, 0, 0, buff, 0x804, 0);
}

void* SataCdRomHvXGD2TestMode() {
    return HvxDvdAuthTestMode();
}

NTSTATUS SataCdRomHvAuthExInitialize() {
    s32 res = SataCdRomHvAuthExMarshal(4, 0, 0, 0);

    if (res == (s32)0xc8000097)
        return STATUS_ACCESS_DENIED;
    if (res == (s32)0xc8000098)
        return STATUS_NOT_FOUND;
    if (res != 0)
        return STATUS_TOO_MANY_SECRETS;

    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomHvAuthExNext(u32 unk1) {
    s32 res = SataCdRomHvAuthExMarshal(5, unk1, 0x22, 0);

    if (res == (s32)0xc800009a)
        return STATUS_NO_MORE_ENTRIES;
    if (res != 0)
        return STATUS_TOO_MANY_SECRETS;

    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomHvAuthExVerify(u32 unk1, s32 unk2) {
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
