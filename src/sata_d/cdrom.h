#pragma once

#include <krnl.h>
#include <types.h>

#include "odd.h"
#include "sata.h"

extern void SataDiskTransferInterrupt(void);

extern bool SataCdRomSscInitialized;

extern int32_t ExpUpdateModule;
extern void* KeDebugMonitorData;

extern uint16_t SataCdRomBootPerfStats;
extern int32_t SataCdRomDriveAuthenticationTick;
extern int8_t SataCdRomExpectingBusReset;
extern int16_t SataCdRomSenseData;
extern int32_t SataCdRomStaticTransferBuffer;
extern uint8_t* SataCdRomXGD2LastDiscAuthTotalTime;

typedef void (*SATA_COMPLETION_ROUTINE)(void* irp, int32_t status, void* info);

typedef struct SATA_CDROM_CHANNEL_EXTENSION {
    char pad0[0x4];
    void (*restartCallback)(void*);
    void (*interruptHandler)();
    SATA_COMPLETION_ROUTINE completionRoutine;
    char pad14[0x14 - 0x10];
    void (*restartFunc)(void*);
    char pad6C[0x6C - 0x18];
    void* deviceExt;
    char pad88[0x88 - 0x70];
    void* bufferPtr;
    int32_t bufferLen;
    char pad98[0x98 - 0x90];
    uint32_t spinlock;
    char padA1[0xA1 - 0x9C];
    uint8_t irql;
    char padA4[0xA4 - 0xA2];
    uint32_t kPcrField;
    uint8_t flags;
    uint8_t retryCount;
    uint8_t unk_0xAA;
    uint8_t unk_0xAB;
    void* currentIrp;
    char padD1[0xD1 - 0xB0];
    uint8_t unk_0xD1;
} SataChannel;

// function-specific struct for SataCdRomAP21Initialize
typedef struct _SataCdRomAP21CmdBuf {
    /* +0x00 */ uint8_t unk_0x00[8];
    /* +0x08 */ uint32_t unk_0x08;
    /* +0x0C */ uint32_t unk_0x0C;
} SataCdRomAP21CmdBuf;

typedef struct _SataTransferDescriptor {
    /* +0x00 */ uint8_t unk_0x00[3];
    /* +0x03 */ uint8_t flags;
    /* +0x04 */ uint32_t byteCount;
    /* +0x08 */ void* unk_0x08;
    /* +0x0c */ uint32_t unk_0x0c;
    /* +0x10 */ uint32_t deviceFlags;
} SataTransferDescriptor;

typedef struct _SataRequest {
    /* +0x00 */ uint8_t unk_0x00[0x10];
    /* +0x10 */ NTSTATUS lastStatus;
    /* +0x14 */ uint32_t transferLength;
    /* +0x18 */ uint8_t unk_0x18[4];
    /* +0x1c */ SataCdRomAP21CmdBuf* unk_0x1C;
    /* +0x20 */ uint8_t unk_0x20[0x50 - 0x20];
    /* +0x50 */ SataTransferDescriptor* pTransferDesc;
} SataRequest;

typedef struct SATA_SMC_NOTIFICATION {
    uint8_t notificationClass;
    uint8_t notificationType;
} SATA_SMC_NOTIFICATION;

extern SataChannel SataCdRomChannelExtension;

// function-specific struct (defined in cdrom.c), not yet confirmed shared
struct RdcDeviceObject;

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
int32_t SataChannelStartPacket(SataChannel* ext, SataRequest* pRequest);
void IoCompleteRequest(SataRequest* pRequest, int32_t priority);
int32_t IoSynchronousDeviceIoControlRequest(uint32_t ioctl, void* deviceExt, void* inBuf, int32_t inLen,
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

NTSTATUS SataCdRomRestrictedDeviceControl(struct RdcDeviceObject* deviceObject, SataRequest* pRequest);
void SataCdRomStartIo(void* deviceObject, void* irp);
void SataCdRomDispatchIo(SataChannel* ext, void* irp);
void SataChannelAbortCurrentPacket(SataChannel* ext);
bool HalIsExecutingPowerDownDpc(void);

void SataCdRomIssueImmediateCommand(SataChannel* ext, uint8_t command);
void SataCdRomStandby();
KIRQL* KeRaiseIrqlToDpcLevel();

NTSTATUS SataCdRomAP21Initialize(SataRequest* pRequest);

void SataChannelInvalidParameterRequest(void* ext, void* irp);
void SataCdRomStartReadTOC(SataChannel* pChannel, SataRequest* pRequest);

bool SataCdRomSscOnReadError(SataRequest* pRequest);

bool SataCdRomIsReadRequest(SataTransferDescriptor* pDesc, bool);
