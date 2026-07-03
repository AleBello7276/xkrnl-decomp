#pragma once

long _InterlockedOr(volatile long* Value, long Mask);

void* __GPRGetReg(unsigned int);
void __GPRSetReg(unsigned int, unsigned long long);
#define GetGPR(x) (uint64_t)__GPRGetReg(x)
#define SetGPR(x, y) __GPRSetReg(x, y)

unsigned int __getr13();
void __setr13(unsigned long long);
#define GetR13() __getr13()
#define SetR13(x) __setr13(x)
#define GetKPCR ((KPCR*)GetR13())

void __emit(unsigned int op);
#define __eieio() __emit(0x7c0006ac)
#define __sync() __emit(0x7C0004AC)
#define __lwsync() __emit(0x7c2004ac)
unsigned long long __mftb();
#define mftb32() ((unsigned int)__mftb());
