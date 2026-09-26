#pragma once

#include "sata_d/sata.h"
#include <types.h>

/* */
void IoCompleteRequest(SATA_REQUEST* pRequest, DWORD Priority);

/* */
void IoReleaseCancelSpinLock();
