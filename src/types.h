//
// not just types but also general stuff and utils
//

#pragma once

#define ALLOC_SECT(section) __declspec(allocate(section))
#define offsetof(type, member) ((size_t)&(((type*)0)->member))

// booleans and stuff
typedef unsigned int bool;
#define true 1
#define false 0
#define nullptr 0x0
#define NULL 0

/*
    cstdint primitives
*/
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef uint32_t uintptr_t;
typedef uint32_t size_t;

/*
    cooler primitives
*/
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long s32;
typedef unsigned long u32;
typedef signed long long s64;
typedef unsigned long long u64;
typedef float f32;
typedef double f64;

/*
    windows types (bleh)
*/
typedef void VOID, *PVOID, *LPVOID;
typedef unsigned char BYTE, *PBYTE;
typedef signed char CHAR;
typedef unsigned char UCHAR;
typedef signed short SHORT;
typedef unsigned short USHORT;
typedef unsigned short WORD;
typedef signed int INT;
typedef unsigned int UINT, UINT_PTR, *PUINT_PTR;
typedef signed long LONG;
typedef unsigned long ULONG;
typedef unsigned long DWORD, *PDWORD;
typedef signed long long LONGLONG;
typedef unsigned long long ULONGLONG;
typedef unsigned long long DWORDLONG;

typedef signed int BOOL;
typedef BYTE BOOLEAN;
#define TRUE 1
#define FALSE 0

typedef uint64_t ULONG_PTR;
typedef ULONG_PTR* PULONG_PTR;

typedef short WCHAR;
typedef char CHAR;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef short* LPWSTR;
typedef const short* LPCWSTR;

typedef union _LARGE_INTEGER {
    struct {
        LONG HighPart;
        DWORD LowPart;
    };
    struct {
        LONG HighPart;
        DWORD LowPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
