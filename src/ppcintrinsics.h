/**************************************************************************
 *
 * Copyright (c) Microsoft Corporation. All rights reserved.
 *
 * Module Name:
 *
 *     PPCIntrinsics.h
 *
 * Abstract:
 *     Platform-specific intrinsics.
 *
 **************************************************************************/

#ifndef __PPCINTRINSICS_H__
#define __PPCINTRINSICS_H__

#ifdef __cplusplus
extern "C" {
#endif

//////////////////////////////////////////////////////////////////////////////
// Standard emit-any-opcode intrinsic
void __emit(unsigned int opcode);

//////////////////////////////////////////////////////////////////////////////
// load/store with update intrinsics

/*
  loadbyteupdate(val, offset, base)
  is equivalent to

  val = *(base + offset);
  base += offset;

  All done in one instruction.  The compiler will store changes to "base"
  back to the variable itself if necessary (it will do so all the time
  in /Od builds).

  Note that the address of the load is exactly base + offset.  Offset is not
  multiplied by the size of the type, it is a byte offset.

  This code will generate the indexed form of the instruction if the
  offset cannot be lowered into an integer constant.

 */
unsigned char __loadbyteupdate(int offset, /* INOUT */ void* base);
unsigned short __loadshortupdate(int offset, /* INOUT */ void* base);
unsigned long __loadwordupdate(int offset, /* INOUT */ void* base);

float __loadfloatupdate(int offset, /* INOUT */ void* base);
double __loaddoubleupdate(int offset, /* INOUT */ void* base);

unsigned __int64 __loaddoublewordupdate(int offset, /* INOUT */ void* base);

/*
  The reason we have these wrapper macros is because the form of the intrinsics
  may change.  This will insulate developers from that change.
 */
#define loadbyteupdate(val, offset, base) val = __loadbyteupdate(offset, base)
#define loadshortupdate(val, offset, base) val = __loadshortupdate(offset, base)
#define loadwordupdate(val, offset, base) val = __loadwordupdate(offset, base)
#define loadfloatupdate(val, offset, base) val = __loadfloatupdate(offset, base)
#define loaddoubleupdate(val, offset, base) val = __loaddoubleupdate(offset, base)
#define loaddoublewordupdate(val, offset, base) val = __loaddoublewordupdate(offset, base)

void __storebyteupdate(unsigned char val, int offset, /* INOUT */ void* base);
void __storeshortupdate(unsigned short val, int offset, /* INOUT */ void* base);
void __storewordupdate(unsigned long val, int offset, /* INOUT */ void* base);
void __storefloatupdate(float val, int offset, /* INOUT */ void* base);
void __storedoubleupdate(double val, int offset, /* INOUT */ void* base);
void __storedoublewordupdate(unsigned __int64 val, int offset, /* INOUT */ void* base);

#define storebyteupdate __storebyteupdate
#define storeshortupdate __storeshortupdate
#define storewordupdate __storewordupdate
#define storefloatupdate __storefloatupdate
#define storedoubleupdate __storedoubleupdate
#define storedoublewordupdate __storedoublewordupdate

//////////////////////////////////////////////////////////////////////////////
// Cache Control
void __dcbz128(int offset, void* base);
void __dcbz(int offset, void* base);
void __dcbt(int offset, const void* base);
// The __xdcbt intrinsic has been removed due to bugs in this instruction.
void __dcbst(int offset, const void* base);
void __dcbf(int offset, const void* base);

//////////////////////////////////////////////////////////////////////////////
// Byte reversal
// Note that the non-volatile versions of the intrinsics will not honor volatileness
// even if base is volatile.
unsigned short __loadshortbytereverse(int offset, const void* base);
unsigned long __loadwordbytereverse(int offset, const void* base);

void __storeshortbytereverse(unsigned short val, int offset, void* base);
void __storewordbytereverse(unsigned int val, int offset, void* base);

#if (_MSC_FULL_VER >= 14002303)

unsigned __int64 __loaddoublewordbytereverse(int offset, const void* base);
unsigned __int64 __loaddoublewordbytereverseG(int offset, const void* base);
unsigned __int64 __loadvolatiledoublewordbytereverse(int offset, const void* base);
unsigned __int64 __loadvolatiledoublewordbytereverseG(int offset, const void* base);
void __storedoublewordbytereverse(unsigned __int64 val, int offset, void* base);
void __storedoublewordbytereverseG(unsigned __int64 val, int offset, void* base);
void __storevolatiledoublewordbytereverse(unsigned __int64 val, int offset, void* base);
void __storevolatiledoublewordbytereverseG(unsigned __int64 val, int offset, void* base);

#else

// not implemented, provide your own

#endif

// These aggressive G-forms only have global interference.  If you use these
// forms you must not be loading/storing to a stack location.
#if (_MSC_FULL_VER > 14001509)
unsigned short __loadshortbytereverseG(int offset, const void* base);
unsigned long __loadwordbytereverseG(int offset, const void* base);
void __storeshortbytereverseG(unsigned short val, int offset, void* base);
void __storewordbytereverseG(unsigned int val, int offset, void* base);
#else
#define __loadshortbytereverseG __loadshortbytereverse
#define __loadwordbytereverseG __loadwordbytereverse
#define __storeshortbytereverseG __storeshortbytereverse
#define __storewordbytereverseG __storewordbytereverse
#endif

// These volatile intrinsics will never get destroyed by optimizations and will always
// emit their respective instruction
unsigned short __loadvolatileshortbytereverse(int offset, volatile const void* base);
unsigned long __loadvolatilewordbytereverse(int offset, volatile const void* base);

void __storevolatileshortbytereverse(unsigned short val, int offset, volatile void* base);
void __storevolatilewordbytereverse(unsigned int val, int offset, volatile void* base);

#if (_MSC_FULL_VER > 14001524)
unsigned short __loadvolatileshortbytereverseG(int offset, const void volatile* base);
unsigned long __loadvolatilewordbytereverseG(int offset, volatile const void* base);

void __storevolatileshortbytereverseG(unsigned short val, int offset, volatile void* base);
void __storevolatilewordbytereverseG(unsigned int val, int offset, volatile void* base);
#else
#define __loadvolatileshortbytereverseG __loadvolatileshortbytereverse
#define __loadvolatilewordbytereverseG __loadvolatilewordbytereverse

#define __storevolatileshortbytereverseG __storevolatileshortbytereverse
#define __storevolatilewordbytereverseG __storevolatilewordbytereverse

#endif

// Standard intrinsics, declared here for completeness
unsigned short _byteswap_ushort(unsigned short value);
unsigned long _byteswap_ulong(unsigned long value);
unsigned __int64 _byteswap_uint64(unsigned __int64 value);

// multiply high intrinsics
__int64 __mulh(__int64 qwA, __int64 qwB);
unsigned __int64 __umulh(unsigned __int64 qwA, unsigned __int64 qwB);

// Map PowerPC mul high instructions to standard VS mul high intrinsics
#define __mulhd __mulh
#define __mulhdu __umulh

//////////////////////////////////////////////////////////////////////////////
// floating point

double __fsqrt(double fval);
float __fsqrts(float fval);
double __frsqrte(double fval);
float __fres(double fval);
#define __fabs fabs

// if fComparand >= 0, return fValGE else returns fLT.  If fComparand is NaN,
// returns fLT
double __fsel(double fComparand, double fValGE, double fLT);
float __fself(float fComparand, float fValGE, float fLT);
#define fpmax(a, b) __fsel((a) - (b), a, b)
#define fpmin(a, b) __fsel((a) - (b), b, a)

#if (_MSC_FULL_VER > 14001727)

// These convert a float/double into an integer, but keep them in the FPR.
// Store it out as a double, then later on load them in as integers manually
// to avoid the load-hit-store flush of float conversions.
//
// They must be double so stfd can be used to store the values without any further tampering.
// Storing to misaligned memory is probably cheaper than the LHS flush.

// round to zero
double __fctiwz(double fParam);
double __fctidz(double fParam);
#endif

#if (_MSC_FULL_VER > 14001729)
// round with current round mode
double __fctiw(double fParam);
double __fctid(double fParam);

// convert an int64 bit pattern that is in a FPR to double with
// current round mode
double __fcfid(double i64Param);
#endif

#if (_MSC_FULL_VER > 14006016)
void __stfiwx(double fParam, int offset, void* base);
#endif

// floating point round.  uses fctid, fcfid pair to round a double
// to an integer using current rounding mode.  May not work for massively
// large values.
double __frnd(double fRoundee);

//////////////////////////////////////////////////////////////////////////////
// Count leading zeros
unsigned int _CountLeadingZeros64(__int64 val);
unsigned int _CountLeadingZeros(long val);

//////////////////////////////////////////////////////////////////////////////
// Read time base counter.
unsigned __int64 __mftb();
// Return the low 32-bits of the mftb counter. This avoids brief inaccurate
// values in the high 32-bits for four cycles every time the bottom 32-bits
// wraps around, while still going ~85 seconds without wrapping.
// QueryPerformanceCounter also avoids this problem.
__inline unsigned int __mftb32() {
    return (unsigned int)__mftb();
}

//////////////////////////////////////////////////////////////////////////////
// Synchronization
#define __sync() __emit(0x7c0004ac)
#define __lwsync() __emit(0x7C2004AC)
#define __eieio() __emit(0x7c0006ac)
// Windows compatible name for a memory barrier. Note that __lwsync is not
// technically a full barrier as reads can still cross __lwsync to be moved
// ahead of writes to different addresses. __sync should be used if you need
// a full barrier.
#define MemoryBarrier __lwsync

//////////////////////////////////////////////////////////////////////////////
// Control hardware thread priority
//
// If there is a priority disparity between the two hardware threads, the
// higher priority thread will get 31 out of 32 dispatch slots.
//
// Idle thread is always at low priority.
//
// Threads doing spin loops may want to use low priority for the spin loop
// (resetting priority when the loop ends) in order to leave more dispatch
// slots for the other thread on the same core.
// Threads are set to medium priority after every context switch and interrupt.
//
void __SetHWThreadPriorityHigh(void);
void __SetHWThreadPriorityMed(void);
void __SetHWThreadPriorityLow(void);

//////////////////////////////////////////////////////////////////////////////
// Trap intrinsics

// if ((          a <            b) && (to & 16)) ___debugbreak();
// if ((          a >            b) && (to &  8)) ___debugbreak();
// if ((          a ==           b) && (to &  4)) ___debugbreak();
// if (((unsigned)a <  (unsigned)b) && (to &  2)) ___debugbreak();
// if (((unsigned)a >  (unsigned)b) && (to &  1)) ___debugbreak();

// 'to' must be a literal constant.  For twi and tdi, 'b' must be a literal
// constant.
void __tw(const unsigned int to, int a, int b);
void __twi(const unsigned int to, int a, const int b);
void __td(const unsigned int to, __int64 a, __int64 b);
void __tdi(const unsigned int to, __int64 a, const __int64 b);

#define __trap() __debugbreak()

#define __twlt(a, b) __tw(16, a, b)
#define __twle(a, b) __tw(20, a, b)
#define __tweq(a, b) __tw(4, a, b)
#define __twge(a, b) __tw(12, a, b)
#define __twgt(a, b) __tw(8, a, b)
#define __twnl(a, b) __tw(12, a, b)
#define __twne(a, b) __tw(24, a, b)
#define __twng(a, b) __tw(20, a, b)
#define __twllt(a, b) __tw(2, a, b)
#define __twlle(a, b) __tw(6, a, b)
#define __twlge(a, b) __tw(5, a, b)
#define __twlgt(a, b) __tw(1, a, b)
#define __twlnl(a, b) __tw(5, a, b)
#define __twlng(a, b) __tw(6, a, b)

#define __tdlt(a, b) __td(16, a, b)
#define __tdle(a, b) __td(20, a, b)
#define __tdeq(a, b) __td(4, a, b)
#define __tdge(a, b) __td(12, a, b)
#define __tdgt(a, b) __td(8, a, b)
#define __tdnl(a, b) __td(12, a, b)
#define __tdne(a, b) __td(24, a, b)
#define __tdng(a, b) __td(20, a, b)
#define __tdllt(a, b) __td(2, a, b)
#define __tdlle(a, b) __td(6, a, b)
#define __tdlge(a, b) __td(5, a, b)
#define __tdlgt(a, b) __td(1, a, b)
#define __tdlnl(a, b) __td(5, a, b)
#define __tdlng(a, b) __td(6, a, b)

#define __twlti(a, b) __twi(16, a, b)
#define __twlei(a, b) __twi(20, a, b)
#define __tweqi(a, b) __twi(4, a, b)
#define __twgei(a, b) __twi(12, a, b)
#define __twgti(a, b) __twi(8, a, b)
#define __twnli(a, b) __twi(12, a, b)
#define __twnei(a, b) __twi(24, a, b)
#define __twngi(a, b) __twi(20, a, b)
#define __twllti(a, b) __twi(2, a, b)
#define __twllei(a, b) __twi(6, a, b)
#define __twlgei(a, b) __twi(5, a, b)
#define __twlgti(a, b) __twi(1, a, b)
#define __twlnli(a, b) __twi(5, a, b)
#define __twlngi(a, b) __twi(6, a, b)

#define __tdlti(a, b) __tdi(16, a, b)
#define __tdlei(a, b) __tdi(20, a, b)
#define __tdeqi(a, b) __tdi(4, a, b)
#define __tdgei(a, b) __tdi(12, a, b)
#define __tdgti(a, b) __tdi(8, a, b)
#define __tdnli(a, b) __tdi(12, a, b)
#define __tdnei(a, b) __tdi(24, a, b)
#define __tdngi(a, b) __tdi(20, a, b)
#define __tdllti(a, b) __tdi(2, a, b)
#define __tdllei(a, b) __tdi(6, a, b)
#define __tdlgei(a, b) __tdi(5, a, b)
#define __tdlgti(a, b) __tdi(1, a, b)
#define __tdlnli(a, b) __tdi(5, a, b)
#define __tdlngi(a, b) __tdi(6, a, b)

//////////////////////////////////////////////////////////////////////////////
// Miscellaneous

// Not a PPC specific intrinsic but specific to our compiler.
// Returns 1 if val is an integer constant at compile time.  This means
// #define'd constants, enumerations and of course integer literals.
// This does NOT mean static const variables, const variables or any variables
// that become constant integers as a result of optimizations.  It also does
// not include addresses of variables or functions.
int __IsIntConst(int val);

// Give up instruction dispatch slots for 16 cycles, to let the other thread
// on this core run faster.
#define YieldProcessor() __emit(0x7FFFFB78)  // or r31,r31,r31

#ifdef __cplusplus
}
#endif

#endif
