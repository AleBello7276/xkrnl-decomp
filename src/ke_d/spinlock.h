#pragma once

#include <krnl.h>

/* */
void KfReleaseSpinLock(PKSPIN_LOCK, KIRQL);

/* */
KIRQL KfAcquireSpinLock(PKSPIN_LOCK);
