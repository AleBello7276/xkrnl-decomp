#pragma once
#include "intrinsics.h"
#include "types.h"

#define ODD_ADDR 0x7FEA1200

#define ODD_OFF_DATA 0x0     // data offset
#define ODD_OFF_DEVICE 0x6   // device offset
#define ODD_OFF_COMMAND 0x7  // command offset
#define ODD_OFF_STATUS 0x7   // status offset

#define ODD_WRITE(type, offset, data)                                                                        \
    *((volatile type*)(ODD_ADDR + offset)) = data;                                                           \
    __eieio()
#define ODD_WRITE_U8(offset, data) ODD_WRITE(uint8_t, offset, data)
#define ODD_WRITE_U32(offset, data) ODD_WRITE(uint32_t, offset, data)

#define ODD_READ(type, offset) *((volatile type*)(ODD_ADDR + offset))
#define ODD_READ_U8(offset) ODD_READ(uint8_t, offset)
#define ODD_READ_U32(offset) ODD_READ(uint32_t, offset)

/* ----- */

#define ODD_WRITE_DATA(data) ODD_WRITE_U32(ODD_OFF_DATA, data)
#define ODD_WRITE_DEVICE(data) ODD_WRITE_U8(ODD_OFF_DEVICE, data)
#define ODD_WRITE_COMMAND(data) ODD_WRITE_U8(ODD_OFF_COMMAND, data)

#define ODD_READ_STATUS() ODD_READ_U8(ODD_OFF_STATUS)
