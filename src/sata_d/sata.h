#pragma once

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

#define ATAPI_REGS_ADDR 0x7FEA1200

#define ATAPI_WRITE(type, offset, data)                                                                      \
    *((volatile type*)(ATAPI_REGS_ADDR + offset)) = data;                                                    \
    __eieio()

#define ATAPI_WRITE_U8(offset, data) ATAPI_WRITE(uint8_t, offset, data)
#define ATAPI_WRITE_U32(offset, data) ATAPI_WRITE(uint32_t, offset, data)

#define ATAPI_READ(type, offset) *((volatile type*)(ATAPI_REGS_ADDR + offset))
#define ATAPI_READ_U8(offset) ATAPI_READ(uint8_t, offset)
#define ATAPI_READ_U32(offset) ATAPI_READ(uint32_t, offset)

/* ----- */

#define ATAPI_WRITE_DATA(data) ATAPI_WRITE_U32(ATAPI_REG_DATA, data)
#define ATAPI_WRITE_DEVICE(data) ATAPI_WRITE_U8(ATAPI_REG_DEVICE, data)
#define ATAPI_WRITE_COMMAND(data) ATAPI_WRITE_U8(ATAPI_REG_COMMAND, data)

#define ATAPI_READ_STATUS() ATAPI_READ_U8(ATAPI_REG_STATUS)

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

typedef void (*SATA_COMPLETION_ROUTINE)(void* irp, int32_t status, void* info);

typedef struct _SATA_REQUEST SATA_REQUEST;

typedef struct _SataExtension {
    uint8_t Reserved_0x00[0x10];
    SATA_REQUEST* Request;
    ULONG Flags;
} SataExtension;

typedef struct _SataChannel {
    uint32_t unk0x0;
    uint32_t unk0x4;
    uint32_t unk0x8;
    char pad0[0x50];
    void* idk0;
    uint32_t idk;
    uint32_t idk2;
    uint32_t idk3;
    SataExtension* ChannelExtension;
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

extern SataChannel SataCdRomChannelExtension;

#define SCRATCH_BUFFER_SIZE 0x800

#pragma section("CLRDATAA", read, write)
extern ALLOC_SECT("CLRDATAA") uint8_t SataCdRomAP21ScratchBuffer[SCRATCH_BUFFER_SIZE];
extern ALLOC_SECT("CLRDATAA") uint32_t SataCdRomHvVerifyComplete;

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
extern bool SataCdRomSscInitialized;
extern uint32_t SataCdRomSscDisabled;
extern uint32_t SataCdRomSscTotalReadErrors;
extern uint64_t SataCdRomAuthenticationDisabled;
