//
// not just types, general stuff and utils
//

#define INLINE_ASM_FUNC(name)                                                                                \
    __declspec(naked) void name(void) {                                                                      \
        __asm {
#define END_ASM_FUNC                                                                                         \
    }                                                                                                        \
    }

#define ALLOC_SECT(section) __declspec(allocate(section))

// general offsetof, used also for inline assembly, specification mentions the possibility of doing :
// `offset SomeStruct.field(reg)` but that seam to not work idk why
#define offsetof(type, member) ((size_t)&(((type*)0)->member))

typedef unsigned char uint8_t;
