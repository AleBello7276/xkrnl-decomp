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
#define SENSE_DATA_SIZE 32

#define SSC_MAX_ATTEMPTS 8
#define SSC_MIN_SPEED 1

// TODO: sort this stuff
extern void SataDiskTransferInterrupt(void);
uint32_t KeInsertQueueDpc(KDPC* Dpc, PVOID arg1, PVOID arg2);
KIRQL KfRaiseIrql(KIRQL Irql);
void KfLowerIrql(KIRQL pIrql);
int32_t IoSynchronousDeviceIoControlRequest(DWORD Ioctl, void* deviceExt, void* inBuf, int32_t inLen,
                                            void* outBuf, int32_t outLen, int32_t* retLen);
bool HalIsExecutingPowerDownDpc(void);
KIRQL KeRaiseIrqlToDpcLevel();
void DbgPrint(char* format, ...);
void SataCdRomSscFinishSpeedDecrease();
void XeCryptSha(const PVOID InputA, DWORD InputA_Size, const PVOID InputB, DWORD InputB_Size,
                const PVOID InputC, DWORD InputC_Size, PVOID Output, DWORD OutputSize);
void VdDisplayFatalError(DWORD errorCode);
void XeKeysGetStatus(DWORD*);
void SataCdRomStandbySynchronized(PVOID);

extern int DAT_80240ef0;

extern BOOL SataCdRomSscInitialized;

extern HMODULE ExpUpdateModule;
extern void* KeDebugMonitorData;

extern uint16_t SataCdRomBootPerfStats;
extern int32_t SataCdRomDriveAuthenticationTick;
extern int8_t SataCdRomExpectingBusReset;
extern BYTE SataCdRomSenseData[SENSE_DATA_SIZE];
extern uint8_t* SataCdRomXGD2LastDiscAuthTotalTime;

extern BYTE SataCdRomHCDFRuntimePatchData_HCDF[RUNTIME_PATCH_DATA_SIZE];
extern BYTE SataCdRomHCDFRuntimePatchData_XGD2[RUNTIME_PATCH_DATA_SIZE];
extern const WORD SataCdRomTSSTValidChecksums[TSST_CHECKSUMS_COUNT];
extern struct _HCDF_RUNTIME_PATCH* SataCdRomActiveHCDFRuntimePatch;

extern KSPIN_LOCK SataCdRomDvdAuthBufferLock;
extern PVOID SataCdRomDvdAuthBufferPhysical;

typedef struct _SATA_CDROM_HCDF_WORK_BUFFER {
    BYTE Data[32];
    BYTE Hash[20];
} SATA_CDROM_HCDF_WORK_BUFFER;

typedef struct _HCDF_RUNTIME_PATCH {
    DWORD Method;
    DWORD BlockAddress;
    BYTE Hash[20];
} HCDF_RUNTIME_PATCH;

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

/* */
void SataCdRomInitialize();

/* */
void SataCdRomInitializeContinue(struct RdcDeviceObject* deviceObject);

/* */
bool SataCdRomPollResetComplete();

/* */
int32_t SataCdRomResetDevice(void* deviceExt);

/* */
bool SataCdRomSelectDeviceAndSpinWhileBusy();

/* */
bool SataCdRomWritePacket(uint32_t* packet);

/* */
void SataCdRomNoTransferInterrupt(void* channelExt, void* arg, uint32_t flags);

/* */
NTSTATUS SataCdromGetLastSenseData(uint8_t* buffer, uint32_t size);

/* */
VOID SataCdRomSetBootPerfStat(ULONG Stat);

/* */
void SataCdRomRecordIncrementStatistic(int32_t arg0);

/* */
void SataCdRomClearAuthenticationStateInternal(uint16_t* arg0);

/* */
void SataCdRomSMCNotification(void* arg1, SATA_SMC_NOTIFICATION* arg2);

/* */
NTSTATUS SataCdRomRestrictedDeviceControl(RDC_DEVICE_OBJECT* deviceObject, SATA_REQUEST* pRequest);

/* */
void SataCdRomStartIo(void* deviceObject, SATA_REQUEST* Request);

/* */
void SataCdRomDispatchIo(SATA_CHANNEL* Channel, void* irp);

/* */
void SataCdRomStandby();

/* */
NTSTATUS SataCdRomAP21Initialize(SATA_REQUEST* pRequest);

/* */
void SataCdRomStartReadTOC(SATA_CHANNEL* pChannel, SATA_REQUEST* pRequest);

/* */
bool SataCdRomSscOnReadError(SATA_REQUEST* pRequest);

/* */
bool SataCdRomIsReadRequest(SATA_TRANSFER_DESCRIPTOR* pDesc, bool);

/* */
void SataCdRomSscSetCurrentSpeed(int, void*);

/* */
void SataCdRomFinishStandby(void* param_1, SATA_REQUEST* Request, NTSTATUS Status);

/* */
void SataCdRomCancelPacket(PSATA_CHANNEL Channel, SATA_REQUEST* pRequest);

/* */
void SataCdRomIssueImmediateCommand(SATA_CHANNEL* Channel, uint8_t command);

/* */
void SataCdRomFinishGeneric(SATA_CHANNEL* Channel, SATA_REQUEST* Request, NTSTATUS Status);

/* */
NTSTATUS SataCdRomSscDisable(BOOL Disable);

/* */
void SataCdRomSscFinishCheckDiscReady(SATA_CHANNEL* Channel, SATA_REQUEST* Request, NTSTATUS Status);

/* */
void SataCdRomSscCheckDiscReady();

/* */
NTSTATUS SataCdRomDriveAuthentication();

/* */
void SataCdRomBackgroundModeNotificationRoutine();

/* */
NTSTATUS SataCdRomDVDAP20AuthenticateDrive();

/* */
void SataCdRomFinishRequestSense(SATA_CHANNEL* Channel, SATA_REQUEST* Request, NTSTATUS Status);

/* */
void SataCdRomIssueAtapiRequest(ATAPI_PACKET* Packet, PVOID, LONG, LONG, ATAPI_REQUEST_ROUTINE Routine);

/* */
NTSTATUS SataCdRomSendE7ReadWrite(DWORD BlockAddress, PVOID Buffer, DWORD TransferLenght, BOOL Read);

/* */
NTSTATUS SataCdRomHLDSDirectRead(DWORD BlockAddress, PVOID Buffer);

/* */
NTSTATUS SataCdRomHLDSJumpToRam();

/* */
NTSTATUS SataCdromCheckTSSTChecksum();

/* */
NTSTATUS SataCdRomHLDSEnableAllCommands();

/* */
NTSTATUS SataCdRomHLDSEnableFlashExe();

/* */
NTSTATUS SataCdRomActivateHCDFRuntimePatch();
