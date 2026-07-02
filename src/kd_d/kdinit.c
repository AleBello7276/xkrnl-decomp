
#include <krnl.h>

#include "kddata.h"

bool KdNetIsEnabled();
KIRQL* KfRaiseIrql(KIRQL);
void KfLowerIrql(KIRQL*);
void KdpNetQuiesce(uint32_t);

uint32_t KdpMoveMemory(uint8_t* dest, uint8_t* src, uint32_t len, bool unk);

void KdLogDbgPrint(STRING* str) {
    KIRQL* oldIrql;
    uint32_t length;

    oldIrql = KfRaiseIrql(IRQL_HIGH_LEVEL);
    length = str->Length;
    if (length > KD_PRINT_BUFFER_SIZE)
        length = KD_PRINT_BUFFER_SIZE;

    if ((KdPrintWritePointer + length) <= (KdPrintCircularBuffer + KD_PRINT_BUFFER_SIZE)) {
        uint32_t moveOff = KdpMoveMemory(KdPrintWritePointer, str->Buffer, length, true);
        KdPrintWritePointer += moveOff;
        if (KdPrintWritePointer >= KdPrintWritePointer + KD_PRINT_BUFFER_SIZE) {
            KdPrintRolloverCount++;
            KdPrintWritePointer = KdPrintCircularBuffer;
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
            if (KdPrintWritePointer >= KdPrintWritePointer + KD_PRINT_BUFFER_SIZE) {
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
