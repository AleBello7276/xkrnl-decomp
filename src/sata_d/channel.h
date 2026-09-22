#pragma once

#include <krnl.h>
#include <types.h>

typedef struct _SATA_REQUEST SATA_REQUEST;

typedef struct _SATA_EXTENSION {
    uint8_t Reserved_0x00[0x10];
    SATA_REQUEST* Request;
    ULONG Flags;
} SATA_EXTENSION;

typedef struct _SataChannel {
    uint32_t unk0x0;
    uint32_t unk0x4;
    uint32_t unk0x8;
    char pad0[0x50];
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
    void* kPcrField;
    uint8_t flags;
    uint8_t retryCount;
    uint8_t unk_0xAA;
    uint8_t unk_0xAB;
    void* currentIrp;
    char padD1[0xD1 - 0xB0];
    uint8_t unk_0xD1;
} SATA_CHANNEL;

/* */
void SataChannelStartNextPacket(void*);

/* */
void SataChannelCancelPacket(SATA_CHANNEL* Channel);

/* */
BOOL SataChannelSpinWhileBusy(DWORD Address);

/* */
void SataChannelInvalidParameterRequest(void* ext, void* irp);

/* */
void SataChannelAbortCurrentPacket(SATA_CHANNEL* Channel);

/* */
NTSTATUS SataChannelResetDevice(SATA_CHANNEL* pChannel, void* PollRoutine);

/* */
void SataChannelSetTimerPeriod(void* channelExt, uint32_t period);

/* */
int32_t SataChannelPrepareBufferTransfer(SATA_CHANNEL* pChannel, void* buffer, int32_t length);

/* */
void SataChannelCopyDoubleBuffer(SATA_CHANNEL* pChannel, void* buffer, int32_t length, int32_t flag);

/* */
int32_t SataChannelStartPacket(SATA_CHANNEL* pChannel, SATA_REQUEST* pRequest);
