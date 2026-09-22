#include "failures.h"

#include "ntosrtl/ntosrtl.h"

LONG lbl_801E4258[272];

// fcking bnelr
void SataCdRomRecordSpeedDecrease(DWORD param_1) {
    if (_InterlockedExchange(&lbl_801E4258[271], TRUE) != 0)
        return;

    assert(param_1 < 5);

    lbl_801E4258[271] = FALSE;
    lbl_801E4258[param_1 + 1]++;
}

// fcking bnelr
void SataCdRomRecordSpeedIncrease(DWORD param_1) {
    if (_InterlockedExchange(&lbl_801E4258[271], TRUE))
        return;

    assert(param_1 < 5);

    lbl_801E4258[271] = FALSE;
    lbl_801E4258[param_1 + 6]++;
}

// fcking bnelr
void SataCdRomRecordIncrementStatistic(DWORD param_1) {
    if (_InterlockedExchange(&lbl_801E4258[271], TRUE))
        return;

    assert(param_1 < 5);

    lbl_801E4258[271] = FALSE;
    lbl_801E4258[param_1 + 11]++;
}

// ugh good enough..
NTSTATUS SataCdRomRecordReset(PVOID param_1) {
    if (_InterlockedExchange(&lbl_801E4258[271], TRUE))
        return STATUS_PENDING;

    RtlMoveMemory(param_1, lbl_801E4258, sizeof(lbl_801E4258) - 4);
    memset(lbl_801E4258, 0, sizeof(lbl_801E4258) - 4);
    lbl_801E4258[271] = FALSE;

    return STATUS_SUCCESS;
}
