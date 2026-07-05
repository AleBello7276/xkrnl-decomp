#pragma once

#include "kd.h"
#include "winnt.h"
#include <krnl.h>


void KdInitializeSystem(bool idk);
void KdLogDbgPrint(STRING* str);
void KdNotifyQuiesce(uint32_t notification);
