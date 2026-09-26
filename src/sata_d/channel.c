#include "channel.h"

#include "io_d/io.h"
#include "ke_d/ke.h"
#include "sata.h"
#include <intrinsics.h>

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

// NON_MATCHING: close but cr6 thing, functional match tho i think
void SataChannelCancelPacket(SATA_CHANNEL* Channel, SATA_REQUEST* Request) {
    LIST_ENTRY* Next;
    LIST_ENTRY* Prev;

    assert(GetKPCR->m_currentIrql == DISPATCH_LEVEL);

    IoReleaseCancelSpinLock();
    KeAcquireSpinLockAtRaisedIrql(&Channel->unk0x108);

    Next = Request->List.Flink;
    if (Next != NULL) {
        Prev = Request->List.Blink;

        Prev->Flink = Next;
        Next->Blink = Prev;

        if (Prev == Next) {
            Channel->ActiveSomethingMask &= ~(1 << (BYTE)(Request->Index + 2));
        }
    }

    KeReleaseSpinLockFromRaisedIrql(&Channel->unk0x108);

    Request->LastStatus = STATUS_CANCELLED;
    Request->TransferLength = 0;
    IoCompleteRequest(Request, 0);
}

void SataChannelDriverNotification(PSATA_NOTIFICATION Notification, ULONG ID) {
    SATA_CHANNEL* Channel = CONTAINING_RECORD(Notification, SATA_CHANNEL, mNotification);

    assert(GetKPCR->m_currentIrql == DISPATCH_LEVEL);
    assert(GetKPCR->m_ProcessorNum == 0);

    switch (ID) {
    case 0:
        break;

    case 1:
        if (Channel->mNotification.unk0x20) {
            Channel->mNotification.unk0x20 = FALSE;
            SataChannelStartNextPacket(Channel);
        }
        return;

    case 2:
        break;

    default:
        return;
    }

    if (Channel->mNotification.unk0x20 == FALSE) {
        Channel->mNotification.unk0x21 = Channel->mNotification.unk0x20;
        __sync();

        SataChannelStartPacket(Channel, Channel->mRequest);

        Channel->mNotification.unk0x20 = TRUE;
    }

    while (Channel->mNotification.unk0x21 == FALSE)
        KeRetireDpcList();
}
