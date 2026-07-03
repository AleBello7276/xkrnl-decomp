#include "cdrom.h"
#include "krnl.h"
#include "types.h"

extern void* SataCdRomDriverObject;

bool SataCdRomSscDiscReady = true;
bool SataCdRomSscPending = true;

int32_t SataCdRomX360Media = 0;
int32_t SataCdRomEmulatorPresent = 0;
int32_t SataCdRomDoUninterruptableReads = 0;
int32_t SataCdRomSscCurrentSpeed = 0;
uint32_t SataCdRomSscMaximumSpeed = 0;
uint32_t SataCdRomSscFastestSpeed = 0;
int32_t SataCdRomSscDesiredSpeed = 0;
uint32_t SataCdRomSscRetryCount = 0;
uint32_t SataCdRomSscReadErrors = 0;
int32_t SataCdRomSscReadCount = 0;
int32_t SataCdRomSscTimeStamp = 0;
bool SataCdRomSscInitialized = 0;
uint32_t SataCdRomSscDisabled = 0;
uint32_t SataCdRomSscTotalReadErrors = 0;
uint64_t SataCdRomAuthenticationDisabled = 0;

ALLOC_SECT("CLRDATAA") uint8_t SataCdRomAP21ScratchBuffer[2048];

// https://github.com/xenia-canary/xenia-canary/blob/canary_experimental/src/xenia/kernel/kernel_state.h#L98
struct TimeStampBundle {
    uint64_t interrupt_time;
    uint64_t system_time;
    uint32_t tick_count;
    uint32_t padding;
};

extern struct TimeStampBundle KeTimeStampBundle;

//
// vendor / revision magics stuff
//
static const uint8_t SataCdRomVendorHLDS[8] = "HL-DT-ST";
static const uint8_t SataCdRomVendorSATA[8] = "SATA    ";
static const uint8_t SataCdRomVendorPBDS[8] = "PBDS    ";
static const uint8_t SataCdRomVendorPLDS[8] = "PLDS    ";
static const uint8_t SataCdRomProductHLDS1[16] = "DVD-ROM GDR3120L";
static const uint8_t SataCdRomProductHLDS2[16] = "DVD-ROM DL10N   ";
static const uint8_t SataCdRomRevisionHLDS0078[4] = "0078";
static const uint8_t SataCdRomRevisionHLDS0079[4] = "0079";
static const uint8_t SataCdRomRevisionHLDS0500[4] = "0500";
static const uint8_t SataCdRomRevisionPLDS7485[4] = "7485";
static const uint8_t SataCdRomRevisionPLDS8385[4] = "8385";

bool SataCdRomPollResetComplete() {
    uint8_t status;

    SataCdRomExpectingBusReset = 1;
    status = ODD_READ_STATUS();
    if ((status != 0x7f) && !(status & ATA_STATUS_BSY)) {
        SataCdRomSscInitialized = false;
        SataCdRomSscPending = true;
        return true;
    }

    return false;
}

int32_t SataCdRomResetDevice(void* deviceExt) {
    return SataChannelResetDevice(deviceExt, SataCdRomPollResetComplete);
}

bool SataCdRomSelectDeviceAndSpinWhileBusy() {
    uint32_t i;
    uint8_t status;

    ODD_WRITE_DEVICE(0);

    for (i = 1000; i != 0; i--) {
        if ((ODD_READ_STATUS() & ATA_STATUS_BSY) == false) {
            return true;
        }
        KeStallExecutionProcessor(100);
    }

    return false;
}

bool SataCdRomWritePacket(uint32_t* pkt) {
    uint32_t i;

    assert(GetKPCR->m_currentIrql == SataCdRomChannelExtension.irql);
    assert((__getr13() + 0x100) == SataCdRomChannelExtension.kPcrField);

    // issue ATA packet command
    ODD_WRITE_COMMAND(ATA_COMMAND_PACKET);

    // wait for device to be ready
    // should not be busy (BSY) and should be requesting data (DRQ)
    // time-out of 100 ms
    i = 1000;
    while (1) {
        uint8_t status = ODD_READ_STATUS();

        if (!(status & ATA_STATUS_BSY) && (status & ATA_STATUS_DRQ)) {
            break;
        }

        // wait 100 microseconds and try again
        KeStallExecutionProcessor(100);

        i--;
        if (i == 0)
            return false;
    }

    // send packet data
    ODD_WRITE_DATA(pkt[0]);
    ODD_WRITE_DATA(pkt[1]);
    ODD_WRITE_DATA(pkt[2]);

    return true;
}

// HACK, so it doesnt inline memcpy, need a deeper look at this
#pragma function(memcpy)
NTSTATUS SataCdromGetLastSenseData(uint8_t* buffer, uint32_t size) {
    const uint32_t SENSE_DATA_SIZE = 18;

    if (buffer == nullptr)
        return STATUS_INVALID_PARAMETER;

    if (size < SENSE_DATA_SIZE)
        return STATUS_BUFFER_TOO_SMALL;

    memcpy(buffer, &SataCdRomSenseData, SENSE_DATA_SIZE);
    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomAP21Initialize(SataRequest* pRequest) {
    const uint32_t SIZE_16 = 0x10;
    SataCdRomAP21CmdBuf* cmdBuf;

    assert(GetKPCR->m_currentIrql < 2);

    if (pRequest->pTransferDesc->unk_0x0c != SIZE_16)
        return STATUS_INVALID_PARAMETER;

    if (pRequest->pTransferDesc->unk_0x08 == nullptr)
        return STATUS_INVALID_PARAMETER;

    cmdBuf = pRequest->unk_0x1C;
    if (pRequest->pTransferDesc->byteCount != SIZE_16)
        return STATUS_INVALID_PARAMETER;

    if (cmdBuf != nullptr) {
        cmdBuf->unk_0x08 = 0;
        cmdBuf->unk_0x0C = 0;
        pRequest->pTransferDesc->flags |= 1;
        SataChannelStartPacket(&SataCdRomChannelExtension, pRequest);
        return STATUS_PENDING;
    }

    return STATUS_INVALID_PARAMETER;
}

void SataCdRomSMCNotification(void* arg1, SATA_SMC_NOTIFICATION* arg2) {
    if (arg2->notificationClass != 0x83)
        return;

    switch (arg2->notificationType) {
    case 0x60:
    case 0x61:
    case 0x63:
    case 0x64:
    case 0x65:
        SataCdRomClearAuthenticationStateInternal(0);
        return;

    case 0x62:
        SataCdRomSetBootPerfStat(0x14);
        SataCdRomRecordIncrementStatistic(0);
        return;

    default:
        return;
    }
}

void SataCdRomStartIo(void* deviceObject, void* irp) {
    SataChannel* ext = &SataCdRomChannelExtension;
    void* curIrp = ext->currentIrp;

    if (irp == curIrp) {
        __sync();
        ext->unk_0xD1 = 1;
        return;
    }

    if (!HalIsExecutingPowerDownDpc() && !(XboxHardwareInfo.Flags & 0x4000)) {
        ext->unk_0xAA = 0;
        ext->unk_0xAB = 4;
        SataCdRomDispatchIo(ext, irp);
        return;
    }

    SataChannelAbortCurrentPacket(ext);
}

typedef struct RdcDeviceObject {
    /* +0x00 */ uint8_t unk_0x00[0x08];
    /* +0x08 */ void* driverObject;
    /* +0x0C */ uint8_t unk_0x0C[0x14 - 0x0C];
    /* +0x14 */ uint32_t flags;
} RdcDeviceObject;

// HACK: it matches but i have to use a label ugh, otherwise it loads STATUS_PENDING into r3 insetad of r31
// etc
NTSTATUS SataCdRomRestrictedDeviceControl(RdcDeviceObject* deviceObject, SataRequest* irp) {
    const uint32_t IDK1 = 0x240DC;
    const uint32_t IDK2 = 0x4D028;

    NTSTATUS status;

    if (irp->pTransferDesc->deviceFlags != IDK1) {
        uint32_t diff = irp->pTransferDesc->deviceFlags - IDK2;
        if (diff == 0 || diff == 4) {
            irp->pTransferDesc->flags |= 1;
            SataChannelStartPacket(&SataCdRomChannelExtension, irp);
            status = STATUS_PENDING;
            goto done;
        }
        status = STATUS_INVALID_DEVICE_REQUEST;
    } else {
        deviceObject->driverObject = &SataCdRomDriverObject;
        deviceObject->flags &= ~1;
        SataCdRomInitializeContinue(deviceObject);
        status = STATUS_SUCCESS;
    }

    irp->lastStatus = status;
    IoCompleteRequest(irp, 0);

done:  // uff
    return status;
}

void SataCdRomStandby() {
    KIRQL* oldIrql;

    assert(XboxHardwareInfo.Flags & 0x2000);

    oldIrql = KeRaiseIrqlToDpcLevel();
    SataCdRomIssueImmediateCommand(&SataCdRomChannelExtension, 0xE0);
    KfLowerIrql(oldIrql);
}

extern void SataCdRomSscFinishSpeedDecrease();
void DbgPrint(char* format, ...);

extern int DAT_80240ef0;

#define SSC_MAX_ATTEMPTS 8
#define SSC_MIN_SPEED 1

bool SataCdRomSscOnReadError(SataRequest* pRequest) {
    uint32_t speedFloor;

    if (SataCdRomSscRetryCount >= SSC_MAX_ATTEMPTS && SataCdRomSscInitialized) {
        SataCdRomSscPending = 0;
        DbgPrint("SATA: SSC disabled.\n");
        return false;
    }

    if (!SataCdRomIsReadRequest(pRequest->pTransferDesc, false))
        return false;

    if (SataCdRomSscReadErrors < SSC_MAX_ATTEMPTS)
        SataCdRomSscReadErrors++;
    SataCdRomSscTotalReadErrors++;
    DbgPrint("SATA: SSC read errors %d %d.\n", SataCdRomSscReadErrors, SataCdRomSscTotalReadErrors);

    SataCdRomSscReadCount = 0;
    SataCdRomSscTimeStamp = KeTimeStampBundle.tick_count;
    SataCdRomSscPending = 1;

    if (SataCdRomSscReadErrors >= SSC_MAX_ATTEMPTS && SataCdRomSscMaximumSpeed == SataCdRomSscFastestSpeed
        && SataCdRomSscMaximumSpeed > 1) {
        SataCdRomSscMaximumSpeed--;
        DbgPrint("SATA: SSC at %d:%d:%d.\n", SataCdRomSscCurrentSpeed, SataCdRomSscMaximumSpeed,
                 SataCdRomSscFastestSpeed);
    }

    speedFloor = (SataCdRomSscFastestSpeed >= 3) ? SataCdRomSscFastestSpeed - 2 : SSC_MIN_SPEED;

    if (SataCdRomSscCurrentSpeed > speedFloor) {
        SataCdRomSscSetCurrentSpeed(SataCdRomSscCurrentSpeed - 1, SataCdRomSscFinishSpeedDecrease);
        return true;
    }

    return false;
}
