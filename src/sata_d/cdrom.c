#include "cdrom.h"
#include "krnl.h"
#include "types.h"

extern void* SataCdRomDriverObject;

uint32_t SataCdRomSscHandler(SATA_CDROM_CHANNEL_EXTENSION* ext);
void SataCdRomIssueAtapiRequest(void* cdb, void* buffer, int32_t length, int32_t flag,
                                SATA_COMPLETION_ROUTINE callback);
void SataCdRomFinishGenericWithOverrun(void* irp, int32_t status, void* info);

static uint8_t SataCdRomHCDFRuntimePatchData_HCDF[0x14] = {
    0x2C, 0, 0x9A, 6, 0x6D, 0xE9, 0xAF, 4, 0x72, 0x48, 0xBD, 0x14, 0xC3, 0xB5, 0, 0, 0x1E, 0xC, 0, 0,
};

static uint8_t SataCdRomHCDFRuntimePatchData_XGD2[0x14] = {
    0x1E, 0x2B, 0xE, 7, 0x16, 0x93, 0x38, 5, 0xE7, 0x3B, 0x2A, 0x16, 0x71, 0xBD, 0, 0, 0xA1, 0xC, 0, 0,
};

static int32_t SataCdromLayer1StartingSector = -1;
static int32_t SataCdRomHLDSSpecialModeSelect[4] = {0x6484C, 0, 0x6484C, 0};

static bool SataCdRomSscDiscReady = true;
static bool SataCdRomSscPending = true;
static int32_t SataCdRomX360Media = 0;
static int32_t SataCdRomEmulatorPresent = 0;
static int32_t SataCdRomDoUninterruptableReads = 0;
static int32_t SataCdRomSscCurrentSpeed = 0;
static int32_t SataCdRomSscMaximumSpeed = 0;
static int32_t SataCdRomSscFastestSpeed = 0;
static int32_t SataCdRomSscDesiredSpeed = 0;
static int32_t SataCdRomSscRetryCount = 0;
static int32_t SataCdRomSscReadErrors = 0;
static int32_t SataCdRomSscReadCount = 0;
static int32_t SataCdRomSscTimeStamp = 0;

#pragma data_seg(".data")
static bool SataCdRomSscInitialized = false;
#pragma data_seg()

static int32_t SataCdRomSscDisabled = 0;
static int32_t SataCdRomSscTotalReadErrors = 0;
static int32_t SataCdRomAuthenticationDisabled = 0;

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

NTSTATUS SataCdromGetLastSenseData(uint8_t* buffer, uint32_t size) {
    const uint32_t SENSE_DATA_SIZE = 18;

    if (buffer == nullptr)
        return STATUS_INVALID_PARAMETER;

    if (size < SENSE_DATA_SIZE)
        return STATUS_BUFFER_TOO_SMALL;

    memcpy(buffer, &SataCdRomSenseData, SENSE_DATA_SIZE);
    return STATUS_SUCCESS;
}

// function-specific struct for SataCdRomAP21Initialize
// offsets inferred from the matching function; not yet confirmed shared
typedef struct SataCdRomAP21PacketDesc {
    /* +0x00 */ uint8_t unk_0x00[3];
    /* +0x03 */ uint8_t unk_0x03;
    /* +0x04 */ uint32_t unk_0x04;
    /* +0x08 */ void* unk_0x08;
    /* +0x0C */ uint32_t unk_0x0C;
} SataCdRomAP21PacketDesc;

// function-specific struct for SataCdRomAP21Initialize
typedef struct SataCdRomAP21CmdBuf {
    /* +0x00 */ uint8_t unk_0x00[8];
    /* +0x08 */ uint32_t unk_0x08;
    /* +0x0C */ uint32_t unk_0x0C;
} SataCdRomAP21CmdBuf;

// function-specific struct for SataCdRomAP21Initialize
typedef struct SataCdRomAP21Device {
    /* +0x00 */ uint8_t unk_0x00[0x1C];
    /* +0x1C */ SataCdRomAP21CmdBuf* unk_0x1C;
    /* +0x20 */ uint8_t unk_0x20[0x30];
    /* +0x50 */ SataCdRomAP21PacketDesc* unk_0x50;
} SataCdRomAP21Device;

NTSTATUS SataCdRomAP21Initialize(SataCdRomAP21Device* dev) {
    const uint32_t SIZE_16 = 0x10;
    SataCdRomAP21CmdBuf* cmdBuf;

    assert(GetKPCR->m_currentIrql < 2);

    if (dev->unk_0x50->unk_0x0C != SIZE_16)
        return STATUS_INVALID_PARAMETER;

    if (dev->unk_0x50->unk_0x08 == nullptr)
        return STATUS_INVALID_PARAMETER;

    cmdBuf = dev->unk_0x1C;
    if (dev->unk_0x50->unk_0x04 != SIZE_16)
        return STATUS_INVALID_PARAMETER;

    if (cmdBuf != nullptr) {
        cmdBuf->unk_0x08 = 0;
        cmdBuf->unk_0x0C = 0;
        dev->unk_0x50->unk_0x03 |= 1;
        SataChannelStartPacket(&SataCdRomChannelExtension, dev);
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
    SATA_CDROM_CHANNEL_EXTENSION* ext = &SataCdRomChannelExtension;
    void* curIrp = ext->currentIrp;

    if (irp == curIrp) {
        __sync();
        ext->unk_0xD1 = 1;
        return;
    }

    if (!HalIsExecutingPowerDownDpc() && !(XboxHardwareInfo.m_something & 0x4000)) {
        ext->unk_0xAA = 0;
        ext->unk_0xAB = 4;
        SataCdRomDispatchIo(ext, irp);
        return;
    }

    SataChannelAbortCurrentPacket(ext);
}

// function-specific structs for SataCdRomRestrictedDeviceControl
// offsets inferred from the matching function; not yet confirmed shared
typedef struct RdcSrb {
    /* +0x00 */ uint8_t unk_0x00[0x03];
    /* +0x03 */ uint8_t flags;
    /* +0x04 */ uint8_t unk_0x04[0x10 - 0x04];
    /* +0x10 */ uint32_t functionCode;
} RdcSrb;

typedef struct RdcIrp {
    /* +0x00 */ uint8_t unk_0x00[0x10];
    /* +0x10 */ NTSTATUS ioStatus;
    /* +0x14 */ uint8_t unk_0x14[0x50 - 0x14];
    /* +0x50 */ RdcSrb* srb;
} RdcIrp;

typedef struct RdcDeviceObject {
    /* +0x00 */ uint8_t unk_0x00[0x08];
    /* +0x08 */ void* driverObject;
    /* +0x0C */ uint8_t unk_0x0C[0x14 - 0x0C];
    /* +0x14 */ uint32_t flags;
} RdcDeviceObject;

// HACK: it matches but i have to use a label ugh, otherwise it loads STATUS_PENDING into r3 insetad of r31
// etc
NTSTATUS SataCdRomRestrictedDeviceControl(RdcDeviceObject* deviceObject, RdcIrp* irp) {
    const uint32_t IDK1 = 0x240DC;
    const uint32_t IDK2 = 0x4D028;

    NTSTATUS status;

    if (irp->srb->functionCode != IDK1) {
        uint32_t diff = irp->srb->functionCode - IDK2;
        if (diff == 0 || diff == 4) {
            irp->srb->flags |= 1;
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

    irp->ioStatus = status;
    IoCompleteRequest(irp, 0);

done:  // uff
    return status;
}

// function-specific structs for SataCdRomStartReadTOC
typedef struct SrtSrb {
    /* +0x00 */ uint8_t unk_0x00[0x04];
    /* +0x04 */ uint32_t allocationLength;
    /* +0x08 */ uint8_t unk_0x08[0x10 - 0x08];
    /* +0x10 */ uint32_t functionCode;
} SrtSrb;

typedef struct SrtIrp {
    /* +0x00 */ uint8_t unk_0x00[0x14];
    /* +0x14 */ uint32_t storedLength;
    /* +0x18 */ uint8_t unk_0x18[0x1c - 0x18];
    /* +0x1c */ void* buffer;
    /* +0x20 */ uint8_t unk_0x20[0x50 - 0x20];
    /* +0x50 */ SrtSrb* srb;
} SrtIrp;
void SataCdRomStartReadTOC(SATA_CDROM_CHANNEL_EXTENSION* ext, void* irpPtr) {
    SrtIrp* irp = (SrtIrp*)irpPtr;
    SrtSrb* srb;
    uint32_t allocationLength;
    uint32_t funcCode;
    union {
        uint64_t qw[2];
        uint8_t bytes[16];
    } cdb;

    if (SataCdRomSscPending) {
        if (SataCdRomSscHandler(ext) != 0)
            return;
    }

    srb = irp->srb;
    allocationLength = srb->allocationLength;

    if (allocationLength == 0 || (allocationLength & 1) || ((uint32_t)irp->buffer & 1)) {
        SataChannelInvalidParameterRequest(ext, irp);
        return;
    }

    if (allocationLength > 0x324)
        allocationLength = 0x324;

    irp->storedLength = allocationLength;

    funcCode = srb->functionCode;
    cdb.qw[0] = 0;
    cdb.qw[1] = 0;
    cdb.bytes[0] = 0x43;
    cdb.bytes[8] = (uint8_t)(uint16_t)allocationLength;
    cdb.bytes[7] = (uint8_t)((uint16_t)allocationLength >> 8);

    if (funcCode == 0x24002) {
        cdb.bytes[1] |= 0x02;
    } else {
        cdb.bytes[9] = (cdb.bytes[9] & 0x3F) | 0x40;
    }

    SataCdRomIssueAtapiRequest(cdb.bytes, irp->buffer, allocationLength, 0,
                               SataCdRomFinishGenericWithOverrun);
}

void SataCdRomStandby(void) {
    KIRQL* oldIrql;

    assert(XboxHardwareInfo.m_something & 0x2000);

    oldIrql = KeRaiseIrqlToDpcLevel();
    SataCdRomIssueImmediateCommand(&SataCdRomChannelExtension, 0xE0);
    KfLowerIrql(oldIrql);
}
