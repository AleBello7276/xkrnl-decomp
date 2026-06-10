#pragma once

//
//  krnl defines and stuff
//

#include "init.h"
#include "types.h"

// IRQLs
typedef uint8_t KIRQL;
#define IRQL_HIGH_LEVEL 0x7c

typedef struct _KPCR {
    char paddd[24];
    KIRQL m_currentIrql;
    char pad[87];
    uint32_t m_stackPtr;
    char pad2[152];
    uint8_t m_ProcessorNum;
} KPCR;

typedef struct _STRING {
    uint16_t Length;
    uint16_t MaximumLength;
    uint8_t* Buffer;
} STRING;

typedef struct _MEMORY_DESCRIPTOR {
    uint32_t unk0;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
} MEMORY_DESCRIPTOR;

typedef struct _MMADDRESS_NODE {
    int unk0;
    int unk1;
    struct _MMADDRESS_NODE* m_parent;
    struct _MMADDRESS_NODE* m_leftLeaf;
    struct _MMADDRESS_NODE* m_rightLeaf;
} MMADDRESS_NODE;

//
// INTRINSICS, move this later!
//

void* __GPRGetReg(uint32_t);
void __GPRSetReg(uint32_t, uint64_t);
#define GetGPR(x) (uint64_t)__GPRGetReg(x)
#define SetGPR(x, y) __GPRSetReg(x, y)

unsigned int __getr13();
void __setr13(uint64_t);
#define GetR13() __getr13()
#define SetR13(x) __setr13(x)

#define GetKPCR ((KPCR*)GetR13())

#define assert(expr)                                                                                         \
    if (!(expr)) {                                                                                           \
        __asm {twi 31, r0, 0x19}                                                                                \
    }
