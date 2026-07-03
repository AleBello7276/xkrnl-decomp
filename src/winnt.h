#pragma once

#include <intrinsics.h>
#include <types.h>

#define INLINE __inline

INLINE LONG InterlockedIncrement(LONG* Addend) {
    LONG v = _InterlockedIncrement(Addend);
    __lwsync();
    return v;
}

INLINE LONG InterlockedDecrement(LONG* Addend) {
    LONG v = _InterlockedDecrement(Addend);
    __lwsync();
    return v;
}

INLINE LONG InterlockedOr(LONG* value, LONG mask) {
    LONG v = _InterlockedOr(value, mask);
    __lwsync();
    return v;
}
