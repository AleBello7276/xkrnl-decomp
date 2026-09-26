#pragma once

#include <krnl.h>

/* */
void KfReleaseSpinLock(PKSPIN_LOCK, KIRQL);

/* */
KIRQL KfAcquireSpinLock(PKSPIN_LOCK);

/* */
void KeAcquireSpinLockAtRaisedIrql(PKSPIN_LOCK pSpinLock);

/* */
void KeReleaseSpinLockFromRaisedIrql(PKSPIN_LOCK pSpinLock);
