#pragma once

#include <krnl.h>
#include <types.h>

// TODO: sort this:
void KeRetireDpcList();

typedef struct _SATA_REQUEST SATA_REQUEST;

typedef struct _SATA_EXTENSION {
    uint8_t Reserved_0x00[0x10];
    SATA_REQUEST* Request;
    ULONG Flags;
} SATA_EXTENSION;

typedef struct _SATA_NOTIFICATION {
    char pad[32];
    BOOLEAN unk0x20;
    BOOLEAN unk0x21;
} SATA_NOTIFICATION, *PSATA_NOTIFICATION;

// must be 212 bytes
typedef struct _SataChannel {
    uint32_t unk0x0;
    uint32_t unk0x4;
    uint32_t unk0x8;
    char pad1[8];
    PVOID* mRoutine;
    char pad0[0x44];
    void* idk0;
    uint32_t idk;
    uint32_t idk2;
    uint32_t idk3;
    SATA_EXTENSION* ChannelExtension;
    char pad88[0x88 - 0x70];
    void* bufferPtr;
    int32_t bufferLen;
    char pad98[0x98 - 0x90];
    KSPIN_LOCK mSpinLock;
    char padA1[0xA1 - 0x9C];
    KIRQL mIrql;
    char padA4[0xA4 - 0xA2];
    PVOID kPcrField;
    uint8_t flags;
    uint8_t retryCount;
    uint8_t unk_0xAA;
    uint8_t unk_0xAB;
    SATA_REQUEST* mRequest;
    SATA_NOTIFICATION mNotification;
    char unk210;
    BYTE ActiveSomethingMask;
    char pad222[0x32];
    KSPIN_LOCK unk0x108;  // likely KSPIN_LOCK
} SATA_CHANNEL, *PSATA_CHANNEL;

/* */
void SataChannelStartNextPacket(PSATA_CHANNEL pChannel);

/* */
void SataChannelCancelPacket(SATA_CHANNEL* Channel, SATA_REQUEST* Request);

/* */
BOOL SataChannelSpinWhileBusy(DWORD Address);

/* */
BOOL SataChannelSpinWhileBusyAndNotDrq(DWORD Address);

/* */
void SataChannelInvalidParameterRequest(void* ext, void* irp);

/* */
void SataChannelAbortCurrentPacket(SATA_CHANNEL* Channel);

/* */
NTSTATUS SataChannelResetDevice(SATA_CHANNEL* pChannel, void* PollRoutine);

/* */
void SataChannelSetTimerPeriod(SATA_CHANNEL* pChannel, DWORD Period);

/* */
int32_t SataChannelPrepareBufferTransfer(SATA_CHANNEL* pChannel, void* buffer, int32_t length);

/* */
void SataChannelCopyDoubleBuffer(SATA_CHANNEL* pChannel, void* buffer, int32_t length, int32_t flag);

/* */
int32_t SataChannelStartPacket(SATA_CHANNEL* pChannel, SATA_REQUEST* pRequest);

/* */
void SataChannelDriverNotification(PSATA_NOTIFICATION Notification, ULONG ID);
