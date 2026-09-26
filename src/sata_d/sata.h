#pragma once

#include "channel.h"
#include "krnl.h"
#include <types.h>

// https://github.com/xenon-emu/xenon/blob/main/Xenon/Core/PCI/SATA.h

//
// ATA Status Register
//

/*
   This register contains the current status of the drive. If the BSY bit is 0,
   the other bits of the register contain valid information; otherwise the other
   bits do not contain valid information. If this register is read by the host
   computer during a pending interrupt, the interrupt condition is cleared

   Bits 1 & 2 are undefined
   Bit 4 is Command Specific
*/

/*
   Bit 0 (ERR / CHK - Error / Check) is defined as ERR for all commands except
   for the Packet and Service commands, for which this bit is defined as CHK
*/
#define ATA_STATUS_ERR_CHK 0x1
/*
   Bit 3 (DRQ - Data Request) indicates by value 1 that the disk drive is ready
   to transfer data between the host computer and the drive. After the computer
   writes a commmand code to the Command register, the drive sets the BSY bit or
   the DRQ bit to 1 until command completion
*/
#define ATA_STATUS_DRQ 0x08
/*
   Bit 5 (DF - Device Fault) indicates by value 1 that a device fault has been
   detected
*/
#define ATA_STATUS_DF 0x20
/*
   Bit 6 (DRDY - Device Ready) is set to 1 to indicate that the disk drive
   accepts commands. If the DRDY bit is 0, the drive will accept and attempt to
   execute the Device Reset and Execute Device Diagnostic commands. Other
   commands will not be accepted, and the drive will set the ABRT bit in the
   Error register and the ERR/CHK bit in the Status register, before resetting
   the BSY bit to indicate completion of the command
*/
#define ATA_STATUS_DRDY 0x40
/*
   Bit 7 (BSY - Busy) is set to 1 whenever the disk drive has control of the
   Command Block registers. If the BSY bit is 1, a write to any Command Block
   register by the host computer will be ignored by the drive. The BSY bit is
   cleared to 0 by the drive at command completion and after setting the DRQ
   status bit to 1 to indicate the device is ready to transfer data
*/
#define ATA_STATUS_BSY 0x80

//
// ATA Commands
//

#define ATA_COMMAND_DEVICE_RESET 0x08
#define ATA_COMMAND_READ_SECTORS 0x20
#define ATA_COMMAND_READ_DMA_EXT 0x25
#define ATA_COMMAND_READ_NATIVE_MAX_ADDRESS_EXT 0x27
#define ATA_COMMAND_WRITE_SECTORS 0x30
#define ATA_COMMAND_WRITE_DMA_EXT 0x35
#define ATA_COMMAND_READ_VERIFY_SECTORS 0x40
#define ATA_COMMAND_READ_VERIFY_SECTORS_EXT 0x42
#define ATA_COMMAND_READ_FPDMA_QUEUED 0x60
#define ATA_COMMAND_SET_DEVICE_PARAMETERS 0x91
#define ATA_COMMAND_PACKET 0xA0
#define ATA_COMMAND_IDENTIFY_PACKET_DEVICE 0xA1
#define ATA_COMMAND_READ_MULTIPLE 0xC4
#define ATA_COMMAND_WRITE_MULTIPLE 0xC5
#define ATA_COMMAND_SET_MULTIPLE_MODE 0xC6
#define ATA_COMMAND_READ_DMA 0xC8
#define ATA_COMMAND_WRITE_DMA 0xCA
#define ATA_COMMAND_STANDBY_IMMEDIATE 0xE0
#define ATA_COMMAND_FLUSH_CACHE 0xE7
#define ATA_COMMAND_IDENTIFY_DEVICE 0xEC
#define ATA_COMMAND_SET_FEATURES 0xEF
#define ATA_COMMAND_SECURITY_SET_PASSWORD 0xF1
#define ATA_COMMAND_SECURITY_UNLOCK 0xF2
#define ATA_COMMAND_SECURITY_DISABLE_PASSWORD 0xF6

//
// SCSI Command Descriptor Block Operation codes
//

// 6 Byte 'Standard' CDB
#define SCSIOP_TEST_UNIT_READY 0x00
#define SCSIOP_REQUEST_SENSE 0x03
#define SCSIOP_FORMAT_UNIT 0x04
#define SCSIOP_INQUIRY 0x12
#define SCSIOP_MODE_SELECT6 0x15
#define SCSIOP_MODE_SENSE6 0x1A
#define SCSIOP_START_STOP 0x1B
#define SCSIOP_TOGGLE_LOCK 0x1E

// 10 Byte CDB
#define SCSIOP_READ_FMT_CAP 0x23
#define SCSIOP_READ_CAPACITY 0x25
#define SCSIOP_READ10 0x28
#define SCSIOP_SEEK10 0x2B
#define SCSIOP_ERASE10 0x2C
#define SCSIOP_WRITE10 0x2A
#define SCSIOP_VER_WRITE10 0x2E
#define SCSIOP_VERIFY10 0x2F
#define SCSIOP_SYNC_CACHE 0x35
#define SCSIOP_WRITE_BUF 0x3B
#define SCSIOP_READ_BUF 0x3C
#define SCSIOP_READ_SUBCH 0x42
#define SCSIOP_READ_TOC 0x43
#define SCSIOP_READ_HEADER 0x44
#define SCSIOP_PLAY_AUDIO10 0x45
#define SCSIOP_GET_CONFIG 0x46
#define SCSIOP_PLAY_AUDIOMSF 0x47
#define SCSIOP_EVENT_INFO 0x4A
#define SCSIOP_TOGGLE_PAUSE 0x4B
#define SCSIOP_STOP 0x4E
#define SCSIOP_READ_INFO 0x51
#define SCSIOP_READ_TRK_INFO 0x52
#define SCSIOP_RES_TRACK 0x53
#define SCSIOP_SEND_OPC 0x54
#define SCSIOP_MODE_SELECT10 0x55
#define SCSIOP_REPAIR_TRACK 0x58
#define SCSIOP_MODE_SENSE10 0x5A
#define SCSIOP_CLOSE_TRACK 0x5B
#define SCSIOP_READ_BUF_CAP 0x5C

// 12 Byte CDB
#define SCSIOP_BLANK 0xA1
#define SCSIOP_SEND_KEY 0xA3
#define SCSIOP_REPORT_KEY 0xA4
#define SCSIOP_PLAY_AUDIO12 0xA5
#define SCSIOP_LOAD_CD 0xA6
#define SCSIOP_SET_RD_AHEAD 0xA7
#define SCSIOP_READ12 0xA8
#define SCSIOP_WRITE12 0xAA
#define SCSIOP_GET_PERF 0xAC
#define SCSIOP_READ_DVD_S 0xAD
#define SCSIOP_SET_STREAM 0xB6
#define SCSIOP_READ_CD_MSF 0xB9
#define SCSIOP_SCAN 0xBA
#define SCSIOP_SET_CD_SPEED 0xBB
#define SCSIOP_PLAY_CD 0xBC
#define SCSIOP_MECH_STATUS 0xBD
#define SCSIOP_READ_CD 0xBE
#define SCSIOP_SEND_DVD_S 0xBF

#define XE_MAX_DMA_PRD 16

//
// ATAPI (ODD) Registers Offsets
//

// Communication with disk drive controllers is achieved via I/O registers.
// Registers and their offsets relative to the base
// address of command block registers and the base address of control block
// registers

// Registers Offsets from Command Block

// Data Reg (Read/Write)
#define ATAPI_REG_DATA 0x0
// Error Reg (Read)
#define ATAPI_REG_ERROR 0x1
// Features Reg (Write)
#define ATAPI_REG_FEATURES 0x1
// Interrupt Reason Reg (Read)
#define ATAPI_REG_INT_REAS 0x2
// Sector Count Reg (Write)
#define ATAPI_REG_SECTOR_COUNT 0x2
// LBA Low Reg (Read/Write)
#define ATAPI_REG_LBA_LOW 0x3
// Byte Count Low Reg (Read/Write)
#define ATAPI_REG_BYTE_COUNT_LOW 0x4
// Byte Count High Reg (Read/Write)
#define ATAPI_REG_BYTE_COUNT_HIGH 0x5
// Device Reg (Read/Write)
#define ATAPI_REG_DEVICE 0x6
// Status Reg (Read)
#define ATAPI_REG_STATUS 0x7
// Command Reg (Write)
#define ATAPI_REG_COMMAND 0x7

#define ATAPI_REGS_ADDR (0x7FEA1200)

#define ATAPI_WRITE(type, base, offset, data)                                                                \
    *((volatile type*)(base + offset)) = data;                                                               \
    __eieio()

#define ATAPI_WRITE_U8(base, offset, data) ATAPI_WRITE(uint8_t, base, offset, data)
#define ATAPI_WRITE_U32(base, offset, data) ATAPI_WRITE(uint32_t, base, offset, data)

#define ATAPI_READ(type, base, offset) *((volatile type*)(base + offset))
#define ATAPI_READ_U8(base, offset) ATAPI_READ(uint8_t, base, offset)
#define ATAPI_READ_U32(base, offset) ATAPI_READ(uint32_t, base, offset)

/* ----- */

#define ATAPI_WRITE_DATA(data) ATAPI_WRITE_U32(ATAPI_REGS_ADDR, ATAPI_REG_DATA, data)
#define ATAPI_WRITE_DEVICE(data) ATAPI_WRITE_U8(ATAPI_REGS_ADDR, ATAPI_REG_DEVICE, data)
#define ATAPI_WRITE_COMMAND(data) ATAPI_WRITE_U8(ATAPI_REGS_ADDR, ATAPI_REG_COMMAND, data)

#define ATAPI_READ_STATUS() ATAPI_READ_U8(ATAPI_REGS_ADDR, ATAPI_REG_STATUS)

//
//
//
/*
   IOCTLs
*/

#define IOCTL_0x4D014                                                                                        \
    CTL_CODE(FILE_DEVICE_CONTROLLER, 0x405, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_SATA_FUNCTION_40A                                                                              \
    CTL_CODE(FILE_DEVICE_CONTROLLER, 0x40A, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define IOCTL_SATA_FUNCTION_40B                                                                              \
    CTL_CODE(FILE_DEVICE_CONTROLLER, 0x40B, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

/*----------------------------------------*/
//
//
//
typedef union _ATAPI_PACKET {
    BYTE Bytes[16];
    QWORD Qwords[2];

    struct {
        BYTE OperationCode;  // +0x00
        BYTE Reserved[15];   // +0x01
    } Generic;

    struct {
        BYTE OperationCode;     // +0x00 = 0x03
        BYTE Reserved1;         // +0x01
        BYTE Reserved2;         // +0x02
        BYTE Reserved3;         // +0x03
        BYTE AllocationLength;  // +0x04
        BYTE Control;           // +0x05
        BYTE Reserved4[10];     // +0x06
    } RequestSense;

} ATAPI_PACKET;

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

typedef void (*SATA_COMPLETION_ROUTINE)(void* irp, int32_t status, void* info);

typedef void (*ATAPI_REQUEST_ROUTINE)(SATA_CHANNEL* Channel, SATA_REQUEST* Request, NTSTATUS Status);

extern SATA_CHANNEL SataCdRomChannelExtension;

#define SCRATCH_BUFFER_SIZE 0x800

#pragma section("CLRDATAA", read, write)
extern ALLOC_SECT("CLRDATAA") uint8_t SataCdRomAP21ScratchBuffer[SCRATCH_BUFFER_SIZE];
extern ALLOC_SECT("CLRDATAA") BOOL SataCdRomHvVerifyComplete;

extern int32_t SataCdRomX360Media;
extern int32_t SataCdRomEmulatorPresent;
extern int32_t SataCdRomDoUninterruptableReads;
extern int32_t SataCdRomSscCurrentSpeed;
extern uint32_t SataCdRomSscMaximumSpeed;
extern uint32_t SataCdRomSscFastestSpeed;
extern int32_t SataCdRomSscDesiredSpeed;
extern uint32_t SataCdRomSscRetryCount;
extern uint32_t SataCdRomSscReadErrors;
extern int32_t SataCdRomSscReadCount;
extern int32_t SataCdRomSscTimeStamp;
extern BOOL SataCdRomSscInitialized;
extern BOOL SataCdRomSscDisabled;
extern uint32_t SataCdRomSscTotalReadErrors;
extern uint64_t SataCdRomAuthenticationDisabled;

typedef DWORD CDROM_HV_FUNCTION;
typedef enum _CDROM_HV_FUNCTION {
    SataCdRomHvBuildNVPage_e = 0x25,
    SataCdRomHvVerifyNVPage_e = 0x26,
    SataCdRomHvRecordAuthenticationPage_e = 0x27,
    SataCdRomHvRecordXControl_e = 0x28,
    SataCdRomHvGetAuthPage_e = 0x29,
    SataCdRomHvVerifyAuthPage_e = 0x2A,
    SataCdRomHvGetNextLBAIndex_e = 0x2B,
    HvxDvdAuthVerifyLBA_e = 0x2c,
    SataCdRomHvClearDiscAuthInfo_e = 0x2D,
    SataCdRomHvGetAuthResults_e = 0x5C,
    SataCdRomHvSetDiscAuthResult_e = 0x5E,
    SataCdRomHvFwcr_e = 0x72,

    Unknown_e = -1,
} CDROM_HV_FUNCTION;
