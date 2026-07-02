#pragma once

//
//  krnl defines and stuff
//

#include "init.h"
#include "ntstatus.h"
#include "types.h"

typedef uint32_t KSPIN_LOCK;  // Kernel Spin Lock

// IRQLs
typedef uint8_t KIRQL;
#define IRQL_HIGH_LEVEL 0x7c

typedef struct _KPCR {
    char paddd[24];
    KIRQL m_currentIrql;
    char pad[87];
    uint32_t m_stackPtr;
    char pad2[140];
    uint32_t unk_0x100;
    char pad3[8];
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

void __emit(uint32_t op);

#define __eieio() __emit(0x7c0006ac)
#define __sync() __emit(0x7C0004AC)

uint64_t __mftb();
#define mftb32() ((uint32_t)__mftb());

unsigned int __getr13();
void __setr13(uint64_t);
#define GetR13() __getr13()
#define SetR13(x) __setr13(x)

#define GetKPCR ((KPCR*)GetR13())

typedef struct _LIST_ENTRY {
    struct _LIST_ENTRY* Flink;
    struct _LIST_ENTRY* Blink;
} LIST_ENTRY, *PLIST_ENTRY;

struct _KDPC;

typedef void (*PKDEFERRED_ROUTINE)(struct _KDPC* Dpc, void* DeferredContext, void* SystemArgument1,
                                   void* SystemArgument2);

typedef struct _KDPC {
    SHORT Type;
    uint8_t Number;
    uint8_t Importance;
    LIST_ENTRY DpcListEntry;
    PKDEFERRED_ROUTINE DeferredRoutine;
    void* DeferredContext;
    void* SystemArgument1;
    void* SystemArgument2;
    PULONG_PTR Lock;
} KDPC, *PKDPC;

#define assert(expr)                                                                                         \
    if (!(expr)) {                                                                                           \
        __asm {twi 31, r0, 0x19}                                                                                \
    }

void* memcpy(void* dst, const void* src, size_t n);
void* memset(void* dst, int c, size_t n);
int memcmp(const void* a, const void* b, size_t n);

char* strcpy(char* dst, const char* src);
char* strcat(char* dst, const char* src);
int strcmp(const char* a, const char* b);
size_t strlen(const char* s);
