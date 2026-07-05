#pragma once
#include <krnl.h>

#define KD_PRINT_BUFFER_SIZE 0x3000

#define KDP_BREAKPOINT_TYPE ULONG
#define BREAKPOINT_TABLE_SIZE 32

#define INITIAL_PACKET_ID 0x80800000
#define SYNC_PACKET_ID 0x800

typedef struct _DBGKD_DEBUG_DATA_HEADER {
    LIST_ENTRY List;
    ULONG OwnerTag;
    ULONG Size;
} DBGKD_DEBUG_DATA_HEADER;

typedef struct _KDDEBUGGER_DATA {
    DBGKD_DEBUG_DATA_HEADER Header;
    uint8_t pad0[0x308];
} KDDEBUGGER_DATA;

// static_assert(sizeof(KDDEBUGGER_DATA) == 0x318, "");

typedef struct _KD_PHYSICAL_PORT {
    bool (*Initialize)(bool);
    bool (*RecivePacket)(void*, void*, void*, void*);
    void (*SendPacket)(void*, void*, void*);
} KD_PHYSICAL_PORT;

typedef struct _BREAKPOINT_ENTRY {
    ULONG Flags;
    ULONG_PTR DirectoryTableBase;
    PVOID Address;
    KDP_BREAKPOINT_TYPE Content;
} BREAKPOINT_ENTRY, *PBREAKPOINT_ENTRY;

extern uint8_t KdPrintCircularBuffer[KD_PRINT_BUFFER_SIZE];
extern uint8_t* KdPrintWritePointer;
extern MEMORY_DESCRIPTOR KdPhysicalMemoryDescriptor;
extern uint32_t KdPrintRolloverCount;
extern LIST_ENTRY KdpDebuggerDataListHead;
extern KDDEBUGGER_DATA KdDebuggerDataBlock;

extern KD_PHYSICAL_PORT* KdPhysicalPort;
extern KD_PHYSICAL_PORT KdNetPhysicalPort;
extern KD_PHYSICAL_PORT KdSerialPhysicalPort;

extern KDP_BREAKPOINT_TYPE KdpBreakpointInstruction;
extern ULONG KdpOweBreakpoint;

extern ULONG KdpNextPacketIdToSend;
extern ULONG KdpPacketIdExpected;

extern BOOLEAN KdpDebuggerStructuresInitialized;
extern BOOLEAN KdpLiteOnly;
extern BOOLEAN KdpControlCPressed;
extern BOOLEAN KdpControlCPending;

extern BREAKPOINT_ENTRY KdpBreakpointTable[BREAKPOINT_TABLE_SIZE];

extern LARGE_INTEGER KdPerformanceCounterRate;
extern LARGE_INTEGER KdTimerStart;
extern LARGE_INTEGER KdTimerStop;
extern LARGE_INTEGER KdTimerDifference;
