#include "cdrom.h"

#include "ema.h"
#include "fatalError.h"
#include "init/kdataseg.h"
#include "ke_d/ke.h"
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

BYTE SataCdRomSenseData[SENSE_DATA_SIZE];

/* __Linker__ */
KSPIN_LOCK SataCdRomDvdAuthBufferLock;

/* __Linker__ */
PVOID SataCdRomDvdAuthBufferPhysical;

ALLOC_SECT("CLRDATAA") uint8_t SataCdRomAP21ScratchBuffer[SCRATCH_BUFFER_SIZE];

BYTE SataCdRomHCDFRuntimePatchData_XGD2[RUNTIME_PATCH_DATA_SIZE]
    = {0x1e, 0x2b, 0x0e, 0x07, 0x16, 0x93, 0x38, 0x05, 0xe7, 0x3b,
       0x2a, 0x16, 0x71, 0xbd, 0x00, 0x00, 0xa1, 0x0c, 0x00, 0x00};

BYTE SataCdRomHCDFRuntimePatchData_HCDF[RUNTIME_PATCH_DATA_SIZE]
    = {0x2c, 0x00, 0x9a, 0x06, 0x6d, 0xe9, 0xaf, 0x04, 0x72, 0x48,
       0xbd, 0x14, 0xc3, 0xb5, 0x00, 0x00, 0x1e, 0x0c, 0x00, 0x00};

const WORD SataCdRomTSSTValidChecksums[TSST_CHECKSUMS_COUNT] = {0xACB7, 0x1ACF, 0x896E, 0xA440};

HCDF_RUNTIME_PATCH* SataCdRomActiveHCDFRuntimePatch = nullptr;

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
    status = ATAPI_READ_STATUS();
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

    ATAPI_WRITE_DEVICE(0);

    for (i = 1000; i != 0; i--) {
        if ((ATAPI_READ_STATUS() & ATA_STATUS_BSY) == false) {
            return true;
        }
        KeStallExecutionProcessor(100);
    }

    return false;
}

bool SataCdRomWritePacket(uint32_t* pkt) {
    uint32_t i;

    assert(GetKPCR->m_currentIrql == SataCdRomChannelExtension.mIrql);
    assert((__getr13() + 0x100) == SataCdRomChannelExtension.kPcrField);

    // issue ATA packet command
    ATAPI_WRITE_COMMAND(ATA_COMMAND_PACKET);

    // wait for device to be ready
    // should not be busy (BSY) and should be requesting data (DRQ)
    // time-out of 100 ms
    i = 1000;
    while (1) {
        uint8_t status = ATAPI_READ_STATUS();

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
    ATAPI_WRITE_DATA(pkt[0]);
    ATAPI_WRITE_DATA(pkt[1]);
    ATAPI_WRITE_DATA(pkt[2]);

    return true;
}

// HACK, so it doesnt inline memcpy, need a deeper look at this
#pragma function(memcpy)
NTSTATUS SataCdromGetLastSenseData(uint8_t* buffer, uint32_t size) {
    const uint32_t BUFF_SIZE = 18;

    if (buffer == nullptr)
        return STATUS_INVALID_PARAMETER;

    if (size < BUFF_SIZE)
        return STATUS_BUFFER_TOO_SMALL;

    memcpy(buffer, &SataCdRomSenseData, BUFF_SIZE);
    return STATUS_SUCCESS;
}

NTSTATUS SataCdRomAP21Initialize(SATA_REQUEST* pRequest) {
    const uint32_t SIZE_16 = 0x10;
    SATA_ATAPI_CMD_CONTEXT* cmdBuf;

    assert(GetKPCR->m_currentIrql < 2);

    if (pRequest->TransferDescriptor->unk_0C != SIZE_16)
        return STATUS_INVALID_PARAMETER;

    if (pRequest->TransferDescriptor->unk_08 == nullptr)
        return STATUS_INVALID_PARAMETER;

    cmdBuf = pRequest->AtapiContext;
    if (pRequest->TransferDescriptor->ByteCount != SIZE_16)
        return STATUS_INVALID_PARAMETER;

    if (cmdBuf != nullptr) {
        cmdBuf->Unknown08 = 0;
        cmdBuf->Unknown0C = 0;
        pRequest->TransferDescriptor->Flags |= 1;
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
    SATA_CHANNEL* ext = &SataCdRomChannelExtension;
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

NTSTATUS SataCdRomRestrictedDeviceControl(RDC_DEVICE_OBJECT* Device, SATA_REQUEST* Request) {
    NTSTATUS status;
    ULONG code = Request->TransferDescriptor->ControlCode;

    switch (code) {
    case IOCTL_SATA_FUNCTION_40A:
    case IOCTL_SATA_FUNCTION_40B:

        Request->TransferDescriptor->Flags |= 0x01;

        SataChannelStartPacket(&SataCdRomChannelExtension, Request);

        status = STATUS_PENDING;
        goto done;
    case IOCTL_CDROM_FUNCTION_037:

        Device->DriverObject = &SataCdRomDriverObject;
        Device->Flags &= ~0x01;

        SataCdRomInitializeContinue(Device);

        status = STATUS_SUCCESS;

        break;

    default:

        status = STATUS_INVALID_DEVICE_REQUEST;

        break;
    }

    Request->LastStatus = status;
    IoCompleteRequest(Request, 0);
done:
    return status;
}

void SataCdRomStandby() {
    KIRQL oldIrql;

    assert(XboxHardwareInfo.Flags & HARDWAREINFO_FLAGS_0x2000);

    oldIrql = KeRaiseIrqlToDpcLevel();
    SataCdRomIssueImmediateCommand(&SataCdRomChannelExtension, 0xE0);
    KfLowerIrql(oldIrql);
}

NTSTATUS SataCdRomActivateHCDFRuntimePatch() {
    SATA_CDROM_HCDF_WORK_BUFFER Buffer;
    NTSTATUS Status;

    DbgPrint("SATA: Activate HCDF Runtime patch method %d\n", SataCdRomActiveHCDFRuntimePatch->Method);

    switch (SataCdRomActiveHCDFRuntimePatch->Method) {
    case 0:
        Status = SataCdRomSendE7ReadWrite(SataCdRomActiveHCDFRuntimePatch->BlockAddress, Buffer.Data,
                                          RUNTIME_PATCH_SIZE, TRUE);
        if (!NT_SUCCESS(Status)) {
            DbgPrint("SATA: HLDS Read failed\n");
            break;
        }

        if (memcmp(SataCdRomHCDFRuntimePatchData_XGD2, Buffer.Data, RUNTIME_PATCH_SIZE) != 0
            && memcmp(SataCdRomHCDFRuntimePatchData_HCDF, Buffer.Data, RUNTIME_PATCH_SIZE) != 0) {
            DbgPrint("SATA: Unexpected previous data\n");
            break;
        }

        Status = SataCdRomSendE7ReadWrite(SataCdRomActiveHCDFRuntimePatch->BlockAddress,
                                          SataCdRomHCDFRuntimePatchData_HCDF, RUNTIME_PATCH_SIZE, FALSE);
        if (!NT_SUCCESS(Status)) {
            DbgPrint("SATA: HLDS Write failed\n");
            break;
        }
        return Status;

    case 1:

        Status = SataCdRomSendE7ReadWrite(NULL, Buffer.Data, 2, TRUE);
        if (NT_SUCCESS(Status)) {
            DbgPrint("SATA: HLDS Read did not fail, indicating firmware mismatch.\n");
            break;
        }

        Status = SataCdRomHLDSEnableAllCommands();
        if (!NT_SUCCESS(Status)) {
            DbgPrint("SATA: HLDS enable all commands failed, indicating non HLDS drive.\n");
            break;
        }

        Status = SataCdRomHLDSDirectRead(SataCdRomActiveHCDFRuntimePatch->BlockAddress + 0x10,
                                         SataCdRomAP21ScratchBuffer);
        DbgPrint("SATA: read direct, status %08x\n", Status);

        if (NT_SUCCESS(Status)) {
            Status = SataCdRomHLDSDirectRead(SataCdRomActiveHCDFRuntimePatch->BlockAddress,
                                             SataCdRomAP21ScratchBuffer);
            DbgPrint("SATA: read direct (final), status %08x\n", Status);
        }

        if (!NT_SUCCESS(Status))
            return Status;

        XeCryptSha(SataCdRomAP21ScratchBuffer, SCRATCH_BUFFER_SIZE, NULL, 0, NULL, 0, Buffer.Hash,
                   RUNTIME_PATCH_DATA_SIZE);
        if (memcmp(Buffer.Hash, SataCdRomActiveHCDFRuntimePatch->Hash, RUNTIME_PATCH_DATA_SIZE) != 0) {
            DbgPrint("SATA: HCDF Media doesn't have proper data at LBA %08x\n",
                     SataCdRomActiveHCDFRuntimePatch->BlockAddress);
            return STATUS_DISK_CORRUPT_ERROR;
        }

        Status = SataCdRomHLDSEnableFlashExe();
        if (!NT_SUCCESS(Status)) {
            DbgPrint("SATA: HLDS enable flash exe failed, %08x\n", Status);
            break;
        }

        Status = SataCdRomHLDSJumpToRam();
        if (!NT_SUCCESS(Status)) {
            DbgPrint("SATA: HLDS Jump to ram failed, %08x\n", Status);
            break;
        }
        return STATUS_SUCCESS;

    case 2:
        if (ExpUpdateModule == nullptr) {
            DbgPrint("SATA: Attempted to authenticate XGD3 disc in blacklisted drive\n");
            break;
        }
        return STATUS_SUCCESS;

    case 3:
        if (ExpUpdateModule == nullptr && SataCdromCheckTSSTChecksum() < 0) {
            DbgPrint("SATA: Attempted to authenticate XGD3 disc in blacklisted drive (TSST)\n");
            break;
        }
        return STATUS_SUCCESS;

    default:
        return STATUS_SUCCESS;
    }

    VdDisplayFatalError(ERROR_XSS_CDROM_COULD_NOT_CREATE_DEVICE);
    return STATUS_UNSUCCESSFUL;
}

bool SataCdRomSscOnReadError(SATA_REQUEST* pRequest) {
    uint32_t speedFloor;

    if (SataCdRomSscRetryCount >= SSC_MAX_ATTEMPTS && SataCdRomSscInitialized) {
        SataCdRomSscPending = 0;
        DbgPrint("SATA: SSC disabled.\n");
        return false;
    }

    if (!SataCdRomIsReadRequest(pRequest->TransferDescriptor, false))
        return false;

    if (SataCdRomSscReadErrors < SSC_MAX_ATTEMPTS)
        SataCdRomSscReadErrors++;
    SataCdRomSscTotalReadErrors++;
    DbgPrint("SATA: SSC read errors %d %d.\n", SataCdRomSscReadErrors, SataCdRomSscTotalReadErrors);

    SataCdRomSscReadCount = 0;
    SataCdRomSscTimeStamp = KeTimeStampBundle.TickCount;
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

void SataCdRomFinishStandby(void* param_1, SATA_REQUEST* Request, NTSTATUS Status) {
    Request->LastStatus = Status;
    IoCompleteRequest(Request, 1);
    SataChannelStartNextPacket(param_1);
}

void SataCdRomCancelPacket() {
    SataChannelCancelPacket(&SataCdRomChannelExtension);
}

void SataCdRomIssueImmediateCommand(SATA_CHANNEL* Channel, uint8_t Command) {
    assert(GetKPCR->m_currentIrql == 2);

    KfRaiseIrql(Channel->mIrql);
    KeAcquireSpinLockAtRaisedIrql(&Channel->mSpinLock);

    Channel->kPcrField = &GetKPCR->unk_0x100;

    if (SataCdRomSelectDeviceAndSpinWhileBusy()) {
        ATAPI_WRITE_COMMAND(Command);
        SataChannelSpinWhileBusy(ATAPI_REGS_ADDR);
    }

    assert(GetKPCR->m_currentIrql == Channel->mIrql);
    assert(&GetKPCR->unk_0x100 == Channel->kPcrField);

    KeReleaseSpinLockFromRaisedIrql(&Channel->mSpinLock);
    KfLowerIrql(2);
}

void SataCdRomStartCheckVerify(SATA_CHANNEL* Channel, SATA_REQUEST* Request) {
    ATAPI_PACKET Packet;
    memset(&Packet, 0, sizeof(ATAPI_PACKET));

    Request->TransferLength = 0;
    Channel->unk_0xAB = 0;

    Packet.Generic.OperationCode = SCSIOP_TEST_UNIT_READY;
    SataCdRomIssueAtapiRequest(&Packet, NULL, NULL, NULL, SataCdRomFinishGeneric);
}

void SataCdRomFinishGeneric(SATA_CHANNEL* Channel, SATA_REQUEST* Request, NTSTATUS Status) {
    ATAPI_PACKET Packet;

    if (Status == STATUS_IO_DEVICE_ERROR) {
        const size_t ALLOCATION_SIZE = 18;

        memset(&Packet, 0, sizeof(Packet));

        Packet.RequestSense.OperationCode = SCSIOP_REQUEST_SENSE;
        Packet.RequestSense.AllocationLength = ALLOCATION_SIZE;

        SataCdRomIssueAtapiRequest(&Packet, (PVOID)SataCdRomStaticTransferBuffer, ALLOCATION_SIZE, NULL,
                                   SataCdRomFinishRequestSense);
        return;
    }

    if (Status == STATUS_IO_TIMEOUT) {
        assert(GetKPCR->m_currentIrql == 2);

        KfRaiseIrql(Channel->mIrql);
        KeAcquireSpinLockAtRaisedIrql(&Channel->mSpinLock);

        Channel->kPcrField = &GetKPCR->unk_0x100;

        SataChannelResetDevice(Channel, SataCdRomPollResetComplete);
        return;
    }

    Request->LastStatus = Status;
    IoCompleteRequest(Request, 1);
    SataChannelStartNextPacket(Channel);
}

void KeAcquireSpinLockAtRaisedIrql(PKSPIN_LOCK Lock);
void SataCdRomRestartCurrentPacket();

void SataCdRomWaitAndRestartCurrentPacket(SATA_CHANNEL* pChannel, ULONG Idk) {
    ULONG Delay = Idk;
    SATA_CHANNEL* Channel;

    SataChannelSetTimerPeriod(pChannel, 100);

    Channel = pChannel;

    assert(GetKPCR->m_currentIrql == 2);

    KfRaiseIrql(Channel->mIrql);

    KeAcquireSpinLockAtRaisedIrql(&Channel->mSpinLock);

    Channel->retryCount = Delay / 100;
    Channel->mRoutine = SataCdRomRestartCurrentPacket;
    Channel->kPcrField = &GetKPCR->unk_0x100;

    assert(GetKPCR->m_currentIrql == Channel->mIrql);
    assert(&GetKPCR->unk_0x100 == Channel->kPcrField);

    KeReleaseSpinLockFromRaisedIrql(&Channel->mSpinLock);

    KfLowerIrql(2);
}

NTSTATUS SataCdRomSscDisable(uint32_t param_1) {
    DWORD local_20[4];

    XeKeysGetStatus(local_20);
    if (((local_20[0] & 0x8000) == 0) || ((local_20[0] & 8) != 0)) {
        SataCdRomSscDisabled = param_1;
        return STATUS_SUCCESS;
    }

    return STATUS_UNSUCCESSFUL;
}

void SataCdRomSscFinishCheckDiscReady(SATA_CHANNEL* Channel, SATA_REQUEST* Request, NTSTATUS Status);

void SataCdRomSscCheckDiscReady() {
    ATAPI_PACKET Packet;
    memset(&Packet, 0, sizeof(ATAPI_PACKET));
    Packet.Generic.OperationCode = SCSIOP_TEST_UNIT_READY;

    SataCdRomIssueAtapiRequest(&Packet, NULL, NULL, NULL, SataCdRomSscFinishCheckDiscReady);
}

NTSTATUS SataCdRomDVDAP20AuthenticateDrive();

// NTSTATUS SataCdRomDriveAuthentication() {
//     NTSTATUS Status;
//     SataCdRomSetBootPerfStat(3);
//     EmaExecute((PVOID)1);
//     Status = SataCdRomDVDAP20AuthenticateDrive();
//     if (NT_SUCCESS(Status)) {
//         return EmaExecute((PVOID)2);
//     }
//
//     SataCdRomSetBootPerfStat(4);
//     return;
// }
