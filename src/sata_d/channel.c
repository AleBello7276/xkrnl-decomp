#include "channel.h"

#include "ke_d/ke.h"
#include "sata.h"

BOOL SataChannelSpinWhileBusy(DWORD Address) {
    const size_t ONE_SECOND_IN_MICRO = 1000000;
    const size_t MICRO_PER_STALL = 100;
    int i;

    for (i = ONE_SECOND_IN_MICRO / MICRO_PER_STALL; i != 0; i--) {
        bool isBusy = (((BYTE*)Address)[ATAPI_REG_STATUS] & ATA_STATUS_BSY);
        if (isBusy == FALSE) {
            return TRUE;
        }
        KeStallExecutionProcessor(MICRO_PER_STALL);
    }

    return FALSE;
}

BOOL SataChannelSpinWhileBusyAndNotDrq(DWORD Address) {
    const size_t ONE_SECOND_IN_MICRO = 1000000;
    const size_t MICRO_PER_STALL = 100;
    int i;

    for (i = ONE_SECOND_IN_MICRO / MICRO_PER_STALL; i != 0; i--) {
        BYTE Status = (((BYTE*)Address)[ATAPI_REG_STATUS]);

        bool isBusy = Status & ATA_STATUS_BSY;
        bool hasDrq = Status & ATA_STATUS_DRQ;
        if (isBusy == FALSE && hasDrq != FALSE) {
            return TRUE;
        }

        KeStallExecutionProcessor(MICRO_PER_STALL);
    }

    return FALSE;
}
