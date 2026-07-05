#include "kddata.h"

uint8_t KdPrintCircularBuffer[KD_PRINT_BUFFER_SIZE] = {0};
uint8_t* KdPrintWritePointer = KdPrintCircularBuffer;
MEMORY_DESCRIPTOR KdPhysicalMemoryDescriptor = {0};
uint32_t KdPrintRolloverCount = 0;

LIST_ENTRY KdpDebuggerDataListHead = {
    &KdpDebuggerDataListHead,  // fl
    &KdpDebuggerDataListHead   // bl
};

KDDEBUGGER_DATA KdDebuggerDataBlock = {
    {0},  // Header
    1     // pad0
};

KD_PHYSICAL_PORT* KdPhysicalPort = &KdSerialPhysicalPort;

KDP_BREAKPOINT_TYPE KdpBreakpointInstruction = 0;
ULONG KdpOweBreakpoint = 0;

ULONG KdpNextPacketIdToSend;
ULONG KdpPacketIdExpected;

BOOLEAN KdpDebuggerStructuresInitialized = FALSE;
BOOLEAN KdpLiteOnly = FALSE;
BOOLEAN KdpControlCPressed = FALSE;
BOOLEAN KdpControlCPending = FALSE;

BREAKPOINT_ENTRY KdpBreakpointTable[BREAKPOINT_TABLE_SIZE] = {0};

LARGE_INTEGER KdPerformanceCounterRate = {0, 0};
LARGE_INTEGER KdTimerStart = {0, 0};
LARGE_INTEGER KdTimerStop = {0, 0};
LARGE_INTEGER KdTimerDifference = {0, 0};
