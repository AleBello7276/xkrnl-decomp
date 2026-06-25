#pragma once

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
