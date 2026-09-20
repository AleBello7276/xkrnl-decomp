#pragma once

#include "devioctl.h"
#include "intrinsics.h"
#include <krnl.h>
#include <types.h>

#include "sata.h"

#define IOCTL_CDROM_FUNCTION_037 CTL_CODE(FILE_DEVICE_CD_ROM, 0x037, METHOD_BUFFERED, FILE_READ_ACCESS)

#define RUNTIME_PATCH_SIZE 18
#define RUNTIME_PATCH_DATA_SIZE ALIGN_UP(RUNTIME_PATCH_SIZE, 4)
#define TSST_CHECKSUMS_COUNT 4

extern void SataDiskTransferInterrupt(void);

extern bool SataCdRomSscInitialized;

extern HMODULE ExpUpdateModule;
extern void* KeDebugMonitorData;

extern uint16_t SataCdRomBootPerfStats;
extern int32_t SataCdRomDriveAuthenticationTick;
extern int8_t SataCdRomExpectingBusReset;
extern int16_t SataCdRomSenseData;
extern int32_t SataCdRomStaticTransferBuffer;
extern uint8_t* SataCdRomXGD2LastDiscAuthTotalTime;

extern BYTE SataCdRomHCDFRuntimePatchData_HCDF[RUNTIME_PATCH_DATA_SIZE];
extern BYTE SataCdRomHCDFRuntimePatchData_XGD2[RUNTIME_PATCH_DATA_SIZE];
extern const WORD SataCdRomTSSTValidChecksums[TSST_CHECKSUMS_COUNT];
extern struct _HCDF_RUNTIME_PATCH* SataCdRomActiveHCDFRuntimePatch;

typedef struct _SATA_CDROM_HCDF_WORK_BUFFER {
    BYTE Data[32];
    BYTE Hash[20];
} SATA_CDROM_HCDF_WORK_BUFFER;

typedef struct _HCDF_RUNTIME_PATCH {
    DWORD Method;
    DWORD BlockAddress;
    BYTE Hash[20];
} HCDF_RUNTIME_PATCH;

typedef struct _SATA_ATAPI_CMD_CONTEXT {
    /* 0x00 */ UCHAR Unknown00[8];
    /* 0x08 */ ULONG Unknown08;
    /* 0x0C */ ULONG Unknown0C;
} SATA_ATAPI_CMD_CONTEXT;  // 0x10

typedef struct _SATA_TRANSFER_DESCRIPTOR {
    UCHAR unk_00[3];
    UCHAR Flags;             // +03
    ULONG ByteCount;         // +04
    PVOID unk_08;            // +08
    ULONG unk_0C;            // +0C
    ULONG ControlCode;       // ioctl control code
} SATA_TRANSFER_DESCRIPTOR;  // 0x14

typedef struct _SATA_REQUEST {
    /* 0x00 */ UCHAR Unknown00[0x10];

    /* 0x10 */ NTSTATUS LastStatus;
    /* 0x14 */ ULONG TransferLength;

    /* 0x18 */ UCHAR Unknown18[4];

    /* 0x1C */ SATA_ATAPI_CMD_CONTEXT* AtapiContext;

    /* 0x20 */ UCHAR Unknown20[0x30];

    /* 0x50 */ SATA_TRANSFER_DESCRIPTOR* TransferDescriptor;
} SATA_REQUEST;

typedef struct SATA_SMC_NOTIFICATION {
    uint8_t notificationClass;
    uint8_t notificationType;
} SATA_SMC_NOTIFICATION;

typedef struct _RDC_DEVICE_OBJECT {
    /* +0x00 */ uint8_t unk_0x00[0x08];
    /* +0x08 */ void* DriverObject;
    /* +0x0C */ uint8_t unk_0x0C[0x14 - 0x0C];
    /* +0x14 */ uint32_t Flags;
} RDC_DEVICE_OBJECT;

void KeStallExecutionProcessor(uint32_t microseconds);
uint32_t KeInsertQueueDpc(KDPC* Dpc, PVOID arg1, PVOID arg2);
uint8_t KfRaiseIrql(uint8_t irql);
void KeAcquireSpinLockAtRaisedIrql(uint32_t* spinlock);
void KeReleaseSpinLockFromRaisedIrql(uint32_t* spinlock);
void KfLowerIrql(KIRQL* irql);
int32_t SataChannelResetDevice(void* deviceExt, void* pollFunc);
void SataChannelSetTimerPeriod(void* channelExt, uint32_t period);
int32_t SataChannelPrepareBufferTransfer(SataChannel* ext, void* buffer, int32_t length);
void SataChannelCopyDoubleBuffer(SataChannel* ext, void* buffer, int32_t length, int32_t flag);
int32_t SataChannelStartPacket(SataChannel* ext, SATA_REQUEST* pRequest);
void IoCompleteRequest(SATA_REQUEST* pRequest, int32_t priority);
int32_t IoSynchronousDeviceIoControlRequest(DWORD Ioctl, void* deviceExt, void* inBuf, int32_t inLen,
                                            void* outBuf, int32_t outLen, int32_t* retLen);

void SataCdRomInitialize();
void SataCdRomInitializeContinue(struct RdcDeviceObject* deviceObject);

bool SataCdRomPollResetComplete();

int32_t SataCdRomResetDevice(void* deviceExt);

bool SataCdRomSelectDeviceAndSpinWhileBusy();

bool SataCdRomWritePacket(uint32_t* packet);
void SataCdRomNoTransferInterrupt(void* channelExt, void* arg, uint32_t flags);

NTSTATUS SataCdromGetLastSenseData(uint8_t* buffer, uint32_t size);

void SataCdRomSetBootPerfStat(int32_t arg0);
void SataCdRomRecordIncrementStatistic(int32_t arg0);

void SataCdRomClearAuthenticationStateInternal(uint16_t* arg0);

void SataCdRomSMCNotification(void* arg1, SATA_SMC_NOTIFICATION* arg2);

NTSTATUS SataCdRomRestrictedDeviceControl(RDC_DEVICE_OBJECT* deviceObject, SATA_REQUEST* pRequest);
void SataCdRomStartIo(void* deviceObject, void* irp);
void SataCdRomDispatchIo(SataChannel* ext, void* irp);
void SataChannelAbortCurrentPacket(SataChannel* ext);
bool HalIsExecutingPowerDownDpc(void);

void SataCdRomIssueImmediateCommand(SataChannel* ext, uint8_t command);
void SataCdRomStandby();
KIRQL* KeRaiseIrqlToDpcLevel();

NTSTATUS SataCdRomAP21Initialize(SATA_REQUEST* pRequest);

void SataChannelInvalidParameterRequest(void* ext, void* irp);
void SataCdRomStartReadTOC(SataChannel* pChannel, SATA_REQUEST* pRequest);

bool SataCdRomSscOnReadError(SATA_REQUEST* pRequest);

bool SataCdRomIsReadRequest(SATA_TRANSFER_DESCRIPTOR* pDesc, bool);

void SataCdRomSscSetCurrentSpeed(int, void*);
