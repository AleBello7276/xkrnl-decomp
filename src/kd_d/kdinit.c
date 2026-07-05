#include "kddata.h"
#include "kdinit.h"

// TODO
extern bool KdNetIsEnabled();
extern KIRQL* KfRaiseIrql(KIRQL);
extern void KfLowerIrql(KIRQL*);
extern void KdpNetQuiesce(uint32_t);
extern uint32_t KdpMoveMemory(uint8_t* dest, uint8_t* src, uint32_t len, bool unk);

extern void* KiDebugRoutine;

extern bool KdpStub(void*, void*, void*);
extern bool KdpTrap(void*, void*, void*);

extern void KdpByteSwapDebuggerDataBlock(KDDEBUGGER_DATA* DebugData);

extern void KdpNetInitializeParameters();

extern LONGLONG KeQueryPerformanceFrequency();

extern void RtlInitAnsiString(PANSI_STRING Destination, LPSTR Source);

extern void DbgLoadImageSymbols(PANSI_STRING String, PVOID ImagePointer, LONG);

extern bool KdPollBreakIn();

extern void DbgBreakPointWithStatus(bool);

extern bool KdPortGetByte(void*);

void KdInitializeSystem(bool idk) {
    if ((XboxHardwareInfo.Flags & HARDWAREINFO_FLAGS_0x80) == false) {
        KiDebugRoutine = &KdpStub;
        if (idk) {
            KdpByteSwapDebuggerDataBlock(&KdDebuggerDataBlock);
            KdDebuggerDataBlock.Header.List.Blink = &KdpDebuggerDataListHead;
            KdDebuggerDataBlock.Header.List.Flink = KdpDebuggerDataListHead.Blink;
            KdpDebuggerDataListHead.Blink->Flink = (LIST_ENTRY*)&KdDebuggerDataBlock;
            KdpDebuggerDataListHead.Blink = &KdDebuggerDataBlock.Header;
            KdpNetInitializeParameters();
            if (KdNetIsEnabled()) {
                KdPhysicalPort = &KdNetPhysicalPort;
            }
        }

        if (KdPhysicalPort->Initialize(true)) {
            bool bldrBit;
            LONG mask;

            KiDebugRoutine = KdpTrap;
            if (KdpDebuggerStructuresInitialized == FALSE) {
                ULONG i;
                KdpBreakpointInstruction = KDP_BREAKPOINT_VALUE;
                KdpOweBreakpoint = FALSE;

                for (i = 0; i < BREAKPOINT_TABLE_SIZE; i++) {
                    KdpBreakpointTable[i].Flags = 0;
                    KdpBreakpointTable[i].DirectoryTableBase = 0;
                    KdpBreakpointTable[i].Address = NULL;
                }

                KdpDebuggerStructuresInitialized = TRUE;
            }

            KdPerformanceCounterRate.QuadPart = KeQueryPerformanceFrequency();
            KdTimerStart.QuadPart = 0;

            KdpNextPacketIdToSend = INITIAL_PACKET_ID | SYNC_PACKET_ID;
            KdpPacketIdExpected = INITIAL_PACKET_ID;

            if (!(XboxHardwareInfo.BldrFlags & HARDWAREINFO_BLDR_0x100))
                KdpLiteOnly = 1;
            mask = KdpLiteOnly >= 0 ? 0x80 : 0x180;

            XboxHardwareInfo.Flags = InterlockedOr(&XboxHardwareInfo.Flags, mask);

            if (idk) {
                ANSI_STRING string;
                LONG idk2;
                KdPortGetByte(&idk2);
                RtlInitAnsiString(&string, "xboxkrnl.exe");
                DbgLoadImageSymbols(&string, KERNEL_BASE_ADDRESS, -1);
                if (KdPollBreakIn()) {
                    DbgBreakPointWithStatus(1);
                }
            }
        }
    }
}

void KdLogDbgPrint(STRING* str) {
    uint32_t length;
    KIRQL* oldIrql;

    oldIrql = KfRaiseIrql(IRQL_HIGH_LEVEL);
    length = str->Length;
    if (length > KD_PRINT_BUFFER_SIZE)
        length = KD_PRINT_BUFFER_SIZE;

    if ((KdPrintWritePointer + length) <= (KdPrintCircularBuffer + KD_PRINT_BUFFER_SIZE)) {
        uint32_t moveOff = KdpMoveMemory(KdPrintWritePointer, str->Buffer, length, true);
        KdPrintWritePointer += moveOff;
        if (KdPrintWritePointer >= KdPrintCircularBuffer + KD_PRINT_BUFFER_SIZE) {
            KdPrintWritePointer = KdPrintCircularBuffer;
            KdPrintRolloverCount++;
        }
    } else {
        uint32_t offset = (uint32_t)(KdPrintCircularBuffer + KD_PRINT_BUFFER_SIZE - KdPrintWritePointer);
        uint32_t moveLen = KdpMoveMemory(KdPrintWritePointer, str->Buffer, offset, true);
        if (moveLen == offset) {
            moveLen += KdpMoveMemory(KdPrintCircularBuffer, str->Buffer + offset, length - offset, true);
        }
        if (moveLen > offset) {
            KdPrintWritePointer = &KdPrintCircularBuffer[moveLen - offset];
            KdPrintRolloverCount++;
        } else {
            KdPrintWritePointer += moveLen;
            if (KdPrintWritePointer >= KdPrintCircularBuffer + KD_PRINT_BUFFER_SIZE) {
                KdPrintWritePointer = KdPrintCircularBuffer;
                KdPrintRolloverCount++;
            }
        }
    }

    KfLowerIrql(oldIrql);
}

void KdNotifyQuiesce(uint32_t notification) {
    if ((XboxHardwareInfo.Flags & 0x80) && KdNetIsEnabled()) {
        KIRQL* old = KfRaiseIrql(IRQL_HIGH_LEVEL);
        KdpNetQuiesce(notification);
        KfLowerIrql(old);
    }
}
