#pragma once

#include <krnl.h>
#include <types.h>

#include "odd.h"
#include "sata.h"

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
} SATA_CDROM_CHANNEL_EXTENSION;

extern SATA_CDROM_CHANNEL_EXTENSION SataCdRomChannelExtension;

// function-specific struct (defined in cdrom.c), not yet confirmed shared
struct RdcIrp;

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
int32_t SataChannelPrepareBufferTransfer(SATA_CDROM_CHANNEL_EXTENSION* ext, void* buffer, int32_t length);
void SataChannelCopyDoubleBuffer(SATA_CDROM_CHANNEL_EXTENSION* ext, void* buffer, int32_t length,
                                 int32_t flag);
int32_t SataChannelStartPacket(SATA_CDROM_CHANNEL_EXTENSION* ext, struct RdcIrp* irp);
void IoCompleteRequest(struct RdcIrp* irp, int32_t priority);
int32_t IoSynchronousDeviceIoControlRequest(uint32_t ioctl, void* deviceExt, void* inBuf, int32_t inLen,
                                            void* outBuf, int32_t outLen, int32_t* retLen);

extern void SataDiskTransferInterrupt(void);

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

typedef struct SATA_SMC_NOTIFICATION {
    uint8_t notificationClass;
    uint8_t notificationType;
} SATA_SMC_NOTIFICATION;

void SataCdRomSMCNotification(void* arg1, SATA_SMC_NOTIFICATION* arg2);

NTSTATUS SataCdRomRestrictedDeviceControl(struct RdcDeviceObject* deviceObject, struct RdcIrp* irp);
void SataCdRomStartIo(void* deviceObject, void* irp);
void SataCdRomDispatchIo(SATA_CDROM_CHANNEL_EXTENSION* ext, void* irp);
void SataChannelAbortCurrentPacket(SATA_CDROM_CHANNEL_EXTENSION* ext);
bool HalIsExecutingPowerDownDpc(void);

// function-specific struct (defined in cdrom.c), not yet confirmed shared
struct SataCdRomAP21Device;

void SataCdRomIssueImmediateCommand(SATA_CDROM_CHANNEL_EXTENSION* ext, uint8_t command);
void SataCdRomStandby(void);
KIRQL* KeRaiseIrqlToDpcLevel(void);

NTSTATUS SataCdRomAP21Initialize(struct SataCdRomAP21Device* dev);

void SataChannelInvalidParameterRequest(void* ext, void* irp);
void SataCdRomStartReadTOC(SATA_CDROM_CHANNEL_EXTENSION* ext, void* irp);
