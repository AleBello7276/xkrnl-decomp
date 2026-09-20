#pragma once
#include <types.h>

/* some info and error names comes from: https://xenonlibrary.com/wiki/Errors*/

/*
 * the format is as shown (i think)
 *
 *  23             16 15              8 7               0
 * +-----------------+-----------------+-----------------+
 * |  patternField   |    bitIndex     |    errorCode    |
 * +-----------------+-----------------+-----------------+
 */

typedef DWORD PatternError;

// figure names / purpose later
typedef enum PatternField {
    PATTERN_FIELD_0 = 0,
    PATTERN_FIELD_1 = 1,
    /* ... */
    PATTERN_FIELD_MAX = 0xFF
} PatternField;

enum PatternBit {

    PATTERN_BIT_SOMETHING_32 = 0x20
};

// figure names / purpose later
typedef enum PatternErrorCode {
    PATTERN_ERROR_NONE = 0,
    UEM_XSS_CDROM_TIMED_OUT = 64,
    UEM_XSS_CDROM_NOT_DMA_CONFIGURED = 65,
    UEM_XSS_CDROM_COULD_NOT_CREATE_DEVICE = 66,
    UEM_XSS_HDD_TIMED_OUT = 67,
    UEM_XSS_HDD_NOT_DMA_CONFIGURED = 68,
    UEM_XSS_HDD_COULD_NOT_CREATE_DEVICE = 69,
    UEM_XSS_HDD_NOT_FOUND = 70,
    UEM_XSS_XAM_NO_DASH = 71,
    UEM_SMC_FLASH_CONFIG_CORRUPT = 72,
    UEM_HSIO_TRAINING_FAILED = 73,
    // UEM_HSIO_CALIBRATION_FAILED = 74, /* Original Xbox 360 */
    UEM_UNEXPECTED_ETHERNET_PHY_VENDOR = 75,
    UEM_ETHERNET_PHY_RESET_FAIL = 76,
    UEM_ETHERNET_PHY_READ_WRITE_FAIL = 77,
    UEM_UNSUPPORTED_GPU = 78,
    UEM_XSS_FLASH_NO_XAM = 79,
    UEM_UNK_80 = 80,
    UEM_UNK_81 = 81,
    // UEM_HSIO_CALIBRATION_FAILED = 82, Xbox 360 S and E

    // code rejects values > 99
    PATTERN_ERROR_MAX = 99
} PatternErrorCode;

// layout
#define PATTERN_FIELD_SHIFT 16
#define PATTERN_BIT_INDEX_SHIFT 8
#define PATTERN_ERROR_SHIFT 0

#define PATTERN_COMPONENT_MASK 0xFF
#define PATTERN_VALUE_MASK 0xFFFFFF

// make a packed value
#define PATTERN_MAKE(field, bit, error)                                                                      \
    ((((DWORD)(field) & PATTERN_COMPONENT_MASK) << PATTERN_FIELD_SHIFT)                                      \
     | (((DWORD)(bit) & PATTERN_COMPONENT_MASK) << PATTERN_BIT_INDEX_SHIFT)                                  \
     | (((DWORD)(error) & PATTERN_COMPONENT_MASK) << PATTERN_ERROR_SHIFT))

// extract macros
#define PATTERN_GET_FIELD(value) ((BYTE)(((DWORD)(value) >> PATTERN_FIELD_SHIFT) & PATTERN_COMPONENT_MASK))

#define PATTERN_GET_BIT_INDEX(value)                                                                         \
    ((BYTE)(((DWORD)(value) >> PATTERN_BIT_INDEX_SHIFT) & PATTERN_COMPONENT_MASK))

#define PATTERN_GET_ERROR(value) ((BYTE)(((DWORD)(value) >> PATTERN_ERROR_SHIFT) & PATTERN_COMPONENT_MASK))

#define ERROR_XSS_CDROM_COULD_NOT_CREATE_DEVICE                                                              \
    PATTERN_MAKE(PATTERN_FIELD_1, PATTERN_BIT_SOMETHING_32, UEM_XSS_CDROM_COULD_NOT_CREATE_DEVICE)
