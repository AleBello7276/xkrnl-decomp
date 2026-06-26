//
// not just types, general stuff and utils
//

#define ALLOC_SECT(section) __declspec(allocate(section))

#define offsetof(type, member) ((size_t)&(((type*)0)->member))

typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

typedef uint32_t uintptr_t;

// booleans
typedef unsigned int bool;
#define true 1
#define false 0

#define nullptr 0x0

typedef uint32_t KSPIN_LOCK;  // Kernel Spin Lock

typedef uint32_t size_t;

typedef void VOID;
typedef VOID* PVOID;

typedef short CSHORT;
typedef uint64_t ULONG_PTR;
typedef ULONG_PTR* PULONG_PTR;
