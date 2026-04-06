#include "kddata.h"

uint8_t KdPrintCircularBuffer[KD_PRINT_BUFFER_SIZE] = {0};
uint8_t* KdPrintWritePointer = KdPrintCircularBuffer;

MEMORY_DESCRIPTOR KdPhysicalMemoryDescriptor = {0};

uint32_t KdPrintRolloverCount = 0;
