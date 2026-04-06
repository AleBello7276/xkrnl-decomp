#pragma once

#include <krnl.h>

#define KD_PRINT_BUFFER_SIZE 0x3000

extern uint8_t KdPrintCircularBuffer[KD_PRINT_BUFFER_SIZE];
extern uint8_t* KdPrintWritePointer;

extern MEMORY_DESCRIPTOR KdPhysicalMemoryDescriptor;

extern uint32_t KdPrintRolloverCount;
